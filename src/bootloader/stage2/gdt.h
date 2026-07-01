#pragma once

#include "stdint.h"
#pragma pack(push, 1)
typedef struct {

  uint16_t SegmentLimit; // Bit 0-15:: 0-15 bits that defines the segment's size
  uint16_t
      SegmentBase1; // Bit 16-31:: 0-15 bits of base memory address of segment
  uint8_t
      SegmentBase2; // Bit 31-39:: 15-23 bits of base memory address of segment

  uint8_t AccessByte; // Bit 40-47:: This byte determines the access related
                      //    information of segment
                      // Bit 40: Access bit :- Used in
                      //    virtual memory
                      // Bit 41: Read/Write bit:- 1= both
                      //    read/write privledge in data descriptor and
                      //    read/execute segment in code descriptor
                      // Bit 42: exclusion bit idk whhat this does rn
                      // Bit 43: Executable Bit :- 0 = data descriptor, 1 = code
                      //    descriptor
                      // Bit 44: 0 = system descriptor, 1= code/data
                      //    desctiptor
                      // Bit 45: Something for virtual Memory
                      // Bit 46-47: Provledge level of this segment descriptor

  uint8_t Granularity; // Bit 48-47 : 16-19 bits of segment's limit
                       // Bit 52: Reserved for OS
                       // Bit 53: Reserved also
                       // Bit 54: Is it gdt for 16bit system or 32 bit system
                       // Bit 55: Is Granularity true or false. if 1 then
                       //     segment limit is in 4KB pages

  uint8_t SegmentBase3; // Bit 56-63: Final 8 bytes of segment base address
} GDT_Entry;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint16_t Limit;
  uint32_t Base;
} GDT_Pointer;
#pragma pack(pop)

uint8_t GDT_Initialize();
