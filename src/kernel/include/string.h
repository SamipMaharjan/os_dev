#include "stdio.h"
#include <stdint.h>

// void far * memcpy(void far *dst, const void far *src, uint16_t num) {
//
//   uint8_t far *u8Dst = (uint8_t far *)dst;
//   const uint8_t far *u8Src = (const uint8_t far *)src;
//
//   // printf("\r\n  destination far %lx", u8Dst);
//   // printf("\r\n  destination %x", u8Dst);
//   // memory.h
//   // printf("\r\n  srouce far %lx", u8Src);
//   // printf("\r\n  source %x", u8Src);
//   //
//   // printf("\r\n  num::, %x", num);
//   for (uint16_t i = 0; i < num; i++)
//     u8Dst[i] = u8Src[i];
//
//   return dst;
// }

void *memset(void *ptr, int value, uint32_t num) {
  uint32_t *u32Ptr = (uint32_t *)ptr;

  for (uint16_t i = 0; i < num; i++)
    u32Ptr[i] = (uint32_t)value;

  return ptr;
}

// int memcmp(const void far *ptr1, const void far *ptr2, uint16_t num) {
//   const uint8_t far *u8Ptr1 = (const uint8_t far *)ptr1;
//   const uint8_t far *u8Ptr2 = (const uint8_t far *)ptr2;
//
//   for (uint16_t i = 0; i < num; i++)
//     if (u8Ptr1[i] != u8Ptr2[i])
//       return 1;
//
//   return 0;
// }
//
// void memmap(void far *outBuffer) {
//   // outBuffer = (void far *)((uint32_t)outBuffer + 4);
//   x86_Map_Memory(outBuffer);
// }
