; This is the stage 1 of the bootloader. 
; The binary from this asm file will be copied to first sector of FAT image
; Here are the things this block of instructions does: 
; - Defines the Bios Parameter Block so that BIOS knows its dealing with FAT12 
; - Reads drive parameters (Max CHS values) using 13h 8h and stores it.
; - Computes Logical Block Address of root directory. 
; - Computes the size of root_directory. 
; - Reads entire rootdirectory entry to memory. Uses 13h 02h, and disk_read routine.
; - Iterates over the entries for "STAGE2  BIN" entry. Using "repe cmpsb" instruction.
; - Loads the File Allocation Table in memory, overwriting the rootdir entries. 
; - Loads the data of first cluster of STAGE2 BIN at STAGE2's segment:offset
; - Uses FAT and cluster numbers to load all data of STAGE2.BIN at its segment:offset
; - Jumps to STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET to run stage 2.

org 0x7C00              ; org directive for telling assembler where the binary will be loaded in memory
bits 16                 ; bits directive for telling assembler the type of CPU we are assembling this into

%define ENDL 0x0D, 0x0A ; Hex for  \r \n  for putting cursor in newline

;
; FAT12 Header
; Boot sector structure
; [ Jump + OEM ]
; [ BPB ]              ← core filesystem parameters
; [ Extended BPB ]     
; [ Boot code ]
; [ 0xAA55 signature ] 

; short: relative jump 
jmp short start
nop

; 8bytes. The OEM identifier represnts the DOS version and is typically ignored but some drivers expect MSWIN4.1
bpb_oem: db 'MSWIN4.1' 

bpb_bytes_per_sector: dw 512
bpb_sectors_per_cluster: db 1       ; should be the power of 2
bpb_reserved_sectors: dw 1          ; no of sectors before FAT section. Counts Boot Sector as well.
bpb_fat_count: db 2                 ; total number of FATs. Normally 2 for redundency.
bpb_dir_entry_count: dw 0E0h        ; 224 entries
bpb_total_sectors: dw 2880          ; 2880 * 512 = 1.44 MB
bpb_media_descriptor_type: db 0F0h  ; F0 = 3.5" floppy disk, F8 = Hard disk
bpb_sectors_per_fat: dw 9           ; 9sectors/fat
bpb_sectors_per_track: dw 18
bpb_heads: dw 2
bpb_hidden_sectors: dd 0            ; No of sectors before the FAT volume starts. Before the BPB starts.
                                    ; Used when using FAT in Hard disks that uses MBR. 

bpb_large_sector_count: dd 0        ; 32-bit total sectors used when disk is too large to fit total sectors in 16-bits
                                    ; Among bpb_total_sectors and bpb_large_sector_count only one will be used.

; extended boot record
ebr_drive_number: db 0 ; 0x00 - 0x7F floppy, 0x80 - 0xFF HDD. Ignored because BIOS gets the drive number in DL register.
db 0
ebr_signature: db 29h
ebr_volume_id: db 11h, 35h, 4h, 79h
ebr_volume_label: db 'SAMIP OS'
ebr_system_id: db 'FAT12   '


; ASM CODE STARTS: 
start:
    mov ax, 0   ; cant write to ds/es directly. Stack segments should be loaded from general registers
    mov ds, ax  ; data segment set to 0 
    mov es, ax  ; extended segment set to 0.  

    ;setup stack
    mov ss, ax
    mov sp, 0x7C00  ; stack grows downward from where we have loaded it in memory.
                    ; And there is 30 KB of free memory below 0x7C00

    ; make the code segment to zero because some bios might put the code in 07C0:0000
    ; so to make sure we are in the expected location even if that happens
    push es                         ; es is set to 0x0000 by BIOS
    push word .after                ; offset of .after
    retf                            ; pops first 2 bytes from stack to IP and next 2 bytes to CS so CS:IP becomes 0000:7C00

