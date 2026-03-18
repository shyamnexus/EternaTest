# OPENSSH Makefile
# Following the pattern of nginx and netsnmp

include $(NVT_PRJCFG_MODEL_CFG)

MULTI_CORES ?= $(shell grep -c ^processor /proc/cpuinfo)
EXTERNAL := ${APP_DIR}/external
INSTALL_DIR = __install
OPENSSH := openssh-10.2

# dependencies
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
all: dependencies openssh

dependencies: init zlib

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
		CHOST=${NVT_HOST} ./configure --prefix=${EXTERNAL}/openssh/dependencies/${ZLIB}/install --static; \
		make CC=${CC} -j$(MULTI_CORES); \
		make install; \
	fi
	@touch dependencies/${ZLIB}/.nvt_finish

openssh: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${OPENSSH}/.nvt_finish ]; then \
		$(call check_exist, ${OPENSSH}); \
		cd ${OPENSSH}; \
		CPPFLAGS="-I${LIBRARY_DIR}/include -I${EXTERNAL}/${INSTALL_DIR}/include -I${EXTERNAL}/openssh/dependencies/${ZLIB}/install/include -I${INCLUDE_DIR}" \
		LDFLAGS="-L${LIBRARY_DIR} -L${LIBRARY_DIR}/output -L${EXTERNAL}/${INSTALL_DIR}/lib -L${LIBRARY_DIR}/external/__install/lib -L${EXTERNAL}/openssh/dependencies/${ZLIB}/install/lib -Wl,--gc-sections" \
		LIBS="-lcrypto -lz" \
		./configure \
		  --prefix=/usr \
		  --sysconfdir=/etc/ssh \
		  --libexecdir=/usr/libexec \
		  --host=${NVT_HOST} \
		  --with-zlib=${EXTERNAL}/openssh/dependencies/${ZLIB}/install \
		  --with-ssl-dir=${LIBRARY_DIR} \
		  --with-privsep-path=/var/empty \
		  --with-privsep-user=sshd \
		  --with-pid-dir=/var/run \
		  --disable-strip \
		  --disable-lastlog \
		  --disable-utmp \
		  --disable-utmpx \
		  --disable-wtmp \
		  --disable-wtmpx \
		  --disable-pututline \
		  --disable-pututxline \
		  --without-pam \
		  --without-shadow \
		  --with-sandbox=no \
		  --with-hardening \
		  --with-selinux=no \
		  --with-kerberos5=no \
		  --disable-etc-default-login \
		  --without-rpath \
		  --without-zlib-version-check \
		  --without-ssh1 \
		  CFLAGS="-Os -ffunction-sections -fdata-sections"; \
		make CC=${CC} -j$(MULTI_CORES); \
	fi
	@cd ${OPENSSH}; make DESTDIR=${EXTERNAL}/${INSTALL_DIR} install-nokeys
	@mkdir -p ${EXTERNAL}/${INSTALL_DIR}/etc/ssh
	@mkdir -p ${EXTERNAL}/${INSTALL_DIR}/var/empty
	@mkdir -p ${EXTERNAL}/${INSTALL_DIR}/var/run
	@mkdir -p ${EXTERNAL}/${INSTALL_DIR}/etc/init.d
	@mkdir -p ${EXTERNAL}/${INSTALL_DIR}/usr/libexec
	@if [ -f sshd_config ]; then \
		cp sshd_config ${EXTERNAL}/${INSTALL_DIR}/etc/ssh/; \
	fi
	@install -m 755 S50sshd ${EXTERNAL}/${INSTALL_DIR}/etc/init.d/
	@echo "Removing unnecessary binaries for IP camera..."
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/usr/bin/ssh 2>/dev/null || true
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/usr/bin/ssh-add 2>/dev/null || true
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/usr/bin/ssh-agent 2>/dev/null || true
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/usr/bin/ssh-keyscan 2>/dev/null || true
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/usr/bin/scp 2>/dev/null || true
	@echo "Keeping ssh-keygen for host key generation"
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/usr/libexec/ssh-pkcs11-helper 2>/dev/null || true
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/usr/libexec/ssh-sk-helper 2>/dev/null || true
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/usr/libexec/ssh-keysign 2>/dev/null || true
	@echo "Removing man pages..."
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/usr/share/man 2>/dev/null || true
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/usr/share 2>/dev/null || true
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/etc/ssh/ssh_config 2>/dev/null || true
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/etc/ssh/moduli 2>/dev/null || true
	@echo "Stripping binaries..."
	@${CROSS_COMPILE}strip ${EXTERNAL}/${INSTALL_DIR}/usr/bin/sftp 2>/dev/null || true
	@${CROSS_COMPILE}strip ${EXTERNAL}/${INSTALL_DIR}/usr/sbin/sshd 2>/dev/null || true
	@${CROSS_COMPILE}strip ${EXTERNAL}/${INSTALL_DIR}/usr/libexec/sftp-server 2>/dev/null || true
	@${CROSS_COMPILE}strip ${EXTERNAL}/${INSTALL_DIR}/usr/libexec/sshd-auth 2>/dev/null || true
	@${CROSS_COMPILE}strip ${EXTERNAL}/${INSTALL_DIR}/usr/libexec/sshd-session 2>/dev/null || true
	@echo "OpenSSH build complete for IP camera (minimal: sshd + sftp only)"
	@echo "Total binaries: sshd, sftp, sftp-server, sshd-auth, sshd-session"
	@# Manually install usr/libexec to rootfs since parent Makefile doesn't handle it
	@mkdir -p ${ROOTFS_DIR}/rootfs/usr/libexec
	@cp -af ${EXTERNAL}/${INSTALL_DIR}/usr/libexec/* ${ROOTFS_DIR}/rootfs/usr/libexec/
	@touch ${OPENSSH}/.nvt_finish


clean:
	@echo ">>>>>>>>>>>>>>>>>>> Remove openssh >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${OPENSSH}
	@echo ">>>>>>>>>>>>>>>>>>> Remove zlib >>>>>>>>>>>>>>>>>>>"
	@rm -rf dependencies/${ZLIB}
	@echo ">>>>>>>>>>>>>>>>>>> Remove installed artifacts >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/etc/ssh
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/var/empty
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/var/run/sshd
	@rm -f ${EXTERNAL}/${INSTALL_DIR}/etc/init.d/S50sshd
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/usr/libexec
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/usr/sbin/sshd
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/usr/bin/ssh*
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/usr/bin/scp
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/usr/bin/sftp
	@# Clean up legacy paths if they exist
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/libexec
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/sbin/sshd
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/bin/ssh*
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/bin/scp
	@rm -rf ${EXTERNAL}/${INSTALL_DIR}/bin/sftp
	@echo ">>>>>>>>>>>>>>>>>>> Remove nvt_finish files >>>>>>>>>>>>>>>>>>>"
	@rm -f dependencies/.nvt_finish
	@rm -f .nvt_finish
