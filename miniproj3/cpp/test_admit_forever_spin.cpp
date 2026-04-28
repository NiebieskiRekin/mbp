#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>

std::atomic<bool> f{false};
std::atomic<bool> g{false};
std::atomic<bool> timeout_flag{false};

void thread_1() {
  // while ¬f.load(||W)
  while (!f.load(std::memory_order_acquire)) {
    if (timeout_flag.load(std::memory_order_relaxed))
      return;

    // g.store(true, ||W)
    g.store(true, std::memory_order_release);

    // g.store(false, ||R)
    g.store(false, std::memory_order_seq_cst);
  }
}

void thread_2() {
  // await g.load(||)
  while (!g.load(std::memory_order_relaxed)) {
    if (timeout_flag.load(std::memory_order_relaxed))
      return;
  }

  // fence(R∥RW)
  std::atomic_thread_fence(std::memory_order_acquire);

  // f.store(true, ||)
  f.store(true, std::memory_order_relaxed);
}

int main() {
  const int num_trials = 10000;
  int starvation_count = 0;

  std::vector<long long> success_times;
  success_times.reserve(num_trials);

  std::cout << "Starting " << num_trials << " test runs...\n";

  for (int i = 0; i < num_trials; ++i) {
    // Reset state for the run
    f.store(false, std::memory_order_relaxed);
    g.store(false, std::memory_order_relaxed);
    timeout_flag.store(false, std::memory_order_relaxed);

    // Start timer
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
 std::ofstream csv_file("admit_results.csv");
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