.after:
    ; read something from floppy disk
    ; BIOS should set DL to drive number
    mov [ebr_drive_number], dl

    ; print message 
    mov si, msg_loading
    call puts

    ; read drive parameters
    ; 8h as argument gets the physical dimensions of the disk
    push es               ; Saving ES register as `int 13h 8h` sets ES:DI address as Hard Disk Parameter Table
    mov ah, 08h           ; Reports disk drive parameters, such as the number of heads, max cylinders, and sectors per track.
    int 13h               ; BIOS Call for disk routines, uses cx(cylinder and sectors) and dh(max-head index) registers. 
    jc floppy_error       ; Handle if error when reading 
    pop es                ; Poping the saved value

    and cl, 0x3F                      ; remove top 2 bits, as top 2 bits holds high bits for max cylinder number 
                                      ; bottom 6 bits holds max sectors per track. 
    xor ch, ch                        ; zeroes out ch register. Which contains the lower 8 bits of max cylinder number
    mov [bpb_sectors_per_track], cx   ; sector count

    inc dh                      ; DH returns max head index (0-based), so increment to get total head COUNT
    mov [bpb_heads], dh         ; head count

    ; read FAT root directory
    ; compute lba of root directory
    mov ax, [bpb_sectors_per_fat]     ; LBA of root_dir = reserved sectors + fat sectors
    mov bl, [bpb_fat_count] 
    xor bh, bh                        ; zeroes bh
    mul bx                            ; multipies bx register with ax register and stores it in ax or dx:ax if not enough space
                                      ; ax = fat_count * sectors per fat
    add ax, [bpb_reserved_sectors]    ; ax = reserved + fat_count * sectors_per_fat
    push ax                           ; saving the computed value

    ; compute size of root directory = (32 * num_of_entries) / bytes_per_sector
    mov ax, [bpb_dir_entry_count]
    shl ax, 5                       ; ax *= 32
    xor dx, dx                      ; dx = 0  
    div word [bpb_bytes_per_sector] ; number of sectors & stores he quotient in AX and remainder in DX
                                    ; specifying word to define the size of dividend

    test dx,dx                      ; if dx!= 0, add 1
    jz .root_dir_after
    inc ax                          ; division remainter != 0, add 1
                                    ; which means we have a sector only partially filled with entries

.root_dir_after:
  mov cl, al                  ; cl = num of secors to read || size of root dir that was computed and saved in ax.
  pop ax                      ; ax = LBA of root directory saved previously
  mov dl, [ebr_drive_number]  ; dl = drive number saved previously || defines drive type.
  mov bx, buffer              ; ES:BX = Address of memory buffer to write the disk data. Used by 13h 02h
  call disk_read              ; If successful returns from the local label .done 
                              ; And writes the disk data to memory 0x7E00 i.e. the buffer label 

  ; search for stage2.bin
  xor bx, bx                  ; setting bx to 0, later it keeps count of no of entries checked 
  mov di, buffer              ; Saving address of buffer in DI register as CMPSB uses the ES:DI registers.  

.search_stage2:                     ; marks the beginning of the loop.
  mov si, file_stage2_bin           
  mov cx, 11                        ; used to indicate charaters to compare by repe instruction 

  ; REPE: repeats a string instruction while operands are equal (zero flag = 1), 
  ; or until cx reaches 0 
  ; 
  ; CMPSB: compares bytes located at DS:SI and ES:DI 
  ; 
  ; So the below instruction compares every character in file_stage2_bin 
  ; against the first 11 bytes in address contained by the ES:DI register.
  ; I.E the start of root directory which contains the first entry's file name 
  push di
  repe cmpsb                        
  pop di                             

  ; IF all 11 bytes in the entry matches to the bytes in file_stage2_bin
  ; zero flag will still be 1 when cx is zero therefore it will jump to .found_stage2 label.  
  je .found_stage2                   


  add di, 32
  inc bx

  ; loop stage_2 if bx < directory entry count
  cmp bx, [bpb_dir_entry_count]
  jl .search_stage2

  ; if stage2 not found 
  jmp stage2_not_found_error

.found_stage2: 
  ; di has the address to the stage2.bin entry
  mov ax, [di + 26]                   ; first logical cluster field ( offset 26 ). Gives the cluster number in FAT.
  mov [stage2_cluster], ax            ; sotring stage2.bin cluster number in stage2_cluster label.

  ; load FAT from disk into memory
  mov ax, [bpb_reserved_sectors]      ; LBA for lba_to_chs.
  mov bx, buffer                      ; Address to copy disk bytes. Used by 13h 02h
  mov cl, [bpb_sectors_per_fat]       ; Number of sectors to read. Used by 13h 02h
  mov dl, [ebr_drive_number]          ; Drive number used by 13h 02h

  call disk_read

  ; SECTION:  read stage2 and process FAT chain
  mov bx, STAGE2_LOAD_SEGMENT
  mov es, bx
  mov bx, STAGE2_LOAD_OFFSET ; ES:BX used by 13h 02h
  
.local_stage2_loop: 
  ; Read next cluster
  mov ax, [stage2_cluster]
  add ax, 31                                          ; LBA used by 13h 02h
                                                      ; first cluster = (stage2_cluster -2) * sectors_per_cluster + start_sector
                                                      ; start sector = reseved + fats + root dir size = 1 + 1 * 18 + 14 = 33
                                                      ; first_cluster = stage2_cluster-2 * 1 + 33 = stage2_cluster + 31
                                                      ; (x-2)*1+33 = x+31

  mov cl, 1                             ; No of sectors to read used by 13h 02h ; improve: make it sectors_per_cluster
  mov dl, [ebr_drive_number]            ; Drive number used by 13h 02h

  call disk_read                        ; Reading cluster into STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

  add bx, [bpb_bytes_per_sector]        ; Adding the read destination address by 1 sector / 512 bytes; improve: make it bytes_per_sector * sectors_per_cluster

  ; compute location of next cluster
  mov ax, [stage2_cluster]               
  mov cx, 3
  mul cx                                ; Cluster * 3
  mov cx, 2                              
  div cx                                ; ax = (cluster * 3)/2
                                        ; ax = byte index ( with respect to FAT sector ) containing the FAT entry, dx = (cluster * 3) % 2
  mov si, buffer                        
  add si, ax                            ; buffer = FAT, ax = byte index containg the FAT entry
  mov ax, [ds:si]                       ; read 2 bytes from FAT+byte_index at index AX.

  or dx, dx                             ; Set zero flag to 1 if there are no remainders. 
  jz .even                              ; jump to even if zero flag is 1. 

