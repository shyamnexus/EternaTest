# Makefile for web application (entry.cgi is phased out)

WEBUI_SRC := $(CURDIR)/www
WEBUI_SUBMODULE := $(CURDIR)/ipcamera_webui
ROOTFS_WWW := $(ROOTFS_DIR)/rootfs/var/www

.PHONY: all clean install install-webui clean-webui build-webui

all: build

build: build-webui
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@echo "Web application build complete"

# Build Angular WebUI from submodule if available and configured
build-webui:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@# Auto-extract www.tar.gz if www folder doesn't exist
	@if [ ! -d "$(WEBUI_SRC)" ] && [ -f "$(CURDIR)/www.tar.gz" ]; then \
		echo "Extracting www.tar.gz..."; \
		tar xzf $(CURDIR)/www.tar.gz -C $(CURDIR); \
		echo "www folder extracted successfully"; \
	fi
	@if [ -d "$(WEBUI_SUBMODULE)" ]; then \
		if [ -d "$(WEBUI_SUBMODULE)/node_modules" ]; then \
			echo "Building Angular WebUI from submodule..."; \
			$(MAKE) -C $(WEBUI_SUBMODULE) build-prod && \
			$(MAKE) -C $(WEBUI_SUBMODULE) install-www && \
			echo "Angular WebUI built and installed to www/"; \
		else \
			echo "ipcamera_webui submodule found but node_modules not installed."; \
			echo "Run 'cd ipcamera_webui && npm install' to enable WebUI builds."; \
			echo "Using pre-built www/ files."; \
		fi \
	else \
		echo "ipcamera_webui submodule not present, using pre-built www/ files."; \
	fi

clean:
	@rm -rf $(ROOTFS_DIR)/rootfs/etc/nginx/*
	@rm -rf $(ROOTFS_DIR)/rootfs/var/www/*
	@rm -rf $(ROOTFS_DIR)/rootfs/usr/conf/*
	@rm -f $(ROOTFS_DIR)/rootfs/etc/init.d/S50nginx
	@rm -f $(ROOTFS_DIR)/rootfs/etc/init.d/S51go2rtc
	@rm -f $(ROOTFS_DIR)/rootfs/bin/go2rtc_linux_arm64
	@rm -f $(ROOTFS_DIR)/rootfs/etc/go2rtc.yaml
	# Remove nobody user entries if present
	@if [ -f $(ROOTFS_DIR)/rootfs/etc/passwd ]; then \
		sed -i '/^nobody:/d' $(ROOTFS_DIR)/rootfs/etc/passwd; \
	fi
	@if [ -f $(ROOTFS_DIR)/rootfs/etc/group ]; then \
		sed -i '/^nobody:/d' $(ROOTFS_DIR)/rootfs/etc/group; \
	fi
	@if [ -f $(ROOTFS_DIR)/rootfs/etc/shadow ]; then \
		sed -i '/^nobody:/d' $(ROOTFS_DIR)/rootfs/etc/shadow; \
	fi

install-nginx-config:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@mkdir -p $(ROOTFS_DIR)/rootfs/usr/conf
	# Add nobody group if not present
	@if ! grep -q '^nobody:' $(ROOTFS_DIR)/rootfs/etc/group; then \
		echo 'nobody:x:65534:' >> $(ROOTFS_DIR)/rootfs/etc/group; \
	fi
	# Add nobody user if not present
	@if ! grep -q '^nobody:' $(ROOTFS_DIR)/rootfs/etc/passwd; then \
		echo 'nobody:x:65534:65534:nobody:/nonexistent:/usr/sbin/nologin' >> $(ROOTFS_DIR)/rootfs/etc/passwd; \
	fi
	# Add nobody shadow entry if not present
	@if [ -f $(ROOTFS_DIR)/rootfs/etc/shadow ] && ! grep -q '^nobody:' $(ROOTFS_DIR)/rootfs/etc/shadow; then \
		echo 'nobody:*:10933:0:99999:7:::' >> $(ROOTFS_DIR)/rootfs/etc/shadow; \
	fi
	cp -avf conf/* $(ROOTFS_DIR)/rootfs/usr/conf/
	cp -avf init.d/S50nginx $(ROOTFS_DIR)/rootfs/etc/init.d/S50nginx

install-webui:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@rm -rf $(ROOTFS_WWW)/*
	@mkdir -p $(ROOTFS_WWW)
	cp -avf $(WEBUI_SRC)/* $(ROOTFS_WWW)/
	@if [ -f $(WEBUI_SRC)/index.csr.html.gz ]; then \
	    gzip -d -c $(WEBUI_SRC)/index.csr.html.gz > $(ROOTFS_WWW)/index.csr.html; \
	fi

clean-webui:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@if [ -d $(ROOTFS_WWW) ]; then \
	    rm -rf $(ROOTFS_WWW)/*; \
	    echo "Removed files from $(ROOTFS_WWW)"; \
	else \
	    echo "$(ROOTFS_WWW) does not exist."; \
	fi
	@# Note: www/ folder in source tree is auto-extracted from www.tar.gz and can be safely removed
	@if [ -d "$(WEBUI_SRC)" ]; then \
	    echo "To clean extracted www folder, run: rm -rf $(WEBUI_SRC)"; \
	fi

install-certs:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@# SSL certificates are generated at runtime on first boot (not shipped in firmware)
	@# This just creates the directory structure - actual certs will be symlinked from /mnt/app
	@mkdir -p $(ROOTFS_DIR)/rootfs/etc/nginx/ssl

install-go2rtc:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@mkdir -p $(ROOTFS_DIR)/rootfs/bin
	@mkdir -p $(ROOTFS_DIR)/rootfs/etc
	cp -avf go2rtc_linux_arm64 $(ROOTFS_DIR)/rootfs/bin/
	cp -avf go2rtc.yaml $(ROOTFS_DIR)/rootfs/etc/
	cp -avf init.d/S51go2rtc $(ROOTFS_DIR)/rootfs/etc/init.d/S51go2rtc
	chmod +x $(ROOTFS_DIR)/rootfs/bin/go2rtc_linux_arm64
	chmod +x $(ROOTFS_DIR)/rootfs/etc/init.d/S51go2rtc

install: install-nginx-config install-webui install-certs install-go2rtc
clean: clean-webui
