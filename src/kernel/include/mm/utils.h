#include <stdint.h>
// #define PAGE_SHIFT 1 << 12
// #define PAGE_SIZE 4096

static inline uint32_t pf_to_paddr(uint32_t pf_number) {
  return pf_number << PAGE_SHIFT;
}

static inline uint32_t memLength_to_pgLength(uint64_t memLength) {
  uint32_t result = memLength >> PAGE_SHIFT;
  if (memLength % PAGE_SIZE != 0) {
    return result + 1;
  }
  return result;
}
