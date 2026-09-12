#include "mod_pages.h"
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
}
} // namespace mod_settings
