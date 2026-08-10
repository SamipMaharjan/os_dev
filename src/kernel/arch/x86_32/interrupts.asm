bits 32

extern printf
global division_error
division_error: 
  mov ax, bx; 
  mov bx, ax; 

  xchg bx, bx
  mov bx, ax; 
  mov ax, bx; 

  push msg
  call printf
  
.hang:
  cli
  hlt
  jmp .hang

msg: db "Dvision Error"
