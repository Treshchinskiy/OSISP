#include <windows.h>
#include <iostream>
#include <vector>
#include <random>
#include <atomic>
#include <time.h>
#include <chrono>
#include <thread>
#include <string>

constexpr int THINKING_TIME_MS = 100;  
constexpr int EATING_TIME_MS = 100;   
constexpr int SIMULATION_DURATION_MS = 5000;

//структура для привидения статистики
struct PhilosopherStats {
    int id;
    int eating_count = 0;       
    int waiting_time = 0;       
};

std::atomic<bool> running(true); //флаги
std::vector<HANDLE> forks;        //семафоры
std::vector<PhilosopherStats> stats; //статистика

// Функция, выполняемая каждым потоком философа
DWORD Philosopher(LPVOID param) {
    int id = *(int*)param; 
    int left_fork = id; //индекс левой вилки
    int right_fork = (id + 1) % forks.size(); //индекс правой вилки

    auto& stat = stats[id];
    auto start_time = std::chrono::steady_clock::now();

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(THINKING_TIME_MS));

        auto wait_start = std::chrono::steady_clock::now();
        
        WaitForSingleObject(forks[left_fork], INFINITE);
        WaitForSingleObject(forks[right_fork], INFINITE);
        
        auto wait_end = std::chrono::steady_clock::now();
        stat.waiting_time += std::chrono::duration_cast<std::chrono::milliseconds>(wait_end - wait_start).count();

        stat.eating_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(EATING_TIME_MS));
        //освобождение ресурсов
        ReleaseSemaphore(forks[left_fork], 1, NULL);
        ReleaseSemaphore(forks[right_fork], 1, NULL);
    }

    return 0;
}

// Функция для вывода статистики по всем философам
void PrintStats() {
    std::cout << "\n--- Simulation Results ---\n";
    for (const auto& stat : stats) {
        std::cout << "Philosopher " << stat.id
                  << " ate " << stat.eating_count
                  << " times, waited a total of " << stat.waiting_time << " ms.\n";
    }
    std::cout << "--------------------------\n";
}


void DisplayStartMessage() {
    std::cout << "Execution has commenced." << std::endl;
}

void DisplayEndMessage() {
    std::cout << "Execution has completed." << std::endl;
}


int main() {
    DisplayStartMessage();
    int num_philosophers;

    std::cout << "Enter the number of philosophers: ";
    std::cin >> num_philosophers;

    if (num_philosophers < 2) {
        std::cerr << "There must be at least 2 philosophers to run this simulation.\n";
        return 1;
    }

    forks.resize(num_philosophers);
    stats.resize(num_philosophers);

    for (int i = 0; i < num_philosophers; ++i) {
        forks[i] = CreateSemaphore(NULL, 1, 1, NULL); //типа вилки
        stats[i].id = i;
    }

    std::vector<HANDLE> threads(num_philosophers);
    std::vector<int> ids(num_philosophers);

    for (int i = 0; i < num_philosophers; ++i) {
        ids[i] = i;
        threads[i] = CreateThread(NULL, 0, Philosopher, &ids[i], 0, NULL);
        if (!threads[i]) {
            std::cerr << "Error creating thread for philosopher " << i << std::endl;
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATION_DURATION_MS));
    running = false; 

    //ждем всех обьектов
    WaitForMultipleObjects(num_philosophers, threads.data(), TRUE, INFINITE);

    for (HANDLE fork : forks) {
        CloseHandle(fork);
    }

    //закрытие дескрипторов 
    for (HANDLE thread : threads) {
        CloseHandle(thread);
    }

    PrintStats();
    DisplayEndMessage();
    
    return 0;
}
