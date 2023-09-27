#pragma once

#include "NavigationInteractionUIContext.h"
#include "VisibilityController.h"
#include "ViewController.h"
#include "Widget.h"

#include "spdlog/spdlog.h"

class ParentObjectViewerViewController : public ViewController<NavigationInteractionUIContext, ParentObjectViewerViewController>
{
public:
  __declspec(property(get = __get__visibilityController)) VisibilityController* _visibilityController;
  __declspec(property(get = __get_IsShowing)) bool IsShowing;

private:
  friend struct ViewController<NavigationInteractionUIContext, ParentObjectViewerViewController>;
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.ObjectViewer", "ParentObjectViewerViewController");
    spdlog::info("ParentObjectViewerViewController :: Returned Class Helper");
    return class_helper;
  }

public:
  void ActivateViewerHUDState(bool isHUDOpen = false)
  {
    static auto ActivateViewerHUDStateMethod = get_class_helper().GetMethod<void(ParentObjectViewerViewController*, bool)>("ActivateViewerHUDState");
    if (ActivateViewerHUDStateMethod) {
      spdlog::info("ParentObjectViewerViewController :: Before ActivateViewerHUDStateMethod");
      ActivateViewerHUDStateMethod(this, isHUDOpen);
      spdlog::info("ParentObjectViewerViewController :: After ActivateViewerHUDStateMethod");
    }
  }

  void TriggerInstantHide()
  {
    static auto TriggerInstantHideMethod = get_class_helper().GetMethod<void(ParentObjectViewerViewController*)>("TriggerInstantHide");
    TriggerInstantHideMethod(this);
  }

  void TriggerShow()
  {
    static auto TriggerShowMethod = get_class_helper().GetMethod<void(ParentObjectViewerViewController*)>("TriggerInstantHide");
    TriggerShowMethod(this);
  }

  VisibilityController* __get__visibilityController()
  {
    static auto field = get_class_helper().GetField("_visibilityController");
    return *(VisibilityController**)((ptrdiff_t)this + field.offset());
  }

  bool __get_IsShowing()
  {
    static auto field = get_class_helper().GetProperty(xorstr_("IsShowing"));
    return *field.GetRaw<bool>(this);
  }

  // void __set_IsInfoShown(bool v)
  // {
  //   static auto field = get_class_helper().GetProperty(xorstr_("IsInfoShown"));
  //   field.SetRaw(this, v);
  // }
};