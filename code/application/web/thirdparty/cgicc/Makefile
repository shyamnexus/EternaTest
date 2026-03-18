
CGICC := cgicc-3.2.20

LOCAL_INSTALL := __install
LOCAL_INSTALL_DIR := $(shell pwd)/${LOCAL_INSTALL}

.PHONY: all install clean

all: install

install:
	@echo ">>>>>>>>>>>>>>>>>>> cgicc local installing into ${LOCAL_INSTALL_DIR} >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${LOCAL_INSTALL_DIR}/.nvt_finish ]; then \
		if [ ! -d ${CGICC} ]; then tar -jxvf ${CGICC}.tar.bz2; fi; \
		mkdir -p ${LOCAL_INSTALL_DIR}; \
		cd ${CGICC}; \
		./configure --prefix=${LOCAL_INSTALL_DIR} --host=${NVT_HOST} CC=${CC} CXX=${CXX} AR=${AR}; \
		make -j$(MULTI_CORES); \
		make install; \
		touch ${LOCAL_INSTALL_DIR}/.nvt_finish; \
	fi

clean:
	@echo ">>>>>>>>>>>>>>>>>>> cgicc clean >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${CGICC} ${LOCAL_INSTALL_DIR}
