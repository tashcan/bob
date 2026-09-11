# Checked startup config output

`file_transaction::Write` is a synchronous local-file primitive. It is used by
Config's private `SaveStartup` for its two existing startup outputs. It is not a
runtime settings API or a substitute for a future asynchronous coordinator.

Initial user-config creation uses CreateOnly. A concurrent creator wins without
being overwritten. Generated runtime-vars output uses ReplaceSnapshot. TOML is
serialized and parsed before file staging; the existing warning header and Windows
text-mode line endings are retained. Save failure is reported without aborting
initialization. Existing user-config documents are not rewritten by this change.

Transactions resolve supported symlink targets, reject hard-linked/non-regular
targets, attempt a cooperative sibling `.lock` once, and stage at most4 MiB in an
exclusively created same-directory transaction folder. Lock files remain in place;
their lifetime is separate from OS lock ownership. Staging names are bounded and
exclusive, not secure by secrecy. Windows transaction folders restrict access to
owner/administrators/SYSTEM; macOS creates them with mode0700. Unexpected staging
collisions are never opened or truncated.

Writes, flush and close are checked. Windows existing-file replacement uses
ReplaceFileW with an owned backup and without ignore-ACL-error flags; new files
use a move without replacement. macOS preserves metadata using fcopyfile and uses
same-filesystem rename, with exclusive rename for creation and a parent-directory
fsync after commit. Platform documentation is not a power-loss test.

Results distinguish NotCommitted, Conflict, Busy, Committed,
DurabilityUnverified and RecoveryRequired. Committed means the supported local
operation completed, not guaranteed survival of every hardware/power failure.
Windows's unsupported ReplaceFile write-through flag is not used. A post-commit
directory-flush error does not pretend the write failed before commit.

Documented Windows partial replacement failures retain the transaction folder and
its old/new files for recovery. Cleanup touches only owned filenames, never sweeps
a directory, and records cleanup errors. There is no automatic startup recovery or
rollback into a destination another writer may have changed. An interruption may
leave a private staging folder; recovery/retention coordination is required before
this primitive supports live user-document edits.

## Boundaries

- Callers supply trusted paths already routed by File::MakePath. This is not an
  arbitrary-path sandbox or a destination registry. Windows UNC/device/alternate
  stream destinations are rejected in this first local-filesystem implementation.
- Canonicalization and no-follow checks are useful validation, not exclusion of an
  adversary swapping parent directories or links. Uncooperative same-permission
  writers remain outside the guarantee. Only participants using the same lock are
  serialized; canonical alias/case behavior still needs platform fixtures.
- No conflict-aware edits, schema merge, bounded runtime queue, UI completion,
  shutdown coordinator, cloud persistence or log-writer replacement is introduced.
- Mod-state PR267 retains its existing API/implementation. Adoption of these shared
  primitives is a separate explicit migration after platform validation.

## Tests

Run the isolated standalone fixture (never against a real game config):

```powershell
clang++ -std=c++23 -Wall -Wextra -Werror -I mods/src tests/file_transaction_test.cc -o file_transaction_test.exe -ladvapi32
./file_transaction_test.exe
```

On macOS omit `-ladvapi32` and use the native compiler. The fixture covers create
races, replacement, partial staged output, injected pre-commit failure boundaries,
post-commit uncertainty, size/path rejection and hard links. Windows additionally
tests held locks/targets and emulates the documented1177 partial-replacement
postcondition, checking that the old copy survives. This is not a naturally
triggered OS1177 or power-loss test. Symlink creation explicitly reports a skip
when unavailable. Production builds contain no fault-injection interface.
On macOS, FIFO destination and lock fixtures run in child processes with a
three-second deadline, verifying rejection without blocking on a FIFO peer.

Native fixture CI runs Windows and macOS ARM/Intel. Fixtures check preservation of
a restrictive Windows DACL and macOS mode/owner/group/extended attribute, plus
contention through a symlink while the canonical lock is held. These are specific
cases, not exhaustive ACL or filesystem coverage. Interrupted-process recovery
and storage behavior under real faults still require follow-up evidence. Runtime
latency/queue tests belong to the asynchronous phase.
