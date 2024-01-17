#include "prime_types.h"

#include <spud/detour.h>

#include "config.h"
#include "utils.h"

#include "prime/BlurController.h"
#include "prime/CallbackContainer.h"
#include "prime/ChatManager.h"
#include "prime/ChatMessageListLocalViewController.h"
#include "prime/ClientModifierType.h"
#include "prime/FleetBarViewController.h"
#include "prime/FleetLocalViewController.h"
#include "prime/FullScreenChatViewController.h"
#include "prime/Hub.h"
#include "prime/InventoryForPopup.h"
#include "prime/KeyCode.h"
#include "prime/MiningObjectViewerWidget.h"
#include "prime/NavigationInteractionUIViewController.h"
#include "prime/ScanEngageButtonsWidget.h"
#include "prime/ScreenManager.h"
#include "prime/AllianceStarbaseObjectViewerWidget.h"
#include "prime/PreScanTargetWidget.h"
#include "prime/ActionRequirement.h"
#include "prime/AllianceStarbaseObjectViewerWidget.h"
#include "prime/AnimatedRewardsScreenViewController.h"
#include "prime/ArmadaObjectViewerWidget.h"
#include "prime/BookmarksManager.h"
#include "prime/CelestialObjectViewerWidget.h"
#include "prime/ChatManager.h"
#include "prime/ChatMessageListLocalViewController.h"
#include "prime/DeploymentManager.h"
#include "prime/EmbassyObjectViewer.h"
#include "prime/FleetBarViewController.h"
#include "prime/FleetLocalViewController.h"
#include "prime/FleetsManager.h"
#include "prime/FullScreenChatViewController.h"
#include "prime/HousingObjectViewerWidget.h"
#include "prime/Hub.h"
#include "prime/KeyCode.h"
#include "prime/MiningObjectViewerWidget.h"
#include "prime/MissionsObjectViewerWidget.h"
#include "prime/NavigationInteractionUIViewController.h"
#include "prime/NavigationSectionManager.h"
#include "prime/PreScanTargetWidget.h"
#include "prime/ScanEngageButtonsWidget.h"
#include "prime/ScanTargetViewController.h"
#include "prime/SceneManager.h"
#include "prime/ScreenManager.h"
#include "prime/StarNodeObjectViewerWidget.h"

#include <il2cpp/il2cpp_helper.h>

#include <EASTL/unordered_map.h>
#include <EASTL/vector.h>
#include <EASTL/unordered_set.h>

#include <chrono>
#include <iostream>

static int i = 0;

bool                                     BlurController_IsTweeningBlur(BlurController* _this);
decltype(BlurController_IsTweeningBlur)* oIsTweeningBlur = nullptr;
bool                                     BlurController_IsTweeningBlur(BlurController* _this3)
{
  return false;
}

int64_t                       BlurControllerCtor(__int64 a1, float a2, float a3, __int64 a4, __int64 a5, __int64 a6);
decltype(BlurControllerCtor)* oBlurControllerCtor = nullptr;
int64_t                       BlurControllerCtor(__int64 a1, float a2, float a3, __int64 a4, __int64 a5, __int64 a6)
{
  return oBlurControllerCtor(a1, a2, a3, a4, a5, a6);
}

