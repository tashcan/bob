#include "settings/boolean_view.h"
#include <cassert>
#include <iostream>

using namespace mod_settings;
int main()
{
  bool           available = true, value = true;
  int            writes = 0;
  ApplyResult    apply  = ApplyResult::Applied;
  BooleanSetting setting({"test.confirmation", "Confirm test",
                          [&] { return available ? ReadResult::Known(value, 1) : ReadResult{}; },
                          [&](bool desired, std::uint64_t) {
                            ++writes;
                            if (apply != ApplyResult::Rejected)
                              value = desired;
                            return apply;
                          }});
  BooleanView    first(setting), second(setting);
  assert(!first.value() && first.Request(false).outcome == Outcome::Rejected);
  first.Bind();
  second.Bind();
  {
    BooleanSetting::RenderScope render(setting);
    assert(first.Request(false).outcome == Outcome::Suppressed);
    assert(first.value() == true && !first.failed() && writes == 0);
  }
  assert(first.Request(false).outcome == Outcome::AppliedVerified);
  assert(first.value() == false && writes == 1);
  assert(second.Request(false).outcome == Outcome::Conflict);
  assert(second.value() == false && second.failed() && writes == 1);
  second.Bind();
  assert(!second.failed());
  apply = ApplyResult::Rejected;
  assert(first.Request(true).outcome == Outcome::Rejected);
  assert(first.value() == false && first.failed());
  apply = ApplyResult::Unverified;
  assert(first.Request(true).outcome == Outcome::Unverified);
  assert(!first.value() && !first.editable() && first.failed());
  auto count = writes;
  assert(first.Request(false).outcome == Outcome::Rejected && writes == count);
  first.Bind();
  assert(first.value() == true);
  setting.InvalidateSession();
  first.Invalidate();
  second.Invalidate();
  assert(!first.value() && first.Request(false).outcome == Outcome::Rejected);
  available = false;
  first.Bind();
  assert(!first.value());
  available = true;
  first.Bind();
  assert(first.value() == true);
  first.Unbind();
  assert(!first.value() && first.Request(false).outcome == Outcome::Rejected);
  assert(writes == count);
  std::cout << "PASS boolean view: render, stale views, unavailable, uncertain writes and teardown\n";
}
