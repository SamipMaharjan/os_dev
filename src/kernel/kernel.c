#include "arch/x86_32/interrupts.h"
#include "arch/x86_32/serial_port.h"
#include "arch/x86_32/vga.h"

void kernel_main(void) {
  terminal_initialize();
  terminal_writestring("Hello, kernel World!\n");

  serial_init();
  serial_write("hello world for serial port");

  IDT_LIDT();
}
