BUILD_DIR?=build/
ASM?=nasm
# CC16?=/opt/watcom/binl/wcc
# LD16?=/opt/watcom/binl/wlink
GCC?=/home/samip/Desktop/projects/BuildCrossCompiler/i686-elf/bin/i686-elf-gcc
LD?=/home/samip/Desktop/projects/BuildCrossCompiler/i686-elf/bin/i686-elf-ld

# SRCS := $(shell find . -name '*.c')
# SOURCES_C=$(wildcard *.c)
# SOURCES_ASM=$(wildcard *.asm)
SOURCES_C=$(shell find . -name '*.c')
SOURCES_ASM=$(shell find . -name '*.asm')

DIRECTORIES=$(shell find  -type d)

# $(info **************************debug:  $(DIRECTORIES) )

OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/kernel/c/%.o, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.o, $(SOURCES_ASM))

C_FLAGS= -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g -Iinclude -I.

.PHONY: all kernel clean always

all: kernel

kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf:  $(OBJECTS_ASM) $(OBJECTS_C)
	# $(ASM) main.asm -f bin -o $(BUILD_DIR)/kernel.elf
	# $(GCC) -c kernel.c -o $(BUILD_DIR)/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	# $(ASM) main.asm -f elf32 -o $(BUILD_DIR)/main.o
	# $(GCC) -T linker.ld -o $(BUILD_DIR)/kernel.elf -ffreestanding -O2 -nostdlib $(BUILD_DIR)/main.o $(BUILD_DIR)/kernel.o -lgcc
	$(GCC) -T linker.ld -o $@ -ffreestanding -O2 -nostdlib  $^  -lgcc

# Cross compiling all c files for protected mode.src/kernel/test/%.c
$(BUILD_DIR)/kernel/c/%.o: %.c  always
	$(GCC) -c $< -o $@ $(C_FLAGS)

$(BUILD_DIR)/kernel/asm/%.o: %.asm always
	$(ASM) $< -f elf32 -g -F dwarf -o $@

always: 
# mkdir -p $(DIRECTORIES:%=build/%)
	mkdir -p $(BUILD_DIR)/kernel/asm
	mkdir -p $(DIRECTORIES:%=$(BUILD_DIR)/kernel/c/%)
	mkdir -p $(DIRECTORIES:%=$(BUILD_DIR)/kernel/asm/%)
	# mkdir -p $(BUILD_DIR)/kernel/c
	# mkdir -p $(BUILD_DIR)/kernel/c/arch/x86_32
	# mkdir -p $(BUILD_DIR)/kernel/c/include/utils

clean: 
	rm-f $(BUILD_DIR)/kernel.elf
