#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <string>

struct ThreadData {
    int* array;
    int start;
    int end;
    bool isDone;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
};

void startFunction() {
    std::cout << "starting" << std::endl;
}

void endFunction() {
    std::cout << "end" << std::endl;
}


DWORD WINAPI SortArrayPart(LPVOID param) {
    ThreadData* data = (ThreadData*)param;

    QueryPerformanceCounter(&data->startTime);  

    std::sort(data->array + data->start, data->array + data->end);

    QueryPerformanceCounter(&data->endTime);  // финальное время
    data->isDone = true;

    
    std::string sortStatus = "Sorting completed for a segment.";
    std::cout << sortStatus << std::endl;

    return 0;
}

void MergeSortedParts(int* array, int size, int numThreads) {
    std::string info = "сортировка разных часте массива и их слияние";
    std::vector<int> mergedArray;
    for (int i = 0; i < numThreads; ++i) {
        int start = (size / numThreads) * i;
        int end = (i == numThreads - 1) ? size : start + (size / numThreads);
        mergedArray.insert(mergedArray.end(), array + start, array + end);
    }
    std::sort(mergedArray.begin(), mergedArray.end());
    std::copy(mergedArray.begin(), mergedArray.end(), array);

    std::string mergeStatus = "Merging sorted parts completed.";
    std::cout << mergeStatus << std::endl;
}

void ShowProgress(ThreadData* threadData, int numThreads) {
    while (true) {
        std::string prog= "Progress";
        std::cout << "----------------------------" << std::endl;
        std::cout << prog << std::endl;
        bool allDone = true;
        for (int i = 0; i < numThreads; ++i) {
            std::string progressStatus = threadData[i].isDone ? "Done" : "Not done";
            std::cout << "Fragment " << i + 1 << ": " << progressStatus << std::endl;
            if (!threadData[i].isDone) {
                allDone = false;
            }
        }
        if (allDone) {
            break;
        }
        Sleep(500); // ждем определенное время чтобы снова проверить свободно или нет
        std::cout << "----------------------------" << std::endl;
    }
}

void ShowThreadTimes(ThreadData* threadData, int numThreads, LARGE_INTEGER frequency) {
    std::cout << "Thread execution times (in milliseconds):" << std::endl;
    for (int i = 0; i < numThreads; ++i) {
        double executionTime = (double)(threadData[i].endTime.QuadPart - threadData[i].startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        std::string timeMessage = "Thread " + std::to_string(i + 1) + ": " + std::to_string(executionTime) + " ms";
        std::cout << timeMessage << std::endl;
    }
}

void GetCpuTimes(FILETIME& idleTime, FILETIME& kernelTime, FILETIME& userTime) {
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
}

ULONGLONG FileTimeToInt64(const FILETIME& ft) {
    return (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

double CalculateCpuUsage(const FILETIME& idleStart, const FILETIME& idleEnd, const FILETIME& kernelStart, const FILETIME& kernelEnd, const FILETIME& userStart, const FILETIME& userEnd) {
    ULONGLONG idleDiff = FileTimeToInt64(idleEnd) - FileTimeToInt64(idleStart);
    ULONGLONG kernelDiff = FileTimeToInt64(kernelEnd) - FileTimeToInt64(kernelStart);
    ULONGLONG userDiff = FileTimeToInt64(userEnd) - FileTimeToInt64(userStart);

    ULONGLONG totalSystem = kernelDiff + userDiff;

    if (totalSystem == 0) {
        return 0.0;
    }

    return (totalSystem - idleDiff) * 100.0 / totalSystem;
}

int main() {
    startFunction();  
    std::string start = ".\\main.exe to start the programm";

    int arraySize, numThreads;
    LARGE_INTEGER frequency;

    QueryPerformanceFrequency(&frequency);  // получаем частоту

    std::string arraySizePrompt = "Enter array size: ";
    std::cout << arraySizePrompt;
    std::cin >> arraySize;

    std::string threadCountPrompt = "Enter number of threads: ";
    std::cout << threadCountPrompt;
    std::cin >> numThreads;

    if (numThreads <= 0 || arraySize <= 0 || numThreads > arraySize) {
        std::string errorMessage = "Invalid parameters!";
        std::cerr << errorMessage << std::endl;
        return 1;
    }

    std::vector<int> array(arraySize);
    srand((unsigned)time(0));

    for (int i = 0; i < arraySize; ++i) {
        array[i] = rand() % 100;
    }

    std::string initialArrayMessage = "Initial array: ";
    std::cout << initialArrayMessage;
    for (int i = 0; i < arraySize; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;

    std::vector<HANDLE> threads(numThreads);
    std::vector<ThreadData> threadData(numThreads);

    int chunkSize = arraySize / numThreads;

    FILETIME idleStart, kernelStart, userStart;
    FILETIME idleEnd, kernelEnd, userEnd;

    GetCpuTimes(idleStart, kernelStart, userStart);  // получаем время

    for (int i = 0; i < numThreads; ++i) {
        threadData[i].array = array.data();
        threadData[i].start = i * chunkSize;
        threadData[i].end = (i == numThreads - 1) ? arraySize : threadData[i].start + chunkSize;
        threadData[i].isDone = false;

        threads[i] = CreateThread(
            NULL, // Security attributes
            0,    // Stack size
            SortArrayPart, // Thread function
            &threadData[i], // Parameter to pass
            0,    // Creation flags
            NULL  // Thread ID
        );
    }

    ShowProgress(threadData.data(), numThreads);

    WaitForMultipleObjects(numThreads, threads.data(), TRUE, INFINITE);

    for (HANDLE thread : threads) {
        CloseHandle(thread);
    }

    MergeSortedParts(array.data(), arraySize, numThreads);

    std::string sortedArrayMessage = "Sorted array: ";
    std::cout << sortedArrayMessage;
    for (int i = 0; i < arraySize; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;

    ShowThreadTimes(threadData.data(), numThreads, frequency);  // выводим стату

    GetCpuTimes(idleEnd, kernelEnd, userEnd);  // получение итога

    double cpuUsage = CalculateCpuUsage(idleStart, idleEnd, kernelStart, kernelEnd, userStart, userEnd);  //считаем производительность

    std::string cpuUsageMessage = "CPU usage during thread execution: ";
    std::cout << cpuUsageMessage << cpuUsage << "%" << std::endl;
    endFunction();


    return 0;
}