.odd:                                   ; If byte_index is odd 
  shr ax, 4                             ; Gets rid of 4 least significant bits
  jmp .next_cluster_after

.even: 
  and ax, 0xFFF                         ; Gets rid of 4 most significant bits

.next_cluster_after: 
  cmp ax, 0xFF8                         ; If end of chain
  jae .read_finish                      ; Jump if ax is above or equal 0xFF8 uses CF register. 

  mov [stage2_cluster], ax              ; If not the end of chain then update the cluster. 
  jmp .local_stage2_loop                ; And jump to start of loop


.read_finish: 
  ; jump to our stage2
  mov dl, [ebr_drive_number]; boot device in dl


  ; setup segment registers for stage2
  mov ax, STAGE2_LOAD_SEGMENT 
  ; xor ax, ax
  mov ds, ax
  mov es, ax

  jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET          ; Jump to the binary loaded in memory at this segment:offset 
  jmp wait_key_and_reboot                             ; should never happen


  cli
  hlt

floppy_error: 
  mov si, msg_read_failed
  call puts
  jmp wait_key_and_reboot

stage2_not_found_error:
  mov si, msg_stage2_not_found
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
lba_to_chs:
  push ax                           
  push dx

  xor dx, dx                        ; Setting dx to 0 before using div instruction as it uses dx register
  div word [bpb_sectors_per_track]  ; divide dx:ax by bpb_sectors_per_track || 19/18
                                    ; ax = LBA / SectorsPerTrack || quotient
                                    ; dx = LBA % SectorsPerTrack || remainder  || SECTOR NUMBER 
  inc dx                            ; dx = (LBA % SectorsPerTrack) + 1 = sector || Since it is not index based.
  mov cx, dx                        ; Saving sector number in cx register

  xor dx, dx                        ; Setting dx to zero before div instruction
  div word [bpb_heads]              ; ax = (LBA/SectorsPerTrack) / Heads ||  quotient  || CYLINDER NUMBER
                                    ; dx = (LBA / SectorsPerTrack) % Heads || remainder || HEAD NUMBER 

  mov dh, dl                        ; dh = head || moving it to the upper bits because int 13h expects it in dh.
  mov ch, al                        ; ch = cylinder (lower 8 bits of 10-bit cylinder number moved from AL to CH)
  shl ah, 6                         ; left shifting AH by 6 as 
  or cl, ah                         ; put upper 2 bits of cylinder number in CL
                                    ; So the bits in final CX register will be like: 
                                    ; 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 
                                    ;  - max cylinder number  -|| - max sector no - | 
                                    ; Bits 0 - 5 = Max sector no (CL register)
                                    ; Bits 6 and 7 = Upper bits of max cylinder no (CL register)
                                    ; Bits 8 to 15 = Lower bits of max cylinder no (CH register)

  pop ax                            ; popping value of DX in AX to get DL
  mov dl, al                        ; restore DL which contains drive number 
  pop ax                            ; restore AX
  ret

; Reads sectors form a  disk using 13h 02h
; Parameters: 
;   -ax: LBA address
;   -cl: number of sectors to read (up to 128)
;   -dl: drive number
;   -es:bx : memory address wher eto store read data
disk_read:
  push ax                   
  push bx                  
  push cx
  push dx
  push di

  push cx                           ; save CL which contains no. of sectors to read 
  call lba_to_chs                   ; compute CHS, returns CX = max no of sectors and cylinders, DH = no of heads
  pop ax                            ; RESTORE CL to AL ,so AL = number of sectors to read 

  mov ah, 02h                       ; argument for int 13h to read sectors 
  mov di, 3                         ; retry count

.retry: 
  pusha                             ; save all register, we dont know what bios modifies
  stc                               ; set carry flag to 1, some BIOS'es dont set it
  int 13h                           ; carry flag cleared = success 
  jnc .done                         ; jump if carry flag is not  zero, meaning if the disk read was successful.

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
; Resets head position and error state
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
msg_stage2_not_found: db 'STAGE2.BIN file not found', ENDL, 0
file_stage2_bin: db 'STAGE2  BIN'
stage2_cluster: dw 0

STAGE2_LOAD_SEGMENT     equ 0x2000
STAGE2_LOAD_OFFSET      equ 0



; $ - start of the current line
; $$ - start of the current section i.e. 0x7C00
times 510-($-$$) db 0 ; times is like loop in assembly
dw 0AA55h

buffer: 
