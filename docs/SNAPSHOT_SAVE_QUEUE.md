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
