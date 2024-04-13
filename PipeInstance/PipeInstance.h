#pragma once
#include <string>
#include <Windows.h>
#include <stdexcept>
#include <vector>
#include <iostream>



class PipeInstance
{
public:
	PipeInstance(std::wstring pipeName);

	void writeWstringToPipe(std::wstring& data);
	std::wstring readWstringFromPipe();
	void openPipeServer();
	void connectPipeClient();
	void waitForClientConnection();
	void closePipe();

private:
	std::wstring _pipeName;
	HANDLE _hPipe;
};

