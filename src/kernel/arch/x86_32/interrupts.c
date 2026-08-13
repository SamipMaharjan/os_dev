#include "interrupts.h"
#include "serial_port.h"
#include "stdio.h"
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
static void print_stack_frame(char *exceptionName, uint32_t eflags,
                              uint32_t segmentSelector, uint32_t errorAddr,
                              uint32_t errorCode) {
  printf("\n%s", exceptionName);
  printf("\nEFLAGS: %x", eflags);
  printf("\nSELECTOR: %x", segmentSelector);
  printf("\nERROR ADDRESS: %x", errorAddr);
  if (errorCode != 0)
    printf("\n%x", errorAddr);
}

void IDT_LIDT() {
  idtr.base = (uint32_t)idt;
  idtr.limit = sizeof(Interrupt_Descriptor) * 255;

  // Interrupt_Descriptor *curr_idt = &idt[0];
  // curr_idt->offset_1 = 0x2000;
  // curr_idt->selector = 0x08;
  // curr_idt->type_attributes = 0b10001111;

  printf("Cause divion error address: %x", &divide_error);

  set_idt_entry(&idt[0], (uint32_t)&divide_error, CODE_DESC, 0b10001111);
  set_idt_entry(&idt[1], (uint32_t)&debug_exception, CODE_DESC, 0b10001111);
  set_idt_entry(&idt[2], (uint32_t)&nmi, CODE_DESC, 0b10001111);
  set_idt_entry(&idt[8], (uint32_t)&double_fault, CODE_DESC, 0b10001111);
  set_idt_entry(&idt[13], (uint32_t)&general_protection_fault, CODE_DESC,
                0b10001111);
  __asm__ volatile("lidt (%0)" : : "r"(&idtr));
};

// isr0
void DivideError(uint32_t errorAddr, uint32_t segmentSelector,
                 uint32_t eflags) {
  print_stack_frame("*******************DIVIDE ERROR********************",
                    eflags, segmentSelector, errorAddr, 0);
}

// isr2
void DebugException(uint32_t errorAddr, uint32_t segmentSelector,
                    uint32_t eflags, int debugReg) {
  print_stack_frame("*******************DEBUG EXCEPTION********************",
                    eflags, segmentSelector, errorAddr, 0);
  // todo: push all debug regs and print
  // printf("\nDebug Register: 0x%x", debugReg);
}

// isr13
void GPF(int errCode) {
  printf("\n******************General Protection Fault*******************");
  printf("\nError Code: 0x%x", errCode);
}

// isr14
void PageFault(int errCode) {
  printf("\n******************PAGE FAULT*******************");
  printf("\nError Code: 0x%x", errCode);
}
