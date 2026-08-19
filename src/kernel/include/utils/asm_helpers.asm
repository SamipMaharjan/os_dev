bits 32 

global cause_divide_error
cause_divide_error: 
  mov eax, 10
  mov ebx, 0
  div ebx
  ret

; .cause_df_handler: 
;   ; cause page fault
;   mov eax, [0xFFFFFFFF] 

global cause_df 
cause_df:
  call cause_divide_error

global cause_gpf
cause_gpf: 
  xor ax, ax
  mov ax, 0x99      ; arbitrary invalid selector
  mov ds, ax
  ret

global cause_pf
cause_pf: 


global cause_db
cause_db: 
  pushfd
  or dword [esp], 0x0100
  popfd
  nop
  nop

global cause_nmi
cause_nmi: 
  int 2
  nop

global cause_be
cause_be: 
  int3
  ret

global cause_oe
cause_oe: 
  mov al, 127
  add al, 1
  into
  nop
  nop

global cause_br 
cause_br: 
  mov ax, 10
  bound ax, [.bound]
.bound: 
  dd 0
  dd 5

; cause invalid opcode exception
global cause_ud 
cause_ud:
    db 0x0F, 0x0B       ; UD2
    nop

; cause device not available
global cause_nm
cause_nm: 
  xchg bx, bx
  mov eax, cr0
  or eax, 0b1000  ; set Task Switch flag to 1
                  ; CPU turns it on automatically when 
                  ; hardware/software taskswtich happens
  mov cr0, eax

  fld dword [.float_num]
  nop
.float_num
  dd 0x3f800000
      
; this does not work
global cause_ts
cause_ts: 
  mov eax, 0x99
  ltr eax
  nop

; segment not present 
; isr11
global cause_np
cause_np: 
  xchg bx, bx
  mov ax, 0x18
  ; mov cs, ax
  ; nop 
  mov ds, ax
  nop

; isr12
global cause_ss
cause_ss: 
  xchg bx, bx
  mov ax, 0x18
  ; mov cs, ax
  ; nop 
  mov ss , ax
  nop

; isr14
global cause_mf
cause_mf:
    ; Enable native x87 exceptions: CR0.NE = 1
    mov eax, cr0
    or  eax, 0x20              ; CR0.NE = bit 5
    mov cr0, eax

    fninit

    ; Get x87 control word
    fnstcw [cw]

    ; Unmask divide-by-zero
    ; Control-word bit 2 = ZM
    and word [cw], 0xFFFB      ; clear bit 2

    fldcw [cw]
                              
                               ; Generate x87 divide-by-zero
    fld1                       ; ST(0) = 1.0
    fldz                       ; ST(0) = 0.0, ST(1) = 1.0
    fdivp st1, st0             ; 1.0 / 0.0 -> #Z pending

    xchg bx, bx
    ; Force the pending exception to be delivered
    fwait
    nop 
    nop

    ; Should never reach here if #MF handler works
    ret

section .data
cw: dw 0

global cause_ac 
cause_ac: 
  int 0x11 
  nop

global cause_mc 
cause_mc:
  int 0x12 
  nop
  
global cause_xm_xf
cause_xm_xf:
  int 0x13 
  nop

global cause_ve
cause_ve: 
  int 0x14
  nop 

global cause_cp
cause_cp: 
  int 0x15
  nop 
