#include "force_close.h"
#if defined(_WIN32)
#include <Windows.h>
#include <process.h>

namespace persistence
{
namespace
{
constexpr ULONGLONG GraceMs = 500;
struct Request {
  SnapshotSaveHost* host;
  HANDLE supervisor;
  ULONGLONG began;
};
Request request{}; // Single process-exit request; no heap or static destructor.
std::atomic_bool requested{false};
unsigned __stdcall Finish(void*)
{
  // This thread never joins a writer, takes a blocking host lock, calls Unity,
  // or reaps the supervisor. Its own deadline remains independent of storage.
  for (;;) {
    const auto elapsed = GetTickCount64() - request.began;
    if (elapsed >= GraceMs) break;
    request.host->RequestStop(StopMode::CancelQueued);
    const auto result = WaitForSingleObject(request.supervisor, 1);
    if (result != WAIT_TIMEOUT) break; // Native completion or failed observation.
  }
  TerminateProcess(GetCurrentProcess(), 1);
  return 0;
}
}

void ForceClose(SnapshotSaveHost* host) noexcept
{
  if (requested.exchange(true)) return;
  request.began = GetTickCount64();
  request.host = host;
  void* duplicate = nullptr;
  if (!host || !host->DuplicateThread(duplicate) || !duplicate) {
    TerminateProcess(GetCurrentProcess(), 1);
    return;
  }
  request.supervisor = static_cast<HANDLE>(duplicate);
  host->RequestStop(StopMode::CancelQueued);
  const auto thread = _beginthreadex(nullptr, 0, Finish, nullptr, 0, nullptr);
  if (!thread) {
    CloseHandle(request.supervisor);
    TerminateProcess(GetCurrentProcess(), 1);
    return;
  }
  // The process is exiting; no detach/join destructor or callback owns this
  // thread. Its handles and the fixed request live only until termination.
  CloseHandle(reinterpret_cast<HANDLE>(thread));
}
}
#endif
