#include "stdio.h"
#include <stdint.h>

typedef struct {
  uint64_t BaseAddr;
  uint64_t Length;
  uint32_t Type;
  uint32_t Attributes;
} __attribute__((packed)) MemoryMapEntry;

typedef struct {
  uint32_t length;
  MemoryMapEntry entry[10];
} __attribute__((packed)) MemoryMap;

// uint32_t *bitmap;
extern void *kernel_end;

#define USABLE 1
#define RESERVED 2
#define MEMORY_MAP_ADDR ((MemoryMap *)0x40500)
#define PAGE_SIZE 4 * 1024 // 4KB page frame
#define BITMAP_WIDTH 32    // 32-bits per bitmap entry

MemoryMap *memoryMap = MEMORY_MAP_ADDR;

uint32_t *bitmap = (uint32_t *)&kernel_end;

void get_memory_map() {
  printf("\nAccessing memory mapp");

  printf("\nLength: %d", memoryMap->length);
  for (int i = 0; i < 6; i++) {
    MemoryMapEntry Entry = memoryMap->entry[i];
    printf("\n****************************** %dth Entry "
           "****************************",
           i);
    printf("\nbase addr: %lld", Entry.BaseAddr);
    printf("\nlength: %lld", Entry.Length);
    printf("\ntype: %d", Entry.Type);
    printf("\nattribute: %d", Entry.Attributes);
  }
}
void pmm_init() {
  MemoryMapEntry *highestEntry = &memoryMap->entry[0];

  for (uint32_t i = 0; i < memoryMap->length; i++) {
    MemoryMapEntry *Entry = &memoryMap->entry[i];
    if (Entry->BaseAddr > highestEntry->BaseAddr) {
      highestEntry = Entry;
    }
  }
  uint64_t totalMemory = highestEntry->BaseAddr + highestEntry->Length;
  uint32_t totalPages = totalMemory / PAGE_SIZE;
  uint32_t bitmapLength = totalPages / BITMAP_WIDTH;

  // Mark all frames as used
  for (uint32_t i = 0; i < bitmapLength; i++) {
    bitmap[i] = 0xFFFFFFFF;
  }

  // Unmark the frames based on E820
  for (uint32_t i = 0; i < memoryMap->length; i++) {
    MemoryMapEntry *Entry = &memoryMap->entry[i];
    if (Entry->Type == USABLE) {
      bitmap[i] = 0;
    }
  }

  // Mark the pages used by stage2, Kernel

  printf("\n Total Memory : %lld", totalMemory);
  printf("\n Kernel_End: %d", bitmap);
}

void page_to_bitmap(uint32_t page, uint32_t *arrayElement,
                    uint32_t *bitPosition) {
  *arrayElement = page / 32;
  *bitPosition = page % 32;
}

void physical_addr_to_bitmap(uint64_t physical_addr, uint32_t *arrayElement,
                             uint32_t *bitPosition) {
  uint32_t page = physical_addr / PAGE_SIZE;
  page_to_bitmap(page, arrayElement, bitPosition);
}
