#pragma once

#include <il2cpp/il2cpp_helper.h>

struct Vector3 {
public:
  float x;
  float y;
  float z;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine", "Vector3");
    return class_helper;
  }

public:
  static Vector3 zero()
  {
    return get_class_helper().GetStaticField("zeroVector").Get<Vector3>();
  }

  /*  float __get_x() {
        static auto field = get_class_helper().GetField("x");
        return *(float*)((ptrdiff_t)this + field.offset());
    }

    void __set_x(float v) {
        static auto field = get_class_helper().GetField("x");
        *(float*)((ptrdiff_t)this + field.offset()) = v;
    }

    float __get_y() {
        static auto field = get_class_helper().GetField("y");
        return *(float*)((ptrdiff_t)this + field.offset());
    }

    void __set_y(float v) {
        static auto field = get_class_helper().GetField("y");
        *(float*)((ptrdiff_t)this + field.offset()) = v;
    }

    float __get_z() {
        static auto field = get_class_helper().GetField("z");
        return *(float*)((ptrdiff_t)this + field.offset());
    }

    void __set_z(float v) {
        static auto field = get_class_helper().GetField("z");
        *(float*)((ptrdiff_t)this + field.offset()) = v;
    }*/
};