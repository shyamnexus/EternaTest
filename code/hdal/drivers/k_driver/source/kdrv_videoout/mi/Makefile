MODULE_NAME = nvt_mi
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

EXTRA_INCLUDE += \
	-I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME) \
	-I$(KDRV_DIR)/source/rtos_$(SDK_CODENAME)/i2c \


###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_KDRV_MI),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_MI)
endif

SRC += \
	mi.c \
	mi_platform.c \
	mi_main.c \
	mi_drv.c \
	mi_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \

SRC += \
	mi.c \
	mi_platform.c \
	mi_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