__int64                    raise_exception(__int64 a1, __int64 a2, __int64 a3, __int64 a4);
decltype(raise_exception)* oraise_exception = nullptr;
__int64                    raise_exception(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
{
  return oraise_exception(a1, a2, a3, a4);
}

void                                   OnDidChangeSelectedTab_Hook(FullScreenChatViewController* _this);
decltype(OnDidChangeSelectedTab_Hook)* oOnDidChangeSelectedTab = nullptr;
void                                   OnDidChangeSelectedTab_Hook(FullScreenChatViewController* _this)
{
  oOnDidChangeSelectedTab(_this);

  if (_this->_messageList->_inputField) {
    _this->_messageList->_inputField->ActivateInputField();
  }
}

void                             ShopListDirector_Hook(void* _this, void* a1);
decltype(ShopListDirector_Hook)* oShopListDirector = nullptr;
void                             ShopListDirector_Hook(void* _this, void* a1)
{
  return oShopListDirector(_this, a1);
}

void                                 FactionsListDirector_Hook(void* _this, void* a1, void* a2);
decltype(FactionsListDirector_Hook)* oFactionsListDirector = nullptr;
void                                 FactionsListDirector_Hook(void* _this, void* a1, void* a2)
{
  return oFactionsListDirector(_this, a1, a2);
}

// OnBundlesReceived

//
// void
// ChatMessageListLocalViewController_LateUpdate_Hook(ChatMessageListLocalViewController*
// _this); decltype(ChatMessageListLocalViewController_LateUpdate_Hook)*
// oChatMessageListLocalViewController_LateUpdate = nullptr; void
// ChatMessageListLocalViewController_LateUpdate_Hook(ChatMessageListLocalViewController*
// _this)
//{
//	oChatMessageListLocalViewController_LateUpdate(_this);
//	static auto cat = _this->CanvasContext->_category;
//	auto n = _this->CanvasContext->_category;
//	n = n;
//	if (cat != _this->CanvasContext->_category && _this->_inputField) {
//		_this->_inputField->ActivateInputField();
//		cat = _this->CanvasContext->_category;
//	}
//}

LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter_Hook(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
  return nullptr;
  return SetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
}

void RequestDispatcherBase_SetDefaultHeader(void* _this, Il2CppString* key, Il2CppString* value);
decltype(RequestDispatcherBase_SetDefaultHeader)* oRequestDispatcherBase_SetDefaultHeader = nullptr;
void RequestDispatcherBase_SetDefaultHeader(void* _this, Il2CppString* key, Il2CppString* value)
{
  static auto il2cpp_string_new =
      (il2cpp_string_new_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_string_new"));
  oRequestDispatcherBase_SetDefaultHeader(_this, key, value);
}

void                             Chat_handleNewMessage(void* _this, Il2CppString* message);
decltype(Chat_handleNewMessage)* oChatMessage = nullptr;

void Chat_handleNewMessage(void* _this, Il2CppString* message)
{
  wprintf(L"%s\n", message->chars);
  return oChatMessage(_this, message);
  // original(_this, message);
}

class AppConfig
{
public:
  __declspec(property(get = __get_PlatformSettingsUrl,
                      put = __set_PlatformSettingsUrl)) Il2CppString* PlatformSettingsUrl;
  __declspec(property(get = __get_PlatformApiKey, put = __set_PlatformApiKey)) Il2CppString* PlatformApiKey;
  __declspec(property(get = __get_AssetUrlOverride, put = __set_AssetUrlOverride)) Il2CppString* AssetUrlOverride;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.Core", "AppConfig");
    return class_helper;
  }

public:
  Il2CppString* __get_PlatformSettingsUrl()
  {
    static auto prop = get_class_helper().GetProperty("PlatformSettingsUrl");
    return prop.GetRaw<Il2CppString>((void*)this);
  }

  void __set_PlatformSettingsUrl(Il2CppString* v)
  {
    static auto prop = get_class_helper().GetProperty("PlatformSettingsUrl");
    return prop.SetRaw((void*)this, *v);
  }

  Il2CppString* __get_PlatformApiKey()
  {
    static auto prop = get_class_helper().GetProperty("PlatformApiKey");
    return prop.GetRaw<Il2CppString>((void*)this);
  }

  void __set_PlatformApiKey(Il2CppString* v)
  {
    static auto prop = get_class_helper().GetProperty("PlatformApiKey");
    return prop.SetRaw((void*)this, *v);
  }

  Il2CppString* __get_AssetUrlOverride()
  {
    static auto prop = get_class_helper().GetProperty("AssetUrlOverride");
    return prop.GetRaw<Il2CppString>((void*)this);
  }

  void __set_AssetUrlOverride(Il2CppString* v)
  {
    static auto prop = get_class_helper().GetProperty("AssetUrlOverride");
    return prop.SetRaw((void*)this, *v);
  }
};

class Model
{
public:
  __declspec(property(get = __get_AppConfig)) AppConfig* AppConfig_;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.Core", "Model");
    return class_helper;
  }

public:
  AppConfig* __get_AppConfig()
  {
    static auto field = get_class_helper().GetField("_appConfig");
    return *(AppConfig**)((ptrdiff_t)this + field.offset());
  }
};

