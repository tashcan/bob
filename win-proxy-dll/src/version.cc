#include <Windows.h>

typedef DWORD(WINAPI *ORIG_FUNCTION_VerFindFileA)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT, LPSTR, PUINT);
ORIG_FUNCTION_VerFindFileA orig_VerFindFileA;
DWORD
APIENTRY
VerFindFileA(_In_ DWORD uFlags, _In_ LPCSTR szFileName, _In_opt_ LPCSTR szWinDir, _In_ LPCSTR szAppDir,
             _Out_writes_(*puCurDirLen) LPSTR szCurDir, _Inout_ PUINT puCurDirLen,
             _Out_writes_(*puDestDirLen) LPSTR szDestDir, _Inout_ PUINT puDestDirLen)
{
  return (orig_VerFindFileA)(uFlags, szFileName, szWinDir, szAppDir, szCurDir, puCurDirLen, szDestDir, puDestDirLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerFindFileW)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT, LPWSTR, PUINT);
ORIG_FUNCTION_VerFindFileW orig_VerFindFileW;
DWORD
APIENTRY
VerFindFileW(_In_ DWORD uFlags, _In_ LPCWSTR szFileName, _In_opt_ LPCWSTR szWinDir, _In_ LPCWSTR szAppDir,
             _Out_writes_(*puCurDirLen) LPWSTR szCurDir, _Inout_ PUINT puCurDirLen,
             _Out_writes_(*puDestDirLen) LPWSTR szDestDir, _Inout_ PUINT puDestDirLen)
{
  return (orig_VerFindFileW)(uFlags, szFileName, szWinDir, szAppDir, szCurDir, puCurDirLen, szDestDir, puDestDirLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerInstallFileA)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT);
ORIG_FUNCTION_VerInstallFileA orig_VerInstallFileA;
DWORD
APIENTRY
VerInstallFileA(_In_ DWORD uFlags, _In_ LPCSTR szSrcFileName, _In_ LPCSTR szDestFileName, _In_ LPCSTR szSrcDir,
                _In_ LPCSTR szDestDir, _In_ LPCSTR szCurDir, _Out_writes_(*puTmpFileLen) LPSTR szTmpFile,
                _Inout_ PUINT puTmpFileLen)
{
  return (orig_VerInstallFileA)(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile,
                                puTmpFileLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerInstallFileW)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT);
