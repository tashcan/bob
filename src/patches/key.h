#pragma once

#include <absl/strings/ascii.h>
#include <unordered_map>
#include <prime/KeyCode.h>
#include <string>
#include <array>

class Key
{
private:
  static const std::unordered_map<std::string, KeyCode> mappedKeys;

  static int cacheInputFocused;
  static int cacheInputModified;

  static std::array<int, (int)KeyCode::Max> cacheKeyPressed;
  static std::array<int, (int)KeyCode::Max> cacheKeyDown;

public:
  Key();

  static void    ClearInputFocus();
  static void    ResetCache();
  static KeyCode Parse(std::string_view key);

  static bool Pressed(KeyCode key);
  static bool Down(KeyCode key);

  static bool IsModifier(KeyCode key);

  static bool IsModified();
  static bool IsInputFocused();
  static bool HasShift();
  static bool HasAlt();
  static bool HasCtrl();
};
