#include "file_transaction.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

#if _WIN32
#include <Windows.h>
#include <Aclapi.h>
#include <sddl.h>
#pragma comment(lib, "advapi32.lib")
#elif __APPLE__
#include <copyfile.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace file_transaction
{
const char* Name(State state) noexcept
{
  switch (state) {
    case State::NotCommitted:
      return "not-committed";
    case State::Conflict:
      return "conflict";
    case State::Busy:
      return "busy";
    case State::Committed:
      return "committed";
    case State::DurabilityUnverified:
      return "durability-unverified";
    case State::RecoveryRequired:
      return "recovery-required";
  }
  return "unknown";
}
const char* Name(Stage stage) noexcept
{
  switch (stage) {
    case Stage::Validate:
      return "validate";
    case Stage::Resolve:
      return "resolve";
    case Stage::Lock:
      return "lock";
    case Stage::StageFile:
      return "stage";
    case Stage::Write:
      return "write";
    case Stage::Flush:
      return "flush";
    case Stage::Close:
      return "close";
    case Stage::Commit:
      return "commit";
    case Stage::DirectoryFlush:
      return "directory-flush";
    case Stage::Cleanup:
      return "cleanup";
  }
  return "unknown";
}

namespace
{
  namespace fs                   = std::filesystem;
  constexpr std::size_t MaxBytes = 4 * 1024 * 1024;
  std::atomic_uint64_t  sequence{0};

#ifdef MOD_FILE_TRANSACTION_TESTING
  extern bool InjectFailure(Stage);
  extern bool InjectPartialReplace();
#else
  bool InjectFailure(Stage)
  { return false; }
#endif

  struct Failure {
    std::error_code error;
    State           state = State::NotCommitted;
  };
  void Fail(std::errc error, State state = State::NotCommitted)
  { throw Failure{std::make_error_code(error), state}; }
  void Checkpoint(Stage stage)
  {
    if (InjectFailure(stage))
      Fail(std::errc::io_error);
  }

#if _WIN32
  struct Handle {
    HANDLE value = INVALID_HANDLE_VALUE;
    ~Handle()
    {
      if (value != INVALID_HANDLE_VALUE)
        CloseHandle(value);
    }
    Handle()              = default;
    Handle(const Handle&) = delete;
    bool valid() const
    { return value != INVALID_HANDLE_VALUE; }
    void Close()
    {
      auto old = value;
      value    = INVALID_HANDLE_VALUE;
      if (old != INVALID_HANDLE_VALUE && !CloseHandle(old))
        throw Failure{{static_cast<int>(GetLastError()), std::system_category()}};
    }
  };
  void WinFail(State state = State::NotCommitted)
  { throw Failure{{static_cast<int>(GetLastError()), std::system_category()}, state}; }
  struct PrivateSecurity {
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    SECURITY_ATTRIBUTES  attributes{sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE};
    PrivateSecurity()
    {
      // Restrict staged data to its owner, administrators and SYSTEM.
      if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:P(A;;FA;;;SY)(A;;FA;;;BA)(A;;FA;;;OW)",
                                                                SDDL_REVISION_1, &descriptor, nullptr))
        WinFail();
      attributes.lpSecurityDescriptor = descriptor;
    }
    ~PrivateSecurity()
    {
      if (descriptor)
        LocalFree(descriptor);
    }
  };
  struct StagingDirectorySecurity {
    SECURITY_DESCRIPTOR descriptor{};
    std::vector<unsigned char> aclBytes;
    SECURITY_ATTRIBUTES attributes{sizeof(SECURITY_ATTRIBUTES), &descriptor, FALSE};
    StagingDirectorySecurity(HANDLE original, PrivateSecurity& privateSecurity)
    {
      // ReplaceFile merges inherited ACEs using the replacement's parent. A
      // private staging directory with no inheritable ACEs otherwise turns an
      // inherited-only target DACL into an empty DACL. Mirror only the target's
      // inherited file ACEs as inherit-only entries on our PRIVATE container.
      // They grant no access to the container; staged payload stays protected.
      PACL privateAcl = nullptr, originalAcl = nullptr;
      BOOL present = FALSE, defaulted = FALSE;
      if (!GetSecurityDescriptorDacl(privateSecurity.descriptor, &present, &privateAcl, &defaulted) || !privateAcl)
        WinFail();
      struct Descriptor {
        PSECURITY_DESCRIPTOR value = nullptr;
        ~Descriptor() { if (value) LocalFree(value); }
      } source;
      if (original != INVALID_HANDLE_VALUE) {
        const auto error = GetSecurityInfo(original, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr,
                                          &originalAcl, nullptr, &source.value);
        if (error != ERROR_SUCCESS) throw Failure{{static_cast<int>(error), std::system_category()}};
        if (!originalAcl) Fail(std::errc::operation_not_supported);
      }
      const size_t capacity = privateAcl->AclSize + (originalAcl ? originalAcl->AclSize : 0);
      if (capacity > 65535) Fail(std::errc::operation_not_supported);
      aclBytes.resize(capacity);
      auto* acl = reinterpret_cast<PACL>(aclBytes.data());
      if (!InitializeAcl(acl, static_cast<DWORD>(capacity), ACL_REVISION_DS)) WinFail();
      for (DWORD i = 0; i < privateAcl->AceCount; ++i) {
        void* ace = nullptr;
        if (!GetAce(privateAcl, i, &ace) ||
            !AddAce(acl, ACL_REVISION_DS, MAXDWORD, ace, static_cast<ACE_HEADER*>(ace)->AceSize)) WinFail();
      }
      if (originalAcl) for (DWORD i = 0; i < originalAcl->AceCount; ++i) {
        void* raw = nullptr;
        if (!GetAce(originalAcl, i, &raw)) WinFail();
        const auto* ace = static_cast<ACE_HEADER*>(raw);
        if (!(ace->AceFlags & INHERITED_ACE)) continue;
        // Do not guess semantics of propagating/object/conditional file ACEs.
        if (ace->AceFlags != INHERITED_ACE ||
            (ace->AceType != ACCESS_ALLOWED_ACE_TYPE && ace->AceType != ACCESS_DENIED_ACE_TYPE))
          Fail(std::errc::operation_not_supported);
        std::vector<unsigned char> copy(ace->AceSize);
        std::memcpy(copy.data(), raw, copy.size());
        reinterpret_cast<ACE_HEADER*>(copy.data())->AceFlags = INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
        if (!AddAce(acl, ACL_REVISION_DS, MAXDWORD, copy.data(), static_cast<DWORD>(copy.size()))) WinFail();
      }
      if (!InitializeSecurityDescriptor(&descriptor, SECURITY_DESCRIPTOR_REVISION) ||
          !SetSecurityDescriptorDacl(&descriptor, TRUE, acl, FALSE) ||
          !SetSecurityDescriptorControl(&descriptor, SE_DACL_PROTECTED, SE_DACL_PROTECTED)) WinFail();
    }
  };
  void Regular(HANDLE handle)
  {
    BY_HANDLE_FILE_INFORMATION info{};
    if (!GetFileInformationByHandle(handle, &info))
      WinFail();
    if ((info.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) || info.nNumberOfLinks != 1)
      Fail(std::errc::operation_not_supported);
  }
