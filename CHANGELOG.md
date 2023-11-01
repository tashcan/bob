# Change Log

## 0.6.0

- Add customisable hotkeys
- Prevent ship 'locate' functionality when viewing nodes
- Add manage ship key ('N')
- Add repair ship key ('R')
- Add show resarch key ('U')
- Add example configuration file
- Adjust runtime configuration output:
	- Rename 'community_patch_settings_parsed.toml' to 'community_patch_runtime.vars' to avoid confusion
	- Add a massive comment to top of community_patch_runtimes.vars
- Add ui_scale_adjust to allow changes to "step" between ui_scale's
- Make ui_scaleup/ui_scaldown operate like zoom and can now be held
- Add session-based adjustments that last until game is restarted:
	- Add toggle cargo views (ALT 1-5)
	- Add set zoom preset (SHIFT F1-F5) 

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
