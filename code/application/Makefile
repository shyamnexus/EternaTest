include $(NVT_PRJCFG_MODEL_CFG)
include $(BUILD_DIR)/definitions.mk

APPSUBDIRS := $(subst /,,$(filter $(patsubst %,%/,$(NVT_CFG_APP)), $(subst source/, , $(dir $(wildcard source/*/Makefile source/*/makefile)))))
APP_EXTLIST := $(shell cd $(APP_DIR)/external; make -s list)
LIBSUBDIRS := $(filter-out %external, $(subst /,,$(subst $(LIBRARY_DIR)/source, , $(dir $(wildcard $(LIBRARY_DIR)/source/*/Makefile $(LIBRARY_DIR)/source/*/makefile)))))
LIB_EXTLIST := $(shell cd $(LIBRARY_DIR)/external; make -s list)
BUILD_LIST := $(LIB_EXTLIST:%=nvt@ext@lib@%) $(LIBSUBDIRS:%=nvt@lib@%) $(APP_EXTLIST:%=nvt@ext@app@%) $(APPSUBDIRS)
OUTPUT_DIR = output
WEB_DIR := web
IPCAMERA_DIR := $(APP_DIR)/ipcamera
IPCAMERA_BUILD_DIR := $(IPCAMERA_DIR)/build/aarch64-ca53

all: $(OUTPUT_DIR)
	@$(call makedep, $(BUILD_LIST))
	$(MAKE) -C $(WEB_DIR) all
	@if [ -d "$(IPCAMERA_DIR)" ]; then \
		cd $(IPCAMERA_DIR) && cmake . -B build/aarch64-ca53 -DCMAKE_BUILD_TYPE=Release || exit 1; \
		cmake --build build/aarch64-ca53 --parallel || exit 1; \
	fi

$(OUTPUT_DIR):
	@mkdir -p $@

aarch64-gnu: $(OUTPUT_DIR)
	@$(call makedep, $(BUILD_LIST))
	$(MAKE) -C $(WEB_DIR) all
	@if [ -d "$(IPCAMERA_DIR)" ]; then \
		cd $(IPCAMERA_DIR) && cmake . -B build/aarch64-gnu -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-linux-gnu.cmake || exit 1; \
		cmake --build build/aarch64-gnu --parallel || exit 1; \
	fi

x64: $(OUTPUT_DIR)
	@$(call makedep, $(BUILD_LIST))
	$(MAKE) -C $(WEB_DIR) all
	@if [ -d "$(IPCAMERA_DIR)" ]; then \
		cd $(IPCAMERA_DIR) && cmake . -B build/x64 -DIPCAMERA_USE_SYSTEM_COMPILER=ON || exit 1; \
		cmake --build build/x64 --parallel || exit 1; \
	fi

listpackage:
	@echo -e "\e[1;36mAvailable packages: \e[0m"
	@for n in $(BUILD_LIST); do \
		echo -e "\e[1;33m\t$$n \e[0m"; \
	done;
	@echo -e "\e[1;36mAdd \"DEP_LIBRARIES= package_name\" to your Makefile can support dependent build.\e[0m"

