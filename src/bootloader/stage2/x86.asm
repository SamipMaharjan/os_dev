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


;
;     Name:           U4D                                            
;     Operation:      Unsigned 4 byte divide                         
;     Inputs:         DX;AX   Dividend                               
;                     CX;BX   Divisor                                
;     Outputs:        DX;AX   Quotient                               
;                     CX;BX   Remainder                              
;     Volatile:       none                                           
;
global __U4D
__U4D: 
    shl edx, 16                 ; dx to upper half of edx
    mov dx, ax                  ; edx - dividend
    mov eax, edx                ; eax - dividend
    xor edx, edx                ; edx - 0

    shl ecx, 16                 ; cx to upper half of ecx
    mov cx, bx                  ; ecx - divisor

    div ecx                     ; eax - quot, edx - remainder
    mov ebx, edx              
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16

    ret

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
    mov ah, 0Eh           ; int 10h arg for printing a character
    mov al, [bp+4]    
    mov bh, [bp+6]
    int 10h               ; interrupt for  video services
    
    ; restore bx
    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret


;
; void _cdecl x86_Disk_Reset(uint8_t drive);
; 
; INT 
; Input:
; AH = 00h
; DL = Drive number
;
; Output:
; AH = Status of operation
; CF = Set if error, cleared otherwise
global _x86_Disk_Reset
_x86_Disk_Reset: 
    ; make new call frame
    push bp                   ; save old call frame
    mov bp, sp                ; initialize new call frame

    mov ah, 0
    mov dl, [bp+4]            ; dl - drive
    stc 
    int 13h 

    mov ax, 1 
    sbb ax, 0                 ; 1 = true, 0  = false
                              ; subtract with borrow(sbb) = destination - (source + carry flag)

    ; restore old call frame
    mov sp, bp
    pop bp
    ret

;
; void _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t head,
;                           uint16_t sector, uint8_t count, uint8_t far *dataOut);
; INT 13H 02
; Input:
; AH = 02h
; AL = Number of sectors to read
; CH = Cylinder number (10 bit value, upper 2 bits in CL)
; CL = Starting sector number
; DH = Head number
; DL = Drive number
; ES:BX = Address of memory buffer
;
; Output:
; AH = Status of operation
; AL = Number of sectors read
; CF = Set if error, else cleared

global _x86_Disk_Read
_x86_Disk_Read: 
    ; make new call frame
    push bp                   ; save old call frame
    mov bp, sp                ; initialize new call frame

    ; setup args
    mov dl, [bp+4]            ; dl - drive

    mov ch, [bp+6]            ; ch - cylinder (lower 8 bits)
    mov cl, [bp + 7]          ; cl - cylinder to bits 6-7
    shl cl, 6

    mov dh, [bp+8]            ; dh - head

    mov al, [bp+10]           ;  sector
    and al, 3Fh
    or cl, al                 ; cl - sector to bits 0-5

    mov al, [bp+12]           ; al - count

    mov bx, [bp + 16]         ; es:bx - far pointer to data output
    mov es, bx
    mov bx, [bp + 14]         ; data out pointer
                              ; A pointer in real mode is 2 bytes but a far pointer is 4 bytes 
                              ; and since dataOut is uint8_t far pointer the offset is in 
                              ; bp+14 and segment is in bp + 16 

    ; call 13h 02h
    mov ah, 02h
    stc 
    int 13h 


    ; set return values
    mov ax, 1 
    sbb ax, 0                 ; 1 = true, 0  = false
                              ; subtract with borrow= destination - (source + carry flag)

    ; restore regs
    pop es 
    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret

; void _cdecl x86_Disk_GetDriveParams(uint8_t drive, uint8_t *driveTypeOut,
;                                     uint16_t *cylindersOut,
;                                     uint16_t *sectorsOut, uint16_t *headsOut);
; INT 13h 08h
; Input:
; AH = 08h
; DL = Drive number
;
; Output:
; CH = Maximum value for cylinder (10-bit value; upper 2 bits in CL)
; CL = Maximum value for sector
; DH = Maximum value for heads
;
; For fixed disks:
; AH = Status of operation
; DL = Number of fixed disks
; CF = Set if error; otherwise cleared
;
; For Diskette:
; AX = 0
; BL = Bits 7 to 4 = 0
;        Bits 3 to 0 – Valid drive type in CMOS
; BH = 0
; DL = Number of diskettes
; ES:DI = Pointer to 11-byte diskette drive parameter table
global _x86_Disk_GetDriveParams
_x86_Disk_GetDriveParams: 
    ; make new call frame
    push bp                   ; save old call frame
    mov bp, sp                ; initialize new call frame

    ; save regs
    push es 
    push bx
    push si
    push di 

    ; call 13h
    mov dl, [bp + 4]          ; dl - disk drive
    mov ah, 08h
    mov di, 0                 ; es:di - 0000:0000
    mov es, di
    stc 
    int 13h


    ; return success or failure
    mov ax, 1
    sbb ax, 0

    ; out params
    mov si, [bp+6]          ; drive type from bl
    mov [si], bl

    mov bl, ch              ; cylinders - lower bits in ch
    mov bh, cl              ; cylinders - upper bits in cl
    shr bh, 6
    mov si, [bp + 8]        ; cylindersOut
    mov [si], bx

    xor ch, ch              ; sectors - lower 5 bits in cl
    and cl, 3Fh
    mov si, [bp + 10]       ; sectorsOut
    mov [si], cx

    mov cl, dh              ; heads - dh
    mov si, [bp+12]
    mov [si], cx

    ; restore regs
    pop di
    pop si 
    pop bx
    pop es

    ; restore old call frame
    mov sp, bp
    pop bp
    ret