ORIG_FUNCTION_VerInstallFileW orig_VerInstallFileW;
DWORD
APIENTRY
VerInstallFileW(_In_ DWORD uFlags, _In_ LPCWSTR szSrcFileName, _In_ LPCWSTR szDestFileName, _In_ LPCWSTR szSrcDir,
                _In_ LPCWSTR szDestDir, _In_ LPCWSTR szCurDir, _Out_writes_(*puTmpFileLen) LPWSTR szTmpFile,
                _Inout_ PUINT puTmpFileLen)
{
  return (orig_VerInstallFileW)(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile,
                                puTmpFileLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoSizeA)(LPCSTR, LPDWORD);
ORIG_FUNCTION_GetFileVersionInfoSizeA orig_GetFileVersionInfoSizeA;
DWORD
APIENTRY
GetFileVersionInfoSizeA(_In_ LPCSTR       lptstrFilename, /* Filename of version stamped file */
                        _Out_opt_ LPDWORD lpdwHandle      /* Information for use by GetFileVersionInfo */
)
{
  return (orig_GetFileVersionInfoSizeA)(lptstrFilename, lpdwHandle);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoSizeW)(LPCWSTR, LPDWORD);
ORIG_FUNCTION_GetFileVersionInfoSizeW orig_GetFileVersionInfoSizeW;
DWORD
APIENTRY
GetFileVersionInfoSizeW(_In_ LPCWSTR      lptstrFilename, /* Filename of version stamped file */
                        _Out_opt_ LPDWORD lpdwHandle      /* Information for use by GetFileVersionInfo */
)
{
  return (orig_GetFileVersionInfoSizeW)(lptstrFilename, lpdwHandle);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoA)(LPCSTR, DWORD, DWORD, LPVOID);
ORIG_FUNCTION_GetFileVersionInfoA orig_GetFileVersionInfoA;
BOOL APIENTRY                     GetFileVersionInfoA(_In_ LPCSTR      lptstrFilename, /* Filename of version stamped file */
                                                      _Reserved_ DWORD dwHandle, /* Information from GetFileVersionSize */
                                                      _In_ DWORD       dwLen,    /* Length of buffer for info */
                                                      _Out_writes_bytes_(dwLen) LPVOID lpData /* Buffer to place the data structure */
                    )
{
  return (orig_GetFileVersionInfoA)(lptstrFilename, dwHandle, dwLen, lpData);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoW)(LPCWSTR, DWORD, DWORD, LPVOID);
ORIG_FUNCTION_GetFileVersionInfoW orig_GetFileVersionInfoW;
BOOL APIENTRY GetFileVersionInfoW(_In_ LPCWSTR     lptstrFilename,        /* Filename of version stamped file */
                                  _Reserved_ DWORD dwHandle,              /* Information from GetFileVersionSize */
                                  _In_ DWORD       dwLen,                 /* Length of buffer for info */
                                  _Out_writes_bytes_(dwLen) LPVOID lpData /* Buffer to place the data structure */
)
{
  return (orig_GetFileVersionInfoW)(lptstrFilename, dwHandle, dwLen, lpData);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoSizeExA)(DWORD, LPCSTR, LPDWORD);
ORIG_FUNCTION_GetFileVersionInfoSizeExA orig_GetFileVersionInfoSizeExA;
DWORD APIENTRY GetFileVersionInfoSizeExA(_In_ DWORD dwFlags, _In_ LPCSTR lpwstrFilename, _Out_ LPDWORD lpdwHandle)
{
  return (orig_GetFileVersionInfoSizeExA)(dwFlags, lpwstrFilename, lpdwHandle);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoSizeExW)(DWORD, LPCWSTR, LPDWORD);
ORIG_FUNCTION_GetFileVersionInfoSizeExW orig_GetFileVersionInfoSizeExW;
DWORD APIENTRY GetFileVersionInfoSizeExW(_In_ DWORD dwFlags, _In_ LPCWSTR lpwstrFilename, _Out_ LPDWORD lpdwHandle)
{
  return (orig_GetFileVersionInfoSizeExW)(dwFlags, lpwstrFilename, lpdwHandle);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoExA)(DWORD, LPCSTR, DWORD, DWORD, LPVOID);
ORIG_FUNCTION_GetFileVersionInfoExA orig_GetFileVersionInfoExA;
BOOL APIENTRY GetFileVersionInfoExA(_In_ DWORD dwFlags, _In_ LPCSTR lpwstrFilename, _Reserved_ DWORD dwHandle,
                                    _In_ DWORD dwLen, _Out_writes_bytes_(dwLen) LPVOID lpData)
{
  return (orig_GetFileVersionInfoExA)(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoExW)(DWORD, LPCWSTR, DWORD, DWORD, LPVOID);
ORIG_FUNCTION_GetFileVersionInfoExW orig_GetFileVersionInfoExW;
BOOL APIENTRY GetFileVersionInfoExW(_In_ DWORD dwFlags, _In_ LPCWSTR lpwstrFilename, _Reserved_ DWORD dwHandle,
                                    _In_ DWORD dwLen, _Out_writes_bytes_(dwLen) LPVOID lpData)
{
  return (orig_GetFileVersionInfoExW)(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerLanguageNameA)(DWORD, LPSTR, DWORD);
ORIG_FUNCTION_VerLanguageNameA orig_VerLanguageNameA;
DWORD
APIENTRY
VerLanguageNameA(_In_ DWORD wLang, _Out_writes_(cchLang) LPSTR szLang, _In_ DWORD cchLang)
{
  return (orig_VerLanguageNameA)(wLang, szLang, cchLang);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerLanguageNameW)(DWORD, LPWSTR, DWORD);
ORIG_FUNCTION_VerLanguageNameW orig_VerLanguageNameW;
DWORD
APIENTRY
VerLanguageNameW(_In_ DWORD wLang, _Out_writes_(cchLang) LPWSTR szLang, _In_ DWORD cchLang)
{
  return (orig_VerLanguageNameW)(wLang, szLang, cchLang);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerQueryValueA)(LPCVOID, LPCSTR, LPVOID *, PUINT);
ORIG_FUNCTION_VerQueryValueA orig_VerQueryValueA;
BOOL APIENTRY                VerQueryValueA(_In_ LPCVOID pBlock, _In_ LPCSTR lpSubBlock,
                                            _Outptr_result_buffer_(_Inexpressible_("buffer can be PWSTR or DWORD*"))
                                                LPVOID *lplpBuffer,
                                            _Out_ PUINT puLen)
{
  return (orig_VerQueryValueA)(pBlock, lpSubBlock, lplpBuffer, puLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerQueryValueW)(LPCVOID, LPCWSTR, LPVOID *, PUINT);
ORIG_FUNCTION_VerQueryValueW orig_VerQueryValueW;
BOOL APIENTRY                VerQueryValueW(_In_ LPCVOID pBlock, _In_ LPCWSTR lpSubBlock,
                                            _Outptr_result_buffer_(_Inexpressible_("buffer can be PWSTR or DWORD*"))
                                                LPVOID *lplpBuffer,
                                            _Out_ PUINT puLen)
{
  return (orig_VerQueryValueW)(pBlock, lpSubBlock, lplpBuffer, puLen);
}

typedef void(WINAPI *ORIG_FUNCTION_GetFileVersionInfoByHandle)();
ORIG_FUNCTION_GetFileVersionInfoByHandle orig_GetFileVersionInfoByHandle;
void WINAPI                              GetFileVersionInfoByHandle()
{
  (orig_GetFileVersionInfoByHandle)();
}

void VersionDllInit()
{
  HMODULE hOriginalDll = LoadLibraryW(L"C:\\Windows\\system32\\version.dll");
  if (!hOriginalDll) {
    return;
  }
  orig_VerFindFileA        = (ORIG_FUNCTION_VerFindFileA)GetProcAddress(hOriginalDll, "VerFindFileA");
  orig_VerFindFileW        = (ORIG_FUNCTION_VerFindFileW)GetProcAddress(hOriginalDll, "VerFindFileW");
  orig_VerInstallFileA     = (ORIG_FUNCTION_VerInstallFileA)GetProcAddress(hOriginalDll, "VerInstallFileA");
  orig_VerInstallFileW     = (ORIG_FUNCTION_VerInstallFileW)GetProcAddress(hOriginalDll, "VerInstallFileW");
  orig_GetFileVersionInfoA = (ORIG_FUNCTION_GetFileVersionInfoA)GetProcAddress(hOriginalDll, "GetFileVersionInfoA");
  orig_GetFileVersionInfoW = (ORIG_FUNCTION_GetFileVersionInfoW)GetProcAddress(hOriginalDll, "GetFileVersionInfoW");
  orig_GetFileVersionInfoSizeA =
      (ORIG_FUNCTION_GetFileVersionInfoSizeA)GetProcAddress(hOriginalDll, "GetFileVersionInfoSizeA");
  orig_GetFileVersionInfoSizeW =
      (ORIG_FUNCTION_GetFileVersionInfoSizeW)GetProcAddress(hOriginalDll, "GetFileVersionInfoSizeW");
  orig_GetFileVersionInfoExA =
      (ORIG_FUNCTION_GetFileVersionInfoExA)GetProcAddress(hOriginalDll, "GetFileVersionInfoExA");
  orig_GetFileVersionInfoExW =
      (ORIG_FUNCTION_GetFileVersionInfoExW)GetProcAddress(hOriginalDll, "GetFileVersionInfoExW");
  orig_GetFileVersionInfoSizeExA =
      (ORIG_FUNCTION_GetFileVersionInfoSizeExA)GetProcAddress(hOriginalDll, "GetFileVersionInfoSizeExA");
  orig_GetFileVersionInfoSizeExW =
      (ORIG_FUNCTION_GetFileVersionInfoSizeExW)GetProcAddress(hOriginalDll, "GetFileVersionInfoSizeExW");
  orig_VerLanguageNameA = (ORIG_FUNCTION_VerLanguageNameA)GetProcAddress(hOriginalDll, "VerLanguageNameA");
  orig_VerLanguageNameW = (ORIG_FUNCTION_VerLanguageNameW)GetProcAddress(hOriginalDll, "VerLanguageNameW");
  orig_VerQueryValueA   = (ORIG_FUNCTION_VerQueryValueA)GetProcAddress(hOriginalDll, "VerQueryValueA");
  orig_VerQueryValueW   = (ORIG_FUNCTION_VerQueryValueW)GetProcAddress(hOriginalDll, "VerQueryValueW");
  orig_GetFileVersionInfoByHandle =
      (ORIG_FUNCTION_GetFileVersionInfoByHandle)GetProcAddress(hOriginalDll, "GetFileVersionInfoByHandle");
}
