#include "mainDll.h"



int HookAllFunctions()
{
    HMODULE hModule = GetModuleHandle(NULL);

    // Получаем указатель на начало таблицы импортов
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hModule + pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    // Проходим по таблице импортов и устанавливаем хук для каждой функции
    while (pImportDesc->Name != NULL)
    {
        LPCSTR moduleName = (LPCSTR)((BYTE*)hModule + pImportDesc->Name);
        HMODULE hImportModule = GetModuleHandleA(moduleName);
        if (hImportModule == GetModuleHandleA("kernel32.dll"))
        {
            PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + pImportDesc->FirstThunk);
            while (pThunk->u1.Function != NULL)
            {
                if (!(pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG))
                {
                    // Если имя функции доступно, устанавливаем хук
                    LPCSTR functionName = (LPCSTR)(((PIMAGE_IMPORT_BY_NAME)pThunk->u1.AddressOfData)->Name);
                    DetourAttach(&(PVOID&)pThunk->u1.Function, MyHookFunction);
                }
                pThunk++;
            }
        }
        pImportDesc++;
    }
    return 0;
}

void UnHookAllFuntions()
{
   
}

FARPROC WINAPI MyHookFunction(LPCSTR functionName)
{
    // Печатаем название вызванной функции
    std::cout << "Function called: " << functionName << std::endl;

    // Ищем адрес оригинальной функции
    HMODULE hModule = GetModuleHandleA("kernel32.dll");
    FARPROC originalFunction = GetProcAddress(hModule, functionName);

    // Добавляем адрес оригинальной функции в вектор для последующего вызова
    originalFunctions.push_back((PfnOriginalFunction)originalFunction);

    // Возвращаем адрес оригинальной функции
    return originalFunction;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
   
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        pipeClient.waitForClientConnection();
        conf.fileName = pipeClient.readWstringFromPipe();
        conf.funcName = pipeClient.readWstringFromPipe();
        
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        //HookAllFunctions();

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
    //std::cout << "[+] Captured CreateFile call\n";
    std::wstring tmp(L"[+] Captured CreateFile call");
    
    pipeClient.writeWstringToPipe(tmp);
    if ((!_wcsicmp(conf.fileName.c_str(), lpFileName)) || (!_wcsicmp(getFileName(conf.fileName).c_str(), lpFileName)))
    {
        tmp = L"[+] CreateFile FILE WAS HIDDEN";
        pipeClient.writeWstringToPipe(tmp);
        //std::cout << "[+] CreateFile FILE WAS HIDDEN\n";
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
    std::wstring tmp(L"[+] Captured FindFirstfile call");
    pipeClient.writeWstringToPipe(tmp);
    //std::cout << "[+] Captured FindFirstfile call\n";
    if ((!_wcsicmp(conf.fileName.c_str(), lpFileName)) || (!_wcsicmp(getFileName(conf.fileName).c_str(), lpFileName)))
    {
        //std::cout << "[+] FindFirstfile FILE WAS HIDDEN\n";
        tmp = L"[+] FindFirstfile FILE WAS HIDDEN";
        pipeClient.writeWstringToPipe(tmp);
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
    //std::cout << "[+] Captured FindNextFileW call\n";
    std::wstring tmp(L"[+] Captured FindNextFileW call");
    pipeClient.writeWstringToPipe(tmp);
    BOOL ret = pFindNextFileW(hFindFile, lpFindFileData);
    //wcscmp()
    if ((!_wcsicmp(conf.fileName.c_str(), lpFindFileData->cFileName)) || (!_wcsicmp(getFileName(conf.fileName).c_str(), lpFindFileData->cFileName)))
    {
        //std::cout << "[+] FindNextFile FILE WAS HIDDEN\n";
        tmp = L"[+] FindNextFile FILE WAS HIDDEN";
        //pipeClient.writeWstringToPipe(tmp);
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
            *pipeName,                   // Имя канала
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
