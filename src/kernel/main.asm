org 0x100000
bits 32

start: 
  xchg  bx, bx
  mov bl, 'B'
  call Putch32

%define VIDMEM 0xB8000        ; Base address of Color Video Memory
%define Cols 80
%define Lines 25
%define CHAR_ATTRIBUTE 14

_CurrX db 5
_CurrY db 5


;
;  Putch32(): 
;  BL: put character to write here 
;
Putch32: 
  pusha


  mov edi, VIDMEM

  xor eax, eax
  
; Getting the current position in the memory of current x/y coordinates
; since address space is linear we must use the following formula to convert the x/y coordinates of screen to 
; exact byte position in memory
; x+y*screen_width
; but since each column has 2 bytes. One for character and other for attribute. Every cell occupies 2 bytes
; thus to calculate the exact position in memory we must 
; x*2+y*screen_width*2
; or, ( x+y*screen_width ) * 2 
  mov al, byte [ _CurrY ]
  mov ecx, Cols
  mul ecx                     ; y*Cols
  xor ecx, ecx                ; clear ecx
  mov cl, byte [ _CurrX ]     ; insert value of x in cl
  add eax, ecx                ; x + y * Cols
  shl eax, 1                  ; ( y*Cols )2

  add edi, eax      ; Add final result with VIDMEM to get the exact position in memory. 


; watch for a new line
  cmp bl, 0x0A
  jz .Rows

; print a character
  mov dl, bl
  mov dh, CHAR_ATTRIBUTE
  mov [edi], dx
  
; update cursor position
  inc byte  [_CurrX]
  cmp [_CurrX], 0x0A
  jz .Rows
  jmp .done


; GOTO next row
.Rows: 
  mov [_CurrX], 0
  inc [_CurrY]

; Restore registers and return
.done: 
  popa
  ret


