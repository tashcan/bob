#pragma once

class Camera
{
public:
  __declspec(property(get = __get_farClipPlane, put = __set_farClipPlane)) float farClipPlane;
  __declspec(property(get = __get_nearClipPlane, put = __set_nearClipPlane)) float nearClipPlane;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine", "Camera");
    return class_helper;
  }

public:
  float __get_farClipPlane()
  {
    static auto field = get_class_helper().GetProperty("farClipPlane");
    return *field.GetRaw<float>(this);
  }
  void __set_farClipPlane(float v)
  {
    static auto field = get_class_helper().GetProperty("farClipPlane");
    field.SetRaw(this, v);
  }

  float __get_nearClipPlane()
  {
    static auto field = get_class_helper().GetProperty("nearClipPlane");
    return *field.GetRaw<float>(this);
  }
  void __set_nearClipPlane(float v)
  {
    static auto field = get_class_helper().GetProperty("nearClipPlane");
    field.SetRaw(this, v);
  }
};