#include "snapshot_save_host.h"

#if defined(_WIN32)
#include <Windows.h>
#include <process.h>
#endif

namespace persistence
{
#if defined(MOD_SNAPSHOT_HOST_TESTING) && defined(_WIN32)
namespace { void BeforeHostThreadReturn(); }
#endif
SnapshotSaveHost::~SnapshotSaveHost()
{
  // Never hide a blocking destructor or permit code to outlive its module.
  if (!PollStopped()) std::terminate();
}

bool SnapshotSaveHost::Start(std::vector<std::filesystem::path>&& paths)
{
  if (state_.load() != State::Idle) return false;
  state_.store(State::Unavailable);
  if (stop_.load() || paths.empty() || paths.size() > SnapshotSaveService::MaxDestinations ||
      paths.capacity() > SnapshotSaveService::MaxDestinations) return false;
  for (const auto& path : paths)
    if (path.empty() || path.native().capacity() > 32767) return false;
#if defined(_WIN32)
  HMODULE module = nullptr;
  // Ordinary scoped loader reference, not GET_MODULE_HANDLE_EX_FLAG_PIN. Keep
  // code loaded until native supervisor termination, then release on the owner.
  // Live unloading of the game's installed hooks is still unsupported.
  if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                         reinterpret_cast<LPCWSTR>(&SnapshotSaveHost::Entry), &module)) return false;
  module_ = module;
  count_ = paths.size();
  paths_ = std::move(paths);
  state_.store(State::Starting);
  thread_ = reinterpret_cast<void*>(_beginthreadex(nullptr, 0, Entry, this, 0, nullptr));
  if (!thread_) {
    paths = std::move(paths_);
    state_.store(State::Unavailable);
    FreeLibrary(module);
    module_ = nullptr;
    return false;
  }
  return true;
#else
  return false;
#endif
}

std::optional<SnapshotSaveService::Destination> SnapshotSaveHost::TryGetDestination(std::size_t index)
{
  std::unique_lock lock(access_, std::try_to_lock);
  if (!lock || stop_.load() || state_.load() != State::Ready || !service_ || index >= count_) return std::nullopt;
  return service_->GetDestination(index);
}

SnapshotSaveQueue::Submission SnapshotSaveHost::TrySubmit(SnapshotSaveService::Destination destination,
                                                       std::uint64_t revision, std::string&& bytes)
{
  if (stop_.load()) return {SnapshotSaveQueue::Admission::Stopping};
  std::unique_lock lock(access_, std::try_to_lock);
  if (!lock) return {SnapshotSaveQueue::Admission::Busy};
  if (stop_.load()) return {SnapshotSaveQueue::Admission::Stopping};
  if (state_.load() != State::Ready || !service_) return {SnapshotSaveQueue::Admission::InvalidRequest};
  return service_->TrySubmit(destination, revision, std::move(bytes));
}

std::optional<SnapshotSaveQueue::Completion> SnapshotSaveHost::TryTakeCompletion(std::size_t index)
{
  std::unique_lock lock(access_, std::try_to_lock);
  if (!lock || index >= count_) return std::nullopt;
  if (service_) return service_->TryTakeCompletion(service_->GetDestination(index));
  for (auto& result : retained_[index]) {
    if (!result) continue;
    auto taken = std::move(result);
    result.reset();
    return taken;
  }
  return std::nullopt;
}

void SnapshotSaveHost::RequestStop() noexcept
{
  stop_.store(true);
  stop_.notify_all();
}

bool SnapshotSaveHost::PollStopped() noexcept
{
#if defined(_WIN32)
  if (thread_) {
    if (WaitForSingleObject(static_cast<HANDLE>(thread_), 0) != WAIT_OBJECT_0) return false;
    CloseHandle(static_cast<HANDLE>(thread_));
    thread_ = nullptr;
    FreeLibrary(static_cast<HMODULE>(module_));
    module_ = nullptr;
  }
#endif
  return true;
}

#if defined(_WIN32)
unsigned __stdcall SnapshotSaveHost::Entry(void* context)
{
  static_cast<SnapshotSaveHost*>(context)->Run();
#if defined(MOD_SNAPSHOT_HOST_TESTING)
  BeforeHostThreadReturn();
#endif
  return 0; // _beginthreadex wrapper performs CRT thread cleanup before signaling.
}
#endif

void SnapshotSaveHost::Run() noexcept
{
  // This function and all service filesystem/thread lifecycle operations run on
  // the native supervisor. Remove producer access BEFORE draining/destruction.
  std::unique_ptr<SnapshotSaveService> service;
  try {
    service = std::make_unique<SnapshotSaveService>(paths_);
    paths_.clear();
    {
      std::lock_guard lock(access_);
      service_ = service.get();
      state_.store(State::Ready);
    }
    while (!stop_.load()) stop_.wait(false);
    {
      std::lock_guard lock(access_);
      state_.store(State::Stopping);
      service_ = nullptr;
    }
    service->StopAndJoin(StopMode::DrainAccepted);
    {
      std::lock_guard lock(access_);
      for (std::size_t i = 0; i < count_; ++i)
        for (auto& slot : retained_[i]) {
          slot = service->TryTakeCompletion(service->GetDestination(i));
          if (!slot) break;
        }
    }
    service.reset();
    state_.store(State::Stopped);
  } catch (...) {
    {
      std::lock_guard lock(access_);
      service_ = nullptr;
    }
    // Constructor rollback already joins partial workers. If a later operation
    // throws, still finish accepted work before allowing native thread exit.
    if (service) {
      try { service->StopAndJoin(StopMode::DrainAccepted); }
      catch (...) { std::terminate(); } // Never destroy possibly joinable workers.
      service.reset();
    }
    state_.store(State::Unavailable);
  }
}
} // namespace persistence
