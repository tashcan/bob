#include "config.h"
#include "prime_types.h"

#include "mapkey.h"
#include "utils.h"
#include <spdlog/spdlog.h>
#include <spud/detour.h>

#include <il2cpp/il2cpp_helper.h>

#include "prime/EventSystem.h"
#include "prime/Hub.h"
#include "prime/KeyCode.h"
#include "prime/NavigationPan.h"
#include "prime/NavigationZoom.h"
#include "prime/ScreenManager.h"
#include "prime/TMP_InputField.h"

vec3 GetMouseWorldPos(void *cam, vec3 *pos)
{
  static auto il2cpp_runtime_invoke =
      (il2cpp_runtime_invoke_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_runtime_invoke"));
  static auto class_helper = il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.Client.Core", "MathUtils");
  static auto il2cpp_class_get_method_from_name = (il2cpp_class_get_method_from_name_t)(GetProcAddress(
      GetModuleHandle("GameAssembly.dll"), "il2cpp_class_get_method_from_name"));
  static auto fn                                = class_helper.GetMethodInfo("GetMouseWorldPos");
  static auto il2cpp_object_unbox =
      (il2cpp_object_unbox_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_object_unbox"));

  void            *args[2]   = {cam, (void *)pos};
  Il2CppException *exception = NULL;
  auto             result    = il2cpp_runtime_invoke(fn, nullptr, args, &exception);
  return *(vec3 *)(il2cpp_object_unbox(result));
}

auto do_default_zoom = false;

inline void StoreZoom(std::string label, float &zoom, NavigationZoom *_this)
{
  auto old_zoom = zoom;
  zoom          = (_this->Distance - _this->_minimum) / (_this->_maximum - _this->_minimum) * Config::Get().zoom;
  spdlog::info("Changing {} from {} to {}", label, old_zoom, zoom);
}

void NavigationZoom_Update_Hook(auto original, NavigationZoom *_this)
{
  static auto il2cpp_string_new =
      (il2cpp_string_new_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_string_new"));
  static auto GetMousePosition =
      il2cpp_resolve_icall<void(vec3 *)>("UnityEngine.Input::get_mousePosition_Injected(UnityEngine.Vector3&)");
  static auto GetDeltaTime = il2cpp_resolve_icall<float()>("UnityEngine.Time::get_deltaTime()");

  const auto dt               = GetDeltaTime();
  auto       zoomDelta        = 0.0f;
  bool       do_absolute_zoom = false;
  auto       config           = &Config::Get();

  if (!Key::IsInputFocused()) {
    if (MapKey::IsDown(GameFunction::SetZoomPreset1)) {
      return StoreZoom("System Preset 1", config->system_zoom_preset_1, _this);
    } else if (MapKey::IsDown(GameFunction::SetZoomPreset2)) {
      return StoreZoom("System Preset 2", config->system_zoom_preset_2, _this);
    } else if (MapKey::IsDown(GameFunction::SetZoomPreset3)) {
      return StoreZoom("System Preset 3", config->system_zoom_preset_3, _this);
    } else if (MapKey::IsDown(GameFunction::SetZoomPreset4)) {
      return StoreZoom("System Preset 4", config->system_zoom_preset_4, _this);
    } else if (MapKey::IsDown(GameFunction::SetZoomPreset5)) {
      return StoreZoom("System Preset 5", config->system_zoom_preset_5, _this);
    } else if (MapKey::IsDown(GameFunction::SetZoomDefault)) {
      return StoreZoom("System Default", config->default_system_zoom, _this);
    }

    do_absolute_zoom = true;
    if (MapKey::IsDown(GameFunction::ZoomPreset1)) {
      zoomDelta = config->system_zoom_preset_1;
    } else if (MapKey::IsDown(GameFunction::ZoomPreset2)) {
      zoomDelta = config->system_zoom_preset_2;
    } else if (MapKey::IsDown(GameFunction::ZoomPreset3)) {
      zoomDelta = config->system_zoom_preset_3;
    } else if (MapKey::IsDown(GameFunction::ZoomPreset4)) {
      zoomDelta = config->system_zoom_preset_4;
    } else if (MapKey::IsDown(GameFunction::ZoomPreset5)) {
      zoomDelta = config->system_zoom_preset_5;
    }

    if (config->hotkeys_extended) {
      if (MapKey::IsDown(GameFunction::ZoomReset)) {
        do_absolute_zoom = false;
        do_default_zoom  = true;
      } else if (MapKey::IsDown(GameFunction::ZoomMin)) {
        zoomDelta = config->zoom;
      } else if (MapKey::IsDown(GameFunction::ZoomMax)) {
        zoomDelta = 100;
      }
    }

    if (do_default_zoom) {
      do_absolute_zoom = true;
      zoomDelta        = config->default_system_zoom;
    }

    if (zoomDelta == 0.0f) {
      do_absolute_zoom = false;
      zoomDelta        = config->keyboard_zoom_speed * dt;
    }

    if (MapKey::IsPressed(GameFunction::ZoomIn) || do_absolute_zoom) {
      vec3 mousePos;
      GetMousePosition(&mousePos);
      _this->_zoomLocation = vec2{mousePos.x, mousePos.y};
      if (do_absolute_zoom) {
        auto zoom_distance = _this->_minimum + (_this->_maximum - _this->_minimum) * (zoomDelta / config->zoom);
        _this->Distance    = zoom_distance;
      } else {
        _this->_zoomDelta     = zoomDelta;
        _this->_lastZoomDelta = zoomDelta;
      }
      auto worldPos      = GetMouseWorldPos(_this->_sceneCamera, &mousePos);
      _this->_worldPoint = worldPos;
      _this->ZoomCameraAtWorldPoint();
    } else if (MapKey::IsPressed(GameFunction::ZoomOut) && !Key::IsInputFocused()) {
      vec3 mousePos;
      GetMousePosition(&mousePos);
      _this->_zoomLocation  = vec2{mousePos.x, mousePos.y};
      _this->_zoomDelta     = -1.0f * zoomDelta;
      _this->_lastZoomDelta = -1.0f * zoomDelta;
      auto worldPos         = GetMouseWorldPos(_this->_sceneCamera, &mousePos);
      _this->_worldPoint    = worldPos;
      _this->ZoomCameraAtWorldPoint();
    }
  }

  if (zoomDelta > 0.0f && config->use_presets_as_default) {
    if (!do_default_zoom && do_absolute_zoom) {
      StoreZoom("System Preset Default from Preset", config->default_system_zoom, _this);
    }
  }

  do_default_zoom = false;

  original(_this);
}

void NavigationZoom_SetViewParameters_Hook(auto original, NavigationZoom *_this, float radius, NodeDepth depth)
{
  if (depth == NodeDepth::SolarSystem) {
    auto ratio                     = (Config::Get().zoom / radius);
    _this->_farRatioSystemNormal   = 0.55f * ratio;
    _this->_farRatioSystemExtended = 1 * ratio;
    original(_this, radius, depth);
    _this->_sceneCamera->farClipPlane = Config::Get().zoom * 2.75f;
    do_default_zoom                   = true;
  } else {
    original(_this, radius, depth);
  }
}

void InstallZoomHooks()
{
  auto screen_manager_helper   = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Navigation", "NavigationZoom");
  auto ptr_set_view_parameters = screen_manager_helper.GetMethod("SetViewParameters");
  auto ptr_update              = screen_manager_helper.GetMethod("Update");
  if (!ptr_set_view_parameters || !ptr_update) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr_update, NavigationZoom_Update_Hook);
  SPUD_STATIC_DETOUR(ptr_set_view_parameters, NavigationZoom_SetViewParameters_Hook);
}
