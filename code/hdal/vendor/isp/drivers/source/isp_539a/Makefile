MODULE_NAME = nvt_isp_539a
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
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
ISP_BUILDIN_INC_PATH = $(KDRV_DIR)/include/kdrv_builtin
EXTRA_INCLUDE += -I$(ISP_BUILDIN_INC_PATH)

EXTRA_CFLAGS += \
	-D_$(NVT_SDK_TYPE)_ \
	-DDEBUG \
	-D_539A_ \

ifeq ($(CONFIG_NVT_HDAL_ISP_ISP)$(CONFIG_NVT_HDAL_BUILT_IN_WITH_539A_IPP),yy)
OBJ-TAG = $(CONFIG_NVT_HDAL_ISP_ISP)
endif

SRC += \
	isp_api.c \
	isp_cfg.c \
	isp_dbg.c \
	isp_dev.c \
	isp_main.c \
	isp_msg.c \
	isp_mtr.c \
	isp_proc.c \
	isp_uti.c \
	isp_version.c \
	ispt_api.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
SRC += \
	isp_api.c \
	isp_cfg.c \
	isp_dbg.c \
	isp_dev.c \
	isp_msg.c \
	isp_mtr.c \
	isp_sxcmd.c \
	isp_uti.c \
	isp_version.c \
	ispt_api.c

endif

include $(NVT_HDAL_DIR)/driver.mk
