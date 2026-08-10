#include "elf.h"
#include "fat.h"
#include "stdint.h"
#include "stdio.h"
#include "utility.h"

void far *LinearToFar(uint32_t linear) {
  uint16_t segment;
  uint16_t offset;

  if (linear < 0x100000UL) {
    // normal case, well within 1MB
    segment = (uint16_t)(linear >> 4);
    offset = (uint16_t)(linear & 0xF);
  } else {
    // linear >= 1MB: use max segment (0xFFFF) and push the
    // remainder into offset. This only works because A20 is
    // enabled, letting segment*16+offset overflow past 0xFFFFF.
    segment = 0xFFFF;
    offset = (uint16_t)(linear - 0xFFFF0UL);
  }
  void far *final_value = (void far *)(((uint32_t)segment << 16) | offset);

  return final_value;
}

void ELF_Load(uint8_t *elfHeaders, FAT_File far *file, DISK *disk,
              void far *dataOut) {
  // uint32_t program_header_offset = elfHeaders[28];
  // uint16_t program_header_size = elfHeaders[42];
  // uint16_t program_header_entries = elfHeaders[44];
  Elf32_Ehdr *elf32_ehdr = (Elf32_Ehdr *)elfHeaders;
  uint32_t program_header_offset = elf32_ehdr->e_phoff;
  uint16_t program_header_size = elf32_ehdr->e_phentsize;
  uint16_t program_header_entries = elf32_ehdr->e_phnum;

  // uint8_t far *outputPointer = (uint8_t far *)dataOut;

  for (uint8_t current_entry = 0; current_entry < program_header_entries;
       current_entry++) {
    // printf("\r\noutputPointer : %lx", outputPointer);
    uint16_t current_entry_offset =
        program_header_offset + current_entry * program_header_size;
    ELF_ProgramHeader *ProgramHeader =
        (ELF_ProgramHeader *)(elfHeaders + current_entry_offset);

    if (ProgramHeader->segmentType != 1) {
      continue;
    }

    // Get the segment offset from program header
    // file->Position = ProgramHeader->fileOffset;

    // No of bytes to read
    uint32_t fileSize = ProgramHeader->fileSize;
    // Memory allocated for the segment
    uint32_t memSize = ProgramHeader->memSize;
    uint32_t fileOffset = ProgramHeader->fileOffset;
    void far *virtualAddr = LinearToFar(ProgramHeader->virtualAddr);

    printf(
        "\r\nKernel.elf debug size: %lx, pos: %lx, off: %lx, virtualAddr: %lx",
        fileSize, file->Position, fileOffset, virtualAddr);

    // printf("\r\n disk file filesize outputPointer:  %x, %lx, %lx", file,
    // ProgramHeader->fileSize, outputPointer);

    // LOAD segment to dataOut
    uint32_t readBytes =
        FAT_Read(disk, file, fileSize, fileOffset, virtualAddr);

    printf("\r\n Read bytes: %lx ", readBytes);
  }
};
