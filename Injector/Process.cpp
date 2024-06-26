#include "Process.h"


Process::Process(int pid) : _pid(pid)
{
	_pHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION
		| PROCESS_VM_WRITE | PROCESS_VM_READ, false, _pid);
	if (_pHandle == nullptr)
	{
		auto error = GetLastError();
		std::string msg = "Failed to open process. Error: ";
		msg += error;
		throw std::invalid_argument(msg);
	}
}

Process::Process(std::wstring procName)
{
	std::string charProcName(procName.begin(), procName.end());
	int tmpPid = GetProcessIdByName(charProcName);
	Process tmp(tmpPid);
	_pid = tmpPid;
	_pHandle = tmp.getHandle();
}

void Process::openHandleByPid()
{
	Process tmp(_pid);
	_pHandle = tmp.getHandle();
}


void Process::setPid(int pid)
{

}
int Process::getPid()
{
	return _pid;
}

HANDLE Process::getHandle()
{
	return _pHandle;
}

HANDLE Process::createThreadInProcess(LPTHREAD_START_ROUTINE routineStartAddr, void* params)
{
	HANDLE tmpHandle = CreateRemoteThread(_pHandle, 0, 0, routineStartAddr, params, NULL, nullptr);
	if (tmpHandle == nullptr)
	{
		throw std::runtime_error("Failed to create new thread");
	}
	return tmpHandle;
}

void* Process::writeStringToProcess(std::string& data)
{
	void* newPage = VirtualAllocEx(_pHandle, nullptr, data.size() + 1, MEM_COMMIT, PAGE_READWRITE);
	if (newPage == nullptr) return newPage;
	SIZE_T bytesWritten = 0;
	WriteProcessMemory(_pHandle, newPage, data.c_str(), data.size() + 1, &bytesWritten);
	if (bytesWritten - 1 != data.size())
	{
		throw std::runtime_error("Failed to write data");
		return nullptr;
	}
	return newPage;
}

int GetProcessIdByName(std::string desiredProcName)
{
	unsigned long bytesNeeded, currentSize = 512;
	unsigned long* pProcessIds;

	do 
	{
		currentSize *= 2;
		pProcessIds = new unsigned long[currentSize];

		BOOL success = EnumProcesses(pProcessIds, currentSize, &bytesNeeded);
		if (!success)
		{
			auto error = GetLastError();
			std::string msg = "Failed to get process enumeration. Error: ";
			msg += error;
			throw std::runtime_error(msg);
			delete []pProcessIds;
			return -1;
		}
	} while (bytesNeeded == currentSize);

	for (unsigned int i = 0; i < bytesNeeded / sizeof(unsigned long); i++)
	{
		if (pProcessIds[i] == 0) continue;
		HANDLE pHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION
			| PROCESS_VM_WRITE | PROCESS_VM_READ, false, pProcessIds[i]);
		if (pHandle == nullptr) continue;
		char wProcName[MAX_PATH] = "<>";
		if (GetProcessImageFileNameA(pHandle, wProcName, MAX_PATH) > 0)
		{
			std::string processPath = wProcName;
			size_t found = processPath.find_last_of("\\");
			std::string processNameOnly = processPath.substr(found + 1);
			if (processNameOnly == desiredProcName) 
			{
				CloseHandle(pHandle);
				return pProcessIds[i];
			}
		}	
	}
	return -1;
}