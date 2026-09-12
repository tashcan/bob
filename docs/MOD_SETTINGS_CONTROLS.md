# First mod settings controls

Build real controls on the navigation foundation in small slices. Register only
working controls; omit empty groups. Stable setting keys and storage owners stay
independent of labels and placement.

## Initial layout

| Location | Control | Existing owner |
| --- | --- | --- |
| Mod Settings > Navigation | Instant warp mode: Normal (ask), Warp, Jump | `ui.auto_confirm_instant_warp` and the Alt+I action |
| Mod Settings > Fleet Labels | Player label detail and zoom threshold | `graphics.zoom_label_player_detail`, `graphics.zoom_label_player_threshold` |
| Mod Settings > Fleet Labels | Non-player label detail and zoom threshold | `graphics.zoom_label_non_player_detail`, `graphics.zoom_label_non_player_threshold` |
| Mod Settings > Hotkeys | Rebind existing actions | Existing shortcut parser and `MapKey` registrations |
| General > confirmation page | Confirm Forbidden Tech upgrades | Inverse of `ui.auto_confirm_ft_upgrade` |

The first implementation slice is the three-choice instant-warp control. The
other rows above are planned consumers, not claims of implemented UI. Native
confirmation controls stay on the native page. FC retains its existing owner.

## Instant warp mode

Use one selection, not three independently stored flags. The UI and Alt+I call
the same live mutation function. Cycle order remains Normal > Warp > Jump > Normal.
Selecting the current value does not enqueue another save. Invalid choices do
not alter live state or the file. Existing per-ship overrides retain precedence;
the picker changes only the global fallback mode.

Reuse the existing single runtime writer, optimistic conflict handling and
source-preserving TOML edits. UI readback confirms the live value, not durable
storage; asynchronous save failures continue to go to the log. Reopening must
read the current owner, and shortcut changes must refresh a visible selector.
Native selection callbacks need the same rendering, stale-context and reentry
protection already exercised for boolean controls.

## Following slices

Fleet label controls should expose the existing Native / Expanded / Compact /
Threshold detail modes. A threshold is a normalized zoom value in [0, 1]; use a
human percentage and enable its editor only for Threshold mode. Extend the
existing writer's supported value/setting set before offering persistent edits;
do not start a second worker for the same TOML file.

Forbidden Tech currently installs its bypass hooks only when the startup flag is
true. A live toggle needs hook availability reported separately from the setting,
and each hook must consult the current flag before bypassing a dialog. Verify
native extent and shared hook ownership before changing installation behavior.
Confirmation ON means the bypass flag is false. Toggling must never invoke an
upgrade callback by itself.

Hotkey editing follows the first real selection and persistence checks. Reuse the
current parser and binding map; add an explicit capture mode with Escape to cancel,
conflict feedback and a deliberate unbind action. Gameplay shortcuts must not fire
while a chord is being captured. Do not serialize display labels as key identities.

## Runtime gate

Before promoting the Navigation slice, verify all three choices, Alt+I changes
while visible, Back/reopen, restart persistence, and an external TOML edit conflict.
Bind build receipts to the installed artifact. The existing synthetic navigation
probe is not evidence that a new selection widget or real persistence path works.
