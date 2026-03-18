MODULE_NAME = nvt_tse
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

EXTRA_INCLUDE += -I$(NVT_DRIVER_DIR)/include

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	tse_main.c \
	tse_proc.c \
	tse_drv.c \
	tse_ioctl.c \
	tse_api.c \
	tse_export.c \
	tse_platform.c \
	tse.c \
	tse_ver.c \
	#tse_cert.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
include $(NVT_PRJCFG_MODEL_CFG)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \
	-D_$(PCIE_EMU_TYPE)_ \


SRC += \
	tse.c \
	tse_platform.c \
	tse_ver.c \
	tse_cert.c

endif
include $(NVT_HDAL_DIR)/driver.mk
