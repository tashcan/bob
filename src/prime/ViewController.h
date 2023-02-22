#pragma once

#include <il2cpp/il2cpp_helper.h>

template <typename T, typename Y> struct ViewController {
public:
  __declspec(property(get = __get_CanvasContext)) T* CanvasContext;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static IL2CppClassHelper class_helper = Y::get_class_helper().GetParent("ViewController`1");
    return class_helper;
  }

public:
  T* __get_CanvasContext()
  {
    static auto field = get_class_helper().GetProperty("CanvasContext");
    return field.GetRaw<T>(this);
  }
};