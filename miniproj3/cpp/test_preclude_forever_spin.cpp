#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <fstream>

/*
weak fairness precludes an execution in
which thread 1 spins forever
*/

std::atomic<bool> f{false};
std::atomic<bool> timeout_flag{false};

void thread_1() {
  // await f.load(||)
  while (!f.load(std::memory_order_relaxed)) {
    if (timeout_flag.load(std::memory_order_relaxed)) return;
  }
  // fence(R||RW)
  std::atomic_thread_fence(std::memory_order_acquire);
}

void thread_2() {
  // await ¬f.load(||)
  while (f.load(std::memory_order_relaxed)) {
    if (timeout_flag.load(std::memory_order_relaxed)) return;
  }
  std::atomic_thread_fence(std::memory_order_acquire);

  // f.store(true)
  f.store(true);
}

int main() {
  const int num_trials = 10000;
  int starvation_count = 0;

  std::vector<long long> success_times;
  success_times.reserve(num_trials);

  std::cout << "Starting " << num_trials << " test runs...\n";

  for (int i = 0; i < num_trials; ++i) {
    f.store(false, std::memory_order_relaxed);
    timeout_flag.store(false, std::memory_order_relaxed);

    auto start_time = std::chrono::high_resolution_clock::now();

    std::thread t1(thread_1);
    std::thread t2(thread_2);

    bool starved = true;
    long long duration_us = 0;

    while (true) {
      // 1. Check if the threads resolved naturally
      if (f.load(std::memory_order_relaxed)) {
        auto end_time = std::chrono::high_resolution_clock::now();
        duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        starved = false;
        break;
      }

      // 2. Check if the timeout limit (1 second) has been exceeded
      auto current_time = std::chrono::high_resolution_clock::now();
      auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count();

      if (elapsed_us > 1000000) {
        // Trigger the timeout flag to safely terminate the spinning threads
        timeout_flag.store(true, std::memory_order_relaxed);
        break;
      }
    }

    t1.join();
    t2.join();

    // Record the outcome of this specific trial
    if (starved) {
      starvation_count++;
    } else {
      success_times.push_back(duration_us);
    }
  }

  std::cout << "--------------------------------------\n";
  std::cout << "Total Runs: " << num_trials << "\n";
  std::cout << "Starvation Events (Infinite Spins): " << starvation_count
            << "\n";
  std::cout << "Natural Resolutions: " << (num_trials - starvation_count)
            << "\n";
  std::cout << "Starvation Rate: "
            << (static_cast<double>(starvation_count) / num_trials) * 100
            << "%\n";

 std::cout << "Exporting to results.csv...\n";
 std::ofstream csv_file("preclude_results.csv");
 if (csv_file.is_open()) {
   csv_file << "Trial,Latency_us\n";
   for (size_t i = 0; i < success_times.size(); ++i) {
     csv_file << i << "," << success_times[i] << "\n";
   }
   csv_file.close();
   std::cout << "Export complete.\n";
 } else {
   std::cerr << "Failed to open results.csv for writing.\n";
 }

  return 0;
}