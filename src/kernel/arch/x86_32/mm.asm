global enable_paging
enable_paging: 
xchg bx, bx
  ; setting new call frame
  push ebp
  mov ebp, esp
  ; and saving values in all GP registers
  pushad 

  ; pdt_addr 
  ; at bp + ( 2 * word(32bit) )
  mov eax, [ebp+8]
  mov cr3, eax

  mov eax, cr0
  or eax, 0x80000001
  mov cr0, eax

  ; setting back the call frame and register values
  popad
  pop ebp
  ret
