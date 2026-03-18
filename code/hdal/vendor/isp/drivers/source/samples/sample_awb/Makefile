MODULE_NAME = sample_nvt_awb

# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
ISP_DRV_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
ISP_DRV_INC_PATH = $(ISP_DRV_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
ALG_ISP_DRV_INC_PATH=$(NVT_HDAL_DIR)/vendor/alg/isp/drivers
# INC FLAGs

EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_INC_PATH)/comm \
	-I$(KFLOW_INC_PATH) \
	-I$(ISP_DRV_INC_PATH) \
	-I$(ALG_ISP_DRV_INC_PATH)/include/

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	sample_awb.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

SRC += \

endif

include $(NVT_HDAL_DIR)/driver.mk
