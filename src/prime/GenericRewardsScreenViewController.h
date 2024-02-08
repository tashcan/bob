#pragma once

#include <il2cpp/il2cpp_helper.h>

struct GenericRewardsScreenViewController {
public:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.SharedFeatures", "GenericRewardsScreenViewController");
    return class_helper;
  }

  void OnCollectClicked()
  {
    static auto OnCollectClickedMethod = get_class_helper().GetMethod<void(GenericRewardsScreenViewController*)>("OnCollectClicked");
    OnCollectClickedMethod(this);
  }

  bool IsActive()
  {
    auto IsActive = get_class_helper().GetMethod<bool(GenericRewardsScreenViewController*)>("IsActive");
    return IsActive(this);
  }
};