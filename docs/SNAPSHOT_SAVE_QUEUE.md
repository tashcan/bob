# Bounded snapshot scheduling core

`persistence::SnapshotSaveQueue` is an internal, single-destination scheduling
core for generated snapshots. It is not yet a runtime service and has no game
callers. User TOML editing, JSON schema updates, log appends, and cloud preferences
require separate adapters; they must not submit whole-document replacements here.

## Admission and completion

Trusted setup constructs one queue per canonical destination, outside gameplay.
The eventual host must enforce that unique ownership and a global budget across
destinations. Construction resolves the path and can throw. Submission takes a
revision and an owned string, never an arbitrary path or game object.

`TrySubmit` uses a try-lock and performs no serialization, allocation, filesystem
work, or wait for capacity. A successful submission transfers the buffer; a
rejection leaves it intact. This bounds admission work, not preparation of the
snapshot: future adapters must keep serialization and large allocation off input
handlers as well. Neither this test nor a try-lock proves a frame-time budget.

Limits are eight outstanding tickets, 4 MiB of content per request, and 8 MiB of
retained string capacity per queue (plus fixed slot/string overhead and backend
transaction allocations). In-flight payloads count until released. A string with
small content and huge reserved capacity cannot bypass the byte limit. Finished
results retain their slots until consumed, so an abandoned page cannot cause an
unbounded completion backlog or silently discard accepted outcomes.

Admission returns Accepted(ticket), Busy, InvalidRequest, StaleRevision, or Stopping.
Accepted does not mean saved. Revision numbers are nonzero, strictly increasing
within this queue's lifetime, assigned by one adapter authority; they are snapshot
sequence numbers, not disk hashes or external-editor conflict detection. Failed
admission never advances the sequence. Retrying an accepted-but-failed snapshot
requires a fresh revision. Recreating the queue requires a new consumer session;
tickets from different instances must never be compared as global identities.

Worker execution selects admitted tickets in order. It holds no admission lock
while executing storage or freeing payloads, and only one `RunOne` can execute at
a time. `TryTakeCompletion` moves a result to the polling owner without callbacks.
An empty poll can also mean transient lock contention. The future UI adapter must
consume results centrally and check its page/session generation before applying
any continuation; closing a page must not destroy the queue.

## Stop and lifetime

`RequestStop(CancelQueued)` (the default) closes admission without waiting for storage. A submission overlapping
stop may already have passed its admission check; it remains an accounted ticket
and is cancelled by the worker if it has not started. `RunOne` produces explicit
CancelledBeforeStart completions for queued requests after stop. Already selected
work is in-flight and reports its actual result, even if stop arrives meanwhile.
The host must keep pumping until queued cancellation is accounted for.

`RequestStop(DrainAccepted)` instead closes admission while finishing accepted
snapshots in ticket order. Every accepted ticket still gets its actual completion;
drain is not a promise that all writes succeed. Cancellation is sticky: an explicit
CancelQueued request, before or after a drain request, cancels work not yet selected.
A later drain request cannot re-enable those writes. RecoveryRequired also forces
cancellation, even during a drain; already selected storage reports its result.

RecoveryRequired (including an unexpected backend exception whose commit point is
unknown) closes admission and cancels later queued snapshots. Known pre-commit
failures and committed-but-durability-unverified results retain their distinct
outcomes; this layer does not retry or roll back automatically.

This core owns no thread and registers no destructor/exit hook. Its owner must
keep it alive until producers, consumer and worker calls have ended. Destroying it
while another thread uses it is invalid. There is no implicit join, detach, timer,
poll loop, game hook, or claim of safe live unload in this patch.

Inspection found no coordinated storage shutdown in the current host: Windows
`DllMain` has an empty process-detach branch; macOS's injection constructor has no
matching teardown handler. Existing sync threads are not a save lifecycle contract.
Before runtime wiring, choose and validate module/worker ownership, cancellation
wakeup, completion draining, and actual process-exit/unload behavior. A join under
the loader lock or detached worker is not an acceptable default. Indefinitely
stalled OS I/O prevents promising both bounded unload and guaranteed completion.

