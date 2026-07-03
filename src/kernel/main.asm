org 0x0000
bits 16

%define ENDL 0x0D, 0x0A

start: 
    ;print message 
    mov si, msg_hello
    call puts


.halt: 
    cli
    hlt

; Prints a string to the screen
; Params:
;   - ds:si points to the string 
;   - Will have the string in the above segment:offset combination. and print it until it encounters a null character.
;

puts: 
  ; save registers we will modify. As a function must not permanently destroy values in register that it will use. 
  push si 
  push ax

.loop:
  lodsb ; loads next character in al
  
  ; OR destination, source performs an or operation in source and destination and stores the result in destination
  ; if operating OR on itself then it modifies the zero flag register if the result is zero/false
  or al, al ; verify if the next char is null or not
  jz .done

  mov ah, 0x0e ; call bios interrupt
  mov bh, 0
  int 0x10

  jmp .loop

.done: 
  pop ax
  pop si
  ret


msg_hello: db 'Hello world! from kernel updated ', ENDL, 0
; $ - start of the current line
; $$ - start of the current section i.e. 0x7C00
