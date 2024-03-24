#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "GameObject.h"

struct EventSystem {
public:
  static EventSystem* current()
  {
    static auto field = get_class_helper().GetProperty("current");
    return field.GetRaw<EventSystem>(nullptr);
  }

  void SetSelectedGameObject(void*)
  {
    static auto SetSelectedGameObject = get_class_helper().GetMethodSpecial<void(EventSystem*, void*)>(
        "SetSelectedGameObject", [](auto count, auto /*params*/) {
          if (count == 1) {
            return true;
          }
          return false;
        });
    return SetSelectedGameObject(this, nullptr);
  }

  __declspec(property(get = __get_currentSelectedGameObject,
                      put = __set_currentSelectedGameObject)) GameObject* currentSelectedGameObject;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("UnityEngine.UI", "UnityEngine.EventSystems", "EventSystem");
    return class_helper;
  }

public:
  GameObject* __get_currentSelectedGameObject()
  {
    static auto field = get_class_helper().GetProperty("currentSelectedGameObject");
    return field.GetRaw<GameObject>(this);
  }

  void __set_currentSelectedGameObject(GameObject* v)
  {
    static auto field = get_class_helper().GetProperty("currentSelectedGameObject");
    return field.SetRaw(this, v);
  }
};