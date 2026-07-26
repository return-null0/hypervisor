#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "src/hv_core.h"
#include "src/hv_debugfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Renaldo");
MODULE_DESCRIPTION("Minimal VT-x Hypervisor Skeleton");
MODULE_VERSION("0.3");

static int __init hypervisor_init(void)
{
    int ret;

    pr_info("hypervisor: init — starting hypervisor core\n");

    ret = hv_init();
    if (ret) {
        pr_err("hypervisor: hv_init failed (%d)\n", ret);
        return ret;
    }

    ret = hv_debugfs_init();
    if (ret) {
        pr_err("hypervisor: hv_debugfs_init failed (%d)\n", ret);
        hv_shutdown();
        return ret;
    }

    pr_info("hypervisor: hypervisor core initialized with debugfs\n");
    return 0;
}

static void __exit hypervisor_exit(void)
{
    pr_info("hypervisor: exit — shutting down hypervisor core\n");
    hv_debugfs_exit();
    hv_shutdown();
    pr_info("hypervisor: exit complete\n");
}

module_init(hypervisor_init);
module_exit(hypervisor_exit);
