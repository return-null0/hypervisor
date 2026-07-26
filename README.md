# Minimal Hypervisor (Safe VMX Skeleton)

A small educational hypervisor skeleton implemented as a Linux kernel module.  
Designed for learning VT‑x internals, MSR handling, VMXON region setup, and hypervisor architecture — **without entering VMX root mode**, making it safe to run on older hardware.

This project demonstrates how a real hypervisor is structured while avoiding CR4/VMXON instructions that may crash certain machines.

## Features

### ✔ VMX Capability Detection
- Checks CPU support for VMX (`X86_FEATURE_VMX`)
- Reads and validates `IA32_FEATURE_CONTROL` MSR
- Reads `IA32_VMX_BASIC` MSR (revision ID, VMCS size, memory type)

### ✔ VMXON Region Setup (Safe Mode)
- Allocates a properly aligned VMXON region
- Writes the correct VMX revision ID
- Logs virtual + physical addresses
- **Does not execute `vmxon`** (safe mode)

### ✔ Hypervisor Core Layer
- `hv_init()` and `hv_shutdown()` manage VMX state
- Clean separation between hypervisor logic and VMX plumbing

### ✔ DebugFS Interface
Creates `/sys/kernel/debug/hv/` with:
- `vmx_caps` — VMX capability report  
- `vmxon_region` — VMXON region info (virt/phys/revision)

### ✔ Clean Load/Unload
- Safe initialization on `insmod`
- Safe teardown on `rmmod`
- No CR4 writes, no VMXON, no CPU mode changes


## Structure
    src/
        vmx_init.c      # VMXON + VMCS setup
        vmexit.c        # VM‑exit dispatcher
        ept.c           # EPT/NPT identity‑mapped paging
        hv_debugfs.c
        hv_core.c
    hypervisor.c    # Entry + control loop
    Makefile          # Kernel module build
    README.md         # Project overview


## Requirements
- x86_64 CPU with VMX support  
- Virtualization enabled in BIOS  
- Debian or similar Linux  
- Kernel headers installed  

## Check Virtualization

```
grep -E 'vmx|svm' /proc/cpuinfo
lsmod | grep kvm
dmesg | grep -i vmx
```

## Build & Load

```
make
sudo insmod hv.ko
dmesg | grep -i vmx
```

## DebugFS
```
ls /sys/kernel/debug/hv
cat /sys/kernel/debug/hv/vmx_caps
cat /sys/kernel/debug/hv/vmxon_region
```


## Notes
Minimal, educational, not a full hypervisor. This is a safe-mode hypervisor skeleton Focuses on VMX init, VMCS setup, EPT, and basic VM‑exit handling. 
