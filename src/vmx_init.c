#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cpufeature.h>
#include <asm/msr.h>
#include <asm/processor.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <asm/processor-flags.h>
#include <asm/special_insns.h>
#include "vmx_init.h"

#define IA32_FEATURE_CONTROL_MSR 0x3A
#define IA32_VMX_BASIC_MSR       0x480
MODULE_LICENSE("GPL");

static bool vmx_enabled_globally;

bool vmx_supported(void)
{
#ifdef CONFIG_X86_64
    if (!boot_cpu_has(X86_FEATURE_VMX)) {
        pr_err("vmx_init: VMX not supported by CPU\n");
        return false;
    }
#else
    pr_err("vmx_init: non-x86_64 not supported in this minimal skeleton\n");
    return false;
#endif

    return true;
}

static int vmx_enable_feature_control(void)
{
    u64 msr;

    rdmsrl(IA32_FEATURE_CONTROL_MSR, msr);

    if (!(msr & 0x1)) {
        pr_err("vmx_init: BIOS did not lock FEATURE_CONTROL MSR, refusing to modify in minimal skeleton\n");
        return -EIO;
    }

    if (!(msr & (1ULL << 2))) {
        pr_err("vmx_init: VMX outside SMX not enabled in FEATURE_CONTROL MSR\n");
        return -EIO;
    }

    return 0;
}

static void *vmxon_region;
static phys_addr_t vmxon_phys;

static int vmx_allocate_vmxon(void)
{
    u64 vmx_basic;
    u32 revision_id;

    rdmsrl(IA32_VMX_BASIC_MSR, vmx_basic);
    revision_id = vmx_basic & 0xFFFFFFFF;

    vmxon_region = (void *)__get_free_page(GFP_KERNEL | __GFP_ZERO);
    if (!vmxon_region) {
        pr_err("vmx_init: failed to allocate VMXON region\n");
        return -ENOMEM;
    }

    vmxon_phys = virt_to_phys(vmxon_region);

    *(u32 *)vmxon_region = revision_id;

    pr_info("vmx_init: VMXON region allocated at virtual %p physical 0x%llx\n",
            vmxon_region, (unsigned long long)vmxon_phys);

    return 0;
}
static int vmx_enter_root(void)
{
    u64 rflags;

    asm volatile (
        "vmxon %1\n\t"
        "pushfq\n\t"
        "pop %0\n\t"
        : "=r"(rflags)
        : "m"(vmxon_phys)
        : "memory"
    );

    if (rflags & (1 << 0)) {
        pr_err("vmx_init: VMXON failed, CF=1\n");
        return -EIO;
    }

    pr_info("vmx_init: VMXON succeeded, CPU is now in VMX root mode\n");
    return 0;
}

int vmx_enable(void)
{
    int ret;

    if (!vmx_supported())
        return -ENODEV;

    ret = vmx_enable_feature_control();
    if (ret)
        return ret;

    __write_cr4(__read_cr4() | X86_CR4_VMXE);

    ret = vmx_allocate_vmxon();
    if (ret)
        return ret;

    ret = vmx_enter_root();
    if (ret)
        return ret;

    vmx_enabled_globally = true;
    return 0;
}

static void vmx_leave_root(void)
{
    asm volatile("vmxoff");
}

void vmx_disable(void)
{
    if (!vmx_enabled_globally)
        return;

    vmx_leave_root();

    __write_cr4(__read_cr4() & ~X86_CR4_VMXE);

    if (vmxon_region)
        free_page((unsigned long)vmxon_region);

    vmx_enabled_globally = false;
}