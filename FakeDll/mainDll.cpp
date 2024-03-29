#include "mainDll.h"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        //DetourAttach((PVOID*)CreateFileA, MyCreateFile); 
        pCreateFileA = (HANDLE(WINAPI*)(
            LPCSTR lpFileName,
            DWORD dwDesiredAccess,
            DWORD dwShareMode,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDisposition,
            DWORD dwFlagsAndAttributes,
            HANDLE hTemplateFile))
            DetourFindFunction("kernel32.dll", "CreateFileA");

        pFindFirstFileA = (HANDLE(WINAPI*)(
            LPCSTR lpFileName, 
            LPWIN32_FIND_DATAA lpFindFileData))
            DetourFindFunction("kernel32.dll", "FindFirstFileA");

        pFindNextFileA = (BOOL(WINAPI*)(
            HANDLE  hFindFile,
            LPWIN32_FIND_DATAA lpFindFileData))
            DetourFindFunction("kernel32.dll", "FindNextFileA");

        DetourAttach(&(PVOID&)pCreateFileA, MyCreateFileA);
        DetourAttach(&(PVOID&)pFindFirstFileA, MyFindFirstFileA);
        DetourAttach(&(PVOID&)pFindNextFileA, MyFindNextFileA);

        
        DetourTransactionCommit();

        std::cout << "DLL injected successfully!" << std::endl;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourDetach(&(PVOID&)pCreateFileA, MyCreateFileA);
        DetourDetach(&(PVOID&)pFindFirstFileA, MyFindFirstFileA);
        DetourDetach(&(PVOID&)pFindNextFileA, MyFindNextFileA);

        DetourTransactionCommit();

        std::cout << "DLL unloaded." << std::endl;
        break;
    }
    return TRUE;
}

HANDLE WINAPI MyCreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) 
{
    std::cout << "[+] Captured CreateFile call\n";

    HANDLE hFile = pCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    return hFile;
}

HANDLE WINAPI MyFindFirstFileA(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData
)
{
    std::cout << "[+] Captured FindFirstfile call\n";
    HANDLE hFile = pFindFirstFileA(lpFileName, lpFindFileData);

    return hFile;

}

BOOL WINAPI MyFindNextFileA(
    HANDLE  hFindFile,
    LPWIN32_FIND_DATAA lpFindFileData
)
{
    std::cout << "[+] Captured FindNextFileA call\n";
    BOOL ret = pFindNextFileA(hFindFile, lpFindFileData);

    return ret;
}
