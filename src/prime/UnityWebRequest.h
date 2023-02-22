#pragma once

#include <il2cpp/il2cpp_helper.h>

struct UnityWebRequest {
public:
  const uintptr_t get_uploadHandler()
  {
    static auto prop = get_class_helper().GetProperty("uploadHandler");
    auto        s    = prop.Get<uintptr_t>((void*)this);
    return (uintptr_t)s;
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("UnityEngine.UnityWebRequestModule", "UnityEngine.Networking", "UnityWebRequest");
    return class_helper;
  }
};
