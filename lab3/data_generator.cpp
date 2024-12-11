#include <windows.h>
#include <iostream>
#define PIPE_NAME_GEN_TO_SORT L"\\\\.\\pipe\\GenToSortPipe"
#define PIPE_NAME_SORT_TO_OUTPUT L"\\\\.\\pipe\\SortToOutputPipe"
#define BUFFER_SIZE 1024

int main() {
    HANDLE hPipe;
    DWORD dwWritten;
    int data[] = { 5, 3, 8, 6, 2, 7, 4, 1 };
    int dataSize = sizeof(data);

    hPipe = CreateNamedPipeW( 
        PIPE_NAME_GEN_TO_SORT,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,
        BUFFER_SIZE,
        BUFFER_SIZE,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create named pipe." << std::endl;
        return 1;
    }

    std::cout << "Waiting for the sorting process to connect..." << std::endl;

    BOOL isConnected = ConnectNamedPipe(hPipe, NULL);
    if (!isConnected) {
        std::cerr << "Failed to connect to the sorting process." << std::endl;
        CloseHandle(hPipe);
        return 1;
    }

    std::cout << "Sending data to the sorting process..." << std::endl;

    BOOL isSuccess = WriteFile(
        hPipe,
        data,
        dataSize,
        &dwWritten,
        NULL
    );

    if (!isSuccess) {
        std::cerr << "Failed to write data to the pipe." << std::endl;
    }
    else {
        std::cout << "Data sent successfully." << std::endl;
    }

    CloseHandle(hPipe);
    return 0;
}
