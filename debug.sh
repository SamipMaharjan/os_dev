sleep 0.5s
make clean
make -s
bochs -q -f bochs_config -dbg

# Print the line of error in terminal 
./err_finder.sh
