#include "patches/runtime_snapshot_host.h"

#if defined(_WIN32) && defined(_M_X64)
#include "patches/screen_update_hook.h"
#include "quit_drain_gate.h"
#include <Windows.h>
#include <cstring>
#include <il2cpp/il2cpp_helper.h>
#include <spud/detour.h>

namespace
{
using Host = persistence::SnapshotSaveHost;
// Fixed process-lifetime control block. The supervisor owns the service; its
// workers, paths and native handles are reclaimed on shutdown. Installed detours
// have process lifetime, so live module unloading is not supported.
Host* host = nullptr;
std::atomic_bool started{false};
persistence::QuitDrainGate gate;
std::atomic<DWORD> updateThread{0};
const MethodInfo* wantsMethod = nullptr;
void (*requestQuit)(int) = nullptr;
std::atomic_bool available{false}, attempted{false};
thread_local unsigned wantsDepth = 0;

// Exact build261 Windows x64 discovery: native extent411 bytes, SPUD overwrite24.
// Require runtime unwind extent AND the complete29-byte instruction fingerprint.
constexpr unsigned char wantsBytes[]{0x48, 0x89, 0x5c, 0x24, 0x08, 0x56, 0x57, 0x41, 0x56, 0x48,
                                     0x83, 0xec, 0x40, 0x80, 0x3d, 0xad, 0xd2, 0x8d, 0x01, 0x00,
                                     0x75, 0x29, 0x48, 0x8d, 0x0d, 0x9b, 0xdf, 0x63, 0x01};
bool MatchesQuitMethod()
{
  const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleW(L"GameAssembly.dll"));
  if (!base || !wantsMethod || reinterpret_cast<uintptr_t>(wantsMethod->methodPointer) != base + 0x43548c0)
    return false;
  DWORD64 imageBase = 0;
  const auto* extent = RtlLookupFunctionEntry(base + 0x43548c0, &imageBase, nullptr);
  return extent && imageBase == base && extent->BeginAddress == 0x43548c0 && extent->EndAddress == 0x4354a5b &&
         std::memcmp(wantsMethod->methodPointer, wantsBytes, sizeof(wantsBytes)) == 0;
}

bool WantsQuit(auto original)
{
  struct Depth { Depth() { ++wantsDepth; } ~Depth() { --wantsDepth; } } depth;
  const bool gameAllows = original();
  // Preserve the game's vote. Save failures are separate from whether native
  // code is still executing. No join, filesystem call, logging or timer here.
  const bool result = gate.Vote(gameAllows, started.load());
  if (gate.DrainRequested() && !gate.Stopped()) host->RequestStop();
  return result;
}

void UpdateHost()
{
  DWORD unset = 0;
  updateThread.compare_exchange_strong(unset, GetCurrentThreadId());
  if (updateThread.load() != GetCurrentThreadId() || wantsDepth != 0 || !started.load()) return;
  if (!gate.Stopped()) {
    if (!gate.DrainRequested() && host->Status() != Host::State::Unavailable) return;
    if (!host->PollStopped()) return;
    gate.ObserveStopped();
  }
  // Consume before calling into Unity: nested callbacks or a genuine subscriber
  // veto must not create an automatic quit loop. Failed saves still reach here.
  if (gate.TakeResumeRequest()) requestQuit(0);
}
} // namespace
#endif

void InstallRuntimeSnapshotHost()
{
#if defined(_WIN32) && defined(_M_X64)
  if (available) return;
  auto helper = il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine", "Application");
  if (!helper.isValidHelper()) return;
  wantsMethod = helper.GetMethodInfo("Internal_ApplicationWantsToQuit", 0);
  const auto* quit = helper.GetMethodInfo("Quit", 1);
  const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleW(L"GameAssembly.dll"));
  if (!MatchesQuitMethod() || !quit || reinterpret_cast<uintptr_t>(quit->methodPointer) != base + 0x4351c00) return;
  requestQuit = reinterpret_cast<void (*)(int)>(quit->methodPointer);
  // Existing single Update owner; no additional Update detour. Observing its
  // callback is a prerequisite for Start, not just trusting an install return.
  if (install_screen_manager_update_hook() && register_screen_manager_update_callback(UpdateHost)) available = true;
#endif
}

namespace runtime_snapshots
{
bool Start(std::vector<std::filesystem::path>&& paths)
{
#if defined(_WIN32) && defined(_M_X64)
  if (!available || updateThread.load() != GetCurrentThreadId() || attempted || wantsDepth != 0) return false;
  attempted = true;
  try {
    // Install only when a real consumer enrolls, and recheck for client/other-hook
    // drift immediately beforehand. A rejected seam cannot launch any worker.
    if (!MatchesQuitMethod() || !SPUD_STATIC_DETOUR(wantsMethod->methodPointer, WantsQuit)) return false;
    host = new Host;
    // Publish stable control ownership BEFORE native launch. An off-thread quit
    // during launch must be held too. Keep the control block even if launch fails;
    // a concurrent callback may already reference it, and Update reaps/resumes.
    started.store(true);
    return host->Start(std::move(paths));
  } catch (...) {
    return false;
  }
#else
  (void)paths;
  return false;
#endif
}

persistence::SnapshotSaveHost::State Status() noexcept
{
#if defined(_WIN32) && defined(_M_X64)
  if (started.load()) return host->Status();
  if (available && !attempted) return persistence::SnapshotSaveHost::State::Idle;
#endif
  return persistence::SnapshotSaveHost::State::Unavailable;
}
std::optional<persistence::SnapshotSaveService::Destination> TryGetDestination(std::size_t index)
{
#if defined(_WIN32) && defined(_M_X64)
  if (started.load()) return host->TryGetDestination(index);
#else
  (void)index;
#endif
  return std::nullopt;
}
persistence::SnapshotSaveQueue::Submission TrySubmit(persistence::SnapshotSaveService::Destination destination,
                                                   std::uint64_t revision, std::string&& bytes)
{
#if defined(_WIN32) && defined(_M_X64)
  if (started.load()) return host->TrySubmit(destination, revision, std::move(bytes));
#else
  (void)destination; (void)revision; (void)bytes;
#endif
  return {persistence::SnapshotSaveQueue::Admission::InvalidRequest};
}
std::optional<persistence::SnapshotSaveQueue::Completion> TryTakeCompletion(std::size_t index)
{
#if defined(_WIN32) && defined(_M_X64)
  if (started.load()) return host->TryTakeCompletion(index);
#else
  (void)index;
#endif
  return std::nullopt;
}
} // namespace runtime_snapshots
