#include "config_save.h"
#include "toml_editor.h"
#include <cassert>
#include <fstream>
#include <iostream>

using namespace config_edit;
static Request Mode(std::optional<Value> expected, std::string desired)
{ return {"ui", "auto_confirm_instant_warp", std::move(expected), std::move(desired)}; }
int main(int argc, char** argv)
{
  assert(argc == 2);
  TomlEditor editor;
  const auto request = Mode(Value{std::string("none")}, "warp");
  for (const std::string original :
       {"# header\n[ui] # section\nauto_confirm_instant_warp = 'none' # comment\nother = 9\n",
        "\xef\xbb\xbf# BOM\r\n[ui]\r\nauto_confirm_instant_warp = 'none' # comment\r\n",
        "ui = { other = '\xc3\xa9', auto_confirm_instant_warp = 'none' } # tail\n",
        "ui.auto_confirm_instant_warp = '''none'''\n[elsewhere]\nx = nan\n",
        "[\"ui\"]\n\"auto_confirm_instant_warp\" = \"\"\"none\"\"\""}) {
    auto edit = editor.Prepare(original, request);
    assert(edit.outcome == Outcome::Prepared);
    const auto parsed = toml::parse(edit.text);
    assert(parsed["ui"]["auto_confirm_instant_warp"].value<std::string>() == "warp");
    // Only the value token changes; comments, surrounding bytes and line endings remain.
    const auto old      = original.find("'''none'''") != std::string::npos         ? "'''none'''"
                          : original.find("\"\"\"none\"\"\"") != std::string::npos ? "\"\"\"none\"\"\""
                                                                                   : "'none'";
    auto       expected = original;
    expected.replace(expected.find(old), std::string(old).size(), "\"warp\"");
    assert(edit.text == expected);
  }
  for (const std::string original :
       {"", "# comments only", "[ui]", "[ui]\nother = 4\n[other]\nx=3\n", "ui = {} # inline\n", "ui = {other = 4}\n",
        "ui.other = 4\n", "[ui.child]\nx = 4\n"}) {
    const auto edit = editor.Prepare(original, Mode(std::nullopt, "jump"));
    assert(edit.outcome == Outcome::Prepared);
    assert(toml::parse(edit.text)["ui"]["auto_confirm_instant_warp"].value<std::string>() == "jump");
  }
  const std::string special = "quotes \"'\\\n[ui]\nauto_confirm_instant_warp = 'jump'\n\xc3\xa9";
  const auto escaped = editor.Prepare("ui = {auto_confirm_instant_warp='none'}", Mode(request.expected, special));
  assert(escaped.outcome == Outcome::Prepared);
  assert(toml::parse(escaped.text)["ui"]["auto_confirm_instant_warp"].value<std::string>() == special);
  assert(editor.Prepare("[ui]\nauto_confirm_instant_warp='jump'", request).outcome == Outcome::Conflict);
  assert(editor.Prepare("[ui]\nauto_confirm_instant_warp='warp'", request).outcome == Outcome::AlreadySaved);
  assert(editor.Prepare("ui = [", request).outcome == Outcome::InvalidDocument);
  assert(editor.Prepare("ui = 3", request).outcome == Outcome::Unsupported);

  const std::filesystem::path root(argv[1]);
  std::filesystem::create_directories(root);
  const auto        path    = root / "settings.toml";
  const std::string initial = "[ui]\nauto_confirm_instant_warp='none'\nother=true # preserve\n";
  ReplaceConfigText(path, initial);
  const auto external = initial + "# external comment\n";
  ReplaceConfigText(path, external);
  assert(editor.Save(path, request) == Outcome::Saved);
  assert(ReadConfigText(path).ends_with("# external comment\n"));
  ReplaceConfigText(path, "[ui]\nauto_confirm_instant_warp='jump'\n");
  assert(editor.Save(path, request) == Outcome::Conflict);
  assert(!ReplaceConfigText(path, initial, external));
  assert(toml::parse(ReadConfigText(path))["ui"]["auto_confirm_instant_warp"].value<std::string>() == "jump");
  std::cout << "TOML editor preservation/conflict fixtures passed\n";
}
