#include "settings/boolean_view.h"
#include "settings/page_catalog.h"
#include <cassert>
#include <iostream>

using namespace mod_settings;
int main()
{
  unsigned       reads = 0, writes = 0;
  bool           value = false;
  BooleanSetting setting({"mod.example.enabled", "Example",
                          [&] {
                            ++reads;
                            return ReadResult::Known(value, 1);
                          },
                          [&](bool desired, std::uint64_t) {
                            ++writes;
                            value = desired;
                            return ApplyResult::Applied;
                          }});
  PageCatalog    catalog("mod.settings", "Mod Settings");
  assert(catalog.AddPage("group.a", "Group A", "mod.settings") == Registration::Added);
  assert(catalog.AddPage("group.b", "Group B", "mod.settings") == Registration::Added);
  assert(catalog.AddPage("nested", "Nested", "group.a") == Registration::Added);
  assert(catalog.AddPage("empty", "Empty", "mod.settings") == Registration::Added);
  assert(catalog.AddPage("orphan", "Orphan", "missing") == Registration::Invalid);
  assert(catalog.AddPage("group.a", "Duplicate", "mod.settings") == Registration::Duplicate);
  assert(catalog.AddBoolean("nested", setting) == Registration::Added);
  assert(catalog.AddBoolean("nested", setting) == Registration::Duplicate);
  assert(catalog.AddBoolean("group.b", setting) == Registration::Added);
  BooleanSetting collision({"mod.example.enabled", "Other", [] { return ReadResult::Known(true, 1); },
                            [](bool, std::uint64_t) { return ApplyResult::Applied; }});
  assert(catalog.AddBoolean("group.a", collision) == Registration::Invalid);
  auto first = catalog.Build();
  assert(reads == 0 && writes == 0);
  assert(first.size() == 4 && first[0].id == "mod.settings");
  assert(first[3].parent == "group.a");
  first.clear(); // Destroy the old presentation plan before another visit.
  auto second = catalog.Build();
  assert(second.size() == 4 && second[3].booleans[0] == &setting);
  assert(catalog.AddPage("late", "Late", "mod.settings") == Registration::Frozen);
  assert(catalog.AddBoolean("group.a", setting) == Registration::Frozen);

  BooleanView view(*second[3].booleans[0]);
  view.Bind();
  assert(view.Request(true).outcome == Outcome::AppliedVerified);
  assert(value && writes == 1);
  view.Unbind();
  auto        third = catalog.Build();
  BooleanView mirror(*third[2].booleans[0]);
  mirror.Bind();
  assert(mirror.value() == true && writes == 1);
  assert(view.Request(false).outcome == Outcome::Rejected); // Old page cannot write.

  PageCatalog empty("root", "Empty");
  assert(empty.AddPage("unused", "Unused", "root") == Registration::Added);
  assert(empty.Build().empty());
  bool        rejected = false;
  std::thread wrong_thread([&] {
    try {
      (void)catalog.Build();
    } catch (const std::logic_error&) {
      rejected = true;
    }
  });
  wrong_thread.join();
  assert(rejected);
  std::cout << "Settings catalog rebuild/identity/lifetime fixtures passed\n";
}
