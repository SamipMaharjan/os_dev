#include "interrupts.h"
#include "pic.h"
#include "ps2_8042.h"
#include "serial_port.h"
#include "vga.h"

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

// initialize the necessary things for the hardware
// architecture
void arch_init() {
  serial_init();
  terminal_initialize();
  IDT_LIDT();
  PIC_remap(PIC1_OFFSET, PIC2_OFFSET);
  uint16_t imr = pic_get_imr();
  printf("\nIMR value : %d\r\n", imr);

  // disable timer interrupt for now.
  PIC_set_mask(0);

  PS2_Init();
}
