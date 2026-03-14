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

  ---


  ```


StoreLoad reordering


https://cppreference.com/w/c/atomic/memory_order.html


Po poprawieniu implementacji korzystając z barier:
```
566325 runs, i=j=0 count 202
```
Czyli jakieś 4 razy na 10 000 wywołań.

Ale dużo skacze po procesorze, więc pewnie dalej usypia wątki i potencjalnie szereguje wątki jeden po drugim.


Dla relaxed:

```
13911497 runs, i=j=0 count: 461674
```