#include "config.h"
#include "prime/EventSystem.h"
#include "utils.h"
#include <absl/strings/ascii.h>
#include <absl/strings/str_split.h>
#include <cstdint>
#include <iostream>
#include <key.h>
#include <map>
#include <mapkey.h>
#include <modifierkey.h>
#include <prime/TMP_InputField.h>
#include <stdio.h>
#include <string>
#include <string_view>

MapKey::MapKey()
{
  this->Key = KeyCode::None;
}

MapKey MapKey::Parse(std::string_view key)
{
  auto strippedKey = absl::StripTrailingAsciiWhitespace(key);
  auto lowerKey    = absl::AsciiStrToUpper(strippedKey);
  auto wantedKeys  = absl::StrSplit(lowerKey, "-", absl::SkipWhitespace());

  auto mapKey = new MapKey();
  for (std::string_view wantedKey : wantedKeys) {
    auto modifier = ModifierKey::Parse(wantedKey);
    if (modifier.HasModifiers()) {
      mapKey->hasModifiers = true;
      mapKey->Modifiers.emplace_back(modifier);
      mapKey->Shortcuts.emplace_back(wantedKey);
    } else {
      auto parsedKey = Key::Parse(wantedKey);

      if (Key::IsModifier(parsedKey)) {
        continue;
      }

      if (parsedKey != KeyCode::None) {
        mapKey->Key = parsedKey;
        mapKey->Shortcuts.emplace_back(wantedKey);
      }
    }

#ifndef NDEBUG
    if (mapKey->Key == KeyCode::X) {
      std::cout << "\n\n----------\nX key:\n" << mapKey << "\n----------\n\n";
    }
#endif
  }

  return *mapKey;
}

void MapKey::SetMappedKey(GameFunction gameFunction, MapKey mappedKey)
{
  MapKey::mappedKeys[gameFunction] = mappedKey;
}

bool MapKey::IsPressed(GameFunction gameFunction)
{
  MapKey mapKey = MapKey::mappedKeys[(int)gameFunction];
  if (mapKey.Key != KeyCode::None) {
    if (Key::Pressed(mapKey.Key)) {
      return MapKey::HasCorrectModifiers(mapKey);
    }
  }

  return false;
}

bool MapKey::IsDown(GameFunction gameFunction)
{
  MapKey mapKey = MapKey::mappedKeys[(int)gameFunction];

  if (mapKey.Key != KeyCode::None) {
    if (Key::Down(mapKey.Key)) {
      return MapKey::HasCorrectModifiers(mapKey);
    }
  }

  return false;
}

bool MapKey::HasCorrectModifiers(MapKey mapKey)
{
  auto        result  = false;
  std::string section = "non set";
  if (!mapKey.hasModifiers) {
    section = "no modifiers";
    result  = !Key::IsModified();
  } else {
    result = true;
    for (ModifierKey modifier : mapKey.Modifiers) {
      if (!modifier.IsPressed()) {
        section = modifier.GetParsedValues();
        result  = false;
        break;
      }
    }
  }

#ifndef NDEBUG
  if (mapKey.Key == KeyCode::Backslash && Key::Pressed(KeyCode::Backslash)) {
    std::cout << "HasCorrectModifiers(" << mapKey.GetParsedValues() << "): [" << section << "] " << result << "\n";
  }
#endif

  return result;
}

std::string MapKey::GetParsedValues()
{
  std::string output = "";
  for (const std::string_view key : this->Shortcuts) {
    if (output.length()) {
      output.append("-");
    }
    output.append(key);
  }

  return output;
}

std::array<MapKey, (int)GameFunction::Max> MapKey::mappedKeys = {};

std::vector<std::string> Shortcuts = {};
std::vector<ModifierKey> Modifiers = {};

bool    hasModifiers = false;
KeyCode Key          = KeyCode::None;