## Isolated validation

```powershell
clang++ -std=c++23 -Wall -Wextra -Werror -I mods/src tests/snapshot_save_queue_test.cc -o snapshot_save_queue_test.exe
./snapshot_save_queue_test.exe
```

The test backend never touches config files. It holds storage behind an explicit
barrier while exercising admission, polling, a second worker, and stop. It covers
count/capacity bounds, stale revisions, retained completions, retry identity,
queued cancellation versus in-flight completion, and recovery-required closure.
A concurrent producer/worker/consumer fixture checks 500 revisions through slot
reuse with exactly one ordered completion per admission. A watchdog terminates a
hung fixture. This validates scheduling logic, not native
storage durability, production frame latency, UI lifetime integration, or host
shutdown. Those remain separate gates, alongside native macOS transaction tests.

## Explicit worker owner

`SnapshotSaveWorker` owns one queue and one joinable thread. It is an internal
component, not an application singleton; no game path constructs it yet. Trusted
construction resolves the destination and starts the worker, and may throw before
any request is accepted. Start it outside loader callbacks and input handling.
The host must limit owner instances and enforce unique destination enrollment;
this component does not create a registry or a globally bounded worker pool.

Accepted admission wakes the worker with a coalesced atomic notification. There
is no idle polling or unbounded notification counter. The worker clears the wake
flag before draining, retaining a notification that arrives during or after the
drain. Only the worker waits. Producer admission uses try-locks; snapshot
preparation and actual platform notification latency still require measurement.

`RequestStop(mode)` immediately closes owner admission and, for CancelQueued,
queue selection, then wakes an idle worker. The worker synchronizes with any submission already inside
admission before its final cancellation drain, preventing a late accepted ticket
from being left behind after worker exit. In-flight storage finishes normally;
pending work follows the requested drain/cancel mode, and all completions remain available for central
consumption after join. A settings page closing does not stop this owner.

The sole lifecycle owner calls `StopAndJoin(mode)` explicitly outside gameplay and
loader callbacks. It may wait indefinitely for an in-flight OS call, and must not
run concurrently or on the worker. The mode argument is required so joining an
earlier drain cannot silently select default cancellation. `WorkEnded` means queue execution ended, not
that the native thread has fully exited: it never authorizes destruction or
unload. Joining and quiescing all producer/consumer calls are required first.
Like `std::thread`, destroying an unjoined owner terminates; the destructor does
not silently detach or hide a blocking join. It must not be installed as a
page-local or static-destructor-managed service.

