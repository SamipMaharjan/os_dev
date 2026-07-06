org 0x0000
bits 32

%define ENDL 0x0D, 0x0A

start: 
    ;print message 
    xchg bx, bx
    mov eax, 0x1234567
    xchg bx, bx
    mov si, msg_hello


.halt: 
    cli
    hlt

msg_hello: db 'Hello world! from kernel updated ', ENDL, 0
; $ - start of the current line
; $$ - start of the current section i.e. 0x7C00
