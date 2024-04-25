#pragma once
#include <windows.h>
#include <iostream>
#include "detours.h"
#include <vector>
#include "../Injector/Injector.h"
#include "../PipeInstance/PipeInstance.h"



#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif

typedef FARPROC(WINAPI* PfnOriginalFunction)(LPCSTR);


std::vector<PfnOriginalFunction> originalFunctions;


//static std::string fileNameToHide;
DllData conf;
PipeInstance pipeClient(std::wstring(L"\\\\.\\pipe\\NothingSpecialHere"));


MYDLL_API BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);

//void setFileToHideDll(std::string& file);

HANDLE WINAPI MyCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
);
HANDLE (WINAPI *pCreateFileW)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) = CreateFileW;

HANDLE WINAPI  MyFindFirstFileW(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW lpFindFileData
);


HANDLE (WINAPI  *pFindFirstFileW)(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW  lpFindFileData
) = FindFirstFileW;

BOOL WINAPI MyFindNextFileW(
    HANDLE  hFindFile,
    LPWIN32_FIND_DATAW lpFindFileData
);

BOOL (WINAPI *pFindNextFileW)(
    HANDLE  hFindFile,
    LPWIN32_FIND_DATAW lpFindFileData
) = FindNextFileW;

bool readConfigFromPipe(LPCWSTR* pipeName);
std::wstring getFileName(const std::wstring& filePath);

extern "C" void* target;

extern "C" void MyHookFunction();
extern "C" void MyHookFunctionAsm();