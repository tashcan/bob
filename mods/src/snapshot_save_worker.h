#pragma once

#include "snapshot_save_queue.h"

#include <thread>

namespace persistence
{
// Internal lifecycle owner, not a game singleton or settings-page member.
// Construct outside the loader lock. The sole lifecycle owner MUST call
// StopAndJoin outside gameplay/loader callbacks before destruction or unload.
// Destruction without joining terminates, matching std::thread; it never hides
// an unbounded join or detaches a worker that could outlive the loaded module.
class SnapshotSaveWorker final
{
public:
  // hostCancellation, when supplied, must outlive the worker and its joined thread.
  explicit SnapshotSaveWorker(const std::filesystem::path& trustedDestination,
                              const std::atomic_bool* hostCancellation = nullptr);
  ~SnapshotSaveWorker()                                    = default;
  SnapshotSaveWorker(const SnapshotSaveWorker&)            = delete;
  SnapshotSaveWorker& operator=(const SnapshotSaveWorker&) = delete;

  [[nodiscard]] SnapshotSaveQueue::Submission                TrySubmit(std::uint64_t revision, std::string&& bytes);
  [[nodiscard]] std::optional<SnapshotSaveQueue::Completion> TryTakeCompletion();
  void RequestStop(StopMode mode = StopMode::CancelQueued) noexcept;
  // Informational: true means queue execution has ended, NOT that the native
  // thread has exited or that destroying this object is safe. Join is mandatory.
  [[nodiscard]] bool WorkEnded() const noexcept;
  // Sole lifecycle owner only; may wait indefinitely for an in-flight OS call.
  // Idempotent after join. Never call concurrently or from the worker itself.
  // Require mode explicitly so a join cannot accidentally cancel an earlier drain.
  void StopAndJoin(StopMode mode);

private:
  void              Wake() noexcept;
  void              Run();
  SnapshotSaveQueue queue_;
  std::mutex        admission_;
  std::atomic_bool  stopping_{false};
  std::atomic_bool  wake_{false};
  std::atomic_bool  workEnded_{false};
  // Last member: everything used by Run is initialized before thread launch.
  std::thread worker_;
};
} // namespace persistence
