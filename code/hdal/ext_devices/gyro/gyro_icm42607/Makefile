MODULE_NAME = nvt_gyro_icm42607
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/ext_devices/gyro
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
EXT_DEVICES_INC_PATH=$(NVT_HDAL_DIR)/ext_devices/gyro/include
VENDOR_DRV_INC_PATH=$(NVT_HDAL_DIR)/vendor/isp/drivers
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(VENDOR_DRV_INC_PATH)/include/ \

EXTRA_INCLUDE += \
	-I$(KDRV_DIR)/include/ \
	-I$(KFLOW_DIR)/include/ \
	-I$(EXT_DEVICES_INC_PATH)/ \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
ISP_BUILDIN_INC_PATH = $(KDRV_DIR)/source/kdrv_builtin/isp
EXTRA_INCLUDE += -I$(ISP_BUILDIN_INC_PATH)

EXTRA_CFLAGS += -DDEBUG

SRC += \
	gyro_icm42607.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/include/plat \


SRC += \
	gyro_icm42607.c

endif

include $(NVT_HDAL_DIR)/driver.mk
