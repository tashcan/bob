#pragma once

#include "snapshot_save_host.h"

// Install after il2cpp_init, outside DllMain. No worker is launched here.
void InstallRuntimeSnapshotHost();

namespace runtime_snapshots
{
// Internal trusted registration, called on the observed game Update thread after
// installation. False leaves input owned by the caller. One attempt per process.
// Unsupported clients/platforms cannot start a worker through this adapter.
[[nodiscard]] bool Start(std::vector<std::filesystem::path>&& trustedPaths);
[[nodiscard]] persistence::SnapshotSaveHost::State Status() noexcept;
[[nodiscard]] std::optional<persistence::SnapshotSaveService::Destination> TryGetDestination(std::size_t index);
[[nodiscard]] persistence::SnapshotSaveQueue::Submission TrySubmit(
    persistence::SnapshotSaveService::Destination destination, std::uint64_t revision, std::string&& bytes);
[[nodiscard]] std::optional<persistence::SnapshotSaveQueue::Completion> TryTakeCompletion(std::size_t index);
} // namespace runtime_snapshots
