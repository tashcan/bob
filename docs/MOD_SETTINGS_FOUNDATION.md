# Boolean settings foundation (P1)

This is the controller and Fleet Commander preference adapter for a forthcoming
native settings UI. It does not yet insert rows or persist mod-owned TOML settings.

`settings/boolean_settings.h` is independent of Unity and storage. Definitions have
a stable ID, readable label, read callback, and immediate-write callback. Registry
IDs are unique and registration freezes on first lookup. All operations belong to
the constructing thread; there is no polling, background work or disk I/O.

Consumers must keep unknown state separate from a boolean. `ReadResult` carries
availability, an optional positive value, and an adapter generation. A snapshot
also carries controller identity, revision and lifecycle epoch. Passing a stale
or foreign snapshot rejects the request. `RenderScope` suppresses user-write
handling around native binding/refresh callbacks. Reentrant application is Busy.

Writes re-read before applying, skip already-satisfied values and verify readback.
Failed or uncertain writes never trigger an automatic reverse write. The returned
snapshot contains a fresh authoritative read when available; `Unverified` must
not be rendered as successful application. `AppliedVerified` is local verification,
not a claim of cloud durability.

## Fleet Commander adapter

`FleetCommanderConfirmationSetting()` provides one shared setting for the recovery
shortcut and future UI. ON means show confirmation. Reads use the existing
PersistentPrefsManager's `GetBool(key, false, false)`: the final false prevents
insertion of a missing preference while preserving the game's default. Writes use
the native FC setter. The adapter never instantiates managers, invokes abilities,
forces cloud saves, or enumerates other preferences.

Metadata is resolved lazily and checked before access. Two weak handles detect
replacement of the preference manager or saved-data object without retaining
account data. Unavailability invalidates the observed generation. One process-wide
root holds the constant, non-sensitive preference key; no work runs while idle.

P2 **must** wire `InvalidateFleetCommanderConfirmationSession()` to reliable
account/reload lifecycle notifications before retaining UI snapshots. Observed
object identity alone cannot prove that an account transition did not happen
between two reads. The current shortcut takes and consumes a fresh snapshot in
one game-thread operation; this change introduces no long-lived native UI snapshot.
Context teardown, widget ownership, native error presentation and per-platform UI
hook evidence are P2 work, not claims of this foundation.

The existing Ctrl+Alt+F8 shortcut remains one-way and keeps its input/config guards.
Its log distinguishes already-enabled, verified application and unverified failure.

## Standalone controller tests

Windows (clang++ with the installed C++ toolchain):

```powershell
clang++ -std=c++23 -Wall -Wextra -Werror -I mods/src tests/boolean_settings_test.cc -o boolean_settings_test.exe
./boolean_settings_test.exe
```

On Unix, use the equivalent compiler invocation with `-pthread`. Keep executables
outside tracked source. These tests cover the pure state machine, not native ABI,
account lifecycle wiring, frame timings, or cloud persistence.

Before extending the UI, measure the baseline and candidate with the same scene,
FPS cap and diagnostics: no scheduled closed-menu work or per-frame allocations;
initial target <=1 ms added normal bind/refresh work at p95, <=2 ms per normal
operation. These are proposed UI acceptance budgets, not measured P1 results.
