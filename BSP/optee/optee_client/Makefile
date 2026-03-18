ifeq (,$(wildcard /opt/utility/cmake-3.5))
CMAKE = cmake
else
CMAKE = cmake-3.5
endif

.PHONY: all ta tc install clean
ifeq ($(ARCH),arm)
export TA_DEV_KIT_DIR=${OPTEE_DIR}/optee_os/out/arm-plat-novatek/export-ta_arm32
else
export TA_DEV_KIT_DIR=${OPTEE_DIR}/optee_os/out/arm-plat-novatek/export-ta_arm64
endif

PWD := $(shell pwd)

TA_DIRS := optee_examples/acipher/ta \
           optee_examples/aes/ta \
           optee_examples/hello_world/ta \
	   optee_examples/hotp/ta \
	   optee_examples/random/ta \
	   optee_examples/secure_storage/ta \
	   optee_examples/nvt_crypto/ta \
	   optee_test/ta \

TA_OUTPUT_DIR := $(ROOTFS_DIR)/rootfs/lib/optee_armtz

all: tc ta

ta:
	@$(foreach dir,$(TA_DIRS), $(MAKE) -C $(dir) O=output &&) true

tc:
ifeq ($(ARCH),arm)
	@mkdir -p build && \
	cd build && \
	$(CMAKE) .. \
	-DOPENSSL_ROOT_DIR=${LIBRARY_DIR}/external/openssl-1.0.2d; \
	make
else
	@mkdir -p build && \
	cd build && \
	$(CMAKE) .. \
	-DOPENSSL_ROOT_DIR=${OPTEE_DIR}/optee_client/optee_test/host/openssl/lib/aarch64; \
	make
endif
install:
	@mkdir -p ${LIBRARY_DIR}/output
	@make install -C ./build
	@cp -avf build/output/bin $(ROOTFS_DIR)/rootfs/usr
	@cp -avf build/output/sbin $(ROOTFS_DIR)/rootfs/usr
	@cp -avf build/output/usr/lib $(ROOTFS_DIR)/rootfs/usr
	@mkdir -p $(ROOTFS_DIR)/rootfs/usr/lib
	@cp -avf build/output/lib/libteec.so* $(ROOTFS_DIR)/rootfs/usr/lib/
	@cp -avf build/output/lib/*.* ${LIBRARY_DIR}/output
	@cp -avf build/output/include/*.h ${LIBRARY_DIR}/include
	@cp -avf build/optee_benchmark/libyaml/*.so ${LIBRARY_DIR}/output
	@mkdir -p $(TA_OUTPUT_DIR)
	@$(foreach dir,$(TA_DIRS), find $(dir) -name *.ta | xargs -i cp -avf {} $(TA_OUTPUT_DIR) &&) true

clean:
	@if [ -d $(TA_DEV_KIT_DIR) ]; then \
		$(foreach dir,$(TA_DIRS), $(MAKE) clean -C $(dir) O=output q=@ &&) true; \
	else \
		echo TA_DEV_KIT_DIR is not existing. skip clean ta; \
	fi
	@rm build -rf

