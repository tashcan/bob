#pragma once

#include <il2cpp/il2cpp_helper.h>

class GenericButtonContext
{
public:
  __declspec(property(get = __get_Interactable, put = __set_Interactable)) bool Interactable;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.UI", "GenericButtonContext");
    return class_helper;
  }

public:
  bool __get_Interactable()
  {
    static auto field = get_class_helper().GetProperty("Interactable");
    return *field.Get<bool>(this);
  }
  void __set_Interactable(bool v)
  {
    static auto field = get_class_helper().GetProperty("Interactable");
    field.SetRaw<bool>(this, v);
  }
};