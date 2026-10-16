A hobby x86 operating system written from scratch in **C and x86 Assembly**, focused on learning operating-system and low-level systems development.

### Current Progress

* BIOS/FAT12 bootloader
* WCC cross compiler setup
* A20 line and GDT setup
* Transition to 32-bit protected mode
* ELF kernel loading
* `i686-elf` cross-compiler
* VGA and serial port debugging setup
* CPU exception handling for all major faults
* Handling keyboard and timer interrupts
* Physical memory management using bitmap alloation technique
* Virtual memory management and paging
* Higher Half Kernel on progress

### Build 

Requires:
* NASM
* wcc (Real Mode cross compiler) 
* `i686-elf-gcc` (Protected Mode cross compiler)
* QEMU / Bochs


### Resources
* [OSDev Wiki](https://wiki.osdev.org/)
* [BrokenThorn OS Development](http://www.brokenthorn.com/Resources/OSDevIndex.html)
* [Nanobyte OS Development Tutorials](https://www.youtube.com/@nanobyte)
