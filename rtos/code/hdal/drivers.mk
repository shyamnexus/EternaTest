ANSI_RED='\033[0;31m'
ANSI_YELLOW='\033[1;33m'
ANSI_RESET='\033[0m'

PWD := $(shell pwd)
.PHONY: modules modules_install clean $(SUBDIRS)
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
KERVER = $(shell cat ${KERNELDIR}/include/config/kernel.release)
###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

endif

EXIST-SUB-MAKEFILES = $(foreach n, $(SUBDIRS), $(wildcard $(n)/Makefile))
EXIST-SUBDIRS = $(EXIST-SUB-MAKEFILES:%/Makefile=%)
NON-EXIST-SUBDIRS = $(filter-out $(EXIST-SUBDIRS), $(SUBDIRS))

modules: $(SUBDIRS)
ifeq ($(SUBDIRS), )
	@echo "nothing to be done for '$(notdir $(PWD))'"
endif

$(EXIST-SUBDIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

$(NON-EXIST-SUBDIRS):
	@echo -e "$(ANSI_RED)$@/Makefile is not existing. skipped.$(ANSI_RESET)"

modules_install: $(SUBDIRS)
ifeq ($(NVT_PRJCFG_CFG),Linux)
	@depmod -b $(ROOTFS_DIR)/rootfs -a $(KERVER)
endif

clean: $(SUBDIRS)
	@rm -rf ../output

chk_cert: $(SUBDIRS)
