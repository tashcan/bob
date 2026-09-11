#define MOD_SNAPSHOT_QUEUE_TESTING
#define MOD_SNAPSHOT_QUEUE_ADMISSION_TESTING
#include "snapshot_save_queue.cc"
#include "snapshot_save_worker.cc"

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <vector>

namespace persistence
{
namespace
{
  std::mutex              admissionTestMutex;
  std::condition_variable admissionChanged;
  bool                    pauseAdmission = false, admissionEntered = false, allowAdmission = false;
  void                    BeforeSnapshotEnqueue()
  {
    std::unique_lock lock(admissionTestMutex);
    if (!pauseAdmission)
      return;
    admissionEntered = true;
    admissionChanged.notify_all();
    admissionChanged.wait(lock, [] { return allowAdmission; });
  }
  std::mutex               backendMutex;
  std::condition_variable  backendChanged;
  bool                     entered = false, allow = false, block = true;
  std::vector<std::string> writes;
  file_transaction::State  backendState = file_transaction::State::Committed;
  file_transaction::Result Execute(const std::filesystem::path&, std::string_view bytes)
  {
    std::unique_lock lock(backendMutex);
    entered = true;
    backendChanged.notify_all();
    if (block)
      backendChanged.wait(lock, [] { return allow; });
    writes.emplace_back(bytes);
    file_transaction::Result result;
    result.state = backendState;
    return result;
  }
} // namespace
} // namespace persistence

