#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "FleetLocalViewController.h"

struct FleetBarContext {
public:
  __declspec(property(get = __get_CurrentFleet)) void* CurrentFleet;

public:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.HUD", "FleetBarContext");
    return class_helper;
  }

public:
  void* __get_CurrentFleet()
  {
    static auto field = get_class_helper().GetProperty("CurrentFleet");
    return field.Get<void>(this);
  }
};

struct FleetBarViewController {
public:
  __declspec(property(get = __get__fleetPanelController)) FleetLocalViewController* _fleetPanelController;

  void RequestSelect(int32_t index, bool simulated = false)
  {
    static auto RequestSelect = get_class_helper().GetMethodSpecial<void(FleetBarViewController*, int32_t, bool)>(
        "RequestSelect", [](auto count, auto params) {
          if (count != 2) {
            return false;
          }
          auto p1 = params[0].parameter_type->type;
          if (p1 == IL2CPP_TYPE_I4) {
            return true;
          }
          return false;
        });
    RequestSelect(this, index, simulated);
  }

  bool IsIndexSelected(int32_t index)
  {
    static auto IsIndexSelected =
        get_class_helper().GetMethod<bool(FleetBarViewController*, int32_t)>("IsIndexSelected");
    return IsIndexSelected(this, index);
  }

  FleetBarContext* CanvasContext()
  {
    static auto n = get_class_helper().GetProperty("CanvasContext");
    return n.GetRaw<FleetBarContext>(this);
  }

private:
  friend class ObjectFinder<FleetBarViewController>;

public:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.HUD", "FleetBarViewController");
    return class_helper;
  }

public:
  FleetLocalViewController* __get__fleetPanelController()
  {
    static auto field = get_class_helper().GetField("_fleetPanelController").offset();
    return *(FleetLocalViewController**)((uintptr_t)this + field);
  }
};
