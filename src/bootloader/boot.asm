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

bdb_bytes_per_sector: dw 512
bdb_sectors_per_cluster: db 1
bdb_reserved_sectors: dw 1
bdb_fat_count: db 2
bdb_dir_entry_count: dw 0E0h
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
    mov ax, 0   ; cant write to ds/es directly. Stack segments should be loaded from general registers
    mov ds, ax
    mov es, ax

    ;setup stack
    mov ss, ax
    mov sp, 0x7C00  ; stack grows downward from where we have loaded it in memory.

    ; make the code segment to zero because some bios might put the code in 07C0:0000
    ; so to make sure we are in the expected location even if that happens
    push es ; es is set to 0x0000 by BIOS
    push word .after ; offset of .after
    retf  ; pops first 2 bytes from stack to IP and next 2 bytes to CS so CS:IP becomes 0000:7C00

.after:
    ;read something from floppy disk
    ; BIOS should set DL to drive number
    mov [ebr_drive_number], dl

    ; mov ax, 1 ; LBA=1, second sector form disk
    ; mov cl, 1 ; 1 sector to read
    ; mov bx, 0x7E00 ; memory address to copy the disk data to. 
    ; call disk_read

    ; ;print message 
    ; mov si, msg_loading
    ; call puts

    ; read drive parameters
    ; 8h as argument gets the physical dimensions of the disk
    push es
    mov ah, 08h
    int 13h
    jc floppy_error
    pop es

    and cl, 0x3F      ; remove top 2 bits, as top 2 holds ax cylinder number and bottom 6 bits holds max sector number. 
    xor ch, ch        ; zeroes out ch register.
    mov [bdb_sectors_per_track], cx   ; sector count

    inc dh ; DH returns max head index (0-based), so increment to get total head COUNT
    mov [bdb_heads], dh         ; head count

    ; read FAT root directory
    ; compute lba of root directory
    mov ax, [bdb_sectors_per_fat]    ; LBA of root_dir = reserved sectors + fat sectors
    mov bl, [bdb_fat_count] 
    xor bh, bh  ; zeroes bh
    mul bx      ; multipies bx register with ax register and stores it in ax or dx:ax if not enough space
                ; ax = fat_count * sectors per fat
    add ax, [bdb_reserved_sectors] ; ax = reserved + fat_count * sectors_per_fat
    push ax

    ; compute size of root directory = (32 * num_of_entries) / bytes_per_sector
    mov ax, [bdb_dir_entry_count]
    shl ax, 5                       ; ax *= 32
    xor dx, dx                      ; dx = 0  
    div word [bdb_bytes_per_sector] ; number of sectors & stores he quotient in AX and remainder in DX

    test dx,dx          ; if dx!= 0, add 1
    jz .root_dir_after
    inc ax              ; division remainter != 0, add 1
                        ; which means we have a sector only partially filled with entries

.root_dir_after:
  mov cl, al ; cl = num of secors to read / size of root dir that was computed and saved in ax.
  pop ax     ; ax = LBA of root directory saved previously
  mov dl, [ebr_drive_number] ; dl = drive number saved previously || defines drive type.
  mov bx, buffer              ; es:bx = buffer



  call disk_read


  ; search for kernel.bin
  xor bx, bx ; keeps count of no of entries checked 
  mov di, buffer ; keeps the current directory entry 

.search_kernel: ; marks the beginning of the loop.
  mov si, file_kernel_bin
  mov cx, 11    ; compare  up to 11 characters used by repe instruction to keep count
  push di
  ; repe: repeas a string instruction while operands are equal (zero flag = 1), 
  ; or until cx reaches 0 


  repe cmpsb    ; compares bytes located at ds:si and es:di 
  pop di
  je .found_kernel


  add di, 32
  inc bx
  cmp bx, [bdb_dir_entry_count]




  jl .search_kernel


  ; if kernel not found 
  jmp kernel_not_found_error

.found_kernel: 
  ; di should have the address to the entry
  mov ax, [di + 26]         ; first logical cluster field ( offset 26 msg_). Gives the cluster index in FAT.
  mov [kernel_cluster], ax

  ; load FAT from disk into memory
  mov ax, [bdb_reserved_sectors]
  mov bx, buffer
  mov cl, [bdb_sectors_per_fat]
  mov dl, [ebr_drive_number]
  call disk_read


  ; read kernel and process FAT chain
  mov bx, KERNEL_LOAD_SEGMENT
  mov es, bx
  mov bx, KERNEL_LOAD_OFFSET


.local_kernel_loop: 

  ; Read next cluster
  mov ax, [kernel_cluster]
  add ax, 31                                          ; first cluster = (kernel_cluster -2) * sectors_per_cluster * start_sector
                                                      ; start sector = reseved + fats + root dir size = 1 * 18 + 134 = 33

  mov cl, 1
  mov dl, [ebr_drive_number]
  call disk_read
   

  add bx, [bdb_bytes_per_sector]

  ; compute lcoation of next cluster
  mov ax, [kernel_cluster]
  mov cx, 3
  mul cx
  mov cx, 2
  div cx                                              ; ax = index of entry in FAT, dx = cluster mod 2


  mov si, buffer
  add si, ax
  mov ax, [ds:si]             ; read entry from FAT table at index ax


  or dx, dx
  jz .even

.odd: 
  shr ax, 4
  jmp .next_cluster_after

.even: 
  and ax, 0xFFF

.next_cluster_after: 
  cmp ax, 0xFF8                         ; end of chain
  jae .read_finish

  mov [kernel_cluster], ax

  jmp .local_kernel_loop

.read_finish: 
  ; jump to our kernel
  mov dl, [ebr_drive_number]; boot device in dl


    ; set segment registers
  mov ax, KERNEL_LOAD_SEGMENT 
  mov ds, ax
  mov es, ax

  jmp KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET
  jmp wait_key_and_reboot ; should never happen


  cli
  hlt

  

floppy_error: 
  mov si, msg_read_failed
  call puts
  jmp wait_key_and_reboot

kernel_not_found_error:
  mov si, msg_kernel_not_found
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


msg_loading: db 'Loading...!', ENDL, 0
msg_read_failed: db 'Read form disk failed', ENDL, 0
msg_kernel_not_found: db 'KERNEL.BIN file not found', ENDL, 0
file_kernel_bin: db 'KERNEL  BIN'
kernel_cluster: dw 0

KERNEL_LOAD_SEGMENT     equ 0x2000
KERNEL_LOAD_OFFSET      equ 0



; $ - start of the current line
; $$ - start of the current section i.e. 0x7C00
times 510-($-$$) db 0 ; times is like loop in assembly
dw 0AA55h

buffer: 
