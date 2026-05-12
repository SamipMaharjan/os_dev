; Calling convention is rules the caller and callee has to adhere to.
; Contains rules about: 
; - How function has to be called
; - How parameters are passed
; - How stack is managed
; 
; CDECL calling convention
; Arguments: 
;     - must be passed through stack
;    2 - must be pushed from right to left
;     - caller removes parameters from stack
; Return: 
;     - integers, pointers: EAX
;     - floating point: ST0
; Registers: 
;    1 - EAX, ECX, EDX are saved by the caller
;     - All others saved by callee

bits 16

section _TEXT class=CODE

; In 64 bit mode dividend can be 128 bits long and divisor can be 64 bits long. 
; But 32 bits architecture dividend should be 64 bits long and divisor should be 32. 
; And the quotient should be 32 bits long as well. 
; The WCC compiler however casts both the divisor and dividend to be largest type i.e. of length 64 bits. 
; In doing so the normal instructions for dividing will not be supported and WCC will try to 
; link the division part to runtime routines _U8DQ and _U8DR. But since linking to external WCC 
; libraries are disabled for the bootloader stage 2 it will not work. 
;
; The following routine handles division manually.
global _x86_div64_32
_x86_div64_32:
    push bp               ; save old call frame
    mov bp, sp            ; initialize new call frame

    push bx

    ; divide upper 32 bits
    mov eax, [bp+8]                 ; eax <- upper 32 bits of dividend / first argument
                                    ; Since its little endian, the upper 32 bits will be in higher half of the memory so 
                                    ; we do bp+8 to access the upper bits. 
    mov ecx, [bp+ 12 ]              ; ecx <- divisor || second argument 
    xor edx, edx
    div ecx                         ; eax - quotient, edx - remainder

    ; store upper 32 bit of quotient
    mov bx, [bp+16]                 ; get pointer to quotient
    mov [bx + 4], eax               ; store quotient in the upper half 

    ; divide lower 32 bits
    mov eax, [bp + 4]               ; eax <- lower 32 bits of dividend
                                    ; edx <- has old remainder
    div ecx

    ; store results
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret





; global makes sure the label can be accessed from other files when linking.
global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype: 
    ; make new call frame
    push bp                   ; save old call frame
    mov bp, sp                ; initialize new call frame

    ; save bx
    push bx

    ; [bp+0] - old call frame
    ; [bp+2] - return address (small memory model => 2 bytes)
    ; [bp+4] - first argument (character); bytes are converted to words (you can't push a single byte on the stack)
    ; [bp+6] - second argument (page)
    mov ah, 0Eh
    mov al, [bp+4]    
    mov bh, [bp+6]
    int 10h               ; interrupt for  video services
    
    ; restore bx
    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret
