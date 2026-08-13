bits 32 

global cause_divide_error
cause_divide_error: 
  mov eax, 10
  mov ebx, 0
  div ebx
  ret

global cause_gpf
cause_gpf: 
  xor ax, ax
  mov ax, 0x99      ; arbitrary invalid selector
  mov ds, ax
  ret

global cause_pf
cause_pf: 


global cause_db
cause_db: 
  xchg bx, bx
  pushfd
  or dword [esp], 0x0100
  popfd
  nop
  nop

global cause_nmi
cause_nmi: 
  int 2
