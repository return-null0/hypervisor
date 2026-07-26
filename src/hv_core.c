#include <linux/kernel.h>
#include "hv_core.h"
#include "vmx_init.h"

static bool hv_initialized;

int hv_init(void)
{
    int ret;

    pr_info("hv_core: initializing hypervisor core\n");

    if (hv_initialized) {
        pr_info("hv_core: already initialized\n");
        return 0;
    }

    ret = vmx_enable();
    if (ret) {
        pr_err("hv_core: vmx_enable failed (%d)\n", ret);
        return ret;
    }

    hv_initialized = true;
    pr_info("hv_core: hypervisor core initialized (safe VMX skeleton)\n");
    return 0;
}

void hv_shutdown(void)
{
    if (!hv_initialized) {
        pr_info("hv_core: shutdown requested but core not initialized\n");
        return;
    }

    pr_info("hv_core: shutting down hypervisor core\n");
    vmx_disable();
    hv_initialized = false;
    pr_info("hv_core: hypervisor core shut down\n");
}
