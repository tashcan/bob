#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "HttpRequest.h"

struct HttpResponse {
public:
  struct System_Byte_array {
    Il2CppObject        obj;
    Il2CppArrayBounds*  bounds;
    il2cpp_array_size_t max_length;
    uint8_t             m_Items[65535];
  };

  __declspec(property(get = __get_bytes)) System_Byte_array* Bytes;

  HttpRequest* get_Request()
  {
    static auto prop = get_class_helper().GetProperty("Request");
    auto        s    = prop.GetRaw<HttpRequest>((void*)this);
    return s;
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Digit.Engine.Network", "Digit.Networking.Network", "HttpResponse");
    return class_helper;
  }

public:
  System_Byte_array* __get_bytes()
  {
    static auto prop = get_class_helper().GetProperty("Bytes");
    return prop.GetRaw<System_Byte_array>(this);
  }
};
