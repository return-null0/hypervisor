#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cpufeature.h>
#include <asm/msr.h>
#include <asm/processor.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <asm/processor-flags.h>
#include <asm/special_insns.h>
#include <asm/tlbflush.h> /* Included for cr4_set_bits and cr4_clear_bits */
#include "vmx_init.h"

#define IA32_FEATURE_CONTROL_MSR 0x3A
#define IA32_VMX_BASIC_MSR       0x480

MODULE_LICENSE("GPL");

static bool vmx_enabled_globally = false;

static void *vmxon_region = NULL;
static phys_addr_t vmxon_phys = 0;

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

    pr_info("vmx_init: CPU reports VMX capability\n");
    return true;
}

static int vmx_enable_feature_control(void)
{
    u64 msr;

    rdmsrl(IA32_FEATURE_CONTROL_MSR, msr);

    if (!(msr & 0x1)) {
        pr_err("vmx_init: FEATURE_CONTROL MSR is not locked by BIOS\n");
        return -EIO;
    }

    if (!(msr & (1ULL << 2))) {
        pr_err("vmx_init: VMX outside SMX is not enabled in FEATURE_CONTROL MSR\n");
        return -EIO;
    }

    pr_info("vmx_init: FEATURE_CONTROL MSR indicates VMX is allowed\n");
    return 0;
}

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

    pr_info("vmx_init: VMXON region allocated\n");
    pr_info("vmx_init:   virtual = %p\n", vmxon_region);
    pr_info("vmx_init:   physical = 0x%llx\n", (unsigned long long)vmxon_phys);
    pr_info("vmx_init:   revision_id = 0x%x\n", revision_id);

    return 0;
}

static struct vmx_state g_vmx_state;

int vmx_enable(void)
{
    int ret;
    u8 vmxon_ret;

    memset(&g_vmx_state, 0, sizeof(g_vmx_state));
    pr_info("vmx_init: starting active VMX enable sequence\n");

    g_vmx_state.supported = vmx_supported();
    if (!g_vmx_state.supported)
        return -ENODEV;

    ret = vmx_enable_feature_control();
    if (ret)
        return ret;
    g_vmx_state.feature_control_ok = true;

    ret = vmx_allocate_vmxon();
    if (ret)
        return ret;

    g_vmx_state.vmxon_allocated = true;
    g_vmx_state.vmxon_virt = vmxon_region;
    g_vmx_state.vmxon_phys = vmxon_phys;
    g_vmx_state.vmx_revision_id = *(u32 *)vmxon_region;

    cr4_set_bits(X86_CR4_VMXE);
    pr_info("vmx_init: CR4.VMXE enabled\n");

    asm volatile (
        "vmxon %[pa]\n\t"
        "setna %[ret]\n\t"
        : [ret] "=rm" (vmxon_ret)
        : [pa] "m" (vmxon_phys)
        : "cc", "memory"
    );

    if (vmxon_ret) {
        pr_err("vmx_init: VMXON instruction failed\n");
        cr4_clear_bits(X86_CR4_VMXE); /* Revert CR4 state safely */
        return -EFAULT;
    }

    pr_info("vmx_init: VMXON executed successfully. CPU is in VMX Root Operation.\n");
    vmx_enabled_globally = true;
    return 0;
}

void vmx_disable(void)
{
    pr_info("vmx_init: disabling VMX\n");

    if (vmx_enabled_globally) {
        asm volatile ("vmxoff" : : : "cc");
        pr_info("vmx_init: VMXOFF executed\n");

        cr4_clear_bits(X86_CR4_VMXE);
        vmx_enabled_globally = false;
    }

    if (vmxon_region) {
        free_page((unsigned long)vmxon_region);
        vmxon_region = NULL;
        vmxon_phys = 0;
        pr_info("vmx_init: VMXON region freed\n");
    }

    memset(&g_vmx_state, 0, sizeof(g_vmx_state));
}

void vmx_get_state(struct vmx_state *out)
{
    if (!out)
        return;
    *out = g_vmx_state;
}