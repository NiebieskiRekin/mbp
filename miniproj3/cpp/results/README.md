### Weak fairness preclude
![Screenshot 2026-04-26 at 19-47-45 psw-handout.pdf.png](../../Screenshot%202026-04-26%20at%2019-47-45%20psw-handout.pdf.png)

### Weak fairness admit
![Screenshot 2026-04-26 at 19-46-16 psw-handout.pdf.png](../../Screenshot%202026-04-26%20at%2019-46-16%20psw-handout.pdf.png)

### Preclude C++
```C++
void thread_1() {
  // await f.load(||)
  while (!f.load(std::memory_order_relaxed)) {
  }
  // fence(R||RW)
  std::atomic_thread_fence(std::memory_order_acquire);
}
```

```C++
void thread_2() {
// await ¬f.load(||)
  while (f.load(std::memory_order_relaxed)) {
  }
  std::atomic_thread_fence(std::memory_order_acquire);
  
  // f.store(true)
  f.store(true);
}
```

### Admit C++
```C++
void thread_1() {
  // while ¬f.load(||W)
  while (!f.load(std::memory_order_acquire)) {

    // g.store(true, ||W)
    g.store(true, std::memory_order_release);

    // g.store(false, ||R)
    g.store(false, std::memory_order_seq_cst);
  }
}
```

```C++
void thread_2() {
  // await g.load(||)
  while (!g.load(std::memory_order_relaxed)) {
  }

  // fence(R∥RW)
  std::atomic_thread_fence(std::memory_order_acquire);

  // f.store(true, ||)
  f.store(true, std::memory_order_relaxed);
}
```


### Results x86/ARM
![histogram_pmem_mac.png](histogram_pmem_mac.png)


### Interval of `g=true`
```C++
void thread_1() {
  // while ¬f.load(||W)
  while (!f.load(std::memory_order_acquire)) {

    // g.store(true, ||W)
    g.store(true, std::memory_order_release);

>>> for(volatile int i=0; i<15; i++);

    // g.store(false, ||R)
    g.store(false, std::memory_order_seq_cst);
  }
}
```

### Results ARM
![histogram_mac_mac.png](histogram_mac_mac.png)