#define MOD_SNAPSHOT_QUEUE_TESTING
#include "snapshot_save_queue.cc"

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

namespace persistence
{
namespace
{
  std::mutex               backendMutex;
  std::condition_variable  backendChanged;
  bool                     block = false, entered = false, allow = false, throwFailure = false;
  file_transaction::State  backendState = file_transaction::State::Committed;
  std::vector<std::string> writes;

  file_transaction::Result Execute(const std::filesystem::path&, std::string_view bytes)
  {
    std::unique_lock lock(backendMutex);
    entered = true;
    backendChanged.notify_all();
    if (block)
      backendChanged.wait(lock, [] { return allow; });
    if (throwFailure)
      throw std::runtime_error("backend failure");
    writes.emplace_back(bytes);
    file_transaction::Result result;
    result.state = backendState;
    return result;
  }
} // namespace
} // namespace persistence

int main()
{
  // A blocking admission regression must fail the fixture, not hang CI forever.
  std::promise<void> finished;
  std::thread        watchdog([future = finished.get_future()] {
    if (future.wait_for(std::chrono::seconds(15)) == std::future_status::timeout)
      std::abort();
  });
  using namespace persistence;
  using Queue     = SnapshotSaveQueue;
  using Admission = Queue::Admission;
  // Backend is isolated in memory: no fixture writes to a real config.
  const auto  destination = std::filesystem::temp_directory_path() / "stfc-queue-fixture.vars";
  Queue       queue(destination);
  std::string invalid = "keep";
  assert(queue.TrySubmit(0, std::move(invalid)).state == Admission::InvalidRequest && invalid == "keep");
  std::string oversized(Queue::MaxPayload + 1, 'x');
  assert(queue.TrySubmit(1, std::move(oversized)).state == Admission::InvalidRequest);
  std::string excessiveCapacity = "small";
  excessiveCapacity.reserve(Queue::MaxRetainedBytes + 1);
  assert(queue.TrySubmit(1, std::move(excessiveCapacity)).state == Admission::InvalidRequest);

  // Hold storage indefinitely until the test releases it; admission/polling and
  // stop must finish while the worker is still inside that backend.
  auto first = queue.TrySubmit(1, std::string("first"));
  assert(first.state == Admission::Accepted && first.ticket);
  {
    std::lock_guard lock(backendMutex);
    block = true;
  }
  std::thread worker([&] { assert(queue.RunOne()); });
  {
    std::unique_lock lock(backendMutex);
    assert(backendChanged.wait_for(lock, std::chrono::seconds(3), [] { return entered; }));
  }
  assert(!queue.RunOne()); // No second disk transaction while the first is active.
  assert(!queue.TryTakeCompletion());
  auto second = queue.TrySubmit(2, std::string("second"));
  assert(second.state == Admission::Accepted);
  std::string stale = "stale";
  assert(queue.TrySubmit(1, std::move(stale)).state == Admission::StaleRevision && stale == "stale");
  for (std::uint64_t revision = 3; revision <= Queue::MaxOutstanding; ++revision)
    assert(queue.TrySubmit(revision, std::string("queued")).state == Admission::Accepted);
  std::string rejected = "retry";
  assert(queue.TrySubmit(9, std::move(rejected)).state == Admission::Busy && rejected == "retry");
  queue.RequestStop();
  assert(queue.TrySubmit(9, std::move(rejected)).state == Admission::Stopping);
  {
    std::lock_guard lock(backendMutex);
    allow = true;
    backendChanged.notify_all();
  }
  worker.join();
  auto completion = queue.TryTakeCompletion();
  assert(completion && completion->ticket == first.ticket && completion->result.committed());
  assert(completion->outcome == Queue::Outcome::Executed);
  for (std::uint64_t revision = 2; revision <= Queue::MaxOutstanding; ++revision) {
    assert(queue.RunOne());
    completion = queue.TryTakeCompletion();
    assert(completion && completion->revision == revision);
    assert(completion->outcome == Queue::Outcome::CancelledBeforeStart);
  }
  assert(!queue.RunOne() && !queue.TryTakeCompletion());
  assert(writes == std::vector<std::string>{"first"});
  block = false;

  // Finished-but-unconsumed results reserve their slot: closing a UI cannot
  // silently discard an accepted write result or create an unbounded mailbox.
  Queue mailbox(destination);
  for (std::uint64_t revision = 1; revision <= Queue::MaxOutstanding; ++revision) {
    assert(mailbox.TrySubmit(revision, std::string("done")).state == Admission::Accepted);
    assert(mailbox.RunOne());
  }
  assert(mailbox.TrySubmit(9, std::string("retry")).state == Admission::Busy);
  assert(mailbox.TryTakeCompletion()->revision == 1);
  assert(mailbox.TrySubmit(9, std::string("retry")).state == Admission::Accepted);
  assert(mailbox.RunOne());
  for (std::uint64_t revision = 2; revision <= 9; ++revision)
    assert(mailbox.TryTakeCompletion()->revision == revision);

  Queue       byteBound(destination);
  std::string large(Queue::MaxPayload, 'x');
  auto        capacity = large.capacity();
  assert(byteBound.TrySubmit(1, std::move(large)).state == Admission::Accepted);
  std::string another(Queue::MaxPayload, 'y');
  if (another.capacity() <= Queue::MaxRetainedBytes - capacity) {
    assert(byteBound.TrySubmit(2, std::move(another)).state == Admission::Accepted);
    assert(byteBound.TrySubmit(3, std::string("overflow")).state == Admission::Busy);
  } else {
    assert(byteBound.TrySubmit(2, std::move(another)).state == Admission::Busy);
  }
  assert(byteBound.RunOne());
  assert(byteBound.TrySubmit(3, std::string("fits now")).state == Admission::Accepted);
  byteBound.RequestStop();
  while (byteBound.RunOne()) {}
  while (byteBound.TryTakeCompletion()) {}

  // An uncertain backend failure stops later snapshots from replacing files
  // whose recovery state needs inspection. Known failures remain retryable with
  // a fresh revision; admission high-water never silently regresses.
  Queue failures(destination);
  backendState = file_transaction::State::NotCommitted;
  assert(failures.TrySubmit(10, std::string("failed")).state == Admission::Accepted);
  assert(failures.RunOne());
  assert(failures.TryTakeCompletion()->result.state == backendState);
  assert(failures.TrySubmit(10, std::string("retry")).state == Admission::StaleRevision);
  assert(failures.TrySubmit(11, std::string("retry")).state == Admission::Accepted);
  assert(failures.TrySubmit(12, std::string("later")).state == Admission::Accepted);
  throwFailure = true;
  assert(failures.RunOne());
  assert(failures.TryTakeCompletion()->result.state == file_transaction::State::RecoveryRequired);
  assert(failures.TrySubmit(13, std::string("blocked")).state == Admission::Stopping);
  assert(failures.RunOne());
  assert(failures.TryTakeCompletion()->outcome == Queue::Outcome::CancelledBeforeStart);
  assert(!failures.RunOne());

  // Exercise slot reuse while producer, storage worker and completion consumer
  // run concurrently. Every accepted revision must finish exactly once in order.
  throwFailure = false;
  backendState = file_transaction::State::Committed;
  writes.clear();
  Queue                   concurrent(destination);
  constexpr std::uint64_t count = 500;
  std::atomic_bool        consumed{false};
  std::thread             storage([&] {
    while (!consumed.load())
      if (!concurrent.RunOne())
        std::this_thread::yield();
  });
  std::thread             consumer([&] {
    for (std::uint64_t revision = 1; revision <= count;) {
      auto done = concurrent.TryTakeCompletion();
      if (!done) {
        std::this_thread::yield();
        continue;
      }
      assert(done->revision == revision && done->ticket == revision && done->result.committed());
      ++revision;
    }
    consumed.store(true);
  });
  for (std::uint64_t revision = 1; revision <= count; ++revision) {
    std::string payload = std::to_string(revision);
    for (;;) {
      auto admitted = concurrent.TrySubmit(revision, std::move(payload));
      if (admitted.state == Admission::Accepted)
        break;
      assert(admitted.state == Admission::Busy && payload == std::to_string(revision));
      std::this_thread::yield();
    }
  }
  consumer.join();
  storage.join();
  assert(writes.size() == count);
  for (std::uint64_t revision = 1; revision <= count; ++revision)
    assert(writes[revision - 1] == std::to_string(revision));
  finished.set_value();
  watchdog.join();
  std::cout << "PASS bounded snapshot admission, ordering, slow storage, completion retention and stop\n";
}
