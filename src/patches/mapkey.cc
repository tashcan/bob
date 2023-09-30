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
#include <string>
#include <string_view>

MapKey::MapKey()
{
  this->Key = KeyCode::None;
}

MapKey* MapKey::Parse(std::string_view key)
{
  auto strippedKey = absl::StripTrailingAsciiWhitespace(key);
  auto lowerKey    = absl::AsciiStrToUpper(strippedKey);
  auto wantedKeys  = absl::StrSplit(lowerKey, "-", absl::SkipWhitespace());

  auto mapKey = new MapKey();
  for (const auto wantedKey : wantedKeys) {
    auto modifier = ModifierKey::Parse(wantedKey);
    if (modifier) {
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
  }

  if (mapKey->Key != KeyCode::None) {
    return mapKey;
  }

  return nullptr;
}

bool MapKey::HasGameFunction(GameFunction gameFunction)
{
  return MapKey::mappedKeys.contains(gameFunction);
}

void MapKey::AddMappedKey(GameFunction gameFunction, MapKey* mappedKey)
{
  if (MapKey::HasGameFunction(gameFunction)) {
    return MapKey::SetMappedKey(gameFunction, mappedKey);
  }

  MapKey::mappedKeys.insert(std::make_pair(gameFunction, mappedKey));
}

void MapKey::SetMappedKey(GameFunction gameFunction, MapKey* mappedKey)
{
  if (!MapKey::HasGameFunction(gameFunction)) {
    return MapKey::AddMappedKey(gameFunction, mappedKey);
  }

  MapKey::mappedKeys[gameFunction] = mappedKey;
}

bool MapKey::IsPressed(GameFunction gameFunction)
{
  if (!MapKey::HasGameFunction(gameFunction)) {
    return false;
  }

  auto    result = false;
  MapKey* mapKey = MapKey::mappedKeys.at(gameFunction);

  if (Key::Down(mapKey->Key)) {
    if (mapKey->Modifiers.empty()) {
      result = !Key::IsModified();
    } else {
      result = true;
      for (const auto modifier : mapKey->Modifiers) {
        if (!modifier->IsPressed()) {
          result = false;
          break;
        }
      }
    }
  }

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

std::map<GameFunction, MapKey*> MapKey::mappedKeys = {};

std::vector<std::string>  Shortcuts = {};
std::vector<ModifierKey*> Modifiers = {};

KeyCode Key;
