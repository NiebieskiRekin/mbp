# Testing circular code reordering from reads bypassing writes

## Testing setups

1. Laptop 1:
- ```
  Architecture:                x86_64
    CPU op-mode(s):            32-bit, 64-bit
    Address sizes:             48 bits physical, 48 bits virtual
    Byte Order:                Little Endian
    CPU(s):                      12
    On-line CPU(s) list:       0-11
    Vendor ID:                   AuthenticAMD
    Model name:                AMD Ryzen 5 5600U with Radeon Graphics
        CPU family:              25
        Model:                   80
        Thread(s) per core:      2
        Core(s) per socket:      6
        Socket(s):               1
        Stepping:                0
        Frequency boost:         enabled
        CPU(s) scaling MHz:      76%
        CPU max MHz:             4291,7529
        CPU min MHz:             413,6630
        BogoMIPS:                4591,31
        Flags:                   fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ht syscall nx mmxext fxsr_opt pdpe1gb rdtscp lm constant_tsc rep_good nopl xtopology nonstop_tsc cpuid extd_apicid aperfmperf rapl pni pclmulqdq monito
                                r ssse3 fma cx16 sse4_1 sse4_2 movbe popcnt aes xsave avx f16c rdrand lahf_lm cmp_legacy svm extapic cr8_legacy abm sse4a misalignsse 3dnowprefetch osvw ibs skinit wdt tce topoext perfctr_core perfctr_nb bpext perfctr_llc mwaitx cpb cat_l3 cdp_l3 hw
                                _pstate ssbd mba ibrs ibpb stibp vmmcall fsgsbase bmi1 avx2 smep bmi2 erms invpcid cqm rdt_a rdseed adx smap clflushopt clwb sha_ni xsaveopt xsavec xgetbv1 xsaves cqm_llc cqm_occup_llc cqm_mbm_total cqm_mbm_local user_shstk clzero irperf xsaveerptr 
                                rdpru wbnoinvd cppc arat npt lbrv svm_lock nrip_save tsc_scale vmcb_clean flushbyasid decodeassists pausefilter pfthreshold avic v_vmsave_vmload vgif v_spec_ctrl umip pku ospke vaes vpclmulqdq rdpid overflow_recov succor smca fsrm debug_swap
    Virtualization features:     
    Virtualization:            AMD-V
    Caches (sum of all):         
    L1d:                       192 KiB (6 instances)
    L1i:                       192 KiB (6 instances)
    L2:                        3 MiB (6 instances)
    L3:                        16 MiB (1 instance)
    NUMA:                        
    NUMA node(s):              1
    NUMA node0 CPU(s):         0-11
    Vulnerabilities:             
    Gather data sampling:      Not affected
    Ghostwrite:                Not affected
    Indirect target selection: Not affected
    Itlb multihit:             Not affected
    L1tf:                      Not affected
    Mds:                       Not affected
    Meltdown:                  Not affected
    Mmio stale data:           Not affected
    Old microcode:             Not affected
    Reg file data sampling:    Not affected
    Retbleed:                  Not affected
    Spec rstack overflow:      Mitigation; Safe RET
    Spec store bypass:         Mitigation; Speculative Store Bypass disabled via prctl
    Spectre v1:                Mitigation; usercopy/swapgs barriers and __user pointer sanitization
    Spectre v2:                Mitigation; Retpolines; IBPB conditional; IBRS_FW; STIBP always-on; RSB filling; PBRSB-eIBRS Not affected; BHI Not affected
    Srbds:                     Not affected
    Tsa:                       Mitigation; Clear CPU buffers
    Tsx async abort:           Not affected
    Vmscape:                   Mitigation; IBPB before exit to userspace

  ---

  HP Pavilion Aero Laptop 13-be0xxx
  CPU: AMD Ryzen 5 5600U x86_64 12 Threads 1 NUMA node
  RAM: 16GiB DDR4 3200 MT/s SODIMM Volatile Synchronous Unbuffered (Unregistered, non-ECC) Dual-channel
  OS: Fedora Linux 43 ; Linux 6.19.7-200.fc43.x86_64 #1 SMP PREEMPT_DYNAMIC

  ```

