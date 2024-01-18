#pragma once
#include <il2cpp/il2cpp_helper.h>

struct UIBehaviour {
public:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("UnityEngine.UI", "UnityEngine.EventSystems", "UIBehaviour");
    return class_helper;
  }
};