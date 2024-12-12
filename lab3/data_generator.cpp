#include <windows.h>
#include <iostream>
#include <string>
#define PIPE_GEN_TO_SORT L"\\\\.\\pipe\\GenToSortPipe"
#define PIPE_SORT_TO_OUTPUT L"\\\\.\\pipe\\SortToOutputPipe"
#define MAX_BUFFER_SIZE 1024


void DisplayStartMessage() {
    std::cout << "Execution has commenced." << std::endl;
}

void DisplayEndMessage() {
    std::cout << "Execution has completed." << std::endl;
}

void setupPipe(HANDLE& pipeHandle) {
    pipeHandle = CreateNamedPipeW(
        PIPE_GEN_TO_SORT,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,
        MAX_BUFFER_SIZE,
        MAX_BUFFER_SIZE,
        0,
        NULL
    );

    if (pipeHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка: Невозможно создать канал связи." << std::endl;
        exit(1);
    }
}

void waitForConnection(HANDLE pipeHandle) {
    if (!ConnectNamedPipe(pipeHandle, NULL)) {
        std::cerr << "Ошибка: Подключение невозможно." << std::endl;
        CloseHandle(pipeHandle);
        exit(1);
    }
}

void sendDataToPipe(HANDLE pipeHandle) {
    DWORD bytesWritten;
    int data[] = { 4723, 11444, 98732, 73412, 8887, 91234, 23456, 123456 };
    if (!WriteFile(pipeHandle, data, sizeof(data), &bytesWritten, NULL)) {
        std::cerr << "Ошибка при отправке данных." << std::endl;
    } else {
        std::cout << "Данные успешно отправлены." << std::endl;
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    DisplayStartMessage();
    HANDLE pipeHandle;
    setupPipe(pipeHandle);
    std::cout << "Ожидание подключения..." << std::endl;
    waitForConnection(pipeHandle);

    std::cout << "Отправляем данные для обработки..." << std::endl;
    sendDataToPipe(pipeHandle);

    CloseHandle(pipeHandle);
    DisplayEndMessage();
    return 0;
}
