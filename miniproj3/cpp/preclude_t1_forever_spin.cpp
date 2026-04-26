#include <atomic>
#include <pthread.h>

/*
weak fairness precludes an execution in
which thread 1 spins forever
*/

std::atomic<bool> f{false};

/*
while ¬condition
  ; // spin
fence(R∥RW)
*/
inline void await(std::atomic<bool> condition) {
  while (!condition.load(std::memory_order_relaxed))
    ;
  std::atomic_thread_fence(std::memory_order_acquire);
}

// await f.load(∥)
int t1(void *) {
  await(f.load(std::memory_order_relaxed));
  return 1;
}

/*
await ¬f.load(∥)
f.store(true)
*/
int t2(void *) {
  await(!f.load(std::memory_order_relaxed));
  f.store(true);
  return 1;
}

int main() { return 0; }