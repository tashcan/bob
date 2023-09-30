#include "config.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <toml++/toml.h>

#include <spdlog/spdlog.h>

#include <absl/strings/str_split.h>

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
  this->sync_active_missions = get_config_or_default(config, parsed, "sync", "active_missions", false);
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

  if (!std::filesystem::exists("community_patch_settings.toml")) {
    message.str("");
    message << "Creating new config file";
    spdlog::warn(message.str());

    std::ofstream config_file;
    config_file.open("community_patch_settings.toml");
    config_file << parsed;
    config_file.close();
  }

  message.str("");
  message << "Creating paresd config file";
  spdlog::info(message.str());

  std::ofstream config_file;
  config_file.open("community_patch_settings_parsed.toml");
  config_file << parsed;
  config_file.close();

  std::cout << "Running config: \n" << parsed << "\n\n";
}
