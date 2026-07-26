#include <linux/kernel.h>
#include "hv_core.h"
#include "vmx_init.h"
#include "ept.h"

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

    ret = ept_init();
    if (ret) {
        pr_err("hv_core: ept_init failed (%d)\n", ret);
        vmx_disable();
        return ret;
    }

    hv_initialized = true;
    pr_info("hv_core: hypervisor core initialized (safe VMX + mock EPT)\n");
    return 0;
}

void hv_shutdown(void)
{
    if (!hv_initialized) {
        pr_info("hv_core: shutdown requested but core not initialized\n");
        return;
    }

    pr_info("hv_core: shutting down hypervisor core\n");
    ept_destroy();
    vmx_disable();
    hv_initialized = false;
    pr_info("hv_core: hypervisor core shut down\n");
}