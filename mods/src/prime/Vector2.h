#pragma once

#include <il2cpp/il2cpp_helper.h>

#include <compare>

struct Vector2 {
public:
  __declspec(property(get = __get_x, put = __set_x)) float x;
  __declspec(property(get = __get_y, put = __set_y)) float y;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine", "Vector2");
    return class_helper;
  }

public:
  float __get_x()
  {
    static auto field = get_class_helper().GetField("x");
    return *(float*)((ptrdiff_t)this + field.offset());
  }

  void __set_x(float v)
  {
    static auto field                           = get_class_helper().GetField("x");
    *(float*)((ptrdiff_t)this + field.offset()) = v;
  }

  float __get_y()
  {
    static auto field = get_class_helper().GetField("y");
    return *(float*)((ptrdiff_t)this + field.offset());
  }

  void __set_y(float v)
  {
    static auto field                           = get_class_helper().GetField("y");
    *(float*)((ptrdiff_t)this + field.offset()) = v;
  }
};

#pragma pack(push, 1)
struct vec2 {
  float x, y;

  friend auto operator<=>(const vec2&, const vec2&) = default;
};
#pragma pack(pop)