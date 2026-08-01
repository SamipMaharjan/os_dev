#include "fat.h"
#include "stdint.h"

typedef struct {
  uint32_t segmentType;
  uint32_t fileOffset;
  uint32_t dstAddr;
  uint32_t physicalAddr;
  uint32_t fileSize;
  uint32_t memSize;
  uint32_t flags;
  uint32_t alignment;
} ELF_ProgramHeader;

void ELF_Load(uint8_t *elfHeaders, FAT_File *file, DISK *disk,
              void far *dataOut);
