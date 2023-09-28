#include "config.h"
#include "prime_types.h"

#include <spud/detour.h>

#include "utils.h"

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

void NavigationZoom_Update_Hook(auto original, NavigationZoom *_this)
{
  if ((NodeDepth)_this->_depth == NodeDepth::SolarSystem) {
    // _this->SetViewParameters(_this->_viewRadius, NodeDepth::SolarSystem);
  }

  static auto il2cpp_string_new =
      (il2cpp_string_new_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_string_new"));
  static auto GetKeyInt = il2cpp_resolve_icall<bool(KeyCode)>("UnityEngine.Input::GetKeyInt(UnityEngine.KeyCode)");
  static auto GetKeyDownInt =
      il2cpp_resolve_icall<bool(KeyCode)>("UnityEngine.Input::GetKeyDownInt(UnityEngine.KeyCode)");
  static auto GetMousePosition =
      il2cpp_resolve_icall<void(vec3 *)>("UnityEngine.Input::get_mousePosition_Injected(UnityEngine.Vector3&)");
  static auto GetDeltaTime = il2cpp_resolve_icall<float()>("UnityEngine.Time::get_deltaTime()");

  auto       section_manager = Hub::get_SectionManager();
  const auto current_section = section_manager->CurrentSection;

  const auto is_in_chat = current_section == SectionID::Chat_Private_Message
                          || current_section == SectionID::Chat_Alliance || current_section == SectionID::Chat_Main
                          || current_section == SectionID::Chat_Private_List;

  const auto is_in_system_galaxy =
      current_section == SectionID::Navigation_Galaxy || current_section == SectionID::Navigation_System
      || current_section == SectionID::Starbase_Interior || current_section == SectionID::Starbase_Exterior;

  const auto dt        = GetDeltaTime();
  auto       zoomDelta = Config::Get().keyboard_zoom_speed * dt;

  if (is_in_chat) {
    return original(_this);
    ;
  }

  auto is_input_focused = []() {
    bool is_input_focused = false;
    auto eventSystem      = EventSystem::current();
    if (eventSystem) {
      auto n = eventSystem->currentSelectedGameObject;
      if (!n) {
        return false;
      }
      try {
        if (n) {
          auto n2 = n->GetComponentFastPath2<TMP_InputField>();
          if (n2) {
            return n2->isFocused;
          }
        }
      } catch (...) {
        return false;
      }
    }
    return false;
  };

  bool do_absolute_zoom = true;
  if (GetKeyDownInt(KeyCode::F1) && !is_input_focused()) {
    zoomDelta = Config::Get().system_zoom_preset_1;
  } else if (GetKeyDownInt(KeyCode::F2) && !is_input_focused()) {
    zoomDelta = Config::Get().system_zoom_preset_2;
  } else if (GetKeyDownInt(KeyCode::F3) && !is_input_focused()) {
    zoomDelta = Config::Get().system_zoom_preset_3;
  } else if (GetKeyDownInt(KeyCode::F4) && !is_input_focused()) {
    zoomDelta = Config::Get().system_zoom_preset_4;
  } else if (GetKeyDownInt(KeyCode::F5) && !is_input_focused()) {
    zoomDelta = Config::Get().system_zoom_preset_5;
  } else {
    do_absolute_zoom = false;
  }

  bool is_shift_pressed = GetKeyInt(KeyCode::LeftShift) || GetKeyInt(KeyCode::RightShift);
  if (GetKeyDownInt(KeyCode::Equals) && !is_input_focused()) {
    if (Config::Get().default_system_zoom > 0.0f) {
      zoomDelta = Config::Get().default_system_zoom;
      do_absolute_zoom = true;
      do_default_zoom = false;
    }
  } else if (GetKeyDownInt(KeyCode::Minus) && !is_input_focused()) {
    zoomDelta = Config::Get().zoom;
    do_absolute_zoom = true;
    do_default_zoom = false;
  } else if (GetKeyDownInt(KeyCode::Backspace) && !is_input_focused()) {
    zoomDelta = 100;
    do_absolute_zoom = true;
    do_default_zoom = false;
  }

  if (do_default_zoom) {
    if (Config::Get().default_system_zoom > 0.0f) {
      do_absolute_zoom = true;
      zoomDelta        = Config::Get().default_system_zoom;
    }
    do_default_zoom = false;
  }

  if ((GetKeyInt(KeyCode::Q) && !is_input_focused()) || do_absolute_zoom) {
    vec3 mousePos;
    GetMousePosition(&mousePos);
    _this->_zoomLocation = vec2{mousePos.x, mousePos.y};
    if (do_absolute_zoom) {
      auto minMaxRange   = (_this->_maximum - _this->_minimum);
      auto percentage    = (zoomDelta / Config::Get().zoom);
      auto zoom_distance = _this->_minimum + (_this->_maximum - _this->_minimum) * (zoomDelta / Config::Get().zoom);
      _this->Distance    = zoom_distance;
    } else {
      _this->_zoomDelta     = zoomDelta;
      _this->_lastZoomDelta = zoomDelta;
    }
    auto worldPos      = GetMouseWorldPos(_this->_sceneCamera, &mousePos);
    _this->_worldPoint = worldPos;
    _this->ZoomCameraAtWorldPoint();
  } else if (GetKeyInt(KeyCode::E) && !is_input_focused()) {
    vec3 mousePos;
    GetMousePosition(&mousePos);
    _this->_zoomLocation  = vec2{mousePos.x, mousePos.y};
    _this->_zoomDelta     = -1.0f * zoomDelta;
    _this->_lastZoomDelta = -1.0f * zoomDelta;
    auto worldPos         = GetMouseWorldPos(_this->_sceneCamera, &mousePos);
    _this->_worldPoint    = worldPos;
    _this->ZoomCameraAtWorldPoint();
  } else {
    original(_this);
  }
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