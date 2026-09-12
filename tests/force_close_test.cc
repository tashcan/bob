#define MOD_SNAPSHOT_QUEUE_TESTING
#define MOD_SNAPSHOT_HOST_TESTING
#include "snapshot_save_queue.cc"
#include "snapshot_save_worker.cc"
#include "snapshot_save_service.cc"
#include "snapshot_save_host.cc"
#include "force_close.cc"
#include <cassert>
#include <iostream>

#if defined(_WIN32)
struct Capture { volatile LONG calls; ULONGLONG began; };
Capture* capture;
int mode;
std::atomic_bool releaseActive{false}, accessHeld{false};
namespace persistence
{
struct SnapshotHostTestAccess {
  static std::mutex& Access(SnapshotSaveHost& host) { return host.access_; }
};
namespace
{
void BeforeHostThreadReturn() {}
file_transaction::Result Execute(const std::filesystem::path&, std::string_view)
{
  InterlockedIncrement(&capture->calls);
  if (mode == 1) Sleep(INFINITE); // Storage never returns; deadline must still win.
  if (mode == 4) while (!releaseActive.load()) Sleep(1);
  Sleep(80);
  file_transaction::Result result;
  result.state = file_transaction::State::Committed;
  return result;
}
}
}

int wmain(int argc, wchar_t** argv)
{
  const bool child = argc == 4;
  const auto owner = child ? std::stoul(argv[3]) : GetCurrentProcessId();
  const auto name = L"Local\\STFCForceCloseTest_" + std::to_wstring(owner);
  const auto mapping = child ? OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name.c_str()) :
      CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(Capture), name.c_str());
  assert(mapping);
  capture = static_cast<Capture*>(MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Capture)));
  assert(capture);
  if (child) {
    mode = std::stoi(argv[2]);
    persistence::SnapshotSaveHost* host = nullptr;
    if (mode) {
      host = new persistence::SnapshotSaveHost;
      std::vector<std::filesystem::path> paths;
      paths.push_back(std::filesystem::temp_directory_path() / (L"force-close-" + std::to_wstring(owner)));
      assert(host->Start(std::move(paths)));
      std::optional<persistence::SnapshotSaveService::Destination> destination;
      while (!(destination = host->TryGetDestination(0))) Sleep(1);
      using Admission = persistence::SnapshotSaveQueue::Admission;
      while (host->TrySubmit(*destination, 1, "first").state != Admission::Accepted) Sleep(1);
      while (!InterlockedCompareExchange(&capture->calls, 0, 0)) Sleep(1);
      while (host->TrySubmit(*destination, 2, "queued").state != Admission::Accepted) Sleep(1);
      if (mode == 3 || mode == 4) host->RequestStop();
      if (mode == 4) {
        while (host->Status() != persistence::SnapshotSaveHost::State::Stopping) Sleep(1);
        // Hold producer access across cancellation and active-write completion.
        // Neither the deadline nor queued cancellation may depend on this lock.
        new std::thread([host] {
          std::lock_guard lock(persistence::SnapshotHostTestAccess::Access(*host));
          accessHeld.store(true);
          while (!releaseActive.load()) Sleep(1);
          Sleep(200);
        });
        while (!accessHeld.load()) Sleep(1);
      }
    }
    capture->began = GetTickCount64();
    persistence::ForceClose(host);
    releaseActive.store(true);
    Sleep(INFINITE); // Simulate no further Unity/owner updates.
    return 99;
  }
  wchar_t executable[32768]{};
  assert(GetModuleFileNameW(nullptr, executable, 32768));
  for (int scenario = 0; scenario != 5; ++scenario) {
    capture->calls = 0;
    capture->began = 0;
    auto command = L"\"" + std::wstring(executable) + L"\" --child " + std::to_wstring(scenario) +
                   L" " + std::to_wstring(owner);
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    assert(CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                          nullptr, nullptr, &startup, &process));
    const auto waited = WaitForSingleObject(process.hProcess, 5000);
    if (waited != WAIT_OBJECT_0) TerminateProcess(process.hProcess, 98);
    assert(waited == WAIT_OBJECT_0);
    DWORD code = 0;
    assert(GetExitCodeProcess(process.hProcess, &code) && code == 1);
    const auto elapsed = GetTickCount64() - capture->began;
    assert(capture->began && elapsed < 2500); // Scheduling tolerance, not a 2.5s policy.
    if (scenario == 1) assert(elapsed >= 450);
    assert(capture->calls == (scenario ? 1 : 0)); // Queued second write never executes.
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    std::cout << "force-close scenario " << scenario << ": " << elapsed << "ms\n";
  }
  UnmapViewOfFile(capture);
  CloseHandle(mapping);
}
#else
namespace persistence { namespace {
file_transaction::Result Execute(const std::filesystem::path&, std::string_view) { return {}; }
} }
int main() { std::cout << "Windows-only force-close fixture skipped\n"; }
#endif
