#!/bin/bash

if test "$#" -ne 1; then
	echo "Usage: > $0 YOUR_DESKTOP_IP_ADDR"
	exit
fi

echo "target remote $1:3333"
/opt/arm/gcc-linaro-arm-none-eabi-4.9-2014.09_linux/bin/arm-none-eabi-gdb -ex "target remote $1:3333" ./code/application/source/test/output/rtos-main.img