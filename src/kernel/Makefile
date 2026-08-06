BUILD_DIR?=build/
ASM?=nasm
# CC16?=/opt/watcom/binl/wcc
# LD16?=/opt/watcom/binl/wlink
GCC?=/home/samip/Desktop/projects/BuildCrossCompiler/i686-elf/bin/i686-elf-gcc
LD?=/home/samip/Desktop/projects/BuildCrossCompiler/i686-elf/bin/i686-elf-ld

SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)

OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/kernel/c/%.o, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.o, $(SOURCES_ASM))


.PHONY: all kernel clean always

all: kernel

kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf:  $(OBJECTS_ASM) $(OBJECTS_C)
	# echo "Hello"
	# $(ASM) main.asm -f bin -o $(BUILD_DIR)/kernel.elf
	# $(GCC) -c kernel.c -o $(BUILD_DIR)/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	# $(ASM) main.asm -f elf32 -o $(BUILD_DIR)/main.o
	# $(GCC) -T linker.ld -o $(BUILD_DIR)/kernel.elf -ffreestanding -O2 -nostdlib $(BUILD_DIR)/main.o $(BUILD_DIR)/kernel.o -lgcc
	# $(info **************************debug :  $^ )
	# $(GCC) -T linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc
	$(GCC) -T linker.ld -o $@ -ffreestanding -O2 -nostdlib  $^  -lgcc

# Cross compiling all c files for protected mode.
$(BUILD_DIR)/kernel/c/%.o: %.c always
	# $(GCC) $(CFLAGS16) -fo=$@ $<
	# $(ASM) main.asm -f bin -o $(BUILD_DIR)/kernel.elf
	$(GCC) -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra  

$(BUILD_DIR)/kernel/asm/%.o: %.asm always
	$(ASM) $< -f elf32 -o $@

always: 
	mkdir -p $(BUILD_DIR)/kernel/asm
	mkdir -p $(BUILD_DIR)/kernel/c

clean: 
	rm -f $(BUILD_DIR)/kernel.elf
