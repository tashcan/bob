#pragma once

#include "boolean_settings.h"

namespace mod_settings
{
// One instance per live view, never a persisted copy of the preference. Native
// adapters own rendering and invalidate this view before its context is released.
class BooleanView
{
public:
  explicit BooleanView(BooleanSetting& setting)
      : setting_(setting)
  {
  }

  void Bind()
  {
    snapshot_   = setting_.Observe();
    unresolved_ = !snapshot_.state.known();
    failed_     = false;
    bound_      = true;
  }
  void Unbind()
  {
    snapshot_   = {};
    bound_      = false;
    unresolved_ = true;
    failed_     = false;
  }
  void Invalidate()
  {
    snapshot_   = {};
    unresolved_ = true;
    failed_     = false;
  }
  WriteResult Request(bool desired)
  {
    if (!editable())
      return {Outcome::Rejected, snapshot_};
    auto result = setting_.SetFromUser(desired, snapshot_);
    if (result.outcome == Outcome::Suppressed || result.outcome == Outcome::Busy)
      return result;
    snapshot_   = result.snapshot;
    failed_     = result.outcome != Outcome::AppliedVerified && result.outcome != Outcome::Unchanged;
    unresolved_ = !snapshot_.state.known() || result.outcome == Outcome::Unverified;
    return result;
  }
  bool editable() const
  { return bound_ && !unresolved_ && snapshot_.state.known(); }
  bool failed() const
  { return failed_; }
  std::optional<bool> value() const
  { return editable() ? snapshot_.state.value : std::nullopt; }
  BooleanSetting& setting()
  { return setting_; }

private:
  BooleanSetting& setting_;
  Snapshot        snapshot_;
  bool            bound_      = false;
  bool            unresolved_ = true;
  bool            failed_     = false;
};
} // namespace mod_settings
