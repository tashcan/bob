#pragma once

#include <il2cpp/il2cpp_helper.h>

struct HttpResponse {
public:
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
};
