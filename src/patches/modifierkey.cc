#pragma once

#include "config.h"
#include "utils.h"
#include <key.h>
#include <modifierkey.h>
#include <string>
#include <string_view>
#include <iostream>
#include <abseil-cpp/absl/strings/str_split.h>

ModifierKey::ModifierKey() {}

ModifierKey* ModifierKey::Parse(std::string_view key)
{
  auto strippedKey = absl::StripTrailingAsciiWhitespace(key);
  auto upperKey    = absl::AsciiStrToUpper(strippedKey);
  auto wantedKeys  = absl::StrSplit(upperKey, "-", absl::SkipWhitespace());

  auto modifierKey = new ModifierKey();
  for (const auto wantedKey : wantedKeys) {
    if (wantedKey == "SHIFT") {
      modifierKey->AddModifier(wantedKey, KeyCode::LeftShift, KeyCode::RightShift);
    } else if (wantedKey == "ALT") {
      modifierKey->AddModifier(wantedKey, KeyCode::LeftAlt, KeyCode::RightAlt);
    } else if (wantedKey == "CTRL") {
      modifierKey->AddModifier(wantedKey, KeyCode::LeftControl, KeyCode::RightControl);
    } else if (wantedKey == "APPLE") {
      modifierKey->AddModifier(wantedKey, KeyCode::LeftApple, KeyCode::RightApple);
    } else if (wantedKey == "CMD") {
      modifierKey->AddModifier(wantedKey, KeyCode::LeftCommand, KeyCode::RightCommand);
    } else if (wantedKey == "WIN") {
      modifierKey->AddModifier(wantedKey, KeyCode::LeftWindows, KeyCode::RightWindows);
    } else {
      auto parsedKey = Key::Parse(wantedKey);

      if (Key::IsModifier(parsedKey)) {
        modifierKey->AddModifier(wantedKey, parsedKey, KeyCode::None);
      }
    }
  }

  if (!modifierKey->Modifiers.empty()) {
    return modifierKey;
  }

  return nullptr;
}

bool ModifierKey::Contains(KeyCode modifier)
{
  return (std::find(this->Modifiers.begin(), this->Modifiers.end(), modifier) != this->Modifiers.end());
}

void ModifierKey::AddModifier(std::string_view shortcut, KeyCode modifier1, KeyCode modifier2)
{
  if (!this->Contains(modifier1)) {
    this->Modifiers.emplace_back(modifier1);
    this->Shortcuts.emplace_back(shortcut);
  }

  if (!this->Contains(modifier2)) {
    this->Modifiers.emplace_back(modifier2);
    this->Shortcuts.emplace_back(shortcut);
  }
}

bool ModifierKey::IsPressed()
{
  for (auto modifier : this->Modifiers) {
    auto result = Key::Pressed(modifier);
    if (modifier == KeyCode::LeftShift) {
      std::cout << "LeftShift = " << result << "\n";
    }
    if (result) {
        return true;
    }
  }

  return false;
}

std::string ModifierKey::GetParsedValues()
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
