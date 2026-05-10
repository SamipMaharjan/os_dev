bits 16                     # compile this for 16 bit real mode

section _ENTRY class=CODE   # Everything below this belongs to _ENTRY section of class CODE. 
                            # Until it reaches another section directve

extern _cstart_             # Directive for declaring external function/label/vairables 
                            # used by linker when linking.
                            
global entry                # Exports the label so other files can access it. 
                            # Without it a label is local to a file.

; using a small memory model so stack and data segments should be the same.
entry: 
  cli           ; Clear interrupt flag
  mov ax, ds    ; data segment is already defined by stage 1 
  mov ss, ax    ; so just moving the same value to stack segment.
  mov sp, 0     
  mov bp, sp    ; Setting base pointer.
  sti           ; Set interrupt flag

  ; expect boot drive in dl, send it as argument to cstart functio 
  xor dh, dh
  push dx
  call _cstart_

  cli 
  hlt
