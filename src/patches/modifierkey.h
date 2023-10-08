#pragma once

#include <absl/strings/ascii.h>
#include <key.h>
#include <map>
#include <patches/gamefunctions.h>
#include <prime/KeyCode.h>
#include <string>

class ModifierKey
{
public:
  ModifierKey();

  static ModifierKey* Parse(std::string_view key);

  void AddModifier(std::string_view shortcut, KeyCode modifier1, KeyCode modifier2);
  bool Contains(KeyCode modifier);
  bool IsPressed();
  bool IsDown();

  std::string GetParsedValues();

  std::vector<KeyCode>     Modifiers;
  std::vector<std::string> Shortcuts;
};
