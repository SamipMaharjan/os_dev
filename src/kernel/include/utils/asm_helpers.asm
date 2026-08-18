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
  
