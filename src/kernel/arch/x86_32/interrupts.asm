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


; isr8
global double_fault
extern DoubleFault
; DF_msg: db 0x0D, 0x0A, "*******************DOUBLE FAULT****************", 0
double_fault: 
  ; push DF_msg
  ; call printf
  call DoubleFault
  jmp hang

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

hang:
  cli
  hlt
  jmp hang
