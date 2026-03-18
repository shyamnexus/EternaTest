ifeq ("$(wildcard */*/*/*.c)","")
all:
	echo ">>> Skip"
clean:
	echo ">>> Skip"
install:
	@echo ">>>>>>>>>>>>>>>>>>> live555MediaServer >>>>>>>>>>>>>>>>>>>$(LIBRARY_DIR)/output"
	@cp -avf ./release/nvtlive555/lib/*.* $(LIBRARY_DIR)/output
else
all:
ifeq ($(NVT_PRJCFG_CFG), Linux)
	@echo live555 config to nvt-linux
	@cd src/live; sh genMakefiles nvt-linux; make $(NVT_MULTI_CORES_FLAG)
else ifeq ($(NVT_PRJCFG_CFG), rtos)
	@echo live555 config to nvt-rtos
	@cd src/live; sh genMakefiles nvt-rtos; make $(NVT_MULTI_CORES_FLAG)
else
	@echo live555 config to ubuntu
	@cd src/live; sh genMakefiles ubuntu; make
endif

clean:
ifeq ($(NVT_PRJCFG_CFG), Linux)
	@echo live555 config to nvt-linux
	@cd src/live; sh genMakefiles nvt-linux; make clean
else ifeq ($(NVT_PRJCFG_CFG), rtos)
	@echo live555 config to nvt-rtos
	@cd src/live; sh genMakefiles nvt-rtos; make clean
else
	@echo ubuntu
	@cd src/live; sh genMakefiles ubuntu; make clean
endif
	-rm -f src/live/BasicUsageEnvironment/Makefile
	-rm -f src/live/Makefile
	-rm -f src/live/UsageEnvironment/Makefile
	-rm -f src/live/groupsock/Makefile
	-rm -f src/live/liveMedia/Makefile
	-rm -f src/live/mediaServer/Makefile
	-rm -f src/live/proxyServer/Makefile
	-rm -f src/live/testProgs/Makefile
	-rm -f src/live/nvtrtspd/Makefile

install:
	@cd src/live; make install
endif

