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
  assert(second.size() == 4 && std::get<BooleanSetting*>(second[3].items[0]) == &setting);
  assert(catalog.AddPage("late", "Late", "mod.settings") == Registration::Frozen);
  assert(catalog.AddBoolean("group.a", setting) == Registration::Frozen);

  BooleanView view(*std::get<BooleanSetting*>(second[3].items[0]));
  view.Bind();
  assert(view.Request(true).outcome == Outcome::AppliedVerified);
  assert(value && writes == 1);
  view.Unbind();
  auto        third = catalog.Build();
  BooleanView mirror(*std::get<BooleanSetting*>(third[2].items[0]));
  mirror.Bind();
  assert(mirror.value() == true && writes == 1);
  assert(view.Request(false).outcome == Outcome::Rejected); // Old page cannot write.

  PageCatalog empty("root", "Empty");
  assert(empty.AddPage("unused", "Unused", "root") == Registration::Added);
  assert(empty.AddHeading("unused", "lonely", "Heading without controls") == Registration::Added);
  assert(empty.Build().empty());
  ChoiceSetting player({"player", "Player detail", [] { return ValueReadResult<int>::Known(0, 1); },
                        [](int, std::uint64_t) { return ApplyResult::Applied; }},
                       {"Native", "Expanded", "Compact", "Threshold"});
  ChoiceSetting other({"other", "Other detail", [] { return ValueReadResult<int>::Known(0, 1); },
                       [](int, std::uint64_t) { return ApplyResult::Applied; }},
                      {"Native", "Expanded", "Compact", "Threshold"});
  SliderSetting playerSlider({"player.zoom", "Threshold", [] { return ValueReadResult<float>::Known(0.5f, 1); },
                              [](float, std::uint64_t) { return ApplyResult::Applied; }},
                             0, 1, 0.01f, [] { return true; });
  SliderSetting otherSlider({"other.zoom", "Threshold", [] { return ValueReadResult<float>::Known(0.5f, 1); },
                             [](float, std::uint64_t) { return ApplyResult::Applied; }},
                            0, 1, 0.01f, [] { return true; });
  PageCatalog   combined("labels", "Fleet Labels");
  assert(combined.AddHeading("labels", "player.heading", "Player") == Registration::Added);
  assert(combined.AddChoice("labels", player) == Registration::Added);
  assert(combined.AddSlider("labels", playerSlider) == Registration::Added);
  assert(combined.AddHeading("labels", "other.heading", "Non-player") == Registration::Added);
  assert(combined.AddChoice("labels", other) == Registration::Added);
  assert(combined.AddSlider("labels", otherSlider) == Registration::Added);
  const auto ordered = combined.Build();
  assert(ordered.size() == 1 && ordered[0].ControlRows() == 10 && ordered[0].items.size() == 6);
  assert(std::get<PageCatalog::Heading>(ordered[0].items[0]).label == "Player");
  assert(std::get<ChoiceSetting*>(ordered[0].items[1]) == &player);
  assert(std::get<SliderSetting*>(ordered[0].items[2]) == &playerSlider);
  assert(std::get<PageCatalog::Heading>(ordered[0].items[3]).label == "Non-player");
  assert(std::get<ChoiceSetting*>(ordered[0].items[4]) == &other);
  assert(std::get<SliderSetting*>(ordered[0].items[5]) == &otherSlider);
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
