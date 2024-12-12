#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>

void connectToGeneratorPipe(HANDLE& pipeHandle) {
    pipeHandle = CreateFileW(
        L"\\\\.\\pipe\\GenToSortPipe",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (pipeHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка: невозможно подключиться к генератору данных." << std::endl;
        exit(1);
    }
}

void sortReceivedData(int* buffer, int numElements) {
    std::sort(buffer, buffer + numElements);
}

HANDLE createOutputPipe() {
    HANDLE hPipeWrite = CreateNamedPipeW(
        L"\\\\.\\pipe\\SortToOutputPipe",
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,
        1024,
        1024,
        0,
        NULL
    );

    if (hPipeWrite == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка при создании канала для отправки данных." << std::endl;
        exit(1);
    }

    if (!ConnectNamedPipe(hPipeWrite, NULL)) {
        std::cerr << "Ошибка при подключении к выходному каналу." << std::endl;
        exit(1);
    }

    return hPipeWrite;
}
void DisplayStartMessage() {
    std::cout << "Execution has commenced." << std::endl;
}

void DisplayEndMessage() {
    std::cout << "Execution has completed." << std::endl;
}

int main() {
    DisplayStartMessage();
    HANDLE hPipeRead, hPipeWrite;
    int buffer[1024];
    DWORD dwRead;

    connectToGeneratorPipe(hPipeRead);
    std::cout << "Данные получены, начинаем сортировку..." << std::endl;

    if (!ReadFile(hPipeRead, buffer, sizeof(buffer), &dwRead, NULL)) {
        std::cerr << "Ошибка при чтении данных из канала." << std::endl;
        return 1;
    }

    int numElements = dwRead / sizeof(int);
    sortReceivedData(buffer, numElements);

    hPipeWrite = createOutputPipe();

    WriteFile(hPipeWrite, buffer, numElements * sizeof(int), &dwRead, NULL);

    std::cout << "Сортированные данные отправлены." << std::endl;

    CloseHandle(hPipeRead);
    CloseHandle(hPipeWrite);
    DisplayEndMessage();
    return 0;
}
