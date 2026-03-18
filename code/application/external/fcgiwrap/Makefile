# FCGIWRAP Makefile
# Moved from parent external/Makefile

include $(NVT_PRJCFG_MODEL_CFG)

MULTI_CORES ?= $(shell grep -c ^processor /proc/cpuinfo)
EXTERNAL := ${APP_DIR}/external
INSTALL_DIR = __install
FCGIWRAP := fcgiwrap-1.1.0

# dependencies
LIB_FCGI2 := fcgi2-2.4.6

unexport CC
unexport CPP
unexport CXX
unexport LD
unexport AR
unexport NM
unexport STRIP
unexport OBJCOPY
unexport OBJDUMP
unexport RANLIB
unexport MAKE

AR="${CROSS_COMPILE}ar"
CC="${CROSS_COMPILE}gcc"
CXX="${CROSS_COMPILE}g++"
CPP="${CROSS_COMPILE}gcc"
RANLIB="${CROSS_COMPILE}ranlib"
export PKG_CONFIG_PATH=${SYSROOT_PATH}/usr/lib/pkgconfig
export PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH}

check_exist = if [ ! -e $(1)/.nvt_finish ]; then rm -rf $(1); tar -jxvf $(1).tar.bz2; fi

.PHONY: all clean dependencies fcgiwrap fcgi2 init
all: dependencies fcgiwrap

dependencies: init fcgi2

init:
	@if [ ! -e "${EXTERNAL}/${INSTALL_DIR}" ]; then \
		mkdir -p ${EXTERNAL}/${INSTALL_DIR}; \
	fi
	@if [ -z "${CROSS_COMPILE}" ]; then \
		echo "Please execute \"source build/envsetup.sh\" firstly.  Stop."; exit 1; \
	fi

fcgi2: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f dependencies/${LIB_FCGI2}/.nvt_finish ]; then \
		cd dependencies; \
		$(call check_exist, ${LIB_FCGI2}); \
		cd ${LIB_FCGI2}; \
		if [ -f autogen.sh ]; then ./autogen.sh; fi; \
		CPPFLAGS="-I${INCLUDE_DIR}" \
		LDFLAGS="" \
		./configure --host=${NVT_HOST} --prefix=${EXTERNAL}/fcgiwrap/dependencies/${LIB_FCGI2}/install --enable-static --disable-shared; \
		make CC=${CC} -j$(MULTI_CORES); \
		make install; \
	fi
	@touch dependencies/${LIB_FCGI2}/.nvt_finish

fcgiwrap: fcgi2
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${FCGIWRAP}/.nvt_finish ]; then \
		$(call check_exist, ${FCGIWRAP}); \
		cd ${FCGIWRAP}; \
		autoreconf -i; \
		CPPFLAGS="-I${EXTERNAL}/fcgiwrap/dependencies/${LIB_FCGI2}/install/include -I${INCLUDE_DIR}" \
		LDFLAGS="-L${EXTERNAL}/fcgiwrap/dependencies/${LIB_FCGI2}/install/lib" \
		ac_cv_func_malloc_0_nonnull=yes \
		./configure --host=${NVT_HOST} --prefix=/usr; \
		make CC=${CC} CFLAGS="-Wno-error" CPPFLAGS="-I${EXTERNAL}/fcgiwrap/dependencies/${LIB_FCGI2}/install/include -I${INCLUDE_DIR}" LDFLAGS="-L${EXTERNAL}/fcgiwrap/dependencies/${LIB_FCGI2}/install/lib" -j$(MULTI_CORES); \
	fi
	@cd ${FCGIWRAP}; make install DESTDIR=${EXTERNAL}/${INSTALL_DIR}
	@touch ${FCGIWRAP}/.nvt_finish

clean:
	@echo ">>>>>>>>>>>>>>>>>>> Remove fcgiwrap >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${FCGIWRAP}
	@echo ">>>>>>>>>>>>>>>>>>> Remove fcgi2 >>>>>>>>>>>>>>>>>>>"
	@rm -rf dependencies/${LIB_FCGI2}
	@echo ">>>>>>>>>>>>>>>>>>> Remove nvt_finish files >>>>>>>>>>>>>>>>>>>"
	@rm -f dependencies/.nvt_finish
	@rm -f .nvt_finish
