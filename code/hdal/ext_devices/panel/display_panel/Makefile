MODULE_NAME = nvt_dispdev_panel
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/ext_devices/panel
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_DIR)/source/kdrv_videoout/include \
	-I$(KDRV_DIR)/include/comm \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \
	$(DISPLCDSEL_PARAMETER) \
	$(DISPDEV_PARAMETER) \


SRC += \
	dispdev_main.c \
	dispdev_proc.c \
	dispdev_drv.c \
	dispdev_api.c \
	dispdev_platform.c \
	dispdev_panlcomn.c \
	dispdev_ver.c \


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \
	$(DISPLCDSEL_PARAMETER) \
	$(DISPDEV_PARAMETER) \


SRC += \
	dispdev_panlcomn.c \
	dispdev_platform.c\
	dispdev_ver.c\

endif

include $(NVT_HDAL_DIR)/driver.mk
