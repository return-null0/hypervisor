#ifndef EPT_H
#define EPT_H

#include <linux/types.h>

#define EPT_PAGE_SIZE      4096UL
#define EPT_GUEST_SIZE_MB  16UL

struct ept_state {
    bool initialized;
    void *pml4;
    phys_addr_t pml4_phys;
    unsigned long mapped_bytes;
};

int ept_init(void);
void ept_destroy(void);
void ept_get_state(struct ept_state *out);

#endif
