#include <Windows.h>

typedef DWORD(WINAPI *ORIG_FUNCTION_VerFindFileA)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT, LPSTR, PUINT);
ORIG_FUNCTION_VerFindFileA orig_VerFindFileA;
DWORD WINAPI VerFindFileA(DWORD uFlags, LPCSTR szFileName, LPCSTR szWinDir, LPCSTR szAppDir, LPSTR szCurDir, PUINT puCurDirLen, LPSTR szDestDir, PUINT puDestDirLen)
{
    return (orig_VerFindFileA)(uFlags, szFileName, szWinDir, szAppDir, szCurDir, puCurDirLen, szDestDir, puDestDirLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerFindFileW)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT, LPWSTR, PUINT);
ORIG_FUNCTION_VerFindFileW orig_VerFindFileW;
DWORD WINAPI VerFindFileW(DWORD uFlags, LPCWSTR szFileName, LPCWSTR szWinDir, LPCWSTR szAppDir, LPWSTR szCurDir, PUINT puCurDirLen, LPWSTR szDestDir, PUINT puDestDirLen)
{
    return (orig_VerFindFileW)(uFlags, szFileName, szWinDir, szAppDir, szCurDir, puCurDirLen, szDestDir, puDestDirLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerInstallFileA)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT);
ORIG_FUNCTION_VerInstallFileA orig_VerInstallFileA;
DWORD WINAPI VerInstallFileA(DWORD uFlags, LPCSTR szSrcFileName, LPCSTR szDestFileName, LPCSTR szSrcDir, LPCSTR szDestDir, LPCSTR szCurDir, LPSTR szTmpFile, PUINT puTmpFileLen)
{
    return (orig_VerInstallFileA)(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile, puTmpFileLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerInstallFileW)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT);
