#define MOD_SNAPSHOT_QUEUE_TESTING
#define MOD_SNAPSHOT_SERVICE_TESTING
#define MOD_SNAPSHOT_HOST_TESTING
#include "snapshot_save_queue.cc"
#include "snapshot_save_worker.cc"
#include "snapshot_save_service.cc"
#include "snapshot_save_host.cc"
#define MOD_QUIT_DRAIN_GATE_TESTING
#include "quit_drain_gate.h"
#include <cassert>
#include <condition_variable>
#include <future>
#include <iostream>

namespace persistence
{
namespace
{
std::mutex testMutex;
std::condition_variable changed;
bool backendEntered = false, releaseBackend = false;
#if defined(_WIN32)
bool returnEntered = false, releaseReturn = false;
#endif
bool blockConstruction = false, constructionEntered = false, releaseConstruction = false;
std::atomic_bool blockVote{false};
bool voteEntered = false, releaseVote = false;
void BeforeQuitVoteCompareExchange()
{
  if (!blockVote.exchange(false)) return;
  std::unique_lock lock(testMutex);
  voteEntered = true;
  changed.notify_all();
  changed.wait(lock, [] { return releaseVote; });
}
void BeforeServiceWorkerStart(std::size_t)
{
  std::unique_lock lock(testMutex);
  if (!blockConstruction) return;
  constructionEntered = true;
  changed.notify_all();
  changed.wait(lock, [] { return releaseConstruction; });
}
file_transaction::Result Execute(const std::filesystem::path&, std::string_view bytes)
{
  std::unique_lock lock(testMutex);
  backendEntered = true;
  changed.notify_all();
  changed.wait(lock, [] { return releaseBackend; });
  file_transaction::Result result;
  result.state = bytes == "fail" ? file_transaction::State::NotCommitted : file_transaction::State::Committed;
  return result;
}
#if defined(_WIN32)
void BeforeHostThreadReturn()
{
  std::unique_lock lock(testMutex);
  returnEntered = true;
  changed.notify_all();
  changed.wait(lock, [] { return releaseReturn; });
}
#endif
} // namespace
} // namespace persistence

