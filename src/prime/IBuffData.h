#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "ClientModifierType.h"

class IBuffData
{
public:
  __declspec(property(get = __get_modifierType)) ClientModifierType ModifierType;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit", "IBuffData");
    return class_helper;
  }

public:
  ClientModifierType __get_modifierType()
  {
    static auto prop = get_class_helper().GetProperty("ModifierType");
    return *prop.Get<ClientModifierType>((void*)this);
  }
};