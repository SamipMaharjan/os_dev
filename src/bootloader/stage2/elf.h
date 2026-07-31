//
#include "fat.h"
#include "stdint.h"
void ELF_Load(uint8_t *elfHeaders, FAT_File *file, DISK *disk,
              void far *dataOut);
