#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <asm/page.h>
#include "ept.h"

static struct ept_state g_ept;

static void *alloc_page_zero(void)
{
    void *p = (void *)__get_free_page(GFP_KERNEL | __GFP_ZERO);
    if (!p)
        pr_err("ept: failed to allocate page\n");
    return p;
}

static int ept_build_identity_map(void)
{
    unsigned long guest_bytes = EPT_GUEST_SIZE_MB * 1024UL * 1024UL;
    void *pml4, *pdpt, *pd, *pt;
    phys_addr_t pml4_phys;

    pml4 = alloc_page_zero();
    if (!pml4)
        return -ENOMEM;

    pml4_phys = (phys_addr_t)page_to_pfn(virt_to_page(pml4)) << PAGE_SHIFT;

    pdpt = alloc_page_zero();
    if (!pdpt)
        goto err_pml4;

    pd = alloc_page_zero();
    if (!pd)
        goto err_pdpt;

    pt = alloc_page_zero();
    if (!pt)
        goto err_pd;

    {
        u64 *pml4e = (u64 *)pml4;
        phys_addr_t pdpt_phys =
            (phys_addr_t)page_to_pfn(virt_to_page(pdpt)) << PAGE_SHIFT;

        pml4e[0] = pdpt_phys | (1ULL << 0) | (1ULL << 1);
    }

    {
        u64 *pdpte = (u64 *)pdpt;
        phys_addr_t pd_phys =
            (phys_addr_t)page_to_pfn(virt_to_page(pd)) << PAGE_SHIFT;

        pdpte[0] = pd_phys | (1ULL << 0) | (1ULL << 1);
    }

    {
        u64 *pde = (u64 *)pd;
        phys_addr_t pt_phys =
            (phys_addr_t)page_to_pfn(virt_to_page(pt)) << PAGE_SHIFT;

        pde[0] = pt_phys | (1ULL << 0) | (1ULL << 1);
    }

    {
        u64 *pte = (u64 *)pt;
        unsigned long gpa;

        for (gpa = 0; gpa < guest_bytes; gpa += EPT_PAGE_SIZE) {
            unsigned long idx = gpa / EPT_PAGE_SIZE;
            phys_addr_t hpa = gpa;

            pte[idx] = hpa | (1ULL << 0) | (1ULL << 1);
        }

        g_ept.mapped_bytes = guest_bytes;
    }

    g_ept.initialized = true;
    g_ept.pml4 = pml4;
    g_ept.pml4_phys = pml4_phys;

    pr_info("ept: identity-mapped %lu MB of guest space\n", EPT_GUEST_SIZE_MB);
    pr_info("ept: PML4 virt=%p phys=0x%llx\n",
            g_ept.pml4, (unsigned long long)g_ept.pml4_phys);

    return 0;

err_pd:
    free_page((unsigned long)pd);
err_pdpt:
    free_page((unsigned long)pdpt);
err_pml4:
    free_page((unsigned long)pml4);
    return -ENOMEM;
}

int ept_init(void)
{
    memset(&g_ept, 0, sizeof(g_ept));
    return ept_build_identity_map();
}

void ept_destroy(void)
{
    if (!g_ept.initialized)
        return;

    {
        u64 *pml4 = g_ept.pml4;
        u64 *pdpt, *pd, *pt;

        if (!pml4)
            goto out;

        pdpt = page_address(pfn_to_page((pml4[0] & PAGE_MASK) >> PAGE_SHIFT));
        pd   = page_address(pfn_to_page((pdpt[0] & PAGE_MASK) >> PAGE_SHIFT));
        pt   = page_address(pfn_to_page((pd[0] & PAGE_MASK) >> PAGE_SHIFT));

        free_page((unsigned long)pt);
        free_page((unsigned long)pd);
        free_page((unsigned long)pdpt);
        free_page((unsigned long)pml4);
    }

out:
    memset(&g_ept, 0, sizeof(g_ept));
    pr_info("ept: destroyed EPT hierarchy\n");
}

void ept_get_state(struct ept_state *out)
{
    if (!out)
        return;
    *out = g_ept;
}
