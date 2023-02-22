#pragma once

#include <vector>

#include "prime/Toast.h"

class Config
{
public:
  Config();

  static Config& Get();

  void Load();

public:
  float ui_scale;
  float zoom;
  bool  free_resize;
  bool  adjust_scale_res;
  bool  show_all_resolutions;

  bool  use_out_of_dock_power;
  float system_pan_momentum_falloff;

  float keyboard_zoom_speed;

  bool  hotkeys_enabled;
  bool  use_scopely_hotkeys;
  float default_system_zoom;

  float system_zoom_preset_1;
  float system_zoom_preset_2;
  float system_zoom_preset_3;
  float system_zoom_preset_4;
  float system_zoom_preset_5;

  int32_t target_framerate;
  int32_t vsync;
  float   transition_time;

  bool             borderless_fullscreen_f11;
  std::vector<int> disabled_banner_types;

  bool extend_donation_slider;
  bool disable_galaxy_chat;
  bool disable_toast_banners;
  bool fix_unity_web_requests;

  bool show_cargo_default;
  bool show_player_cargo;
  bool show_station_cargo;
  bool show_hostile_cargo;
  bool show_armada_cargo;
};