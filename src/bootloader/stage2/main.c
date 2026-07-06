#pragma once

#include "disk.h"
#include "fat.h"
#include "gdt.h"
#include "memdefs.h"
#include "stdint.h"
#include "stdio.h"
#include "x86.h"

void _cdecl cstart_(uint16_t bootDrive) {

  DISK disk;
  if (!DISK_Initialize(&disk, bootDrive)) {
    printf("Disk init error \r\n");
    goto end;
  }

  if (!FAT_Initialize(&disk)) {
    printf("FAT init error\r\n");
    goto end;
  }

  // browse files in root
  FAT_File far *fd = FAT_Open(&disk, "/");

  FAT_DirectoryEntry entry;
  int i = 0;
  printf("\n\rACTUAL DIR ENTRY NAMES: \r\n");
  while (FAT_ReadEntry(&disk, fd, &entry) && i++ < 5) {
    for (int i = 0; i < 11; i++)
      if (entry.Name[i] == ' ') {
        printf(" %x ", entry.Name[i]);
      } else {
        putc(entry.Name[i]);
      }
    printf("\r\n");
  }
  FAT_Close(fd);

  // read test.txt
  char buffer[100];
  uint32_t read;
  // printf("\r\n Buffer address thats passed %x\r\n", buffer);

  fd = FAT_Open(&disk, "mydir/test.txt");

  while ((read = FAT_Read(&disk, fd, sizeof(buffer), buffer))) {
    for (uint32_t i = 0; i < read; i++) {
      if (buffer[i] == '\n')
        putc('\r');
      putc(buffer[i]);
    }
  }

  fd = FAT_Open(&disk, "kernel.bin");

  printf("\r\nKernel bin file handle: %d", fd->Handle);
  printf("\r\nKernel bin buffer address: %lx", KERNEL_32_START);

  uint8_t far *kernel32 = (uint8_t far *)KERNEL_32_START;
  uint16_t currentByte = 0;

  // Reading kernel from disk to 0x00100000;
  while ((read = FAT_Read(&disk, fd, sizeof(buffer), buffer))) {
    for (uint32_t i = 0; i < read; i++) {
      kernel32[currentByte] = buffer[i];
      currentByte++;
      // if (buffer[i] == '\n')
      //   putc('\r');
      // putc(buffer[i]);
      printf(" %x ", buffer[i]);
    }
  }

  FAT_Close(fd);

  GDT_Initialize();

  x86_Enable_A20();

  x86_Enable_Pmode();

// Enabling protected mode by setting bit-0 of cr0 register to 1
// _asm {
// cli
// mov eax, cr0
// or eax, 1
// mov cr0, eax
//
//  xchg bx, bx
//  mov ax, 0x10
//  mov ds, ax
//  mov es, ax
//  mov ss, ax
//  mov esp, 0x90000
// }

// x86_Init_Segment_Regs_For_Pmode();
// _asm xchg bx, bx;
end:
  for (;;)
    ;
}
