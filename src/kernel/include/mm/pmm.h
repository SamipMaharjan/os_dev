#include <stdint.h>

void print_memory_map();
void pmm_init();
uint32_t pmm_alloc();
void pmm_free(uint32_t page_frame);
