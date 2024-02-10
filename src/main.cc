#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

#include <filesystem>

#include "patches/patches.h"

void VersionDllInit();

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID /*lpReserved*/)
{
  std::filesystem::path game_path;

  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      TCHAR szFileName[MAX_PATH];
      GetModuleFileName(NULL, szFileName, MAX_PATH);

      game_path = szFileName;

      if (!game_path.filename().generic_wstring().starts_with(L"prime")) {
        return TRUE;
      }

      // This is just for debugging
#ifndef NDEBUG
      AllocConsole();
      FILE* fp;
      freopen_s(&fp, "CONOUT$", "w", stdout);
#endif
      // Since we are replacing version.dll, need the proper forwards
      VersionDllInit();
      Patches::Apply();
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

void* operator new[](size_t size, const char* /*name*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/,
                     int /*line*/)
{
  return malloc(size);
}

void* operator new[](size_t size, size_t /*alignment*/, size_t /*alignmentOffset*/, const char* /*name*/, int /*flags*/,
                     unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
  return malloc(size);
}