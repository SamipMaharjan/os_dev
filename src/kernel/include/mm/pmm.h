#include <stdint.h>
#define PAGE_SHIFT 12
void print_memory_map();
void pmm_init();
uint32_t pmm_alloc();
void pmm_free(uint32_t page_frame);
uint64_t pmm_final_used_pf();
uint64_t pmm_final_usable_pf();
// uint64_t pmm_final_usable_memory();
// static inline uint32_t pf_to_paddr(uint32_t pf_number) {
//   return pf_number << PAGE_SHIFT;
// }
