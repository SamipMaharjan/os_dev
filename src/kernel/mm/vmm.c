#include "constants.h"
#include "mm/pmm.h"
#include "stdio.h"
#include "string.h"
#include "utils.h"
#include <stdint.h>

// #define PDE_PRESENT (1 << 0)
// #define PDE_WRITABLE (1 << 1)
// #define PDE_USER (1 << 2)
// #define PDE_ACCESSED (1 << 5)
// #define PDE_PAGE_SIZE (1 << 7)
//
// #define PAGE_SIZE 4096
//
// #define PTE_PRESENT (1 << 0)
// #define PTE_WRITABLE (1 << 1)
// #define PTE_USER (1 << 2)
// #define PTE_WRITE_THROUGH (1 << 3)
// #define PTE_CACHE_DISABLE (1 << 4)
// #define PTE_ACCESSED (1 << 5)
// #define PTE_DIRTY (1 << 6)
// #define PTE_PAT (1 << 7)
//
// #define MAX_ENTRIES 1024

uint32_t *kernel_pdt;
typedef uint32_t PageDirectoryEntry;
typedef uint32_t PageTableEntry;

PageDirectoryEntry *page_dir;
PageTableEntry *page_table;

// TODO: consider changed for the higher half styled kernel
void vmm_map_pages(uint32_t v_addr, uint32_t p_addr, uint32_t no_of_pages) {
  v_addr = v_addr & 0xFFFFF000;
  p_addr = p_addr & 0xFFFFF000;
  // uint32_t pt_addr = (uint32_t)page_table &
  uint32_t remaining_pages = no_of_pages;
  uint16_t pdt_entry = v_addr >> 22;
  uint16_t pt_entry = (v_addr >> 12) & 0b1111111111;

  while (remaining_pages && pdt_entry < MAX_ENTRIES) {
    PageTableEntry *page_table;
    if (page_dir[pdt_entry] & PDE_PRESENT) {
      page_table = (PageTableEntry *)(page_dir[pdt_entry] & 0xFFFFF000);
    } else {
      // Allocate physcal memory for Page Table from PMM
      uint32_t kernel_pte_pf = pmm_alloc();
      // j
      page_table = (PageTableEntry *)pf_to_paddr(kernel_pte_pf);
      // j
      memset(page_table, 0, PAGE_SIZE);
      // Raise present and wriatble bits of PDT entry
      page_dir[pdt_entry] = (uint32_t)page_table | PDE_PRESENT | PDE_WRITABLE;
    }

    // mapping page_table
    while (remaining_pages && pt_entry < MAX_ENTRIES) {
      page_table[pt_entry] = p_addr | PTE_PRESENT | PTE_WRITABLE;
      remaining_pages--;
      pt_entry++;
      p_addr += PAGE_SIZE;
    }

    // updating the vaddr for next iter.
    // uint32_t allocated_count = MAX_ENTRIES - (v_addr >> 12);
    // v_addr += (++pdt_entry) << 22;
    pdt_entry++;
    pt_entry = 0;
  }
}
void vmm_init() {
  uint32_t kernel_pdt_pf = pmm_alloc();
  page_dir = (PageDirectoryEntry *)pf_to_paddr(kernel_pdt_pf);
  memset(page_dir, 0, PAGE_SIZE);

  uint64_t kend_addr = pmm_kend_addr();
  printf("kend addr: %d", kend_addr);
  uint32_t no_of_pages = memLength_to_pgLength(kend_addr);
  printf("no of pages: %d", no_of_pages);
  vmm_map_pages(0, 0, no_of_pages);

  // page_dir[pd_index] = page_table_paddr | PDE_PRESENT | PDE_WRITABLE;
  //
  // page_table[pt_index] = page_paddr | PTE_PRESENT | PTE_WRITABLE;
  // uint32_t kernel_PET = pmm_alloc();
}
