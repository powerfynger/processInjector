#include "mainDll.h"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
   
    LPCWSTR pipeName = L"\\\\.\\pipe\\NothingSpecialHere";
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        if (!readConfigFromPipe(&pipeName)) break;
         

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
        
        if (conf.funcName == "CreateFile") DetourAttach(&(PVOID&)pCreateFileA, MyCreateFileA);
        if (conf.funcName == "FindFirstFile") DetourAttach(&(PVOID&)pFindFirstFileA, MyFindFirstFileA);
        if (conf.funcName == "FindNextFile") DetourAttach(&(PVOID&)pFindNextFileA, MyFindNextFileA);
        
        
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
    
    if (std::equal(conf.fileName.begin(), conf.fileName.end(), lpFileName))
    {
        std::cout << "[+] CreateFile FILE WAS HIDDEN\n";
        return INVALID_HANDLE_VALUE;
    }
    
    HANDLE hFile = pCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    return hFile;
}

HANDLE WINAPI MyFindFirstFileA(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData
)
{
    std::cout << "[+] Captured FindFirstfile call\n";
    if (std::equal(conf.fileName.begin(), conf.fileName.end(), lpFileName))
    {
        std::cout << "[+] FindFirstfile FILE WAS HIDDEN\n";
        return INVALID_HANDLE_VALUE;
    }
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
    if (std::equal(conf.fileName.begin(), conf.fileName.end(), lpFindFileData->cFileName))
    {
        std::cout << "[+] FindNextFile FILE WAS HIDDEN\n";
        return false;
    }

    return ret;
}

bool readConfigFromPipe(LPCWSTR* pipeName)
{
    DllData receivedData;
    DWORD bytesRead = 0;
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    while (hPipe == INVALID_HANDLE_VALUE)
    {
        hPipe = CreateFile(
            *pipeName,                     // Имя канала
            GENERIC_READ,                // Режим доступа
            0,                           // Нет разделяемого доступа
            NULL,                        // Защита по умолчанию
            OPEN_EXISTING,               // Открыть существующий канал
            0,                           // Атрибуты по умолчанию
            NULL                         // Нет атрибутов шаблона файла
        );
        Sleep(1000);
        std::cout << "Waiting for connection\n";
    }
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        std::cout << "Failed to open named pipe. Error code: " << GetLastError() << std::endl;
        return false;
    }

    // Считываем данные из канала
    std::cout << "Pipe connected\n";

    int fileNameLen = 0, funcLen = 0;

    if (!ReadFile(hPipe, &fileNameLen, sizeof(int), &bytesRead, NULL))
    {
        std::cout << "Failed to read from named pipe. Error code: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return false;
    }
    std::cout << "Readed: " << fileNameLen << " bytes\n";

    if (!ReadFile(hPipe, &funcLen, sizeof(int), &bytesRead, NULL))
    {
        std::cout << "Failed to read from named pipe. Error code: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return false;
    }

    std::cout << "Readed: " << funcLen << " bytes\n";

    // Чтение имени файла
    std::vector<char> fileNameBuffer(fileNameLen + 1); 
    if (!ReadFile(hPipe, fileNameBuffer.data(), fileNameLen * sizeof(char), &bytesRead, NULL))
    {
        std::cout << "Failed to read from named pipe. Error code: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return false;
    }
    fileNameBuffer[fileNameLen] = '\0'; 
    receivedData.fileName = fileNameBuffer.data();
    std::cout << "Readed: " << receivedData.fileName << "\n";

    // Чтение имени функции
    std::vector<char> funcNameBuffer(funcLen + 1); 
    if (!ReadFile(hPipe, funcNameBuffer.data(), funcLen * sizeof(char), &bytesRead, NULL))
    {
        std::cout << "Failed to read from named pipe. Error code: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return false;
    }
    funcNameBuffer[funcLen] = '\0';
    receivedData.funcName = funcNameBuffer.data();
    std::cout << "Readed: " << receivedData.funcName << "\n";


    conf.fileName = receivedData.fileName;
    conf.funcName = receivedData.funcName;



    CloseHandle(hPipe);

    return true;
}