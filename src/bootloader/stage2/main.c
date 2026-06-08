#pragma once

#include "disk.h"
#include "stdint.h"
#include "stdio.h"

void _cdecl cstart_(uint16_t bootDrive) {
  DISK *disk;
  static uint8_t sectorBuffer[512];
  const char *far_str = "far string";

  printf("Formatted %% %c %s %ls\r\n", 'a', "string", far_str);
  printf("Formatted %d %i %x %p %o %hd %hi %hhu %hhd\r\n", 1234, -5678, 0xdead,
         0xbeef, 012345, (short)27, (short)-42, (unsigned char)20,
         (signed char)-10);
  printf("Formatted %ld %lx %lld %llx\r\n", -100000000l, 0xdeadbeeful,
         10200300400ll, 0xdeadbeeffeebdaedull);

  // Reads Drive parameters using 13h 08h and stores it in disk.
  DISK_Initialize(disk, disk->id);

  printf("\n\n DISK PARAMETERS of disk %d : %d %d %d ", bootDrive,
         disk->cylinders, disk->heads, disk->sectors);

  DISK_ReadSectors(disk, 0, 1, sectorBuffer);

  printf("\n LBA 0: %x", sectorBuffer);
}
