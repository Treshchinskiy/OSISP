#include <windows.h>
#include <iostream>
#include <vector>
#include <random>
#include <atomic>
#include <chrono>
#include <thread>

constexpr int THINKING_TIME_MS = 100;  
constexpr int EATING_TIME_MS = 100;   
constexpr int SIMULATION_DURATION_MS = 5000;

struct PhilosopherStats {
    int id;
    int eating_count = 0;        
    int waiting_time = 0;       
};

std::atomic<bool> running(true); 
std::vector<HANDLE> forks;       
std::vector<PhilosopherStats> stats; 

DWORD  Philosopher(LPVOID param) {
    int id = *(int*)param;
    int left_fork = id;
    int right_fork = (id + 1) % forks.size();
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

        ReleaseSemaphore(forks[left_fork], 1, NULL);
        ReleaseSemaphore(forks[right_fork], 1, NULL);
    }

    return 0;
}

void PrintStats() {
    for (const auto& stat : stats) {
        std::cout << "Philosopher " << stat.id
                  << " ate " << stat.eating_count
                  << " times, waited " << stat.waiting_time << " ms in total.\n";
    }
}

int main() {
    int num_philosophers;

    std::cout << "Enter the number of philosophers: ";
    std::cin >> num_philosophers;

    forks.resize(num_philosophers);
    stats.resize(num_philosophers);

    for (int i = 0; i < num_philosophers; ++i) {
        forks[i] = CreateSemaphore(NULL, 1, 1, NULL); 
        stats[i].id = i;
    }

    std::vector<HANDLE> threads(num_philosophers);
    std::vector<int> ids(num_philosophers);
    for (int i = 0; i < num_philosophers; ++i) {
        ids[i] = i;
        threads[i] = CreateThread(NULL, 0, Philosopher, &ids[i], 0, NULL);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATION_DURATION_MS));
    running = false;

    WaitForMultipleObjects(num_philosophers, threads.data(), TRUE, INFINITE);

    for (HANDLE fork : forks) {
        CloseHandle(fork);
    }

    for (HANDLE thread : threads) {
        CloseHandle(thread);
    }

    PrintStats();

    return 0;
}
