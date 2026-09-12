#include "mod_pages.h"
#include "fleet_labels.h"
#include "warp_mode.h"

namespace mod_settings
{
PageCatalog& ModPages()
{
  static PageCatalog catalog("community_mod.settings", "Mod Settings");
  return catalog;
}
void RegisterModPages()
{
  auto& catalog = ModPages();
  catalog.AddPage("community_mod.navigation", "Navigation", "community_mod.settings");
  catalog.AddPage("community_mod.navigation.warp", "Instant warp mode", "community_mod.navigation");
  catalog.AddChoice("community_mod.navigation.warp", WarpModeSetting());
  catalog.AddPage("community_mod.labels", "Fleet Labels", "community_mod.settings");
  for (bool player : {true, false}) {
    const char* id = player ? "community_mod.labels.player" : "community_mod.labels.other";
    catalog.AddPage(id, player ? "Player fleets" : "Non-player fleets", "community_mod.labels");
    catalog.AddChoice(id, FleetLabelDetailSetting(player));
    catalog.AddSlider(id, FleetLabelThresholdSetting(player));
  }
}
} // namespace mod_settings
