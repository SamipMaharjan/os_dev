#include "disk.h"
#include "stdio.h"
#include "x86.h"
// #include <stdint.h>

bool DISK_Initialize(DISK *disk, uint8_t driveNumber) {
  uint8_t driveType;
  uint16_t cylinders, sectors, heads;

  disk->id = driveNumber;

  if (!x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders, &sectors,
                               &heads)) {
    return false;
  }

  disk->id = driveNumber;
  disk->cylinders = cylinders + 1;
  disk->heads = heads + 1;
  disk->sectors = sectors;

  return true;
}
void DISK_LBA2CHS(DISK *disk, uint32_t lba, uint16_t *cylindersOut,
                  uint16_t *headsOut, uint16_t *sectorsOut) {
  // sector = (LBA % sectors per track + 1)
  *sectorsOut = (lba % disk->sectors) + 1;

  // cylinder = (LBA / sectors per track )/ heads
  *cylindersOut = (lba / disk->sectors) / disk->heads;

  // head = (LBA / sectors per track )% heads
  *headsOut = (lba / disk->sectors) % disk->heads;
}

bool DISK_ReadSectors(DISK *disk, uint32_t lba, uint8_t sectors,
                      void far *dataOut) {
  uint16_t cylinder, sector, head;
  DISK_LBA2CHS(disk, lba, &cylinder, &head, &sector);

  for (int i = 0; i < 3; i++) {
    bool ok = x86_Disk_Read(disk->id, cylinder, head, sector, sectors, dataOut);
    if (ok) {
      return true;
    }
    ok = x86_Disk_Reset(disk->id);
  }
  return false;
}
