#pragma once

#include <il2cpp/il2cpp_helper.h>

template <typename T> struct StateContainer {
public:
  __declspec(property(get = __get_CurrentState)) T CurrentState;
  __declspec(property(get = __get_PreviousState)) T PreviousState;

private
  T _currentState;
private
  T _previousState;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Models", "StateContainer`1");
    return class_helper;
  }
  T __get_CurrentState()
  {
    static auto field = get_class_helper().GetProperty("CurrentState");
    return *field.Get<T>(this);
  }
  T __get_PreviousState()
  {
    static auto field = get_class_helper().GetProperty("PreviousState");
    return *field.Get<T>(this);
  }
};