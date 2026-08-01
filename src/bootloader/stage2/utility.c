#include "stdint.h"

uint32_t align(uint32_t number, uint32_t alignTo) {
  if (alignTo == 0) {
    return number;
  }

  uint32_t rem = number % alignTo;
  return (rem > 0) ? (number + alignTo - rem) : number;
}

uint32_t far_pointer_to_linear_address(void far *farPointer) {
  uint32_t segment = (uint32_t)((uint32_t)farPointer >> (4 * 4));
  uint16_t offset = (uint32_t)farPointer & 0x0000FFFF;

  uint32_t linear_address = segment * 0x10 + offset;

  return linear_address;
}

void breakpoint() { _asm xchg bx, bx }
