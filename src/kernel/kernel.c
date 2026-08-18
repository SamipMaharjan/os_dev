#include "arch/x86_32/interrupts.h"
#include "arch/x86_32/serial_port.h"
#include "arch/x86_32/vga.h"
#include "stdio.h"
#include "utils/helpers.h"
#include <stdint.h>

void kernel_main(void) {
  serial_init();
  terminal_initialize();

  printf("\nHello world from stdio with numbers %d", 14);
  printf("\nHello world from stdio with string %s", "the string");
  printf("\nHello world from stdio with char %c", 'c');
  printf("\nHello world from stdio with hex %x", 0xdeadbeef);
  printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);
  printf("\nhello world");

  IDT_LIDT();

  cause_oe();
}