#elif __APPLE__
  struct Handle {
    int value = -1;
    ~Handle()
    {
      if (value >= 0)
        close(value);
    }
    Handle()              = default;
    Handle(const Handle&) = delete;
    bool valid() const
    { return value >= 0; }
    void Close()
    {
      auto old = value;
      value    = -1;
      if (old >= 0 && close(old) != 0)
        throw Failure{{errno, std::generic_category()}};
    }
  };
  void PosixFail(State state = State::NotCommitted)
  { throw Failure{{errno, std::generic_category()}, state}; }
  void Regular(int descriptor)
  {
    struct stat info{};
    if (fstat(descriptor, &info))
      PosixFail();
    if (!S_ISREG(info.st_mode) || info.st_nlink != 1)
      Fail(std::errc::operation_not_supported);
  }
#endif
} // namespace

Result Write(const fs::path& destination, std::string_view bytes, Mode mode)
{
  Result   result;
  fs::path transaction, payload, backup;
  bool     preserve = false;
  // Cleanup never sweeps a directory. Only these owned names can be removed.
  auto cleanup = [&] {
    if (transaction.empty())
      return;
    if (preserve) {
      result.recovery_directory = transaction;
      return;
    }
    std::error_code first, ec;
    for (const auto& path : {payload, backup, transaction}) {
      if (path.empty())
        continue;
      fs::remove(path, ec);
      if (ec && !first)
        first = ec;
    }
    if (first) {
      result.recovery_directory = transaction;
      if (!result.error) {
        result.error = first;
        result.stage = Stage::Cleanup;
      }
    }
  };
  try {
    if (bytes.size() > MaxBytes || destination.empty())
      Fail(std::errc::invalid_argument);
    const auto raw = destination.native();
    if (raw.find(typename fs::path::value_type{}) != decltype(raw)::npos)
      Fail(std::errc::invalid_argument);
    result.stage = Stage::Resolve;
    auto target  = fs::weakly_canonical(fs::absolute(destination));
#if _WIN32
    // Config paths may be user-selected, but device/UNC/alternate-stream writes
    // are outside this first local-filesystem transaction contract.
    const auto native = target.native();
    if (native.rfind(L"\\\\", 0) == 0 || native.find(L':', 2) != std::wstring::npos)
      Fail(std::errc::operation_not_supported);
#endif
    if (!fs::is_directory(target.parent_path()))
      Fail(std::errc::not_a_directory);
    result.stage = Stage::Lock;
    Checkpoint(result.stage);
#if _WIN32 || __APPLE__
    Handle lock, existing, staged;
    auto   lockPath = target;
    lockPath += ".lock";
#if _WIN32
    PrivateSecurity security;
    lock.value = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, &security.attributes, OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (!lock.valid()) {
      auto error = GetLastError();
      WinFail(error == ERROR_SHARING_VIOLATION || error == ERROR_LOCK_VIOLATION ? State::Busy : State::NotCommitted);
    }
    Regular(lock.value);
    existing.value = CreateFileW(target.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                 nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (!existing.valid() && GetLastError() != ERROR_FILE_NOT_FOUND)
      WinFail();
    if (existing.valid())
      Regular(existing.value);
#else
    // Validate descriptors without waiting for a FIFO peer or acquiring a tty.
    lock.value = open(lockPath.c_str(), O_CREAT | O_RDWR | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK | O_NOCTTY, 0600);
    if (!lock.valid())
      PosixFail();
    Regular(lock.value);
    if (flock(lock.value, LOCK_EX | LOCK_NB))
      PosixFail(errno == EWOULDBLOCK || errno == EAGAIN ? State::Busy : State::NotCommitted);
    existing.value = open(target.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK | O_NOCTTY);
    if (!existing.valid() && errno != ENOENT)
      PosixFail();
    if (existing.valid())
      Regular(existing.value);
#endif
    const bool existed = existing.valid();
    if (existed && mode == Mode::CreateOnly)
      Fail(std::errc::file_exists, State::Conflict);
    result.stage = Stage::StageFile;
    Checkpoint(result.stage);
#if _WIN32
    StagingDirectorySecurity directorySecurity(existing.value, security);
#endif
    for (int attempt = 0; attempt < 16; ++attempt) {
      auto candidate = target.parent_path() / (".stfc-save-" + std::to_string(++sequence));
#if _WIN32
      candidate += "-" + std::to_string(GetCurrentProcessId());
      if (CreateDirectoryW(candidate.c_str(), &directorySecurity.attributes)) {
        transaction = candidate;
        break;
      }
      if (GetLastError() != ERROR_ALREADY_EXISTS)
        WinFail();
#else
      candidate += "-" + std::to_string(getpid());
      if (mkdir(candidate.c_str(), 0700) == 0) {
        transaction = candidate;
        break;
      }
      if (errno != EEXIST)
        PosixFail();
#endif
    }
    if (transaction.empty())
      Fail(std::errc::file_exists);
    payload = transaction / "new";
    backup  = transaction / "previous";
#if _WIN32
    staged.value = CreateFileW(payload.c_str(), GENERIC_READ | GENERIC_WRITE, 0, &security.attributes, CREATE_NEW,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (!staged.valid())
      WinFail();
#else
    staged.value = open(payload.c_str(), O_CREAT | O_EXCL | O_RDWR | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (!staged.valid())
      PosixFail();
#endif
    result.stage = Stage::Write;
    // Deliberately stage a prefix before the injected failure to test cleanup of
    // genuinely partial output, not merely a rejected request.
    std::size_t offset = 0;
    while (offset < bytes.size()) {
      auto length = std::min<std::size_t>(4096, bytes.size() - offset);
#if _WIN32
      DWORD written = 0;
      if (!WriteFile(staged.value, bytes.data() + offset, static_cast<DWORD>(length), &written, nullptr))
        WinFail();
      if (!written)
        Fail(std::errc::io_error);
#else
      auto written = write(staged.value, bytes.data() + offset, length);
      if (written < 0 && errno == EINTR)
        continue;
      if (written < 0)
        PosixFail();
      if (!written)
        Fail(std::errc::io_error);
#endif
      offset += written;
      Checkpoint(result.stage);
    }
#if __APPLE__
    if (existed && fcopyfile(existing.value, staged.value, nullptr, COPYFILE_METADATA))
      PosixFail();
#endif
    result.stage = Stage::Flush;
    Checkpoint(result.stage);
#if _WIN32
    if (!FlushFileBuffers(staged.value))
      WinFail();
#else
    if (fsync(staged.value))
      PosixFail();
#endif
    result.stage = Stage::Close;
    staged.Close();
    Checkpoint(result.stage);
    existing.Close();
    result.stage = Stage::Commit;
    Checkpoint(result.stage);
#if _WIN32
    if (existed) {
      preserve = true;
      BOOL replaced;
#ifdef MOD_FILE_TRANSACTION_TESTING
      if (InjectPartialReplace()) {
        if (!MoveFileExW(target.c_str(), backup.c_str(), 0))
          WinFail(State::RecoveryRequired);
        SetLastError(ERROR_UNABLE_TO_MOVE_REPLACEMENT_2);
        replaced = FALSE;
      } else
#endif
        replaced = ReplaceFileW(target.c_str(), payload.c_str(), backup.c_str(), 0, nullptr, nullptr);
      if (!replaced) {
        const auto      error = GetLastError();
        std::error_code inspectionError;
        const bool      backupExists = fs::exists(backup, inspectionError);
        preserve                     = error == ERROR_UNABLE_TO_MOVE_REPLACEMENT_2 || backupExists || inspectionError;
        throw Failure{{static_cast<int>(error), std::system_category()},
                      preserve ? State::RecoveryRequired : State::NotCommitted};
      }
      preserve = false;
    } else if (!MoveFileExW(payload.c_str(), target.c_str(), MOVEFILE_WRITE_THROUGH)) {
      const auto error = GetLastError();
      WinFail(error == ERROR_ALREADY_EXISTS || error == ERROR_FILE_EXISTS ? State::Conflict : State::NotCommitted);
    }
#else
    if (renamex_np(payload.c_str(), target.c_str(), existed ? 0 : RENAME_EXCL))
      PosixFail(errno == EEXIST ? State::Conflict : State::NotCommitted);
#endif
    result.state = State::Committed;
    result.stage = Stage::DirectoryFlush;
#if __APPLE__
    Handle parent;
    parent.value = open(target.parent_path().c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (!parent.valid() || fsync(parent.value)) {
      result.state = State::DurabilityUnverified;
      result.error = {errno, std::generic_category()};
    }
#endif
    if (InjectFailure(Stage::DirectoryFlush)) {
      result.state = State::DurabilityUnverified;
      result.error = std::make_error_code(std::errc::io_error);
    }
#else
    Fail(std::errc::operation_not_supported);
#endif
  } catch (const Failure& failure) {
    result.state = failure.state;
    result.error = failure.error;
  } catch (const fs::filesystem_error& failure) {
    result.error = failure.code();
  } catch (...) {
    result.error = std::make_error_code(std::errc::io_error);
  }
  cleanup();
  return result;
}
} // namespace file_transaction
