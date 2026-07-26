#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "vmx_init.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Renaldo");
MODULE_DESCRIPTION("Minimal VT-x/AMD-V Hypervisor (Kernel Module)");
MODULE_VERSION("0.1");

static int __init hypervisor_init(void)
{
    pr_info("hypervisor: init\n");

    if (!vmx_supported()) {
        pr_err("hypervisor: CPU does not support VMX/SVM\n");
        return -ENODEV;
    }

    if (vmx_enable() != 0) {
        pr_err("hypervisor: failed to enable VMX\n");
        return -EIO;
    }

    pr_info("hypervisor: VMX enabled, ready for guest setup\n");
    return 0;
}

static void __exit hypervisor_exit(void)
{
    pr_info("hypervisor: exit\n");
    vmx_disable();
}


module_init(hypervisor_init);
module_exit(hypervisor_exit);
