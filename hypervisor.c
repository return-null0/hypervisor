#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "src/vmx_init.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Renaldo");
MODULE_DESCRIPTION("Minimal VT-x Hypervisor Skeleton");
MODULE_VERSION("0.1");

static int __init hypervisor_init(void)
{
    int ret;

    pr_info("hypervisor: init — starting VMX capability checks\n");

    /* Step 1: CPU capability */
    if (!vmx_supported()) {
        pr_err("hypervisor: CPU does not support VMX\n");
        return -ENODEV;
    }

    ret = vmx_enable();
    if (ret) {
        pr_err("hypervisor: vmx_enable failed (%d)\n", ret);
        return ret;
    }

    pr_info("hypervisor: VMX enabled — hypervisor root mode active\n");
    return 0;
}

static void __exit hypervisor_exit(void)
{
    pr_info("hypervisor: exit — disabling VMX\n");
    vmx_disable();
    pr_info("hypervisor: VMX disabled — module unloaded\n");
}

module_init(hypervisor_init);
module_exit(hypervisor_exit);
