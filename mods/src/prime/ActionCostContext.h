#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "ActivatedAbilityType.h"

struct ActionCostContext {
public:
  __declspec(property(get = __get__activatedAbilityType)) ActivatedAbilityType activatedAbilityType;

  ActivatedAbilityType __get__activatedAbilityType()
  {
    static auto field = get_class_helper().GetProperty("ActivatedAbilityType");
    return *field.Get<ActivatedAbilityType>(this);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.HUD", "ActionCostContext");
    return class_helper;
  }
};
