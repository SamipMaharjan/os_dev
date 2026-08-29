# sleep 0.5s
# make -s
# qemu-system-i386 -s -S -fda build/main_floppy.img -serial file:serial.log -display gtk
# sleep 0.5s
# gdb build/kernel.elf

#!/usr/bin/env fish
make -s

qemu-system-i386 -s -S -fda build/main_floppy.img -serial file:serial.log -display gtk &
set QEMU_PID $last_pid

sleep 0.5

gdb build/kernel.elf

kill $QEMU_PID 2>/dev/null
