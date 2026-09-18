#include "arch.h"
#include "arch/x86_32/pmm.h"
#include "stdio.h"
#include "utils/helpers.h"
#include <stdbool.h>
#include <stdint.h>

void kernel_main(void) {
  // have to get the memory mappings from the stage2's buffer address.
  arch_init();
  get_memory_map();
  pmm_init();

  // printf("Hello world from stdio with numbers %d \n", 14);
  // printf("Hello world from stdio with string %s \n", "the string");
  // printf("\nHello world from stdio with char %c", 'c');
  // printf("\nHello world from stdio with hex %x", 0xdeadbeef);
  // printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  // printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  // printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  // printf("\nHello world");

  while (true) {
    halt();
  }
}
