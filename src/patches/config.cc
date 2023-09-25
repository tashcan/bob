#include "config.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

#include <toml++/toml.h>

#include <spdlog/spdlog.h>

#include <absl/strings/str_split.h>

Config::Config()
{
  Load();
}

Config& Config::Get()
{
  static Config config;
  return config;
}

void Config::Load()
{
  spdlog::info("Loading Config");

  toml::table config;
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

  this->fix_unity_web_requests = config["tech"]["fix_unity_web_requests"].value_or(true);

  this->ui_scale                    = config["graphics"]["ui_scale"].value_or(0.9f);
  this->zoom                        = config["graphics"]["zoom"].value_or(2500.f);
  this->free_resize                 = config["graphics"]["free_resize"].value_or(true);
  this->adjust_scale_res            = config["graphics"]["adjust_scale_res"].value_or(false);
  this->system_pan_momentum_falloff = config["graphics"]["system_pan_momentum_falloff"].value_or(0.8f);
  this->keyboard_zoom_speed         = config["graphics"]["keyboard_zoom_speed"].value_or(350.0f);
  this->borderless_fullscreen_f11   = config["graphics"]["borderless_fullscreen_f11"].value_or(true);
  this->target_framerate            = config["graphics"]["target_framerate"].value_or(60);
  this->vsync                       = config["graphics"]["vsync"].value_or(1);
  this->transition_time             = config["graphics"]["transition_time"].value_or(0.01f);
  this->show_all_resolutions        = config["graphics"]["show_all_resolutions"].value_or(false);
  this->default_system_zoom         = config["graphics"]["default_system_zoom"].value_or(0.0f);

  this->system_zoom_preset_1 = config["graphics"]["system_zoom_preset_1"].value_or(0.0f);
  this->system_zoom_preset_2 = config["graphics"]["system_zoom_preset_2"].value_or(0.0f);
  this->system_zoom_preset_3 = config["graphics"]["system_zoom_preset_3"].value_or(0.0f);
  this->system_zoom_preset_4 = config["graphics"]["system_zoom_preset_4"].value_or(0.0f);
  this->system_zoom_preset_5 = config["graphics"]["system_zoom_preset_5"].value_or(0.0f);

  this->hotkeys_enabled     = config["control"]["hotkeys_enabled"].value_or(true);
  this->use_scopely_hotkeys = config["control"]["use_scopely_hotkeys"].value_or(false);

  this->use_out_of_dock_power = config["buffs"]["use_out_of_dock_power"].value_or(false);

  this->disable_toast_banners  = config["ui"]["disable_toast_banners"].value_or(true);
  this->extend_donation_slider = config["ui"]["extend_donation_slider"].value_or(false);
  this->disable_galaxy_chat    = config["ui"]["disable_galaxy_chat"].value_or(false);
  this->show_cargo_default     = config["ui"]["show_cargo_default"].value_or(false);
  this->show_player_cargo      = config["ui"]["show_player_cargo"].value_or(false);
  this->show_station_cargo     = config["ui"]["show_station_cargo"].value_or(true);
  this->show_hostile_cargo     = config["ui"]["show_hostile_cargo"].value_or(true);
  this->show_armada_cargo      = config["ui"]["show_armada_cargo"].value_or(true);

  this->always_skip_reveal_sequence  = config["ui"]["always_skip_reveal_sequence"].value_or(false);
  this->stay_in_bundle_after_summary = config["ui"]["stay_in_bundle_after_summary"].value_or(true);

  this->sync_host = config["sync"]["host"].value_or("");
  this->sync_port = config["sync"]["port"].value_or(80);
  this->sync_token = config["sync"]["token"].value_or("");

  std::string disabled_banner_types_str = config["ui"]["disabled_banner_types"].value_or("");

  std::vector<std::string> types = absl::StrSplit(disabled_banner_types_str, ",", absl::SkipWhitespace());
  for (const absl::string_view _type : types) {
    auto type = absl::StripLeadingAsciiWhitespace(_type);
    if (type == "Standard") {
      this->disabled_banner_types.emplace_back(ToastState::Standard);
    } else if (type == "FactionWarning") {
      this->disabled_banner_types.emplace_back(ToastState::FactionWarning);
    } else if (type == "FactionLevelUp") {
      this->disabled_banner_types.emplace_back(ToastState::FactionLevelUp);
    } else if (type == "FactionLevelDown") {
      this->disabled_banner_types.emplace_back(ToastState::FactionLevelDown);
    } else if (type == "FactionDiscovered") {
      this->disabled_banner_types.emplace_back(ToastState::FactionDiscovered);
    }
    /* else if (type == "IncomingAttack") {
        this->disabled_banner_types.emplace_back(ToastState::IncomingAttack);
    } */
    else if (type == "IncomingAttackFaction") {
      this->disabled_banner_types.emplace_back(ToastState::IncomingAttackFaction);
    } else if (type == "FleetBattle") {
      this->disabled_banner_types.emplace_back(ToastState::FleetBattle);
    } /*else if (type == "StationBattle") {
        this->disabled_banner_types.emplace_back(ToastState::StationBattle);
    } else if (type == "StationVictory") {
        this->disabled_banner_types.emplace_back(ToastState::StationVictory);
    } else if (type == "StationDefeat") {
        this->disabled_banner_types.emplace_back(ToastState::StationDefeat);
    } */
    else if (type == "Victory") {
      this->disabled_banner_types.emplace_back(ToastState::Victory);
    } else if (type == "Defeat") {
      this->disabled_banner_types.emplace_back(ToastState::Defeat);
    } else if (type == "Event") {
      this->disabled_banner_types.emplace_back(ToastState::Tournament);
    } else if (type == "ArmadaCreated") {
      this->disabled_banner_types.emplace_back(ToastState::ArmadaCreated);
    } else if (type == "ArmadaCanceled") {
      this->disabled_banner_types.emplace_back(ToastState::ArmadaCanceled);
    } else if (type == "ArmadaIncomingAttack") {
      this->disabled_banner_types.emplace_back(ToastState::ArmadaIncomingAttack);
    } else if (type == "ArmadaBattleWon") {
      this->disabled_banner_types.emplace_back(ToastState::ArmadaBattleWon);
    } else if (type == "ArmadaBattleLost") {
      this->disabled_banner_types.emplace_back(ToastState::ArmadaBattleLost);
    } else if (type == "DiplomacyUpdated") {
      this->disabled_banner_types.emplace_back(ToastState::DiplomacyUpdated);
    } else if (type == "JoinedTakeover") {
      this->disabled_banner_types.emplace_back(ToastState::JoinedTakeover);
    } else if (type == "CompetitorJoinedTakeover") {
      this->disabled_banner_types.emplace_back(ToastState::CompetitorJoinedTakeover);
    } else if (type == "AbandonedTerritory") {
      this->disabled_banner_types.emplace_back(ToastState::AbandonedTerritory);
    } else if (type == "TakeoverVictory") {
      this->disabled_banner_types.emplace_back(ToastState::TakeoverVictory);
    } else if (type == "TakeoverDefeat") {
      this->disabled_banner_types.emplace_back(ToastState::TakeoverDefeat);
    }
  }

  if (!std::filesystem::exists("community_patch_settings.toml")) {
    std::ofstream config_file;
    config_file.open("community_patch_settings.toml");
    config_file << config;
    config_file.close();
  }
}