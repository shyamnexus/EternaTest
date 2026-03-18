###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
SUBDIRS += \
	isp \
	isp_539a \
	ae \
	awb \
	af \
	iq \
	iq_539a \
	vpe \
	eis \
	samples/sample_ae \
	samples/sample_af \
	samples/sample_awb \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

SUBDIRS := $(dir $(shell find . -name Makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)

endif

include $(NVT_HDAL_DIR)/drivers.mk

#dependency
$(filter-out isp, $(SUBDIRS)): isp

