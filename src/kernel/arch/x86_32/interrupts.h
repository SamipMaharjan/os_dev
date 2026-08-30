#include "stdio.h"
#include <stdint.h>
void IDT_LIDT();
void divide_error();
void debug_exception();
void double_fault();
void general_protection_fault();
void nmi();
void breakpoint_exception();
void overflow_exception();
void bound_range_exceeded();
void invalid_opcode();
void device_not_available();
void invalid_tss();
void segment_not_present();
void stack_segment_fault();
void floating_point_exception();
void alignment_check();
void machine_check();
void sse_avx_fp_exception();
void virtualization_exception();
void control_protection_exception();
void hypervisor_injection_exception();
void security_exception();
void vmm_communication_exception();
void timer_interrupt();
void keyboard_interrupt();

static inline void print_stack_frame(char *exceptionName, uint32_t eflags,
                                     uint32_t segmentSelector,
                                     uint32_t errorAddr, uint32_t errorCode) {
  printf("\n%s", exceptionName);
  printf("\nEFLAGS: %x", eflags);
  printf("\nSELECTOR: %x", segmentSelector);
  printf("\nERROR ADDRESS: %x", errorAddr);
  if (errorCode != 0)
    printf("\n ERROR CODE: %x", errorCode);
}
