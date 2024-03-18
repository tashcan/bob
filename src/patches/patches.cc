#include "patches.h"

#include <Windows.h>

#include <filesystem>

#include <spud/detour.h>

#include <libil2cpp/il2cpp-api-types.h>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "Dbghelp.h"

#include "version.h"
#include <map>

#define DO_API(r, n, p) using n##_t = r(*) p;
#define DO_API_NO_RETURN(r, n, p) DO_API(r, n, p)
#include <libil2cpp/il2cpp-api-functions.h>
#undef DO_API
#undef DO_API_NO_RETURN

void InstallUiScaleHooks();
void InstallZoomHooks();
void InstallBuffFixHooks();
void InstallFreeResizeHooks();
void InstallToastBannerHooks();
void InstallPanHooks();
void InstallWebRequestHooks();
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

#ifndef NDEBUG
  AllocConsole();
  FILE* fp;
  freopen_s(&fp, "CONOUT$", "w", stdout);
#endif

  auto file_logger = spdlog::basic_logger_mt("default", "community_patch.log", true);
  auto sink        = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  file_logger->sinks().push_back(sink);
  spdlog::set_default_logger(file_logger);

  spdlog::info("Initializing STFC Community Patch ({})", VER_PRODUCT_VERSION_STR);

  const std::map<std::string, void*> patches = {
      {"UiScaleHooks", InstallUiScaleHooks},
      {"ZoomHooks", InstallZoomHooks},
      {"BuffFixHooks", InstallBuffFixHooks},
      {"ToastBannerHooks", InstallToastBannerHooks},
      {"PanHooks", InstallPanHooks},
      {"WebRequestHooks", InstallWebRequestHooks},
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
  auto patch_total = patches.size();
  for (auto& kv : patches) {
    auto patch_name = kv.first;
    auto patch_func = kv.second;
    patch_count++;
    spdlog::info("Patching {:>2} of {} ({})", patch_count, patch_total, patch_name);
    reinterpret_cast<void (*)()>(patch_func)();
  }

  spdlog::info("");
#if VERSION_PATCH
  spdlog::info("Loaded beta verison {}.{}.{} Patch {}", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH);
  spdlog::info("");
  spdlog::info("NOTE: Beta versions may have unexpected bugs and issues, please visit Ripper's discord");
  spdlog::info("      for hints and help");
#else
  spdlog::info("Loaded release verison {}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
#endif

  spdlog::info("");
  spdlog::info("Please see https://github.com/tashcan/bob for latest configuration help, examples and future releases");
  spdlog::info("");

  return r;
}

void CreateMiniDump(EXCEPTION_POINTERS* pep)
{
  // Open the file
  typedef BOOL (*PDUMPFN)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
                          PMINIDUMP_EXCEPTION_INFORMATION   ExceptionParam,
                          PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                          PMINIDUMP_CALLBACK_INFORMATION    CallbackParam);

  HANDLE hFile =
      CreateFileW(L"Minidump.dmp", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  HMODULE h   = ::LoadLibraryW(L"DbgHelp.dll");
  PDUMPFN pFn = (PDUMPFN)GetProcAddress(h, "MiniDumpWriteDump");

  if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
    MINIDUMP_EXCEPTION_INFORMATION mdei;

    mdei.ThreadId          = GetCurrentThreadId();
    mdei.ExceptionPointers = pep;
    mdei.ClientPointers    = TRUE;

    MINIDUMP_TYPE mdt = MiniDumpNormal;

    (*pFn)(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (pep != 0) ? &mdei : 0, 0, 0);

    CloseHandle(hFile);
  }
}

LONG WINAPI CrashHandler(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
  CreateMiniDump(ExceptionInfo);
  return EXCEPTION_EXECUTE_HANDLER;
}

void Patches::Apply()
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
