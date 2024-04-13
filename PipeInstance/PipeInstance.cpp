#include "PipeInstance.h"

PipeInstance::PipeInstance(std::wstring pipeName) : _hPipe(INVALID_HANDLE_VALUE), _pipeName(pipeName) {}

void PipeInstance::openPipeServer()
{
    _hPipe = CreateNamedPipeW(
        _pipeName.c_str(),          // Имя канала
        PIPE_ACCESS_DUPLEX,         // Режим доступа
        PIPE_TYPE_MESSAGE |         // Режим передачи данных сообщениями
        PIPE_READMODE_MESSAGE |     // Режим чтения данных сообщениями
        PIPE_WAIT,                  // Режим ожидания
        PIPE_UNLIMITED_INSTANCES,   // Максимальное количество экземпляров канала
        1024,                       // Размер выходного буфера
        1024,                       // Размер входного буфера
        0,                          // Время ожидания
        NULL                        // Защита по умолчанию
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
        _pipeName.c_str(),           // Имя канала
        PIPE_ACCESS_DUPLEX | PIPE_NOWAIT,                // Режим доступа
        0,                           // Нет разделяемого доступа
        NULL,                        // Защита по умолчанию
        OPEN_EXISTING,               // Открыть существующий канал
        0,                           // Атрибуты по умолчанию
        NULL                         // Нет атрибутов шаблона файла
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
