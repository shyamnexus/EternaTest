MODULE_NAME = nvt_dsiphy

# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
DSIPHY_DRIVER_PATH = $(KDRV_DIR)/source/source/kdrv_videoout/dsiphy

# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \
	-I$(KDRV_DIR)/source/kdrv_videoout/include/ \

EXTRA_INCLUDE += \
	-I$(NVT_DRIVER_DIR)/include \
	-I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME) \


EXTRA_INCLUDE += -I$(DSIPHY_DRIVER_PATH)

EXTRA_INCLUDE += -I$(CURR_DRIVER_DIR)/source/kdrv_videoout/dsiphy/include

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -D_$(NVT_SDK_TYPE)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_KDRV_DSIPHY),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_DSIPHY)
endif

SRC += \
	dsiphy.c \
	dsiphy_main.c \
	dsiphy_proc.c \
	dsiphy_drv.c \
	dsiphy_api.c \
	dsiphy_ver.c \
	dsiphy_platform.c \
	dsiphy_export.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \


SRC += \
	dsiphy.c \
	dsiphy_platform.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
