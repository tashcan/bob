#pragma once

#include <il2cpp/il2cpp_helper.h>

template <typename T, typename Y> struct Widget {
public:
  __declspec(property(get = __get_Context)) T* Context;
  __declspec(property(get = __get_enabled)) bool enabled;

  bool IsActive()
  {
    auto IsActive = get_class_helper().GetMethod<bool(Widget*)>("IsActive");
    return IsActive(this);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static IL2CppClassHelper class_helper = Y::get_class_helper().GetParent("Widget`1");
    return class_helper;
  }

public:
  bool __get_enabled()
  {
    static auto field = get_class_helper().GetProperty("enabled");
    return field.Get<bool>(this);
  }

  T* __get_Context()
  {
    static auto field = get_class_helper().GetProperty("Context");
    return field.GetRaw<T>(this);
  }
};