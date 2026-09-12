# Native settings selection appearance

The build261 `gamesettings` Pre-Bundle contains a `LanguageOption` prefab used
by `SelectionItemOptionWidget`. Its direct children include `BG`, `Label`,
`CheckBox`, and `Arrow`. The widget's Toggle and Animator live on the row itself.
Toggle.graphic is null; the Animator controls the actual checkmark.

`ImageToggleOn` and `ImageToggleOff` animate three distinct parts:

| Part | On | Off |
| --- | --- | --- |
| Arrow | Active | Inactive |
| BG.sprite | `SelectedBG_raw` | Native normal background |
| Label font color | Dark | White |

The clips also animate BG geometry. Both contain no animation events. These
findings came from read-only prefab/clip inspection, not runtime animation sampling.
The controller's selection state is independent of its pointer Button layer.

The normal BG sprite is shared with the native category, text and slider rows.
Category rows use a direct `Background` child; text/slider/selection rows use `BG`.
This explains why looking for Image on the widget root did not tint the backgrounds.

For owned choices, override the drawn BG sprite while retaining native selection
animation, geometry, and checkmark activity. Collect the normal and selected
sprites from existing settings widgets using weak handles. If a sprite is not
available yet, leave the native presentation for that state until a bind/input
event supplies it. No bundles are loaded, cloned, modified, or retained by this
adapter. Rich text keeps labels light at rest, dark while pressed, and bold for
the selected value. The checkmark tint follows that contrast treatment.

Pointer feedback uses the existing native input-state transition event. The new
Windows x64 `Selectable.DoStateTransition` hook has an 805-byte PE unwind extent
at RVA `0x47A9650`, exceeding SPUD's 24-byte overwrite. No other mod hook owns
this target. It immediately passes through other control classes and only styles
selection rows tracked by the mod. It does not run through an Update hook.

Client SHA256:
`487af4bb9c697c353be9714359a97dddcece5dab872622a6c498a27bbfc44f40`.
The native UI remains omitted on other platforms; Windows hook evidence is not
proof of macOS hook fit. Runtime checks must verify release/drag-out behavior,
shortcut readback, selected checkmarks, and restoration of pooled native widgets.
