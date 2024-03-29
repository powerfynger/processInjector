#pragma once
#include "Process.h"
#include <iostream>

class Injector
{
public:
	Injector(int pid);

	void injectDll(std::string& dllPath);
private:
	int _pid;
	Process _targetProcess;

	bool _checkDllExists(std::string& dllPath);

};

