#pragma once

#include <il2cpp/il2cpp_helper.h>
#include "VisibilityController.h"

struct NavigationInteractionUIViewController {
public:
  bool IsSetCourseVisible()
  {
    static auto widget_class = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Navigation", "SetCourseWidget");
    if (!get_class_helper().get_cls() || !widget_class.get_cls()) {
      return false;
    }
    static auto widget_field     = get_class_helper().GetField("_setCourseWidget");
    static auto visibility_field = widget_class.GetField("_visibilityController");
    static auto active_property  = widget_class.GetProperty("isActiveAndEnabled");
    if (!widget_field.get_info() || !visibility_field.get_info()) {
      return false;
    }

    auto* widget = *reinterpret_cast<Il2CppObject**>(reinterpret_cast<char*>(this) + widget_field.offset());
    if (!widget) {
      return false;
    }
    auto* visibility =
        *reinterpret_cast<VisibilityController**>(reinterpret_cast<char*>(widget) + visibility_field.offset());
    if (!visibility
        || (visibility->_state != VisibilityState::Visible && visibility->_state != VisibilityState::Show)) {
      return false;
    }
    const auto* active = active_property.Get<bool>(widget);
    return active && *active;
  }

  void OnSetCourseButtonClick()
  {
    static auto OnSetCourseButtonClick =
        get_class_helper().GetMethod<void(NavigationInteractionUIViewController*)>("OnSetCourseButtonClick");
    OnSetCourseButtonClick(this);
  }

  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Navigation", "NavigationInteractionUIViewController");
    return class_helper;
  }

private:
  friend class ObjectFinder<NavigationInteractionUIViewController>;
};
