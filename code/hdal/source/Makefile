MODULE_NAME = libhdal
# DIRs
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
OUTPUT_DIR = $(NVT_HDAL_DIR)/output
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(NVT_VOS_DIR)/include \
	-I$(NVT_VOS_DIR)/drivers/include \
	-I$(KDRV_DIR)/include \
	-I$(KFLOW_DIR)/include \
	-I$(NVT_HDAL_DIR)/include \
	-I$(NVT_HDAL_DIR)/source/include \
	-I$(NVT_HDAL_DIR)/drivers/k_flow/source/kflow_videoenc/unit/include_protected \
	-I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_videocodec/include \
	-I$(NVT_HDAL_DIR)/drivers/k_flow/source/kflow_videodec/unit/include_protected \
	-I$(NVT_HDAL_DIR)/drivers/k_flow/source/kflow_audioenc/unit/include_protected \
	-I$(NVT_HDAL_DIR)/drivers/k_flow/source/kflow_audiodec/unit/include_protected

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += $(NVT_ASAN)
EXTRA_LDFLAGS = $(NVT_ASAN)    -lpthread -lm

SRC += \
	hd_debug/hd_debug_get.c \
	hd_debug/hd_debug_set.c \
	hd_debug/hd_debug_menu.c \
	hd_logger/hd_logger.c \
	hd_common.c \
	hd_logger/hd_logger_p.c \
	hd_util_menu.c \
	hd_common_menu.c \
	hd_audiodec.c \
	hd_audiodec_menu.c \
	hd_audioenc.c \
	hd_audioenc_menu.c \
	hd_videodec_menu.c \
	hd_videoenc_menu.c \
	hd_videocapture.c \
	hd_videocapture_menu.c \
	hd_videoout.c \
	hd_videoout_menu.c \
	hd_videoprocess.c \
	hd_videoprocess_menu.c \
	hd_audiocapture.c \
	hd_audiocapture_menu.c \
	hd_audioout.c \
	hd_audioout_menu.c \
	hd_int.c \
	hd_util.c \
	hd_videodec.c \
	hd_videoenc.c \
	hd_gfx.c \
	hd_osg.c \
	hd_osg_menu.c \
	hd_bnr.c \

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

#INCs for EXTRA_CFLAGS

EXTRA_INCLUDE += \
	-I$(RTOS_LIB_DIR) \
	-I$(RTOS_KERNEL_DIR)/portable/GCC/ARM_CA9  \
	-I$(RTOS_KERNEL_DIR)/include \
	-I$(RTOS_KERNEL_DIR)/include/private \
	-I$(RTOS_CURR_DEMO_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include/portable/novatek

EXTRA_CFLAGS += -Wno-format

SRC += \
	hd_common.c \
	hd_int.c \
	hd_common_menu.c \
	hd_util.c \
	hd_util_menu.c \
	hd_debug/hd_debug_get.c \
	hd_debug/hd_debug_set.c \
	hd_debug/hd_debug_menu.c \
	hd_logger/hd_logger.c \
	hd_logger/hd_logger_p.c \
	hd_audiocapture.c \
	hd_audiocapture_menu.c \
	hd_audiodec.c \
	hd_audiodec_menu.c \
	hd_audioenc.c \
	hd_audioenc_menu.c \
	hd_audioout.c \
	hd_audioout_menu.c \
	hd_videocapture.c \
	hd_videocapture_menu.c \
	hd_videodec.c \
	hd_videodec_menu.c \
	hd_videoenc.c \
	hd_videoenc_menu.c \
	hd_videoout.c \
	hd_videoout_menu.c \
	hd_videoprocess.c \
	hd_videoprocess_menu.c \
	hd_gfx.c \
	hd_osg.c \
	hd_osg_menu.c \
	hd_bnr.c \

endif

include $(NVT_HDAL_DIR)/library.mk
