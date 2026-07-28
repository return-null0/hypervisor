# Minimal Hypervisor (VMX Skeleton)

A foundational Type-2 hypervisor implemented as a Linux kernel module for x86_64 architectures. Originally designed as a safe-mode skeleton, this project has been upgraded to actively transition the CPU into VMX Root Operation. It serves as a working template for understanding VT-x internals, MSR handling, Extended Page Tables (EPT), and modern Linux kernel control register manipulation.

The project now establishes the foundation for guest-to-host control transfers. The VM exit dispatcher in vmexit.c includes an inline assembly bridge for the vmread instruction, allowing the host to actively extract the primary exit reason from the Virtual Machine Control Structure (VMCS) at encoding 0x4402.

## Features

| Feature Category | Description | 
| -------- | -------- |
| Hardware Validation   | Validates X86_FEATURE_VMX CPU support and verifies the IA32_FEATURE_CONTROL MSR is unlocked by the BIOS.   |
| State Management | Allocates an aligned VMXON memory region, calculates the hardware revision ID via IA32_VMX_BASIC MSR, and actively transitions the CPU state.   |
| Memory Virtualization    | Constructs a four-level EPT hierarchy (PML4, PDPT, PD, PT) in kernel memory, providing a mock identity-mapped sandbox for the first 16 MB of guest-physical space.   | 
| System Introspection | Exposes real-time kernel memory addresses, mapping statuses, and VMX capability reports directly to user space via the debugfs interface.|


## Project Architecture

| File | Purpose| 
| -------- | -------- |
| vmx_init.c   | Handles MSR validation, VMXON region memory allocation, and active transitions into VMX Root Operation.   |
| vmexit.c | Houses the VM-exit dispatcher and vmread wrappers for VMCS introspection.  |
| ept.c | Implements the Extended Page Table logic and guest-to-host identity mapping.   | 
| hv_debugfs.c | Exposes hypervisor state and memory allocations to /sys/kernel/debug/hv/. |
| hv_core.c | Coordinates the lifecycle sequence for the core hypervisor subsystems.| 
| hypervisor.c | Entry Point |


**System Requirements**
An x86_64 processor with VT-x hardware support is required. Virtualization must be explicitly enabled within the system BIOS or UEFI settings. A modern Linux distribution (such as Debian) with matching kernel headers installed is necessary for successful compilation, as the code relies on current kernel APIs for control register manipulation.

# Guide and Controls

**Verifying Hardware Support**
Before building the module, confirm your processor and kernel recognize virtualization capabilities by running `grep -E 'vmx|svm' /proc/cpuinfo`

**Compilation and Module Loading**
Compile the hypervisor using the standard make command in the project directory. Load the resulting kernel module with elevated privileges by executing `sudo insmod hv.ko`. You can track the initialization sequence and confirm successful VMX execution by viewing the kernel ring buffer with the command `dmesg | grep -i vmx`


**Interacting with DebugFS**
Once the module is actively running, you can introspect its state by navigating to `/sys/kernel/debug/hv/`. You must act as the root user to bypass standard kernel security restrictions.

Read `/sys/kernel/debug/hv/vmx_caps` to review the hardware capability report.

Read `/sys/kernel/debug/hv/vmxon_region` to inspect the virtual address, physical address, and revision ID allocated for VMX root mode.

Read `/sys/kernel/debug/hv/ept_state` to verify the initialization of the Extended Page Table, view the PML4 addresses, and confirm the size of the identity-mapped guest space.

**Safe Unloading**
To gracefully exit VMX root operation and free all allocated kernel memory, execute `sudo rmmod hv`. Verify the teardown sequence in your kernel logs to ensure vmxoff was executed correctly.