#pragma once

#include "snapshot_save_service.h"
#include <vector>

namespace persistence
{
// One-shot native supervisor. Start/PollStopped/destruction belong to one owner
// thread, outside the loader lock. Start must return before consumers begin;
// other methods are safe for concurrent callers until destruction is quiesced.
// Windows only until an equivalent native-thread exit observation is validated.
// No implicit join, detach, filesystem work, or worker construction on the owner.
class SnapshotSaveHost final
{
public:
  enum class State { Idle, Starting, Ready, Stopping, Stopped, Unavailable };
  SnapshotSaveHost() = default;
  ~SnapshotSaveHost();
  SnapshotSaveHost(const SnapshotSaveHost&) = delete;
  SnapshotSaveHost& operator=(const SnapshotSaveHost&) = delete;

  // Trusted enrollment only. Takes ownership after successful launch; max four
  // paths, each bounded to 32767 native code units (including retained capacity).
  // An unsuccessful start is terminal. No retry/new session during shutdown.
  [[nodiscard]] bool Start(std::vector<std::filesystem::path>&& paths) noexcept;
  [[nodiscard]] State Status() const noexcept { return state_.load(); }
  [[nodiscard]] std::optional<SnapshotSaveService::Destination> TryGetDestination(std::size_t index);
  [[nodiscard]] SnapshotSaveQueue::Submission TrySubmit(SnapshotSaveService::Destination destination,
                                                       std::uint64_t revision, std::string&& bytes);
  [[nodiscard]] std::optional<SnapshotSaveQueue::Completion> TryTakeCompletion(std::size_t index);
  void RequestStop(StopMode mode = StopMode::DrainAccepted) noexcept;
#if defined(_WIN32)
  // Owner-thread snapshot of the native supervisor handle. Caller closes the
  // duplicate; observing it needs no owner Update callback or host lock.
  [[nodiscard]] bool DuplicateThread(void*& duplicate) const noexcept;
#endif
  // Zero-timeout native thread observation, NOT WorkEnded or a std::thread join.
  // Reclaims native handle/reference only after the supervisor actually exits.
  // False includes a failed native wait; never authorize exit on an uncertain wait.
  [[nodiscard]] bool PollStopped() noexcept;

private:
#if defined(MOD_SNAPSHOT_HOST_TESTING)
  friend struct SnapshotHostTestAccess;
#endif
  void Run() noexcept;
#if defined(_WIN32)
  static unsigned __stdcall Entry(void* context);
#endif
  std::atomic<State> state_{State::Idle};
  std::atomic_bool stop_{false};
  std::atomic_bool cancelQueued_{false};
  std::mutex access_;
  SnapshotSaveService* service_ = nullptr; // borrowed only while access_ is held
  std::vector<std::filesystem::path> paths_;
  std::size_t count_ = 0;
  std::array<std::array<std::optional<SnapshotSaveQueue::Completion>, SnapshotSaveQueue::MaxOutstanding>,
             SnapshotSaveService::MaxDestinations> retained_;
#if defined(_WIN32)
  void* thread_ = nullptr;
  void* module_ = nullptr;
#endif
};
} // namespace persistence
