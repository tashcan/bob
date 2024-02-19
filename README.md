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

## Building

First clone and initialise the repository:

```bash
git clone https://github.com/tashcan/bob.git
cd bob
git submodule update --recursive --init
```

This will download three submodules and further submodules that they have, so this will take some time to complete.

If you have Visual Studio Code, open the folder and it should ask you to install the CMake extensions bundle.  Once the extensions are installed, you can build the project by pressing `F7` or right clicking on the `CMakeLists.txt` file and selecting `Build All Projects`.  This may ask you to configure the project and pick various items to use in the build.  That should only occur once.

If you do not have Visual Studio Code, this project uses CMake, so the simplest way to build it on Windows:

```ps1
mkdir build
cd build
cmake ../
cmake --build .
```

## Usage

Please note that when this project compiles, it will create a DLL called `stfc-communty-patch.dll`.  This file must be either copied to the `C:\Games\Star Trek Fleet Command\Star Trek Fleet Command\default\game` folder as `version.dll` or create a symbolic link to the file using an elevated (administrator) command prompt:

```console
cd C:\Games\Star Trek Fleet Command\Star Trek Fleet Command\default\game
mklink C:\Source\tashcan\debug\stfc-communty-patch.dll version.dll
```

If you do link the file, please note you will need to close the game to recompile.

## Disclaimer

This is intended to give people insight and possiblity to add new things for QoL improvements.

There is no guarantee or promise that using this for features outside of what is officially offered via this repository will not result actions against your account.

All features and additions provided here via this repository are sanctioned by Scopely and thus aren't subject to account actions.

## License

- GPLv3
