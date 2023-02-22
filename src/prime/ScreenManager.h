#pragma once

#include "CanvasController.h"
#include "CanvasScaler.h"
#include "MonoSingleton.h"

struct ScreenManager : MonoSingleton<ScreenManager> {
public:
  __declspec(property(get = __get_m_canvasRootScaler)) CanvasScaler* m_canvasRootScaler;
  __declspec(property(get = __get__minimumCanvasScaleFactor)) float _minimumCanvasScaleFactor;
  __declspec(property(get = __get__maximumCanvasScaleFactor)) float _maximumCanvasScaleFactor;

  static CanvasController* GetTopCanvas(bool visibleOnly = false)
  {
    static auto GetTopCanvas = get_class_helper().GetMethod<CanvasController*(bool)>("GetTopCanvas");
    return GetTopCanvas(visibleOnly);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.UI", "ScreenManager");
    return class_helper;
  }

public:
  CanvasScaler* __get_m_canvasRootScaler()
  {
    static auto field = get_class_helper().GetField("m_canvasRootScaler");
    return *(CanvasScaler**)((ptrdiff_t)this + field.offset());
  }

  float __get__minimumCanvasScaleFactor()
  {
    static auto field = get_class_helper().GetField("_minimumCanvasScaleFactor").offset();
    return *(float*)((ptrdiff_t)this + field);
  }

  float __get__maximumCanvasScaleFactor()
  {
    static auto field = get_class_helper().GetField("_maximumCanvasScaleFactor").offset();
    return *(float*)((ptrdiff_t)this + field);
  }
};