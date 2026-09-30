#!/usr/bin/env fish

# Don't let Ctrl+C kill the script itself.
trap '' INT

make -s

qemu-system-i386 \
    -s -S \
    -fda build/main_floppy.img \
    -serial file:serial.log \
    -display gtk &

set QEMU_PID $last_pid
# set QEMU_PID (jobs -p | tail -n 1)

echo "QEMU PID = $QEMU_PID"

sleep 0.5

gdb build/kernel.elf

kill -9 $QEMU_PID 2>/dev/null
