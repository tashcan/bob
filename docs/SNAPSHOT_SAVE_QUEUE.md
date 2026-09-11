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

`RequestStop` closes admission without waiting for storage. A submission overlapping
stop may already have passed its admission check; it remains an accounted ticket
and is cancelled by the worker if it has not started. `RunOne` produces explicit
CancelledBeforeStart completions for queued requests after stop. Already selected
work is in-flight and reports its actual result, even if stop arrives meanwhile.
The host must keep pumping until queued cancellation is accounted for.

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
