#define USABLE 1
#define RESERVED 2
#define MEMORY_MAP_ADDR ((MemoryMap *)0x40500)
#define PAGE_SIZE 4096  // 4KB page frame
#define PAGE_SHIFT 12   // For multiplication or division
#define BITMAP_WIDTH 32 // 32-bits per bitmap entry
#define KERNEL_START 1048576

#define PDE_PRESENT (1 << 0)
#define PDE_WRITABLE (1 << 1)
#define PDE_USER (1 << 2)
#define PDE_ACCESSED (1 << 5)
#define PDE_PAGE_SIZE (1 << 7)

#define PAGE_SIZE 4096

#define PTE_PRESENT (1 << 0)
#define PTE_WRITABLE (1 << 1)
#define PTE_USER (1 << 2)
#define PTE_WRITE_THROUGH (1 << 3)
#define PTE_CACHE_DISABLE (1 << 4)
#define PTE_ACCESSED (1 << 5)
#define PTE_DIRTY (1 << 6)
#define PTE_PAT (1 << 7)

#define MAX_ENTRIES 1024
#define ONE_GIB 0x40000000
#define HIGHER_HALF 0xC0000000
