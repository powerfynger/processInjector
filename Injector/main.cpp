#include <iostream>
#include <string>
#include "Process.h"
#include "Injector.h"

int main(int argc, char* argv[]) 
{
    int pid = 0;
    std::string processName = "mspaint.exe";
    std::string functionName;
    std::string hideFileName;

    for (int i = 1; i < argc; ++i) 
    {
        std::string arg = argv[i];
        if (arg == "-pid" && i + 1 < argc) 
        {
            pid = std::stoi(argv[++i]);
            static Process tmp(pid);
            std::cout << "Process PID: " << tmp.getPid();
            Injector a(pid);
            std::string c("123");
            a.injectDll(c);
        }
        else if (arg == "-name" && i + 1 < argc) 
        {
            processName = argv[++i];
            static Process tmp(processName);
            std::cout << "Process PID: " << tmp.getPid();
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

    


    //std::cout << "PID: " << pid << std::endl;
    //std::cout << "Process Name: " << processName << std::endl;
    //std::cout << "Function Name: " << functionName << std::endl;
    //std::cout << "Hide File Name: " << hideFileName << std::endl;





    return 0;
}