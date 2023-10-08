#pragma once

#include <il2cpp/il2cpp_helper.h>

template <typename Y> struct ActionRequirement {

public:
  __declspec(property(get = __get_IsMet)) bool IsMet;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static IL2CppClassHelper class_helper = Y::get_class_helper().GetParent("ActionRequirement");
    return class_helper;
  }

public:
  bool __get_IsMet()
  {
    static auto field = get_class_helper().GetProperty(xorstr_("IsMet"));
    return field.GetRaw<bool>(this);
  }
};