#!/bin/sh
cat nvt-fastboot-etc.dts \
nvt-fastboot-hdal-mem.dtsi \
nvt-fastboot-ipp.dtsi \
nvt-fastboot-sie.dtsi \
nvt-fastboot-venc.dtsi \
nvt-fastboot-acap.dtsi \
nvt-fastboot-rtos-ai.dtsi \
nvt-fastboot-sen_os04c10_240FPS.dtsi \
nvt-fastboot-ae-os04c10.dtsi \
nvt-fastboot-awb-os04c10.dtsi \
nvt-fastboot-iq-os04c10.dtsi \
nvt-fastboot-front-iq.dtsi \
> nvt-fastboot.dts.tmp 
dtc -@ -O dtb -b 0 -W no-unit_address_format -W no-unit_address_vs_reg -o nvt-fastboot.dtb nvt-fastboot.dts.tmp 
