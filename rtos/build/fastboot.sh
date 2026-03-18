#/bin/sh
# cfg_530_RTOS_BOOT_LINUX_EVB
# cfg_530_RTOS_BOOT_LINUX_NOR_EVB
# cfg_530_RTOS_BOOT_LINUX_EMMC_EVB
CFG_MODEL=cfg_530_RTOS_BOOT_LINUX_EVB
LINUX_TOOLCHAIN=aarch64-ca53-linux-gnueabihf-8.4.01
RTOS_TOOLCHAIN=aarch64-ca53-elf-newlibc-8.5

function build_rtos() {
	cd rtos && \
	source build/envsetup.sh && \
	lunch rtos $CFG_MODEL $RTOS_TOOLCHAIN && \
	make && \
	cp output/application.bin output/rtos-main.bin ../. && \
	cd .. && \
	clear_nvt_environment
}

function clean_rtos() {
	cd rtos && \
	source build/envsetup.sh && \
	make clean && \
	rm -f ../application.bin ../rtos-main.bin && \
	cd .. && \
	clear_nvt_environment
}

function build_linux() {
	source build/envsetup.sh && \
	lunch Linux $CFG_MODEL $LINUX_TOOLCHAIN && \
	make && \
	clear_nvt_environment
}

function clean_linux() {
	source build/envsetup.sh && \
	make clean && \
	clear_nvt_environment
}

function build_all() {
	build_rtos
	if [ $? -ne 0 ]; then exit -1; fi;
	build_linux
}

function clean_all() {
	clean_rtos
	if [ $? -ne 0 ]; then exit -1; fi;
	clean_linux
}

case $1 in
	build)
		build_all
		;;
	build_linux)
		build_linux
		;;
	build_rtos)
		build_rtos
		;;
	clean)
		clean_all
		;;
	clean_linux)
		clean_linux
		;;
	clean_rtos)
		clean_rtos
		;;
	*)
		echo "unknown command"
		;;
esac
