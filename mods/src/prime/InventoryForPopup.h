#pragma once

#include <il2cpp/il2cpp_helper.h>

struct InventoryForPopup {
public:
  __declspec(property(get = __get_IsDonationUse, put = __set_IsDonationUse)) bool IsDonationUse;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Inventories", "InventoryForPopup");
    return class_helper;
  }

public:
  bool __get_IsDonationUse()
  {
    static auto field = get_class_helper().GetProperty("IsDonationUse");
    return field.GetRaw(this);
  }

  void __set_IsDonationUse(float v)
  {
    static auto field = get_class_helper().GetProperty("IsDonationUse");
    return field.SetRaw(this, v);
  }
};
