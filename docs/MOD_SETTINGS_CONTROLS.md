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
| Future separate branch: Hotkeys | Rebind existing actions | Existing shortcut parser and `MapKey` registrations |
| General > confirmation page | Confirm Forbidden Tech upgrades | Inverse of `ui.auto_confirm_ft_upgrade` |

The controls branch implements instant warp, Fleet Labels and Forbidden Tech on
Windows x64. Hotkey editing remains a separate branch. Native confirmation
controls stay on the native page. FC retains its existing owner.

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

## Fleet Labels and Forbidden Tech

One Fleet Labels page contains a non-clickable Player heading, its Native /
Expanded / Compact / Threshold choices and percentage slider, followed by the
same controls under a Non-player heading. Each profile has its own owner and
selection; native text rows provide the headings without navigation or actions.
Threshold is stored in [0, 1], edited in 1% steps,
and enabled only in Threshold mode. At 0% labels stay compact; at 100% they stay
expanded. Reading a player-authored fractional value does not round or save it.
Each user edit updates the existing live profile and refreshes tracked labels.
The native slider callbacks use the same typed snapshot/reentry guards as choices.
Unknown values suppress the slider and numeric label; disabled known values remain
visible. Releasing a pooled widget restores its label, active state and interaction.

Windows installs the existing fleet-label and Forbidden Tech hooks when the mod
settings UI is enabled, so changing their values does not require a restart.
Each FT hook consults the current bypass flag; hook availability is separate from
the value. Other platforms retain startup-controlled installation and omit this UI.
Confirmation ON means the bypass flag is false. Toggling must never invoke an
upgrade callback by itself.

The existing TOML writer now registers these additional keys at startup. One
worker serializes changes to the same file, keeping the latest pending intent
**per key**. A 150 ms quiet period coalesces slider motion; normal quit flushes the
pending value without waiting out that delay. F10 retains its existing force-close
cancellation and 500 ms best effort bound. Save failures/conflicts log the affected
section and key and leave the live setting in place. Numeric edits use the TOML
serializer and the same source-preserving edit/reparse/external-edit checks.
Page opens and native rendering never enqueue saves.

Exact Windows build261 unwind extents, checked before expanding installation:

| Native target | RVA | Bytes |
| --- | --- | --- |
| SliderOptionWidget.SetWidgetData | D09C50 | 592 |
| SliderOptionWidget.OnSliderValueChanged | D0A1E0 | 117 |
| SliderOptionWidget.OnAboutToReleaseContext | D09EA0 | 288 |
| NavigationLOD.UpdateLOD | F8ECF0 | 75 |
| NavigationFleetWidget.OnDidBindContext | F7C870 | 335 |
| NavigationFleetWidget.OnAboutToReleaseContext | F7CEA0 | 283 |
| NavigationFleetWidget.OnEnable | F7D8B0 | 344 |
| NavigationFleetWidget.OnDisable | F7DAF0 | 236 |
| MessageBox.Show(context) | 70B5F0 | 81 |
| MessageBox.Show(context, callback) | 70B650 | 257 |
| TextOptionWidget.SetWidgetData | D0A470 | 293 |
| TextOptionWidget.ClearWidgetData | D0A680 | 271 |

These exceed the bundled x64 SPUD 24-byte overwrite. Runtime also rejects tiny
or interior entries using unwind metadata. Client SHA256:
`487af4bb9c697c353be9714359a97dddcece5dab872622a6c498a27bbfc44f40`.
This is Windows evidence, not proof of macOS hook fit or native widget behavior.

## Future organization and commands (design notes)

Use the existing TOML sections as the organizing vocabulary: Audio (`[audio]`),
Buffs (`[buffs]`), Config (`[config]`), Control (`[control]`), Graphics, and so on.
Introduce a group when it gains a working control and an explicit apply path.
Do not populate empty groups or build a generic editor for every config key.
Human labels and nested pages can be clearer than raw keys; changing placement
must not change storage identity. The current Navigation and Fleet Labels layout
remains the test layout while membership is settled.

A future **Restart client** command could support controls that explicitly need
restart. It would perform an ordinary client restart, settle pending saves using
the existing lifecycle, and relaunch through a supported lifecycle owner. Cache
clearing is a separate operation and must not be called by this command. This is
an idea only: the current branch adds neither restart-only controls nor a restart
command. The ownership/relaunch details need their own design before implementation.

Hotkey editing follows the first real selection and persistence checks. Reuse the
current parser and binding map; add an explicit capture mode with Escape to cancel,
conflict feedback and a deliberate unbind action. Gameplay shortcuts must not fire
while a chord is being captured. Do not serialize display labels as key identities.

## Runtime gate

Before promoting the Navigation slice, verify all three choices, Alt+I changes
while visible, Back/reopen, restart persistence, and an external TOML edit conflict.
Bind build receipts to the installed artifact. The existing synthetic navigation
probe is not evidence that a new selection widget or real persistence path works.
