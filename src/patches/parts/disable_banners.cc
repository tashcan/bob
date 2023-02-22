#include "config.h"

#include <spud/detour.h>

#include <il2cpp/il2cpp_helper.h>

#include "prime/Toast.h"

struct ToastObserver {
};

void ToastObserver_EnqueueToast_Hook(auto original, ToastObserver *_this, Toast *toast)
{
  if (std::find(Config::Get().disabled_banner_types.begin(), Config::Get().disabled_banner_types.end(),
                toast->get_State())
      != Config::Get().disabled_banner_types.end()) {
    return;
  }
  original(_this, toast);
}

void ToastObserver_EnqueueOrCombineToast_Hook(auto original, ToastObserver *_this, Toast *toast, uintptr_t cmpAction)
{
  if (std::find(Config::Get().disabled_banner_types.begin(), Config::Get().disabled_banner_types.end(),
                toast->get_State())
      != Config::Get().disabled_banner_types.end()) {
    return;
  }
  original(_this, toast, cmpAction);
}

void InstallToastBannerHooks()
{
  auto helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.HUD", "ToastObserver");
  auto ptr    = helper.GetMethodXor("EnqueueToast");
  if (!ptr) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr, ToastObserver_EnqueueToast_Hook);

  helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.HUD", "ToastObserver");
  ptr    = helper.GetMethodXor("EnqueueOrCombineToast");
  if (!ptr) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr, ToastObserver_EnqueueOrCombineToast_Hook);
}
