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

  spdlog::info("Initializing STFC Community Patch ({})", VER_PRODUCT_VERSION_STR);

  spdlog::info("Step 1");
  InstallUiScaleHooks();
  spdlog::info("Step 2");
  InstallZoomHooks();
  spdlog::info("Step 3");
  InstallBuffFixHooks();
  spdlog::info("Step 4");
  InstallToastBannerHooks();
  spdlog::info("Step 5");
  InstallPanHooks();
  spdlog::info("Step 6");
  InstallWebRequestHooks();
  spdlog::info("Step 7");
  InstallImproveResponsivenessHooks();
  spdlog::info("Step 8");
  InstallHotkeyHooks();
  spdlog::info("Step 9");
  InstallFreeResizeHooks();
  spdlog::info("Step 10");
  InstallTempCrashFixes();
  InstallTestPatches();
  InstallMiscPatches();
  InstallChatPatches();
  InstallResolutionListFix();
  InstallSyncPatches();
  spdlog::info("Finished");

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
  TCHAR szFileName[MAX_PATH];
  GetModuleFileName(NULL, szFileName, MAX_PATH);

  std::filesystem::path game_path = szFileName;

  if (!game_path.filename().generic_wstring().starts_with(L"prime")) {
    return;
  }

  auto assembly = LoadLibraryA("GameAssembly.dll");

  try {
    auto file_logger = spdlog::basic_logger_mt("default", "community_patch.log", true);
    spdlog::set_default_logger(file_logger);

    file_logger->sinks().push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

#ifndef NDEBUG
    const auto log_level = spdlog::level::debug;
    const auto str_level = "DEBUG";
#else
    const auto log_level = spdlog::level::info;
    const auto str_level = "info";
#endif

    spdlog::set_level(log_level);
    spdlog::flush_on(log_level);
    spdlog::warn("Setting log level to {}", str_level);

    auto n = GetProcAddress(assembly, "il2cpp_init");
    SPUD_STATIC_DETOUR(n, il2cpp_init_hook);
    ;
  } catch (...) {
    // Failed to Apply at least some patches
  }
}