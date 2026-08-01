#pragma once

#include "disk.h"
#include "elf.h"
#include "fat.h"
#include "gdt.h"
#include "memdefs.h"
#include "stdint.h"
#include "stdio.h"
#include "utility.h"
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
  char buffer[512];
  uint32_t read;

  fd = FAT_Open(&disk, "mydir/test.txt");

  while ((read = FAT_Read(&disk, fd, sizeof(buffer), buffer))) {
    for (uint32_t i = 0; i < read; i++) {
      if (buffer[i] == '\n')
        putc('\r');
      putc(buffer[i]);
    }
  }

  FAT_File far *elf_file;
  elf_file = FAT_Open(&disk, "kernel.elf");

  // TODO: This static size for storing elf header will be a problem when size
  // of elf headers exceeds 512. Its better to calculate the total size of ELF
  // and Program Headers dynamically using the values from ELF headers
  uint8_t elfHeaders[512];
  FAT_Read(&disk, elf_file, sizeof(elfHeaders), elfHeaders);

  // Reading ELF file headers from disk to 0x00100000
  ELF_Load(elfHeaders, elf_file, &disk, MEMORY_PARSED_KERNEL);
  breakpoint();

  FAT_Close(fd);

  GDT_Initialize();

  x86_Enable_A20();

  x86_Enable_Pmode();

// x86_Init_Segment_Regs_For_Pmode();
end:
  for (;;)
    ;
}