void* AppConfig_LoadConfig(auto original)
{
  auto app_config = original();
  return app_config;
}

AppConfig* Model_LoadConfigs(auto original, Model* _this)
{
  original(_this);
  auto config = _this->AppConfig_;

  static auto il2cpp_string_new_utf16 = (il2cpp_string_new_utf16_t)(GetProcAddress(
      GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_string_new_utf16")));
  static auto il2cpp_string_new =
      (il2cpp_string_new_t)(GetProcAddress(GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_string_new")));

  if (!Config::Get().config_settings_url.empty()) {
    auto new_settings_url       = il2cpp_string_new(Config::Get().config_settings_url.c_str());
    config->PlatformSettingsUrl = new_settings_url;
  }

  if (!Config::Get().config_assets_url_override.empty()) {
    auto new_url             = il2cpp_string_new(Config::Get().config_assets_url_override.c_str());
    config->AssetUrlOverride = new_url;
  }

  return config;
}

eastl::unordered_map<Il2CppClass*, eastl::vector<uintptr_t>> tracked_objects;

void* track_ctor(auto original, void* _this) {
  auto cls = (Il2CppObject*)_this;
  auto &tracked_object_vector = tracked_objects[cls->klass];
  tracked_object_vector.emplace_back(uintptr_t(_this));
  return original(_this);
}

void track_destroy(auto original, void* _this)
{
  auto cls = (Il2CppObject*)_this;
  auto& tracked_object_vector = tracked_objects[cls->klass];
  tracked_object_vector.erase_first(uintptr_t(_this));
  original(_this);
}

template<typename T> void TrackObject() {
  static eastl::unordered_set<void*> seen_ctor;
  static eastl::unordered_set<void*> seen_destroy;

  auto &alliance_widget = T::get_class_helper();
  auto ctor       = alliance_widget.GetMethod(".ctor");
  auto on_destroy = alliance_widget.GetMethod("OnDestroy");

  if (seen_ctor.find(ctor) == eastl::end(seen_ctor)) {
    SPUD_STATIC_DETOUR(ctor, track_ctor);
    seen_ctor.emplace(ctor);
  }

  if (seen_destroy.find(ctor) == eastl::end(seen_destroy)) {
    SPUD_STATIC_DETOUR(on_destroy, track_destroy);
    seen_destroy.emplace(on_destroy);
  }
}


void InstallTestPatches()
{
  auto app_config      = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.Core", "AppConfig");
  auto load_config_ptr = app_config.GetMethod("LoadConfig");
  SPUD_STATIC_DETOUR(load_config_ptr, AppConfig_LoadConfig);

  auto model            = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.Core", "Model");
  auto load_configs_ptr = model.GetMethod("LoadConfigs");
  SPUD_STATIC_DETOUR(load_configs_ptr, Model_LoadConfigs);

  auto battle_target_data =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Models", "BattleTargetData");
  battle_target_data = battle_target_data;

  TrackObject<HousingObjectViewerWidget>();
  TrackObject<AllianceStarbaseObjectViewerWidget>();
  TrackObject<ArmadaObjectViewerWidget>();
  TrackObject<CelestialObjectViewerWidget>();
  TrackObject<EmbassyObjectViewer>();
  TrackObject<MiningObjectViewerWidget>();
  TrackObject<MissionsObjectViewerWidget>();
  TrackObject<PreScanTargetWidget>();
  TrackObject<HousingObjectViewerWidget>();
  TrackObject<FleetBarViewController>();
}