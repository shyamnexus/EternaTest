MODULE_NAME = nvt_dsi
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
DSIPHY_DRIVER_PATH = $(KDRV_DIR)/source/kdrv_videoout/dsiphy/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \
	-I$(KDRV_DIR)/source/kdrv_videoout/include \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)
EXTRA_INCLUDE += -I$(DSIPHY_DRIVER_PATH) -I$(DSIPHY_DRIVER_PATH)/include

EXTRA_INCLUDE += \
	-I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME) \
	-I$(KDRV_DIR)/source/rtos_$(SDK_CODENAME)/i2c \


###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_KDRV_DSI),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_DSI)
endif

SRC += \
	dsi.c \
	dsi_api.c \
	dsi_drv.c \
	dsi_int.c \
	dsi_main.c \
	dsi_proc.c \
	dsi_platform.c \
	dsi_ver.c \
	dsi_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \

SRC += \
	dsi.c \
	dsi_int.c \
	dsi_platform.c \
	dsi_ver.c \
	dsi_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
