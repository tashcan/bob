#pragma once

#include "file_transaction.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>

namespace persistence
{
enum class StopMode { DrainAccepted, CancelQueued };
// Internal scheduling core for ONE trusted generated-output destination. This
// is not a user-TOML editor or a thread owner. Construct outside gameplay; the
// future host must own/join its worker before destroying this object or unloading.
class SnapshotSaveQueue final
{
public:
  static constexpr std::size_t MaxOutstanding   = 8;
  static constexpr std::size_t MaxPayload       = 4 * 1024 * 1024;
  static constexpr std::size_t MaxRetainedBytes = 8 * 1024 * 1024;
  using Ticket                                  = std::uint64_t;
  enum class Admission { Accepted, Busy, InvalidRequest, StaleRevision, Stopping };
  struct Submission {
    Admission state;
    Ticket    ticket = 0;
  };
  enum class Outcome { Executed, CancelledBeforeStart };
  struct Completion {
    Ticket                   ticket   = 0;
    std::uint64_t            revision = 0;
    Outcome                  outcome  = Outcome::CancelledBeforeStart;
    file_transaction::Result result;
  };

  // Optional host cancellation must outlive this queue and only transition to true.
  explicit SnapshotSaveQueue(const std::filesystem::path& trustedDestination,
                             const std::atomic_bool* hostCancellation = nullptr);
  SnapshotSaveQueue(const SnapshotSaveQueue&)            = delete;
  SnapshotSaveQueue& operator=(const SnapshotSaveQueue&) = delete;

  // No serialization, filesystem work or mutex wait. Accepted alone moves bytes;
  // rejection leaves the caller's buffer intact. Capacity counts against limits.
  // Preparing a snapshot is the adapter's job and must also stay off hot paths.
  [[nodiscard]] Submission TrySubmit(std::uint64_t revision, std::string&& bytes);
  // Poll on the owning consumer thread. No callbacks or borrowed UI objects.
  // Empty means no completion available OR transient lock contention; retry later.
  [[nodiscard]] std::optional<Completion> TryTakeCompletion();
  // Cancellation is sticky and wins over concurrent drain requests.
  void RequestStop(StopMode mode = StopMode::CancelQueued) noexcept;

  // Worker-only, synchronous; runs at most one transaction (or cancellation).
  // Concurrent invocations do not overlap disk writes. False means no work or
  // another worker is active. Stopping still requires pumping pending outcomes.
  // This does not create a thread, wait for disk at teardown, or detach anything.
  [[nodiscard]] bool RunOne();

private:
  enum class Phase { Empty, Queued, Running, Done };
  struct Slot {
    Phase       phase = Phase::Empty;
    std::string bytes;
    std::size_t retainedBytes = 0;
    Completion  completion;
  };
  const std::filesystem::path      destination_;
  const std::atomic_bool* const hostCancellation_;
  std::array<Slot, MaxOutstanding> slots_;
  std::mutex                       mutex_;
  std::atomic_bool                 stopping_{false};
  std::atomic_bool                 cancelQueued_{false};
  std::atomic_flag                 running_       = ATOMIC_FLAG_INIT;
  std::size_t                      retainedBytes_ = 0;
  std::uint64_t                    lastRevision_  = 0;
  Ticket                           nextTicket_    = 1;
};
} // namespace persistence
