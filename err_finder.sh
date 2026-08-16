err_addr=$(grep -oP "(?<=ERROR ADDRESS:).*" serial.log)
echo "Err addr: $err_addr"
i686-elf-addr2line -e ./build/kernel.elf -f -C $err_addr 
