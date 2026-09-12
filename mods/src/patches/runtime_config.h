#pragma once
#include <toml++/toml.h>

namespace runtime_config
{
// Startup only: retain the semantic disk value as the optimistic comparison base.
void Configure(const toml::table& loaded);
void Install();
void SaveWarpMode(const char* mode) noexcept;
#if _WIN32
void ForceClose() noexcept;
#endif
} // namespace runtime_config
