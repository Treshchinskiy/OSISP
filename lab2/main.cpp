#include <windows.h>
#include <iostream>
#include <algorithm>  
#include <vector>
#include <thread>
#include <chrono>


void SortData(char* data, size_t start, size_t end) {
    std::sort(data + start, data + end);
}

void TraditionalSort(const char* fileName, DWORD fileSize) {
    HANDLE hFile = CreateFileA(
        fileName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    std::vector<char> data(fileSize);
    DWORD bytesRead;
    ReadFile(hFile, data.data(), fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    auto start = std::chrono::high_resolution_clock::now();
    std::sort(data.begin(), data.end());
    auto end = std::chrono::high_resolution_clock::now();

    hFile = CreateFileA(
        fileName,
        GENERIC_WRITE,
        0,
        NULL,
        TRUNCATE_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    DWORD bytesWritten;
    WriteFile(hFile, data.data(), fileSize, &bytesWritten, NULL);
    CloseHandle(hFile);

    std::chrono::duration<double> duration = end - start;
    std::cout << "Время выполнения традиционной сортировки: " << duration.count() << " секунд." << std::endl;
}

void TraditionalSortMultithreaded(const char* fileName, DWORD fileSize, int numThreads) {
    HANDLE hFile = CreateFileA(
        fileName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    std::vector<char> data(fileSize);
    DWORD bytesRead;
    ReadFile(hFile, data.data(), fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    size_t chunkSize = fileSize / numThreads;
    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; ++i) {
        size_t startIdx = i * chunkSize;
        size_t endIdx = (i == numThreads - 1) ? fileSize : (i + 1) * chunkSize;
        threads.emplace_back(SortData, data.data(), startIdx, endIdx);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::sort(data.begin(), data.end()); 

    auto end = std::chrono::high_resolution_clock::now();

    hFile = CreateFileA(
        fileName,
        GENERIC_WRITE,
        0,
        NULL,
        TRUNCATE_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    DWORD bytesWritten;
    WriteFile(hFile, data.data(), fileSize, &bytesWritten, NULL);
    CloseHandle(hFile);

    std::chrono::duration<double> duration = end - start;
    std::cout << "Время выполнения многопоточной традиционной сортировки: " << duration.count() << " секунд." << std::endl;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    const char* fileName = "text.txt";

    HANDLE hFile = CreateFileA(
        fileName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка открытия файла: " << GetLastError() << std::endl;
        return 1;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        std::cerr << "Ошибка: файл пуст или не удалось получить размер файла." << std::endl;
        CloseHandle(hFile);
        return 1;
    }
    HANDLE hMapping = CreateFileMappingA(
        hFile,
        NULL,
        PAGE_READWRITE,
        0,
        0,
        NULL);

    if (hMapping == NULL) {
        std::cerr << "Ошибка создания отображения файла: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return 1;
    }

    LPVOID pFileView = MapViewOfFile(
        hMapping,
        FILE_MAP_READ | FILE_MAP_WRITE,
        0,
        0,
        0);

    if (pFileView == NULL) {
        std::cerr << "Ошибка проецирования файла в память: " << GetLastError() << std::endl;
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return 1;
    }

    char* pData = (char*)pFileView;
    
    int numThreads;
    std::cout << "Введите количество потоков для отображаемого файла: ";
    std::cin >> numThreads;
    
    size_t chunkSize = fileSize / numThreads;
    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; ++i) {
        size_t startIdx = i * chunkSize;
        size_t endIdx = (i == numThreads - 1) ? fileSize : (i + 1) * chunkSize;
        threads.emplace_back(SortData, pData, startIdx, endIdx);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    if (!FlushViewOfFile(pFileView, 0)) {
        std::cerr << "Ошибка обновления данных в файле: " << GetLastError() << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Время выполнения сортировки с использованием отображения в память (" << numThreads << " потоков): " << duration.count() << " секунд." << std::endl;

    UnmapViewOfFile(pFileView);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    TraditionalSort(fileName, fileSize);  
    TraditionalSortMultithreaded(fileName, fileSize, numThreads);  

    return 0;
}
