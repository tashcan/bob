#pragma once

#include <il2cpp/il2cpp_helper.h>

struct AnimatedRewardsScreenViewController {
public:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Missions.UI", "AnimatedRewardsScreenViewController");
    return class_helper;
  }

  void GoBackToLastSection()
  {
    static auto GoBackToLastSectionMethod = get_class_helper().GetMethod<void(AnimatedRewardsScreenViewController*)>("GoBackToLastSection");
    GoBackToLastSectionMethod(this);
  }

  void OnCollectClicked()
  {
    static auto OnCollectClickedMethod = get_class_helper().GetMethod<void(AnimatedRewardsScreenViewController*)>("OnCollectClicked");
    OnCollectClickedMethod(this);
  }

  bool IsActive()
  {
    auto IsActive = get_class_helper().GetMethod<bool(AnimatedRewardsScreenViewController*)>("IsActive");
    return IsActive(this);
  }
};