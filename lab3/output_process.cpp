#include <windows.h>
#include <iostream>
#include <vector>
#define MAX_BUFFER 1024

void connectToOutputPipe(HANDLE& pipeHandle) {
    pipeHandle = CreateFileW(
        L"\\\\.\\pipe\\SortToOutputPipe",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (pipeHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка: невозможно подключиться к выходному каналу." << std::endl;
        exit(1);
    }
}

void DisplayStartMessage() {
    std::cout << "Execution has commenced." << std::endl;
}

void DisplayEndMessage() {
    std::cout << "Execution has completed." << std::endl;
}

void processReceivedData(HANDLE pipeHandle) {
    int buffer[MAX_BUFFER];
    DWORD bytesRead;

    if (!ReadFile(pipeHandle, buffer, sizeof(buffer), &bytesRead, NULL)) {
        std::cerr << "Ошибка при чтении данных из выходного канала." << std::endl;
        return;
    }

    int numResults = bytesRead / sizeof(int);
    std::cout << "Сортированные данные: ";
    for (int i = 0; i < numResults; ++i) {
        std::cout << buffer[i] << " ";
    }
    std::cout << std::endl;
}

int main() {
    DisplayStartMessage();
    HANDLE hPipe;
    connectToOutputPipe(hPipe);
    processReceivedData(hPipe);
    CloseHandle(hPipe);
    DisplayEndMessage()
    return 0;
}
