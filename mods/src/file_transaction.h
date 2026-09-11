#pragma once

#include <filesystem>
#include <string_view>
#include <system_error>

namespace file_transaction
{
enum class Mode { CreateOnly, ReplaceSnapshot };
enum class State { NotCommitted, Conflict, Busy, Committed, DurabilityUnverified, RecoveryRequired };
enum class Stage { Validate, Resolve, Lock, StageFile, Write, Flush, Close, Commit, DirectoryFlush, Cleanup };

struct Result {
  State           state = State::NotCommitted;
  Stage           stage = Stage::Validate;
  std::error_code error;
  // Nonempty only when transaction-owned files were retained for inspection/recovery.
  std::filesystem::path recovery_directory;
  bool                  committed() const noexcept
  { return state == State::Committed || state == State::DurabilityUnverified; }
};

// Synchronous storage primitive for trusted, already-routed local destinations.
// Not a runtime/UI API, revision-aware editor, or sandbox for arbitrary paths.
// Preserves supported symlink targets; rejects hard-linked/non-regular destinations.
// Uses a cooperative sibling lock. Uncooperative external writers are not excluded.
[[nodiscard]] Result Write(const std::filesystem::path& destination, std::string_view bytes, Mode mode);
const char*          Name(State state) noexcept;
const char*          Name(Stage stage) noexcept;
} // namespace file_transaction
