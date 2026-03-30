# Memory inconsistency can derail synchronization

## Testing setups

1. M1 Pro (Laptop 1)
```
================ ARCHITECTURE REPORT (macOS) ================
Model Name:     MacBook Pro
Processor:      Apple M1 Pro (Architecture: arm64)
Total Cores:    8 (Threads: 8)
Byte Order:     Little Endian
--------------------------- CACHES ---------------------------
L1 Data:        64 KiB
L1 Instruction: 128 KiB
L2 Unified:     4 MiB
L3 Cache:       Unified System Level Cache (SLC)
-------------------------- SOFTWARE --------------------------
OS Version:     macOS 15.7.2 (Build 24G325)
Kernel:         Darwin 24.6.0 Darwin Kernel Version 24.6.0: Wed Oct 15 21:12:06 PDT 2025; root:xnu-11417.140.69.703.14~1/RELEASE_ARM64_T6000 arm64
Memory (RAM):   16 GB
==============================================================
```

## Kompilacja
Instalacja Meson, Conan i Ninja

```bash
uv venv # python3 -m venv .venv
source .venv/bin/activate # adjust shell
uv pip install conan meson ninja
```

Instalacja zależności
```bash
cd cpp/src
conan install . --output-folder=../conanbuild --build=missing
```

Konfiguracja środowiska budowania
```bash
meson setup ../mesonbuild --native-file ../conanbuild/conan_meson_native.ini
```

Kompilacja
```bash
meson compile -C ../mesonbuild
```

Uruchomienie
```
../mesonbuild/benchmark_reordering
../mesonbuild/benchmark_reordering_separated
```


## Wyniki testów
1. M1 Pro - `benchmark_reordering_separated.cpp`
```
Load Average: 1.39, 1.76, 2.04
------------------------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------
BM_WithoutBarrier/real_time            446 ns          441 ns    196068699 Reorder_Count=41
BM_WithPartialBarrier/real_time        374 ns          373 ns    212374789 Reorder_Count=0
BM_WithFullBarrier/real_time           378 ns          377 ns    217174315 Reorder_Count=0
```

2. M1 Pro - `benchmark_reordering.cpp`
```
Load Average: 1.73, 2.83, 2.68
------------------------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------
BM_WithoutBarrier/real_time            394 ns          392 ns    224253793 Reorder_Count=8
BM_WithPartialBarrier/real_time        395 ns          394 ns    214398493 Reorder_Count=0
BM_WithBarrier/real_time               416 ns          415 ns    215988921 Reorder_Count=0
```

3. M1 Pro - `benchmark_reordering_separated.cpp` (`--benchmark_min_time=180s`)
```
------------------------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------
BM_WithoutBarrier/real_time            356 ns          355 ns    714142217 Reorder_Count=1
BM_WithPartialBarrier/real_time        436 ns          432 ns    603749475 Reorder_Count=0
BM_WithFullBarrier/real_time           401 ns          398 ns    582111940 Reorder_Count=0
```


## Rezultaty
| Maszyna       | Procesor | Liczba iteracji | Liczba anomalii | Przepustowość (Bez synchronizacji) | Przepustowość (Częściowa bariera) | Przepustowość (Pełna bariera) |
|:--------------| :--- |:----------------|:----------------| :--- | :--- |:----------------------------------|
| **Laptop 1**  | Apple M1 Pro | **~224M** | 8               | 2,54 M-ops/s| 2,53 M-ops/s | 2,40 M-ops/s |

## Notki

Store Coalescing - 
**TLDR** - zapisy w tej samej linii Cache są łączone i wysyłane po wykonaniu (mała szansa anomalii), 
ale będąc w różnych liniach Cache, operacje są rozdzielone (większa szansa anomalii).

procesor może łączyć kilka operacji zapisu dotyczących pojedynczej linii Cache w jedną, co może 
powodować, że wymuszenie znajdowania się zmiennych w innych liniach "poprawia" częstotliwość występowania anomalii, 
ponieważ zmienne nie są łączone w jedną linię Cache, co może prowadzić do większej liczby przypadków, 
gdzie operacje zapisu są widoczne dla innych wątków w różnym czasie.

## Wykresy
![img.png](img.png)