#pragma once
#include "toml_editor.h"
#include <chrono>
#include <toml++/toml.h>

namespace runtime_config
{
// Startup only: retain the semantic disk value as the optimistic comparison base.
void Configure(const toml::table& loaded);
void Install();
void SaveWarpMode(const char* mode) noexcept;
void SaveSetting(const char* section, const char* key, config_edit::Value value,
                 std::chrono::milliseconds delay = {}) noexcept;
#if _WIN32
void ForceClose() noexcept;
#endif
} // namespace runtime_config
