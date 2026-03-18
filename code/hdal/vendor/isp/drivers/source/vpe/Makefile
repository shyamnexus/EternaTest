MODULE_NAME = nvt_vpe
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
KFLOW_INC_COMM_PATH = $(KFLOW_DIR)/include/kflow_common
KFLOW_INC_VPROC_PATH = $(KFLOW_DIR)/include/kflow_videoprocess
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
VPE_DRV_INC_PATH=$(NVT_HDAL_DIR)/vendor/isp/drivers
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_INC_PATH)/comm \
	-I$(KFLOW_INC_PATH) \
	-I$(KFLOW_INC_COMM_PATH) \
	-I$(KFLOW_INC_VPROC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(VPE_DRV_INC_PATH)/include \
	-I$(VPE_DRV_INC_PATH)/source/vpe \
	-I$(VPE_DRV_INC_PATH)/source/vpe/vpe_lib \
	-I$(VPE_DRV_INC_PATH)/source/vpe/vpe_param

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_ISP_VPE),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_ISP_VPE)
endif

SRC += \
	vpe_lib/vpe_api.c \
	vpe_lib/vpe_common_param.c \
	vpe_lib/vpe_msg.c \
	vpe_lib/vpet_api.c \
	vpe_param/vpe_param_default.c \
	vpe_dbg.c \
	vpe_dev.c \
	vpe_main.c \
	vpe_proc.c \
	vpe_uti.c \
	vpe_version.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
SRC += \
	vpe_lib/vpe_api.c \
	vpe_lib/vpe_common_param.c \
	vpe_lib/vpet_api.c \
	vpe_lib/vpe_msg.c \
	vpe_param/vpe_param_default.c \
	vpe_dbg.c \
	vpe_dev.c \
	vpe_sxcmd.c \
	vpe_uti.c \
	vpe_version.c

endif

include $(NVT_HDAL_DIR)/driver.mk
