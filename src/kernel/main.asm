org 0x100000
bits 32

start: 
  ; xchg  bx, bx
  ; mov bl, 'B'
  ; call Putch32
  mov ebx, print_msg
  call Puts32
  cli 
  hlt

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


; Puts32():
; ebx: address of start of string
Puts32: 
  ; save registers
  pusha
  ; save string address
  mov edi, ebx

; start loop
.loop:
  ; xchg bx, bx
  ; get current character
  mov bl, [edi]
  ; check if its null character or not
  cmp bl, 0
  jz .done
  ; if not then call putc32 

  call Putch32
  ; update current character pointer or bl register
  
; restart loop
.Next: 
  inc edi
  jmp .loop

.done: 
  mov bl, [_CurrY]
  mov bh, [_CurrX]

  call .MoveCursor

  popa 
  ret

; Move cursor to current x/y position
.MoveCursor: 
  ; calculate the current position in string
  ; as its not current pos in memory we can use the formula
  ; pos = x + y * columns
  pusha

  xchg bx, bx
  xor eax, eax
  mov al, bl
  mov cl, Cols
  mul cl              ; y * columns
  shr bx, 8
  add ax, bx          ; x + y * columns

  ; storing final position in bx as ax is required by I/O
  mov bx, ax          

  ; Now, we use Port Mapped I/O instructions 
  ; 0x3D4 :- Read/Write to Instruction register
  ; 0x3D5 :- Read/Write to Data register
  ; And in the instruction register:
  ; 0x0E :- indicates the upper byte of cursor position
  ; 0x0F :- indicates the lower byte of cursor position
  
  ; I/O write to instruction register of VGA.
  ; Indicates Data reg will have lower byte of cursor position.
  mov dx, 0x3D4
  mov al, 0x0F
  out dx, al 

  ; I/O write to data reg of VGA. 
  ; The lower byte of cursor position.
  mov dx, 0x3D5
  mov al, bl          ;why the hell out no take bl reggggggggggggg
  out dx, al

  ; I/O write to instruction register of VGA.
  ; Indicates Data reg will have upper byte of cursor position.
  mov dx, 0x3D4
  mov ax, 0x0E
  out dx, ax

  ; I/O write to data reg of VGA. 
  ; The upper byte of cursor position.
  mov dx, 0x3D5
  mov al, bh
  out dx, al

  popa 
  ret


   

  


print_msg db 'Hello World From Protected Mode and VGA', 0;
