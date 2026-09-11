#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

namespace mod_settings
{
enum class Availability { Known, Unavailable, Unsupported };
struct ReadResult {
  Availability        availability = Availability::Unavailable;
  std::optional<bool> value;
  std::uint64_t       generation = 0;
  static ReadResult   Known(bool value, std::uint64_t generation)
  { return {Availability::Known, value, generation}; }
  bool known() const
  { return availability == Availability::Known && value.has_value() && generation != 0; }
};
enum class ApplyResult { Applied, Rejected, Unverified };
enum class Outcome { AppliedVerified, Unchanged, Suppressed, Busy, Conflict, Rejected, Unverified };
struct Definition {
  std::string                 id;
  std::string                 label;
  std::function<ReadResult()> read;
  // Adapter must revalidate its own generation immediately before mutation.
  std::function<ApplyResult(bool, std::uint64_t)> write;
};
struct Snapshot {
  ReadResult    state;
  std::uint64_t revision = 0;
  std::uint64_t epoch    = 0;
  std::uint64_t owner    = 0;
};
struct WriteResult {
  Outcome  outcome;
  Snapshot snapshot;
};

// All calls, including scope destruction, belong to the constructing UI thread.
// No timers, background work, locks, storage, or IL2CPP dependencies.
class BooleanSetting
{
public:
  explicit BooleanSetting(Definition definition)
      : definition_(std::move(definition))
  {
  }
  BooleanSetting(const BooleanSetting&)               = delete;
  BooleanSetting&    operator=(const BooleanSetting&) = delete;
  const std::string& id() const
  { return definition_.id; }
  const std::string& label() const
  { return definition_.label; }

  // One process-lifetime presentation adapter. Notifications are synchronous,
  // bounded and run under the write reentry guard; rendering cannot write back.
  bool SetChangeObserver(void (*observer)())
  {
    CheckThread();
    if (change_observer_ && change_observer_ != observer)
      return false;
    change_observer_ = observer;
    return true;
  }

  class RenderScope
  {
  public:
    explicit RenderScope(BooleanSetting& setting)
        : setting_(setting)
    {
      setting_.CheckThread();
      ++setting_.render_depth_;
    }
    ~RenderScope()
    { --setting_.render_depth_; }
    RenderScope(const RenderScope&)            = delete;
    RenderScope& operator=(const RenderScope&) = delete;

  private:
    BooleanSetting& setting_;
  };

  Snapshot Observe()
  {
    CheckThread();
    if (observing_)
      return {{}, current_.revision, epoch_, identity_};
    struct Observing {
      bool& flag;
      Observing(bool& flag)
          : flag(flag)
      { flag = true; }
      ~Observing()
      { flag = false; }
    } guard(observing_);
    const auto read_epoch = epoch_;
    ReadResult next;
    try {
      next = definition_.read();
    } catch (...) {
      next = {};
    }
    if (read_epoch != epoch_)
      next = {};
    if (!next.known()) {
      next.value.reset();
      next.generation = 0;
      if (next.availability == Availability::Known)
        next.availability = Availability::Unavailable;
    }
    if (next.availability != current_.state.availability || next.value != current_.state.value
        || next.generation != current_.state.generation)
      ++current_.revision;
    current_.state = next;
    current_.epoch = epoch_;
    current_.owner = identity_;
    return current_;
  }

  void InvalidateSession()
  {
    CheckThread();
    ++epoch_;
    current_ = {{}, current_.revision + 1, epoch_, identity_};
  }

  WriteResult SetFromUser(bool desired, const Snapshot& observed)
  {
    CheckThread();
    if (render_depth_)
      return {Outcome::Suppressed, current_};
    if (applying_)
      return {Outcome::Busy, current_};
    // Cover read callbacks as well as the setter: neither can reenter application.
    struct Applying {
      bool& flag;
      Applying(bool& flag)
          : flag(flag)
      { flag = true; }
      ~Applying()
      { flag = false; }
    } guard(applying_);
    const auto before = Observe();
    if (!before.state.known())
      return {Outcome::Rejected, before};
    if (observed.owner != identity_ || !observed.state.known() || observed.epoch != before.epoch
        || observed.revision != before.revision || observed.state.generation != before.state.generation
        || observed.state.value != before.state.value)
      return {Outcome::Conflict, before};
    if (*before.state.value == desired) {
      NotifyChanged();
      if (epoch_ != before.epoch)
        return {Outcome::Unverified, current_};
      return {Outcome::Unchanged, before};
    }
    ApplyResult applied = ApplyResult::Unverified;
    try {
      applied = definition_.write(desired, before.state.generation);
    } catch (...) {
    }
    if (epoch_ != before.epoch)
      return {Outcome::Unverified, current_};
    const auto after = Observe();
    if (!after.state.known() || after.state.generation != before.state.generation)
      return {Outcome::Unverified, after};
    if (applied == ApplyResult::Applied && *after.state.value == desired) {
      NotifyChanged();
      if (epoch_ != before.epoch)
        return {Outcome::Unverified, current_};
      return {Outcome::AppliedVerified, after};
    }
    // Preserve the authoritative readback even when the adapter reports failure.
    return {applied == ApplyResult::Rejected ? Outcome::Rejected : Outcome::Unverified, after};
  }

private:
  void NotifyChanged()
  {
    // Presentation failure does not undo or misreport an authoritative write.
    try {
      if (change_observer_)
        change_observer_();
    } catch (...) {
    }
  }
  void CheckThread() const
  {
    if (std::this_thread::get_id() != thread_)
      throw std::logic_error("setting thread mismatch");
  }
  Definition                               definition_;
  inline static std::atomic<std::uint64_t> next_identity_{1};
  const std::uint64_t                      identity_ = next_identity_.fetch_add(1);
  const std::thread::id                    thread_   = std::this_thread::get_id();
  Snapshot                                 current_;
  std::uint64_t                            epoch_        = 1;
  unsigned                                 render_depth_ = 0;
  bool                                     applying_     = false;
  bool                                     observing_    = false;
  void (*change_observer_)()                             = nullptr;
};

enum class Registration { Added, Duplicate, Invalid, Frozen };
class BooleanRegistry
{
public:
  Registration Register(Definition definition)
  {
    CheckThread();
    if (frozen_)
      return Registration::Frozen;
    if (definition.id.empty() || definition.label.empty() || !definition.read || !definition.write)
      return Registration::Invalid;
    if (settings_.contains(definition.id))
      return Registration::Duplicate;
    auto id = definition.id;
    settings_.try_emplace(std::move(id), std::move(definition));
    return Registration::Added;
  }
  BooleanSetting* Find(const std::string& id)
  {
    CheckThread();
    frozen_ = true;
    auto it = settings_.find(id);
    return it == settings_.end() ? nullptr : &it->second;
  }
  void InvalidateSession()
  {
    CheckThread();
    for (auto& [id, setting] : settings_)
      setting.InvalidateSession();
  }

private:
  void CheckThread() const
  {
    if (std::this_thread::get_id() != thread_)
      throw std::logic_error("registry thread mismatch");
  }
  const std::thread::id                 thread_ = std::this_thread::get_id();
  bool                                  frozen_ = false;
  std::map<std::string, BooleanSetting> settings_;
};
} // namespace mod_settings
