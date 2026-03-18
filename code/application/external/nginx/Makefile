# NGINX Makefile
# Moved from parent external/Makefile

include $(NVT_PRJCFG_MODEL_CFG)

MULTI_CORES ?= $(shell grep -c ^processor /proc/cpuinfo)
EXTERNAL := ${APP_DIR}/external
INSTALL_DIR = __install
NGINX := nginx-1.29.0

# dependencies
LIB_FCGI2 := fcgi2-2.4.6
PCRE2 := pcre2-10.45
ZLIB := zlib-1.3.1

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

.PHONY: all clean dependencies
all: dependencies nginx

dependencies: init zlib pcre2 fcgi2

init:
	@if [ ! -e "${EXTERNAL}/${INSTALL_DIR}" ]; then \
		mkdir -p ${EXTERNAL}/${INSTALL_DIR}; \
	fi
	@if [ -z "${CROSS_COMPILE}" ]; then \
		echo "Please execute \"source build/envsetup.sh\" firstly.  Stop."; exit 1; \
	fi

zlib: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f dependencies/${ZLIB}/.nvt_finish ]; then \
		cd dependencies; \
		$(call check_exist, ${ZLIB}); \
		cd ${ZLIB}; \
		CHOST=${NVT_HOST} ./configure --prefix=${EXTERNAL}/nginx/dependencies/${ZLIB}/install --static; \
		make CC=${CC} -j$(MULTI_CORES); \
		make install; \
	fi
	@touch dependencies/${ZLIB}/.nvt_finish

pcre2: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f dependencies/${PCRE2}/.nvt_finish ]; then \
		cd dependencies; \
		$(call check_exist, ${PCRE2}); \
		cd ${PCRE2}; \
		CPPFLAGS="-I${INCLUDE_DIR}" \
		LDFLAGS="" \
		./configure --host=${NVT_HOST} --prefix=${EXTERNAL}/nginx/dependencies/${PCRE2}/install --enable-jit --enable-static --disable-shared; \
		make CC=${CC} -j$(MULTI_CORES); \
		make install; \
	fi
	@touch dependencies/${PCRE2}/.nvt_finish

fcgi2: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f dependencies/${LIB_FCGI2}/.nvt_finish ]; then \
		cd dependencies; \
		$(call check_exist, ${LIB_FCGI2}); \
		cd ${LIB_FCGI2}; \
		if [ -f autogen.sh ]; then ./autogen.sh; fi; \
		CPPFLAGS="-I${INCLUDE_DIR}" \
		LDFLAGS="" \
		./configure --host=${NVT_HOST} --prefix=${EXTERNAL}/nginx/dependencies/${LIB_FCGI2}/install --enable-static --disable-shared; \
		make CC=${CC} -j$(MULTI_CORES); \
		make install; \
	fi
	@touch dependencies/${LIB_FCGI2}/.nvt_finish

nginx: init 
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${NGINX}/.nvt_finish ]; then \
		$(call check_exist, ${NGINX}); \
		cd ${NGINX}; \
		./configure \
		  --prefix=/usr \
		  --with-http_ssl_module \
		  --with-http_gzip_static_module \
		  --with-http_v2_module \
		  --with-pcre \
		  --with-cc-opt="-I${EXTERNAL}/${INSTALL_DIR}/include -I${EXTERNAL}/nginx/dependencies/${PCRE2}/install/include -I${EXTERNAL}/nginx/dependencies/${ZLIB}/install/include -I${EXTERNAL}/nginx/dependencies/${LIB_FCGI2}/install/include -I${INCLUDE_DIR}" \
		  --with-ld-opt="-L${LIBRARY_DIR}/external/__install/lib -L${EXTERNAL}/nginx/dependencies/${PCRE2}/install/lib -L${EXTERNAL}/nginx/dependencies/${ZLIB}/install/lib -L${EXTERNAL}/nginx/dependencies/${LIB_FCGI2}/install/lib"; \
		make CC=${CC} -j$(MULTI_CORES); \
	fi
	@cd ${NGINX}; make install DESTDIR=${EXTERNAL}/${INSTALL_DIR}
	@mkdir -p ${ROOTFS_DIR}/rootfs/usr/logs
	@touch ${ROOTFS_DIR}/rootfs/usr/logs/.gitexist
	@touch ${NGINX}/.nvt_finish

clean:
	@echo ">>>>>>>>>>>>>>>>>>> Remove nginx >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${NGINX}
	@echo ">>>>>>>>>>>>>>>>>>> Remove zlib >>>>>>>>>>>>>>>>>>>"
	@rm -rf dependencies/${ZLIB}
	@echo ">>>>>>>>>>>>>>>>>>> Remove pcre2 >>>>>>>>>>>>>>>>>>>"
	@rm -rf dependencies/${PCRE2}
	@echo ">>>>>>>>>>>>>>>>>>> Remove fcgi2 >>>>>>>>>>>>>>>>>>>"
	@rm -rf dependencies/${LIB_FCGI2}
	@echo ">>>>>>>>>>>>>>>>>>> Remove nvt_finish files >>>>>>>>>>>>>>>>>>>"
	@rm -f dependencies/.nvt_finish
	@rm -f .nvt_finish
