#include "arch/vga.h"
#include "serial_port.h"

void kernel_main(void) {
  terminal_initialize();
  terminal_writestring("Hello, kernel World!\n");
  terminal_writestring("hehe Hello, kernel World!\n");
  terminal_writestring("hehe 2Hello, kernel World!\n");

  serial_init();
  serial_write("hello world for serial port");
}
