#include "fat.h"
#include "stdint.h"

#pragma pack(push, 1)
typedef struct {
  uint32_t segmentType;
  uint32_t fileOffset;
  uint32_t virtualAddr;
  uint32_t physicalAddr;
  uint32_t fileSize;
  uint32_t memSize;
  uint32_t flags;
  uint32_t alignment;
} ELF_ProgramHeader;
#pragma pack(pop)

void ELF_Load(uint8_t *elfHeaders, FAT_File far *file, DISK *disk,
              void far *dataOut);
