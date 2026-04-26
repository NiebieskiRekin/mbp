#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

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

  // f.store(true, ||)
  f.store(true, std::memory_order_relaxed);
}

int main() {
  const int num_trials = 10000;
  int starvation_count = 0;

  std::cout << "Starting " << num_trials << " test runs...\n";

  for (int i = 0; i < num_trials; ++i) {
    // Reset state for the run
    f.store(false, std::memory_order_relaxed);
    g.store(false, std::memory_order_relaxed);
    timeout_flag.store(false, std::memory_order_relaxed);

    std::thread t1(thread_1);
    std::thread t2(thread_2);

    // Give the threads a short window to resolve naturally.
    // If they don't resolve in 100 microseconds, we assume they are caught
    // in the weak fairness starvation loop.
    std::this_thread::sleep_for(std::chrono::microseconds(1000));

    // If f is still false, T1 is still spinning. Trigger the timeout.
    if (!f.load(std::memory_order_relaxed)) {
      timeout_flag.store(true, std::memory_order_relaxed);
      starvation_count++;
    }

    t1.join();
    t2.join();
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

  return 0;
}