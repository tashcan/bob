#include "config.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>

#include <absl/strings/str_split.h>
#include <mapkey.h>
#include <prime/KeyCode.h>

std::map<std::string, int> bannerTypes{
    {"Standard", ToastState::Standard},
    {"FactionWarning", ToastState::FactionWarning},
    {"FactionLevelUp", ToastState::FactionLevelUp},
    {"FactionLevelDown", ToastState::FactionLevelDown},
    {"FactionDiscovered", ToastState::FactionDiscovered},
    {"IncomingAttackFaction", ToastState::IncomingAttackFaction},
    {"FleetBattle", ToastState::FleetBattle},
    {"Victory", ToastState::Victory},
    {"Defeat", ToastState::Defeat},
    {"Event", ToastState::Tournament},
    {"ArmadaCreated", ToastState::ArmadaCreated},
    {"ArmadaCanceled", ToastState::ArmadaCanceled},
    {"ArmadaIncomingAttack", ToastState::ArmadaIncomingAttack},
    {"ArmadaBattleWon", ToastState::ArmadaBattleWon},
    {"ArmadaBattleLost", ToastState::ArmadaBattleLost},
    {"DiplomacyUpdated", ToastState::DiplomacyUpdated},
    {"JoinedTakeover", ToastState::JoinedTakeover},
    {"CompetitorJoinedTakeover", ToastState::CompetitorJoinedTakeover},
    {"AbandonedTerritory", ToastState::AbandonedTerritory},
    {"TakeoverVictory", ToastState::TakeoverVictory},
    {"TakeoverDefeat", ToastState::TakeoverDefeat},
};

Config::Config()
{
  Load();
}

void Config::Save(toml::table config, std::string_view filename, bool apply_warning)
{
  std::ofstream config_file;
  config_file.open(filename);
  if (apply_warning) {
    char buff[44];
    snprintf(buff, 44, "%-44s", CONFIG_FILE_DEFAULT);

    config_file << "#######################################################################\n";
    config_file << "#######################################################################\n";
    config_file << "####                                                               ####\n";
    config_file << "#### NOTE: This file is not the configuration file that is used    ####\n";
    config_file << "####       by the STFC community patch.  It is provided to help    ####\n";
    config_file << "####       see what configuration is being used by the runtime     ####\n";
    config_file << "####       and any desired settings should be copied to the same   ####\n";
    config_file << "####       section in: " << buff << " ####\n";
    config_file << "####                                                               ####\n";
    config_file << "#######################################################################\n";
    config_file << "#######################################################################\n\n";
  }
  config_file << config;
  config_file.close();
}

Config& Config::Get()
{
  static Config config;
  return config;
}

std::string get_config_type_as_string(toml::node_type type)
{
  switch (type) {
    case toml::node_type::none:
      return "Not-a-node.";
    case toml::node_type::table:
      return "toml::table.";
    case toml::node_type::array:
      return "toml::array.";
    case toml::node_type::string:
      return "toml::value<std::string>.";
    case toml::node_type::integer:
      return "toml::value<int64_t>.";
    case toml::node_type::floating_point:
      return "toml::value<double>.";
    case toml::node_type::boolean:
      return "toml::value<bool>.";
    case toml::node_type::date:
      return "toml::value<date>.";
    case toml::node_type::time:
      return "toml::value<time>.";
    case toml::node_type::date_time:
      return "toml::value<date_time>.";
  };

  return "The node type is unknown";
}

template <typename T>
inline T get_config_or_default(toml::table config, toml::table& new_config, std::string_view section,
                               std::string_view item, T default_value)
{
  config.emplace<toml::table>(section, toml::table());
  new_config.emplace<toml::table>(section, toml::table());

  auto sectionTable = new_config[section];
  auto final_value  = config[section][item].value_or(default_value);
  sectionTable.as_table()->insert_or_assign(item, final_value);

  spdlog::info("config value {}.{} value: {}", section, item, final_value);

  return (T)final_value;
}

