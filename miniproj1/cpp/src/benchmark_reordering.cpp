#include <atomic>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <pthread.h>

std::atomic<int> x{0}, y{0};
int i, j;

// 0 = Czekaj na start rundy
// 1 = Wykonaj zasadniczne przetwarzanie wątku
// 2 = Zakończ działanie wątku (teardown)
std::atomic<int> run_state{0};
std::atomic<int> done_count{0};

void *t1_worker(void *arg) {
  // Odczytujemy, czy w tym teście używamy bariery sprzętowej
  bool with_fence = (bool)(uintptr_t)arg;

  while (true) {
    int state = 0;
    // Czekaj na sygnał startu (1) lub zakończenia (2)
    while ((state = run_state.load(std::memory_order_acquire)) == 0)
      ;
    if (state == 2)
      break;

    x.store(1, std::memory_order_relaxed);

    if (with_fence) {
      std::atomic_thread_fence(std::memory_order_seq_cst);
    } else {
      std::atomic_signal_fence(std::memory_order_acq_rel);
    }

    i = y.load(std::memory_order_relaxed);

    // Zgłoś gotowość po wykonaniu sekcji krytycznej
    done_count.fetch_add(1, std::memory_order_release);

    // Czekaj na reset przez wątek główny
    while ((state = run_state.load(std::memory_order_acquire)) == 1)
      ;
    if (state == 2)
      break;

    done_count.fetch_add(1, std::memory_order_release);
  }
  return nullptr;
}

void *t2_worker(void *arg) {
  bool with_fence = (bool)(uintptr_t)arg;

  while (true) {
    int state = 0;
    while ((state = run_state.load(std::memory_order_acquire)) == 0)
      ;
    if (state == 2)
      break;

    y.store(1, std::memory_order_relaxed);

    if (with_fence) {
      std::atomic_thread_fence(std::memory_order_seq_cst);
    } else {
      std::atomic_signal_fence(std::memory_order_acq_rel);
    }

    j = x.load(std::memory_order_relaxed);

    done_count.fetch_add(1, std::memory_order_release);

    while ((state = run_state.load(std::memory_order_acquire)) == 1)
      ;
    if (state == 2)
      break;

    done_count.fetch_add(1, std::memory_order_release);
  }
  return nullptr;
}

// Główny silnik zarządzający testem
static void RunReorderTest(benchmark::State &state, bool with_fence) {
  // Reset stanu początkowego dla nowej grupy testów
  run_state.store(0, std::memory_order_relaxed);
  done_count.store(0, std::memory_order_relaxed);
  uint64_t both_zero_count = 0;

  pthread_t pth1, pth2;
  void *arg = (void *)(uintptr_t)with_fence;
  pthread_create(&pth1, nullptr, t1_worker, arg);
  pthread_create(&pth2, nullptr, t2_worker, arg);

  for (auto _ : state) {
    // Reset zmiennych dla iteracji
    x.store(0, std::memory_order_relaxed);
    y.store(0, std::memory_order_relaxed);
    i = 2;
    j = 2;
    done_count.store(0, std::memory_order_relaxed);

    // Start przetwarzania przez wątki
    run_state.store(1, std::memory_order_release);

    // Oczekiwanie na zakończenie operacji przez wątki
    while (done_count.load(std::memory_order_acquire) != 2)
      ;

    if (i == 0 && j == 0) {
      both_zero_count++;
    }

    // Koniec iteracji
    run_state.store(0, std::memory_order_release);
    while (done_count.load(std::memory_order_acquire) != 4)
      ;
  }

  run_state.store(2, std::memory_order_release);
  pthread_join(pth1, nullptr);
  pthread_join(pth2, nullptr);

  state.counters["Reorder_Count"] =
      benchmark::Counter(both_zero_count, benchmark::Counter::kDefaults);
}

static void BM_WithoutBarrier(benchmark::State &state) {
  RunReorderTest(state, false);
}
BENCHMARK(BM_WithoutBarrier)->UseRealTime();

static void BM_WithBarrier(benchmark::State &state) {
  RunReorderTest(state, true);
}
BENCHMARK(BM_WithBarrier)->UseRealTime();

BENCHMARK_MAIN();