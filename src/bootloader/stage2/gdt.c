#include "gdt.h"
#include "memdefs.h"
#include "stdint.h"
#include "stdio.h"
#include "utility.h"
#include "x86.h"

uint8_t GDT_Initialize() {
  GDT_Entry far *GDT = (GDT_Entry far *)GDT_BASE;

  // Null Descriptor
  GDT[0].SegmentLimit = 0;
  GDT[0].SegmentBase1 = 0;
  GDT[0].SegmentBase2 = 0;
  GDT[0].AccessByte = 0;
  GDT[0].Granularity = 0;
  GDT[0].SegmentBase3 = 0;

  // Code Descriptor
  GDT[1].SegmentLimit = 0xFFFF;
  GDT[1].SegmentBase1 = 0;
  GDT[1].SegmentBase2 = 0;
  GDT[1].AccessByte = 0b10011010;
  GDT[1].Granularity = 0b11001111;
  GDT[1].SegmentBase3 = 0;

  // Data Descriptor
  GDT[2].SegmentLimit = 0xFFFF;
  GDT[2].SegmentBase1 = 0;
  GDT[2].SegmentBase2 = 0;
  GDT[2].AccessByte = 0b10010010;
  GDT[2].Granularity = 0b11001111;
  GDT[2].SegmentBase3 = 0;

  // #NP Descriptor
  // For cause_np routine
  GDT[3].SegmentLimit = 0xFFFF;
  GDT[3].SegmentBase1 = 0;
  GDT[3].SegmentBase2 = 0;
  GDT[3].AccessByte = 0b00010010;
  GDT[3].Granularity = 0b11001111;
  GDT[3].SegmentBase3 = 0;
  uint8_t gdt_size = sizeof(GDT_Entry) * 4;

  uint8_t far *gdt_end = (uint8_t far *)((uint8_t far *)GDT + gdt_size);

  // The end of GDT will contain the GDT_Pointer table;
  GDT_Pointer far *gdt_pointer = (GDT_Pointer far *)gdt_end;

  gdt_pointer->Limit = gdt_size - 1;
  gdt_pointer->Base = far_pointer_to_linear_address((void far *)GDT);

  uint16_t gdt_pointer_segment = (uint32_t)gdt_pointer >> (4 * 4);
  uint16_t gdt_pointer_offset = (uint32_t)gdt_pointer & 0x0000FFFF;

  x86_Set_GDTR(gdt_pointer_segment, gdt_pointer_offset);

  return 0;
}
