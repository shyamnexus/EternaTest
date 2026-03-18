MODULE_NAME = nvt_aud_emu
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/ext_devices/audio
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/aud_emu/include
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
EXTRA_INCLUDE += -I$(KFLOW_INC_PATH)/include

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/include

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/source/kdrv_audioio

EXTRA_INCLUDE += -I$(KDRV_DIR)/source/include/

EXTRA_CFLAGS += \
	-I$(src)/../include \
	-DDEBUG \


SRC += \
	aud_emu_main.c \
	aud_emu_drv.c \
	aud_emu.c \
	aud_i2c.c \


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

EXTRA_CFLAGS += \
	-DDEBUG \
	-Wno-format \


SRC += \
	aud_emu.c \
	aud_i2c.c

endif

include $(NVT_HDAL_DIR)/driver.mk
