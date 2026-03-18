# Avahi mDNS/DNS-SD daemon Makefile
# For network service discovery (.local hostname resolution)

include $(NVT_PRJCFG_MODEL_CFG)

MULTI_CORES ?= $(shell grep -c ^processor /proc/cpuinfo)
EXTERNAL := ${APP_DIR}/external
INSTALL_DIR = __install
AVAHI := avahi-0.8

# Dependencies (expat required for parsing .service XML files)
EXPAT := expat-2.5.0

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
export PKG_CONFIG_PATH=${EXTERNAL}/avahi/dependencies/install/lib/pkgconfig:${SYSROOT_PATH}/usr/lib/pkgconfig
export PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH}

check_exist = if [ ! -e $(1)/.nvt_finish ]; then rm -rf $(1); tar -jxvf $(1).tar.bz2; fi

.PHONY: all clean dependencies install
all: dependencies avahi

dependencies: init expat

init:
	@if [ ! -e "${EXTERNAL}/${INSTALL_DIR}" ]; then \
		mkdir -p ${EXTERNAL}/${INSTALL_DIR}; \
	fi
	@mkdir -p ${EXTERNAL}/avahi/dependencies/install
	@if [ -z "${CROSS_COMPILE}" ]; then \
		echo "Please execute \"source build/envsetup.sh\" firstly.  Stop."; exit 1; \
	fi

expat: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f dependencies/${EXPAT}/.nvt_finish ]; then \
		cd dependencies; \
		$(call check_exist, ${EXPAT}); \
		cd ${EXPAT}; \
		./configure \
		  --host=${NVT_HOST} \
		  --prefix=${EXTERNAL}/avahi/dependencies/install \
		  --enable-static \
		  --disable-shared; \
		make -j$(MULTI_CORES); \
		make install; \
	fi
	@touch dependencies/${EXPAT}/.nvt_finish

avahi: init dependencies
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${AVAHI}/.nvt_finish ]; then \
		$(call check_exist, ${AVAHI}); \
		cd ${AVAHI}; \
		CPPFLAGS="-I${EXTERNAL}/avahi/dependencies/install/include -I${INCLUDE_DIR}" \
		LDFLAGS="-L${EXTERNAL}/avahi/dependencies/install/lib -L${LIBRARY_DIR}" \
		./configure \
		  --host=${NVT_HOST} \
		  --prefix=/usr \
		  --sysconfdir=/etc \
		  --localstatedir=/var \
		  --with-distro=none \
		  --disable-glib \
		  --disable-gobject \
		  --disable-libevent \
		  --disable-qt5 \
		  --disable-gtk \
		  --disable-gtk3 \
		  --disable-dbus \
		  --disable-gdbm \
		  --disable-libdaemon \
		  --disable-python \
		  --disable-pygobject \
		  --disable-python-dbus \
		  --disable-mono \
		  --disable-monodoc \
		  --disable-autoipd \
		  --disable-manpages \
		  --disable-xmltoman \
		  --disable-tests \
		  --disable-compat-libdns_sd \
		  --disable-compat-howl \
		  --enable-static \
		  --disable-shared \
		  --with-xml=expat; \
		make -j$(MULTI_CORES); \
		make DESTDIR=${EXTERNAL}/${INSTALL_DIR} install; \
	fi
	@${CROSS_COMPILE}strip ${EXTERNAL}/${INSTALL_DIR}/usr/sbin/avahi-daemon 2>/dev/null || true
	@touch ${AVAHI}/.nvt_finish

install:
	@echo ">>>>>>>>>>>>>>>>>>> avahi install >>>>>>>>>>>>>>>>>>>"
	@mkdir -p ${ROOTFS_DIR}/rootfs/usr/sbin
	@mkdir -p ${ROOTFS_DIR}/rootfs/etc/avahi/services
	@mkdir -p ${ROOTFS_DIR}/rootfs/etc/init.d
	@# Install avahi-daemon binary
	@if [ -f ${EXTERNAL}/${INSTALL_DIR}/usr/sbin/avahi-daemon ]; then \
		cp -f ${EXTERNAL}/${INSTALL_DIR}/usr/sbin/avahi-daemon ${ROOTFS_DIR}/rootfs/usr/sbin/; \
	fi
	@# Install init script (generates service XML at boot)
	@cp -f ${EXTERNAL}/avahi/S50avahi ${ROOTFS_DIR}/rootfs/etc/init.d/
	@chmod 755 ${ROOTFS_DIR}/rootfs/etc/init.d/S50avahi
	@# Create minimal avahi-daemon.conf
	@echo "[server]" > ${ROOTFS_DIR}/rootfs/etc/avahi/avahi-daemon.conf
	@echo "use-ipv4=yes" >> ${ROOTFS_DIR}/rootfs/etc/avahi/avahi-daemon.conf
	@echo "use-ipv6=no" >> ${ROOTFS_DIR}/rootfs/etc/avahi/avahi-daemon.conf
	@echo "allow-interfaces=eth0,wlan0" >> ${ROOTFS_DIR}/rootfs/etc/avahi/avahi-daemon.conf
	@echo "[publish]" >> ${ROOTFS_DIR}/rootfs/etc/avahi/avahi-daemon.conf
	@echo "publish-addresses=yes" >> ${ROOTFS_DIR}/rootfs/etc/avahi/avahi-daemon.conf
	@echo "publish-hinfo=no" >> ${ROOTFS_DIR}/rootfs/etc/avahi/avahi-daemon.conf
	@echo "publish-workstation=no" >> ${ROOTFS_DIR}/rootfs/etc/avahi/avahi-daemon.conf

clean:
	@echo ">>>>>>>>>>>>>>>>>>> avahi clean >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${AVAHI} dependencies/${EXPAT}
	@rm -rf dependencies/install
