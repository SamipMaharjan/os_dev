bits 16

section _ENTRY class=CODE

extern _cstart_
global entry

; using a small memory model so stack and data segments should be the same.
entry: 
  cli
  mov ax, ds    ; data segment is already defined by stage 1 
  mov ss, ax    ; so just moving the same value to stack segment.
  mov sp, 0
  mov bp, sp
  sti

  ; expect boot drive in dl, send it as argument to cstart functio 
  xor dh, dh
  push dx
  call _cstart_

  cli 
  hlt
