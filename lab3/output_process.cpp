#include <windows.h>
#include <iostream>
#define PIPE_NAME_GEN_TO_SORT L"\\\\.\\pipe\\GenToSortPipe"
#define PIPE_NAME_SORT_TO_OUTPUT L"\\\\.\\pipe\\SortToOutputPipe"
#define BUFFER_SIZE 1024

int main() {
    HANDLE hPipe;
    DWORD dwRead;
    int buffer[BUFFER_SIZE];

    hPipe = CreateFileW( 
        PIPE_NAME_SORT_TO_OUTPUT,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to connect to the sorter pipe." << std::endl;
        return 1;
    }

    std::cout << "Receiving sorted data..." << std::endl;

    BOOL isSuccess = ReadFile(
        hPipe,
        buffer,
        BUFFER_SIZE,
        &dwRead,
        NULL
    );

    if (!isSuccess || dwRead == 0) {
        std::cerr << "Failed to read data from the pipe." << std::endl;
        CloseHandle(hPipe);
        return 1;
    }

    int dataSize = dwRead / sizeof(int);

    std::cout << "Sorted data received. Outputting..." << std::endl;

    for (int i = 0; i < dataSize; ++i) {
        std::cout << buffer[i] << " ";
    }
    std::cout << std::endl;

    CloseHandle(hPipe);
    return 0;
}
