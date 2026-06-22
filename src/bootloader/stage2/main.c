#pragma once

#include "disk.h"
#include "fat.h"
#include "stdint.h"
#include "stdio.h"

void _cdecl cstart_(uint16_t bootDrive) {
  // DISK *disk;
  // static uint8_t sectorBuffer[512];
  // const char *far_str = "far string";
  //
  // printf("Formatted %% %c %s %ls\r\n", 'a', "string", far_str);
  //
  // printf("Formatted %d %i %x %p %o %hd %hi %hhu %hhd\r\n", 1234, -5678,
  // 0xdead,
  //        0xbeef, 012345, (short)27, (short)-42, (unsigned char)20,
  //        (signed char)-10);
  //
  // printf("Formatted %ld %lx %lld %llx\r\n", -100000000l, 0xdeadbeeful,
  //        10200300400ll, 0xdeadbeeffeebdaedull);
  //
  // // Reads Drive parameters using 13h 08h and stores it in disk.
  // DISK_Initialize(disk, bootDrive);
  //
  // printf("\n\n DISK PARAMETERS of disk %d : %d %d %d ", bootDrive,
  //        disk->cylinders, disk->heads, disk->sectors);
  //
  // DISK_ReadSectors(disk, 0, 1, sectorBuffer);
  //
  // FAT_Initialize(disk);
  DISK disk;
  if (!DISK_Initialize(&disk, bootDrive)) {
    printf("Disk init error \r\n");
    goto end;
  }

  // DISK_ReadSectors(&disk, 19, 1, g_Data);

  if (!FAT_Initialize(&disk)) {
    printf("FAT init error\r\n");
    goto end;
  }

  // browse files in root
  FAT_File far *fd = FAT_Open(&disk, "/");
  FAT_DirectoryEntry entry;
  int i = 0;
  while (FAT_ReadEntry(&disk, fd, &entry) && i++ < 5) {
    printf("  ");
    for (int i = 0; i < 11; i++)
      putc(entry.Name[i]);
    printf("\r\n");
  }
  FAT_Close(fd);

  // read test.txt
  char buffer[100];
  uint32_t read;
  fd = FAT_Open(&disk, "/TEST.TXT");
  while ((read = FAT_Read(&disk, fd, sizeof(buffer), buffer))) {
    for (uint32_t i = 0; i < read; i++) {
      if (buffer[i] == '\n')
        putc('\r');
      putc(buffer[i]);
    }
  }
  FAT_Close(fd);

end:
  for (;;)
    ;
}
