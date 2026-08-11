bits 32

extern printf
extern GPF

global division_error
msg: db 0x0D, 0x0A,"**********************DVISION ERROR****************", 0
division_error: 
  push msg
  call printf
  jmp hang

global double_fault
DF_msg: db 0x0D, 0x0A, "*******************DOUBLE FAULT****************", 0
double_fault: 
  push DF_msg
  call printf
  jmp hang

global general_protection_fault
; GPF_msg: db 0x0D, 0x0A, "*******************GENERAL PROTECTION FAULT****************", 0
; GPF_msg2: db 0x0D, 0x0A,"*******************ERR_CODE: %d****************", 0
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

hang:
  cli
  hlt
  jmp hang
