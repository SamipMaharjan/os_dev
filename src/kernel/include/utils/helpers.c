#include "helpers.h"
void breakpoint() {
  __asm__ volatile("xchg %bx, %bx");
  __asm__ volatile("int3");
  cause_be();
  return;
}
