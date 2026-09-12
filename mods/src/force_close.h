#pragma once
#include "snapshot_save_host.h"

namespace persistence
{
#if defined(_WIN32)
// Owner thread only, after Start returns. Host must outlive the process. Once
// called, no other caller may start/reap the host. Does not call Unity.
void ForceClose(SnapshotSaveHost* host) noexcept;
#endif
}
