#include "runtime_config_writer.h"

#if _WIN32
#include <Windows.h>
#endif

#ifndef CONFIG_EDIT_SAVE
#define CONFIG_EDIT_SAVE(editor, path, request) (editor).Save(path, request)
#endif

namespace config_edit
{
RuntimeConfigWriter::RuntimeConfigWriter(std::filesystem::path path, std::optional<Value> initial, Reporter report)
    : path_(std::move(path))
    , saved_(std::move(initial))
    , report_(report)
{
}

RuntimeConfigWriter::~RuntimeConfigWriter()
{
  Stop(false);
  if (worker_.joinable())
    worker_.join();
}

std::uint64_t RuntimeConfigWriter::Submit(std::string mode)
{
  if (mode != "none" && mode != "warp" && mode != "jump")
    return 0;
  std::lock_guard lock(mutex_);
  if (stopping_ || cancel_pending_.load())
    return 0;
  pending_ = Pending{++revision_, {"ui", "auto_confirm_instant_warp", saved_, std::move(mode)}};
  has_work_.store(true);
  if (!worker_.joinable()) {
    try {
      worker_ = std::thread(&RuntimeConfigWriter::Run, this);
    } catch (...) {
      pending_.reset();
      has_work_.store(false);
      completion_ = {revision_, Outcome::IoError};
      return 0;
    }
  }
  wake_.notify_one();
  return revision_;
}

void RuntimeConfigWriter::Stop(bool cancel_pending)
{
  if (cancel_pending)
    RequestCancelPending();
  std::lock_guard lock(mutex_);
  stopping_ = true;
  if (cancel_pending && pending_) {
    completion_ = {pending_->revision, Outcome::Cancelled};
    pending_.reset();
  }
  wake_.notify_one();
}

void RuntimeConfigWriter::RequestCancelPending()
{
  cancel_pending_.store(true);
  wake_.notify_one();
}

RuntimeConfigWriter::Completion RuntimeConfigWriter::LastCompletion()
{
  std::lock_guard lock(mutex_);
  return completion_;
}

void RuntimeConfigWriter::Run()
{
  for (;;) {
    Pending work;
    {
      std::unique_lock lock(mutex_);
      wake_.wait(lock, [&] { return stopping_ || cancel_pending_.load() || pending_.has_value(); });
      if (cancel_pending_.load()) {
        stopping_ = true;
        if (pending_)
          completion_ = {pending_->revision, Outcome::Cancelled};
        pending_.reset();
      }
      if (!pending_)
        break;
      work = std::move(*pending_);
      pending_.reset();
    }
    Outcome outcome;
    try {
      outcome = CONFIG_EDIT_SAVE(editor_, path_, work.edit);
    } catch (...) {
      outcome = Outcome::IoError;
    }
    {
      std::lock_guard lock(mutex_);
      if (outcome == Outcome::Saved || outcome == Outcome::AlreadySaved) {
        // Rebase our queued intent over our own successful write, never over a
        // conflicting external edit. Failed saves leave the acknowledged value alone.
        if (pending_ && pending_->edit.expected == saved_)
          pending_->edit.expected = work.edit.desired;
        saved_ = work.edit.desired;
      }
      if (work.revision >= completion_.revision)
        completion_ = {work.revision, outcome};
    }
    if (report_ && outcome != Outcome::Saved && outcome != Outcome::AlreadySaved) {
      try {
        report_(outcome);
      } catch (...) { /* Diagnostics cannot kill the worker. */
      }
    }
    {
      std::lock_guard lock(mutex_);
      // A diagnostic callback is still active worker work, even after disk I/O.
      has_work_.store(pending_.has_value());
    }
  }
  has_work_.store(false);
  finished_.store(true);
}

bool RuntimeConfigWriter::PollStopped()
{
  if (!worker_.joinable()) {
    std::lock_guard lock(mutex_);
    return stopping_;
  }
#if _WIN32
  if (WaitForSingleObject(worker_.native_handle(), 0) != WAIT_OBJECT_0)
    return false;
#else
  // The game adapter is Windows-only until native macOS quit integration is validated.
  if (!finished_.load())
    return false;
#endif
  worker_.join();
  return true;
}

#if _WIN32
void* RuntimeConfigWriter::NativeHandle()
{ return worker_.joinable() ? worker_.native_handle() : nullptr; }
#endif
} // namespace config_edit
