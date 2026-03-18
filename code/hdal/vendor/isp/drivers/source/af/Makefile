MODULE_NAME = nvt_af
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
MTR_DRIVER_DIR =$(NVT_HDAL_DIR)/ext_devices/motor/mtr_common
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
PREBUILT_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers/source/af
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
	-I$(MTR_DRIVER_DIR) \
	-I$(ALG_ISP_DRV_INC_PATH)/include \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(ALG_ISP_DRV_INC_PATH)/source/af \
	-I$(ALG_ISP_DRV_INC_PATH)/source/af/af_lib \
	-I$(ALG_ISP_DRV_INC_PATH)/source/af/af_param \
	-I$(ALG_ISP_DRV_INC_PATH)/source/af/af_task \
	-I$(ALG_ISP_DRV_INC_PATH)/source/af/af_ui/

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
# to solve the 'make modules_install' got empty CONFIG_OLD_MCOUNT

EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_ISP_AE),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_ISP_AE)
endif


SRC += \
	af_main.c \
	af_proc.c \
	af_version.c \
	af_lib/af_alg.c \
	af_lib/af_flow.c \
	af_lib/af_msg.c \
	af_lib/aft_api.c \
	af_lib/aft_cfg.c \
	af_lib/aft_dtsi.c \
	af_param/af_param_default.c \
	af_task/af_task.c \
	af_task/af_task_id.c \
	af_ui/af_ui.c

ifeq ($(wildcard $(SRC)),$(strip $(SRC)))
PREBUILT_INSTALL = \
	$($(MODULE_NAME)-objs) \
	Makefile \
	af_dbg.h \
	af_main.c \
	af_main.h \
	af_proc.c \
	af_proc.h \
	af_version.c \
	af_version.h \
	af_param/af_param_default.c \
	../../include/af_alg.h \
	../../include/af_ui.h \
	../../include/aft_api.h \

endif
###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

SRC += \
	af_sxcmd.c \
	af_version.c \
	af_lib/af_alg.c \
	af_lib/af_flow.c \
	af_lib/af_msg.c \
	af_lib/aft_api.c \
	af_lib/aft_cfg.c \
	af_lib/aft_dtsi.c \
	af_param/af_param_default.c \
	af_task/af_task.c \
	af_task/af_task_id.c \
	af_ui/af_ui.c

endif

include $(NVT_HDAL_DIR)/driver.mk