2. Desktop 1:
  ```
  Architecture:             x86_64
    CPU op-mode(s):         32-bit, 64-bit
    Address sizes:          39 bits physical, 48 bits virtual
    Byte Order:             Little Endian
  CPU(s):                   4
    On-line CPU(s) list:    0-3
  Vendor ID:                GenuineIntel
    BIOS Vendor ID:         QEMU
    Model name:             Intel(R) Core(TM) i5-6400 CPU @ 2.70GHz
      BIOS Model name:      pc-i440fx-10.1  CPU @ 2.0GHz
      BIOS CPU family:      1
      CPU family:           6
      Model:                94
      Thread(s) per core:   1
      Core(s) per socket:   4
      Socket(s):            1
      Stepping:             3
      BogoMIPS:             5423.99
      Flags:                fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ss ht syscall nx pdpe
                            1gb rdtscp lm constant_tsc arch_perfmon rep_good nopl xtopology cpuid tsc_known_freq pni pclmulqdq vmx ssse3 fma cx16 p
                            dcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand hypervisor lahf_lm abm 3dnowpre
                            fetch cpuid_fault invpcid_single pti ssbd ibrs ibpb stibp tpr_shadow vnmi flexpriority ept vpid ept_ad fsgsbase tsc_adj
                            ust bmi1 avx2 smep bmi2 erms invpcid mpx rdseed adx smap clflushopt xsaveopt xsavec xgetbv1 xsaves arat umip md_clear f
                            lush_l1d arch_capabilities
  Virtualization features:  
    Virtualization:         VT-x
    Hypervisor vendor:      KVM
    Virtualization type:    full
  Caches (sum of all):      
    L1d:                    128 KiB (4 instances)
    L1i:                    128 KiB (4 instances)
    L2:                     16 MiB (4 instances)
    L3:                     16 MiB (1 instance)
  NUMA:                     
    NUMA node(s):           1
    NUMA node0 CPU(s):      0-3
  Vulnerabilities:          
    Gather data sampling:   Unknown: Dependent on hypervisor status
    Itlb multihit:          Not affected
    L1tf:                   Mitigation; PTE Inversion; VMX flush not necessary, SMT disabled
    Mds:                    Mitigation; Clear CPU buffers; SMT Host state unknown
    Meltdown:               Mitigation; PTI
    Mmio stale data:        Mitigation; Clear CPU buffers; SMT Host state unknown
    Reg file data sampling: Not affected
    Retbleed:               Mitigation; IBRS
    Spec rstack overflow:   Not affected
    Spec store bypass:      Mitigation; Speculative Store Bypass disabled via prctl
    Spectre v1:             Mitigation; usercopy/swapgs barriers and __user pointer sanitization
    Spectre v2:             Mitigation; IBRS; IBPB conditional; STIBP disabled; RSB filling; PBRSB-eIBRS Not affected; BHI SW loop, KVM SW loop
    Srbds:                  Unknown: Dependent on hypervisor status
    Tsx async abort:        Not affected
  
  ---

  CPU: Intel(R) Core(TM) i5-6400 CPU @ 2.70GHz x86_64 4 Threads 1 NUMA node
  RAM: 8GiB DDR4 2133 MT/s DIMM Volatile Synchronous (non-ECC)
  OS: Debian 12 Linux ; Linux 6.1.0-31-amd64 #1 SMP PREEMPT_DYNAMIC

  ```

