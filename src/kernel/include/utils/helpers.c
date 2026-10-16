#include "helpers.h"
#include <stdint.h>
#define PAGE_SHIFT 12

void halt() { __asm__ volatile("hlt"); }
void breakpoint() {
  __asm__ volatile("xchg %bx, %bx");
  // __asm__ volatile("int3");
  // cause_be();
  return;
}
