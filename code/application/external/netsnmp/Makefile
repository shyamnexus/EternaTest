# NETSNMP Makefile
# Moved from parent external/Makefile

include $(NVT_PRJCFG_MODEL_CFG)

EXTERNAL := ${APP_DIR}/external
INSTALL_DIR = __install
NETSNMP := net-snmp-5.9.4

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

.PHONY: all clean
all: netsnmp

init:
	@if [ ! -e "${EXTERNAL}/${INSTALL_DIR}" ]; then \
		mkdir -p ${EXTERNAL}/${INSTALL_DIR}; \
	fi
	@if [ -z "${CROSS_COMPILE}" ]; then \
		echo "Please execute \"source build/envsetup.sh\" firstly.  Stop."; exit 1; \
	fi

netsnmp: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${NETSNMP}/.nvt_finish ]; then \
		$(call check_exist, ${NETSNMP}); \
		cd ${NETSNMP}; \
		CPPFLAGS="-I${EXTERNAL}/${INSTALL_DIR}/include -I${INCLUDE_DIR}" \
		LDFLAGS="-L${EXTERNAL}/${INSTALL_DIR}/lib" \
		./configure \
		  --prefix=${EXTERNAL}/${INSTALL_DIR} \
		  --host=${NVT_HOST} \
		  CC=${CC} CXX=${CXX} AR=${AR} \
		  --with-default-snmp-version=3 \
		  --enable-snmpv3 \
		  --without-openssl \
		  --with-logfile=/var/log/snmpd.log \
		  --with-persistent-directory=/var/lib/snmp \
		  --with-sys-contact="root@localhost" \
		  --with-sys-location="Unknown" \
		  --enable-ucd-snmp-compatibility \
		  --enable-agentx-dom-sock-only \
		  --with-security-modules="usm" \
		  --enable-static \
		  --disable-shared \
		  --disable-debugging \
		  --without-libwrap \
		  --without-zlib \
		  --without-rpm \
		  --without-dmalloc \
		  --without-efence \
		  --disable-embedded-perl \
		  --disable-perl-cc-checks \
		  --disable-scripts \
		  --disable-applications \
		  --disable-manuals \
		  --disable-mib-loading \
		  --disable-mibs \
		  --with-endianness=little \
		  --with-gnu-ld; \
		make -j$(MULTI_CORES); \
	fi
	@mkdir -p ${EXTERNAL}/${INSTALL_DIR}/sbin
	@mkdir -p ${EXTERNAL}/${INSTALL_DIR}/etc/snmp
	@cp ${NETSNMP}/agent/snmpd ${EXTERNAL}/${INSTALL_DIR}/sbin/
	@${CROSS_COMPILE}strip ${EXTERNAL}/${INSTALL_DIR}/sbin/snmpd
	@echo "# SNMP Configuration File" > ${EXTERNAL}/${INSTALL_DIR}/etc/snmp/snmpd.conf
	@echo "# Add your SNMP configuration here" >> ${EXTERNAL}/${INSTALL_DIR}/etc/snmp/snmpd.conf
	@if [ -n "${ROOTFS_DIR}" -a -d "${PWD}/env" ]; then \
		mkdir -p ${ROOTFS_DIR}/rootfs/etc/snmp; \
		mkdir -p ${ROOTFS_DIR}/rootfs/var/log; \
		mkdir -p ${ROOTFS_DIR}/rootfs/var/lib/snmp; \
		cp -afv ${PWD}/env/etc/snmp/* ${ROOTFS_DIR}/rootfs/etc/snmp/ 2>/dev/null || true; \
		cp -afv ${PWD}/env/var/log/* ${ROOTFS_DIR}/rootfs/var/log/ 2>/dev/null || true; \
		cp -afv ${PWD}/env/var/lib/snmp/* ${ROOTFS_DIR}/rootfs/var/lib/snmp/ 2>/dev/null || true; \
	fi
	@touch ${NETSNMP}/.nvt_finish

clean:
	@echo ">>>>>>>>>>>>>>>>>>> Remove netsnmp >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${NETSNMP}
	@rm -f .nvt_finish
	@if [ -n "${ROOTFS_DIR}" ]; then \
		if [ -e "${ROOTFS_DIR}/rootfs/etc/snmp" ]; then \
			rm -rf ${ROOTFS_DIR}/rootfs/etc/snmp; \
			echo "removed ${ROOTFS_DIR}/rootfs/etc/snmp"; \
		fi; \
		if [ -e "${ROOTFS_DIR}/rootfs/var/log/snmpd.log" ]; then \
			rm -f ${ROOTFS_DIR}/rootfs/var/log/snmpd.log; \
			echo "removed ${ROOTFS_DIR}/rootfs/var/log/snmpd.log"; \
		fi; \
		if [ -e "${ROOTFS_DIR}/rootfs/var/lib/snmp" ]; then \
			rm -rf ${ROOTFS_DIR}/rootfs/var/lib/snmp; \
			echo "removed ${ROOTFS_DIR}/rootfs/var/lib/snmp"; \
		fi; \
	fi
