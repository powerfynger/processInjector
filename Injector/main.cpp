#include <iostream>
#include <string>
#include "Process.h"
#include "Injector.h"
//#include "..\FakeDll\mainDll.h"
// TODO:
// Отдельный класс для сервера/клиента pipe
// Мониторинг вызовов произвольных функций из kernel32

int wmain(int argc, wchar_t* argv[]) 
{
    std::string dllName("C:\\Users\\puuni\\source\\repos\\processInjector\\x64\\Debug\\FakeDll.dll");

    int pid = 0;
    std::wstring processName;
    std::wstring functionName;
    std::wstring hideFileName;

    for (int i = 1; i < argc; ++i) 
    {
        std::wstring arg = argv[i];
        if (arg == L"-pid" && i + 1 < argc) 
        {
            pid = std::stoi(argv[++i]);        
        }
        else if (arg == L"-name" && i + 1 < argc) 
        {
            processName = argv[++i];
        }
        else if (arg == L"-func" && i + 1 < argc) 
        {

            functionName = argv[++i];
        }
        else if (arg == L"-hide" && i + 1 < argc) 
        {
            hideFileName = argv[++i];
        }
    }

    //Injector a(GetCurrentProcessId());
    //a.injectDll(dllName, hideFileName, functionName);
    
    //Sleep(5000);
    //LoadLibraryA(dllName.c_str());
    //GetTickCount();
    //Sleep(5000);
    //return 0;

    if (processName != L"")
    {
        Injector inj(processName);
        inj.injectDll(dllName, hideFileName, functionName);
    }
    else
    {
        Injector inj(pid);
        inj.injectDll(dllName, hideFileName, functionName);
    }
    

    //WIN32_FIND_DATAA fd = { 0 };
    //FILE_NAME_INFO inf = { 0 };

    //HANDLE hFind = FindFirstFileA("C:\\Users\\puuni\\aaa.pdf", &fd);
    //std::cout << fd.cFileName << std::endl;

    //if (FindNextFileA(hFind, &fd))
    //{
        //std::cout << fd.cFileName << std::endl;
    //}

    //HINSTANCE dynamicLib = LoadLibraryA("FakeDll.dll");
/*    Injector a(GetCurrentProcessId());
    a.injectDll(dllName, hideFileName, functionName);
    GetTickCount()*/;
    //fd = { 0 };
    //hFind = FindFirstFileA("C:\\Users\\puuni\\aaa.pdf", &fd);
    
    //std::cout << fd.cFileName << std::endl;

    //if (FindNextFileA(hFind, &fd))
    //{
        //std::cout << fd.cFileName << std::endl;
    //}
    
    return 0;
}