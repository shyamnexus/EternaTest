MODULE_NAME = kdrv_csi_tx
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
	-I$(KDRV_DIR)/source/kdrv_videoout/include/ \

EXTRA_INCLUDE += \
	-I$(NVT_DRIVER_DIR)/include \
	-I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME) \


EXTRA_INCLUDE += \
	-I$(DSIPHY_DRIVER_PATH) \
	-I$(DSIPHY_DRIVER_PATH)/include \


###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	csi_tx.c \
	csi_tx_api.c \
	csi_tx_drv.c \
	csi_tx_int.c \
	csi_tx_main.c \
	csi_tx_proc.c \
	csi_tx_platform.c \


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \

SRC += \
	csi_tx.c \
	csi_tx_int.c \
	csi_tx_platform.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
