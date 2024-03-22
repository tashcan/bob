#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "BattleTargetData.h"
#include "ScanEngageButtonsWidget.h"
#include "ViewController.h"

class ScanTargetViewController : public ViewController<BattleTargetData, ScanTargetViewController>
{
public:
  __declspec(property(get = __get__scanEngageButtonsWidget)) ScanEngageButtonsWidget* _scanEngageButtonsWidget;

private:
  friend class ObjectFinder<ScanTargetViewController>;
  friend struct ViewController<BattleTargetData, ScanTargetViewController>;
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Combat", "ScanTargetViewController");
    return class_helper;
  }

public:
  ScanEngageButtonsWidget* __get__scanEngageButtonsWidget()
  {
    static auto field = get_class_helper().GetField("_scanEngageButtonsWidget").offset();
    return *(ScanEngageButtonsWidget**)((char*)this + field);
  }
};
