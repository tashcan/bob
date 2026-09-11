#define MOD_FILE_TRANSACTION_TESTING
#include "file_transaction.cc"

#include <cassert>
#include <fstream>
#include <iostream>
#include <optional>
#include <thread>

#if __APPLE__
#include <signal.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#endif

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

#if _WIN32
std::pair<bool, std::wstring> Dacl(const std::filesystem::path& path)
{
  // Use the same ACL API family as the writer: querying a legacy descriptor can
  // canonicalize its auto-inheritance flags before the operation being tested.
  PSECURITY_DESCRIPTOR descriptor = nullptr;
  auto name = path.native();
  assert(GetNamedSecurityInfoW(name.data(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr,
                               nullptr, nullptr, &descriptor) == ERROR_SUCCESS);
  LPWSTR text = nullptr;
  assert(ConvertSecurityDescriptorToStringSecurityDescriptorW(descriptor, SDDL_REVISION_1,
                                                              DACL_SECURITY_INFORMATION, &text, nullptr));
  std::wstring result(text);
  LocalFree(text);
  SECURITY_DESCRIPTOR_CONTROL control{};
  DWORD                       revision = 0;
  assert(GetSecurityDescriptorControl(descriptor, &control, &revision));
  LocalFree(descriptor);
  const auto entries = result.find(L'(');
  assert(entries != std::wstring::npos);
  // ReplaceFile may add the informational AUTO_INHERITED marker. Compare the
  // actual ordered ACEs and protected-inheritance bit, not that history marker.
  return {(control & SE_DACL_PROTECTED) != 0, result.substr(entries)};
}
#endif

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

  // Replace preserves deliberately restrictive destination metadata, rather
  // than widening it to the staging file's defaults.
  const auto metadata = root / "metadata.toml";
  assert(Write(metadata, "old", Mode::CreateOnly).committed());
#if _WIN32
  // Ordinary externally created files can have only inherited permissions.
  // Replacing through a private subdirectory must not erase those entries.
  PSECURITY_DESCRIPTOR inheritedSecurity = nullptr;
  assert(ConvertStringSecurityDescriptorToSecurityDescriptorW(
      L"D:PAI(D;OICI;FW;;;BG)(A;OICI;FA;;;OW)(A;OICI;FR;;;BU)", SDDL_REVISION_1,
      &inheritedSecurity, nullptr));
  SECURITY_ATTRIBUTES inheritedAttributes{sizeof(SECURITY_ATTRIBUTES), inheritedSecurity, FALSE};
  const auto inheritedRoot = root / "inherited";
  assert(CreateDirectoryW(inheritedRoot.c_str(), &inheritedAttributes));
  LocalFree(inheritedSecurity);
  const auto inheritedFile = inheritedRoot / "existing.toml";
  { std::ofstream out(inheritedFile); out << "old"; assert(out.good()); }
  const auto inheritedDacl = Dacl(inheritedFile);
  assert(Write(inheritedFile, "first", Mode::ReplaceSnapshot).committed());
  const auto firstContent = Read(inheritedFile);
  const auto firstDacl = Dacl(inheritedFile);
  if (firstContent != "first" || firstDacl != inheritedDacl) {
    std::cerr << "Inherited replacement content matches: " << (firstContent == "first") << '\n';
    std::wcerr << L"Before protected=" << inheritedDacl.first << L" " << inheritedDacl.second << L'\n'
               << L"After protected=" << firstDacl.first << L" " << firstDacl.second << std::endl;
  }
  assert(firstContent == "first");
  assert(firstDacl == inheritedDacl);
  assert(Write(inheritedFile, "second", Mode::ReplaceSnapshot).committed());
  assert(Read(inheritedFile) == "second" && Dacl(inheritedFile) == inheritedDacl);
  PSECURITY_DESCRIPTOR restricted = nullptr;
  assert(
      ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:P(A;;FA;;;OW)", SDDL_REVISION_1, &restricted, nullptr));
  assert(
      SetFileSecurityW(metadata.c_str(), DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION, restricted));
  LocalFree(restricted);
  const auto beforeDacl = Dacl(metadata);
  assert(Write(metadata, "new", Mode::ReplaceSnapshot).committed());
  assert(Dacl(metadata) == beforeDacl && Read(metadata) == "new");
#elif __APPLE__
  assert(chmod(metadata.c_str(), 0640) == 0);
  constexpr const char* attribute = "com.stfc-mod.fixture";
  assert(setxattr(metadata.c_str(), attribute, "kept", 4, 0, 0) == 0);
  struct stat beforeMetadata{}, afterMetadata{};
  assert(stat(metadata.c_str(), &beforeMetadata) == 0);
  assert(Write(metadata, "new", Mode::ReplaceSnapshot).committed());
  assert(stat(metadata.c_str(), &afterMetadata) == 0);
  assert(beforeMetadata.st_mode == afterMetadata.st_mode && beforeMetadata.st_uid == afterMetadata.st_uid
         && beforeMetadata.st_gid == afterMetadata.st_gid);
  char attributeValue[4]{};
  assert(getxattr(metadata.c_str(), attribute, attributeValue, sizeof(attributeValue), 0, 0) == 4);
  assert(std::string_view(attributeValue, 4) == "kept" && Read(metadata) == "new");
#endif

#if __APPLE__
  // A nonregular destination (or lock) must be rejected before waiting for a
  // FIFO peer. Bound the child so a regression fails instead of hanging CI.
  for (bool lockFixture : {false, true}) {
    const auto fifoTarget = root / (lockFixture ? "fifo-lock.toml" : "fifo.toml");
    auto       fifo       = fifoTarget;
    if (lockFixture)
      fifo += ".lock";
    assert(mkfifo(fifo.c_str(), 0600) == 0);
    const auto child = fork();
    assert(child >= 0);
    if (child == 0) {
      const auto rejected = Write(fifoTarget, "bad", Mode::ReplaceSnapshot);
      _exit(rejected.state == State::NotCommitted
                    && rejected.error == std::make_error_code(std::errc::operation_not_supported)
                ? 0
                : 1);
    }
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    int        status   = 0;
    pid_t      waited;
    do {
      waited = waitpid(child, &status, WNOHANG);
      if (waited == child || (waited < 0 && errno != EINTR))
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (std::chrono::steady_clock::now() < deadline);
    if (waited != child) {
      kill(child, SIGKILL);
      while (waitpid(child, &status, 0) < 0 && errno == EINTR) {}
      assert(false && "FIFO rejection exceeded deadline");
    }
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    assert(fs::is_fifo(fifo));
    fs::remove(fifo);
  }
#endif

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
    auto canonicalLock = path;
    canonicalLock += ".lock";
#if _WIN32
    auto aliasLock =
        CreateFileW(canonicalLock.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    assert(aliasLock != INVALID_HANDLE_VALUE);
#elif __APPLE__
    auto aliasLock = open(canonicalLock.c_str(), O_RDWR | O_CLOEXEC);
    assert(aliasLock >= 0 && flock(aliasLock, LOCK_EX | LOCK_NB) == 0);
#endif
    assert(Write(alias, "must not write", Mode::ReplaceSnapshot).state == State::Busy);
    assert(Read(path) == "value = 3\n");
#if _WIN32
    CloseHandle(aliasLock);
#elif __APPLE__
    close(aliasLock);
#endif
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
