#include <atomic>
#include <pthread.h>

/*
weak fairness admits an execution in
which thread 1 spins forever
*/

std::atomic<bool> f{false};
std::atomic<bool> g{false};

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

/*
while ¬f.load(∥W)
  g.store(true,∥W)
  g.store(false,∥R)
*/
int t1(void *) {
  /*
  f.load(∥W) = LoadStore -> memory_order_acquire = LoadLoad + LoadStore (||RW)
  */
  while (!f.load(std::memory_order_acquire)) {
    // StoreStore -> memory_order_release = StoreStore + LoadStore (RW||W)
    g.store(true, std::memory_order_release);

    // StoreLoad -> memory_order_seq_cst = Release (RW||W) + StoreLoad (W||R)
    g.store(false, std::memory_order_seq_cst);
  }
  return 1;
}

/*
await g.load(∥)
f.store(true,∥)
*/
int t2(void *) {
  await(g.load(std::memory_order_relaxed));
  f.store(true, std::memory_order_relaxed);
  return 1;
}

int main() { return 0; }