#pragma once
#include "Process.h"
#include <iostream>

struct DllData {
	std::wstring fileName;
	std::wstring funcName;
};

class Injector
{
public:
	Injector(int pid);
	Injector(std::wstring procName);

	void injectDll(std::string& dllPath, std::wstring& fileToHide, std::wstring& funcToTrack);
private:
	int _pid;
	Process _targetProcess;
	std::string _fileToHide;

	bool _checkDllExists(std::string& dllPath);
	bool _writeToPipe(DllData& data, LPCWSTR* pipeName);

};

