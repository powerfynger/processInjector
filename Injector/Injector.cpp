#include "Injector.h"


Injector::Injector(int pid) : _pid(pid), _targetProcess(pid) {}

void Injector::injectDll(std::string& dllPath)
{
    if (!_checkDllExists(dllPath)) 
    {
        throw std::runtime_error("DLL not found: " + dllPath);
    }

    
    void* remoteDllPath = _targetProcess.writeStringToProcess(dllPath);
    if (remoteDllPath == nullptr) 
    {
        throw std::runtime_error("Failed to allocate memory in target process");
    }

    
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (kernel32 == nullptr) 
    {
        throw std::runtime_error("Failed to get kernel32.dll handle");
    }

    LPTHREAD_START_ROUTINE loadLibraryAddr = (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryA");
    if (loadLibraryAddr == nullptr) 
    {
        throw std::runtime_error("Failed to get LoadLibraryA address");
    }

    
    HANDLE remoteThread = _targetProcess.createThreadInProcess(loadLibraryAddr, remoteDllPath);
    if (remoteThread == nullptr) 
    {
        throw std::runtime_error("Failed to create remote thread");
    }

    
    WaitForSingleObject(remoteThread, INFINITE);

    
    VirtualFreeEx(_targetProcess.getHandle(), remoteDllPath, 0, MEM_RELEASE);

    
    CloseHandle(remoteThread);
}

bool Injector::_checkDllExists(std::string& dllPath)
{
	HINSTANCE dynamicLib = LoadLibraryA(dllPath.c_str());

	if (dynamicLib)
	{
		std::cout << "Exists!" << std::endl;
		FreeLibrary(dynamicLib);
		return true;
	}
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        //return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);
	return false;
}