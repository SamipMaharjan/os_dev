bits 32

extern printf

; isr0
global divide_error
extern DivideError
extern cause_gpf
; msg: db 0x0D, 0x0A,"**********************DIVIDE ERROR****************", 0
divide_error: 
  ; uncomment this and gpf entry to cause double fault
  ; int 0x14

  call DivideError
  jmp hang


; isr1
global debug_exception
extern DebugException
debug_exception: 
;  Use the debug registers later
  ; pushad
  call DebugException
  ; popad
  ; add esp, 4
  jmp hang


; isr2
global nmi
; extern DebugException
extern NonMaskableInterrupt
nmi: 
  call NonMaskableInterrupt
  cli
  lidt [.null_idt] 
  int 3

align 4 
.null_idt:
  dw 0 ; limit
  dd 0 ; base

;  Use the debug registers later

  jmp hang

; isr3
global breakpoint_exception
breakpoint_exception: 
  xchg bx, bx;
  iret

; isr4
global overflow_exception
extern OverflowException
overflow_exception: 
  call OverflowException
  jmp hang

; isr5
global bound_range_exceeded
extern BoundRangeExceeded
bound_range_exceeded: 
  call BoundRangeExceeded
  jmp hang

; isr6
global invalid_opcode
extern InvalidOpcode
invalid_opcode: 
  call InvalidOpcode
  jmp hang

; isr7
global device_not_available
extern DeviceNotAvailable
device_not_available: 
  call DeviceNotAvailable
  jmp hang

; isr8
global double_fault
extern DoubleFault
double_fault: 
  ; push DF_msg
  ; call printf
  call DoubleFault
  jmp hang

; isr10
global invalid_tss
extern InvalidTss
invalid_tss: 
  ; push DF_msg
  ; call printf
  call InvalidTss
  jmp hang

; isr11
global segment_not_present
extern SegmentNotPresent
segment_not_present: 
  call SegmentNotPresent
  jmp hang


; isr12
global stack_segment_fault
extern StackSegmentFault
stack_segment_fault: 
  call StackSegmentFault
  jmp hang

; isr 13
global general_protection_fault
extern GeneralProtectionFault
GP_ErrCode: dd 0
general_protection_fault: 
  ; saving the err code in memory
  ; push eax
  ; mov eax, [ esp + 4]
  ; mov [GP_ErrCode], eax
  ; pop eax
  ;
  ; pushad
  ;
  ; push [GP_ErrCode]
  ; call GPF
  ; pop eax
  ;
  ; popad
  ; pop eax
  ;
  call GeneralProtectionFault
  jmp hang

  ; iret


; isr14
global page_fault
extern PageFault
PF_ErrCode: dd 0
page_fault: 
  ; saving the err code in memory
  pushad
  mov eax, [ esp + 32 ]
  push eax
  call PageFault
  add esp, 4
  popad
  add esp, 4
  ; call PageFault
  jmp hang

; isr16
global floating_point_exception
extern FloatingPointException
floating_point_exception: 
  call FloatingPointException
  jmp hang

; isr17
global alignment_check
extern AlignmentCheck
alignment_check: 
  call AlignmentCheck
  jmp hang

; isr18
global machine_check
extern MachineCheck
machine_check: 
  call MachineCheck
  jmp hang

; isr19
global sse_avx_fp_exception
extern SseAvxFpException
sse_avx_fp_exception: 
  call SseAvxFpException
  jmp hang

; isr20
global virtualization_exception
extern VirtualizationException
virtualization_exception: 
  call VirtualizationException
  jmp hang

; isr21
global control_protection_exception
extern ControlProtectionException
control_protection_exception: 
  call ControlProtectionException
  jmp hang

; isr28
global hypervisor_injection_exception
extern HypervisorInjectionException
hypervisor_injection_exception: 
  call HypervisorInjectionException
  jmp hang
 
; isr30
global vmm_communication_exception
extern VmmCommunicationException
vmm_communication_exception: 
  call VmmCommunicationException
  jmp hang

; isr30
global security_exception
extern SecurityException
security_exception: 
  call SecurityException
  jmp hang


hang:
  cli
  hlt
  jmp hang
