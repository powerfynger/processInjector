#include <iostream>
#include <string>
#include "Process.h"
#include "Injector.h"
#include "..\FakeDll\mainDll.h"


int main(int argc, char* argv[]) 
{
    std::string dllName("FakeDll.dll");

    int pid = 0;
    std::string processName;
    std::string functionName;
    std::string hideFileName;

    for (int i = 1; i < argc; ++i) 
    {
        std::string arg = argv[i];
        if (arg == "-pid" && i + 1 < argc) 
        {
            pid = std::stoi(argv[++i]);        
        }
        else if (arg == "-name" && i + 1 < argc) 
        {
            processName = argv[++i];
        }
        else if (arg == "-func" && i + 1 < argc) 
        {
            functionName = argv[++i];
        }
        else if (arg == "-hide" && i + 1 < argc) 
        {
            hideFileName = argv[++i];
        }
    }


    WIN32_FIND_DATAA fd = { 0 };
    FILE_NAME_INFO inf = { 0 };

    HANDLE hFind = FindFirstFileA("C:\\Users\\puuni\\aaa.pdf", &fd);
    std::cout << fd.cFileName << std::endl;

    if (FindNextFileA(hFind, &fd))
    {
        std::cout << fd.cFileName << std::endl;
    }

    //HINSTANCE dynamicLib = LoadLibraryA("FakeDll.dll");
    Injector a(GetCurrentProcessId());
    a.injectDll(dllName, hideFileName, functionName);
    
    fd = { 0 };
    hFind = FindFirstFileA("C:\\Users\\puuni\\aaa.pdf", &fd);
    
    std::cout << fd.cFileName << std::endl;

    if (FindNextFileA(hFind, &fd))
    {
        std::cout << fd.cFileName << std::endl;
    }

    return 0;
}