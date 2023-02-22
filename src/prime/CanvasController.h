#pragma once

#include <il2cpp/il2cpp_helper.h>

struct CanvasController {
public:
  void ResetFocus()
  {
    static auto ResetFocus = get_class_helper().GetMethod<void(CanvasController*)>(xorstr_("ResetFocus"));
    ResetFocus(this);
  }

  bool Visible()
  {
    static auto field = get_class_helper().GetProperty(xorstr_("Visible"));
    return field.Get<bool>(this);
  }

  bool get_enabled()
  {
    static auto field = get_class_helper().GetProperty(xorstr_("enabled"));
    return field.Get<bool>(this);
  }

  bool m_Visible()
  {
    static auto field = get_class_helper().GetField(xorstr_("m_visible")).offset();
    return *(bool*)((ptrdiff_t)this + field);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.UI", "CanvasController");
    return class_helper;
  }
};