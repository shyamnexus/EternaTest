MODULE_NAME = kdrv_hwcopy
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
MODULE_INC_PATH = $(shell pwd)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \
	-I$(MODULE_INC_PATH) \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	hwcopy_main.c \
					   hwcopy_proc.c \
					   hwcopy_drv.c \
					   hwcopy_api.c \
					   hwcopy.c \
					   hwcopy_platform.c \
					   emu_hwcopy.c \
					   kdrv_hwcopy_if.c \
					   hwcopy_cert.c \
	hwcopy_export.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
include $(NVT_PRJCFG_MODEL_CFG)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \
	-D_$(MSIX_TYPE)_ \
	-D_$(PCIE_EMU_TYPE)_ \

SRC += \
	hwcopy.c \
	hwcopy_platform.c \
	kdrv_hwcopy_if.c \
	hwcopy_cert.c	\

endif

include $(NVT_HDAL_DIR)/driver.mk
