#include "snapshot_save_worker.h"

#include <utility>

namespace persistence
{
SnapshotSaveWorker::SnapshotSaveWorker(const std::filesystem::path& trustedDestination,
                                     const std::atomic_bool* hostCancellation)
    : queue_(trustedDestination, hostCancellation)
    , worker_([this] { Run(); })
{
}

SnapshotSaveQueue::Submission SnapshotSaveWorker::TrySubmit(std::uint64_t revision, std::string&& bytes)
{
  if (stopping_.load())
    return {SnapshotSaveQueue::Admission::Stopping};
  std::unique_lock lock(admission_, std::try_to_lock);
  if (!lock.owns_lock())
    return {SnapshotSaveQueue::Admission::Busy};
  if (stopping_.load())
    return {SnapshotSaveQueue::Admission::Stopping};
  auto submitted = queue_.TrySubmit(revision, std::move(bytes));
  if (submitted.state == SnapshotSaveQueue::Admission::Accepted)
    Wake();
  return submitted;
}

std::optional<SnapshotSaveQueue::Completion> SnapshotSaveWorker::TryTakeCompletion()
{ return queue_.TryTakeCompletion(); }

void SnapshotSaveWorker::Wake() noexcept
{
  // Coalesced wakeups have no counter to overflow. The worker clears this BEFORE
  // draining, so a submission during/after draining cannot lose its notification.
  wake_.store(true);
  wake_.notify_one();
}

void SnapshotSaveWorker::RequestStop(StopMode mode) noexcept
{
  stopping_.store(true);
  // Drain closes admission; cancellation additionally closes queued selection.
  queue_.RequestStop(mode);
  Wake();
}

bool SnapshotSaveWorker::WorkEnded() const noexcept
{ return workEnded_.load(); }

void SnapshotSaveWorker::StopAndJoin(StopMode mode)
{
  RequestStop(mode);
  if (worker_.joinable())
    worker_.join();
}

void SnapshotSaveWorker::Run()
{
  for (;;) {
    wake_.wait(false);
    wake_.store(false);
    if (stopping_.load()) {
      // Synchronize with a producer that passed its stop check before stop was
      // requested. After this barrier no accepted ticket can appear behind the
      // final stop drain. Only the worker waits for this short lock.
      {
        std::lock_guard lock(admission_);
        // Preserve draining; concurrent cancellation remains sticky in the queue.
        queue_.RequestStop(StopMode::DrainAccepted);
      }
      while (queue_.RunOne()) {}
      workEnded_.store(true);
      return;
    }
    while (queue_.RunOne()) {}
  }
}
} // namespace persistence
