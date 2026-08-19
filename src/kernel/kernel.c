#include "arch/x86_32/interrupts.h"
#include "arch/x86_32/serial_port.h"
#include "arch/x86_32/vga.h"
#include "stdio.h"
#include "utils/helpers.h"
#include <stdint.h>

void kernel_main(void) {
  serial_init();
  terminal_initialize();

  printf("Hello world from stdio with numbers %d \n", 14);
  printf("Hello world from stdio with string %s \n", "the string");
  printf("\nHello world from stdio with char %c", 'c');
  printf("\nHello world from stdio with hex %x", 0xdeadbeef);
  printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  printf("\nHello world");

  IDT_LIDT();

  cause_cp();
}
