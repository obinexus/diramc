#!/bin/bash
export LD_PRELOAD=./lib/libdiram.so
export DIRAM_TARGET_LIB=./lib/libpolycall.so

# Run your program
./test_polycall

# Check for core dumps
if [ -f core ]; then
    echo "Program crashed! Analyzing core dump..."
    gdb ./test_polycall core -batch -ex "bt"
fi
