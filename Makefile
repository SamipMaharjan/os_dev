ASM=nasm
CC=gcc
CC16=/opt/watcom/binl/wcc
LD16=/opt/watcom/binl/wlink

SRC_DIR=src
TOOLS_DIR=tools
BUILD_DIR=build


# initizlized phony targets phony targets are always executed even if: 
# 1. there is a filed named exactly like the target
# 2. even if the file is already ready/exists. 
.PHONY: all floppy_image kernel bootloader clean always tools_fat

all: floppy_image tools_fat 
#
# Floppy image
#
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	# generates 1.44MB of zeroes in main_floppy.img file.
	dd if=/dev/zero of=$(BUILD_DIR)/main_floppy.img bs=512 count=2880 
	
	# this makes main_floppy.img's filesystem a fat12 type with SAMIPOS as its volume name
	mkfs.fat -F 12 -n "SAMIPOS" $(BUILD_DIR)/main_floppy.img 

	# copies content of input file to output file with 'no truncate' as option so that the file size is preserved
	dd if=$(BUILD_DIR)/stage1.bin of=$(BUILD_DIR)/main_floppy.img conv=notrunc 

	#  mcopy: tool to copy to/from fat file systems without mounting them.
	#  -i: tells to treat file as an image.
	#  kernel.bin is the input file.
	#  :: says it to copy it in the root directory section of the fat fs.
	mcopy -i $(BUILD_DIR)/main_floppy.img $(BUILD_DIR)/stage2.bin "::stage2.bin"
	mcopy -i $(BUILD_DIR)/main_floppy.img $(BUILD_DIR)/kernel.bin "::kernel.bin"
	# for testing
	mcopy -i $(BUILD_DIR)/main_floppy.img test.txt "::test.txt"


#
# Bootloader
#
# bootloader: $(BUILD_DIR)/bootloader.bin

bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always 
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))


stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always 
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.bin


$(BUILD_DIR)/kernel.bin: always
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Tools
#
tools_fat: $(BUILD_DIR)/tools/fat

$(BUILD_DIR)/tools/fat: always $(TOOLS_DIR)/fat/fat.c
	mkdir -p $(BUILD_DIR)/tools
	$(CC) -g -o $(BUILD_DIR)/tools/fat $(TOOLS_DIR)/fat/fat.c

#
# Always
#
always: 
	mkdir -p $(BUILD_DIR)

#
# Clean
#
clean: 
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	rm -rf $(BUILD_DIR)/*
