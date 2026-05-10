BUILD_DIR?=build/
ASM?=nasm
ASMFLAGS?=-f obj

# Open-watcom compiler
CC16?=/opt/watcom/binl/wcc
# Open-watcom linker
LD16?=/opt/watcom/binl/wlink

#  -4: optimizes code for intel 80486 processor
#  -d3: Makes .exe larger but enables support for Watcom debugger
#  -s : Disables stack overflow checks. But if it happens memory corruption will occur. 
#  -wx: Set warning level max. Helps with clean code
#  -ms: Small memory model
#  -zl: Emit default library names in the object file to void standard C libraries. 
#  -zq: Quiet mode to supress informational messages
CFLAGS16?=-4 -d3 -s -wx -ms -zl -zq

# Filenames seperated by spaces
SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)

# pattern substitute string: 1st arg= pattern, 2nd= substitution, 3rd Source/files/text
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/stage2/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/stage2/asm/%.obj, $(SOURCES_ASM))

.PHONY: all stage2 clean

all: stage2

stage2: $(BUILD_DIR)/stage2.bin

# Linking all object files and getting final executable
# NAME = Output filename
# FILE = Object files to link 
# OPTION MAP = Generate map file for debugging.
$(BUILD_DIR)/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(LD16) NAME $(BUILD_DIR)/stage2.bin FILE \{ $(OBJECTS_ASM) $(OBJECTS_C) \} OPTION MAP=$(BUILD_DIR)/stage2.map @linker.lnk 

# Cross compiling %.c for real mode.
$(BUILD_DIR)/stage2/c/%.obj: %.c always
	$(CC16) $(CFLAGS16) -fo=$@ $<

# Compiling the assembly files.
$(BUILD_DIR)/stage2/asm/%.obj: %.asm always
	$(ASM) $(ASMFLAGS) -o $@ $<

always: 
	mkdir -p $(BUILD_DIR)/stage2/asm
	mkdir -p $(BUILD_DIR)/stage2/c

clean: 
	rm -f $(BUILD_DIR)/stage2.bin

# $@ : file name of the target
# $< : name of first dependency
