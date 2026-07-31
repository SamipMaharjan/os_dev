BUILD_DIR?=build/
ASM?=nasm
# CC16?=/opt/watcom/binl/wcc
# LD16?=/opt/watcom/binl/wlink
GCC?=/home/samip/Desktop/projects/BuildCrossCompiler/i686-elf/bin/i686-elf-gcc
LD?=/home/samip/Desktop/projects/BuildCrossCompiler/i686-elf/bin/i686-elf-ld

.PHONY: all kernel clean

all: kernel

kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: 
	# HELLO WORLD FROM GCC
	# $(ASM) main.asm -f bin -o $(BUILD_DIR)/kernel.elf
	$(GCC) -c kernel.c -o $(BUILD_DIR)/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	$(ASM) main.asm -f elf32 -o $(BUILD_DIR)/main.o
	$(GCC) -T linker.ld -o $(BUILD_DIR)/kernel.elf -ffreestanding -O2 -nostdlib $(BUILD_DIR)/main.o $(BUILD_DIR)/kernel.o -lgcc

clean: 
	rm -f $(BUILD_DIR)/kernel.elf
