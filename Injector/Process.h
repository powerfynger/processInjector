#pragma once

#include <Windows.h>
#include <string>
#include <stdexcept>
#include <psapi.h>



class Process
{
public:
	Process(int pid);
	Process(std::string procName);
	void openHandleByPid();
	int getPid();
	void setPid(int pid);

	void* writeStringToProcess(std::string& data);
	HANDLE getHandle();
	HANDLE createThreadInProcess(LPTHREAD_START_ROUTINE routineStartAddr, void* params);
private:
	int _pid = 0;
	HANDLE _pHandle = nullptr;
};

int GetProcessIdByName(std::string desiredProcName);