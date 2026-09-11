#include "fc_confirmation_reset.h"

#include <il2cpp-tabledefs.h>
#include <il2cpp/il2cpp_helper.h>
#include <spdlog/spdlog.h>

namespace
{

// Read the existing singleton rather than calling Instance, which can instantiate one during login/reload.
Il2CppObject* ExistingSingleton(IL2CppClassHelper& helper)
{
  auto  parent = helper.GetParent("MonoSingleton`1");
  auto* cls    = parent.get_cls();
  auto* field  = cls ? il2cpp_class_get_field_from_name(cls, "s_instance") : nullptr;
  if (!field || !(field->type->attrs & FIELD_ATTRIBUTE_STATIC))
    return nullptr;

  auto* initialized_field = il2cpp_class_get_field_from_name(cls, "s_initialized");
  if (!initialized_field || initialized_field->type->type != IL2CPP_TYPE_BOOLEAN
      || !(initialized_field->type->attrs & FIELD_ATTRIBUTE_STATIC))
    return nullptr;
  bool initialized = false;
  il2cpp_field_static_get_value(initialized_field, &initialized);
  if (!initialized)
    return nullptr;

  Il2CppObject* instance = nullptr;
  il2cpp_field_static_get_value(field, &instance);
  return instance;
}

bool ReadBoolField(Il2CppObject* object, const char* name, bool& value)
{
  auto* field = il2cpp_class_get_field_from_name(object->klass, name);
  if (!field || field->type->type != IL2CPP_TYPE_BOOLEAN || (field->type->attrs & FIELD_ATTRIBUTE_STATIC))
    return false;
  il2cpp_field_get_value(object, field, &value);
  return true;
}

bool ReadHidden(const MethodInfo* getter, Il2CppObject* manager, bool& hidden)
{
  Il2CppException* exception = nullptr;
  auto*            result    = il2cpp_runtime_invoke(getter, manager, nullptr, &exception);
  if (exception || !result)
    return false;
  hidden = *static_cast<bool*>(il2cpp_object_unbox(result));
  return true;
}

} // namespace

void EnableFleetCommanderAbilityConfirmation()
{
  auto prefs_class =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.PersistentPrefs", "PersistentPrefsManager");
  auto  fc_class = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.FleetCommander", "FleetCommanderManager");
  auto* prefs    = ExistingSingleton(prefs_class);
  auto* manager  = ExistingSingleton(fc_class);
  bool  loaded   = false;
  bool  loading  = true;
  if (!prefs || !manager || !ReadBoolField(prefs, "_isCloudFileLoaded", loaded)
      || !ReadBoolField(prefs, "_isLoading", loading) || !loaded || loading) {
    spdlog::warn("[FCConfirmation] Reset unavailable: account preferences are not ready; try again after loading");
    return;
  }

  const auto* getter = fc_class.GetMethodInfo("get_HideFleetCommanderAbilityUseConfirmationHidden", 0);
  const auto* setter = fc_class.GetMethodInfo("set_HideFleetCommanderAbilityUseConfirmationHidden", 1);
  if (!getter || !setter || getter->return_type->type != IL2CPP_TYPE_BOOLEAN || getter->return_type->byref
      || setter->return_type->type != IL2CPP_TYPE_VOID || setter->parameters[0]->type != IL2CPP_TYPE_BOOLEAN
      || setter->parameters[0]->byref || (getter->flags & METHOD_ATTRIBUTE_STATIC)
      || (setter->flags & METHOD_ATTRIBUTE_STATIC)) {
    spdlog::error("[FCConfirmation] Reset unavailable: client confirmation methods are missing or incompatible");
    return;
  }

  bool hidden = false;
  if (!ReadHidden(getter, manager, hidden)) {
    spdlog::error("[FCConfirmation] Could not read confirmation preference; no reset attempted");
    return;
  }
  spdlog::info("[FCConfirmation] Before reset: hide_fcaa_use_confirmation={}", hidden);
  if (!hidden) {
    spdlog::info("[FCConfirmation] Fleet Commander ability confirmation is already enabled");
    return;
  }

  // Use the same client setter as the preference UI. Never activate an ability or force a cloud upload.
  bool             hide_confirmation = false;
  void*            args[]            = {&hide_confirmation};
  Il2CppException* exception         = nullptr;
  il2cpp_runtime_invoke(setter, manager, args, &exception);
  if (exception) {
    spdlog::error("[FCConfirmation] Client setter failed; confirmation state is unverified");
    return;
  }
  if (!ReadHidden(getter, manager, hidden) || hidden) {
    spdlog::error("[FCConfirmation] Reset readback failed; confirmation state is unverified");
    return;
  }
  spdlog::info("[FCConfirmation] Fleet Commander ability confirmation enabled (hide=false); cloud persistence pending");
}
