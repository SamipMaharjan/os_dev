bits 32 

global cause_division_error
cause_division_error: 
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
