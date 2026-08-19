sleep 0.5s
make -s
qemu-system-i386 -fda build/main_floppy.img -serial file:serial.log -display gtk
