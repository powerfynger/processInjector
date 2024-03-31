#include "Injector.h"


Injector::Injector(int pid) : _pid(pid), _targetProcess(pid) {}

Injector::Injector(std::string procName) : _targetProcess(procName) { _pid = 123; }

void Injector::injectDll(std::string& dllPath, std::string& fileToHide, std::string& funcToTrack)
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

    LPCWSTR pipeName = L"\\\\.\\pipe\\NothingSpecialHere";
    static DllData data;

    data.fileName = fileToHide;
    data.funcName = funcToTrack;


    if (!_writeToPipe(data, &pipeName))
    {
        CloseHandle(remoteThread);
        return;
    }

    WaitForSingleObject(remoteThread, INFINITE);

    
    VirtualFreeEx(_targetProcess.getHandle(), remoteDllPath, 0, MEM_RELEASE);

    
    CloseHandle(remoteThread);
}


bool Injector::_writeToPipe(DllData& data, LPCWSTR* pipeName)
{
    HANDLE hPipe;
    hPipe = CreateNamedPipe(
        *pipeName,                    // Имя канала
        PIPE_ACCESS_DUPLEX,          // Режим доступа
        PIPE_TYPE_MESSAGE |         // Режим передачи данных сообщениями
        PIPE_READMODE_MESSAGE |     // Режим чтения данных сообщениями
        PIPE_WAIT,                  // Режим ожидания
        PIPE_UNLIMITED_INSTANCES,   // Максимальное количество экземпляров канала
        sizeof(DllData),             // Размер выходного буфера
        sizeof(DllData),             // Размер входного буфера
        0,                          // Время ожидания
        NULL                        // Защита по умолчанию
    );

    if (hPipe == INVALID_HANDLE_VALUE) 
    {
        //std::cerr << "Failed to create named pipe. Error code: " << GetLastError() << std::endl;
        throw std::runtime_error("Failed to create named pipe.");
        return false;
    }

    if (!ConnectNamedPipe(hPipe, NULL)) 
    {
        //std::cerr << "Failed to connect to named pipe. Error code: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        throw std::runtime_error("Failed to connect to named pipe.");
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hPipe, &data, sizeof(DllData), &bytesWritten, NULL)) 
    {
        //std::cerr << "Failed to write to named pipe. Error code: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        throw std::runtime_error("Failed to write to named pipe.");
        return false;
    }

    CloseHandle(hPipe);
    return true;
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

