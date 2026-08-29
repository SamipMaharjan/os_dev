# while true 
# do
# tail -f serial.log
cat serial.log
echo -e "\n\n\n===============ERROR DETAILS============"
err_addr=$(grep -oP "(?<=ERROR ADDRESS:).*" serial.log)
i686-elf-addr2line -e ./build/kernel.elf -f -C $err_addr 
# sleep 1s
#  one 
