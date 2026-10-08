#include "mm/pmm.h"
#include <stdint.h>

#define PDE_PRESENT (1 << 0)
#define PDE_WRITABLE (1 << 1)
#define PDE_USER (1 << 2)
#define PDE_ACCESSED (1 << 5)
#define PDE_PAGE_SIZE (1 << 7)

#define PTE_PRESENT (1 << 0)
#define PTE_WRITABLE (1 << 1)
#define PTE_USER (1 << 2)
#define PTE_WRITE_THROUGH (1 << 3)
#define PTE_CACHE_DISABLE (1 << 4)
#define PTE_ACCESSED (1 << 5)
#define PTE_DIRTY (1 << 6)
#define PTE_PAT (1 << 7)

uint32_t *kernel_pdt;
typedef uint32_t PageDirectoryEntry;
typedef uint32_t PageTableEntry;

void vmm_init() {
  pmm_init();
  uint32_t kernel_pdt_pf = pmm_alloc();
  uint32_t kernel_pte_pf = pmm_alloc();

  PageDirectoryEntry *page_dir =
      (PageDirectoryEntry *)pf_to_paddr(kernel_pdt_pf);
  PageTableEntry *page_table = (PageDirectoryEntry *)pf_to_paddr(kernel_pte_pf);
  // page_dir[pd_index] = page_table_paddr | PDE_PRESENT | PDE_WRITABLE;
  //
  // page_table[pt_index] = page_paddr | PTE_PRESENT | PTE_WRITABLE;
  // uint32_t kernel_PET = pmm_alloc();
}
