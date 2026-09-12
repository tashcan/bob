#include "runtime_config_writer.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace config_edit;
using namespace std::chrono_literals;
namespace
{
std::mutex              gate;
std::condition_variable changed;
bool                    entered = false, released = false;
Outcome                 first_result = Outcome::Saved;
std::vector<Request>    requests;
std::thread::id         save_thread;
int                     reports = 0;

void Check(bool value)
{
  if (!value)
    throw std::runtime_error("worker fixture failed");
}
Outcome Save(TomlEditor&, const std::filesystem::path&, const Request& request)
{
  std::unique_lock lock(gate);
  save_thread = std::this_thread::get_id();
  requests.push_back(request);
  if (requests.size() == 1) {
    entered = true;
    changed.notify_all();
    if (!changed.wait_for(lock, 5s, [] { return released; }))
      throw std::runtime_error("fixture timed out");
    return first_result;
  }
  return Outcome::Saved;
}
void Report(Outcome)
{
  std::lock_guard lock(gate);
  ++reports;
}
void Begin(Outcome result)
{
  entered = released = false;
  requests.clear();
  reports      = 0;
  first_result = result;
}
void AwaitSave()
{
  std::unique_lock lock(gate);
  Check(changed.wait_for(lock, 5s, [] { return entered; }));
}
void Release()
{
  std::lock_guard lock(gate);
  released = true;
  changed.notify_all();
}
} // namespace

// Block at the real worker's save boundary to exercise scheduling deterministically.
#define CONFIG_EDIT_SAVE(editor, path, request) Save(editor, path, request)
#include "../mods/src/runtime_config_writer.cc"

int main()
{
  try {
    for (auto outcome : {Outcome::Saved, Outcome::AlreadySaved, Outcome::Conflict, Outcome::IoError}) {
      Begin(outcome);
      RuntimeConfigWriter writer("unused", Value{std::string("none")}, Report);
      Check(writer.Submit("invalid") == 0);
      Check(writer.Submit("warp") == 1);
      AwaitSave();
      Check(writer.HasWork());
      Check(writer.Submit("jump") == 2);
      Check(writer.Submit("none") == 3);
      writer.Stop(false);
      Check(!writer.PollStopped());
      Check(writer.Submit("warp") == 0);
      Release();
      auto deadline = std::chrono::steady_clock::now() + 5s;
      while (!writer.PollStopped()) {
        Check(std::chrono::steady_clock::now() < deadline);
        std::this_thread::yield();
      }
      Check(!writer.HasWork());
      Check(requests.size() == 2);
      Check(requests[1].desired == Value{std::string("none")});
      const bool success = outcome == Outcome::Saved || outcome == Outcome::AlreadySaved;
      Check(requests[1].expected == std::optional<Value>{std::string(success ? "warp" : "none")});
      Check(reports == (success ? 0 : 1));
      Check(save_thread != std::this_thread::get_id());
      Check(writer.LastCompletion().revision == 3);
      Check(writer.LastCompletion().outcome == Outcome::Saved);
    }
    Begin(Outcome::Saved);
    {
      RuntimeConfigWriter writer("unused", Value{std::string("none")});
      writer.Submit("warp");
      AwaitSave();
      writer.Submit("jump");
      writer.Stop(true);
      Check(writer.LastCompletion().revision == 2);
      Check(writer.LastCompletion().outcome == Outcome::Cancelled);
      Release();
      auto deadline = std::chrono::steady_clock::now() + 5s;
      while (!writer.PollStopped()) {
        Check(std::chrono::steady_clock::now() < deadline);
        std::this_thread::yield();
      }
      Check(requests.size() == 1);
      Check(writer.LastCompletion().revision == 2);
      Check(writer.LastCompletion().outcome == Outcome::Cancelled);
    }
    RuntimeConfigWriter idle("unused", std::nullopt);
    idle.Stop(false);
    Check(idle.PollStopped());
    std::cout << "Runtime writer coalescing/drain/cancel fixtures passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
