#pragma once

#include <absl/strings/ascii.h>
#include <key.h>
#include <array>
#include <modifierkey.h>
#include <patches/gamefunctions.h>
#include <prime/KeyCode.h>
#include <string>

class MapKey
{
public:
  MapKey();

private:
  static std::array<MapKey, (int)GameFunction::Max> mappedKeys;

  bool hasModifiers;

public:
  static MapKey Parse(std::string_view key);
  static void   SetMappedKey(GameFunction gameFunction, MapKey mappedKey);
  static bool   IsPressed(GameFunction gameFunction);
  static bool   IsDown(GameFunction gameFunction);
  static bool   HasCorrectModifiers(MapKey mapKey);

  std::string GetParsedValues();

  std::vector<ModifierKey> Modifiers;
  std::vector<std::string> Shortcuts;

  KeyCode Key;
};
