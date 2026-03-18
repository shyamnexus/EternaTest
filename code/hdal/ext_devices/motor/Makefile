###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
SUBDIRS += \
	dummy \
	mtr_an41908 \
	mtr_ms41949 \
	mtr_ti8833 \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

SUBDIRS := $(dir $(shell find . -name Makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)

endif

include $(NVT_HDAL_DIR)/drivers.mk
