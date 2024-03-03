#pragma once

#include "BlurController.h"

struct TransitionManager {
public:
  __declspec(property(get = __get_BlurController)) BlurController* SBlurController;
  __declspec(property(get = __get__waitFrames, put = __set__waitFrames)) int _waitFrames;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.LoadingScreen", "TransitionManager");
    return class_helper;
  }

public:
  BlurController* __get_BlurController()
  {
    static auto field = get_class_helper().GetField("BlurController");
    return *(BlurController**)((ptrdiff_t)this + field.offset());
  }

  int __get__waitFrames()
  {
    static auto field = get_class_helper().GetField("_waitFrames");
    return *(int*)((ptrdiff_t)this + field.offset());
  }
  void __set__waitFrames(int v)
  {
    static auto field                         = get_class_helper().GetField("_waitFrames");
    *(int*)((ptrdiff_t)this + field.offset()) = v;
  }
};