#include "Injector.h"


Injector::Injector(int pid) : _pid(pid), _targetProcess(pid) {}

void Injector::injectDll(std::string& dllPath)
{
	_checkDllExists(dllPath);
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
	return false;
}