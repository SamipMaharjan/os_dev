#include "fat.h"
#include "stdint.h"
#include "stdio.h"

void ELF_Load(uint8_t *elfHeaders, FAT_File *file, DISK *disk,
              void far *dataOut) {
  uint32_t program_header_offset = elfHeaders[28];
  uint16_t program_header_size = elfHeaders[42];
  uint16_t program_header_entries = elfHeaders[44];
  uint8_t far *outputPointer = (uint8_t far *)dataOut;

  printf("outputPointer : %lx", outputPointer);

  uint8_t current_entry = 0;

  while (current_entry < program_header_entries) {
    uint16_t current_entry_offset =
        program_header_offset + current_entry * program_header_size;

    // Get the segment offset from program header
    file->Position = elfHeaders[current_entry_offset + 4];

    // No of bytes to read
    uint32_t fileSize = elfHeaders[current_entry_offset + 16];

    // Memory allocated for the segment
    uint32_t memSize = elfHeaders[current_entry_offset + 20];

    printf("\r\n disk file filesize outputPointer: %x, %x, %ld, %lx ", disk,
           file, fileSize, outputPointer);

    // Read segment to dataOut
    FAT_Read(disk, file, fileSize, outputPointer);

    outputPointer = (uint8_t far *)outputPointer + memSize;
    current_entry += 1;
  }
};
