#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "MonoSingleton.h"

class LanguageManager : public MonoSingleton<LanguageManager>
{
public:
  bool TryGetTranslation(Il2CppString *category, Il2CppString *key, Il2CppString **translatedText)
  {
    static auto TryGetTranslation =
        get_class_helper().GetMethod<bool(LanguageManager *, Il2CppString *, Il2CppString *, Il2CppString **)>(
            "TryGetTranslation");
    return TryGetTranslation(this, category, key, translatedText);
  }

private:
  friend struct MonoSingleton<LanguageManager>;
  static IL2CppClassHelper &get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.Localization", "LanguageManager");
    return class_helper;
  }
};