int main()
{
  using namespace persistence;
  using Admission = SnapshotSaveQueue::Admission;
  using State = SnapshotSaveHost::State;
  std::promise<void> done;
  std::thread watchdog([future = done.get_future()] {
    if (future.wait_for(std::chrono::seconds(15)) == std::future_status::timeout) std::abort();
  });
  QuitDrainGate gate;
  QuitDrainGate dormant;
  assert(dormant.Vote(true));
  assert(!dormant.TryActivate()); // A quit granted before launch forbids launch.
  assert(gate.TryActivate());
  assert(!gate.Vote(false) && !gate.DrainRequested());
  assert(!gate.TakeResumeRequest());
  {
    QuitDrainGate cancelled;
    assert(cancelled.TryActivate());
    assert(!cancelled.Vote(true) && cancelled.DrainRequested());
    assert(!cancelled.Vote(false));
    cancelled.ObserveStopped();
    assert(!cancelled.TakeResumeRequest()); // A later genuine veto cancels resume.
    assert(!cancelled.TryActivate()); // It does not resurrect stopped workers.
  }
  {
    QuitDrainGate racing;
    assert(racing.TryActivate() && !racing.Vote(true));
    blockVote.store(true);
    std::thread staleVote([&] { assert(racing.Vote(true)); });
    {
      std::unique_lock lock(testMutex);
      changed.wait(lock, [] { return voteEntered; });
    }
    racing.ObserveStopped();
    assert(racing.TakeResumeRequest());
    {
      std::lock_guard lock(testMutex);
      releaseVote = true;
      changed.notify_all();
    }
    staleVote.join();
    assert(!racing.TakeResumeRequest()); // Stale CAS cannot re-arm a consumed resume.
  }
  const auto destination = std::filesystem::temp_directory_path() / "stfc-host-fixture.vars";
#if defined(_WIN32)
  {
    SnapshotSaveHost host;
    assert(host.PollStopped());
    assert(host.Start({destination}));
    while (host.Status() == State::Starting) std::this_thread::yield();
    assert(host.Status() == State::Ready);
    auto handle = host.TryGetDestination(0);
    while (!handle) handle = host.TryGetDestination(0);
    assert(host.TrySubmit(*handle, 1, std::string("fail")).state == Admission::Accepted);
    {
      std::unique_lock lock(testMutex);
      changed.wait(lock, [] { return backendEntered; });
    }
    assert(host.TrySubmit(*handle, 2, std::string("success")).state == Admission::Accepted);
    assert(!gate.Vote(true) && gate.DrainRequested());
    host.RequestStop();
    std::string late = "unchanged";
    assert(host.TrySubmit(*handle, 3, std::move(late)).state == Admission::Stopping && late == "unchanged");
    assert(!host.PollStopped() && !gate.TakeResumeRequest());
    {
      std::lock_guard lock(testMutex);
      releaseBackend = true;
      changed.notify_all();
    }
    {
      std::unique_lock lock(testMutex);
      changed.wait(lock, [] { return returnEntered; });
    }
    // Service and worker destruction have completed, but the native supervisor
    // is deliberately still running. A published Stopped state is insufficient.
    assert(host.Status() == State::Stopped && !host.PollStopped());
    assert(!gate.TakeResumeRequest());
    int failures = 0, commits = 0;
    for (int i = 0; i != 2; ++i) {
      auto completion = host.TryTakeCompletion(0);
      assert(completion && completion->outcome == SnapshotSaveQueue::Outcome::Executed);
      if (completion->result.committed()) ++commits;
      else ++failures;
    }
    assert(failures == 1 && commits == 1 && !host.TryTakeCompletion(0));
    {
      std::lock_guard lock(testMutex);
      releaseReturn = true;
      changed.notify_all();
    }
    while (!host.PollStopped()) std::this_thread::yield();
    gate.ObserveStopped();
    assert(gate.TakeResumeRequest() && !gate.TakeResumeRequest());
    // Save failure cannot strand shutdown, but a real subscriber veto survives.
    assert(!gate.Vote(false) && !gate.TakeResumeRequest());
    assert(gate.Vote(true));
    assert(!host.Start({destination}));
  }
  {
    SnapshotSaveHost failed;
    // Existing directory is invalid as a snapshot target. Failure is asynchronous
    // and does not poison the process lease for the next independently owned host.
    assert(failed.Start({std::filesystem::temp_directory_path()}));
    while (!failed.PollStopped()) std::this_thread::yield();
    assert(failed.Status() == State::Unavailable);
  }
  {
    {
      std::lock_guard lock(testMutex);
      blockConstruction = true;
    }
    SnapshotSaveHost starting;
    assert(starting.Start({destination}));
    {
      std::unique_lock lock(testMutex);
      changed.wait(lock, [] { return constructionEntered; });
    }
    starting.RequestStop();
    assert(!starting.TryGetDestination(0) && !starting.PollStopped());
    {
      std::lock_guard lock(testMutex);
      releaseConstruction = true;
      changed.notify_all();
    }
    while (!starting.PollStopped()) std::this_thread::yield();
    assert(starting.Status() == State::Stopped);
  }
  {
    SnapshotSaveHost concurrent;
    assert(concurrent.Start({destination}));
    while (concurrent.Status() == State::Starting) std::this_thread::yield();
    auto handle = concurrent.TryGetDestination(0);
    while (!handle) handle = concurrent.TryGetDestination(0);
    std::atomic_int accepted{0};
    std::thread producer([&] {
      for (std::uint64_t revision = 1;; ++revision) {
        std::string bytes = "success";
        const auto result = concurrent.TrySubmit(*handle, revision, std::move(bytes));
        if (result.state == Admission::Accepted) ++accepted;
        else assert(bytes == "success");
        if (result.state == Admission::Stopping) break;
        std::this_thread::yield();
      }
    });
    while (accepted.load() == 0) std::this_thread::yield();
    concurrent.RequestStop();
    producer.join();
    while (!concurrent.PollStopped()) std::this_thread::yield();
    int completed = 0;
    while (auto result = concurrent.TryTakeCompletion(0)) {
      assert(result->result.committed());
      ++completed;
    }
    assert(completed == accepted.load());
  }
  {
    SnapshotSaveHost cancelled;
    cancelled.RequestStop();
    std::vector paths{destination};
    assert(!cancelled.Start(std::move(paths)) && paths.size() == 1);
  }
  std::cout << "native supervisor, retained failures, launch-stop and quit gate passed\n";
#else
  SnapshotSaveHost unsupported;
  std::vector paths{destination};
  assert(!unsupported.Start(std::move(paths)) && paths.size() == 1);
  assert(unsupported.Status() == State::Unavailable && unsupported.PollStopped());
  std::string bytes = "untouched";
  assert(unsupported.TrySubmit({}, 1, std::move(bytes)).state == Admission::InvalidRequest && bytes == "untouched");
  assert(!gate.Vote(true));
  gate.ObserveStopped();
  assert(gate.TakeResumeRequest() && !gate.TakeResumeRequest());
  assert(!gate.Vote(false) && gate.Vote(true));
  std::cout << "quit gate and unsupported native host rejection passed (no macOS lifecycle claim)\n";
#endif
  done.set_value();
  watchdog.join();
}
