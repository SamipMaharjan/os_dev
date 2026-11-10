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

global transition_higherhalf
transition_higherhalf:
  push ebp
  mov ebp, esp
  ; pushad
  mov eax, [ebp+8] ; access the higher half addr 

  ; resetting the call frame
  mov esp, ebp
  pop ebp
  jmp eax

  ; should never happen
  hlt

  