The host integration gate remains open: identify a pre-teardown supervisor that
can join off the game thread while keeping the mod loaded, drain outcomes, and
validate process exit and explicit unload separately. A forced process exit is
an interruption with potentially retained staging, not a successful final flush.
Do not wire this owner into startup until that contract is satisfied on each
enabled platform. Windows process detach is too late for this coordination, as
other threads may already have been terminated. See [Microsoft's DllMain
contract](https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain) and the
[C++ thread lifetime contract](https://eel.is/c%2B%2Bdraft/thread.thread.class).

`tests/snapshot_save_worker_test.cc` exercises actual worker startup, stalled
storage and non-waiting stop, retained completions after join, 500 wake cycles,
and admission racing with stop. A test-only barrier also pauses admission after
its final stop check, proving a late accepted ticket is cancelled before join
returns. It uses an isolated backend and a watchdog, not
game files. Compile with the same standalone command as the queue fixture using
the worker test filename. The Persistence fixtures workflow runs all three
fixtures on native Windows, macOS ARM and macOS Intel; adding the workflow does
not itself count as a successful CI run.

Drain fixtures additionally hold the first write behind a barrier and verify that
the second accepted write finishes, that cancellation before/after drain wins,
and that known failure, durability uncertainty and recovery-required results keep
their distinct meaning. Late admission across the final stop check is tested in
both modes. This supplies a worker policy for future orderly quit integration;
it does not change F10, defer Unity quitting, or wire any game callbacks.

## Bounded registered service

Enrollment reserves each destination together with its derived `.lock` identity.
A destination cannot replace another destination's lock file, even when declared
in reverse order or with an equivalent spelling. Replacing a held lock could
otherwise let another process lock a new inode while the old inode remains held.

`SnapshotSaveService` owns one process-wide lease and up to four registered
generated-snapshot destinations. Construction validates every destination before
starting any worker. A second live service is refused, including after the first
service joins but before it is destroyed. Partial startup failure joins already
created workers before releasing the lease. Construction and explicit teardown
belong to the supervisor/startup scope, never a settings/input or loader callback.

Each destination has one worker and its existing eight-ticket/eight-MiB capacity
limit. The process service therefore permits at most four worker threads,
32 outstanding tickets and32MiB of retained payload capacity. A stalled destination
does not stop other destinations from progressing; each file remains serialized.
These limits cover service-owned requests, not buffers callers prepare before
submission. Idle workers sleep. Existing low-level queue/worker types remain
internal implementation components, not alternate feature-facing save APIs.

Trusted startup supplies the path list once. Callers receive an opaque destination
handle and submit only revisions and owned bytes; submission never resolves paths
or serializes config. A monotonically assigned session prevents handles from a
destroyed service being accepted by its replacement. Invalid handles preserve the
input buffer. Ticket numbers are scoped to the destination and service session,
not globally unique. Consumers must retain that identity with their requests.

Enrollment requires existing parents, canonicalizes supported aliases and rejects
duplicate/ambiguous destinations and existing nonregular or hardlinked files.
Windows compares leaf names ordinally without case. Other platforms conservatively
reject case-equivalent ASCII names, and multiple names containing non-ASCII bytes
in the same parent, rather than guess volume case/normalization rules for absent
files. A single Unicode destination is supported. This may reject distinct files
on a case-sensitive volume. The native transaction still performs write-time path
validation; this registry is not protection against uncooperative directory/link
replacement or malicious native code bypassing the internal API.

Windows also rejects components ending in dots/spaces before and after
canonicalization, because Win32 target normalization could otherwise diverge
from the sibling lock's identity. An absent leaf containing a tilde is rejected
conservatively: creating another destination on an 8.3-enabled volume could turn
it into that file's short alias. Existing aliases remain subject to filesystem
equivalence checks. The rule does not depend on permission to query volume policy.

Stopping first closes service admission, then requests every worker's stop before
joining any worker. Completions remain readable after join. The host must quiesce
callers before destruction; a destination handle does not extend service lifetime.
Destruction without explicit joining still terminates rather than hiding a blocking
join or allowing a detached thread to outlive the module.

Shutdown policy: distinguish the attempt's result from whether worker shutdown
finished. A failed save remains failed; once workers terminate, save failure alone
must not veto process exit. In-flight OS I/O cannot safely be cancelled by a queue
flag. Preserve transaction-owned recovery artifacts when an outcome is uncertain.
The Windows mod Quit shortcut (F10 by default) force-closes independently of Unity
after up to 500 ms of best-effort save cleanup. With no active supervisor it exits
immediately. Otherwise a native deadline thread requests cancellation of queued
writes and waits on a duplicated supervisor handle. It terminates the process as
soon as that handle signals or the grace period expires; it never joins a writer
or needs another Update callback. Cancellation can escalate an existing normal
drain. Active writes finish normally during the grace period. Thread/handle setup
failure falls back to immediate termination. The deadline begins when the shortcut
is received; OS scheduling still affects when termination actually executes.
F10 detection itself remains in the existing game input handler.
The macOS shortcut continues to call `PrimeApp::Quit`. Result presentation
and targeted user-TOML editing remain separate integration work; no current feature
registers a runtime destination or submits runtime saves.

The service fixture exercises exclusive ownership, constructor rollback after a
worker has started, destination ambiguity, stale handles across replacement,
bounded retained tickets, independent progress under stalled storage, and admission
closure with retained outcomes after join. It uses an isolated backend and the
native CI matrix; it is not game runtime or filesystem interruption evidence.

## Lazy game host and shutdown

`runtime_snapshots` is the internal game-facing registration/submission adapter.
Trusted registration takes up to four paths on the observed Update thread. It
launches a native supervisor; the supervisor enrolls paths and constructs the
existing service and workers. Status remains Starting until enrollment completes.
Registration is one-shot, including failed attempts. Ordinary consumers receive
destination handles and never supply a path with a save request. Preparing bytes
is still an adapter responsibility; this is not a preserving user-TOML editor.

Initialization runs after `il2cpp_init`, outside `DllMain`. It registers with the
existing ScreenManager.Update owner but starts no thread. When inactive, its
callback does no logging, timing capture or filesystem work. The quit detour is
installed during initialization so it can close registration even when a quit
request arrives before the first consumer. Admission requires an
observed Update callback, the build261 Windows x64 method RVA, full instruction
fingerprint, and native unwind extent. Mismatch leaves runtime saves unavailable.
macOS and other architectures reject registration until their shutdown seam is
validated; this does not disable the synchronous checked startup outputs.

Registration, quit votes, native-stop observation and automatic resume consumption
share one atomic lifecycle state. Registration either claims ownership before quit
is permitted or is rejected permanently. A delayed vote cannot re-arm an automatic
resume that has already been consumed.

An allowed game quit vote closes admission and wakes the supervisor. The game
callback returns without waiting on storage. The supervisor removes borrowed
service access under its mutex, drains accepted work, joins every worker, retains
unconsumed completions, and destroys the service. Producer and completion access
use try-lock operations; consumers cannot race service destruction. Retained
completions preserve their destination index and ticket, including failed saves.
No game object or callback crosses onto the supervisor or workers.

The Update callback polls the native supervisor handle with a zero timeout during
shutdown. A published Stopped state alone cannot authorize quitting. After actual
native thread exit it closes the handle, releases the temporary loader reference,
and requests Unity quit once. The game's real subscriber vote is preserved, and
a later genuine veto cancels automatic resumption; a resumed veto does not cause
an automatic retry loop. The stopped service remains
unavailable for new requests if the game vetoes exit; it is not automatically restarted.
Failed saves do not veto exit once workers have terminated. Indefinitely stalled
OS I/O can still delay normal exit: no timer pretends to cancel an in-flight write.

The supervisor uses an ordinary scoped module reference, not a permanent module
PIN. The small adapter control block and detour have process lifetime; there is no
static thread destructor, detached worker, or loader-lock join. Hot module unloading
is unsupported, as it is for the installed game hooks. Force termination/crash can
bypass this lifecycle; no power-loss or guaranteed shutdown-duration claim is made.

The Windows host fixture verifies stop during startup, concurrent admission versus
shutdown, retained failure/success outcomes, rejection after stop, and the interval
between service destruction and native supervisor exit. Other platform fixtures
verify the quit gate and unsupported-host rejection only. These are isolated native
tests, not evidence of exact-artifact F10/window-X runtime behavior.

The Windows force-close fixture launches isolated child processes with no further
owner updates after F10. It checks idle exit, an indefinitely blocked writer, an
active writer that finishes, and escalation of normal draining to queued-write
cancellation. Children must terminate through the force-close path; the queued
second write must never execute. This is native process evidence, not a live-game
input or forced-write durability test.

Native lifecycle references: Microsoft documents the retained caller-owned handle
and automatic CRT cleanup for [_beginthreadex](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/beginthread-beginthreadex),
zero-timeout [WaitForSingleObject](https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject),
and scoped versus pinned [module references](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandleexw).
