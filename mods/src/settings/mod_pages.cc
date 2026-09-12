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
    catalog.AddHeading("community_mod.labels", player ? "community_mod.labels.player" : "community_mod.labels.other",
                       player ? "Player" : "Non-player");
    catalog.AddChoice("community_mod.labels", FleetLabelDetailSetting(player));
    catalog.AddSlider("community_mod.labels", FleetLabelThresholdSetting(player));
  }
}
} // namespace mod_settings
