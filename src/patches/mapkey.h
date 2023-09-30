#pragma once

#include <absl/strings/ascii.h>
#include <key.h>
#include <map>
#include <modifierkey.h>
#include <patches/gamefunctions.h>
#include <prime/KeyCode.h>
#include <string>

class MapKey
{
private:
  static std::map<GameFunction, MapKey*> mappedKeys;

public:
  MapKey();

  static MapKey* Parse(std::string_view key);
  static void    AddMappedKey(GameFunction gameFunction, MapKey* mappedKey);
  static void    SetMappedKey(GameFunction gameFunction, MapKey* mappedKey);
  static bool    HasGameFunction(GameFunction gameFunction);
  static bool    IsPressed(GameFunction gameFunction);

  std::string GetParsedValues();

  std::vector<ModifierKey*> Modifiers;
  std::vector<std::string> Shortcuts;

  KeyCode Key;
};
