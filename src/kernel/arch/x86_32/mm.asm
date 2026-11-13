global enable_paging
enable_paging: 
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
  ; mov esp, ebp
  pop ebp
  ret

; return cr3's value 
global x86_get_cr3
x86_get_cr3:
  push ebp
  mov ebp, esp

  mov eax, cr3

  pop ebp
  ret
