#pragma once
#include <il2cpp/il2cpp_helper.h>

#include "ActionCostContext.h"
#include "ActivatedAbilityType.h"
#include "ViewController.h"

class ActionPromptPopupViewController : public ViewController<ActionCostContext, ActionPromptPopupViewController>
{
public:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.HUD", "ActionPromptPopupViewController");
    return class_helper;
  }

  static bool IsInstance(void *obj)
  {
    return get_class_helper().get_cls() == ((Il2CppObject*)obj)->klass;
  }

  void OnActionButtonClick()
  {
    static auto OnActionButtonClickMethod =
        get_class_helper().GetMethod<void(ActionPromptPopupViewController*)>("OnActionButtonClick");
    OnActionButtonClickMethod(this);
  }
};