void parse_config_shortcut(toml::table config, toml::table& new_config, std::string_view item,
                           GameFunction gameFunction, std::string_view default_value)
{
  auto section = "shortcuts";

  config.emplace<toml::table>(section, toml::table());
  new_config.emplace<toml::table>(section, toml::table());

  auto sectionTable = new_config[section];
  auto config_value = config[section][item].value_or(default_value);

  MapKey      mapKey   = MapKey::Parse(config_value);
  std::string shortcut = "disabled";

  if (mapKey.Key == KeyCode::None) {
    mapKey = MapKey::Parse(default_value);
  }

  if (mapKey.Key != KeyCode::None) {
    shortcut = mapKey.GetParsedValues();
  }

  MapKey::SetMappedKey(gameFunction, mapKey);

  sectionTable.as_table()->insert_or_assign(item, shortcut);

  spdlog::info("shortcut value {}.{} value: {}", section, item, shortcut);
  ;
}

void Config::Load()
{
  spdlog::info("Loading Config");

  toml::table config;
  toml::table parsed;
  bool        write_config = false;
  try {
    config       = toml::parse_file("community_patch_settings.toml");
    write_config = true;
  } catch (const toml::parse_error& e) {
    spdlog::warn("Failed to load config file, falling back to default settings: {}", e.description());
    write_config = false;
  } catch (...) {
    spdlog::info("Failed to load config file, falling back to default settings");
    write_config = false;
  }

  this->ui_scale         = get_config_or_default(config, parsed, "graphics", "ui_scale", 0.9f);
  this->ui_scale_adjust  = get_config_or_default(config, parsed, "graphics", "ui_scale_adjust", 0.05f);
  this->zoom             = get_config_or_default(config, parsed, "graphics", "zoom", 2500.f);
  this->free_resize      = get_config_or_default(config, parsed, "graphics", "free_resize", true);
  this->adjust_scale_res = get_config_or_default(config, parsed, "graphics", "adjust_scale_res", false);
  this->system_pan_momentum_falloff =
      get_config_or_default(config, parsed, "graphics", "system_pan_momentum_falloff", 0.8f);
  this->keyboard_zoom_speed = get_config_or_default(config, parsed, "graphics", "keyboard_zoom_speed", 350.0f);
  this->borderless_fullscreen_f11 =
      get_config_or_default(config, parsed, "graphics", "borderless_fullscreen_f11", true);
  this->target_framerate     = get_config_or_default(config, parsed, "graphics", "target_framerate", 60);
  this->vsync                = get_config_or_default(config, parsed, "graphics", "vsync", 1);
  this->transition_time      = get_config_or_default(config, parsed, "graphics", "transition_time", 0.01f);
  this->show_all_resolutions = get_config_or_default(config, parsed, "graphics", "show_all_resolutions", false);
  this->default_system_zoom  = get_config_or_default(config, parsed, "graphics", "default_system_zoom", 0.0f);

  this->system_zoom_preset_1 = get_config_or_default(config, parsed, "graphics", "system_zoom_preset_1", 0.0f);
  this->system_zoom_preset_2 = get_config_or_default(config, parsed, "graphics", "system_zoom_preset_2", 0.0f);
  this->system_zoom_preset_3 = get_config_or_default(config, parsed, "graphics", "system_zoom_preset_3", 0.0f);
  this->system_zoom_preset_4 = get_config_or_default(config, parsed, "graphics", "system_zoom_preset_4", 0.0f);
  this->system_zoom_preset_5 = get_config_or_default(config, parsed, "graphics", "system_zoom_preset_5", 0.0f);

  this->hotkeys_enabled     = get_config_or_default(config, parsed, "control", "hotkeys_enabled", true);
  this->hotkeys_extended    = get_config_or_default(config, parsed, "control", "hotkeys_extended", true);
  this->use_scopely_hotkeys = get_config_or_default(config, parsed, "control", "use_scopely_hotkeys", false);

  this->use_out_of_dock_power = get_config_or_default(config, parsed, "buffs", "use_out_of_dock_power", false);

  this->disable_toast_banners  = get_config_or_default(config, parsed, "ui", "disable_toast_banners", true);
  this->extend_donation_slider = get_config_or_default(config, parsed, "ui", "extend_donation_slider", false);
  this->disable_galaxy_chat    = get_config_or_default(config, parsed, "ui", "disable_galaxy_chat", false);
  this->show_cargo_default     = get_config_or_default(config, parsed, "ui", "show_cargo_default", false);
  this->show_player_cargo      = get_config_or_default(config, parsed, "ui", "show_player_cargo", false);
  this->show_station_cargo     = get_config_or_default(config, parsed, "ui", "show_station_cargo", true);
  this->show_hostile_cargo     = get_config_or_default(config, parsed, "ui", "show_hostile_cargo", true);
  this->show_armada_cargo      = get_config_or_default(config, parsed, "ui", "show_armada_cargo", true);

  this->always_skip_reveal_sequence = get_config_or_default(config, parsed, "ui", "always_skip_reveal_sequence", false);
  this->stay_in_bundle_after_summary =
      get_config_or_default(config, parsed, "ui", "stay_in_bundle_after_summary", true);

  this->fix_unity_web_requests = get_config_or_default(config, parsed, "tech", "fix_unity_web_requests", true);

  this->sync_url        = get_config_or_default<std::string>(config, parsed, "sync", "url", "");
  this->sync_token      = get_config_or_default<std::string>(config, parsed, "sync", "token", "");
  this->sync_battlelogs = get_config_or_default(config, parsed, "sync", "battlelogs", false);
  this->sync_resources  = get_config_or_default(config, parsed, "sync", "resources", false);
  this->sync_officer    = get_config_or_default(config, parsed, "sync", "officer", false);
  this->sync_missions   = get_config_or_default(config, parsed, "sync", "missions", false);
  this->sync_research   = get_config_or_default(config, parsed, "sync", "research", false);
  this->sync_tech       = get_config_or_default(config, parsed, "sync", "tech", false);
  this->sync_traits     = get_config_or_default(config, parsed, "sync", "traits", false);
  this->sync_buildings  = get_config_or_default(config, parsed, "sync", "buildings", false);
  this->sync_ships      = get_config_or_default(config, parsed, "sync", "ships", false);

  // must explicitly included std::string typing here or we get back char * which fails us!
  std::string disabled_banner_types_str =
      get_config_or_default<std::string>(config, parsed, "ui", "disabled_banner_types", "");

  std::vector<std::string> types = absl::StrSplit(disabled_banner_types_str, ",", absl::SkipWhitespace());

  std::string       bannerString = "";
  std::stringstream message;
  message << "Parsing banner strings";

  spdlog::info(message.str());

  for (const auto& [key, value] : bannerTypes) {
    auto lower_key = absl::AsciiStrToLower(key);

    for (const std::string_view _type : types) {
      auto stripped_type = absl::StripLeadingAsciiWhitespace(_type);
      auto lower_type    = absl::AsciiStrToLower(stripped_type);

      if (lower_key == lower_type) {
        this->disabled_banner_types.emplace_back(value);
        if (bannerString.length() != 0) {
          bannerString.append(", ");
        }
        bannerString.append(key);
      }
    }
  }

  message.str("");
  message << "Final disabledbanner types: " << bannerString;
  spdlog::info(message.str());

  parsed["ui"].as_table()->insert_or_assign("disabled_banner_types", bannerString);

  parse_config_shortcut(config, parsed, "move_left1", GameFunction::MoveLeft1, "LEFT");
  parse_config_shortcut(config, parsed, "move_left2", GameFunction::MoveLeft2, "A");
  parse_config_shortcut(config, parsed, "move_right1", GameFunction::MoveRight1, "RIGHT");
  parse_config_shortcut(config, parsed, "move_right2", GameFunction::MoveRight2, "D");
  parse_config_shortcut(config, parsed, "move_down1", GameFunction::MoveDown1, "DOWN");
  parse_config_shortcut(config, parsed, "move_down2", GameFunction::MoveDown2, "S");
  parse_config_shortcut(config, parsed, "move_up1", GameFunction::MoveUp1, "UP");
  parse_config_shortcut(config, parsed, "move_up2", GameFunction::MoveUp2, "W");

  parse_config_shortcut(config, parsed, "select_chatalliance", GameFunction::SelectChatAlliance, "CTRL-2");
  parse_config_shortcut(config, parsed, "select_chatglobal", GameFunction::SelectChatGlobal, "CTRL-1");
  parse_config_shortcut(config, parsed, "select_chatprivate", GameFunction::SelectChatPrivate, "CTRL-3");

  parse_config_shortcut(config, parsed, "select_ship1", GameFunction::SelectShip1, "1");
  parse_config_shortcut(config, parsed, "select_ship2", GameFunction::SelectShip2, "2");
  parse_config_shortcut(config, parsed, "select_ship3", GameFunction::SelectShip3, "3");
  parse_config_shortcut(config, parsed, "select_ship4", GameFunction::SelectShip4, "4");
  parse_config_shortcut(config, parsed, "select_ship5", GameFunction::SelectShip5, "5");
  parse_config_shortcut(config, parsed, "select_ship6", GameFunction::SelectShip6, "6");
  parse_config_shortcut(config, parsed, "select_ship7", GameFunction::SelectShip7, "7");
  parse_config_shortcut(config, parsed, "select_ship8", GameFunction::SelectShip8, "8");

  parse_config_shortcut(config, parsed, "action_primary", GameFunction::ActionPrimary, "SPACE");
  parse_config_shortcut(config, parsed, "action_secondary", GameFunction::ActionSecondary, "R");
  parse_config_shortcut(config, parsed, "action_view", GameFunction::ActionView, "V");
  parse_config_shortcut(config, parsed, "action_recall", GameFunction::ActionRecall, "R");
  parse_config_shortcut(config, parsed, "action_repair", GameFunction::ActionRepair, "R");
  parse_config_shortcut(config, parsed, "show_chat", GameFunction::ShowChat, "C");
  parse_config_shortcut(config, parsed, "show_chatside1", GameFunction::ShowChatSide1, "ALT-C");
  parse_config_shortcut(config, parsed, "show_chatside2", GameFunction::ShowChatSide2, "`");
  parse_config_shortcut(config, parsed, "show_galaxy", GameFunction::ShowGalaxy, "G");
  parse_config_shortcut(config, parsed, "show_system", GameFunction::ShowSystem, "H");
  parse_config_shortcut(config, parsed, "zoom_preset1", GameFunction::ZoomPreset1, "F1");
  parse_config_shortcut(config, parsed, "zoom_preset2", GameFunction::ZoomPreset2, "F2");
  parse_config_shortcut(config, parsed, "zoom_preset3", GameFunction::ZoomPreset3, "F3");
  parse_config_shortcut(config, parsed, "zoom_preset4", GameFunction::ZoomPreset4, "F4");
  parse_config_shortcut(config, parsed, "zoom_preset5", GameFunction::ZoomPreset5, "F5");
  parse_config_shortcut(config, parsed, "zoom_in", GameFunction::ZoomIn, "Q");
  parse_config_shortcut(config, parsed, "zoom_out", GameFunction::ZoomOut, "E");
  parse_config_shortcut(config, parsed, "zoom_max", GameFunction::ZoomMax, "MINUS");
  parse_config_shortcut(config, parsed, "zoom_min", GameFunction::ZoomMin, "BACKSPACE");
  parse_config_shortcut(config, parsed, "zoom_reset", GameFunction::ZoomReset, "=");
  parse_config_shortcut(config, parsed, "ui_scaleup", GameFunction::UiScaleUp, "PGUP");
  parse_config_shortcut(config, parsed, "ui_scaledown", GameFunction::UiScaleDown, "PGDOWN");

  if (this->hotkeys_extended) {
    parse_config_shortcut(config, parsed, "show_awayteam", GameFunction::ShowAwayTeam, "SHIFT-T");
    parse_config_shortcut(config, parsed, "show_gifts", GameFunction::ShowGifts, "\\");
    parse_config_shortcut(config, parsed, "show_alliance", GameFunction::ShowAlliance, "SHIFT-\\");
    parse_config_shortcut(config, parsed, "show_alliance_help", GameFunction::ShowAllianceHelp, "ALT-\\");
    parse_config_shortcut(config, parsed, "show_alliance_help", GameFunction::ShowAllianceArmada, "CTRL-\\");
    parse_config_shortcut(config, parsed, "show_bookmarks", GameFunction::ShowBookmarks, "B");
    parse_config_shortcut(config, parsed, "show_commander", GameFunction::ShowCommander, "O");
    parse_config_shortcut(config, parsed, "show_daily", GameFunction::ShowDaily, "Z");
    parse_config_shortcut(config, parsed, "show_events", GameFunction::ShowEvents, "T");
    parse_config_shortcut(config, parsed, "show_exocomp", GameFunction::ShowExoComp, "X");
    parse_config_shortcut(config, parsed, "show_factions", GameFunction::ShowFactions, "F");
    parse_config_shortcut(config, parsed, "show_inventory", GameFunction::ShowInventory, "I");
    parse_config_shortcut(config, parsed, "show_missions", GameFunction::ShowMissions, "M");
    parse_config_shortcut(config, parsed, "show_research", GameFunction::ShowResearch, "U");
    parse_config_shortcut(config, parsed, "show_officers", GameFunction::ShowOfficers, "SHIFT-O");
    parse_config_shortcut(config, parsed, "show_qtrials", GameFunction::ShowQTrials, "SHIFT-Q");
    parse_config_shortcut(config, parsed, "show_refinery", GameFunction::ShowRefinery, "SHIFT-F");
    parse_config_shortcut(config, parsed, "show_ships", GameFunction::ShowShips, "N");
    parse_config_shortcut(config, parsed, "show_stationexterior", GameFunction::ShoWStationExterior, "SHIFT-G");
    parse_config_shortcut(config, parsed, "show_stationinterior", GameFunction::ShowStationInterior, "SHIFT-H");
    parse_config_shortcut(config, parsed, "set_zoom_preset1", GameFunction::SetZoomPreset1, "SHIFT-F1");
    parse_config_shortcut(config, parsed, "set_zoom_preset2", GameFunction::SetZoomPreset2, "SHIFT-F2");
    parse_config_shortcut(config, parsed, "set_zoom_preset3", GameFunction::SetZoomPreset3, "SHIFT-F3");
    parse_config_shortcut(config, parsed, "set_zoom_preset4", GameFunction::SetZoomPreset4, "SHIFT-F4");
    parse_config_shortcut(config, parsed, "set_zoom_preset5", GameFunction::SetZoomPreset5, "SHIFT-F5");
    parse_config_shortcut(config, parsed, "toggle_cargo_default", GameFunction::ToggleCargoDefault, "ALT-1");
    parse_config_shortcut(config, parsed, "toggle_cargo_player", GameFunction::ToggleCargoPlayer, "ALT-2");
    parse_config_shortcut(config, parsed, "toggle_cargo_station", GameFunction::ToggleCargoStation, "ALT-3");
    parse_config_shortcut(config, parsed, "toggle_cargo_hostile", GameFunction::ToggleCargoHostile, "ALT-4");
    parse_config_shortcut(config, parsed, "toggle_cargo_armada", GameFunction::ToggleCargoArmada, "ALT-5");
  }

  if (!std::filesystem::exists(CONFIG_FILE_DEFAULT)) {
    message.str("");
    message << "Creating " << CONFIG_FILE_DEFAULT << " (default config file)";
    spdlog::warn(message.str());

    Config::Save(config, CONFIG_FILE_DEFAULT, false);
  }

  message.str("");
  message << "Creating " << CONFIG_FILE_RUNTIME << " (final config file)";
  spdlog::info(message.str());

  if (std::filesystem::exists(CONFIG_FILE_PARSED)) {
    message << "Removing " << CONFIG_FILE_PARSED << " (old parsed file)";
    std::filesystem::remove(CONFIG_FILE_PARSED);
  }

  Config::Save(parsed, CONFIG_FILE_RUNTIME);

  std::cout << message.str() << ":\n-----------------------------\n\n" << parsed << "\n\n-----------------------------\n";
}
