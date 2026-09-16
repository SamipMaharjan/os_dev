original_out_buffer: 
  dd 0
global _x86_Map_Memory
_x86_Map_Memory:
  push bp                   ; save old call frame
  mov bp, sp                ; initialize new call frame

  cli
  pusha

  ; Get the first argument
  mov eax, [bp+4]
  
  ; Store it in original_out_buffer
  mov [original_out_buffer], eax

  ; Increment by 4
  ; First 4 bytes contains the 
  ; number of entries.
  add eax, 4

  ; Set es:di to arg1(outputBuffer)
  mov di, ax
  shr eax, 16
  mov es, ax


  ; Set EBX to 0
  mov ebx, 0

.loop:
  ; Set EDX to magic number 
  mov edx, 0x534D4150
  ; Set EAX to 0x0000E820
  mov eax, 0x0000E820

  ; Set ECX to 24
  mov ecx, 24

  ; call int 0x15
  int 0x15

  ; CF set = error
  jc .error

  ; increment the entry count
  call .increment_entry_count

  add di, 24
  ; ebx=0 means final entry
  cmp ebx, 0
  jnz .loop

.end: 
xchg bx, bx
  call .set_length_on_buffer
  popa
  sti
  ; restore call frame
  mov sp, bp
  pop bp
  ret

.error: 
  mov eax, 1
  jmp .end
  hlt 

.increment_entry_count: 
  push eax

  mov eax, [entries_length]
  inc eax
  mov [entries_length], eax

  pop eax
  ret

.set_length_on_buffer
  push eax
  push ebx
  push es
  push di

  mov eax, [original_out_buffer]
  mov ebx, [entries_length]
  mov di, ax
  shr eax, 16
  mov es, ax
  mov es:[di], ebx

  pop di
  pop es
  pop ebx
  pop eax
  ret

entries_length: 
  dd 0 
