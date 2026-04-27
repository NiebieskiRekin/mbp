#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

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

  std::cout << "Starting " << num_trials << " test runs...\n";

  for (int i = 0; i < num_trials; ++i) {
    f.store(false, std::memory_order_relaxed);
    timeout_flag.store(false, std::memory_order_relaxed);

    std::thread t1(thread_1);
    std::thread t2(thread_2);

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

 // ??? TODO: export vector(success_times) to a csv for histogram generation ???

  return 0;
}