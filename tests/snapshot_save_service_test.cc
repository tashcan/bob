#define MOD_SNAPSHOT_QUEUE_TESTING
#define MOD_SNAPSHOT_SERVICE_TESTING
#include "snapshot_save_queue.cc"
#include "snapshot_save_worker.cc"
#include "snapshot_save_service.cc"
#include <cassert>
#include <condition_variable>
#include <future>
#include <iostream>

namespace persistence
{
namespace
{
std::mutex backendMutex;
std::condition_variable changed;
bool entered = false, release = false;
int finishedB = 0;
bool failSecondStart = false;
void BeforeServiceWorkerStart(std::size_t index)
{
  if (failSecondStart && index == 1) throw std::runtime_error("injected thread-start failure");
}
file_transaction::Result Execute(const std::filesystem::path& path, std::string_view)
{
  std::unique_lock lock(backendMutex);
  if (path.filename() == "a.vars") {
    entered = true;
    changed.notify_all();
    changed.wait(lock, [] { return release; });
  } else {
    ++finishedB;
    changed.notify_all();
  }
  file_transaction::Result result;
  result.state = file_transaction::State::Committed;
  return result;
}
}
}

int main()
{
  using namespace persistence;
  using Admission = SnapshotSaveQueue::Admission;
  std::promise<void> done;
  std::thread watchdog([future = done.get_future()] {
    if (future.wait_for(std::chrono::seconds(15)) == std::future_status::timeout) std::abort();
  });
  const auto root = std::filesystem::temp_directory_path();
  const std::array paths{root / "a.vars", root / "b.vars"};
  auto rejects = [](auto&& construct) {
    bool rejected = false;
    try { construct(); } catch (const std::exception&) { rejected = true; }
    assert(rejected);
  };
  rejects([&] { SnapshotSaveService invalid(std::span<const std::filesystem::path>{}); });
  const std::array tooMany{paths[0], paths[1], root/"c.vars", root/"d.vars", root/"e.vars"};
  rejects([&] { SnapshotSaveService invalid(tooMany); });
  const std::array aliases{paths[0], root / "." / "A.vars"};
  rejects([&] { SnapshotSaveService invalid(aliases); });
  const std::array lockCollision{root / "snapshot.vars", root / "snapshot.vars.lock"};
  const std::array reverseLockCollision{lockCollision[1], lockCollision[0]};
  const std::array caseLockCollision{root / "snapshot.vars", root / "SNAPSHOT.VARS.LOCK"};
  rejects([&] { SnapshotSaveService invalid(lockCollision); });
  rejects([&] { SnapshotSaveService invalid(reverseLockCollision); });
  rejects([&] { SnapshotSaveService invalid(caseLockCollision); });
#if defined(_WIN32)
  const std::array dotAliases{root / "stfc-service-absent.vars", root / "stfc-service-absent.vars."};
  const std::array spaceAliases{root / "stfc-service-absent.vars", root / "stfc-service-absent.vars "};
  const std::array shortAliases{root / "stfc-service-long-absent.vars", root / "STFCSE~1.VAR"};
  rejects([&] { SnapshotSaveService invalid(dotAliases); });
  rejects([&] { SnapshotSaveService invalid(spaceAliases); });
  rejects([&] { SnapshotSaveService invalid(shortAliases); });
  const auto parentWithoutSeparator = root.filename().empty() ? root.parent_path() : root;
  const std::array parentSpelling{std::filesystem::path(parentWithoutSeparator.native() + L".") / "a.vars"};
  rejects([&] { SnapshotSaveService invalid(parentSpelling); });
#endif
  failSecondStart = true;
  rejects([&] { SnapshotSaveService partial(paths); });
  failSecondStart = false; // A prior worker must have joined and released its lease.
  SnapshotSaveService::Destination old;
  {
    SnapshotSaveService service(paths);
    old = service.GetDestination(0);
    const auto b = service.GetDestination(1);
    // A failed second lease must not release the first one's process claim.
    rejects([&] { SnapshotSaveService duplicate(paths); });
    rejects([&] { SnapshotSaveService duplicate(paths); });
    std::string invalidBytes = "untouched";
    assert(service.TrySubmit({}, 1, std::move(invalidBytes)).state == Admission::InvalidRequest);
    assert(invalidBytes == "untouched");
    assert(service.TrySubmit(old, 1, std::string("first")).state == Admission::Accepted);
    {
      std::unique_lock lock(backendMutex);
      changed.wait(lock, [] { return entered; });
    }
    // Complete a different destination while A's backend remains stalled.
    assert(service.TrySubmit(b, 1, std::string("other")).state == Admission::Accepted);
    {
      std::unique_lock lock(backendMutex);
      changed.wait(lock, [] { return finishedB == 1; });
    }
    // Count bound includes A's in-flight ticket, with rejection retaining input.
    for (std::uint64_t revision = 2; revision <= SnapshotSaveQueue::MaxOutstanding; ++revision)
      assert(service.TrySubmit(old, revision, std::string("queued")).state == Admission::Accepted);
    std::string extra = "retained";
    assert(service.TrySubmit(old, 9, std::move(extra)).state == Admission::Busy && extra == "retained");
    service.RequestStop(StopMode::DrainAccepted);
    assert(service.TrySubmit(b, 2, std::string("late")).state == Admission::Stopping);
    auto joining = std::async(std::launch::async, [&] { service.StopAndJoin(StopMode::DrainAccepted); });
    {
      std::lock_guard lock(backendMutex);
      release = true;
    }
    changed.notify_all();
    joining.get();
    int completions = 0;
    while (auto completion = service.TryTakeCompletion(old)) {
      assert(completion->result.state == file_transaction::State::Committed);
      ++completions;
    }
    assert(completions == 8 && service.TryTakeCompletion(b).has_value());
  }
  {
    SnapshotSaveService replacement(paths);
    std::string stale = "old session";
    assert(replacement.TrySubmit(old, 10, std::move(stale)).state == Admission::InvalidRequest);
    assert(stale == "old session" && !replacement.TryTakeCompletion(old));
    replacement.StopAndJoin(StopMode::DrainAccepted);
  }
  done.set_value();
  watchdog.join();
  std::cout << "snapshot service tests passed\n";
}
