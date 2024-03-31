#pragma once
#include "Process.h"
#include <iostream>

struct DllData {
	std::string fileName;
	std::string funcName;
};

class Injector
{
public:
	Injector(int pid);
	Injector(std::string procName);

	void injectDll(std::string& dllPath, std::string& fileToHide, std::string& funcToTrack);
private:
	int _pid;
	Process _targetProcess;
	std::string _fileToHide;

	bool _checkDllExists(std::string& dllPath);
	bool _writeToPipe(DllData& data, LPCWSTR* pipeName);

};

