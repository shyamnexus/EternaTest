MULTI_CORES ?= $(shell grep -c ^processor /proc/cpuinfo)
TOOLS = ${PWD}
SHELL = /bin/bash
INSTALL_DIR = __install

ZLIB := zlib-1.2.8
LZO := lzo-2.08
E2FSPROGS := e2fsprogs-1.44.6
MTD_UTILS := mtd-utils-2.0.2
MEMTESTER := memtester-4.5.1
BONNIE := bonnie++-1.03e
ETHTOOL := ethtool-5.4
GDB := gdb-8.3
TERMCAP := termcap-1.3.1
NCURSES := ncurses-6.1
HTOP := htop-3.2.0
PROCPS := procps-3.2.8
SDCARD_TEST := sdcard_test.sh
I2C_TOOL := i2c-tools-3.1.2
STRESS := stress-1.0.4
STRESSNG := stress-ng-0.16.02
IOZONE := iozone-3.469
NETSTRESS := netstress_1.2.0
STRACE := strace-5.14
OPROFILE := oprofile-0.9.9
GATOR := gator
VDBENCH := VDBench
PHYTOOL := phytool
LIBPCAP := libpcap-1.9.1
TCPDUMP := tcpdump-4.9.3
DOSFSTOOLS := dosfstools-4.1
FIO := fio-3.20
SYSSTAT := sysstat-12.5.2
PTP4L := linuxptp-3.1
PTPD:= ptpd-ptpd.2.3.1
LLCBENCH := llcbench
LIBPNG := libpng-1.6.37
FBGRAB := fbgrab
VALGRIND := valgrind-3.17.0
LIBUUID := libuuid-1.0.3
NUMACTL := numactl-2.0.14
RT_TEST := rt-tests-2.4
LINPACK := linpack
USERSPACE_RCU := userspace-rcu-0.9.6
POPT := popt-1.19
XML2 := libxml2-2.9.3
LTTNG_UST := lttng-ust-2.10.3
LTTNG_TOOLS := lttng-tools-2.10.6
LTTNG_MODULES := lttng-modules-2.12.11
GPERFTOOLS := gperftools
MMC_UTILS := mmc-utils-master
unexport CC
unexport CPP
unexport CXX
unexport LD
unexport AR
unexport NM
unexport STRIP
unexport OBJCOPY
unexport OBJDUMP

CC="${CROSS_COMPILE}gcc"
CXX="${CROSS_COMPILE}g++"
CPP="${CROSS_COMPILE}gcc"

