MODEL_NAME = pr2020k
MODULE_NAME = nvt_sen_ad_$(MODEL_NAME)
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/ext_devices/ad
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KDRV_COMM_INC_PATH = $(KDRV_DIR)/include/comm
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
EXT_DEVICES_INC_PATH=$(NVT_HDAL_DIR)/ext_devices/ad/ad_common

# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_COMM_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \

EXTRA_INCLUDE += \
	-I$(KDRV_DIR)/include/ \
	-I$(KFLOW_DIR)/include/ \
	-I$(EXT_DEVICES_INC_PATH)/ \
	-I$(EXT_DEVICES_INC_PATH)/ad_drv/ \

EXTRA_INCLUDE += -I$(CURR_DRIVER_DIR)/sen_ad_$(MODEL_NAME)/external

EXTRA_FLGAS = -DNVT_PLATFORM
EXTRA_COMM_FLAG = -D_MODEL_NAME=$(MODEL_NAME)
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \
	$(EXTRA_COMM_FLAG) \
	$(EXTRA_FLGAS) \


SRC += \
	ad_i2c.c \
	ad_std_drv.c \
	ad_std_drv_param.c \
	ad_devnode_parser.c \
	ad_drv.c \
	ad_pr2020k.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += $(EXTRA_COMM_FLAG) $(EXTRA_FLGAS)
EXTRA_INCLUDE += \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/include/plat \


SRC += \
	ad_i2c.c \
	ad_dtsi_parser.c \
	ad_std_drv.c \
	ad_std_drv_param.c \
	ad_drv.c \
	ad_pr2020k.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
