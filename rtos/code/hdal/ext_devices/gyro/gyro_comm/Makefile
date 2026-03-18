MODULE_NAME = gyro_comm
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/ext_devices/gyro
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
EXT_DEVICES_INC_PATH=$(NVT_HDAL_DIR)/ext_devices/gyro/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	gyro_comm_main.c \
	gyro_comm_proc.c \
	gyro_comm_api.c \
	gyro_comm_ver.c \

# gyro_comm_platform.c gyro_comm.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \


SRC += \
		gyro_comm_api.c\

endif

include $(NVT_HDAL_DIR)/driver.mk
