MODULE_NAME = nvt_awb
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
PREBUILT_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers/source/awb
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
ALG_ISP_DRV_INC_PATH=$(NVT_HDAL_DIR)/vendor/alg/isp/drivers
# INC FLAGs

EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_INC_PATH)/comm \
	-I$(KFLOW_INC_PATH) \
	-I$(ALG_ISP_DRV_INC_PATH)/include \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(ALG_ISP_DRV_INC_PATH)/source/awb \
	-I$(ALG_ISP_DRV_INC_PATH)/source/awb/awb_lib \
	-I$(ALG_ISP_DRV_INC_PATH)/source/awb/awb_param \
	-I$(ALG_ISP_DRV_INC_PATH)/source/awb/awb_task \
	-I$(ALG_ISP_DRV_INC_PATH)/source/awb/awb_ui \
	-I$(ALG_ISP_DRV_INC_PATH)/source/awb/awb_nnsc/

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
# to solve the 'make modules_install' got empty CONFIG_OLD_MCOUNT

EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_ISP_AWB),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_ISP_AWB)
endif

SRC += \
	awb_dbg.c \
	awb_main.c \
	awb_proc.c \
	awb_version.c \
	awb_lib/awb_alg.c \
	awb_lib/awb_msg.c \
	awb_lib/awb_flow.c \
	awb_lib/awb_flow_ext.c \
	awb_lib/awbt_api.c \
	awb_lib/awbt_cfg.c \
	awb_lib/awbt_dtsi.c \
	awb_param/awb_param_default.c \
	awb_task/awb_task.c \
	awb_task/awb_task_id.c \
	awb_ui/awb_ui.c \
	awb_nnsc/awb_nnsc.c

ifeq ($(wildcard $(SRC)),$(strip $(SRC)))
PREBUILT_INSTALL = \
	$($(MODULE_NAME)-objs) \
	Makefile \
	awb_dbg.c \
	awb_dbg.h \
	awb_main.c \
	awb_main.h \
	awb_proc.c \
	awb_proc.h \
	awb_version.c \
	awb_version.h \
	awb_param/awb_param_default.c \
	../../include/awb_alg.h \
	../../include/awb_ui.h \
	../../include/awb_nnsc.h \
	../../include/awbt_api.h \

endif
###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

MODPATH := $(shell echo $(PWD) | awk -F'source/' '{print $$NF}';)

SRC += \
	awb_dbg.c \
	awb_sxcmd.c \
	awb_version.c \
	awb_lib/awb_alg.c \
	awb_lib/awb_msg.c \
	awb_lib/awb_flow.c \
	awb_lib/awb_flow_ext.c \
	awb_lib/awbt_api.c \
	awb_lib/awbt_cfg.c \
	awb_lib/awbt_dtsi.c \
	awb_param/awb_param_default.c \
	awb_task/awb_task.c \
	awb_task/awb_task_id.c \
	awb_ui/awb_ui.c \
	awb_nnsc/awb_nnsc.c

endif

include $(NVT_HDAL_DIR)/driver.mk
