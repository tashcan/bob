#include "key.h"
#include "config.h"
#include "prime/EventSystem.h"
#include "utils.h"
#include <absl/strings/ascii.h>
#include <cstdint>
#include <iostream>
#include <map>
#include <prime/TMP_InputField.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

const std::map<std::string, KeyCode> Key::mappedKeys = {
    {"LALT", KeyCode::LeftAlt},
    {"LAPPLE", KeyCode::LeftApple},
    {"LCOM", KeyCode::LeftCommand},
    {"LCTRL", KeyCode::LeftControl},
    {"ALTGR", KeyCode::AltGr},
    {"END", KeyCode::End},
    {"HOME", KeyCode::Home},
    {"PGDOWN", KeyCode::PageDown},
    {"PGUP", KeyCode::PageUp},
    {"DOWN", KeyCode::DownArrow},
    {"LEFT", KeyCode::LeftArrow},
    {"RIGHT", KeyCode::RightArrow},
    {"UP", KeyCode::UpArrow},
    {"BACKSPACE", KeyCode::Backspace},
    {"BREAK", KeyCode::Break},
    {"CAPS", KeyCode::CapsLock},
    {"CLEAR", KeyCode::Clear},
    {"DELETE", KeyCode::Delete},
    {"ESCAPE", KeyCode::Escape},
    {"HELP", KeyCode::Help},
    {"INSERT", KeyCode::Insert},
    {"LSHIFT", KeyCode::LeftShift},
    {"LWIN", KeyCode::LeftWindows},
    {"MENU", KeyCode::Menu},
    {"PAUSE", KeyCode::Pause},
    {"PRINT", KeyCode::Print},
    {"RALT", KeyCode::RightAlt},
    {"RAPPLE", KeyCode::RightApple},
    {"RCOM", KeyCode::RightCommand},
    {"RCTRL", KeyCode::RightControl},
    {"RETURN", KeyCode::Return},
    {"RSHIFT", KeyCode::RightShift},
    {"RWIN", KeyCode::RightWindows},
    {"SCROLL", KeyCode::ScrollLock},
    {"SYSREQ", KeyCode::SysReq},
    {"TAB", KeyCode::Tab},
    {"MOUSE0", KeyCode::Mouse0},
    {"MOUSE1", KeyCode::Mouse1},
    {"MOUSE2", KeyCode::Mouse2},
    {"MOUSE3", KeyCode::Mouse3},
    {"MOUSE4", KeyCode::Mouse4},
    {"MOUSE5", KeyCode::Mouse5},
    {"MOUSE6", KeyCode::Mouse6},
    {"SPACE", KeyCode::Space},
    {"MINUS", KeyCode::Minus},
    {"_", KeyCode::Underscore},
    {",", KeyCode::Comma},
    {";", KeyCode::Semicolon},
    {":", KeyCode::Colon},
    {"!", KeyCode::Exclaim},
    {"?", KeyCode::Question},
    {".", KeyCode::Period},
    {"'", KeyCode::Quote},
    {"(", KeyCode::LeftParen},
    {")", KeyCode::RightParen},
    {"[", KeyCode::LeftBracket},
    {"]", KeyCode::RightBracket},
    {"{", KeyCode::LeftCurlyBracket},
    {"}", KeyCode::RightCurlyBracket},
    {"@", KeyCode::At},
    {"*", KeyCode::Asterisk},
    {"/", KeyCode::Slash},
    {"\"", KeyCode::DoubleQuote},
    {"\\", KeyCode::Backslash},
    {"&", KeyCode::Ampersand},
    {"#", KeyCode::Hash},
    {"%", KeyCode::Percent},
    {"`", KeyCode::BackQuote},
    {"^", KeyCode::Caret},
    {"+", KeyCode::Plus},
    {"<", KeyCode::Less},
    {"=", KeyCode::Equals},
    {">", KeyCode::Greater},
    {"|", KeyCode::Pipe},
    {"~", KeyCode::Tilde},
    {"$", KeyCode::Dollar},
    {"0", KeyCode::Alpha0},
    {"1", KeyCode::Alpha1},
    {"2", KeyCode::Alpha2},
    {"3", KeyCode::Alpha3},
    {"4", KeyCode::Alpha4},
    {"5", KeyCode::Alpha5},
    {"6", KeyCode::Alpha6},
    {"7", KeyCode::Alpha7},
    {"8", KeyCode::Alpha8},
    {"9", KeyCode::Alpha9},
    {"A", KeyCode::A},
    {"B", KeyCode::B},
    {"C", KeyCode::C},
    {"D", KeyCode::D},
    {"E", KeyCode::E},
    {"F", KeyCode::F},
    {"F1", KeyCode::F1},
    {"F10", KeyCode::F10},
    {"F11", KeyCode::F11},
    {"F12", KeyCode::F12},
    {"F13", KeyCode::F13},
    {"F14", KeyCode::F14},
    {"F15", KeyCode::F15},
    {"F2", KeyCode::F2},
    {"F3", KeyCode::F3},
    {"F4", KeyCode::F4},
    {"F5", KeyCode::F5},
    {"F6", KeyCode::F6},
    {"F7", KeyCode::F7},
    {"F8", KeyCode::F8},
    {"F9", KeyCode::F9},
    {"G", KeyCode::G},
    {"H", KeyCode::H},
    {"I", KeyCode::I},
    {"J", KeyCode::J},
    {"K", KeyCode::K},
    {"L", KeyCode::L},
    {"M", KeyCode::M},
    {"N", KeyCode::N},
    {"O", KeyCode::O},
    {"P", KeyCode::P},
    {"Q", KeyCode::Q},
    {"R", KeyCode::R},
    {"S", KeyCode::S},
    {"T", KeyCode::T},
    {"U", KeyCode::U},
    {"V", KeyCode::V},
    {"W", KeyCode::W},
    {"X", KeyCode::X},
    {"Y", KeyCode::Y},
    {"Z", KeyCode::Z},
    {"NUMLOCK", KeyCode::Numlock},
    {"KEY0", KeyCode::Keypad0},
    {"KEY1", KeyCode::Keypad1},
    {"KEY2", KeyCode::Keypad2},
    {"KEY3", KeyCode::Keypad3},
    {"KEY4", KeyCode::Keypad4},
    {"KEY5", KeyCode::Keypad5},
    {"KEY6", KeyCode::Keypad6},
    {"KEY7", KeyCode::Keypad7},
    {"KEY8", KeyCode::Keypad8},
    {"KEY9", KeyCode::Keypad9},
    {"KEYDIVIDE", KeyCode::KeypadDivide},
    {"KEYENTER", KeyCode::KeypadEnter},
    {"KEYEQUAL", KeyCode::KeypadEquals},
    {"KEYMINUS", KeyCode::KeypadMinus},
    {"KEYMULTI", KeyCode::KeypadMultiply},
    {"KEYPERIOD", KeyCode::KeypadPeriod},
    {"KEYPLUS", KeyCode::KeypadPlus},
};

