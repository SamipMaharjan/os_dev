#include "serial_port.h"
#include "utils/helpers.h"
#include "vga.h"
#include <stdint.h>

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) IDTR;

typedef struct {
  uint16_t offset_1;       // offset bits 0..15
  uint16_t selector;       // a code segment selector in GDT or LDT
  uint8_t zero;            // unused, set to 0
  uint8_t type_attributes; // gate type, dpl, and p fields
  uint16_t offset_2;       // offset bits 16..31
} __attribute__((packed)) Interrupt_Descriptor;

static Interrupt_Descriptor idt[255]; // 255 entries in IDT
static IDTR idtr;

void IDT_LIDT() {
  idtr.base = (uint32_t)idt;
  idtr.limit = sizeof(Interrupt_Descriptor) * 255;

  terminal_writestring("IDT_LIDT");
  serial_write("IDT_LIDT");

  breakpoint();
  __asm__ volatile("lidt (%0)" : : "r"(&idtr));
  breakpoint();
};
