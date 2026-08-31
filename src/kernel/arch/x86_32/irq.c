#include "interrupts.h"
#include "io_calls.h"
#include "pic.h"
#include <stdint.h>

#define IRQ0_TIMER 0
#define IRQ1_KEYBOARD 1

#define PS2_DATA_PORT 0x60

void KeyboardInterrupt() {
  // print_stack_frame(
  //     "***************************KEYBOARD
  //     INTERRUPT*************************", eflags, segmentSelector,
  //     prevInstructionAddr, 0);

  char key = inb(PS2_DATA_PORT);
  // printf("\n keyboard interrupt: %c", key);
  printf("\n keyboard interrupt: %x", (uint8_t)key);
  PIC_sendEOI(IRQ1_KEYBOARD);
}

void TimerInterrupt() {
  printf("\ntimer interrupt");
  PIC_sendEOI(IRQ0_TIMER);
}
