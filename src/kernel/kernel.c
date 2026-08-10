#include "arch/x86_32/interrupts.h"
#include "arch/x86_32/serial_port.h"
#include "arch/x86_32/vga.h"
#include "stdio.h"
#include "utils/asm_helper.h"
#include "utils/helpers.h"
#include <stdint.h>

void kernel_main(void) {
  terminal_initialize();
  serial_init();
  // terminal_writestring("Hello, kernel World!\n");
  // serial_write("hello world for serial port");
  // serial_write("22hello world for serial port");
  // serial_write("\n 33hello world for serial port");
  printf("Hello world from stdio");
  printf("Hello world from stdio");
  printf("\nHello world from stdio");
  printf("\nHello world from stdio with numbers %d", 14);
  printf("\nHello world from stdio with string %s", "the string");
  printf("\nHello world from stdio with char %c", 'c');
  printf("\nHello world from stdio with hex %llx", 0xdeadbeeffeebdead);

  printf("\nHello world from stdio with hex %lx", cause_division_error);
  // uint32_t a = 0xFFFFEEEE;
  // uint16_t b = a;
  // uint32_t upperBitsOffset = a & 0xFFFF0000;
  // upperBitsOffset = upperBitsOffset >> 16;
  // printf("\n print b %x", b);
  // printf("\n print ubo %x", upperBitsOffset);

  IDT_LIDT();
  breakpoint();
  cause_division_error();

  // printf("\n Printf after division error ");
  // breakpoint();
}
