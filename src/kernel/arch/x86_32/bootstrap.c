// void vmm_init() {
// uint32_t kernel_pdt_pf = pmm_alloc();
// page_dir = (PageDirectoryEntry *)pf_to_paddr(kernel_pdt_pf);
// memset(page_dir, 0, PAGE_SIZE);
//
// uint64_t final_usable_pf = pmm_final_usable_pf();
// uint32_t no_of_pages = final_usable_pf;

// printf("\nfinal usable pf %d ", no_of_pages);
// identity mapping entire 128MiB region
// vmm_map_pages(0, 0, no_of_pages);
// vmm_print_pte();
// enable_paging((uint32_t)page_dir);
// printf("\nPage Dir addr %x", page_dir);
// printf("\nPage Table addr %x", page_dir[0] & 0xFFFFF000);
// PageTableEntry *pte = (PageTableEntry *)(page_dir[0] & 0xFFFFF000);
// printf("\nPage Table entry %x", pte[0]);

// map pde 768-799 to 0-31
// direct_map_C0000000();

// printf("\n print something addr : %x", &transition_higherhalf);
// printf("\n direct_map_C0000000 addr : %x", &direct_map_C0000000);
// printf("\n direct_map_C0000000 addr : %x", &direct_map_C0000000);
// printf("\n print something addr : %x", &transition_higherhalf);
// vmm_print_pte();
// vmm_print_pdt();

// breakpoint();
// // higher-half mapping
// no_of_pages = memLength_to_pgLength(ONE_GIB);
// vmm_map_pages(HIGHER_HALF, 0, no_of_pages);
// vmm_print_pdt();
// breakpoint();

// page_dir[pd_index] = page_table_paddr | PDE_PRESENT | PDE_WRITABLE;
//
// page_table[pt_index] = page_paddr | PTE_PRESENT | PTE_WRITABLE;
// uint32_t kernel_PET = pmm_alloc();
// }
