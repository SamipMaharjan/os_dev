#define COM1 0x3F8

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void serial_init(void) {
  outb(COM1 + 1, 0x00); // Disable interrupts
  outb(COM1 + 3, 0x80); // Enable DLAB
  outb(COM1 + 0, 0x03); // Baud divisor low byte (38400 baud)
  outb(COM1 + 1, 0x00); // Baud divisor high byte
  outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
  outb(COM1 + 2, 0xC7); // Enable FIFO
  outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

int serial_is_transmit_empty(void) { return inb(COM1 + 5) & 0x20; }

void serial_putchar(char c) {
  while (!serial_is_transmit_empty())
    ;

  outb(COM1, c);
}
void serial_write(const char *str) {
  while (*str) {
    if (*str == '\n')
      serial_putchar('\r');

    serial_putchar(*str++);
  }
}
