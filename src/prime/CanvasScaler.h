#pragma once

#include "Vector2.h"

struct CanvasScaler {
public:
  __declspec(property(get = __get_scaleFactor, put = __set_scaleFactor)) float scaleFactor;
  __declspec(property(get = __get_referenceResolution, put = __set_referenceResolution)) vec2 referenceResolution;

  void SetScaleFactor(float v)
  {
    static auto SetScaleFactor = get_class_helper().GetMethod<void(CanvasScaler*, float)>(xorstr_("SetScaleFactor"));
    SetScaleFactor(this, v);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("UnityEngine.UI", "UnityEngine.UI", "CanvasScaler");
    return class_helper;
  }

public:
  float __get_scaleFactor()
  {
    static auto field = get_class_helper().GetProperty(xorstr_("scaleFactor"));
    return *field.Get<float>(this);
  }
  void __set_scaleFactor(float v)
  {
    static auto field = get_class_helper().GetProperty(xorstr_("scaleFactor"));
    field.SetRaw<float>(this, v);
  }

  vec2 __get_referenceResolution()
  {
    static auto field = get_class_helper().GetProperty(xorstr_("referenceResolution"));
    return *field.Get<vec2>(this);
  }
  void __set_referenceResolution(vec2 v)
  {
    static auto field = get_class_helper().GetProperty(xorstr_("referenceResolution"));
    field.SetRaw<vec2>(this, v);
  }
};