#include "config.h"
#include "prime_types.h"
#include "utils.h"

#include <Windows.h>

#include <iostream>

#include <spud/detour.h>

#include <il2cpp/il2cpp_helper.h>

#include "prime/CanvasScaler.h"
#include "prime/ScreenManager.h"

#include <algorithm>

#include "spdlog/spdlog.h"

void SetResolution_Hook(auto original, int x, int y, int mode, int unk)
{
  spdlog::trace("Setting resoltuion {} x {}", x, y);
  return original(x, y, mode, unk);
}

void ScreenManager_UpdateCanvasRootScaleFactor_Hook(auto original, ScreenManager* _this)
{
  original(_this);

  if (Config::Get().ui_scale != 0.0f) {
    static auto get_height_method = il2cpp_resolve_icall<int()>("UnityEngine.Screen::get_height()");
    static auto get_width_method  = il2cpp_resolve_icall<int()>("UnityEngine.Screen::get_width()");

    static auto ref_height = 1080;
    static auto ref_width  = 1920;

    auto scr_height = (float)get_height_method();
    auto scr_width  = (float)get_width_method();
    auto dpi        = Config::GetDPI();

    auto adjustedFactor = scr_height / (float)ref_height;

    if (!Config::Get().adjust_scale_res) {
      adjustedFactor = 1.0f;
    }

    auto n = (Config::Get().ui_scale * adjustedFactor * dpi);
    if (isnan(n)) {
      n = 1.0f;
    }
    n = std::clamp(n, 0.1f, 5.0f);

    _this->m_canvasRootScaler->scaleFactor = n;
  }
}

BOOL SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
  spdlog::trace("Window size/position {} (x) {} (y) {} (cx) {} (cy)", X, Y, cx, cy);
  return SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

void InstallUiScaleHooks()
{
  auto screen_manager_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.UI", "ScreenManager");
  auto ptr_update_scale      = screen_manager_helper.GetMethod("UpdateCanvasRootScaleFactor");
  if (!ptr_update_scale) {
    return;
  }

  SPUD_STATIC_DETOUR(ptr_update_scale, ScreenManager_UpdateCanvasRootScaleFactor_Hook);
  static auto SetResolution = il2cpp_resolve_icall<void(int, int, int, int)>(
      "UnityEngine.Screen::SetResolution(System.Int32,System.Int32,UnityEngine.FullScreenMode,System.Int32)");
  SPUD_STATIC_DETOUR(SetResolution, SetResolution_Hook);

  Config::RefreshDPI();
}
