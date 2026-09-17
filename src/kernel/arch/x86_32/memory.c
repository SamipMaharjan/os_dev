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

#define MEMORY_MAP_ADDR ((MemoryMap *)0x40500)
MemoryMap *memoryMap = MEMORY_MAP_ADDR;

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