check_exist = if [ ! -e $(1) ]; then tar -jxvf $(1).tar.bz2; fi
check_exist_cp = if [ -e $(INSTALL_DIR)/$(1) ]; then mkdir -p ${ROOTFS_DIR}/rootfs/$(1); cp -arfv $(INSTALL_DIR)/$(1)/* ${ROOTFS_DIR}/rootfs/$(1); fi

.PHONY: mtd-utils bonnie ethtool iozone gdb htop procps memtester
all: memtester bonnie ethtool iozone htop gdb procps e2fsprogs mtd-utils

init:
	@if [ ! -e "${TOOLS}/${INSTALL_DIR}" ]; then \
		mkdir ${TOOLS}/${INSTALL_DIR}; \
	fi
	@if [ -z "${CROSS_COMPILE}" ]; then \
		echo "Please execute \"source build/envsetup.sh\" firstly.  Stop."; exit 1; \
	fi

zlib: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${ZLIB})
	@cd ${ZLIB}; prefix=${TOOLS}/${INSTALL_DIR} CC=${CC} CFLAGS="-O4" ./configure; make; make install;

lzo: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LZO})
	@cd ${LZO}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} CC=${CC} --host=${NVT_HOST}; make; make install;

e2fsprogs: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${E2FSPROGS})
	@cd ${E2FSPROGS}; patch -p1 < misc-e4defrag.c-use-posix_fallocate64-if-fallocate64-is-unavailable; ./configure --prefix=${TOOLS}/${INSTALL_DIR} CC=${CC} --host=${NVT_HOST}; make -j$(MULTI_CORES); make install;

mtd-utils: zlib lzo libuuid
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${MTD_UTILS})
	cd ${MTD_UTILS}; \
	CFLAGS=-I${TOOLS}/${INSTALL_DIR}/include CPPFLAGS=-I${TOOLS}/${INSTALL_DIR}/include LDFLAGS=-L${TOOLS}/${INSTALL_DIR}/lib ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR} --without-xattr; \
	make; make install;

memtester: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${MEMTESTER})
	@cd ${MEMTESTER}; echo "${CC} -O2 -DPOSIX -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64 -DTEST_NARROW_WRITES -c" > conf-cc; echo "${CC} -s" > conf-ld; make;
	@cd ${MEMTESTER}; make; make install INSTALLPATH=${TOOLS}/${INSTALL_DIR}

bonnie: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${BONNIE})
	@cd ${BONNIE}; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make; mkdir -p ${TOOLS}/${INSTALL_DIR}/sbin/; cp bonnie++ zcav ${TOOLS}/${INSTALL_DIR}/sbin/;

ethtool: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${ETHTOOL})
	@cd ${ETHTOOL}; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make; make install;

gdb: termcap ncurses
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${GDB})
	@cd ${GDB}; ./configure LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib" CXXFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR} --without-x --disable-gdbtk --disable-tui --without-included-regex --without-included-gettext; make -j$(MULTI_CORES); make install

termcap: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${TERMCAP})
	@cd ${TERMCAP}; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make -j$(MULTI_CORES); make install;

ncurses: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${NCURSES})
	@cd ${NCURSES}; ./configure LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib" CXXFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make -j$(MULTI_CORES); make install.libs install.data install.includes install.ncurses;

htop: ncurses
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${HTOP})
	@cd ${HTOP}; ./configure --host=$(NVT_HOST) --prefix=${TOOLS}/${INSTALL_DIR} --disable-unicode LDFLAGS="-static -L${TOOLS}/${INSTALL_DIR}/lib" CPPFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include"; make; make install;

procps: ncurses
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${PROCPS})
	@cd ${PROCPS}; make lib64=lib SHARED=0 CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include/ncurses -I${TOOLS}/${INSTALL_DIR}/include" \
			DESTDIR=${TOOLS}/${INSTALL_DIR} CC=${CC} LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib" LIBPROC="proc/libproc.a" install="install -D" install;

mmctest: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/; cp ${SDCARD_TEST} ${TOOLS}/${INSTALL_DIR}/usr/bin/

i2c_tool: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${I2C_TOOL})
	@cd ${I2C_TOOL}; CC=${CC} make; prefix=${TOOLS}/${INSTALL_DIR}/usr make install;

stress: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${STRESS})
	@cd ${STRESS}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST}; make; make install;

stress-ng: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${STRESSNG})
	cd ${STRESSNG}; make CC=${CC} HAVE_LIB_AIO=0 HAVE_SYS_CAP_H=0 HAVE_FLOAT_DECIMAL=0 HAVE_SECCOMP_H=0 HAVE_LIB_Z=0 DESTDIR=${TOOLS}/${INSTALL_DIR} install;

strace: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${STRACE})
	@cd ${STRACE}; ./configure --host=${NVT_HOST} --target=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR} --enable-mpers=no; make; make install;

iozone: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${IOZONE})
	@cd ${IOZONE}; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make linux CC=${CC}; mkdir -p ${TOOLS}/${INSTALL_DIR}/sbin/; cp iozone fileop pit_server ${TOOLS}/${INSTALL_DIR}/sbin/;

netstress: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${NETSTRESS})
	@cd ${NETSTRESS}; mkdir -p ${TOOLS}/${INSTALL_DIR}/bin/; ${CC} -o ${TOOLS}/${INSTALL_DIR}/bin/$@ netstress.c

oprofile: init 
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${OPROFILE})
	@cd ${OPROFILE}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST} --with-kernel=${OUTPUT_DIR}; make -j${MULTI_CORES}; make install

gator: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${GATOR})
	cd ${GATOR}/driver; make -C ${KERNELDIR} M=${PWD}/${GATOR}/driver ARCH=arm CROSS_COMPILE=${CROSS_COMPILE} modules; cp gator.ko ${TOOLS}/${INSTALL_DIR}
	cd ${GATOR}/daemon; make CROSS_COMPILE=${CROSS_COMPILE} ;cp gatord ${TOOLS}/${INSTALL_DIR}
	cp ${CROSS_TOOLCHAIN_PATH}/target/lib/libgcc_s.so.1 ${ROOTFS_DIR}/rootfs/lib

VDBench: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${VDBENCH})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${VDBENCH}; make CFLAG="-I${SYSROOT_PATH}/usr/include/tirpc" LDRPC="-ltirpc" -j${MULTI_CORES}; rm Bins/*.o; cp Bins/* ${TOOLS}/${INSTALL_DIR}/usr/bin/; cp -R Scripts ${TOOLS}/${INSTALL_DIR}/usr/bin/; cp VDBench.sh ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${VDBENCH}/ThirdBenches/coremark_v1.0; make -j${MULTI_CORES}
	@make install

phytool: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${PHYTOOL})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${PHYTOOL}; make CC=${CC} -j${MULTI_CORES}; rm *.o; cp phytool ${TOOLS}/${INSTALL_DIR}/usr/bin/

libpcap: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LIBPCAP})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${LIBPCAP}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST}; make -j${MULTI_CORES}; make install; rm *.o

tcpdump: libpcap
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${TCPDUMP})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${TCPDUMP}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST}; make -j${MULTI_CORES}; make install; rm *.o

dosfstools: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${DOSFSTOOLS})
	@cd ${DOSFSTOOLS}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST} --without-udev; make -j${MULTI_CORES}; make install

fio: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${FIO})
	@cd ${FIO}; ./configure --build-static  --prefix=${TOOLS}/${INSTALL_DIR}/fio; make; make install

sysstat: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${SYSSTAT})
	@cd ${SYSSTAT}; ./configure --host=${NVT_HOST} --disable-documentation; make -j${MULTI_CORES}; make DESTDIR=${TOOLS}/${INSTALL_DIR} install;

ptp4l: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${PTP4L})
	@cd ${PTP4L}; make -j$(MULTI_CORES); make DESTDIR=${TOOLS}/${INSTALL_DIR} install;

ptpd: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${PTPD})
	@cd ${PTPD}; autoreconf -vi; ./configure --prefix=${TOOLS}/${INSTALL_DIR} CC=${CC} --host=${NVT_HOST}; export ac_cv_func_malloc_0_nonnull=yes; make -j$(MULTI_CORES); make install;

llcbench: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LLCBENCH})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${LLCBENCH}; make cache-bench; make install;

libpng: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	if [ ! -f ${LIBPNG}/.nvt_finish ]; then \
		$(call check_exist, ${LIBPNG}); \
		cd ${LIBPNG}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST} CC=$(CC) CXX=$(CXX) CPP=${CROSS_COMPILE}cpp; \
		make -j$(MULTI_CORES); make install; \
		fi
	@touch ${LIBPNG}/.nvt_finish

fbgrab: init libpng
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	if [ ! -f ${FBGRAB}/.nvt_finish ]; then \
		$(call check_exist, ${FBGRAB}); \
		cd ${FBGRAB}; make LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib" CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" CC=$(CC); make DESTDIR=${TOOLS}/${INSTALL_DIR} install; \
		fi
	@touch ${FBGRAB}/.nvt_finish

valgrind: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	if [ ! -f ${VALGRIND}/.nvt_finish ]; then \
		$(call check_exist, ${VALGRIND}); \
		cd ${VALGRIND}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST} CC=$(CC) CXX=$(CXX) CPP=${CROSS_COMPILE}cpp; \
		make -j$(MULTI_CORES); \
		fi
	@cd ${VALGRIND}; make install;
	@touch ${VALGRIND}/.nvt_finish

libuuid: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LIBUUID})
	@cd ${LIBUUID}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST}; make -j${MULTI_CORES}; make install

numactl: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${NUMACTL})
	@cd ${NUMACTL}; ./autogen.sh; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST}; make -j${MULTI_CORES}; make install

rt-test: init numactl
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${RT_TEST})
	@cd ${RT_TEST}; make LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib" CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" -j${MULTI_CORES}; make DESTDIR=${TOOLS}/${INSTALL_DIR} install

linpack: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LINPACK})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${LINPACK}; make CC=${CC} STRIP=$(STRIP) -j${MULTI_CORES};
	@cp -arfv $(LINPACK)/linpack ${TOOLS}/${INSTALL_DIR}/usr/bin

userspace_rcu: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${USERSPACE_RCU})
	@cd ${USERSPACE_RCU}; libtoolize --force; aclocal; autoheader; autoconf; automake --force-missing --add-missing; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make -j6; make install

popt: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${POPT})
	@cd ${POPT}; ./configure --host=$(NVT_HOST) --prefix=${TOOLS}/${INSTALL_DIR}; make -j6; make install

libxml2: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${XML2}); \
	cd ${XML2}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} CC=${CC} CXX=${CXX} AR=${AR} --host=${NVT_HOST} --without-python --without-lzma --target=${NVT_HOST}; make -j${MULTI_CORES}; \
	@cd ${XML2}; make install; cd -;
	@rm -rf ${TOOLS}/${INSTALL_DIR}/lib/cmake ${TOOLS}/${INSTALL_DIR}/lib/xml2Conf.sh ${TOOLS}/${INSTALL_DIR}/lib/pkgconfig

lttng_ust: init popt libxml2 userspace_rcu e2fsprogs
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LTTNG_UST})
	@cd ${LTTNG_UST}; CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" CPPFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib -Wl,-rpath-link=${TOOLS}/$(INSTALL_DIR)/lib" ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR};
	@cd ${LTTNG_UST}; make -j${MULTI_CORES}; make install;

lttng_tools: init lttng_ust
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LTTNG_TOOLS})
	@cd ${LTTNG_TOOLS}; libxml2_CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include/libxml2" CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" CPPFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib -Wl,-rpath-link=${TOOLS}/$(INSTALL_DIR)/lib" ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR};
	@cd ${LTTNG_TOOLS}; make -j${MULTI_CORES}; make install;

lttng_necessary_libs: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/etc; cp lttng.sh ${TOOLS}/${INSTALL_DIR}/etc;
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/lib; mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/lib;
	@cp -arfv ${TOOLS}/$(INSTALL_DIR)/lib/libuuid* ${TOOLS}/${INSTALL_DIR}/usr/lib;
	@cp -arfv ${CROSS_TOOLCHAIN_PATH}/${NVT_HOST}/sysroot/lib/libdl* ${TOOLS}/${INSTALL_DIR}/lib;
	@cp -arfv ${CROSS_TOOLCHAIN_PATH}/lib/libz* ${TOOLS}/${INSTALL_DIR}/usr/lib;

lttng_modules: init lttng_tools libuuid lttng_necessary_libs
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LTTNG_MODULES})
	@cd ${LTTNG_MODULES}; make -j${MULTI_CORES}; make modules_install INSTALL_MOD_PATH=${TOOLS}/${INSTALL_DIR}

gperftools: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	if [ ! -f ${GPERFTOOLS}/.nvt_finish ]; then \
		$(call check_exist, ${GPERFTOOLS}); \
		cd ${GPERFTOOLS}; \
		./autogen.sh --prefix=${INSTALL_DIR} CC=${CC} CXX=${CXX} AR=${AR} --host=${NVT_HOST} --target=${NVT_HOST}; \
		./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST} CC=$(CC) CXX=$(CXX) CPP=${CROSS_COMPILE}cpp; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${GPERFTOOLS}; make install;
	@touch ${GPERFTOOLS}/.nvt_finish

mmc: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${MMC_UTILS})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${MMC_UTILS}; make CC=${CC} -j${MULTI_CORES}; rm *.o; cp mmc ${TOOLS}/${INSTALL_DIR}/usr/bin/

install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@$(call check_exist_cp,bin)
	@$(call check_exist_cp,sbin)
	@$(call check_exist_cp,usr/bin)
	@$(call check_exist_cp,usr/sbin)
	@$(call check_exist_cp,usr/lib)
	@$(call check_exist_cp,lib)
	@$(call check_exist_cp,usr/local/lib)
	@$(call check_exist_cp,usr/local/bin)
	@$(call check_exist_cp,usr/local/sbin)

clean:
	@echo ">>>>>>>>>>>>>>>>>>> Remove >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${TOOLS}/${INSTALL_DIR}
	@rm -rf ${ZLIB} ${LZO} ${E2FSPROGS} ${MTD_UTILS} ${BONNIE} ${ETHTOOL} ${TERMCAP} ${GDB} ${NCURSES} ${HTOP} ${IPERF} ${PROCPS} ${I2C_TOOL} ${STRESS} ${STRESSNG} ${IOZONE} ${NETSTRESS} ${STRACE} ${MEMTESTER} ${USERSPACE_RCU}
	@rm -rf ${POPT} ${XML2} ${LTTNG_UST} ${LTTNG_TOOLS} ${LTTNG_MODULES} ${OPROFILE} ${GATOR} ${VDBENCH} ${PHYTOOL}
	@rm -rf ${LIBPCAP} ${TCPDUMP} ${DOSFSTOOLS} ${FIO} ${SYSSTAT} ${PTP4L} ${PTPD} ${IPROUTE2} ${FBGRAB} ${LIBPNG} ${VALGRIND} ${LIBUUID} ${RT_TEST} ${NUMACTL} ${LINPACK} ${GPERFTOOLS} ${MMC_UTILS}
