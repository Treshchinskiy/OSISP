#include <windows.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <thread>
#include <chrono>
#include <vcruntime.h>

void PerformSort(char* buffer, size_t startIndex, size_t endIndex) {
    std::sort(buffer + startIndex, buffer + endIndex);
}

void DisplayStartMessage() {
    std::cout << "Execution has commenced." << std::endl;
}

void DisplayEndMessage() {
    std::cout << "Execution has completed." << std::endl;
}

void SingleThreadSort(const char* filePath, DWORD fileLength) {
    std::string p = "Запускаем одиночный поток";
    HANDLE inputFile = CreateFileA(
        filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    std::vector<char> fileContents(fileLength);
    DWORD readBytes;
    ReadFile(inputFile, fileContents.data(), fileLength, &readBytes, NULL);
    CloseHandle(inputFile);

    auto startTime = std::chrono::high_resolution_clock::now();
    std::sort(fileContents.begin(), fileContents.end());
    auto endTime = std::chrono::high_resolution_clock::now();

    inputFile = CreateFileA(
        filePath, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD writtenBytes;
    WriteFile(inputFile, fileContents.data(), fileLength, &writtenBytes, NULL);
    CloseHandle(inputFile);

    std::chrono::duration<double> elapsedTime = endTime - startTime;
    std::cout << "Time taken for single-threaded sort: " << elapsedTime.count() << " seconds." << std::endl;
}

void MultiThreadedSort(const char* filePath, DWORD fileLength, int threadCount) {
    std::string p = "запускаем несколько потоков для сортировки";
    HANDLE fileHandle = CreateFileA(
        filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    std::vector<char> fileContents(fileLength);
    DWORD bytesRead;
    ReadFile(fileHandle, fileContents.data(), fileLength, &bytesRead, NULL);
    CloseHandle(fileHandle);

    size_t segmentSize = fileLength / threadCount;
    std::vector<std::thread> workerThreads;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < threadCount; ++i) {
        size_t startIdx = i * segmentSize;
        size_t endIdx = (i == threadCount - 1) ? fileLength : (i + 1) * segmentSize;
        workerThreads.emplace_back(PerformSort, fileContents.data(), startIdx, endIdx);
    }

    for (auto& thread : workerThreads) {
        thread.join();
    }

    std::sort(fileContents.begin(), fileContents.end());

    auto endTime = std::chrono::high_resolution_clock::now();

    fileHandle = CreateFileA(
        filePath, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD bytesWritten;
    WriteFile(fileHandle, fileContents.data(), fileLength, &bytesWritten, NULL);
    CloseHandle(fileHandle);

    std::chrono::duration<double> elapsedTime = endTime - startTime;
    std::cout << "Time taken for multi-threaded sort: " << elapsedTime.count() << " seconds." << std::endl;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    DisplayStartMessage();

    //название файла
    const char* targetFile = "text.txt";

    HANDLE fileHandle = CreateFileA(
        targetFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Error while opening file: " << GetLastError() << std::endl;
        return 1;
    }

    DWORD fileSize = GetFileSize(fileHandle, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        std::cerr << "Error: File is empty or size retrieval failed." << std::endl;
        CloseHandle(fileHandle);
        return 1;
    }

    HANDLE fileMapping = CreateFileMappingA(
        fileHandle, NULL, PAGE_READWRITE, 0, 0, NULL);

    if (fileMapping == NULL) {
        std::cerr << "Error creating file mapping: " << GetLastError() << std::endl;
        CloseHandle(fileHandle);
        return 1;
    }

    LPVOID mappedView = MapViewOfFile(
        fileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

    if (mappedView == NULL) {
        std::cerr << "Error mapping file view: " << GetLastError() << std::endl;
        CloseHandle(fileMapping);
        CloseHandle(fileHandle);
        return 1;
    }

    char* fileData = (char*)mappedView;

    int threadCount;
    std::cout << "Enter thread count for memory-mapped sorting: ";
    std::cin >> threadCount;

    size_t chunkSize = fileSize / threadCount;
    std::vector<std::thread> threadList;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < threadCount; ++i) {
        size_t startIdx = i * chunkSize;
        size_t endIdx = (i == threadCount - 1) ? fileSize : (i + 1) * chunkSize;
        threadList.emplace_back(PerformSort, fileData, startIdx, endIdx);
    }

    for (auto& thread : threadList) {
        thread.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = endTime - startTime;

    std::cout << "Sorting completed using memory mapping with " << threadCount << " threads in " << elapsedTime.count() << " seconds." << std::endl;

    UnmapViewOfFile(mappedView);
    CloseHandle(fileMapping);
    CloseHandle(fileHandle);

    SingleThreadSort(targetFile, fileSize);
    MultiThreadedSort(targetFile, fileSize, threadCount);

    DisplayEndMessage();
    return 0;
}
