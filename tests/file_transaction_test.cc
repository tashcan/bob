#define MOD_FILE_TRANSACTION_TESTING
#include "file_transaction.cc"

#include <cassert>
#include <fstream>
#include <iostream>
#include <optional>
#include <thread>

namespace file_transaction
{
namespace
{
  thread_local std::optional<Stage> failureStage;
  thread_local bool                 partialReplace = false;
  bool                              InjectFailure(Stage stage)
  { return failureStage == stage; }
[[maybe_unused]] bool InjectPartialReplace()
  { return partialReplace; }
} // namespace
} // namespace file_transaction

std::string Read(const std::filesystem::path& path)
{
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

int main()
{
  namespace fs = std::filesystem;
  using namespace file_transaction;
  auto root =
      fs::temp_directory_path()
      / ("stfc-transaction-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  assert(fs::create_directory(root));
  const auto path   = root / "settings.toml";
  auto       result = Write(path, "value = 1\n", Mode::CreateOnly);
  assert(result.state == State::Committed && !result.error);
  assert(Read(path) == "value = 1\n");
  assert(Write(path, "value = 2\n", Mode::CreateOnly).state == State::Conflict);
  assert(Read(path) == "value = 1\n");

  for (auto stage : {Stage::Lock, Stage::StageFile, Stage::Write, Stage::Flush, Stage::Close, Stage::Commit}) {
    failureStage = stage;
    result       = Write(path, std::string(9000, 'x'), Mode::ReplaceSnapshot);
    failureStage.reset();
    assert(result.state == State::NotCommitted && result.error && result.stage == stage);
    assert(Read(path) == "value = 1\n");
    assert(result.recovery_directory.empty());
    for (const auto& entry : fs::directory_iterator(root))
      assert(entry.path().filename().string().rfind(".stfc-save-", 0) != 0);
  }

  result = Write(path, "value = 2\n", Mode::ReplaceSnapshot);
  assert(result.state == State::Committed && Read(path) == "value = 2\n");
  failureStage = Stage::DirectoryFlush;
  result       = Write(path, "value = 3\n", Mode::ReplaceSnapshot);
  failureStage.reset();
  assert(result.state == State::DurabilityUnverified && result.committed());
  assert(Read(path) == "value = 3\n");

  // Two create-only writers must never replace the winner, even when both began
  // from an absent destination. Busy is an explicit rejection, not accepted work.
  const auto  race = root / "race.toml";
  Result      a, b;
  std::thread first([&] { a = Write(race, "a", Mode::CreateOnly); });
  std::thread second([&] { b = Write(race, "b", Mode::CreateOnly); });
  first.join();
  second.join();
  assert(a.committed() != b.committed());
  assert(Read(race) == (a.committed() ? "a" : "b"));

  assert(!Write(root / "missing" / "bad.toml", "x", Mode::CreateOnly).committed());
  assert(!Write(path, std::string(4 * 1024 * 1024 + 1, 'x'), Mode::ReplaceSnapshot).committed());
  assert(Read(path) == "value = 3\n");

  // A hard link must not silently split into independent state files on replace.
  const auto hard = root / "hard.toml";
  fs::create_hard_link(path, hard);
  assert(!Write(hard, "bad", Mode::ReplaceSnapshot).committed());
  assert(Read(path) == "value = 3\n");
  fs::remove(hard);

#if _WIN32
  auto lockPath = path;
  lockPath += ".lock";
  auto held = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
  assert(held != INVALID_HANDLE_VALUE);
  assert(Write(path, "bad", Mode::ReplaceSnapshot).state == State::Busy);
  CloseHandle(held);
  auto targetHeld = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
  assert(targetHeld != INVALID_HANDLE_VALUE);
  result = Write(path, "bad", Mode::ReplaceSnapshot);
  assert(!result.committed() && Read(path) == "value = 3\n");
  CloseHandle(targetHeld);
  // Emulate the documented Windows1177 postcondition, not just a pre-commit
  // exception. The only old copy must survive cleanup in the recovery directory.
  partialReplace = true;
  result         = Write(path, "new candidate", Mode::ReplaceSnapshot);
  partialReplace = false;
  assert(result.state == State::RecoveryRequired && !result.recovery_directory.empty());
  assert(!fs::exists(path));
  assert(Read(result.recovery_directory / "previous") == "value = 3\n");
  assert(Read(result.recovery_directory / "new") == "new candidate");
  fs::rename(result.recovery_directory / "previous", path);
  fs::remove(result.recovery_directory / "new");
  fs::remove(result.recovery_directory);
#endif
  // Link creation can require developer mode/privilege on Windows; explicitly
  // report the skip instead of claiming the alias fixture ran.
  std::error_code linkError;
  const auto      alias = root / "alias.toml";
  fs::create_symlink(path, alias, linkError);
  if (!linkError) {
    assert(Write(alias, "value = 4\n", Mode::ReplaceSnapshot).committed());
    assert(fs::is_symlink(alias) && Read(path) == "value = 4\n");
  } else {
    std::cout << "SKIP symlink fixture: " << linkError.message() << '\n';
  }
  // Only this exclusively created test directory is recursively removed.
  assert(fs::canonical(root).parent_path() == fs::canonical(fs::temp_directory_path()));
  assert(fs::canonical(root).filename() == root.filename());
  fs::remove_all(root);
  std::cout << "PASS checked file transactions and pre-commit fault fixtures\n";
}
