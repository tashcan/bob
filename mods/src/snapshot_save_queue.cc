#include "snapshot_save_queue.h"

#include <limits>
#include <stdexcept>
#include <utility>

namespace persistence
{
namespace
{
#ifdef MOD_SNAPSHOT_QUEUE_ADMISSION_TESTING
  void BeforeSnapshotEnqueue();
#endif
  std::filesystem::path Resolve(const std::filesystem::path& path)
  {
    if (path.empty() || path.native().find(std::filesystem::path::value_type{}) != path.native().npos)
      throw std::invalid_argument("Invalid snapshot destination");
    return std::filesystem::weakly_canonical(std::filesystem::absolute(path));
  }
#ifdef MOD_SNAPSHOT_QUEUE_TESTING
  file_transaction::Result Execute(const std::filesystem::path&, std::string_view);
#else
  file_transaction::Result Execute(const std::filesystem::path& path, std::string_view bytes)
  { return file_transaction::Write(path, bytes, file_transaction::Mode::ReplaceSnapshot); }
#endif
} // namespace

SnapshotSaveQueue::SnapshotSaveQueue(const std::filesystem::path& trustedDestination)
    : destination_(Resolve(trustedDestination))
{
}

SnapshotSaveQueue::Submission SnapshotSaveQueue::TrySubmit(std::uint64_t revision, std::string&& bytes)
{
  if (stopping_.load())
    return {Admission::Stopping};
  if (!revision || bytes.size() > MaxPayload || bytes.capacity() > MaxRetainedBytes)
    return {Admission::InvalidRequest};
  std::unique_lock lock(mutex_, std::try_to_lock);
  if (!lock.owns_lock())
    return {Admission::Busy};
  if (stopping_.load())
    return {Admission::Stopping};
#ifdef MOD_SNAPSHOT_QUEUE_ADMISSION_TESTING
  // Pause after the final stop check to exercise admission overlapping shutdown.
  BeforeSnapshotEnqueue();
#endif
  if (revision <= lastRevision_)
    return {Admission::StaleRevision};
  // Never wrap ticket/revision identity. A new owner/session is required.
  if (nextTicket_ == std::numeric_limits<Ticket>::max())
    return {Admission::InvalidRequest};
  if (bytes.capacity() > MaxRetainedBytes - retainedBytes_)
    return {Admission::Busy};
  for (auto& slot : slots_) {
    if (slot.phase != Phase::Empty)
      continue;
    slot.retainedBytes = bytes.capacity();
    slot.bytes.swap(bytes);
    slot.completion.ticket   = nextTicket_++;
    slot.completion.revision = revision;
    slot.phase               = Phase::Queued;
    retainedBytes_ += slot.retainedBytes;
    lastRevision_ = revision;
    return {Admission::Accepted, slot.completion.ticket};
  }
  return {Admission::Busy};
}

std::optional<SnapshotSaveQueue::Completion> SnapshotSaveQueue::TryTakeCompletion()
{
  std::unique_lock lock(mutex_, std::try_to_lock);
  if (!lock.owns_lock())
    return std::nullopt;
  Slot* oldest = nullptr;
  for (auto& slot : slots_)
    if (slot.phase == Phase::Done && (!oldest || slot.completion.ticket < oldest->completion.ticket))
      oldest = &slot;
  if (!oldest)
    return std::nullopt;
  auto completion    = std::move(oldest->completion);
  oldest->completion = {};
  oldest->phase      = Phase::Empty;
  return completion;
}

void SnapshotSaveQueue::RequestStop(StopMode mode) noexcept
{
  if (mode == StopMode::CancelQueued)
    cancelQueued_.store(true);
  stopping_.store(true);
}

bool SnapshotSaveQueue::RunOne()
{
  if (running_.test_and_set())
    return false;
  struct Release {
    std::atomic_flag& flag;
    ~Release()
    { flag.clear(); }
  } release{running_};
  Slot*       selected = nullptr;
  bool        cancelled;
  std::string bytes;
  {
    std::lock_guard lock(mutex_);
    for (auto& slot : slots_)
      if (slot.phase == Phase::Queued && (!selected || slot.completion.ticket < selected->completion.ticket))
        selected = &slot;
    if (!selected)
      return false;
    // Once selected, this request is in-flight; a later stop cannot claim to
    // cancel an OS operation that may already have committed.
    cancelled       = cancelQueued_.load();
    selected->phase = Phase::Running;
    bytes.swap(selected->bytes);
  }
  file_transaction::Result result;
  if (!cancelled) {
    try {
      result = Execute(destination_, bytes);
    } catch (...) {
      // An unexpected backend exception cannot establish whether commit occurred.
      result.state = file_transaction::State::RecoveryRequired;
      result.error = std::make_error_code(std::errc::io_error);
    }
  }
  // Free potentially large buffers on the worker, outside the admission lock.
  std::string{}.swap(bytes);
  {
    std::lock_guard lock(mutex_);
    retainedBytes_ -= selected->retainedBytes;
    selected->retainedBytes      = 0;
    selected->completion.outcome = cancelled ? Outcome::CancelledBeforeStart : Outcome::Executed;
    selected->completion.result  = std::move(result);
    selected->phase              = Phase::Done;
    if (!cancelled && selected->completion.result.state == file_transaction::State::RecoveryRequired)
      RequestStop(StopMode::CancelQueued);
  }
  return true;
}
} // namespace persistence
