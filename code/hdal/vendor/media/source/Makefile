MODULE_NAME = libvendor_media
# DIRs
MEDIA_LIB_DIR = $(NVT_HDAL_DIR)/vendor/media
OUTPUT_DIR = $(NVT_HDAL_DIR)/vendor/output
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
# INCs
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_SRC_INC_PATH = $(NVT_HDAL_DIR)/source/include
MEDIA_LIB_INC_PATH = $(MEDIA_LIB_DIR)/include
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(HDAL_SRC_INC_PATH) \
	-I$(MEDIA_LIB_INC_PATH) \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(KFLOW_DIR)/source/kflow_videodec/unit/include_protected \
	-I$(KFLOW_DIR)/source/kflow_videoenc/unit/include_protected \
	-I$(KDRV_DIR)/include/kdrv_builtin \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
CFLAGS = $(PRJCFG_CFLAGS)
EXTRA_CFLAGS += $(NVT_ASAN)   $(CPPFLAGS) $(CFLAGS)   -D__LINUX -Werror
EXTRA_LDFLAGS = $(NVT_ASAN)

#--------- Compiling --------------------

SRC += \
	vendor_common.c \
	vendor_gfx.c \
	vendor_audiocapture.c \
	vendor_audiodec.c \
	vendor_audioenc.c \
	vendor_audioout.c \
	vendor_videocapture.c \
	vendor_videodec.c \
	vendor_videoprocess.c \
	vendor_videoout.c \
	vendor_videoenc.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
# DIRs
RTOS_KERNEL_DIR = $(KERNELDIR)/lib/FreeRTOS
RTOS_LIB_DIR = $(KERNELDIR)/lib
RTOS_CURR_DEMO_DIR = $(KERNELDIR)/demos/novatek/$(SDK_CODENAME)
RTOS_POSIX_DIR = $(KERNELDIR)/lib/FreeRTOS-Plus-POSIX
RTOS_POSIX_SRC_DIR = $(RTOS_POSIX_DIR)/source
ISP_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
# INCs
ISP_DRIVER_INC_PATH = $(ISP_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(ISP_DRIVER_INC_PATH) \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/include/plat \

EXTRA_CFLAGS += -Wno-format
SRC += \
	vendor_audiocapture.c \
	  vendor_audiodec.c \
	  vendor_audioenc.c \
	  vendor_audioout.c \
	  vendor_videocapture.c \
	  vendor_videoenc.c \
	  vendor_videodec.c \
	  vendor_videoout.c \
	  vendor_gfx.c \
	  vendor_common.c \
	  vendor_videoprocess.c \


endif

include $(NVT_HDAL_DIR)/library.mk
