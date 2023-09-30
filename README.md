# Star Trek Fleet Command - Community Patch

<p align="center">
  <img src="https://img.shields.io/badge/License-GPLv3-blue.svg" alt="License: GPLv3">
</p>

<p align="center">
   A community patch that adds a couple of tweaks to the mobile game <b>Star Trek Fleet Command&#8482;</b>
</p>

> [!NOTE]
> The code here is a bit of a mess right now, I will be gradually going through and cleaning it up over the next couple of weeks.

## Features

- Set system UI scale
- Set system zoom
  - default
  - maximum
  - keyboard speed
  - presets (1-5)
- Set transition time
- Disable various toast banners
- Disable galaxy chat
- Enable/Disable hotkeys (community mod or scopely)
- Enable extended donation slider (alliance)
- Show alternative cargo screens for:
  - default
  - player
  - station
  - hostile
  - armada
- Press ESCAPE to remove pre-scan viewers
- Skip reveal sections when opening chests
- Exit section when collecting gifts
- Create default toml file settings file if none exists
- Create parsed toml file to show what settings have been applied

## Keyboard shortcuts

Key | Shortcut
--: | ---
SPACE | Perform default action
1-8 | Ship select/focus
F1-F5 | Zoom presets
Q | Zoom Out
E | Zoom In
T | Events
G | Galaxy
H | System
R | When ship selected, recall ship
R | When clicking on mine/player/enemy, perform non-default action (eg, scan)
V | When clicking on mine/player/enemy, toggle view of cargo or default screen
C | Focus Chat (or Open Alliance Chat - Full Screen)
Alt-C | Focus Chat (or Open Alliance Chat - Side of Screen)

The following keybinds have recently been added:

> [!NOTE]
> Can be disabled via `hotkeys_extended` config option

Key | Shortcut
--: | ---
MINUS | Zoom (min)
EQUAL | Zoom (default)
BACKSPACE | Zoom (max)
B | Bookmarks
F | Factions
Shift-F | reFinery
I | Inventory
M | Active Missions
O | Command Center
Shift-O | Officers
Shift-Q | Q-Trials
Shift-T | Away Teams
X | ExoComp
Z | Daily Missions
` | Open Alliance Chat - Side of Screen

## Contributing / Building

If you wish to contribute to the project, or simply compile the DLL yourself, please see [CONTRIBUTING.md](CONTRIBUTING.md)

## Configuration

An example configuration file is [example_community_patch_settings.toml](example_community_patch_settings.toml) and should be
renamed to `community_patch_settings.toml`.  When running this file will be parsed (see `community_patch.log`) and the running
values can be found in `community_patch_settings_parsed.toml`.  If you have any problems with a setting, check the log and parsed
file to verify that the setting was applied.

## Disclaimer

This is intended to give people insight and possiblity to add new things for QoL improvements.

There is no guarantee or promise that using this for features outside of what is officially offered via this repository will not result actions against your account.

All features and additions provided here via this repository are sanctioned by Scopely and thus aren't subject to account actions.

## License

- GPLv3
