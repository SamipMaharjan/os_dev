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
#define EXCEPTION_TYPE_ATTRIBUTE 0b10001111
#define INTERRUPT_TYPE_ATTRIBUTE 0b10001110
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
    printf("\n ERROR CODE: %x", errorCode);
}

void IDT_LIDT() {
  idtr.base = (uint32_t)idt;
  idtr.limit = sizeof(Interrupt_Descriptor) * 255;

  // Interrupt_Descriptor *curr_idt = &idt[0];
  // curr_idt->offset_1 = 0x2000;
  // curr_idt->selector = 0x08;
  // curr_idt->type_attributes = 0b10001111;

  set_idt_entry(&idt[0], (uint32_t)&divide_error, CODE_DESC,
                EXCEPTION_TYPE_ATTRIBUTE);
  set_idt_entry(&idt[1], (uint32_t)&debug_exception, CODE_DESC,
                EXCEPTION_TYPE_ATTRIBUTE);
  set_idt_entry(&idt[2], (uint32_t)&nmi, CODE_DESC, INTERRUPT_TYPE_ATTRIBUTE);
  set_idt_entry(&idt[3], (uint32_t)&breakpoint_exception, CODE_DESC,
                EXCEPTION_TYPE_ATTRIBUTE);
  set_idt_entry(&idt[4], (uint32_t)&overflow_exception, CODE_DESC,
                EXCEPTION_TYPE_ATTRIBUTE);
  set_idt_entry(&idt[5], (uint32_t)&bound_range_exceeded, CODE_DESC,
                EXCEPTION_TYPE_ATTRIBUTE);
  set_idt_entry(&idt[6], (uint32_t)&invalid_opcode, CODE_DESC,
                EXCEPTION_TYPE_ATTRIBUTE);
  set_idt_entry(&idt[7], (uint32_t)&device_not_available, CODE_DESC,
                EXCEPTION_TYPE_ATTRIBUTE);
  set_idt_entry(&idt[8], (uint32_t)&double_fault, CODE_DESC,
                EXCEPTION_TYPE_ATTRIBUTE);
  set_idt_entry(&idt[13], (uint32_t)&general_protection_fault, CODE_DESC,
                EXCEPTION_TYPE_ATTRIBUTE);

  // USE LIDT instruction
  __asm__ volatile("lidt (%0)" : : "r"(&idtr));
};

// isr0
void DivideError(uint32_t errorAddr, uint32_t segmentSelector,
                 uint32_t eflags) {
  print_stack_frame("*******************DIVIDE ERROR********************",
                    eflags, segmentSelector, errorAddr, 0);
}

// isr1
void DebugException(uint32_t errorAddr, uint32_t segmentSelector,
                    uint32_t eflags, int debugReg) {
  print_stack_frame("*******************DEBUG EXCEPTION********************",
                    eflags, segmentSelector, errorAddr, 0);
  // todo: push all debug regs and print
  // printf("\nDebug Register: 0x%x", debugReg);
}

// isr2
void NonMaskableInterrupt(uint32_t errorAddr, uint32_t segmentSelector,
                          uint32_t eflags) {
  print_stack_frame(
      "*******************Non Maskable Interrupt********************", eflags,
      segmentSelector, errorAddr, 0);

  // break to view the err addr before crashing
  breakpoint();
}

// isr4
void OverflowException(uint32_t errorAddr, uint32_t segmentSelector,
                       uint32_t eflags) {
  // printf("\n\rtest here too");
  print_stack_frame("*******************OVERFLOW EXCEPTION********************",
                    eflags, segmentSelector, errorAddr, 0);
}

// isr5
void BoundRangeExceeded(uint32_t errorAddr, uint32_t segmentSelector,
                        uint32_t eflags) {
  print_stack_frame(
      "*******************BOUND RANGE EXCCEEDED********************", eflags,
      segmentSelector, errorAddr, 0);
}

// isr6
void InvalidOpcode(uint32_t errorAddr, uint32_t segmentSelector,
                   uint32_t eflags) {
  print_stack_frame("*******************INVALID OPCODE********************",
                    eflags, segmentSelector, errorAddr, 0);
}

// isr7
void DeviceNotAvailable(uint32_t errorAddr, uint32_t segmentSelector,
                        uint32_t eflags) {
  print_stack_frame(
      "*******************DEVICE NOT AVAILABLE******************** \n Trying "
      "to exceute FPU instruction when CR0.TS = 1",
      eflags, segmentSelector, errorAddr, 0);
}

// isr8
void DoubleFault(uint32_t errorAddr, uint32_t segmentSelector,
                 uint32_t eflags) {
  print_stack_frame("*******************DOUBLE FAULT********************",
                    eflags, segmentSelector, errorAddr, 0);
}

// isr13
void GeneralProtectionFault(int errCode, uint32_t errorAddr,
                            uint32_t segmentSelector, uint32_t eflags) {
  print_stack_frame(
      "*******************GENERAL PROTECTION FAULT********************", eflags,
      segmentSelector, errorAddr, errCode);
}

// isr14
void PageFault(int errCode) {
  printf("\n******************PAGE FAULT*******************");
  printf("\nError Code: 0x%x", errCode);
}
