#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

// Use relaxed atomics to prevent compiler reordering but allow CPU reordering
std::atomic<int> x{0}, y{0};
int i, j;

// Synchronization primitives
std::atomic<int> start_flag{0};
std::atomic<int> done_count{0};

static uint64_t run_count = 1;
static uint64_t both_zero_count = 0;

void sigint_handler(int sig) {
  printf("\n%lu runs, i=j=0 count: %lu\n", run_count - 1, both_zero_count);
  _exit(sig);
}

void *t1(void *) {
  while (true) {
    // Wait for the signal to start
    while (start_flag.load(std::memory_order_acquire) != 1)
      ;

    x.store(1, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    i = y.load(std::memory_order_relaxed);

    // Signal we are done with this iteration
    done_count.fetch_add(1, std::memory_order_release);

    // Wait for main thread to reset the flag
    while (start_flag.load(std::memory_order_acquire) == 1)
      ;
    done_count.fetch_add(1, std::memory_order_release);
  }
  return nullptr;
}

void *t2(void *) {
  while (true) {
    // Wait for the signal to start
    while (start_flag.load(std::memory_order_acquire) != 1)
      ;

    y.store(1, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    j = x.load(std::memory_order_relaxed);

    // Signal we are done with this iteration
    done_count.fetch_add(1, std::memory_order_release);

    // Wait for main thread to reset the flag
    while (start_flag.load(std::memory_order_acquire) == 1)
      ;
    done_count.fetch_add(1, std::memory_order_release);
  }
  return nullptr;
}

int main() {
  signal(SIGINT, sigint_handler);

  pthread_t pth1, pth2;
  pthread_create(&pth1, NULL, t1, NULL);
  pthread_create(&pth2, NULL, t2, NULL);

  while (1) {
    // Reset variables
    x.store(0, std::memory_order_relaxed);
    y.store(0, std::memory_order_relaxed);
    i = 0;
    j = 0;
    done_count.store(0, std::memory_order_relaxed);

    // Break threads out of busy wait
    start_flag.store(1, std::memory_order_release);

    // Wait for both threads to finish processing
    while (done_count.load(std::memory_order_acquire) != 2)
      ;

    if (i == 0 && j == 0) {
      both_zero_count++;
      printf("Run %lu: i=0, j=0\n", run_count);
    }

    // Reset the start flag
    start_flag.store(0, std::memory_order_release);
    while (done_count.load(std::memory_order_acquire) != 4)
      ;

    run_count++;
  }
}