#include "interrupts.h"
#include "pic.h"

#define IRQ0_TIMER 0
#define IRQ1_KEYBOARD 1

void KeyboardInterrupt() {
  // print_stack_frame(
  //     "***************************KEYBOARD
  //     INTERRUPT*************************", eflags, segmentSelector,
  //     prevInstructionAddr, 0);

  printf("\nhello from keyboard interrupt");
  PIC_sendEOI(IRQ1_KEYBOARD);
}

void TimerInterrupt() {
  // printf("\ntimer interrupt");
  PIC_sendEOI(IRQ0_TIMER);
}
