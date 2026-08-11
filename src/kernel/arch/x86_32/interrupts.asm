bits 32

extern printf

; isr0
global divide_error
msg: db 0x0D, 0x0A,"**********************DVISION ERROR****************", 0
divide_error: 
  push msg
  call printf
  jmp hang


; isr1
global debug_exception
extern DebugException
debug_exception: 
;  Use the debug registers later
  pushad
  call DebugException
  popad
  add esp, 4
  jmp hang


; isr2
global nmi
; extern DebugException
nmi: 
  cli
  lidt [.null_idt] 
  int 3

align 4 
.null_idt:
  dw 0 ; limit
  dd 0 ; base

;  Use the debug registers later

  jmp hang

; isr8
global double_fault
DF_msg: db 0x0D, 0x0A, "*******************DOUBLE FAULT****************", 0
double_fault: 
  push DF_msg
  call printf
  jmp hang

global general_protection_fault
extern GPF
GP_ErrCode: dd 0
general_protection_fault: 
  ; saving the err code in memory
  push eax
  mov eax, [ esp + 4]
  mov [GP_ErrCode], eax
  pop eax

  pushad

  push [GP_ErrCode]
  call GPF
  pop eax

  popad
  pop eax

  xchg bx, bx
  jmp hang

  ; iret


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
  jmp hang

hang:
  cli
  hlt
  jmp hang
