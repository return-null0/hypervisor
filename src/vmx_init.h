#ifndef VMX_INIT_H
#define VMX_INIT_H

struct vmx_state {
    bool supported;
    bool feature_control_ok;
    bool vmxon_allocated;
    void *vmxon_virt;
    phys_addr_t vmxon_phys;
    u32 vmx_revision_id;
};

bool vmx_supported(void);
int vmx_enable(void);
void vmx_disable(void);
void vmx_get_state(struct vmx_state *out);

#endif
