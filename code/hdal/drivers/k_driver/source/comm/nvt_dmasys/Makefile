MODULE_NAME = nvt_dmasys
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \

KDRV_INC_PATH = $(NVT_HDAL_DIR)/drivers/k_driver

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-I$(KDRV_INC_PATH)/include \
	-I$(NVT_VOS_DIR)/drivers/include \

SRC += \
	dma_util.c \
	dma_nvt.c \
	dma_proc.c \


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

endif

include $(NVT_HDAL_DIR)/driver.mk
