bits 32 

global cause_division_error
cause_division_error: 
  mov eax, 10
  mov ebx, 0
  div ebx
  ret

