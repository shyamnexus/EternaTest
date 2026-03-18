MODULE_NAME = nvt_ide
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \
	-I$(KDRV_DIR)/source/kdrv_videoout/include \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_KDRV_IDE),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_IDE)
endif

SRC += \
	ide2.c \
	ide2_int.c \
	ide2_osd.c \
	ide2_video.c \
	idec.c \
	idec_cmd.c \
	ide_api.c \
	ide_drv.c \
	ide_main.c \
	ide_platform.c \
	ide_proc.c \
	ide_ver.c \
	ide_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \

SRC += \
	ide2.c \
	ide2_int.c \
	ide_platform.c \
	ide2_osd.c \
	ide2_video.c \
	idec.c \
	idec_cmd.c \
	ide_ver.c \
	ide_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
