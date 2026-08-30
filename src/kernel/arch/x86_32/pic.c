#include "io_calls.h"
#include "utils/helpers.h"
#include <stdint.h>

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define PIC_EOI 0x20 /* End-of-interrupt command code */

#define PIC_SELECT_IRR 0xa
#define PIC_SELECT_ISR 0xb

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

// #define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
// #define ICW1_SINGLE 0x02    /* Single (cascade) mode */
// #define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
// #define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
// #define ICW1_INIT 0x10      /* Initialization - required! */
//
// #define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
// #define ICW4_AUTO 0x02       /* Auto (normal) EOI */
// #define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
// #define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
// #define ICW4_SFNM 0x10       /* Special fully nested (not) */

// #define CASCADE_IRQ 2

void PIC_sendEOI(uint8_t irq) {
  if (irq >= 8) {
    outb(PIC2_COMMAND, PIC_EOI);
  }
  outb(PIC1_COMMAND, PIC_EOI);
}

void PIC_remap(int offset1, int offset2) {
  // 1st Initialization Command Word ICW1:
  // makes the pic wait for 3 ICWs on its data port
  outb(PIC1_COMMAND, 0x11);
  io_wait();
  outb(PIC2_COMMAND, 0x11);
  io_wait();

  // 2nd ICW: vector offset
  outb(PIC1_DATA, offset1);
  io_wait();
  outb(PIC2_DATA, offset2);
  io_wait();

  // 3rd ICW: master/slave mappings
  outb(PIC1_DATA, 0b00000100); // ICW3 in master PIC is a bitmask of irq lines
                               // where 1 represents a slave pic attachment
  io_wait();

  outb(PIC2_DATA, 0x2); // ICW3 in slave is just a number which tells the
                        // line index of master its attached to
  io_wait();

  // 4th ICW: tell the pic if its 8080 mode or 8086 mode
  outb(PIC1_DATA, 0x01);
  io_wait();
  outb(PIC2_DATA, 0x01);
  io_wait();

  // Unmask the PIC
  outb(PIC1_DATA, 0);
  outb(PIC2_DATA, 0);

  // Enable CPU interrupts
  __asm__ volatile("sti");

  PIC_sendEOI(0);
}
void PIC_disable() {
  outb(PIC1_DATA, 0xff);
  outb(PIC2_DATA, 0xff);
}
void IRQ_set_mask(uint8_t irq_line) {
  uint8_t port;
  uint8_t value = 0;
  if (irq_line < 8)
    port = PIC1_DATA;
  else {
    port = PIC2_DATA;
    irq_line -= 8;
  }
  value = inb(port) | 1 << irq_line;
  outb(port, value);
}

void IRQ_clear_mask(uint8_t irq_line) {
  uint8_t port;
  uint8_t value = 0;
  if (irq_line < 8)
    port = PIC1_DATA;
  else {
    port = PIC2_DATA;
    irq_line -= 8;
  }
  value = inb(port) & ~(1 << irq_line);
  outb(port, value);
}

uint16_t pic_get_imr() { return inb(PIC2_DATA) << 8 | inb(PIC1_DATA); }

uint16_t pic_get_irr() {
  // select the IRR register
  outb(PIC1_COMMAND, PIC_SELECT_IRR);
  outb(PIC2_COMMAND, PIC_SELECT_IRR);

  return inb(PIC2_COMMAND) << 8 | inb(PIC1_COMMAND);
}

uint16_t pic_get_isr() {
  // select the ISR register
  outb(PIC1_COMMAND, PIC_SELECT_ISR);
  outb(PIC2_COMMAND, PIC_SELECT_ISR);

  return inb(PIC2_COMMAND) << 8 | inb(PIC1_COMMAND);
}
