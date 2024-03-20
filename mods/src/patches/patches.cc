#include "patches.h"
#include "version.h"

#include <spud/detour.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#if _WIN32
#include <Windows.h>
#endif

#include <array>

void InstallUiScaleHooks();
void InstallZoomHooks();
void InstallBuffFixHooks();
void InstallFreeResizeHooks();
void InstallToastBannerHooks();
void InstallPanHooks();
void InstallImproveResponsivenessHooks();
void InstallHotkeyHooks();
void InstallTestPatches();
void InstallMiscPatches();
void InstallChatPatches();
void InstallResolutionListFix();
void InstallTempCrashFixes();
void InstallSyncPatches();

__int64 __fastcall il2cpp_init_hook(auto original, const char* domain_name)
{
  auto r = original(domain_name);

#ifndef NDEBUG && _WIN32
  AllocConsole();
  FILE* fp;
  freopen_s(&fp, "CONOUT$", "w", stdout);
#endif

  auto file_logger = spdlog::basic_logger_mt("default", "community_patch.log", true);
  auto sink        = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  file_logger->sinks().push_back(sink);
  spdlog::set_default_logger(file_logger);

  spdlog::info("Initializing STFC Community Patch ({})", VER_PRODUCT_VERSION_STR);

  const std::pair<std::string, void (*)()> patches[] = {
      {"UiScaleHooks", InstallUiScaleHooks},
      {"ZoomHooks", InstallZoomHooks},
      {"BuffFixHooks", InstallBuffFixHooks},
      {"ToastBannerHooks", InstallToastBannerHooks},
      {"PanHooks", InstallPanHooks},
      {"ImproveResponsivenessHooks", InstallImproveResponsivenessHooks},
      {"HotkeyHooks", InstallHotkeyHooks},
      {"FreeResizeHooks", InstallFreeResizeHooks},
      {"TempCrashFixes", InstallTempCrashFixes},
      {"TestPatches", InstallTestPatches},
      {"MiscPatches", InstallMiscPatches},
      {"ChatPatches", InstallChatPatches},
      {"ResolutionListFix", InstallResolutionListFix},
      {"SyncPatches", InstallSyncPatches},
  };

  auto patch_count = 0;
  auto patch_total = sizeof(patches) / sizeof(patches[0]);
  for (const auto& [patch_name, patch_func] : patches) {
    patch_count++;
    spdlog::info("Patching {:>2} of {} ({})", patch_count, patch_total, patch_name);
    patch_func();
  }

  spdlog::info("");
#if VERSION_PATCH
  spdlog::info("Loaded beta version {}.{}.{} (Patch {})", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION,
               VERSION_PATCH);
  spdlog::info("");
  spdlog::info("NOTE: Beta versions may have unexpected bugs and issues");
#else
  spdlog::info("Loaded release version {}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
#endif

  spdlog::info("");
  spdlog::info("Please see https://github.com/tashcan/bob for latest configuration help, examples and future releases");
  spdlog::info("or visit the STFC Community Mod discord server at https://discord.gg/PrpHgs7Vjs");
  spdlog::info("");

  return r;
}

void ApplyPatches()
{
  auto assembly = LoadLibraryA("GameAssembly.dll");

  try {
#ifndef NDEBUG
    const auto log_level = spdlog::level::trace;
#else
    const auto log_level = spdlog::level::info;
#endif

    spdlog::set_level(log_level);
    spdlog::flush_on(log_level);

    auto n = GetProcAddress(assembly, "il2cpp_init");
    SPUD_STATIC_DETOUR(n, il2cpp_init_hook);
  } catch (...) {
    // Failed to Apply at least some patches
  }
}
