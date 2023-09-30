#pragma once

#include <absl/strings/ascii.h>
#include <map>
#include <prime/KeyCode.h>
#include <string>
#include <vector>

class Key
{
private:
  static const std::map<std::string, KeyCode> mappedKeys;
  static const std::vector<KeyCode>           modifiers;

public:
  Key();

  static KeyCode Parse(std::string_view key);

  static bool Pressed(KeyCode key);
  static bool Down(KeyCode key);

  static bool IsModifier(KeyCode key);

  static bool IsModified();
  static bool IsInputFocused();
  static bool HasShift();
  static bool HasAlt();
  static bool HasCtrl();

  static void ClearInput();
};
