#include "stdio.h"
#include "utils/helpers.h"
#include <stdbool.h>
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
#define PAGE_SIZE 4096  // 4KB page frame
#define BITMAP_WIDTH 32 // 32-bits per bitmap entry
#define KERNEL_START 1048576
#define UINT32_MAX 0xFFFFFFFFu
MemoryMap *memoryMap = MEMORY_MAP_ADDR;
uint32_t search_start = 0;

uint32_t *bitmap = (uint32_t *)&kernel_end;

void page_to_bitmap(uint32_t page, uint32_t *bitmap_Y, uint32_t *bitmap_X) {
  *bitmap_Y = page / 32;
  *bitmap_X = page % 32;
}

void addr_to_bitmap(uint64_t physical_addr, uint32_t *bitmap_Y,
                    uint32_t *bitmap_X) {
  uint32_t page = physical_addr / PAGE_SIZE;
  page_to_bitmap(page, bitmap_Y, bitmap_X);
}

// need to ceil  the end result so this is used.
uint32_t memLength_to_pgLength(uint64_t memLength) {
  uint32_t result = memLength / PAGE_SIZE;
  if (memLength % PAGE_SIZE != 0) {
    return result + 1;
  }
  return result;
}

void print_memory_map() {
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

void pmm_alloc(uint32_t pages) {
  bitmap[pages] = 1;
  // print_memory_map();
};
void pmm_free(uint32_t frame) {

};

void pmm_alloc_many(uint64_t addr, uint64_t len_in_bytes) {
  uint32_t bitmap_Y;
  uint32_t bitmap_X;
  addr_to_bitmap(addr, &bitmap_Y, &bitmap_X);

  // Remaining pages to allocate
  uint32_t remaining_pages = memLength_to_pgLength(len_in_bytes);

  // usable frames for first 32-bit slot as bitmap_X is dynamic
  uint8_t usable_frames = 32 - bitmap_X;

  // Does remaining pages overflow from the first accessed 32-bit entry.
  bool isOverflow = remaining_pages >= usable_frames;

  if (isOverflow) {
    uint32_t bitmap_operand = UINT32_MAX << bitmap_X;
    bitmap[bitmap_Y] = bitmap[bitmap_Y] & bitmap_operand;
  } else {
    uint32_t bitmap_operand = (UINT32_MAX << bitmap_X) &
                              UINT32_MAX >> (32 - (bitmap_X + remaining_pages));
    bitmap[bitmap_Y] = bitmap[bitmap_Y] & bitmap_operand;
  }

  remaining_pages -= usable_frames;

  uint32_t current_Y = bitmap_Y + 1;
  while (remaining_pages > 0) {
    if (remaining_pages >= 32) {
      bitmap[current_Y] = UINT32_MAX;
      remaining_pages -= 32;
      current_Y++;
    } else {
      uint32_t bitmap_operand = (1U << remaining_pages) - 1;
      bitmap[current_Y] = bitmap[current_Y] & bitmap_operand;
      remaining_pages = 0;
    }
  }
};
void pmm_free_many(uint64_t addr, uint64_t len_in_bytes) {
  uint32_t bitmap_Y;
  uint32_t bitmap_X;
  addr_to_bitmap(addr, &bitmap_Y, &bitmap_X);

  // Remaining pages to allocate
  uint32_t remaining_pages = memLength_to_pgLength(len_in_bytes);

  // usable frames for first slot as bitmap_X is dynamic
  uint8_t usable_frames = 32 - bitmap_X;

  // Does remaining pages overflow from the first accessed 32-bit entry.
  bool isOverflow = remaining_pages >= usable_frames;

  if (isOverflow) {
    uint32_t bitmap_operand = (1U << bitmap_X) - 1;
    bitmap[bitmap_Y] = bitmap[bitmap_Y] & bitmap_operand;
  } else {
    uint32_t bitmap_operand =
        ((1U << bitmap_X) - 1) | (UINT32_MAX << (bitmap_X + remaining_pages));
    bitmap[bitmap_Y] = bitmap[bitmap_Y] & bitmap_operand;
  }

  remaining_pages -= usable_frames;

  uint32_t current_Y = bitmap_Y + 1;
  while (remaining_pages > 0) {
    if (remaining_pages >= 32) {
      bitmap[current_Y] = 0;
      remaining_pages -= 32;
      current_Y++;
    } else {
      uint32_t bitmap_operand = UINT32_MAX << remaining_pages;
      bitmap[current_Y] = bitmap[current_Y] & bitmap_operand;
      remaining_pages = 0;
    }
  }
  // bitmap[bitmap_Y] = ((1U << bitmap_X) - 1) & bitmap[bitmap_Y];
};

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
    //
    bitmap[i] = UINT32_MAX;
  }

  // Unmark the frames based on E820
  for (uint32_t i = 0; i < memoryMap->length; i++) {
    MemoryMapEntry *Entry = &memoryMap->entry[i];

    // keep memory below 1 mb reserved
    if (Entry->Type == USABLE && Entry->BaseAddr >= KERNEL_START) {
      uint64_t usable_page_start = Entry->BaseAddr / PAGE_SIZE;
      uint64_t usable_page_end =
          usable_page_start + memLength_to_pgLength(Entry->Length);

      printf("\n --- base addr: %lld", usable_page_start);
      printf("\n --- length: %lld", usable_page_end);

      for (uint64_t j = usable_page_start; j < usable_page_end; j += 32) {
        // marks 32 pages as free
        bitmap[j] = 0;
      }
    }
  }

  // Mark the pages used by stage2, Kernel, bootloader and BIOS as reserved
  uint64_t allocated_page_start = KERNEL_START / PAGE_SIZE;
  uint32_t allocated_mem_length =
      (uint64_t)&kernel_end + bitmapLength - KERNEL_START;
  uint64_t allocated_page_length = memLength_to_pgLength(allocated_mem_length);
  uint64_t allocated_page_end = allocated_page_start + allocated_page_length;

  printf("used p start and length , %lld , %lld", allocated_page_start,
         allocated_page_length);

  for (uint64_t i = allocated_page_start; i < allocated_page_end; i += 32) {
    // marks 32 pages as used
    bitmap[i] = UINT32_MAX;
  }

  printf("\n Total Memory : %lld", totalMemory);
  printf("\n Kernel_End: %d", &kernel_end);
  printf("\n kend  + bitmap len: %lld",
         0x100000 - ((uint64_t)&kernel_end + bitmapLength));
  printf(
      "\n test + bitmap len: %lld",
      memLength_to_pgLength(((uint64_t)&kernel_end + bitmapLength) - 0x100000));

  return;
}
