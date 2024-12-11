#include <windows.h>
#include <iostream>
#include <algorithm>
#define PIPE_NAME_GEN_TO_SORT L"\\\\.\\pipe\\GenToSortPipe"
#define PIPE_NAME_SORT_TO_OUTPUT L"\\\\.\\pipe\\SortToOutputPipe"
#define BUFFER_SIZE 1024

int main() {
    HANDLE hPipeRead, hPipeWrite;
    DWORD dwRead, dwWritten;
    int buffer[BUFFER_SIZE];
    int dataSize;

    hPipeRead = CreateFileW( 
        PIPE_NAME_GEN_TO_SORT,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipeRead == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to connect to the data generator pipe." << std::endl;
        return 1;
    }

    std::cout << "Receiving data from the generator..." << std::endl;

    BOOL isSuccess = ReadFile(
        hPipeRead,
        buffer,
        BUFFER_SIZE,
        &dwRead,
        NULL
    );

    if (!isSuccess || dwRead == 0) {
        std::cerr << "Failed to read data from the pipe." << std::endl;
        CloseHandle(hPipeRead);
        return 1;
    }

    dataSize = dwRead / sizeof(int);

    std::cout << "Data received. Sorting..." << std::endl;

    std::sort(buffer, buffer + dataSize);

    CloseHandle(hPipeRead);

    hPipeWrite = CreateNamedPipeW( 
        PIPE_NAME_SORT_TO_OUTPUT,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,
        BUFFER_SIZE,
        BUFFER_SIZE,
        0,
        NULL
    );

    if (hPipeWrite == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create pipe to the output process." << std::endl;
        return 1;
    }

    std::cout << "Waiting for the output process to connect..." << std::endl;

    BOOL isConnected = ConnectNamedPipe(hPipeWrite, NULL);
    if (!isConnected) {
        std::cerr << "Failed to connect to the output process." << std::endl;
        CloseHandle(hPipeWrite);
        return 1;
    }

    std::cout << "Sending sorted data to the output process..." << std::endl;

    isSuccess = WriteFile(
        hPipeWrite,
        buffer,
        dataSize * sizeof(int),
        &dwWritten,
        NULL
    );

    if (!isSuccess) {
        std::cerr << "Failed to write data to the pipe." << std::endl;
    }
    else {
        std::cout << "Sorted data sent successfully." << std::endl;
    }

    CloseHandle(hPipeWrite);
    return 0;
}
