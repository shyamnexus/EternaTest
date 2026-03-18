KDRV_FLOW=$(NVT_HDAL_DIR)/drivers/k_flow
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
SUBDIRS += \
	kflow_common \
	kflow_rpc \
	kflow_gfx \
	kflow_gfx/videosprite \
	kflow_videoprocess \
	kflow_videoprocess_539a \
	kflow_videocapture \
	kflow_videoout/unit \
	kflow_videoenc/unit \
	kflow_videodec/unit \
	kflow_audiocapture/unit \
	kflow_audioenc/unit \
	kflow_audiodec/unit \
	kflow_audioout/unit \
	kflow_csitx \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

SUBDIRS := $(dir $(shell find . -name Makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)

endif

include $(NVT_HDAL_DIR)/drivers.mk

# dependency for multi-thread
kflow_gfx: kflow_common
kflow_gfx/videosprite: kflow_gfx
kflow_videoprocess: kflow_gfx/videosprite
kflow_videoprocess_539a: kflow_gfx/videosprite
kflow_videocapture: kflow_common
kflow_videoout/unit: kflow_gfx/videosprite
kflow_videoenc/unit: kflow_gfx/videosprite
kflow_videodec/unit: kflow_videoenc/unit
kflow_audiocapture/unit: kflow_common
kflow_audioenc/unit: kflow_common
kflow_audiodec/unit: kflow_common
kflow_audioout/unit: kflow_common
kflow_audioout/unit: kflow_audiocapture/unit
kflow_csitx: kflow_common