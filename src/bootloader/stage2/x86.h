#pragma once
#include "stdint.h"

void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor,
                         uint64_t *quotientOut, uint32_t *remainderOut);

void _cdecl x86_Video_WriteCharTeletype(char c, uint8_t page);

bool _cdecl x86_Disk_Reset(uint8_t drive);

bool _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t head,
                          uint16_t sector, uint8_t count, void far *dataOut);

bool _cdecl x86_Disk_GetDriveParams(uint8_t drive, uint8_t *driveTypeOut,
                                    uint16_t *cylindersOut,
                                    uint16_t *sectorsOut, uint16_t *headsOut);

// Sets up the value in GDTR register using LGDT instruction
void _cdecl x86_Set_GDTR(uint16_t gdt_pointer_segment,
                         uint16_t gdt_pointer_offset);

// Appearently A20 line is always on for Emulators like Bochs and QEMU.
// But just in case this thing runs on real hardware one day ;)
void _cdecl x86_Enable_A20();

// Enables protected mode by setting bit-0 of cr0 register.
void _cdecl x86_Enable_Pmode(uint32_t entryAddr);

// //
// void _cdecl x86_Init_Segment_Regs_For_Pmode();

//