int main()
{
  using namespace persistence;
  using Admission = SnapshotSaveQueue::Admission;
  using Outcome   = SnapshotSaveQueue::Outcome;
  std::promise<void> finished;
  std::thread        watchdog([future = finished.get_future()] {
    if (future.wait_for(std::chrono::seconds(15)) == std::future_status::timeout)
      std::abort();
  });
  const auto         destination = std::filesystem::temp_directory_path() / "stfc-worker-fixture.vars";
  {
    SnapshotSaveWorker owner(destination);
    auto               first = owner.TrySubmit(1, std::string("first"));
    assert(first.state == Admission::Accepted);
    {
      std::unique_lock lock(backendMutex);
      assert(backendChanged.wait_for(lock, std::chrono::seconds(3), [] { return entered; }));
    }
    auto second = owner.TrySubmit(2, std::string("cancel"));
    assert(second.state == Admission::Accepted);
    owner.RequestStop();
    assert(!owner.WorkEnded());
    assert(owner.TrySubmit(3, std::string("late")).state == Admission::Stopping);
    assert(!owner.TryTakeCompletion());
    {
      std::lock_guard lock(backendMutex);
      allow = true;
      backendChanged.notify_all();
    }
    owner.StopAndJoin(StopMode::CancelQueued);
    assert(owner.WorkEnded());
    auto done = owner.TryTakeCompletion();
    assert(done && done->ticket == first.ticket && done->outcome == Outcome::Executed && done->result.committed());
    done = owner.TryTakeCompletion();
    assert(done && done->ticket == second.ticket && done->outcome == Outcome::CancelledBeforeStart);
    assert(!owner.TryTakeCompletion());
    owner.StopAndJoin(StopMode::CancelQueued); // Explicit lifecycle operation is idempotent.
  }
  assert(writes == std::vector<std::string>{"first"});
  block = false;
  // A drain keeps accepted work executable while closing admission. Explicit
  // cancellation (before OR after drain) and uncertain storage failures override
  // that policy. Ordinary failure/durability outcomes must not become "saved".
  for (auto state : {file_transaction::State::Committed, file_transaction::State::NotCommitted,
                     file_transaction::State::DurabilityUnverified, file_transaction::State::RecoveryRequired}) {
    for (int cancelOrder : {0, 1, 2}) {
      entered = allow = false;
      block           = true;
      backendState    = state;
      writes.clear();
      SnapshotSaveWorker owner(destination);
      assert(owner.TrySubmit(1, std::string("active")).state == Admission::Accepted);
      {
        std::unique_lock lock(backendMutex);
        assert(backendChanged.wait_for(lock, std::chrono::seconds(3), [] { return entered; }));
      }
      assert(owner.TrySubmit(2, std::string("accepted")).state == Admission::Accepted);
      if (cancelOrder == 1)
        owner.RequestStop(StopMode::CancelQueued);
      owner.RequestStop(StopMode::DrainAccepted);
      if (cancelOrder == 2)
        owner.RequestStop(StopMode::CancelQueued);
      owner.RequestStop(StopMode::DrainAccepted); // Cannot undo cancellation.
      assert(!owner.WorkEnded());
      assert(owner.TrySubmit(3, std::string("late")).state == Admission::Stopping);
      {
        std::lock_guard lock(backendMutex);
        allow = true;
        backendChanged.notify_all();
      }
      owner.StopAndJoin(StopMode::DrainAccepted);
      auto active = owner.TryTakeCompletion();
      auto queued = owner.TryTakeCompletion();
      assert(active && active->outcome == Outcome::Executed && active->result.state == state);
      const bool cancelled = cancelOrder != 0 || state == file_transaction::State::RecoveryRequired;
      assert(queued && queued->revision == 2);
      assert(queued->outcome == (cancelled ? Outcome::CancelledBeforeStart : Outcome::Executed));
      if (!cancelled)
        assert(queued->result.state == state);
      assert(writes.size() == (cancelled ? 1u : 2u));
      assert(!owner.TryTakeCompletion());
    }
  }
  block        = false;
  backendState = file_transaction::State::Committed;
  {
    // Repeated empty-to-nonempty transitions exercise wake coalescing; no sleep
    // is needed on the producer to make a notification visible to the worker.
    SnapshotSaveWorker owner(destination);
    for (std::uint64_t revision = 1; revision <= 500; ++revision) {
      std::string bytes = std::to_string(revision);
      for (;;) {
        auto accepted = owner.TrySubmit(revision, std::move(bytes));
        if (accepted.state == Admission::Accepted)
          break;
        assert(accepted.state == Admission::Busy);
        std::this_thread::yield();
      }
      std::optional<SnapshotSaveQueue::Completion> done;
      while (!(done = owner.TryTakeCompletion()))
        std::this_thread::yield();
      assert(done->revision == revision && done->result.committed());
    }
    owner.StopAndJoin(StopMode::CancelQueued); // Wakes an idle worker too.
    assert(owner.WorkEnded());
  }
  // Deterministically admit a ticket AFTER stop, by holding the producer just
  // past the queue's final stop check. The worker must not exit ahead of it.
  for (auto mode : {StopMode::CancelQueued, StopMode::DrainAccepted}) {
    SnapshotSaveWorker owner(destination);
    {
      std::lock_guard lock(admissionTestMutex);
      pauseAdmission   = true;
      admissionEntered = allowAdmission = false;
    }
    SnapshotSaveQueue::Submission submitted{Admission::Busy};
    std::thread                   producer([&] { submitted = owner.TrySubmit(1, std::string("late ticket")); });
    {
      std::unique_lock lock(admissionTestMutex);
      assert(admissionChanged.wait_for(lock, std::chrono::seconds(3), [] { return admissionEntered; }));
    }
    owner.RequestStop(mode);
    assert(!owner.WorkEnded());
    {
      std::lock_guard lock(admissionTestMutex);
      allowAdmission = true;
      admissionChanged.notify_all();
    }
    producer.join();
    owner.StopAndJoin(mode);
    assert(submitted.state == Admission::Accepted);
    auto done = owner.TryTakeCompletion();
    assert(done && done->ticket == submitted.ticket);
    assert(done->outcome == (mode == StopMode::CancelQueued ? Outcome::CancelledBeforeStart : Outcome::Executed));
    assert(!owner.TryTakeCompletion());
    pauseAdmission = false;
  }
  // Also exercise unforced scheduler races; all admitted work is accounted for.
  for (int attempt = 0; attempt < 50; ++attempt) {
    SnapshotSaveWorker            owner(destination);
    SnapshotSaveQueue::Submission submitted{Admission::Busy};
    std::atomic_bool              go{false};
    std::thread                   producer([&] {
      while (!go.load())
        std::this_thread::yield();
      submitted = owner.TrySubmit(1, std::string("racing"));
    });
    go.store(true);
    owner.RequestStop();
    producer.join();
    owner.StopAndJoin(StopMode::CancelQueued);
    auto done = owner.TryTakeCompletion();
    if (submitted.state == Admission::Accepted)
      assert(done && done->ticket == submitted.ticket);
    else
      assert(!done && (submitted.state == Admission::Stopping || submitted.state == Admission::Busy));
    assert(!owner.TryTakeCompletion());
  }
  finished.set_value();
  watchdog.join();
  std::cout << "PASS worker wakeup, stalled stop, cancellation, retained results and admission/stop races\n";
}
