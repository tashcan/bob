#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "Vector3.h"

struct Transform {
  __declspec(property(get = __get_LocalScale, put = __set_LocalScale)) Vector3* localScale;

  Vector3* __get_LocalScale()
  {
    static auto field = get_class_helper().GetProperty("localScale");
    return field.Get<Vector3>(this);
  }

  void __set_LocalScale(Vector3* v)
  {
    static auto prop = get_class_helper().GetProperty("localScale");
    return prop.SetRaw((void*)this, *v);
  }


private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine", "Transform");
    return class_helper;
  }
};
