#pragma once

#include <il2cpp/il2cpp_helper.h>

class IBuffComparer
{
public:
  long get_HullId()
  {
    static auto prop = get_class_helper().GetProperty("HullID");
    return *prop.Get<long>((void*)this);
  }

  long get_FactionId()
  {
    static auto prop = get_class_helper().GetProperty("FactionID");
    return *prop.Get<long>((void*)this);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Services", "IBuffComparer");
    return class_helper;
  }
};