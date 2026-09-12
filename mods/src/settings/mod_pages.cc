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
  // Group by the existing TOML section only when there is a working control.
  // Placement and display names do not change setting or storage identities.
  catalog.AddPage("community_mod.graphics", "Graphics", "community_mod.settings");
  catalog.AddPage("community_mod.ui", "User Interface", "community_mod.settings");
  catalog.AddPage("community_mod.navigation.warp", "Instant warp mode", "community_mod.ui");
  catalog.AddChoice("community_mod.navigation.warp", WarpModeSetting());
  catalog.AddPage("community_mod.labels", "Fleet Labels", "community_mod.graphics");
  for (bool player : {true, false}) {
    catalog.AddHeading("community_mod.labels", player ? "community_mod.labels.player" : "community_mod.labels.other",
                       player ? "Player" : "Non-player", true);
    catalog.AddChoice("community_mod.labels", FleetLabelDetailSetting(player));
    catalog.AddSlider("community_mod.labels", FleetLabelThresholdSetting(player));
  }
}
} // namespace mod_settings
