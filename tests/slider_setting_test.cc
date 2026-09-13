#include "settings/native_boolean_callback.h"
#include "settings/native_view_state.h"
#include "settings/page_catalog.h"
#include <cassert>
#include <iostream>
#include <limits>

using namespace mod_settings;
float stored = 0;
float GetFloat(Il2CppObject*, const MethodInfo*)
{ return stored; }
void SetFloat(Il2CppObject*, float value, const MethodInfo*)
{ stored = value; }
int main()
{
  float         value   = 0.33333f;
  bool          enabled = true, available = true;
  int           writes = 0;
  SliderSetting setting({"threshold", "Threshold",
                         [&] { return available ? ValueReadResult<float>::Known(value, 1) : ValueReadResult<float>{}; },
                         [&](float desired, std::uint64_t) {
                           ++writes;
                           value = desired;
                           return ApplyResult::Applied;
                         }},
                        0, 1, 0.01f, [&] { return enabled; });
  NativeViewState view(setting), stale(setting);
  view.Bind();
  stale.Bind();
  assert(view.known() && view.number() == 0.33333f); // Reading does not quantize player-authored values.
  assert(view.Request(0.604f) == Outcome::AppliedVerified && std::abs(value - 0.6f) < 0.000001f);
  assert(stale.Request(0.8f) == Outcome::Conflict && writes == 1);
  assert(view.Request(std::numeric_limits<float>::quiet_NaN()) == Outcome::Rejected);
  assert(view.Request(std::numeric_limits<float>::infinity()) == Outcome::Rejected);
  assert(view.Request(-0.01f) == Outcome::Rejected && view.Request(1.01f) == Outcome::Rejected);
  enabled = false;
  assert(view.known() && !view.enabled() && view.Request(0.9f) == Outcome::Rejected && writes == 1);
  enabled = true;
  {
    NativeViewState::RenderScope render(view);
    assert(view.Request(0.9f) == Outcome::Suppressed && writes == 1);
  }
  assert(view.Request(1.0f) == Outcome::AppliedVerified && value == 1.0f);
  assert(view.Request(0.0f) == Outcome::AppliedVerified && value == 0.0f);
  available = false;
  view.Bind();
  assert(!view.known() && !view.enabled() && view.Request(0.8f) == Outcome::Rejected);
  available = true;
  value     = std::numeric_limits<float>::quiet_NaN();
  view.Bind();
  assert(!view.known());
  value = 0.5f;
  view.Bind();
  view.Unbind();
  assert(view.Request(0.7f) == Outcome::Rejected);
  PageCatalog catalog("root", "Mod Settings");
  assert(catalog.AddPage("labels", "Fleet Labels", "root") == Registration::Added);
  assert(catalog.AddSlider("labels", setting) == Registration::Added);
  assert(catalog.AddSlider("labels", setting) == Registration::Duplicate);
  assert(catalog.Build().size() == 2);
  assert(catalog.AddSlider("labels", setting) == Registration::Frozen);

  Il2CppType single{}, nothing{}, integer{};
  single.type  = IL2CPP_TYPE_R4;
  nothing.type = IL2CPP_TYPE_VOID;
  integer.type = IL2CPP_TYPE_I4;
  const Il2CppType* parameters[]{&single};
  MethodInfo        getter{}, setter{};
  getter.return_type      = &single;
  setter.return_type      = &nothing;
  setter.parameters       = parameters;
  setter.parameters_count = 1;
  NativeCallback<float>       get;
  NativeCallback<void, float> set;
  assert(get.Initialize(&getter, GetFloat) && set.Initialize(&setter, SetFloat));
  float input = 0.67f, output = 0;
  void* args[]{&input};
  set.method()->invoker_method(nullptr, set.method(), nullptr, args, nullptr);
  get.method()->invoker_method(nullptr, get.method(), nullptr, nullptr, &output);
  assert(output == input);
  NativeCallback<float> wrong;
  getter.return_type = &integer;
  assert(!wrong.Initialize(&getter, GetFloat));
  std::cout
      << "PASS slider: range, finite values, dependency, readback, stale views, render guard and Single callbacks\n";
}
