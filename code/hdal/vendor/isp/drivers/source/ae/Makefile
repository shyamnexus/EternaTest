MODULE_NAME = nvt_ae
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
PREBUILT_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers/source/ae
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
	-I$(ALG_ISP_DRV_INC_PATH)/source/ae \
	-I$(ALG_ISP_DRV_INC_PATH)/source/ae/ae_lib \
	-I$(ALG_ISP_DRV_INC_PATH)/source/ae/ae_param \
	-I$(ALG_ISP_DRV_INC_PATH)/source/ae/ae_task \
	-I$(ALG_ISP_DRV_INC_PATH)/source/ae/ae_ui \
	-I$(ALG_ISP_DRV_INC_PATH)/source/ae/ae_nnsc/

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
# to solve the 'make modules_install' got empty CONFIG_OLD_MCOUNT

EXTRA_CFLAGS += -DDEBUG

SRC += \
	ae_dbg.c \
	ae_main.c \
	ae_proc.c \
	ae_version.c \
	ae_lib/ae_alg.c \
	ae_lib/ae_curve.c \
	ae_lib/ae_dciris.c \
	ae_lib/ae_afd.c \
	ae_lib/ae_flow.c \
	ae_lib/ae_msg.c \
	ae_lib/aet_api.c \
	ae_lib/aet_cfg.c \
	ae_lib/aet_dtsi.c \
	ae_lib/ae_api.c \
	ae_param/ae_param_default.c \
	ae_task/ae_task.c \
	ae_task/ae_task_id.c \
	ae_ui/ae_ui.c \
	ae_nnsc/ae_nnsc.c

ifeq ($(wildcard $(SRC)),$(strip $(SRC)))
PREBUILT_INSTALL = \
	$($(MODULE_NAME)-objs) \
	Makefile \
	ae_dbg.c \
	ae_dbg.h \
	ae_main.c \
	ae_main.h \
	ae_proc.c \
	ae_proc.h \
	ae_version.c \
	ae_version.h \
	ae_param/ae_param_default.c \
	../../include/ae_alg.h \
	../../include/ae_ui.h \
	../../include/ae_nnsc.h \
	../../include/aet_api.h \

endif
###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

SRC += \
	ae_dbg.c \
	ae_sxcmd.c \
	ae_version.c \
	ae_lib/ae_alg.c \
	ae_lib/ae_curve.c \
	ae_lib/ae_dciris.c \
	ae_lib/ae_afd.c \
	ae_lib/ae_flow.c \
	ae_lib/ae_msg.c \
	ae_lib/aet_api.c \
	ae_lib/aet_cfg.c \
	ae_lib/aet_dtsi.c \
	ae_lib/ae_api.c \
	ae_param/ae_param_default.c \
	ae_task/ae_task.c \
	ae_task/ae_task_id.c \
	ae_ui/ae_ui.c \
	ae_nnsc/ae_nnsc.c

endif

include $(NVT_HDAL_DIR)/driver.mk
