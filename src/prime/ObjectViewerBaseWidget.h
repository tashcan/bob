#pragma once

#include "NavigationInteractionUIContext.h"
#include "VisibilityController.h"
#include "Widget.h"

template <typename Y>
class ObjectViewerBaseWidget : public Widget<NavigationInteractionUIContext, ObjectViewerBaseWidget<Y>>
{
public:
  __declspec(property(get = __get__visibilityController)) VisibilityController* _visibilityController;
  __declspec(property(get = __get_IsInfoShown, put = __set_IsInfoShown)) bool IsInfoShown;

private:
  friend struct Widget<NavigationInteractionUIContext, ObjectViewerBaseWidget>;
  static IL2CppClassHelper& get_class_helper()
  {
    static IL2CppClassHelper class_helper = Y::get_class_helper().GetParent("ObjectViewerBaseWidget");
    return class_helper;
  }

public:
  VisibilityController* __get__visibilityController()
  {
    static auto field = get_class_helper().GetField("_visibilityController");
    return *(VisibilityController**)((ptrdiff_t)this + field.offset());
  }

  bool __get_IsInfoShown()
  {
    static auto field = get_class_helper().GetProperty(xorstr_("IsInfoShown"));
    return *field.GetRaw<bool>(this);
  }

  void __set_IsInfoShown(bool v)
  {
    static auto field = get_class_helper().GetProperty(xorstr_("IsInfoShown"));
    field.SetRaw(this, v);
  }
};