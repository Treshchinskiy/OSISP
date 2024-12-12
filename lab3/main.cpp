#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

void DisplayStartMessage() {
    std::cout << "Execution has commenced." << std::endl;
}

void DisplayEndMessage() {
    std::cout << "Execution has completed." << std::endl;
}

bool initializeProcess(const std::string& executableName) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (!CreateProcessW(
        std::wstring(executableName.begin(), executableName.end()).c_str(), // Путь к исполняемому файлу (в виде wide string)
        NULL, // Аргументы командной строки (NULL, если их нет)
        NULL, // Атрибуты безопасности для процесса (NULL, если не нужны)
        NULL, // Атрибуты безопасности для потоков (NULL, если не нужны)
        FALSE, // Определяет, будет ли процесс наследовать дескрипторы. FALSE означает, что наследование отключено
        0, // Дополнительные флаги создания процесса (0 - нет специальных флагов)
        NULL, // Переменная окружения для нового процесса (NULL означает использование текущего окружения)
        NULL, // Рабочая директория нового процесса (NULL означает использовать текущую директорию)
        &si // Указатель на структуру STARTUPINFOW, содержащую информацию о запуске процесса
    )) {
        std::cerr << "Ошибка: невозможно запустить процесс " << executableName << std::endl;
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

void launchProcesses() {
    //запуск всех exe файлов
    std::vector<std::string> processes = {
        "DataGenerator",
        "Sorter",
        "OutputProcess"
    };

    for (auto& processName : processes) {
        if (!initializeProcess(processName + ".exe")) {
            std::cerr << "Не удалось инициализировать: " << processName << std::endl;
        } else {
            std::cout << "Процесс успешно инициализирован: " << processName << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


int main() {
    SetConsoleOutputCP(CP_UTF8);
    DisplayStartMessage();

    launchProcesses();
    std::cout << "Все процессы инициализированы." << std::endl;
    DisplayEndMessage();

    return 0;
}
