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

uint32_t search_start = 0;
uint32_t bitmapLength;

MemoryMap *memoryMap = MEMORY_MAP_ADDR;
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

uint32_t pmm_alloc() {
  uint32_t current_entry = search_start;

  for (uint32_t i = 0; i < bitmapLength; i++) {
    if (current_entry >= bitmapLength) {
      current_entry = 0;
    }
    if (bitmap[current_entry] != UINT32_MAX) {
      uint8_t free_bit_position = __builtin_ctz(~bitmap[current_entry]);
      uint32_t alloc_mask = 1U << free_bit_position;
      bitmap[current_entry] |= alloc_mask;

      uint32_t page_frame_no = current_entry * 32 + free_bit_position;
      search_start = current_entry;

      return page_frame_no;
    }
    current_entry++;
  }
  return 0;
};
uint8_t pmm_free(uint32_t page_frame) {
  if (page_frame >= bitmapLength * 32)
    return 1;

  uint32_t bitmap_X;
  uint32_t bitmap_Y;

  page_to_bitmap(page_frame, &bitmap_Y, &bitmap_X);

  uint32_t free_mask = ~(1U << bitmap_X);
  bitmap[bitmap_Y] &= free_mask;
  search_start = bitmap_Y;

  return 0;

  // uint32_t current_entry = search_start;
  // uint32_t page_frame_no = 0;
  //
  // for (uint32_t i = 0; i < bitmapLength; i++) {
  //   if (current_entry >= bitmapLength) {
  //     current_entry = 0;
  //   }
  //   if (bitmap[current_entry] != UINT32_MAX) {
  //     uint8_t free_pf_position = __builtin_ctz(~bitmap[current_entry]);
  //     page_frame_no = current_entry * 32 + free_pf_position;
  //   }
  //   current_entry++;
  // }
  //
  // search_start = current_entry;
  //
  // return page_frame_no;
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
    bitmap[bitmap_Y] = bitmap[bitmap_Y] | bitmap_operand;
    remaining_pages -= usable_frames;
  } else {
    uint32_t bitmap_operand = (UINT32_MAX << bitmap_X) &
                              UINT32_MAX >> (32 - (bitmap_X + remaining_pages));
    bitmap[bitmap_Y] = bitmap[bitmap_Y] | bitmap_operand;
    remaining_pages = 0;
  }

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
  // if addr not page aligned make it
  if (addr % PAGE_SIZE != 0) {
    addr = addr + (addr % PAGE_SIZE);
  }
  uint32_t bitmap_Y;
  uint32_t bitmap_X;
  addr_to_bitmap(addr, &bitmap_Y, &bitmap_X);

  // Remaining pages to allocate
  // Not using memLength_to_pgLength as
  // the value needs to be floored when length is not
  // page aligned
  uint32_t remaining_pages = len_in_bytes / PAGE_SIZE;

  // usable frames for first slot as bitmap_X is dynamic
  uint8_t usable_frames = 32 - bitmap_X;

  // Does remaining pages overflow from the first accessed 32-bit entry.
  bool isOverflow = remaining_pages >= usable_frames;

  if (isOverflow) {
    uint32_t bitmap_operand = (1U << bitmap_X) - 1;

    printf("\n sinside isoverflow %d %d", bitmap_X, bitmap_operand);

    printf("\n sinside bitmap_Y: %x y: %d", bitmap[bitmap_Y], bitmap_Y);
    bitmap[bitmap_Y] = bitmap[bitmap_Y] & bitmap_operand;
    printf("\n sinside bitmap_Y %x", bitmap[bitmap_Y]);
    remaining_pages -= usable_frames;

  } else {
    uint32_t bitmap_operand =
        ((1U << bitmap_X) - 1) | (UINT32_MAX << (bitmap_X + remaining_pages));
    bitmap[bitmap_Y] = bitmap[bitmap_Y] & bitmap_operand;
    remaining_pages = 0;
  }

  uint32_t current_Y = bitmap_Y + 1;
  while (remaining_pages > 0) {

    printf("\n sinside loop");
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
  printf("\n sinside pmm_free_many");
};

void pmm_init() {
  MemoryMapEntry *highestUsableEntry = &memoryMap->entry[0];

  for (uint32_t i = 0; i < memoryMap->length; i++) {
    MemoryMapEntry *Entry = &memoryMap->entry[i];
    if (Entry->BaseAddr > highestUsableEntry->BaseAddr && Entry->Type == 1) {
      highestUsableEntry = Entry;
    }
  }

  uint64_t totalMemory =
      highestUsableEntry->BaseAddr + highestUsableEntry->Length;
  uint32_t totalPages = totalMemory / PAGE_SIZE;

  // todo: handle the case when remainder remains.
  bitmapLength = totalPages / BITMAP_WIDTH;

  uint32_t bitmap_size = bitmapLength * 4; // each entry is 4 bytes / 32 bits

  // Mark all frames as used
  for (uint32_t i = 0; i < bitmapLength; i++) {
    bitmap[i] = UINT32_MAX;
  }

  // Free the frames above 1MB based on E820
  for (uint32_t i = 0; i < memoryMap->length; i++) {
    MemoryMapEntry *Entry = &memoryMap->entry[i];
    printf("\n sinside unmarking baseaddr: %lld", Entry->BaseAddr);
    if (Entry->Type == USABLE && Entry->BaseAddr >= KERNEL_START) {
      pmm_free_many(Entry->BaseAddr, Entry->Length);
    }
  }
  breakpoint();

  // Mark the pages used by Kernel as reserved
  uint32_t kernel_to_bitmap_length =
      (uint64_t)&kernel_end + bitmap_size - KERNEL_START;

  pmm_alloc_many(KERNEL_START, kernel_to_bitmap_length);

  breakpoint();

  printf("\n bitmapLength %d", bitmapLength);
  printf("\n kend  + bitmap len: %lld",
         0x100000 - ((uint64_t)&kernel_end + bitmap_size));
  printf(
      "\n test + bitmap len: %lld",
      memLength_to_pgLength(((uint64_t)&kernel_end + bitmap_size) - 0x100000));

  return;
}
