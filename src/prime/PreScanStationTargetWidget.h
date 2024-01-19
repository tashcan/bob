#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "PreScanTargetWidget.h"

struct PreScanStationTargetWidget : PreScanTargetWidget {
private:
  friend class ObjectFinder<PreScanStationTargetWidget>;
  friend class ObjectViewerBaseWidget<PreScanStationTargetWidget>;
  friend class ObjectFinder<PreScanTargetWidget>;
  friend class ObjectViewerBaseWidget<PreScanTargetWidget>;

 public:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Combat", "PreScanStationTargetWidget");
    return class_helper;
  }
};
