#include <linux/kernel.h>

// Inline assembly to execute VMREAD
static inline unsigned long vmcs_read32(unsigned long field)
{
    unsigned long value;
    asm volatile (
        "vmread %1, %0\n\t"
        : "=r" (value)
        : "r" (field)
        : "cc"
    );
    return value;
}

void vmexit_handle(void)
{
    u32 exit_reason = (u32)vmcs_read32(0x4402);

    pr_info("vmexit: intercepted VM exit, reason = 0x%x\n", exit_reason);

    /* 
     * Future implementation: 
     * switch (exit_reason) {
     *     case EXIT_REASON_CPUID: handle_cpuid(); break;
     *     case EXIT_REASON_EPT_VIOLATION: handle_ept(); break;
     *     default: handle_unknown(); break;
     * }
     */
}