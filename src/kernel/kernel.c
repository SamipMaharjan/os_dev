#include "arch.h"
#include "mm/pmm.h"
#include "stdio.h"
#include "utils/helpers.h"
#include <stdbool.h>
#include <stdint.h>

void kernel_main(void) {
  // have to get the memory mappings from the stage2's buffer address.
  arch_init();
  pmm_init();

  // breakpoint();
  // uint32_t page_frame = pmm_alloc();
  // printf("\n page frame %x", page_frame);
  // breakpoint();
  // pmm_free(page_frame);
  // breakpoint();

  // printf("Hello world from stdio with numbers %d \n", 14);
  // printf("Hello world from stdio with string %s \n", "the string");
  // printf("\nHello world from stdio with char %c", 'c');
  // printf("\nHello world from stdio with hex %x", 0xdeadbeef);
  // printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  // printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  // printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  // printf("\nHello world, %x", 1U << 31);

  while (true) {
    halt();
  }
}
