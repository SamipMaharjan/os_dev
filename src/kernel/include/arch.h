#include <stdint.h>
void arch_init();
void enable_paging(uint32_t pdt_addr);
uint32_t x86_get_cr3();
