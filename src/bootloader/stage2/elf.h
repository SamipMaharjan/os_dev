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

#pragma pack(push, 1)
typedef struct {
  unsigned char e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint32_t e_entry;
  uint32_t e_phoff;
  uint32_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
} Elf32_Ehdr;
#pragma pack(pop)

void ELF_Load(uint8_t *elfHeaders, FAT_File far *file, DISK *disk,
              void far *dataOut);
