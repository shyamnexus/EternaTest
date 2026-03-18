include $(NVT_PRJCFG_MODEL_CFG)

EXTLIB = ${LIBRARY_DIR}/external
SHELL = /bin/bash
INSTALL_DIR =${EXTLIB}/__install
#MAKEFILE_LIST = freetype libnl openssl
# openssl for tee
MULTI_CORES ?= $(shell grep -c ^processor /proc/cpuinfo)
MAKEFILE_LIST = openssl libnl libuvc alsa-lib sqlcipher curl miniupnpc civetweb freetype live555 vo-aacenc


XML2 := libxml2-2.9.3
AXTLS := axtls
FUSE := fuse-2.9.4
FREETYPE := freetype-2.14.1
UNWIND := libunwind-1.6.2
SQLITE3 :=sqlite3-3.10.0
FFMPEG := ffmpeg-3.0
FFMPEG4 := ffmpeg-4.4
LIBNL := libnl-3.2.27
OPENSSL := openssl-3.5.4
CURL := curl-8.16.0
LIBUSB := libusb-1.0.26
LIBUVC := libuvc-0.0.6
ALSA-LIB := alsa-lib-1.1.9
ALSA-UTILS := alsa-utils-1.1.9
OPENCV := opencv-4.5.5
SQLCIPHER := sqlcipher-4.11.0
MINIUPNPC := miniupnpc-2.3.3
CIVETWEB := civetweb-1.16
LIVE555 := live
VO-AACENC := vo-aacenc-0.1.3

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
AR="${CROSS_COMPILE}ar"
LD="${CROSS_COMPILE}ld"
RANLIB="${CROSS_COMPILE}ranlib"

check_exist = if [ ! -e $(1)/.nvt_finish ]; then rm -rf $(1); tar -jxvf $(1).tar.bz2; fi

.PHONY: $(MAKEFILE_LIST)
all: $(MAKEFILE_LIST)

list:
	@echo $(MAKEFILE_LIST)

init:
	@if [ ! -e "${INSTALL_DIR}" ]; then \
		mkdir -p ${INSTALL_DIR}; \
	fi
	@if [ -z "${CROSS_COMPILE}" ]; then \
		echo "Please execute \"source build/envsetup.sh\" firstly.  Stop."; exit 1; \
	fi

alsa-lib: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${ALSA-LIB}/.nvt_finish ]; then \
		$(call check_exist, ${ALSA-LIB}); \
		cd ${ALSA-LIB};./configure CC=${CC} --host=${NVT_HOST} --disable-python;make; \
	fi
	@cd ${ALSA-LIB}; make install DESTDIR=${INSTALL_DIR};
	@if [ ! -e "${${INSTALL_DIR}/share/alsa}" ]; then \
		mkdir -p ${INSTALL_DIR}/share/alsa; \
		mkdir -p ${INSTALL_DIR}/share/alsa/cards; \
		mkdir -p ${INSTALL_DIR}/share/alsa/pcm; \
	fi
	@cp ${INSTALL_DIR}/usr/share/alsa/alsa.conf ${INSTALL_DIR}/share/alsa/
	@cp ${INSTALL_DIR}/usr/share/alsa/cards/aliases.conf ${INSTALL_DIR}/share/alsa/cards/aliases.conf
	@cp ${INSTALL_DIR}/usr/share/alsa/pcm/default.conf ${INSTALL_DIR}/share/alsa/pcm
	@cp ${INSTALL_DIR}/usr/share/alsa/pcm/dmix.conf ${INSTALL_DIR}/share/alsa/pcm
	@cp ${INSTALL_DIR}/usr/share/alsa/pcm/dsnoop.conf ${INSTALL_DIR}/share/alsa/pcm
	@cp -arfv ${INSTALL_DIR}/share/alsa ${ROOTFS_DIR}/rootfs/usr/share;
	@cp ${INSTALL_DIR}/usr/lib/libasound.* ${LIBRARY_DIR}/output;
	@cp -arvf ${INSTALL_DIR}/usr/include ${LIBRARY_DIR};
	@touch ${ALSA-LIB}/.nvt_finish

