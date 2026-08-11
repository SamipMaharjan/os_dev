#include "interrupts.h"
#include "serial_port.h"
#include "stdio.h"
#include "utils/asm_helper.h"
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

#define CODE_DESC 0x08
#define DATA_DESC 0x10

static void set_idt_entry(Interrupt_Descriptor *curr_idt, uint32_t offset,
                          uint16_t selector, uint8_t type_attributes) {
  uint32_t upperBitsOffset = offset & 0xFFFF0000;
  upperBitsOffset = upperBitsOffset >> 16;

  curr_idt->offset_1 = offset;
  curr_idt->offset_2 = upperBitsOffset;
  curr_idt->selector = selector;
  curr_idt->type_attributes = type_attributes;
}

void IDT_LIDT() {
  idtr.base = (uint32_t)idt;
  idtr.limit = sizeof(Interrupt_Descriptor) * 255;

  // Interrupt_Descriptor *curr_idt = &idt[0];
  // curr_idt->offset_1 = 0x2000;
  // curr_idt->selector = 0x08;
  // curr_idt->type_attributes = 0b10001111;

  printf("Cause divion error address: %x", &division_error);

  set_idt_entry(&idt[0], (uint32_t)&division_error, CODE_DESC, 0b10001111);
  set_idt_entry(&idt[8], (uint32_t)&double_fault, CODE_DESC, 0b10001111);
  set_idt_entry(&idt[13], (uint32_t)&general_protection_fault, CODE_DESC,
                0b10001111);
  __asm__ volatile("lidt (%0)" : : "r"(&idtr));
};

void GPF(int errCode) {
  printf("\n******************General Protection Fault*******************");
  printf("\nError Code: 0x%x", errCode);
}

// static void IDT_Setup(){
//
// }
