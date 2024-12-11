#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>

struct ThreadData {
    int* array;
    int start;
    int end;
    bool isDone;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
};

DWORD WINAPI SortArrayPart(LPVOID param) {
    ThreadData* data = (ThreadData*)param;
    
    QueryPerformanceCounter(&data->startTime);  
    
    std::sort(data->array + data->start, data->array + data->end);  
    
    QueryPerformanceCounter(&data->endTime); 
    data->isDone = true;
    
    return 0;
}

void MergeSortedParts(int* array, int size, int numThreads) {
    std::vector<int> mergedArray;
    for (int i = 0; i < numThreads; ++i) {
        int start = (size / numThreads) * i;
        int end = (i == numThreads - 1) ? size : start + (size / numThreads);
        mergedArray.insert(mergedArray.end(), array + start, array + end);
    }
    std::sort(mergedArray.begin(), mergedArray.end());
    std::copy(mergedArray.begin(), mergedArray.end(), array);
}

void ShowProgress(ThreadData* threadData, int numThreads) {
    while (true) {
        bool allDone = true;
        for (int i = 0; i < numThreads; ++i) {
            std::cout << "Fragment " << i + 1 << ": " 
                      << (threadData[i].isDone ? "Done" : "Not done") 
                      << std::endl;
            if (!threadData[i].isDone) {
                allDone = false;
            }
        }
        if (allDone) {
            break;
        }
        Sleep(500);
        std::cout << "----------------------------" << std::endl;
    }
}

void ShowThreadTimes(ThreadData* threadData, int numThreads, LARGE_INTEGER frequency) {
    std::cout << "Thread execution times (in milliseconds):" << std::endl;
    for (int i = 0; i < numThreads; ++i) {
        double executionTime = (double)(threadData[i].endTime.QuadPart - threadData[i].startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        std::cout << "Thread " << i + 1 << ": " << executionTime << " ms" << std::endl;
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
    int arraySize, numThreads;
    LARGE_INTEGER frequency;
    
    QueryPerformanceFrequency(&frequency);  

    std::cout << "enter array size: ";
    std::cin >> arraySize;
    
    std::cout << "Enter number of threads: ";
    std::cin >> numThreads;
    
    if (numThreads <= 0 || arraySize <= 0 || numThreads > arraySize) {
        std::cerr << "Invalid parameters!" << std::endl;
        return 1;
    }

    std::vector<int> array(arraySize);
    srand((unsigned)time(0));
    
    for (int i = 0; i < arraySize; ++i) {
        array[i] = rand() % 100;
    }
    
    std::cout << "Initial array: ";
    for (int i = 0; i < arraySize; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;

    std::vector<HANDLE> threads(numThreads);
    std::vector<ThreadData> threadData(numThreads);
    
    int chunkSize = arraySize / numThreads;
    
    FILETIME idleStart, kernelStart, userStart;
    FILETIME idleEnd, kernelEnd, userEnd;

    GetCpuTimes(idleStart, kernelStart, userStart);  

    for (int i = 0; i < numThreads; ++i) {
        threadData[i].array = array.data();
        threadData[i].start = i * chunkSize;
        threadData[i].end = (i == numThreads - 1) ? arraySize : threadData[i].start + chunkSize;
        threadData[i].isDone = false;
        
        threads[i] = CreateThread(
            NULL, //локатор безопасности 
            0, //стек
            SortArrayPart, //указатель на функцию
            &threadData[i], //аргумент который мы передаем в функцию
            0, //флаг создания потока
            NULL //идентификатор потока
        );
    }

    ShowProgress(threadData.data(), numThreads);
    
    WaitForMultipleObjects(numThreads, threads.data(), TRUE, INFINITE);
    
    for (HANDLE thread : threads) {
        CloseHandle(thread);
    }

    MergeSortedParts(array.data(), arraySize, numThreads);

    std::cout << "Sorted array: ";
    for (int i = 0; i < arraySize; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;

    ShowThreadTimes(threadData.data(), numThreads, frequency); 

    GetCpuTimes(idleEnd, kernelEnd, userEnd);  

    double cpuUsage = CalculateCpuUsage(idleStart, idleEnd, kernelStart, kernelEnd, userStart, userEnd);  
    std::cout << "CPU usage during thread execution: " << cpuUsage << "%" << std::endl;

    return 0;
}
