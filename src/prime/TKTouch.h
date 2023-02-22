#pragma once

#include <il2cpp/il2cpp_helper.h>

enum class NodeDepth {
  Galaxy       = 1,
  SolarSystem  = 2,
  PlanetSystem = 4,
  Starbase     = 8,
};

struct TKTouch {
public:
  __declspec(property(get = __get_phase, put = __set_phase)) int phase;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp-firstpass", "", "TKTouch");
    return class_helper;
  }

public:
  int __get_phase()
  {
    static auto field = get_class_helper().GetField("phase");
    return *(int*)((ptrdiff_t)this + field.offset());
  }

  void __set_phase(int phase)
  {
    static auto field                         = get_class_helper().GetField("phase");
    *(int*)((ptrdiff_t)this + field.offset()) = phase;
  }
};