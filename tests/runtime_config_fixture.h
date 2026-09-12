#pragma once
#include <Windows.h>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <mutex>

namespace spdlog
{
template <class... Args> void warn(const char*, Args&&...) {}
} // namespace spdlog
namespace config_edit
{
enum class Outcome { Conflict, InvalidDocument, Unsupported };
// Controllable boundary; the fixture below includes the actual adapter bodies.
struct RuntimeConfigWriter {
  bool     work = false, stopped = false, finished = false, cancelled = false;
  bool     block_cancel = false;
  unsigned submissions  = 0;
  HANDLE   handle       = nullptr;
  bool     HasWork() const
  { return work; }
  void RequestCancelPending()
  { cancelled = true; }
  void Stop(bool cancel)
  {
    stopped   = true;
    cancelled = cancel;
    if (cancel && block_cancel) {
      std::puts("pending cancellation requested");
      std::fflush(stdout);
      Sleep(INFINITE); // Deadline must be independent of this stalled caller.
    }
  }
  bool PollStopped() const
  { return finished; }
  void* NativeHandle() const
  { return handle; }
  unsigned Submit(const char*)
  { return stopped ? 0 : ++submissions; }
};
} // namespace config_edit