3. Desktop 2:
  ```

Architecture:                x86_64
  CPU op-mode(s):            32-bit, 64-bit
  Address sizes:             48 bits physical, 48 bits virtual
  Byte Order:                Little Endian
CPU(s):                      24
  On-line CPU(s) list:       0-23
Vendor ID:                   AuthenticAMD
  Model name:                AMD Ryzen 9 9900X 12-Core Processor
    CPU family:              26
    Model:                   68
    Thread(s) per core:      2
    Core(s) per socket:      12
    Socket(s):               1
    Stepping:                0
    Frequency boost:         enabled
    CPU(s) scaling MHz:      49%
    CPU max MHz:             5662,0161
    CPU min MHz:             613,9540
    BogoMIPS:                8782,92
    Flags:                   fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ht syscall nx mmxext fxsr_opt pdpe1gb rdtscp lm constant_tsc rep_good amd_lbr_v2 n
                             opl xtopology nonstop_tsc cpuid extd_apicid aperfmperf rapl pni pclmulqdq monitor ssse3 fma cx16 sse4_1 sse4_2 movbe popcnt aes xsave avx f16c rdrand lahf_lm cmp_legacy svm extapic
                              cr8_legacy abm sse4a misalignsse 3dnowprefetch osvw ibs skinit wdt tce topoext perfctr_core perfctr_nb bpext perfctr_llc mwaitx cpuid_fault cpb cat_l3 cdp_l3 hw_pstate ssbd mba pe
                             rfmon_v2 ibrs ibpb stibp ibrs_enhanced vmmcall fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid cqm rdt_a avx512f avx512dq rdseed adx smap avx512ifma clflushopt clwb avx512cd s
                             ha_ni avx512bw avx512vl xsaveopt xsavec xgetbv1 xsaves cqm_llc cqm_occup_llc cqm_mbm_total cqm_mbm_local user_shstk avx_vnni avx512_bf16 clzero irperf xsaveerptr rdpru wbnoinvd cpp
                             c arat npt lbrv svm_lock nrip_save tsc_scale vmcb_clean flushbyasid decodeassists pausefilter pfthreshold avic v_vmsave_vmload vgif x2avic v_spec_ctrl vnmi avx512vbmi umip pku ospk
                             e avx512_vbmi2 gfni vaes vpclmulqdq avx512_vnni avx512_bitalg avx512_vpopcntdq rdpid bus_lock_detect movdiri movdir64b overflow_recov succor smca fsrm avx512_vp2intersect flush_l1d
                              amd_lbr_pmc_freeze
Virtualization features:     
  Virtualization:            AMD-V
Caches (sum of all):         
  L1d:                       576 KiB (12 instances)
  L1i:                       384 KiB (12 instances)
  L2:                        12 MiB (12 instances)
  L3:                        64 MiB (2 instances)
NUMA:                        
  NUMA node(s):              1
  NUMA node0 CPU(s):         0-23
Vulnerabilities:             
  Gather data sampling:      Not affected
  Ghostwrite:                Not affected
  Indirect target selection: Not affected
  Itlb multihit:             Not affected
  L1tf:                      Not affected
  Mds:                       Not affected
  Meltdown:                  Not affected
  Mmio stale data:           Not affected
  Old microcode:             Not affected
  Reg file data sampling:    Not affected
  Retbleed:                  Not affected
  Spec rstack overflow:      Mitigation; IBPB on VMEXIT only
  Spec store bypass:         Mitigation; Speculative Store Bypass disabled via prctl
  Spectre v1:                Mitigation; usercopy/swapgs barriers and __user pointer sanitization
  Spectre v2:                Mitigation; Enhanced / Automatic IBRS; IBPB conditional; STIBP always-on; PBRSB-eIBRS Not affected; BHI Not affected
  Srbds:                     Not affected
  Tsa:                       Not affected
  Tsx async abort:           Not affected
  Vmscape:                   Mitigation; IBPB on VMEXIT

---

CPU: AMD Ryzen 9 9900X 12-Core Processor @ 5.60GHz x86_64 24 Threads 1 NUMA node
RAM: 64GiB DDR5 6000 MT/s DIMM Volatile Synchronous Unbuffered (Ungregistered, non-ECC) Dual-channel
OS: Fedora 43 Linux ; Linux 6.19.7-200.fc43.x86_64 #1 SMP PREEMPT_DYNAMIC

```




Zjawisko inaczej nazywane: StoreLoad reordering

https://cppreference.com/w/c/atomic/memory_order.html


Po poprawieniu implementacji korzystając z barier (`main.cpp` na laptop 1):
```
566325 runs, i=j=0 count 202
```
Czyli jakieś 4 razy na 10 000 wywołań.

Ale dużo skacze po procesorze, więc pewnie dalej usypia wątki i potencjalnie szereguje wątki jeden po drugim.


Dla relaxed (laptop 1):

```
13911497 runs, i=j=0 count: 461674
```

(desktop 1, 0,0226833478):
```
39747043 runs, i=j=0 count: 901596
```


Z ciekawości uruchomiłem też na vmce na starszym procesorze (komp 1).
Na 2 vCPU działa tak, że nigdy nie ma i=j=0, ale na 4 vCPU już się pojawiają. `369025 runs, i=j=0 count 156` (0,000422736)

Fun fact, relax na tym starym procesorze kręci jakieś cuda typu: `645056 runs, i=j=0 count: 587151` czyli jakieś 90% to i=j=0 XD

Fence rozwiązuje wszystkie problemy: `std::atomic_thread_fence(std::memory_order_seq_cst);` lub `asm volatile("" ::: "memory");`


```bash
g++ -O3 benchmark_reordering.cpp -o bm_test -lbenchmark -lpthread
```

```
Laptop 1.
2026-03-15T00:25:35+01:00
Running ./bm_test
Run on (12 X 1111.72 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 512 KiB (x6)
  L3 Unified 16384 KiB (x1)
Load Average: 0.45, 0.52, 0.55
--------------------------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------------------
BM_WithoutBarrier/real_time        351 ns          350 ns      1996351 Reorder_Count=1.08742M
BM_WithBarrier/real_time           377 ns          376 ns      1650937 Reorder_Count=0
```

```
Desktop 2.
2026-03-15T12:45:02+01:00
Running ./bm_test
Run on (24 X 613.954 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x12)
  L1 Instruction 32 KiB (x12)
  L2 Unified 1024 KiB (x12)
  L3 Unified 32768 KiB (x2)
Load Average: 0.50, 0.48, 0.40
--------------------------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------------------
BM_WithoutBarrier/real_time        232 ns          232 ns      2426015 Reorder_Count=2.35045M
BM_WithBarrier/real_time           309 ns          309 ns      2099201 Reorder_Count=0
```

