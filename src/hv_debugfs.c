#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include "vmx_init.h"

static struct dentry *hv_root;

static int vmx_caps_show(struct seq_file *m, void *v)
{
    struct vmx_state st;

    vmx_get_state(&st);

    seq_puts(m, "VMX capability report\n");
    seq_printf(m, "  supported          : %s\n", st.supported ? "yes" : "no");
    seq_printf(m, "  feature_control_ok : %s\n", st.feature_control_ok ? "yes" : "no");
    seq_printf(m, "  vmxon_allocated    : %s\n", st.vmxon_allocated ? "yes" : "no");
    seq_printf(m, "  vmx_revision_id    : 0x%x\n", st.vmx_revision_id);

    return 0;
}

static int vmx_caps_open(struct inode *inode, struct file *file)
{
    return single_open(file, vmx_caps_show, NULL);
}

static const struct file_operations vmx_caps_fops = {
    .owner   = THIS_MODULE,
    .open    = vmx_caps_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int vmxon_region_show(struct seq_file *m, void *v)
{
    struct vmx_state st;

    vmx_get_state(&st);

    seq_puts(m, "VMXON region\n");
    seq_printf(m, "  allocated : %s\n", st.vmxon_allocated ? "yes" : "no");
    seq_printf(m, "  virt      : %p\n", st.vmxon_virt);
    seq_printf(m, "  phys      : 0x%llx\n",
               (unsigned long long)st.vmxon_phys);

    return 0;
}

static int vmxon_region_open(struct inode *inode, struct file *file)
{
    return single_open(file, vmxon_region_show, NULL);
}

static const struct file_operations vmxon_region_fops = {
    .owner   = THIS_MODULE,
    .open    = vmxon_region_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

int hv_debugfs_init(void)
{
    hv_root = debugfs_create_dir("hv", NULL);
    if (!hv_root)
        return -ENOMEM;

    debugfs_create_file("vmx_caps", 0444, hv_root, NULL, &vmx_caps_fops);
    debugfs_create_file("vmxon_region", 0444, hv_root, NULL, &vmxon_region_fops);

    pr_info("hv_debugfs: created /sys/kernel/debug/hv/*\n");
    return 0;
}

void hv_debugfs_exit(void)
{
    debugfs_remove_recursive(hv_root);
    hv_root = NULL;
    pr_info("hv_debugfs: removed /sys/kernel/debug/hv\n");
}
