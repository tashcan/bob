#pragma once

struct NavigationManager {
public:
  void HideInteraction()
  {
    static auto HideInteraction = get_class_helper().GetMethod<void(NavigationManager*)>("HideInteraction");
    HideInteraction(this);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Navigation", "NavigationManager");
    return class_helper;
  }
};