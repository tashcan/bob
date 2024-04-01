# Change Log

## 0.6.1

- Switch to xmake
- Add support for macOS
- Add maximum donation for alliance sliders
- Add ui_scale_viewer to scale the size of object viewers
- Fix ui_scale or ui_scale_viewer so when set to 0.0f disable those scaling features
- Fix zoom_min and zoom_max no longer set default system zoom (only presets can)

## 0.6.0

- Add customisable hotkeys
- Add example configuration file
- Make ui_scaleup/ui_scaldown operate like zoom and can now be held
- Add ui_scale_adjust to allow changes to "step" between ui_scale's
- Add option to disable:
  - Ship locate when preview window open
  - Ending program via escape
  - Movement via keyboard
- Add session-based adjustments that last until game is restarted:
  - Add toggle cargo views (ALT 1-5)
  - Add set zoom preset (SHIFT F1-F5)
- Fix speed issues with ObjectFinder by writing our own
- Adjust runtime configuration output:
  - Rename 'community_patch_settings_parsed.toml' to 'community_patch_runtime.vars' to avoid confusion
  - Add a massive comment to top of community_patch_runtimes.vars
  - When possible report bad configuration values
- Add new key mappings:
  - N: Manage Ship
  - R: Repair Ship
  - U: Show Resarch
  - CTRL-ALT-MINUS: Disable all hotkeys
  - CTRL-ALT-EQUAL: Enable all hotkeys
  - F9: Log level to Debug
  - F11: Log level to Info
  - SHIFT-T: Show Away Team
  - SLASH (Forward): Show Gifts
  - SLASH (Backward): Show Alliance
  - CTRL-L: Toggle whether Locate will work when preview popup visible
  - CTRL-R: Toggle whether Recall will work when preview popup visible
  - ALT-1: Toggle whether Cargo will show for Default
  - ALT-2: Toggle whether Cargo will show for Player
  - ALT-3: Toggle whether Cargo will show for Station
  - ALT-4: Toggle whether Cargo will show for Hostile
  - ALT-5: Toggle whether Cargo will show for Armada
- Add new key mappings (alpha testing only due to implementation issues):
  - SHIFT-SLASH (Backward): Show Alliance Help
  - CTRL-SLASH (Backward): Show Alliance Armada
  - l: Show Lookup

## 0.5.2

- fix free resize not working anymore
- restrict the stay in bundle after summary behavior to officer and event store
- add player names to battle log sync
- fix trait sync not working
- add sync of active missions

## 0.5.0 (future)

- add default toml config when none exists with default values

- add parsed toml config output to show what settings are in use

- add zoom shortcuts (minus, equals, backspace) to provide max, default and minimum zoom

- add more keyboard shortcuts for missions (daily, away, etc), station views, quick chat

## 0.4.0

- who knows

## 0.3.8

- add drydock H hotkey

## 0.3.2

- add compatibility with Unity 2020.3 LTS

## 0.3.1

- add option to disable galaxy chat
- fix a bug where [Space] didn't trigger correctly

## 0.3.0

- add recall option, this is bound to `R` when not having another item selected that has an `R` action
- remove fix for iss jellyfish as this is now fixed in the regular client (`fix_iss_jellyfish_warp` config no longer has any effect)
- remove fix for stella as this is now fixed in the regular client (`fix_stella` config no longer has any effect)
- remove fix for faction as this is now fixed in regular client (`fix_faction` config no longer has any effect)
- re-enable free resizing of window regardless of aspect ratio
- extend alliance donation slider to be the amount required to reach the next alliance level
- extend ship selection hotkeys to 7

# 0.2.0

- initial release
