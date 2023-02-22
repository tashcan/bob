#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "BattleTargetData.h"
#include "VisibilityController.h"
#include "Widget.h"

class RewardsButtonWidget : public Widget<BattleTargetData, RewardsButtonWidget>
{
public:
  __declspec(property(get = __get__rewardsController)) VisibilityController* _rewardsController;

  void RewardsClicked()
  {
    auto RewardsClicked = get_class_helper().GetMethod<void(Widget*)>("RewardsClicked");
    return RewardsClicked(this);
  }

private:
  friend struct Widget<BattleTargetData, RewardsButtonWidget>;
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Combat", "RewardsButtonWidget");
    return class_helper;
  }

public:
  VisibilityController* __get__rewardsController()
  {
    static auto field = get_class_helper().GetField("_rewardsController");
    return *(VisibilityController**)((ptrdiff_t)this + field.offset());
  }
};