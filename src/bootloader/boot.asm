org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A ; Hex for  \r \n  for putting cursor in newline

;
; FAT12 Header
; short: relative jump 
;
jmp short start
nop

; 8bytes. The OEM identifier represnts the DOS version and is typically ignored but some drivers expect MSWIN4.1
bdb_oem db 'MSWIN4.1' 

bdb_byptes_per_sector: dw 512
bdb_sectors_per_cluster: db 1
bdb_reserved_sectors: dw 1
bdb_fat_count: db 2
bdb_dir_entrier_count: dw 0E0h
bdb_total_sectors: dw 2880 ; 2880 * 512 = 1.44 MB
bdb_media_descriptor_type: db 0F0h ; F0 = 3.5" floppy disk
bdb_sectors_per_fat: dw 9 ; 9sectors/fat
bdb_sectors_per_track: dw 18
bdb_heads: dw 2
bdb_hidden_sectors: dd 0
bdb_large_sector_count: dd 0

; extended boot record
ebr_drive_number: db 0 ; 0x00 floppy, 0x80 hdd, useless
db 0
ebr_signature: db 29h
ebr_volume_id: db 11h, 35h, 4h, 79h
ebr_volume_label: db 'SAMIP OS'
ebr_system_id: db 'FAT12   '


; ASM CODE STARTS: 
start: 
  jmp main

; Prints a string to the screen
; Params:
;   - ds:si points to the string 
;   - Will have the string in the above segment:offset combination. and print it until it encounters a null character.
;

; Instructions for printing Characters in screen
puts: 
  ; save registers we will modify. As a function must not permanently destroy values in register that it will use. 
  push si 
  push ax

.loop:
  lodsb ; loads next character in al
  
  ; OR destination, source performs an or operation in source and destination and stores the result in destination
  ; if operating OR on itself then it modifies the zero flag register if the result is zero/false
  or al, al ; verify if the next har is null or not
  jz .done

  mov ah, 0x0e ; call bios interrupt
  mov bh, 0
  int 0x10

  jmp .loop

.done: 
  pop ax
  pop si
  ret

main:
    mov ax, 0   ; cant write to ds/es directly. Stack segments should be loaded from general registers
    mov ds, ax
    mov es, ax

    ;setup stack
    mov ss, ax
    mov sp, 0x7C00  ; stack grows downward from where we have loaded it in memory.

    ;read something from floppy disk
    ; BIOS should set DL to drive number
    mov [ebr_drive_number], dl

    mov ax, 1 ; LBA=1, second sector form disk
    mov cl, 1 ; 1 sector to read
    mov bx, 0x7E00 ; memory address to copy the disk data to. 
    call disk_read

    ;print message 
    mov si, msg_hello
    call puts

    cli
    hlt

floppy_error: 
  mov si, msg_read_failed
  call puts
  jmp wait_key_and_reboot

wait_key_and_reboot: 
   mov ah, 0 
   int 16h    ; wait for keypress
   jmp 0FFFFh:0 ; jump to beginning of BIOS, should reboot
   hlt

.halt: 
  cli ; disable interrupts, this way CPU can't get out of "halt" state 
  hlt

;
; Disk routines
; 


; 
; Converts and LBA address to a CHS address
; Parameters:
;   - ax: LBA address
; Returns: 
;   - cx [bits 0-5]: sector number
;   - cx [bits 6-15]: cylinder
;   - dh: head
;

lba_to_chs:
  push ax
  push dx

  xor dx, dx ; dx = 0
  div word [bdb_sectors_per_track]  ; divide dx:ax by bdb_sectors_per_track || 19/18
                                    ; ax = LBA / SectorsPerTrack 
                                    ; dx = LBA % SectorsPerTrack || remainder || sector on that track. || 1  
  
  inc dx ; dx = (LBA % SectorsPerTrack) + 1 = sector || 2
  mov cx, dx ; cx = sector

  xor dx, dx ; dx = 0
  div word [bdb_heads] ; ax = (LBA/SectorsPerTrack) / Heads ||  quotient  = Track when track is not the the first head.
                       ; dx = (LBA / SectorsPerTrack) % Heads = head || 1 % 5 || 1 
                       

  mov dh, dl ; dh = head || moving it to the upper bits because int 13h expects it in dh.
  mov ch, al ; ch = cylinder (lower 8 bits)
  shl ah, 6
  or cl, ah  ; put upper 2 bits of cylinder in CL

  pop ax
  mov dl, al ; restore DL 
  pop ax
  ret


; 
; Reads sectors form a  disk
; Parameters: 
;   -ax: LBA address
;   -cl: number of sectors to read (up to 128)
;   -dl: drive umber
;   -es:bx: memory address wher eto store read data
; 

disk_read:
  push ax ; save register that we will modify
  push bx
  push cx
  push dx
  push di

  push cx ; save CL which contains no. of sectors to read 
  call lba_to_chs ; compute CHS
  pop ax ; AL = number of sectors to read

  mov ah, 02h
  mov di, 3 ; retry count

.retry: 
  pusha ; save all register, we dont know what bios modifies
  stc ; set carry flag, some BIOS'es dont set it
  int 13h ; carry flag cleared = success
  jnc .done ; jump if carry flag is not  zero, meaning if the disk read was successful.

  ; read failed
  popa
  call disk_reset

  dec di
  test di, di 
  jnz .retry

.fail: 
  ; all attempts are exhausted
  jmp floppy_error

.done: 
  popa

  pop di ; pop the saved registers
  pop dx
  pop cx
  pop bx
  pop ax
  ret

;
; Resets disk controller
; Parameters: 
; dl: drive number
;
disk_reset: 
  pusha 
  mov ah, 0
  stc
  int 13h 
  jc floppy_error
  popa 
  ret 


msg_hello: db 'Hello world!', ENDL, 0

msg_read_failed: db 'Read form disk failed', ENDL, 0

; $ - start of the current line
; $$ - start of the current section i.e. 0x7C00
times 510-($-$$) db 0 ; times is like loop in assembly
dw 0AA55h
