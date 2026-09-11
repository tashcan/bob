#pragma once
#include "snapshot_save_worker.h"
#include <array>
#include <memory>
#include <span>

namespace persistence
{
// Internal process-wide owner of registered generated-snapshot destinations.
// Construct off gameplay/loader callbacks. Quiesce callers and explicitly join
// before destruction. No singleton destructor or implicit detach/join.
class SnapshotSaveService final
{
public:
  static constexpr std::size_t MaxDestinations = 4;
  class Destination {
  public:
    Destination() = default;
  private:
    friend class SnapshotSaveService;
    std::uint64_t session_ = 0;
    std::size_t index_ = 0;
  };
  explicit SnapshotSaveService(std::span<const std::filesystem::path> trustedPaths);
  ~SnapshotSaveService() = default;
  SnapshotSaveService(const SnapshotSaveService&) = delete;
  SnapshotSaveService& operator=(const SnapshotSaveService&) = delete;
  [[nodiscard]] Destination GetDestination(std::size_t index) const;
  [[nodiscard]] SnapshotSaveQueue::Submission TrySubmit(Destination destination, std::uint64_t revision,
                                                       std::string&& bytes);
  [[nodiscard]] std::optional<SnapshotSaveQueue::Completion> TryTakeCompletion(Destination destination);
  void RequestStop(StopMode mode) noexcept;
  void StopAndJoin(StopMode mode);
private:
  // Declared first, destroyed last: ownership is never released before workers.
  struct Lease {
    Lease();
    ~Lease();
  } lease_;
  const std::uint64_t session_;
  const std::size_t count_;
  std::atomic_bool stopping_{false};
  std::array<std::unique_ptr<SnapshotSaveWorker>, MaxDestinations> workers_;
  bool Valid(Destination destination) const noexcept;
};
}
