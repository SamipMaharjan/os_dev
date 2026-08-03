sleep 0.5s
make clean
make -s
bochs -q -f bochs_config -dbg