const std::vector<KeyCode> Key::modifiers = {
    KeyCode::LeftAlt,
    KeyCode::LeftApple,
    KeyCode::LeftCommand,
    KeyCode::LeftControl,
    KeyCode::AltGr,
    KeyCode::LeftShift,
    KeyCode::LeftWindows,
    KeyCode::RightAlt,
    KeyCode::RightApple,
    KeyCode::RightCommand,
    KeyCode::RightControl,
    KeyCode::RightShift,
    KeyCode::RightWindows,
};

Key::Key()
{
  // Map();
}

KeyCode Key::Parse(std::string_view key)
{
  auto wantedKey = absl::AsciiStrToUpper(key);
  for (const auto& [value, keycode] : mappedKeys) {
    if (key == value) {
      return (KeyCode)keycode;
    }
  }

  return KeyCode::None;
}

bool Key::IsModifier(KeyCode key)
{
  return (std::find(Key::modifiers.begin(), Key::modifiers.end(), key) != Key::modifiers.end());
}

bool Key::IsModified()
{
  for (const auto modifier: Key::modifiers) {
    if (Key::Pressed(modifier)) {
      return true;
    }
  };

  return false;
}

bool Key::Down(KeyCode key)
{
  static auto GetKeyDownInt =
      il2cpp_resolve_icall<bool(KeyCode)>("UnityEngine.Input::GetKeyDownInt(UnityEngine.KeyCode)");
  
  return GetKeyDownInt(key);
}

bool Key::Pressed(KeyCode key)
{
  static auto GetKeyInt = il2cpp_resolve_icall<bool(KeyCode)>("UnityEngine.Input::GetKeyInt(UnityEngine.KeyCode)");
  
  return GetKeyInt(key);
}

bool Key::IsInputFocused()
{
  auto eventSystem      = EventSystem::current();
  if (eventSystem) {
    auto n = eventSystem->currentSelectedGameObject;
    if (!n) {
      return false;
    }
    try {
      if (n) {
        auto n2 = n->GetComponentFastPath2<TMP_InputField>();
        if (n2) {
          return n2->isFocused;
        }
      }
    } catch (...) {
      return false;
    }
  }
  return false;
}

bool Key::HasShift()
{
  return Key::Pressed(KeyCode::LeftShift) || Key::Pressed(KeyCode::RightShift);
}

bool Key::HasAlt()
{
  return Key::Pressed(KeyCode::LeftAlt) || Key::Pressed(KeyCode::RightShift);
}

bool Key::HasCtrl()
{
  return Key::Pressed(KeyCode::LeftControl) || Key::Pressed(KeyCode::RightControl);
}

void Key::ClearInput()
{
  try {
    if (auto eventSystem = EventSystem::current(); eventSystem) {
      if (auto n = eventSystem->currentSelectedGameObject; n) {
        auto n2 = n->GetComponentFastPath2<TMP_InputField>();
        if (n2 && n2->isFocused) {
          eventSystem->SetSelectedGameObject(nullptr);
          return;
        }
      }
    }
  } catch (...) {
    //
  }
}
