#pragma once
#include <atomic>

namespace persistence
{
#if defined(MOD_QUIT_DRAIN_GATE_TESTING)
namespace { void BeforeQuitVoteCompareExchange(); }
#endif
// Registration, real game votes and resume consumption share one atomic state.
// A permitted quit before registration permanently closes registration. After
// activation, only native supervisor termination can authorize normal exit.
class QuitDrainGate final
{
  enum class Phase { Dormant, Active, DrainResume, DrainVeto, StoppedResume, StoppedVeto, Done };
public:
  bool TryActivate() noexcept
  {
    auto expected = Phase::Dormant;
    return phase_.compare_exchange_strong(expected, Phase::Active);
  }
  bool Vote(bool gameAllows) noexcept
  {
    auto before = phase_.load();
    for (;;) {
      auto after = before;
      bool allow = false;
      switch (before) {
        case Phase::Dormant:
          if (gameAllows) after = Phase::Done;
          allow = gameAllows;
          break;
        case Phase::Active:
          if (gameAllows) after = Phase::DrainResume;
          break;
        case Phase::DrainResume:
        case Phase::DrainVeto:
          after = gameAllows ? Phase::DrainResume : Phase::DrainVeto;
          break;
        case Phase::StoppedResume:
        case Phase::StoppedVeto:
          after = gameAllows ? Phase::Done : Phase::StoppedVeto;
          allow = gameAllows;
          break;
        case Phase::Done:
          return gameAllows; // Terminal: no stale vote can re-arm resumption.
      }
#if defined(MOD_QUIT_DRAIN_GATE_TESTING)
      BeforeQuitVoteCompareExchange();
#endif
      if (phase_.compare_exchange_weak(before, after)) return allow;
    }
  }
  bool DrainRequested() const noexcept
  {
    const auto phase = phase_.load();
    return phase == Phase::DrainResume || phase == Phase::DrainVeto;
  }
  // Owner only, AFTER observing native supervisor termination (or failed launch).
  void ObserveStopped() noexcept
  {
    auto before = phase_.load();
    for (;;) {
      Phase after;
      if (before == Phase::DrainResume) after = Phase::StoppedResume;
      else if (before == Phase::Active || before == Phase::DrainVeto) after = Phase::StoppedVeto;
      else return;
      if (phase_.compare_exchange_weak(before, after)) return;
    }
  }
  bool Stopped() const noexcept
  {
    const auto phase = phase_.load();
    return phase == Phase::StoppedResume || phase == Phase::StoppedVeto || phase == Phase::Done;
  }
  bool TakeResumeRequest() noexcept
  {
    auto expected = Phase::StoppedResume;
    return phase_.compare_exchange_strong(expected, Phase::Done);
  }
private:
  std::atomic<Phase> phase_{Phase::Dormant};
};
}
