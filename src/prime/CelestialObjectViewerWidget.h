#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "Widget.h"

#include "NavigationInteractionUIContext.h"
#include "ObjectViewerBaseWidget.h"
#include "ScanEngageButtonsWidget.h"

struct CelestialObjectViewerWidget : public ObjectViewerBaseWidget<CelestialObjectViewerWidget> {
public:
  __declspec(property(get = __get__scanEngageButtonsWidget)) ScanEngageButtonsWidget* _scanEngageButtonsWidget;

public:
  friend class ObjectFinder<CelestialObjectViewerWidget>;
  friend class ObjectViewerBaseWidget<CelestialObjectViewerWidget>;
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.ObjectViewer", "CelestialObjectViewerWidget");
    return class_helper;
  }

public:
  ScanEngageButtonsWidget* __get__scanEngageButtonsWidget()
  {
    static auto field = get_class_helper().GetField("_scanEngageButtonsWidget").offset();
    return *(ScanEngageButtonsWidget**)((char*)this + field);
  }
};
