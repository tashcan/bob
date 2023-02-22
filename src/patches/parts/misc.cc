#include "config.h"
#include "prime_types.h"

#include <spud/detour.h>

#include "utils.h"

#include "prime/BundleDataWidget.h"
#include "prime/ClientModifierType.h"
#include "prime/IList.h"
#include "prime/InventoryForPopup.h"

#include <il2cpp/il2cpp_helper.h>

#include <chrono>
#include <iostream>

int64_t InventoryForPopup_set_MaxItemsToUse(auto original, InventoryForPopup* a1, int64_t a2)
{
  auto d = a1->IsDonationUse;
  if (a1->IsDonationUse && a2 == 50 && Config::Get().extend_donation_slider) {
    return 0;
  }
  return original(a1, a2);
}

void BundleDataWidget_OnActionButtonPressedCallback(auto original, BundleDataWidget* _this)
{
  if (_this->CurrentState & BundleDataWidget::ItemState::CooldownTimerOn) {
    _this->AuxViewButtonPressedHandler();
  } else {
    original(_this);
  }
}

void InstallMiscPatches()
{
  auto h   = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Inventories", "InventoryForPopup");
  auto ptr = h.GetMethodXor("set_MaxItemsToUse");
  if (!ptr) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr, InventoryForPopup_set_MaxItemsToUse);

  auto bundle_data_widget = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Shop", "BundleDataWidget");
  ptr                     = bundle_data_widget.GetMethod("OnActionButtonPressedCallback");
  if (!ptr) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr, BundleDataWidget_OnActionButtonPressedCallback);
}

struct Resolution {
  int m_Width;
  int m_Height;
  int m_RefreshRate;

  bool operator==(const Resolution& other) const
  {
    return this->m_Height == other.m_Height && this->m_Width == other.m_Width;
  }
};

struct ResolutionArray {
  Il2CppObject obj;
  void*        bounds;
  size_t       maxlength;
  Resolution   data[1];
};

#include <algorithm>

ResolutionArray* GetResolutions_Hook(auto original)
{
  auto resolutions = original();

  // Modify
  auto screenWidth  = GetSystemMetrics(SM_CXSCREEN);
  auto screenHeight = GetSystemMetrics(SM_CYSCREEN);

  int targetRefreshRate = 0;
  for (int i = 0; i < resolutions->maxlength; ++i) {
    auto ores = resolutions->data[i];
    if (ores.m_Width == screenWidth && ores.m_Height == screenHeight) {
      targetRefreshRate = std::max(ores.m_RefreshRate, targetRefreshRate);
    }
  }

  std::vector<Resolution> res;
  for (int i = 0; i < resolutions->maxlength; ++i) {
    if (Config::Get().show_all_resolutions)
      resolutions->data[i].m_RefreshRate = targetRefreshRate;

    auto ores = resolutions->data[i];
    if (Config::Get().show_all_resolutions || (ores.m_RefreshRate == targetRefreshRate || targetRefreshRate == 0)) {
      res.push_back(ores);
    }
  }

  res.erase(unique(res.begin(), res.end()), res.end());

  int i = 0;
  for (const auto& resultRes : res) {
    resolutions->data[i] = resultRes;
    ++i;
  }
  resolutions->maxlength = res.size();

  return resolutions;
}

void InstallResolutionListFix()
{
  SPUD_STATIC_DETOUR(il2cpp_resolve_icall<ResolutionArray*()>("UnityEngine.Screen::get_resolutions()"),
                     GetResolutions_Hook);
}

IList* ExtractBuffsOfType_Hook(auto original, ClientModifierType modifier, IList* list)
{

  if (list) {
    for (int i = 0; i < list->Count; ++i) {
      auto item = list->Get(i);
      if (item == 0) {
        return nullptr;
      }
      item = item;
    }
  }
  return original(modifier, list);
}

void InstallTempCrashFixes()
{
  auto BuffService_helper =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Services", "BuffService");

  auto ptr_extract_buffs_of_type = BuffService_helper.GetMethod(xorstr_("ExtractBuffsOfType"));
  SPUD_STATIC_DETOUR(ptr_extract_buffs_of_type, ExtractBuffsOfType_Hook);
}