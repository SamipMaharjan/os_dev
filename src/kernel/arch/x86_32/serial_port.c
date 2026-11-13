#include "printf_states.h"
#include <stdbool.h>
#include <stdint.h>

#define COM1 0x3F8
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

int serial_is_transmit_empty(void) {
  return (inb(COM1 + 5) & 0x60) == 0x60; // THRE (0x20) AND TEMT (0x40)
}

void serial_putc(char c) {
  while (!serial_is_transmit_empty())
    ;

  outb(COM1, c);
}

void serial_puts(const char *str) {
  while (*str) {
    // if (*str == '\n')
    //   serial_putchar('\r');

    serial_putc(*str++);
  }
}

static int *serial_printf_no(int *argp, int length, bool sign, int radix) {
  char buffer[32];
  unsigned long long number;
  int number_sign = 1;
  int pos = 0;

  // The following switch case does:
  // 1. Considers size of arguments and extracts them from stack.
  // 2. Considers size of arguments and updates pointer value to the next
  // argument in stack.
  // 3. Prints the extracted argument / number.
  switch (length) {
  case PRINTF_LENGTH_SHORT_SHORT:
  case PRINTF_LENGTH_SHORT:
  case PRINTF_LENGTH_DEFAULT:
    if (sign) {
      int n = *argp;
      if (n < 0) {
        n = -n;           // converting it to positive
        number_sign = -1; // storing the sign as negative
      }
      number = (unsigned long long)n; // Type casting it to unsigned long long
    } else {
      number = *(unsigned int *)argp;
    }
    argp++;
    break;

  case PRINTF_LENGTH_LONG:
    if (sign) {
      long int n = *(long int *)argp;
      if (n < 0) {
        n = -n;
        number_sign = -1;
      }
      number = (unsigned long long)n;
    } else {
      number = *(unsigned long int *)argp;
    }
    argp += 2;
    break;

  case PRINTF_LENGTH_LONG_LONG:
    if (sign) {
      long long int n = *(long long int *)argp;
      if (n < 0) {
        n = -n;
        number_sign = -1;
      }
      number = (unsigned long long)n;
    } else {
      number = *(unsigned long long *)argp;
    }
    argp += 4;
    break;
  }

  // convert number to ASCII/string
  do {
    // uint32_t rem = number % radix;
    // number = number / radix;
    // x86_div64_32(number, radix, &number, &rem);

    uint32_t rem = number % radix;
    number = number / radix;

    buffer[pos++] = g_HexChars[rem];
  } while (number > 0);
  // add sign
  if (sign && number_sign < 0) {
    buffer[pos++] = '-';
  }

  // print number in reverse order
  while (--pos >= 0)
    serial_putc(buffer[pos]);

  return argp;
}
void serial_print(const char *fmt, ...) {
  int *argp = (int *)&fmt; // putting it as (int*) because stack is
                           // sizeof(int)/2bytes. And so that we can find  next
                           // argument by incrementing the pointer.
  int state = PRINTF_STATE_NORMAL;
  int length = PRINTF_LENGTH_DEFAULT;
  int radix = 10;
  bool sign = false;

  argp++; // incrementing argp so it points to the second argument

  while (*fmt) {
    switch (state) {
    case PRINTF_STATE_NORMAL:
      switch (*fmt) {
      case '%':
        state = PRINTF_STATE_LENGTH;
        break;
      default:
        serial_putc(*fmt);
        break;
      }
      break;

    case PRINTF_STATE_LENGTH:
      switch (*fmt) {
      case 'h':
        length = PRINTF_LENGTH_SHORT;
        state = PRINTF_STATE_LENGTH_SHORT;
        break;
      case 'l':
        length = PRINTF_LENGTH_LONG;
        state = PRINTF_STATE_LENGTH_LONG;
        break;
      default:
        goto PRINTF_STATE_SPEC_;
      }
      break;

    case PRINTF_STATE_LENGTH_SHORT:
      if (*fmt == 'h') {
        length = PRINTF_LENGTH_SHORT_SHORT;
        state = PRINTF_STATE_SPEC;
      } else
        goto PRINTF_STATE_SPEC_;
      break;

    case PRINTF_STATE_LENGTH_LONG:
      if (*fmt == 'l') {
        length = PRINTF_LENGTH_LONG_LONG;
        state = PRINTF_STATE_SPEC;
      } else
        goto PRINTF_STATE_SPEC_;
      break;

    case PRINTF_STATE_SPEC:
    PRINTF_STATE_SPEC_:
      switch (*fmt) {
      // Character printing
      case 'c':
        serial_putc((char)*argp);
        argp++;
        break;

      // Printing string by printing individual character.
      case 's':
        serial_puts(*(char **)argp);
        argp++;
        break;

      // Printing string by printing individual character.
      case '%':
        serial_putc('%');
        break;

      // Printing base 10 decimal/integers
      case 'd':
      case 'i':
        radix = 10;
        sign = true;
        argp = serial_printf_no(argp, length, sign, radix);
        break;

      // Printing unsigned integers
      case 'u':
        radix = 10;
        sign = false;
        argp = serial_printf_no(argp, length, sign, radix);
        break;

      case 'X': // unsigned hex with uppercase characters
      case 'x': // unsigned hex with lowercase characters
      case 'p': // Pointer addresses
        radix = 16;
        sign = false;
        argp = serial_printf_no(argp, length, sign, radix);
        break;

      // Print octal
      case 'o':
        radix = 8;
        sign = false;
        argp = serial_printf_no(argp, length, sign, radix);
        break;

      // Print binary
      case 'b':
        radix = 2;
        sign = false;
        argp = serial_printf_no(argp, length, sign, radix);
        break;

        // ignore invalid spec
      default:
        break;
      }

      // reset state
      state = PRINTF_STATE_NORMAL;
      length = PRINTF_LENGTH_DEFAULT;
      radix = 10;
      sign = false;
      break;
    }

    fmt++;
  }
}
