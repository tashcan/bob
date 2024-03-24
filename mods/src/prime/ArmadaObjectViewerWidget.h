#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "GenericButtonContext.h"
#include "ObjectViewerBaseWidget.h"
#include "Widget.h"

struct ArmadaObjectViewerWidget : public ObjectViewerBaseWidget<ArmadaObjectViewerWidget> {
public:
  void SetCourseToArmada()
  {
    static auto SetCourseToArmada =
        get_class_helper().GetMethod<void(ArmadaObjectViewerWidget*)>("SetCourseToArmada");
    SetCourseToArmada(this);
  }
  void ValidateThenJoinArmada()
  {
    static auto ValidateThenJoinArmada =
        get_class_helper().GetMethod<void(ArmadaObjectViewerWidget*)>("ValidateThenJoinArmada");
    ValidateThenJoinArmada(this);
  }

  void JoinArmada()
  {
    static auto JoinArmada = get_class_helper().GetMethod<void(ArmadaObjectViewerWidget*)>("JoinArmada");
    JoinArmada(this);
  }

  bool HasJoinButton()
  {
    // TODO
  }

private:
  friend class ObjectFinder<ArmadaObjectViewerWidget>;
  friend class ObjectViewerBaseWidget<ArmadaObjectViewerWidget>;

public:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.ObjectViewer", "ArmadaObjectViewerWidget");
    return class_helper;
  }

public:
  GenericButtonContext* __get__joinContext()
  {
    static auto field = get_class_helper().GetField("_joinContext").offset();
    return *(GenericButtonContext**)((char*)this + field);
  }
};