#include "PipeInstance.h"

PipeInstance::PipeInstance(std::wstring pipeName) : _hPipe(INVALID_HANDLE_VALUE), _pipeName(pipeName) {}

void PipeInstance::openPipeServer()
{
    _hPipe = CreateNamedPipeW(
        _pipeName.c_str(),          // ��� ������
        PIPE_ACCESS_DUPLEX,         // ����� �������
        PIPE_TYPE_MESSAGE |         // ����� �������� ������ �����������
        PIPE_READMODE_MESSAGE |     // ����� ������ ������ �����������
        PIPE_WAIT,                  // ����� ��������
        PIPE_UNLIMITED_INSTANCES,   // ������������ ���������� ����������� ������
        1024,                       // ������ ��������� ������
        1024,                       // ������ �������� ������
        0,                          // ����� ��������
        NULL                        // ������ �� ���������
    );

    if (_hPipe == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Failed to create named pipe.");
    }

    if (!(ConnectNamedPipe(_hPipe, NULL) ?
        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)))
    {
        CloseHandle(_hPipe);
        throw std::runtime_error("Failed to connect to named pipe.");
    }
}

void PipeInstance::connectPipeClient()
{
    _hPipe = CreateFile(
        _pipeName.c_str(),           // ��� ������
        PIPE_ACCESS_DUPLEX | PIPE_NOWAIT,                // ����� �������
        0,                           // ��� ������������ �������
        NULL,                        // ������ �� ���������
        OPEN_EXISTING,               // ������� ������������ �����
        0,                           // �������� �� ���������
        NULL                         // ��� ��������� ������� �����
    );
}

void PipeInstance::waitForClientConnection()
{
    while (_hPipe == INVALID_HANDLE_VALUE)
    {
        connectPipeClient();
        std::cout << "Waiting for connection\n";
    }
    DWORD dwMode = PIPE_NOWAIT;
    //SetNamedPipeHandleState(_hPipe, &dwMode, NULL, NULL);
}

void PipeInstance::writeWstringToPipe(std::wstring& data)
{
    DWORD bytesWritten;
    unsigned int dataLen = data.length();

    if (!WriteFile(_hPipe, &dataLen, sizeof(int), &bytesWritten, NULL))
    {
        //CloseHandle(_hPipe);
        //throw std::runtime_error("Failed to write to named pipe.");
    }

    if (!WriteFile(_hPipe, data.c_str(), (dataLen) * sizeof(wchar_t), &bytesWritten, NULL))
    {
        //CloseHandle(_hPipe);
        //throw std::runtime_error("Failed to write to named pipe.");
    }
}

std::wstring PipeInstance::readWstringFromPipe()
{
    unsigned int dataLen;
    DWORD bytesReaded;
    std::wstring res;

    if (!ReadFile(_hPipe, &dataLen, sizeof(int), &bytesReaded, NULL))
    {
        CloseHandle(_hPipe);
        throw std::runtime_error("Failed to read from named pipe.");
    }

    std::vector<wchar_t> dataBuffer(dataLen + 1);

    if (!ReadFile(_hPipe, dataBuffer.data(), dataLen * sizeof(wchar_t), &bytesReaded, NULL))
    {
        throw std::runtime_error("Failed to read from named pipe.");
        CloseHandle(_hPipe);
    }
    dataBuffer[dataLen] = '\0';

    res = dataBuffer.data();
    return res;
}

void PipeInstance::closePipe()
{
    CloseHandle(_hPipe);
}
