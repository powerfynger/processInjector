#include "mainDll.h"

//extern "C" void* target;

void* target;
// wide char to multi byte:
std::string ws2s(const std::wstring& wstr)
{
    int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), 0, 0, 0, 0);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), &strTo[0], size_needed, 0, 0);
    return strTo;
}

/*
int HookAllFunctions()
{

    LPVOID imageBase = GetModuleHandleA(NULL);
    // Получаем указатель на начало таблицы импортов
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)imageBase;
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)imageBase + pDosHeader->e_lfanew);

    
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = NULL;
    IMAGE_DATA_DIRECTORY importsDirectory = pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)imageBase + importsDirectory.VirtualAddress);

    // Проходим по таблице импортов и устанавливаем хук для каждой функции
    while (pImportDesc->Name != NULL)
    {
        LPCSTR moduleName = (LPCSTR)pImportDesc->Name + (DWORD_PTR)imageBase;
        HMODULE hImportModule = GetModuleHandleA(moduleName);
        if (hImportModule == GetModuleHandleA("kernel32.dll"))
        {
            PIMAGE_THUNK_DATA originalFirstThunk = NULL, firstThunk = NULL;
            
            originalFirstThunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)imageBase + pImportDesc->OriginalFirstThunk);
            firstThunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)imageBase + pImportDesc->FirstThunk);

            //PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + pImportDesc->FirstThunk);
            while (originalFirstThunk->u1.AddressOfData != NULL)
            {
                PIMAGE_IMPORT_BY_NAME functionName = (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)imageBase + originalFirstThunk->u1.AddressOfData);
                std::string tmp(functionName->Name);
                if (tmp != "Sleep")
                {
                    ++originalFirstThunk;
                    ++firstThunk;
                    continue;
                }
                std::wstring tmpW(tmp.begin(), tmp.end());

                //-----------------------
                int N = 0, M = 0;

                //-----------------------
                const void* sourceAddress = reinterpret_cast<const void*>(firstThunk->u1.Function);
                void* functionAddress = reinterpret_cast<void*>(&MyHookFunction);
                const char origBufferSize = 64;
                const char bufferSize = 64;
                // Инструкция перехода на нужную функцию в модуле
                unsigned char origBuffer[origBufferSize] = { 0 };
                unsigned char buffer[bufferSize] = { 0 };
                //std::memcpy(origJmpBuffer, sourceAddress, jmpSize);


                unsigned char *pSourceAddress = (unsigned char*)sourceAddress;
                int i = 0;
                while (*pSourceAddress != 0xCC)
                {
                    origBuffer[i] = *pSourceAddress;
                    N++; i++; pSourceAddress++;
                }
                //origBuffer[i-1] += 0x0E;

                while (*pSourceAddress == 0xCC)
                {
                    M++; pSourceAddress++;
                }

                intptr_t funcAddrInt = reinterpret_cast<intptr_t>(functionAddress);
                buffer[0] = 0x50;
                
                buffer[1] = 0x48;
                buffer[2] = 0xb8;
                std::memcpy(buffer + 3, &funcAddrInt, sizeof(funcAddrInt));
                buffer[11] = 0xff;
                buffer[12] = 0xd0;
                
                buffer[13] = 0x58;
                

                //intptr_t relativeOffset = (reinterpret_cast<intptr_t>(sourceAddress)) - reinterpret_cast<intptr_t>(functionAddress);
                

                //std::memcpy(buffer + 1, &relativeOffset, sizeof(relativeOffset));

                // Переходим в режим записи памяти
                DWORD oldProtect;
                VirtualProtect((LPVOID)sourceAddress, N+M, PAGE_EXECUTE_READWRITE, &oldProtect);

                // Записываем новые байты перехода в память
                std::memcpy((LPVOID)sourceAddress, buffer, 15);

                // Также записываем оригинальный джамп
                std::memcpy((LPVOID)((char*)sourceAddress + 14), origBuffer, origBufferSize);


                // Восстанавливаем защиту памяти
                VirtualProtect((LPVOID)sourceAddress, N+M, oldProtect, &oldProtect);


                // Адрес перехода на нужную функцию в таблице импортов
                FARPROC aa = GetProcAddress(hImportModule, tmp.c_str());
                printf("%p\n", firstThunk->u1.Function);
                
                //DetourAttach(&(PVOID&)aa, MyHookFunction);              

                pipeClient.writeWstringToPipe(tmpW);
                break;
                //++originalFirstThunk;
                //++firstThunk;

            }
        }
        pImportDesc++;
    }
    return 0;
}
*/


int HookAllFunctions()
{
    if (conf.funcName == L"") return 1;
    target = DetourFindFunction("kernel32.dll", ws2s(conf.funcName).c_str());
    if (target == INVALID_HANDLE_VALUE) return -1;
    DetourAttach(&(PVOID&)target, MyHookFunctionAsm);
    return 0;
}


void UnHookAllFuntions()
{
   
}

extern "C" void MyHookFunction()
{
    target;
    // Печатаем название вызванной функции
    std::cout << "[+] Function called: " << ws2s(conf.funcName)  << std::endl;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
   
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //conf.funcName = L"Sleep";
        pipeClient.waitForClientConnection();
        conf.fileName = pipeClient.readWstringFromPipe();
        conf.funcName = pipeClient.readWstringFromPipe();
        
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        HookAllFunctions();

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


        
        DetourAttach(&(PVOID&)pCreateFileW, MyCreateFileW);
        DetourAttach(&(PVOID&)pFindFirstFileW, MyFindFirstFileW);
        DetourAttach(&(PVOID&)pFindNextFileW, MyFindNextFileW);
        
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
    //std::wstring tmp(L"[+] Captured CreateFile call");
    
    //pipeClient.writeWstringToPipe(tmp);
    if ((!_wcsicmp(conf.fileName.c_str(), lpFileName)) || (!_wcsicmp(getFileName(conf.fileName).c_str(), lpFileName)))
    {
        std::wstring tmp = L"[+] CreateFile FILE WAS HIDDEN";
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
    //std::wstring tmp(L"[+] Captured FindFirstfile call");
    //pipeClient.writeWstringToPipe(tmp);
    //std::cout << "[+] Captured FindFirstfile call\n";
    if ((!_wcsicmp(conf.fileName.c_str(), lpFileName)) || (!_wcsicmp(getFileName(conf.fileName).c_str(), lpFileName)))
    {
        //std::cout << "[+] FindFirstfile FILE WAS HIDDEN\n";
        std::wstring tmp = L"[+] FindFirstfile FILE WAS HIDDEN";
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
    //std::wstring tmp(L"[+] Captured FindNextFileW call");
    //pipeClient.writeWstringToPipe(tmp);
    BOOL ret = pFindNextFileW(hFindFile, lpFindFileData);
    //wcscmp()
    if ((!_wcsicmp(conf.fileName.c_str(), lpFindFileData->cFileName)) || (!_wcsicmp(getFileName(conf.fileName).c_str(), lpFindFileData->cFileName)))
    {
        //std::cout << "[+] FindNextFile FILE WAS HIDDEN\n";
        std::wstring tmp = L"[+] FindNextFile FILE WAS HIDDEN";
        pipeClient.writeWstringToPipe(tmp);
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
