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

enum class Mode { NONE, PARTIAL, FULL };

template <Mode M>
void *t1_worker([[maybe_unused]] void *arg) {
  while (true) {
    int state = 0;
    while ((state = run_state.load(std::memory_order_acquire)) == 0);
    if (state == 2) break;

    x.store(1, std::memory_order_relaxed);

    if constexpr (M == Mode::FULL) {
      std::atomic_thread_fence(std::memory_order_seq_cst);
      f.store(true, std::memory_order_relaxed);
    }
    else if constexpr (M == Mode::PARTIAL) {
      std::atomic_signal_fence(std::memory_order_acq_rel);
      f.store(true, std::memory_order_release);
    }
    else {
      std::atomic_signal_fence(std::memory_order_acq_rel);
      f.store(true, std::memory_order_relaxed);
    }

    done_count.fetch_add(1, std::memory_order_release);
    while ((state = run_state.load(std::memory_order_acquire)) == 1);
    if (state == 2) break;
    done_count.fetch_add(1, std::memory_order_release);
  }
  return nullptr;
}

template <Mode M> void *t2_worker([[maybe_unused]] void *arg) {
  while (true) {
    int state = 0;
    while ((state = run_state.load(std::memory_order_acquire)) == 0);
    if (state == 2) break;

    y.store(1, std::memory_order_relaxed);

    if constexpr (M == Mode::PARTIAL) {
      while(!f.load(std::memory_order_acquire));
    } else {
      while(!f.load(std::memory_order_relaxed));
    }

    if constexpr (M == Mode::FULL) {
      std::atomic_thread_fence(std::memory_order_seq_cst);
    } else {
      std::atomic_signal_fence(std::memory_order_acq_rel);
    }

    if (x.load(std::memory_order_relaxed) == 0) {
      x_was_zero.fetch_add(1, std::memory_order_relaxed);
    }

    done_count.fetch_add(1, std::memory_order_release);
    while ((state = run_state.load(std::memory_order_acquire)) == 1);
    if (state == 2) break;
    done_count.fetch_add(1, std::memory_order_release);
  }
  return nullptr;
}

// Główny silnik zarządzający testem
template <Mode M>
static void RunReorderTest(benchmark::State &state) {
  // Reset stanu początkowego dla nowej grupy testów
  run_state.store(0, std::memory_order_relaxed);
  done_count.store(0, std::memory_order_relaxed);
  x_was_zero.store(0, std::memory_order_relaxed);

  pthread_t pth1, pth2;
  pthread_create(&pth1, nullptr, t1_worker<M>, nullptr);
  pthread_create(&pth2, nullptr, t2_worker<M>, nullptr);

  for (auto _ : state) {
    // Reset zmiennych dla iteracji
    x.store(0, std::memory_order_relaxed);
    y.store(0, std::memory_order_relaxed);
    f.store(false, std::memory_order_relaxed);
    done_count.store(0, std::memory_order_relaxed);

    // Start przetwarzania przez wątki
    run_state.store(1, std::memory_order_release);

    // Oczekiwanie na zakończenie operacji przez wątki
    while (done_count.load(std::memory_order_acquire) != 2);

    // Koniec iteracji
    run_state.store(0, std::memory_order_release);
    while (done_count.load(std::memory_order_acquire) != 4);
  }

  run_state.store(2, std::memory_order_release);
  pthread_join(pth1, nullptr);
  pthread_join(pth2, nullptr);

  state.counters["Reorder_Count"] =
      benchmark::Counter(x_was_zero, benchmark::Counter::kDefaults);
}

static void BM_WithoutBarrier(benchmark::State &state) {
  RunReorderTest<Mode::NONE>(state);
}
BENCHMARK(BM_WithoutBarrier)->UseRealTime();

static void BM_WithPartialBarrier(benchmark::State &state) {
  RunReorderTest<Mode::PARTIAL>(state);
}
BENCHMARK(BM_WithPartialBarrier)->UseRealTime();

static void BM_WithFullBarrier(benchmark::State &state) {
  RunReorderTest<Mode::FULL>(state);
}
BENCHMARK(BM_WithFullBarrier)->UseRealTime();

BENCHMARK_MAIN();