# Minimal Hypervisor

A tiny VT‑x/AMD‑V hypervisor implemented as a Linux kernel module. Provides the minimal components needed to enter VMX root mode, launch a guest, handle VM exits, and use nested paging.

## Structure
    src/
        vmx_init.c      # VMXON + VMCS setup
        vmexit.c        # VM‑exit dispatcher
        ept.c           # EPT/NPT identity‑mapped paging
    hypervisor.c    # Entry + control loop
    Makefile          # Kernel module build
    README.md         # Project overview


## Requirements
- VT‑x or AMD‑V CPU  
- Virtualization enabled in BIOS  
- Debian or similar Linux  
- Kernel headers installed  

## Check Virtualization

```
grep -E 'vmx|svm' /proc/cpuinfo
lsmod | grep kvm
dmesg | grep -i kvm
```

## Build & Load

```
make
sudo insmod hypervisor.ko
dmesg | grep -i vmx
```


## Notes
Minimal, educational, not a full hypervisor. Focuses on VMX init, VMCS setup, EPT, and basic VM‑exit handling.