alsa-utils: init alsa-lib
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${ALSA-UTILS})
	cd ${ALSA-UTILS};./configure --with-udev-rules-dir=""  --with-systemdsystemunitdir="$(pkg-config systemd --variable=systemdsystemunitdir)" CC=${CC} --host=${NVT_HOST} --with-alsa-inc-prefix="${INSTALL_DIR}/usr/include" --with-alsa-prefix="${INSTALL_DIR}/usr/lib" --disable-alsamixer;make;make install DESTDIR=${INSTALL_DIR};
	@cp -arfv ${INSTALL_DIR}/usr/bin/* ${ROOTFS_DIR}/rootfs/usr/bin;
	@cp -arfv ${INSTALL_DIR}/usr/sbin/* ${ROOTFS_DIR}/rootfs/usr/sbin;
	@cp -arfv ${INSTALL_DIR}/usr/share/* ${ROOTFS_DIR}/rootfs/usr/share;
	@touch ${ALSA-UTILS}/.nvt_finish

libxml2: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${XML2}/.nvt_finish ]; then \
		$(call check_exist, ${XML2}); \
		cd ${XML2}; ./configure --prefix=${INSTALL_DIR} CC=${CC} CXX=${CXX} AR=${AR} --host=${NVT_HOST} --without-python --target=${NVT_HOST}; make -j$(MULTI_CORES); \
	fi
	@cd ${XML2}; make install; cd -;
	@rm -rf ${INSTALL_DIR}/lib/cmake ${INSTALL_DIR}/lib/xml2Conf.sh ${INSTALL_DIR}/lib/pkgconfig
	@touch ${XML2}/.nvt_finish

axtls: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${AXTLS}/.nvt_finish ]; then \
		$(call check_exist, ${AXTLS}); \
		cd ${AXTLS}; make -j$(MULTI_CORES) CC=${CC} AR=${AR}; \
	fi
	@cd ${AXTLS}; make PREFIX=${INSTALL_DIR} install;
	@touch ${AXTLS}/.nvt_finish

fuse: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${FUSE}/.nvt_finish ]; then \
		$(call check_exist, ${FUSE}); \
		cd ${FUSE}; ./configure --host=${NVT_HOST} CC=${CC} LD=${LD} AR=${AR}; \
		make -j$(MULTI_CORES); \
	fi
	cd ${FUSE}; make -j$(MULTI_CORES) install DESTDIR=${EXTLIB}/${FUSE}/install; cp -arvf ${EXTLIB}/${FUSE}/install/usr/local/* ${INSTALL_DIR};
	@touch ${FUSE}/.nvt_finish

freetype: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${FREETYPE}/.nvt_finish ]; then \
		$(call check_exist, ${FREETYPE}); \
		cd ${FREETYPE}; ./configure --prefix=${INSTALL_DIR} --host=${NVT_HOST} CC=${CC} AR=${AR} LD=${LD} \
			--without-zlib --without-png --without-harfbuzz --without-bzip2 --without-brotli && \
		make -j$(MULTI_CORES); \
	fi
	@cd ${FREETYPE}; make install;
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@touch ${FREETYPE}/.nvt_finish

libunwind: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${UNWIND}/.nvt_finish ]; then \
		$(call check_exist, ${UNWIND}); \
		cd ${UNWIND}; mkdir result; CC=${NVT_HOST}-gcc; ./configure --prefix=${INSTALL_DIR} --host=${NVT_HOST}; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${UNWIND}; make install;
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@touch ${UNWIND}/.nvt_finish

opencv: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${OPENCV}/.nvt_finish ]; then \
		$(call check_exist, ${OPENCV}); \
		cd ${OPENCV}; \
		rm -rf build; \
		mkdir build; \
		cd build; \
		cmake -D CMAKE_CXX_COMPILER=${CXX} -D CMAKE_CXX_COMPILER_AR=${AR} -D CMAKE_C_COMPILER=${CC} -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=${INSTALL_DIR}  -DCPU_BASELINE=";NEON" -DCPU_BASELINE_FINAL=";NEON" -D WITH_NEON=ON -D ENABLE_NEON=ON -D USE_NEON=ON -D CMAKE_TOOLCHAIN_FILE=../platforms/linux/aarch64-gnu.toolchain.cmake -D BUILD_opencv_dnn_objdetect=ON -D BUILD_opencv_dnn=ON -D BUILD_ZLIB=ON -D OPENCV_EXTRA_MODULES_PATH=../opencv_contrib-4.5.5/modules -D WITH_FFMPEG=ON -D CUDA_FAST_MATH=0 -D WITH_1394=OFF -D BUILD_opencv_hdf=OFF -D BUILD_opencv_gapi=OFF -D BUILD_opencv_highgui=OFF -D BUILD_opencv_freetype=OFF -D WITH_CUBLAS=OFF -D WITH_CUDA=OFF -D BUILD_opencv_cudacodec=OFF -D WITH_CUDNN=OFF -D OPENCV_DNN_CUDA=OFF -D WITH_V4L=OFF -D WITH_QT=OFF -D WITH_OPENGL=OFF -D WITH_GSTREAMER=OFF -D OPENCV_GENERATE_PKGCONFIG=OFF -D OPENCV_ENABLE_NONFREE=ON -D INSTALL_PYTHON_EXAMPLES=OFF -D INSTALL_C_EXAMPLES=OFF -D BUILD_JAVA=OFF -D BUILD_EXAMPLES=OFF -D BUILD_opencv_python=OFF  .. ; \
        make -j$(MULTI_CORES) CC=${CC} CXX=${CXX} LD=${LD} AR=${AR} SYSROOT_PATH=${SYSROOT_PATH}; \
		make install; \
	fi
	@touch ${OPENCV}/.nvt_finish
	
opencv_static: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${OPENCV}/.nvt_finish_static ]; then \
        $(call check_exist, ${OPENCV}); \
        cd ${OPENCV}; \
        rm -rf build_static; \
        mkdir build_static; \
        cd build_static; \
        cmake -D BUILD_SHARED_LIBS=OFF -D CMAKE_CXX_COMPILER=${CXX} -D CMAKE_CXX_COMPILER_AR=${AR} -D CMAKE_C_COMPILER=${CC} -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=${INSTALL_DIR}  -DCPU_BASELINE=";NEON" -DCPU_BASELINE_FINAL=";NEON" -D WITH_NEON=ON -D ENABLE_NEON=ON -D USE_NEON=ON -D CMAKE_TOOLCHAIN_FILE=../platforms/linux/aarch64-gnu.toolchain.cmake -D BUILD_opencv_dnn_objdetect=ON -D BUILD_opencv_dnn=ON -D BUILD_ZLIB=ON -D OPENCV_EXTRA_MODULES_PATH=../opencv_contrib-4.5.5/modules -D WITH_FFMPEG=ON -D CUDA_FAST_MATH=0 -D WITH_1394=OFF -D BUILD_opencv_hdf=OFF -D BUILD_opencv_gapi=OFF -D BUILD_opencv_highgui=OFF -D BUILD_opencv_freetype=OFF -D WITH_CUBLAS=OFF -D WITH_CUDA=OFF -D BUILD_opencv_cudacodec=OFF -D WITH_CUDNN=OFF -D OPENCV_DNN_CUDA=OFF -D WITH_V4L=OFF -D WITH_QT=OFF -D WITH_OPENGL=OFF -D WITH_GSTREAMER=OFF -D OPENCV_GENERATE_PKGCONFIG=OFF -D OPENCV_ENABLE_NONFREE=ON -D INSTALL_PYTHON_EXAMPLES=OFF -D INSTALL_C_EXAMPLES=OFF -D BUILD_JAVA=OFF -D BUILD_EXAMPLES=OFF -D BUILD_opencv_python=OFF  .. ; \
        make -j$(MULTI_CORES) CC=${CC} CXX=${CXX} LD=${LD} AR=${AR} SYSROOT_PATH=${SYSROOT_PATH}; \
        make install; \
    fi
	@touch ${OPENCV}/.nvt_finish_static
		
sqlite3: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${SQLITE3}/.nvt_finish ]; then \
		$(call check_exist, ${SQLITE3}); \
		cd ${SQLITE3}; make -j$(MULTI_CORES) CC=${CC} AR=${AR} STRIP="${CROSS_COMPILE}strip"; \
	fi
	@cd ${SQLITE3}; make INSTALL_DIR=${INSTALL_DIR} install;
	@touch ${SQLITE3}/.nvt_finish

ffmpeg: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${FFMPEG}/.nvt_finish ]; then \
		$(call check_exist, ${FFMPEG}); \
		cd ${FFMPEG}; ./configure --prefix=${INSTALL_DIR} --cross-prefix=${NVT_HOST}- --target-os=linux --arch=${ARCH} --target-exec="qemu-mips -cpu 24Kec" --disable-everything --enable-muxer=mpegts --enable-muxer=mp4 --disable-armv5te --disable-armv6t2 --disable-armv6 --disable-asm --disable-encoders --disable-decoders --enable-protocol=file --enable-small --disable-static --enable-shared --enable-cross-compile --disable-zlib --disable-mipsfpu --disable-ffmpeg --disable-ffserver --disable-ffprobe --disable-swscale --disable-swresample --disable-avdevice --disable-avfilter --disable-debug; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${FFMPEG}; make install;
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@touch ${FFMPEG}/.nvt_finish

ffmpeg4: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${FFMPEG4}/.nvt_finish ]; then \
		$(call check_exist, ${FFMPEG4}); \
		cd ${FFMPEG4}; ./configure --prefix=${INSTALL_DIR} --cross-prefix=${NVT_HOST}- --target-os=linux --arch=${ARCH} --target-exec="qemu-mips -cpu 24Kec" --enable-muxer=mpegts --enable-muxer=mp4 --disable-armv5te --disable-armv6t2 --disable-armv6 --disable-asm --disable-encoders --disable-decoders --enable-protocol=file --enable-small --enable-static --disable-shared --enable-cross-compile --disable-zlib --disable-mipsfpu --disable-ffmpeg --disable-ffprobe --disable-swscale --disable-swresample --disable-avdevice --disable-avfilter --disable-debug; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${FFMPEG4}; make install;
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@touch ${FFMPEG4}/.nvt_finish

libnl: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${LIBNL}/.nvt_finish ]; then \
		$(call check_exist, ${LIBNL}); \
		cd ${LIBNL}; ./configure --prefix=${INSTALL_DIR} --host=${NVT_HOST} CC=${CC} LD=${LD} AR=${AR}; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${LIBNL}; make install;
	@cp ${INSTALL_DIR}/lib/libnl-3.so ${INSTALL_DIR}/lib/libnl.so ; cp ${INSTALL_DIR}/lib/libnl-genl-3.so ${INSTALL_DIR}/lib/libnl-genl.so
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@touch ${LIBNL}/.nvt_finish

openssl: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${OPENSSL}/.nvt_finish ]; then \
		$(call check_exist, ${OPENSSL}); \
		cd ${OPENSSL}; CROSS_COMPILE="" CC=${CC} LD=${LD} AR=${AR} RANLIB=${RANLIB} ./Configure shared linux-generic32 -DHAVE_CRYPTODEV -DUSE_CRYPTODEV_DIGESTS -DOPENSSL_SSL_CLIENT_ENGINE_AUTO --prefix=${INSTALL_DIR} -I$(NVT_DRIVER_DIR)/include; make -j$(MULTI_CORES); \
	fi
	@cd ${OPENSSL}; make -j$(MULTI_CORES) install_sw;
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig ${INSTALL_DIR}/lib/engines
	@touch ${OPENSSL}/.nvt_finish

curl: init openssl
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
ifeq ($(NVT_CURL_SSL),NVT_CURL_SSL_OPENSSL)
	@if [ -z `cat ${CURL}/.nvt_finish | grep NVT_CURL_SSL_OPENSSL` ]; then \
		$(call check_exist, ${CURL}); \
		cd ${CURL}; CPPFLAGS="-I${INSTALL_DIR}/include -I${INCLUDE_DIR}" LDFLAGS="-L${LIBRARY_DIR} -L${INSTALL_DIR}/lib -Wl,-rpath-link=$(INSTALL_DIR)/lib" ./configure --with-ssl=${INSTALL_DIR} --prefix=${INSTALL_DIR} --with-ca-path="/etc/ssl/certs" --without-libpsl --host=${NVT_HOST}; make -j$(MULTI_CORES); \
	fi
else
	@if [ -z `cat ${CURL}/.nvt_finish | grep NVT_CURL_SSL_AXTLS` ]; then \
		$(call check_exist, ${CURL}); \
		cd ${CURL}; CPPFLAGS="-I${INSTALL_DIR}/include -I${INCLUDE_DIR}" LDFLAGS="-L${LIBRARY_DIR} -L${INSTALL_DIR}/lib -Wl,-rpath-link=$(INSTALL_DIR)/lib" ./configure --without-ssl --with-axtls --enable-ipv6 --prefix=${INSTALL_DIR} --without-libpsl --host=${NVT_HOST}; make -j$(MULTI_CORES); \
	fi
endif
	@cd ${CURL}; make install;
	@cd ${INSTALL_DIR}/bin; rm curl-config
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@echo $(NVT_CURL_SSL) > ${CURL}/.nvt_finish

libusb: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${LIBUSB}/.nvt_finish ]; then \
		$(call check_exist, ${LIBUSB}); \
		cd ${LIBUSB}; ./autogen.sh; \
		./autogen.sh --prefix=${INSTALL_DIR} CC=${CC} CXX=${CXX} AR=${AR} --host=${NVT_HOST} --target=${NVT_HOST} --enable-udev=no; \
		make -j$(MULTI_CORES); make install; \
	fi
	@touch ${LIBUSB}/.nvt_finish

# Disable jpeg support
libuvc: init libusb
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${LIBUVC}/.nvt_finish ]; then \
		$(call check_exist, ${LIBUVC}); \
		cd ${LIBUVC}; rm -rf build; mkdir -p build; \
		cd build; \
		cmake -DCMAKE_TOOLCHAIN_FILE=../nvt.cmake \
			-DCMAKE_C_COMPILER=$(CC) \
			-DCMAKE_CXX_COMPILER=$(CXX) \
			-DCMAKE_CXX_FLAGS=-I${INSTALL_DIR}/include/libusb-1.0 \
			-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
			-DCMAKE_C_FLAGS=-I${INSTALL_DIR}/include/libusb-1.0 ..; \
		make -j$(MULTI_CORES); make install; \
	fi
	@touch ${LIBUVC}/.nvt_finish

sqlcipher: init openssl
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${SQLCIPHER}/.nvt_finish ]; then \
		$(call check_exist, ${SQLCIPHER}); \
		cd ${SQLCIPHER}; \
		CFLAGS="-DSQLITE_HAS_CODEC -DSQLITE_EXTRA_INIT=sqlcipher_extra_init -DSQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown -I${INSTALL_DIR}/include" \
		LDFLAGS="-L${INSTALL_DIR}/lib -lcrypto" \
		./configure --prefix=${INSTALL_DIR} CC=${CC} CXX=${CXX} AR=${AR} --host=${NVT_HOST} \
			--with-tempstore=yes; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${SQLCIPHER}; make install;
	@touch ${SQLCIPHER}/.nvt_finish

miniupnpc: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${MINIUPNPC}/.nvt_finish ]; then \
		$(call check_exist, ${MINIUPNPC}); \
		cd ${MINIUPNPC}; make -j$(MULTI_CORES) CC=${CC} AR=${AR} RANLIB=${RANLIB}; \
	fi
	@cd ${MINIUPNPC}; make PREFIX=${INSTALL_DIR} install INSTALLPREFIX=${INSTALL_DIR} CC=${CC} AR=${AR};
	@touch ${MINIUPNPC}/.nvt_finish

live555: init openssl
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${LIVE555}/.nvt_finish ]; then \
		rm -rf ${LIVE555}; tar -jxvf live555.tar.bz2; \
		cd ${LIVE555}; \
		export SDK_LIB_INCLUDE=${LIBRARY_DIR}/include; \
		./genMakefiles nvt-aarch64; \
		make -j$(MULTI_CORES); \
	fi
	@mkdir -p ${INSTALL_DIR}/lib ${INSTALL_DIR}/include/live555
	@chmod -R u+w ${INSTALL_DIR}/include/live555 2>/dev/null || true
	@for lib_dir in liveMedia groupsock UsageEnvironment BasicUsageEnvironment; do \
		if [ -f ${LIVE555}/$$lib_dir/lib$$lib_dir.a ]; then \
			cp -f ${LIVE555}/$$lib_dir/lib$$lib_dir.a ${INSTALL_DIR}/lib/; \
		fi; \
		if [ -d ${LIVE555}/$$lib_dir/include ]; then \
			cp -rf ${LIVE555}/$$lib_dir/include/* ${INSTALL_DIR}/include/live555/; \
		fi; \
	done
	@touch ${LIVE555}/.nvt_finish

civetweb: init openssl
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${CIVETWEB}/.nvt_finish ]; then \
		$(call check_exist, ${CIVETWEB}); \
		cd ${CIVETWEB}; \
		make -j$(MULTI_CORES) CC=${CC} CXX=${CXX} AR=${AR} \
			WITH_IPV6=1 \
			WITH_WEBSOCKET=1 \
			WITH_SERVER_STATS=1 \
			WITH_CPP=1 \
			COPT="-DUSE_STACK_SIZE=102400 -DUSE_X_DOM_SOCKET -DNDEBUG -I${INSTALL_DIR}/include" \
			LOPT="-L${INSTALL_DIR}/lib -lssl -lcrypto" \
			build lib slib; \
	fi
	@cd ${CIVETWEB}; \
		PREFIX=${INSTALL_DIR} WITH_CPP=1 CC=${CC} CXX=${CXX} AR=${AR} make install-headers; \
		PREFIX=${INSTALL_DIR} make install-lib; \
		PREFIX=${INSTALL_DIR} make install-slib;
	@touch ${CIVETWEB}/.nvt_finish

vo-aacenc: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${VO-AACENC}/.nvt_finish ]; then \
		$(call check_exist, ${VO-AACENC}); \
		cd ${VO-AACENC}; \
		autoreconf -ivf; \
		./configure --prefix=${INSTALL_DIR} --host=${NVT_HOST} CC=${CC} CXX=${CXX} AR=${AR} \
			--enable-static --enable-shared; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${VO-AACENC}; make install;
	@rm -f ${INSTALL_DIR}/lib/pkgconfig/vo-aacenc.pc
	@cp -avf ${INSTALL_DIR}/lib/libvo-aacenc.* ${LIBRARY_DIR}/output;
	@mkdir -p ${LIBRARY_DIR}/include/vo-aacenc
	@cp -avf ${INSTALL_DIR}/include/vo-aacenc/* ${LIBRARY_DIR}/include/vo-aacenc/;
	@touch ${VO-AACENC}/.nvt_finish

install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@if [ -x ${INSTALL_DIR}/include ]; then \
		cp -arvf ${INSTALL_DIR}/include ${LIBRARY_DIR}; \
	fi
	@if [ -x ${INSTALL_DIR}/lib ]; then \
		mkdir -p ${LIBRARY_DIR}/output; \
		cp -arvf ${INSTALL_DIR}/lib/*.* ${LIBRARY_DIR}/output; \
	fi
	@if [ -x ${INSTALL_DIR}/usr/lib ]; then \
		cp -arvf ${INSTALL_DIR}/usr/lib/*.* ${LIBRARY_DIR}/output; \
	fi

clean:
	@echo ">>>>>>>>>>>>>>>>>>> Remove >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${INSTALL_DIR}
	@rm -rf ${XML2} ${AXTLS} ${FUSE} ${FREETYPE} ${FFMPEG} ${LIBNL} ${SQLITE3} ${OPENSSL} ${CURL} ${UNWIND} ${LIBP11} $(FFMPEG4)  ${LIBUSB} ${LIBUVC} ${ALSA-LIB} ${OPENCV} ${ALSA-UTILS} ${SQLCIPHER} ${MINIUPNPC} ${CIVETWEB} ${LIVE555} ${VO-AACENC}
