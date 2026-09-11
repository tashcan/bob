#pragma once
#include <atomic>

namespace persistence
{
// Receives the real game's vote; never turns a veto into permission to quit.
// The owner must ObserveStopped only after native supervisor termination.
class QuitDrainGate final
{
public:
  bool Vote(bool gameAllows, bool active) noexcept
  {
    if (!active) return gameAllows;
    if (!gameAllows) {
      resume_.store(false);
      return false;
    }
    if (stopped_.load()) return true;
    requested_.store(true);
    resume_.store(true);
    return false;
  }
  bool DrainRequested() const noexcept { return requested_.load(); }
  void ObserveStopped() noexcept { stopped_.store(true); }
  bool Stopped() const noexcept { return stopped_.load(); }
  bool TakeResumeRequest() noexcept
  {
    // Consume before reentering Unity. A subsequent genuine veto cannot cause
    // repeated automatic quit requests once the supervisor is gone.
    return stopped_.load() && resume_.exchange(false);
  }
private:
  std::atomic_bool requested_{false}, resume_{false}, stopped_{false};
};
}
