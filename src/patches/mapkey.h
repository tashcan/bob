#pragma once

#include <absl/strings/ascii.h>
#include <array>
#include <key.h>
#include <modifierkey.h>
#include <patches/gamefunctions.h>
#include <prime/KeyCode.h>
#include <string>
#include <vector>

class MapKey
{
public:
  MapKey();

private:
  static std::array<std::vector<MapKey>, (int)GameFunction::Max> mappedKeys;

  bool hasModifiers;

public:
  static MapKey Parse(std::string_view key);
  static void   AddMappedKey(GameFunction gameFunction, MapKey mappedKey);
  static bool   IsPressed(GameFunction gameFunction);
  static bool   IsDown(GameFunction gameFunction);
  static bool   HasCorrectModifiers(MapKey mapKey);

  static std::string GetShortcuts(GameFunction gameFunction);

  std::string GetParsedValues();

  std::vector<ModifierKey> Modifiers;
  std::vector<std::string> Shortcuts;

  KeyCode Key;
};
