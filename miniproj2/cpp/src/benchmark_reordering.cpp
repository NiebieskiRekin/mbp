#include <atomic>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <pthread.h>

alignas(64) std::atomic<int> x{0}, y{0};
alignas(64) std::atomic<bool> f{false};
std::atomic<uint64_t> x_was_zero{0};

// 0 = Czekaj na start rundy
// 1 = Wykonaj zasadniczne przetwarzanie wątku
// 2 = Zakończ działanie wątku (teardown)
std::atomic<int> run_state{0};
std::atomic<int> done_count{0};

bool with_full_fence = false;
bool with_partial_fence = false;

void *t1_worker([[maybe_unused]] void *arg) {
  while (true) {
    int state = 0;
    // Czekaj na sygnał startu (1) lub zakończenia (2)
    while ((state = run_state.load(std::memory_order_acquire)) == 0)
      ;
    if (state == 2)
      break;

    x.store(1, std::memory_order_relaxed);

    if (with_full_fence) {
      std::atomic_thread_fence(std::memory_order_seq_cst);
    } else {
      std::atomic_signal_fence(std::memory_order_acq_rel);
    }

    if (with_partial_fence) {
        f.store(true, std::memory_order_release);
    } else {
        f.store(true, std::memory_order_relaxed);
    }

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

void *t2_worker([[maybe_unused]] void *arg) {
  while (true) {
    int state = 0;
    while ((state = run_state.load(std::memory_order_acquire)) == 0)
      ;
    if (state == 2)
      break;

    y.store(1, std::memory_order_relaxed);

    if (with_partial_fence) {
        while(!f.load(std::memory_order_acquire));
    } else {
        while(!f.load(std::memory_order_relaxed));
    }

    if (with_full_fence) {
      std::atomic_thread_fence(std::memory_order_seq_cst);
    } else {
      std::atomic_signal_fence(std::memory_order_acq_rel);
    }

    if (x.load(std::memory_order_relaxed) == 0) {
      x_was_zero.fetch_add(1, std::memory_order_relaxed);
    }

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
static void RunReorderTest(benchmark::State &state, bool full, bool partial) {
  // Reset stanu początkowego dla nowej grupy testów
  run_state.store(0, std::memory_order_relaxed);
  done_count.store(0, std::memory_order_relaxed);
  x_was_zero.store(0, std::memory_order_relaxed);

  pthread_t pth1, pth2;
  with_full_fence = full;
  with_partial_fence = partial;
  pthread_create(&pth1, nullptr, t1_worker, nullptr);
  pthread_create(&pth2, nullptr, t2_worker, nullptr);

  for (auto _ : state) {
    // Reset zmiennych dla iteracji
    x.store(0, std::memory_order_relaxed);
    y.store(0, std::memory_order_relaxed);
    f.store(false, std::memory_order_relaxed);
    done_count.store(0, std::memory_order_relaxed);

    // Start przetwarzania przez wątki
    run_state.store(1, std::memory_order_release);

    // Oczekiwanie na zakończenie operacji przez wątki
    while (done_count.load(std::memory_order_acquire) != 2)
      ;

    // Koniec iteracji
    run_state.store(0, std::memory_order_release);
    while (done_count.load(std::memory_order_acquire) != 4)
      ;
  }

  run_state.store(2, std::memory_order_release);
  pthread_join(pth1, nullptr);
  pthread_join(pth2, nullptr);

  state.counters["Reorder_Count"] =
      benchmark::Counter(x_was_zero, benchmark::Counter::kDefaults);
}

static void BM_WithoutBarrier(benchmark::State &state) {
  RunReorderTest(state, false, false);
}
BENCHMARK(BM_WithoutBarrier)->UseRealTime();

static void BM_WithPartialBarrier(benchmark::State &state) {
  RunReorderTest(state, false, true);
}
BENCHMARK(BM_WithPartialBarrier)->UseRealTime();

static void BM_WithBarrier(benchmark::State &state) {
  RunReorderTest(state, true, false);
}
BENCHMARK(BM_WithBarrier)->UseRealTime();

BENCHMARK_MAIN();