ORIG_FUNCTION_VerInstallFileW orig_VerInstallFileW;
DWORD WINAPI VerInstallFileW(DWORD uFlags, LPCWSTR szSrcFileName, LPCWSTR szDestFileName, LPCWSTR szSrcDir, LPCWSTR szDestDir, LPCWSTR szCurDir, LPWSTR szTmpFile, PUINT puTmpFileLen)
{
    return (orig_VerInstallFileW)(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile, puTmpFileLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoSizeA)(LPCSTR, LPDWORD);
ORIG_FUNCTION_GetFileVersionInfoSizeA orig_GetFileVersionInfoSizeA;
DWORD WINAPI GetFileVersionInfoSizeA(LPCSTR lptstrFilename, LPDWORD lpdwHandle)
{
    return (orig_GetFileVersionInfoSizeA)(lptstrFilename, lpdwHandle);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoSizeW)(LPCWSTR, LPDWORD);
ORIG_FUNCTION_GetFileVersionInfoSizeW orig_GetFileVersionInfoSizeW;
DWORD WINAPI GetFileVersionInfoSizeW(LPCWSTR lptstrFilename, LPDWORD lpdwHandle)
{
    return (orig_GetFileVersionInfoSizeW)(lptstrFilename, lpdwHandle);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoA)(LPCSTR, DWORD, DWORD, LPVOID);
ORIG_FUNCTION_GetFileVersionInfoA orig_GetFileVersionInfoA;
BOOL WINAPI GetFileVersionInfoA(LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
{
    return (orig_GetFileVersionInfoA)(lptstrFilename, dwHandle, dwLen, lpData);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoW)(LPCWSTR, DWORD, DWORD, LPVOID);
ORIG_FUNCTION_GetFileVersionInfoW orig_GetFileVersionInfoW;
BOOL WINAPI GetFileVersionInfoW(LPCWSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
{
    return (orig_GetFileVersionInfoW)(lptstrFilename, dwHandle, dwLen, lpData);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoSizeExA)(DWORD, LPCSTR, LPDWORD);
ORIG_FUNCTION_GetFileVersionInfoSizeExA orig_GetFileVersionInfoSizeExA;
DWORD WINAPI GetFileVersionInfoSizeExA(DWORD dwFlags, LPCSTR lpwstrFilename, LPDWORD lpdwHandle)
{
    return (orig_GetFileVersionInfoSizeExA)(dwFlags, lpwstrFilename, lpdwHandle);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoSizeExW)(DWORD, LPCWSTR, LPDWORD);
ORIG_FUNCTION_GetFileVersionInfoSizeExW orig_GetFileVersionInfoSizeExW;
DWORD WINAPI GetFileVersionInfoSizeExW(DWORD dwFlags, LPCWSTR lpwstrFilename, LPDWORD lpdwHandle)
{
    return (orig_GetFileVersionInfoSizeExW)(dwFlags, lpwstrFilename, lpdwHandle);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoExA)(DWORD, LPCSTR, DWORD, DWORD, LPVOID);
ORIG_FUNCTION_GetFileVersionInfoExA orig_GetFileVersionInfoExA;
BOOL WINAPI GetFileVersionInfoExA(DWORD dwFlags, LPCSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
{
    return (orig_GetFileVersionInfoExA)(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_GetFileVersionInfoExW)(DWORD, LPCWSTR, DWORD, DWORD, LPVOID);
ORIG_FUNCTION_GetFileVersionInfoExW orig_GetFileVersionInfoExW;
BOOL WINAPI GetFileVersionInfoExW(DWORD dwFlags, LPCWSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
{
    return (orig_GetFileVersionInfoExW)(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerLanguageNameA)(DWORD, LPSTR, DWORD);
ORIG_FUNCTION_VerLanguageNameA orig_VerLanguageNameA;
DWORD WINAPI VerLanguageNameA(DWORD wLang, LPSTR szLang, DWORD cchLang)
{
    return (orig_VerLanguageNameA)(wLang, szLang, cchLang);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerLanguageNameW)(DWORD, LPWSTR, DWORD);
ORIG_FUNCTION_VerLanguageNameW orig_VerLanguageNameW;
DWORD WINAPI VerLanguageNameW(DWORD wLang, LPWSTR szLang, DWORD cchLang)
{
    return (orig_VerLanguageNameW)(wLang, szLang, cchLang);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerQueryValueA)(LPCVOID, LPCSTR, LPVOID *, PUINT);
ORIG_FUNCTION_VerQueryValueA orig_VerQueryValueA;
BOOL WINAPI VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen)
{
    return (orig_VerQueryValueA)(pBlock, lpSubBlock, lplpBuffer, puLen);
}

typedef DWORD(WINAPI *ORIG_FUNCTION_VerQueryValueW)(LPCVOID, LPCWSTR, LPVOID *, PUINT);
ORIG_FUNCTION_VerQueryValueW orig_VerQueryValueW;
BOOL WINAPI VerQueryValueW(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen)
{
    return (orig_VerQueryValueW)(pBlock, lpSubBlock, lplpBuffer, puLen);
}

typedef void(WINAPI *ORIG_FUNCTION_GetFileVersionInfoByHandle)();
ORIG_FUNCTION_GetFileVersionInfoByHandle orig_GetFileVersionInfoByHandle;
void WINAPI GetFileVersionInfoByHandle()
{
    (orig_GetFileVersionInfoByHandle)();
}

void VersionDllInit()
{
    HMODULE hOriginalDll = LoadLibraryW(L"C:\\Windows\\system32\\version.dll");
    orig_VerFindFileA = (ORIG_FUNCTION_VerFindFileA)GetProcAddress(hOriginalDll, "VerFindFileA");
    orig_VerFindFileW = (ORIG_FUNCTION_VerFindFileW)GetProcAddress(hOriginalDll, "VerFindFileW");
    orig_VerInstallFileA = (ORIG_FUNCTION_VerInstallFileA)GetProcAddress(hOriginalDll, "VerInstallFileA");
    orig_VerInstallFileW = (ORIG_FUNCTION_VerInstallFileW)GetProcAddress(hOriginalDll, "VerInstallFileW");
    orig_GetFileVersionInfoA = (ORIG_FUNCTION_GetFileVersionInfoA)GetProcAddress(hOriginalDll, "GetFileVersionInfoA");
    orig_GetFileVersionInfoW = (ORIG_FUNCTION_GetFileVersionInfoW)GetProcAddress(hOriginalDll, "GetFileVersionInfoW");
    orig_GetFileVersionInfoSizeA = (ORIG_FUNCTION_GetFileVersionInfoSizeA)GetProcAddress(hOriginalDll, "GetFileVersionInfoSizeA");
    orig_GetFileVersionInfoSizeW = (ORIG_FUNCTION_GetFileVersionInfoSizeW)GetProcAddress(hOriginalDll, "GetFileVersionInfoSizeW");
    orig_GetFileVersionInfoExA = (ORIG_FUNCTION_GetFileVersionInfoExA)GetProcAddress(hOriginalDll, "GetFileVersionInfoExA");
    orig_GetFileVersionInfoExW = (ORIG_FUNCTION_GetFileVersionInfoExW)GetProcAddress(hOriginalDll, "GetFileVersionInfoExW");
    orig_GetFileVersionInfoSizeExA = (ORIG_FUNCTION_GetFileVersionInfoSizeExA)GetProcAddress(hOriginalDll, "GetFileVersionInfoSizeExA");
    orig_GetFileVersionInfoSizeExW = (ORIG_FUNCTION_GetFileVersionInfoSizeExW)GetProcAddress(hOriginalDll, "GetFileVersionInfoSizeExW");
    orig_VerLanguageNameA = (ORIG_FUNCTION_VerLanguageNameA)GetProcAddress(hOriginalDll, "VerLanguageNameA");
    orig_VerLanguageNameW = (ORIG_FUNCTION_VerLanguageNameW)GetProcAddress(hOriginalDll, "VerLanguageNameW");
    orig_VerQueryValueA = (ORIG_FUNCTION_VerQueryValueA)GetProcAddress(hOriginalDll, "VerQueryValueA");
    orig_VerQueryValueW = (ORIG_FUNCTION_VerQueryValueW)GetProcAddress(hOriginalDll, "VerQueryValueW");
    orig_GetFileVersionInfoByHandle = (ORIG_FUNCTION_GetFileVersionInfoByHandle)GetProcAddress(hOriginalDll, "GetFileVersionInfoByHandle");
}
