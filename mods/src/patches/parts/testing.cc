#include "config.h"

#include "prime/ActionRequirement.h"
#include "prime/AllianceStarbaseObjectViewerWidget.h"
#include "prime/AnimatedRewardsScreenViewController.h"
#include "prime/ArmadaObjectViewerWidget.h"
#include "prime/BlurController.h"
#include "prime/BookmarksManager.h"
#include "prime/CallbackContainer.h"
#include "prime/CelestialObjectViewerWidget.h"
#include "prime/ChatManager.h"
#include "prime/ChatMessageListLocalViewController.h"
#include "prime/ClientModifierType.h"
#include "prime/DeploymentManager.h"
#include "prime/EmbassyObjectViewer.h"
#include "prime/FleetBarViewController.h"
#include "prime/FleetLocalViewController.h"
#include "prime/FleetsManager.h"
#include "prime/FullScreenChatViewController.h"
#include "prime/HousingObjectViewerWidget.h"
#include "prime/Hub.h"
#include "prime/InventoryForPopup.h"
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
#include <prime/UIBehaviour.h>

#include <il2cpp/il2cpp_helper.h>
#include <spud/detour.h>
#include <spud/signature.h>

#include <EASTL/unordered_map.h>
#include <EASTL/unordered_set.h>
#include <EASTL/vector.h>

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