install:
	@$(call makelist, $(addprefix source/, $(APPSUBDIRS)), install)
	@cd $(APP_DIR)/external; make install;
	$(MAKE) -C $(WEB_DIR) install
	@if [ -f "$(IPCAMERA_BUILD_DIR)/apps/ipcamd/ipcamd" ]; then \
		mkdir -p "$(ROOTFS_DIR)/rootfs/bin"; \
		$(STRIP) --strip-all "$(IPCAMERA_BUILD_DIR)/apps/ipcamd/ipcamd" -o "$(ROOTFS_DIR)/rootfs/bin/ipcamd"; \
		chmod +x "$(ROOTFS_DIR)/rootfs/bin/ipcamd"; \
	fi; \
	if [ -d "$(IPCAMERA_BUILD_DIR)/modules" ]; then \
		mkdir -p "$(ROOTFS_DIR)/rootfs/lib"; \
		for so in $(IPCAMERA_BUILD_DIR)/modules/*/libipcam_*.so; do \
			if [ -f "$$so" ]; then \
				$(STRIP) --strip-unneeded "$$so" -o "$(ROOTFS_DIR)/rootfs/lib/$$(basename $$so)"; \
			fi; \
		done; \
	fi; \
	if [ -f "$(IPCAMERA_DIR)/init.d/S30ipcamd" ]; then \
		mkdir -p "$(ROOTFS_DIR)/rootfs/etc/init.d"; \
		cp -f "$(IPCAMERA_DIR)/init.d/S30ipcamd" "$(ROOTFS_DIR)/rootfs/etc/init.d/"; \
		chmod +x "$(ROOTFS_DIR)/rootfs/etc/init.d/S30ipcamd"; \
	fi; \
	if [ -f "$(IPCAMERA_DIR)/init.d/S20ipcamera_config" ]; then \
		mkdir -p "$(ROOTFS_DIR)/rootfs/etc/init.d"; \
		cp -f "$(IPCAMERA_DIR)/init.d/S20ipcamera_config" "$(ROOTFS_DIR)/rootfs/etc/init.d/"; \
		chmod +x "$(ROOTFS_DIR)/rootfs/etc/init.d/S20ipcamera_config"; \
	fi; \
	if [ -f "$(IPCAMERA_DIR)/init.d/S26ntpd" ]; then \
		mkdir -p "$(ROOTFS_DIR)/rootfs/etc/init.d"; \
		cp -f "$(IPCAMERA_DIR)/init.d/S26ntpd" "$(ROOTFS_DIR)/rootfs/etc/init.d/"; \
		chmod +x "$(ROOTFS_DIR)/rootfs/etc/init.d/S26ntpd"; \
	fi; \
	if [ -f "$(IPCAMERA_DIR)/init.d/S60system_logger" ]; then \
		mkdir -p "$(ROOTFS_DIR)/rootfs/etc/init.d"; \
		cp -f "$(IPCAMERA_DIR)/init.d/S60system_logger" "$(ROOTFS_DIR)/rootfs/etc/init.d/"; \
		chmod +x "$(ROOTFS_DIR)/rootfs/etc/init.d/S60system_logger"; \
	fi; \
	if [ -f "$(IPCAMERA_DIR)/init.d/system_logger.sh" ]; then \
		cp -f "$(IPCAMERA_DIR)/init.d/system_logger.sh" "$(ROOTFS_DIR)/rootfs/bin/system_logger.sh"; \
		chmod +x "$(ROOTFS_DIR)/rootfs/bin/system_logger.sh"; \
	fi; \
	if [ -f "$(IPCAMERA_DIR)/configs/ntp.conf" ]; then \
		mkdir -p "$(ROOTFS_DIR)/rootfs/etc"; \
		cp -f "$(IPCAMERA_DIR)/configs/ntp.conf" "$(ROOTFS_DIR)/rootfs/etc/ntp.conf"; \
	fi; \
	if [ -d "$(IPCAMERA_BUILD_DIR)/configs" ]; then \
		mkdir -p "$(ROOTFS_DIR)/rootfs/etc/ipcamera"; \
		cp -rf "$(IPCAMERA_BUILD_DIR)/configs" "$(ROOTFS_DIR)/rootfs/etc/ipcamera/"; \
		find "$(ROOTFS_DIR)/rootfs/etc/ipcamera/configs/config.d" -type f ! -name '.gitexist' -delete; \
		find "$(ROOTFS_DIR)/rootfs/etc/ipcamera/configs/config.factory.d" -type f \( -name "*.bak" -o -name "*~" -o -name ".*.swp" -o -name "#*#" \) -delete; \
		echo "Cleaned config.d/ directory (runtime user configs only)"; \
		echo "Removed backup/temp files from config.factory.d/"; \
	fi; \
	if [ -d "$(IPCAMERA_DIR)/assets" ]; then \
		mkdir -p "$(ROOTFS_DIR)/rootfs/etc/ipcamera/assets"; \
		cd "$(IPCAMERA_DIR)/assets" && find . -mindepth 1 -not -path './models' -not -path './models/*' | while read item; do \
			if [ -d "$$item" ]; then \
				mkdir -p "$(ROOTFS_DIR)/rootfs/etc/ipcamera/assets/$$item"; \
			else \
				cp -f "$$item" "$(ROOTFS_DIR)/rootfs/etc/ipcamera/assets/$$item"; \
			fi; \
		done; \
		echo "Installed ipcamera assets to rootfs (models excluded - stored in app partition)"; \
	fi; \
	mkdir -p "$(ROOTFS_DIR)/rootfs/var/log";

clean:
	@$(call makelist, $(addprefix source/, $(APPSUBDIRS)), clean)
	@cd $(APP_DIR)/external; make clean;
	@rm -rf $(OUTPUT_DIR)
	$(MAKE) -C $(WEB_DIR) clean
	@rm -rf $(IPCAMERA_DIR)/build/aarch64-ca53
	@rm -rf $(IPCAMERA_DIR)/build/aarch64-gnu
	@rm -rf $(IPCAMERA_DIR)/build/x64
	@rm -rf "$(ROOTFS_DIR)/rootfs/etc/ipcamera"
	@rm -f "$(ROOTFS_DIR)/rootfs/etc/init.d/S20ipcamera_config"
	@rm -f "$(ROOTFS_DIR)/rootfs/etc/init.d/S26ntpd"
	@rm -f "$(ROOTFS_DIR)/rootfs/etc/init.d/S30ipcamd"
	@rm -f "$(ROOTFS_DIR)/rootfs/etc/init.d/S20ntpd"
	@rm -f "$(ROOTFS_DIR)/rootfs/etc/ntp.conf"
	@rm -f "$(ROOTFS_DIR)/rootfs/lib/libipcam_"*.so
	@rm -f "$(ROOTFS_DIR)/rootfs/bin/ipcamd"
	@rm -rf "$(ROOTFS_DIR)/rootfs/etc/ipcamera/assets"
