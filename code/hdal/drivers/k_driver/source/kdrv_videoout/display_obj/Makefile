MODULE_NAME = kdrv_videoout
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
	-I$(KDRV_INC_PATH)/comm \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_KDRV_VIDEOOUT),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_VIDEOOUT)
endif

SRC += \
	disp1.c \
	display.c \
	display_obj_api.c \
	display_obj_drv.c \
	display_obj_main.c \
	display_obj_platform.c \
	display_obj_proc.c \
	kdrv_vdoout.c \
	kdrv_vdoout_ver.c \
	disp2.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \

SRC += \
	disp1.c \
	disp2.c \
	display.c \
	display_obj_platform.c \
	kdrv_vdoout.c \
	kdrv_vdoout_ver.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
