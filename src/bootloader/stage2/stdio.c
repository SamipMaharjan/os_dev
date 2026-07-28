#include "stdio.h"
#include "stdint.h"
#include "x86.h"

int *printf_number(int *argp, int length, bool sign, int radix);

void putc(char c) { x86_Video_WriteCharTeletype(c, 0); }
void puts(const char *str) {
  while (*str) {
    putc(*str);
    str++;
  }
};

// States
#define PRINTF_STATE_NORMAL 0
#define PRINTF_STATE_LENGTH 1
#define PRINTF_STATE_LENGTH_SHORT 2
#define PRINTF_STATE_LENGTH_LONG 3
#define PRINTF_STATE_SPEC 4

// Lengths
#define PRINTF_LENGTH_DEFAULT 0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT 2
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4

// Since CDECL calling convention is being used. the arguments ot the functions
// are contigious in stack memory, like :
// 0x2000:*fmt || 0x2002:arg2 || ... || X: argX

void _cdecl printf(const char *fmt, ...) {
  int *argp = (int *)&fmt; // putting it as (int*) because stack is
                           // sizeof(int)/2bytes. And so that we can find next
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
        putc(*fmt);
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
        putc((char)*argp);
        argp++;
        break;

      // Printing string by printing individual character.
      case 's':
        puts(*(char **)argp);
        argp++;
        break;

      // Printing string by printing individual character.
      case '%':
        putc('%');
        break;

      // Printing base 10 decimal/integers
      case 'd':
      case 'i':
        radix = 10;
        sign = true;
        argp = printf_number(argp, length, sign, radix);
        break;

      // Printing unsigned integers
      case 'u':
        radix = 10;
        sign = false;
        argp = printf_number(argp, length, sign, radix);
        break;

      case 'X': // unsigned hex with uppercase characters
      case 'x': // unsigned hex with lowercase characters
      case 'p': // Pointer addresses
        radix = 16;
        sign = false;
        argp = printf_number(argp, length, sign, radix);
        break;

      // Print octal
      case 'o':
        radix = 8;
        sign = false;
        argp = printf_number(argp, length, sign, radix);
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

const char g_HexChars[] = "0123456789abcdef";

// Takes length  sign and base of number and returns the inceremented pointer
// depending on the datatype
int *printf_number(int *argp, int length, bool sign, int radix) {
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
    uint32_t rem;
    x86_div64_32(number, radix, &number, &rem);
    buffer[pos++] = g_HexChars[rem];
  } while (number > 0);
  // add sign
  if (sign && number_sign < 0) {
    buffer[pos++] = '-';
  }

  // print number in reverse order
  while (--pos >= 0)
    putc(buffer[pos]);

  return argp;
}

void printRawBytes(const char *buffer, int no_of_bytes) {
  int current_byte = 1;

  while (current_byte <= no_of_bytes) {
    putc(*buffer);
    // printf("%c", *buffer);
    buffer++;
    current_byte++;
  }
}
