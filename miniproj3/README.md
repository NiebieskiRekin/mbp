# Example of forward progress by strong fairness

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
conan install . --output-folder=build/x86_64 --build=missing
```

Konfiguracja środowiska budowania
```bash
meson setup build/meson-x86_64 --native-file build/x86_64/conan_meson_native.ini
```

Kompilacja
```bash
meson compile -C build/meson-x86_64
```

Uruchomienie
```
build/meson-x86_64/benchmark_reordering_separated
```

Bariery w C++
```
# R || RW
          #LoadLoad
          #LoadStore
          #memory_order_acquire

# RW || W
          #LoadStore
          #StoreStore
          #memory_order_release

# RW || RW
          #LoadStore
          #LoadLoad
          #StoreStore
          #StoreLoad
          #memory_order_seq_cst
```