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
        pCreateFileW = (HANDLE(WINAPI*)(
            LPCWSTR lpFileName,
            DWORD dwDesiredAccess,
            DWORD dwShareMode,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDisposition,
            DWORD dwFlagsAndAttributes,
            HANDLE hTemplateFile))
            DetourFindFunction("kernel32.dll", "CreateFileW");

        pFindFirstFileW = (HANDLE(WINAPI*)(
            LPCWSTR lpFileName,
            LPWIN32_FIND_DATAW lpFindFileData))
            DetourFindFunction("kernel32.dll", "FindFirstFileW");

        pFindNextFileW = (BOOL(WINAPI*)(
            HANDLE  hFindFile,
            LPWIN32_FIND_DATAW lpFindFileData))
            DetourFindFunction("kernel32.dll", "FindNextFileW");
        
        if (conf.funcName == L"CreateFile") DetourAttach(&(PVOID&)pCreateFileW, MyCreateFileW);
        if (conf.funcName == L"FindFirstFile") DetourAttach(&(PVOID&)pFindFirstFileW, MyFindFirstFileW);
        if (conf.funcName == L"FindNextFile") DetourAttach(&(PVOID&)pFindNextFileW, MyFindNextFileW);
        
        
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

        DetourDetach(&(PVOID&)pCreateFileW, MyCreateFileW);
        DetourDetach(&(PVOID&)pFindFirstFileW, MyFindFirstFileW);
        DetourDetach(&(PVOID&)pFindNextFileW, MyFindNextFileW);

        DetourTransactionCommit();

        std::cout << "DLL unloaded." << std::endl;
        break;
    }
    return TRUE;
}

HANDLE WINAPI MyCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) 
{
    std::cout << "[+] Captured CreateFile call\n";
    
    if ((!_wcsicmp(conf.fileName.c_str(), lpFileName)) || (!_wcsicmp(getFileName(conf.fileName).c_str(), lpFileName)))
    {
        std::cout << "[+] CreateFile FILE WAS HIDDEN\n";
        return INVALID_HANDLE_VALUE;
    }
    
    HANDLE hFile = pCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    return hFile;
}

HANDLE WINAPI MyFindFirstFileW(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW lpFindFileData
)
{
    std::cout << "[+] Captured FindFirstfile call\n";
    if ((!_wcsicmp(conf.fileName.c_str(), lpFileName)) || (!_wcsicmp(getFileName(conf.fileName).c_str(), lpFileName)))
    {
        std::cout << "[+] FindFirstfile FILE WAS HIDDEN\n";
        return INVALID_HANDLE_VALUE;
    }

    HANDLE hFile = pFindFirstFileW(lpFileName, lpFindFileData);

    return hFile;

}

BOOL WINAPI MyFindNextFileW(
    HANDLE  hFindFile,
    LPWIN32_FIND_DATAW lpFindFileData
)
{
    std::cout << "[+] Captured FindNextFileW call\n";
    BOOL ret = pFindNextFileW(hFindFile, lpFindFileData);
    //wcscmp()
    if ((!_wcsicmp(conf.fileName.c_str(), lpFindFileData->cFileName)) || (!_wcsicmp(getFileName(conf.fileName).c_str(), lpFindFileData->cFileName)))
    {
        std::cout << "[+] FindNextFile FILE WAS HIDDEN\n";
        BOOL ret = pFindNextFileW(hFindFile, lpFindFileData);
    }
    
    //std::wcout << conf.fileName << "!=" << lpFindFileData->cFileName << std::endl;
    //std::wcout << getFileName(conf.fileName).c_str() << "!=" << lpFindFileData->cFileName << std::endl;


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
    std::vector<wchar_t> fileNameBuffer(fileNameLen + 1); 
    if (!ReadFile(hPipe, fileNameBuffer.data(), fileNameLen * sizeof(wchar_t), &bytesRead, NULL))
    {
        std::cout << "Failed to read from named pipe. Error code: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return false;
    }
    fileNameBuffer[fileNameLen] = '\0'; 
    receivedData.fileName = fileNameBuffer.data();
    std::wcout << "Readed: " << receivedData.fileName << "\n";

    // Чтение имени функции
    std::vector<wchar_t> funcNameBuffer(funcLen + 1); 
    if (!ReadFile(hPipe, funcNameBuffer.data(), funcLen * sizeof(wchar_t), &bytesRead, NULL))
    {
        std::cout << "Failed to read from named pipe. Error code: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return false;
    }
    funcNameBuffer[funcLen] = '\0';
    receivedData.funcName = funcNameBuffer.data();
    std::wcout << "Readed: " << receivedData.funcName << "\n";


    conf.fileName = receivedData.fileName;
    conf.funcName = receivedData.funcName;



    CloseHandle(hPipe);

    return true;
}

std::wstring getFileName(const std::wstring& filePath) {
    size_t pos = filePath.find_last_of(L"/\\");
    if (pos != std::wstring::npos) {
        return filePath.substr(pos + 1);
    }
    return filePath;
}
