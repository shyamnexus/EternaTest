MODULE_NAME = nvt_videosprite
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
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

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_KFLOW_VIDEOSPRITE),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KFLOW_VIDEOSPRITE)
endif

SRC += \
	videosprite.c \
	videosprite_open.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-Wno-format \


SRC += \
videosprite.c videosprite_open.c

endif

include $(NVT_HDAL_DIR)/driver.mk
