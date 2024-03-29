#pragma once
#include <windows.h>
#include <iostream>
#include "detours.h"

#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif

MYDLL_API BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);

HANDLE WINAPI MyCreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
);
HANDLE (WINAPI *pCreateFileA)(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) = CreateFileA;

HANDLE WINAPI  MyFindFirstFileA(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData
);


HANDLE (WINAPI  *pFindFirstFileA)(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData
) = FindFirstFileA;

BOOL WINAPI MyFindNextFileA(
    HANDLE  hFindFile,
    LPWIN32_FIND_DATAA lpFindFileData
);

BOOL (WINAPI *pFindNextFileA)(
    HANDLE  hFindFile,
    LPWIN32_FIND_DATAA lpFindFileData
) = FindNextFileA;

