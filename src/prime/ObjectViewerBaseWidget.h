#pragma once

#include "NavigationInteractionUIContext.h"
#include "ParentObjectViewerViewController.h"
#include "VisibilityController.h"
#include "Widget.h"

template <typename Y>
class ObjectViewerBaseWidget : public Widget<NavigationInteractionUIContext, ObjectViewerBaseWidget<Y>>
{
public:
  __declspec(property(get = __get__visibilityController)) VisibilityController* _visibilityController;
  __declspec(property(get = __get_IsInfoShown, put = __set_IsInfoShown)) bool IsInfoShown;
  __declspec(property(get = __get_Parent)) ParentObjectViewerViewController* Parent;

private:
  friend struct Widget<NavigationInteractionUIContext, ObjectViewerBaseWidget>;
  static IL2CppClassHelper& get_class_helper()
  {
    static IL2CppClassHelper class_helper = Y::get_class_helper().GetParent("ObjectViewerBaseWidget");
    return class_helper;
  }

public:
  void HideAllViewers() {
    static auto HideAllViewersMethod = get_class_helper().GetMethod<void()>("HideAllViewers");

    if (HideAllViewersMethod) {
      HideAllViewersMethod();
    }
  }

  VisibilityController* __get__visibilityController()
  {
    static auto field = get_class_helper().GetField("_visibilityController");
    return *(VisibilityController**)((ptrdiff_t)this + field.offset());
  }

  ParentObjectViewerViewController* __get_Parent()
  {
    static auto field = get_class_helper().GetProperty("Parent");
    return field.GetRaw<ParentObjectViewerViewController>(this);
  }

  bool __get_IsInfoShown()
  {
    static auto field = get_class_helper().GetProperty("IsInfoShown");
    return *field.GetRaw<bool>(this);
  }

  void __set_IsInfoShown(bool v)
  {
    static auto field = get_class_helper().GetProperty("IsInfoShown");
    field.SetRaw(this, v);
  }
};