void RequestDispatcherBase_SetDefaultHeader(void* _this, Il2CppString* key, Il2CppString* value);
decltype(RequestDispatcherBase_SetDefaultHeader)* oRequestDispatcherBase_SetDefaultHeader = nullptr;
void RequestDispatcherBase_SetDefaultHeader(void* _this, Il2CppString* key, Il2CppString* value)
{
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

AppConfig* Model_LoadConfigs(auto original, Model* _this)
{
  original(_this);
  auto config = _this->AppConfig_;

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

std::mutex                                                   tracked_objects_mutex;
eastl::unordered_map<Il2CppClass*, eastl::vector<uintptr_t>> tracked_objects;

void add_to_tracking_recursive(Il2CppClass* klass, void* _this)
{
  if (!klass) {
    return;
  }

  auto& tracked_object_vector = tracked_objects[klass];
  tracked_object_vector.emplace_back(uintptr_t(_this));

  return add_to_tracking_recursive(klass->parent, _this);
}

void remove_from_tracking_all(void* _this)
{
#define GET_CLASS(obj) ((Il2CppClass*)(((size_t)obj) & ~(size_t)1))
  for (auto& [klass, tracked_object_vector] : tracked_objects) {
    tracked_object_vector.erase_first(uintptr_t(_this));
  }
#undef GET_CLASS
}

void remove_from_tracking_recursive(Il2CppClass* klass, void* _this)
{
#define GET_CLASS(obj) ((Il2CppClass*)(((size_t)obj) & ~(size_t)1))
  if (!GET_CLASS(klass)) {
    return;
  }

  if (tracked_objects.find(klass) == tracked_objects.end()) {
    return;
  }

  auto& tracked_object_vector = tracked_objects[GET_CLASS(klass->parent)];
  tracked_object_vector.erase_first(uintptr_t(_this));
  return remove_from_tracking_recursive(GET_CLASS(klass->parent), _this);
#undef GET_CLASS
}

void (*GC_register_finalizer_inner)(unsigned __int64 obj, void(__fastcall* fn)(void*, void*), void* cd,
                                    void(__fastcall** ofn)(void*, void*), void** ocd) = nullptr;

void track_finalizer(void* _this, void*)
{
#define GET_CLASS(obj) ((Il2CppClass*)(((size_t)obj) & ~(size_t)1))
  spdlog::trace("Clearing {}({})", (void*)_this, GET_CLASS(((Il2CppObject*)_this)->klass)->name);
  remove_from_tracking_all(_this);
#undef GET_CLASS
}

void* track_ctor(auto original, void* _this)
{
  auto obj = original(_this);
  if (_this == nullptr) {
    return _this;
  }

  std::scoped_lock lk{tracked_objects_mutex};
  auto             cls = (Il2CppObject*)_this;
  spdlog::trace("Tracking {}({})", _this, cls->klass->name);
  typedef void (*FinalizerCallback)(void* object, void* client_data);
  FinalizerCallback oldCallback = nullptr;
  void*             oldData     = nullptr;
  GC_register_finalizer_inner((intptr_t)_this, track_finalizer, nullptr, &oldCallback, &oldData);
  assert(!oldCallback);
  add_to_tracking_recursive(cls->klass, _this);
  return obj;
}

void track_destroy(auto original, Il2CppObject* _this, uint64_t a2, uint64_t a3)
{
#define GET_CLASS(obj) ((Il2CppClass*)(((size_t)obj) & ~(size_t)1))
  if (_this != nullptr) {
    std::scoped_lock lk{tracked_objects_mutex};
    spdlog::trace("Clearing {}({})", (void*)_this, GET_CLASS(_this->klass)->name);
    remove_from_tracking_all(_this);
  }
  return original(_this, a2, a3);
#undef GET_CLASS
}

void track_free(auto original, void* _this)
{
#define GET_CLASS(obj) ((Il2CppClass*)(((size_t)obj) & ~(size_t)1))
  if (_this != nullptr) {
    std::scoped_lock lk{tracked_objects_mutex};
    auto             cls = (Il2CppObject*)_this;
    remove_from_tracking_all(_this);
    return original(_this);
  }
#undef GET_CLASS
}

void calc_liveness_hook(auto original, void* state)
{
  original(state);

  std::scoped_lock                                    lk{tracked_objects_mutex};
  eastl::vector<eastl::pair<Il2CppClass*, uintptr_t>> objects_to_free;
  eastl::unordered_set<uintptr_t>                     objects_seen;
#define IS_MARKED(obj) (((size_t)(obj)->klass) & (size_t)1)
  for (auto& [klass, objects] : tracked_objects) {
    for (auto object : objects) {
      if (IS_MARKED((Il2CppObject*)object) && objects_seen.find(object) == objects_seen.end()) {
        objects_to_free.emplace_back(klass, object);
        objects_seen.emplace(object);
      }
    }
  }

#undef IS_MARKED

#define GET_CLASS(obj) ((Il2CppClass*)(((size_t)obj) & ~(size_t)1))
  for (auto& [klass, object] : objects_to_free) {
    spdlog::trace("Clearing {}({})", (void*)object, GET_CLASS(klass)->name);
    remove_from_tracking_all((void*)object);
  }
#undef GET_CLASS

  tracked_objects = tracked_objects;
}

static eastl::unordered_set<void*> seen_ctor;
static eastl::unordered_set<void*> seen_destroy;

template <typename T> void TrackObject()
{
  auto& object_class = T::get_class_helper();
  auto  ctor         = object_class.GetMethod(".ctor");
  auto  on_destroy   = object_class.GetMethod("OnDestroy");
  if (seen_ctor.find(ctor) == seen_ctor.end()) {
    SPUD_STATIC_DETOUR(ctor, track_ctor);
    seen_ctor.emplace(ctor);
  }

  if (seen_destroy.find(on_destroy) == seen_destroy.end()) {
    SPUD_STATIC_DETOUR(on_destroy, track_destroy);
    seen_destroy.emplace(on_destroy);
  }
}

void SetActive_hook(auto original, void* _this, bool active)
{
  static auto IsActiveSelf = il2cpp_resolve_icall_typed<bool(void*)>("UnityEngine.GameObject::get_activeSelf()");

  if (active && IsActiveSelf(_this)) {
    return;
    // __debugbreak();
  }
  return original(_this, active);
}

void InstallTestPatches()
{
  auto model            = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.Core", "Model");
  auto load_configs_ptr = model.GetMethod("LoadConfigs");
  SPUD_STATIC_DETOUR(load_configs_ptr, Model_LoadConfigs);

  auto battle_target_data =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Models", "BattleTargetData");
  battle_target_data = battle_target_data;

  TrackObject<PreScanTargetWidget>();
  TrackObject<FleetBarViewController>();
  TrackObject<AllianceStarbaseObjectViewerWidget>();
  TrackObject<AnimatedRewardsScreenViewController>();
  TrackObject<ArmadaObjectViewerWidget>();
  TrackObject<CelestialObjectViewerWidget>();
  TrackObject<EmbassyObjectViewer>();
  TrackObject<FullScreenChatViewController>();
  TrackObject<HousingObjectViewerWidget>();
  TrackObject<MiningObjectViewerWidget>();
  TrackObject<MissionsObjectViewerWidget>();
  TrackObject<NavigationInteractionUIViewController>();
  TrackObject<StarNodeObjectViewerWidget>();

  SPUD_STATIC_DETOUR(il2cpp_unity_liveness_finalize, calc_liveness_hook);

  static auto SetActive =
      il2cpp_resolve_icall_typed<void(void*, bool)>("UnityEngine.GameObject::SetActive(System.Boolean)");
  SPUD_STATIC_DETOUR(SetActive, SetActive_hook);

#if _WIN32
  auto GC_register_finalizer_inner_matches =
      spud::find_in_module("40 56 57 41 57 48 83 EC ? 83 3D", "GameAssembly.dll");
#else
  auto GC_register_finalizer_inner_matches = spud::find_in_module(
      "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC ? 4C 89 45 ? 48 89 4D ? 83 3D", "GameAssembly.dylib");
#endif
  auto GC_register_finalizer_inner_matche = GC_register_finalizer_inner_matches.get(0);
  GC_register_finalizer_inner = (decltype(GC_register_finalizer_inner))GC_register_finalizer_inner_matche.address();
}
