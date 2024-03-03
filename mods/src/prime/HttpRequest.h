#pragma once

#include <il2cpp/il2cpp_helper.h>

struct HttpRequest {
public:
  const wchar_t* get_URL()
  {
    static auto il2cpp_string_length =
        (il2cpp_string_length_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_string_length"));
    static auto il2cpp_string_chars =
        (il2cpp_string_chars_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_string_chars"));

    static auto prop = get_class_helper().GetProperty("URL");
    auto        s    = prop.Get<Il2CppString>((void*)this);
    return il2cpp_string_chars(s);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Digit.Engine.Network", "Digit.Networking.Network", "HttpRequest");
    return class_helper;
  }
};
