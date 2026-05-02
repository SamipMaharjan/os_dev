org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

start: 
  jmp main

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
  or al, al ; verify if the next har is null or not
  jz .done

  mov ah, 0x0e ; call bios interrupt
  mov bh, 0
  int 0x10

  jmp .loop

.done: 
  pop ax
  pop si
  ret

main:
    mov ax, 0   ; cant write to ds/es directly. Stack segments should be loaded from general registers
    mov ds, ax
    mov es, ax

    ;setup stack
    mov ss, ax
    mov sp, 0x7C00  ; stack grows downward from where we have loaded it in memory.

    ;print message 
    mov si, msg_hello
    call puts

    hlt

.halt: 
    jmp .halt

msg_hello: db 'Hello world!', ENDL, 0
; $ - start of the current line
; $$ - start of the current section i.e. 0x7C00
times 510-($-$$) db 0 ; times is like loop in assembly
dw 0AA55h
