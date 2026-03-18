MODULE_NAME = nvt_iq_539a
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
ALG_ISP_DRIVER_DIR=$(NVT_HDAL_DIR)/vendor/alg/isp/drivers
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_INC_PATH)/comm \
	-I$(KFLOW_INC_PATH) \
	-I$(KDRV_DIR)/source/include \
	-I$(CURR_DRIVER_DIR)/include \
	-I$(CURR_DRIVER_DIR)/source/iq_539a \
	-I$(CURR_DRIVER_DIR)/source/iq_539a/iq_flow \
	-I$(CURR_DRIVER_DIR)/source/iq_539a/iq_lib \
	-I$(CURR_DRIVER_DIR)/source/iq_539a/iq_param \
	-I$(CURR_DRIVER_DIR)/source/iq_539a/iq_task \
	-I$(CURR_DRIVER_DIR)/source/iq_539a/iq_ui \
	-I$(CURR_DRIVER_DIR)/source/iq_539a/iq_nnsc \

ifneq ($(wildcard $(ALG_ISP_DRIVER_DIR)/source/iq_539a),)
ISP_IQ_PRBUILT_DIR = ../../../../alg/isp/drivers/source/iq_539a
else
ISP_IQ_PRBUILT_DIR = .
endif

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
# to solve the 'make modules_install' got empty CONFIG_OLD_MCOUNT

EXTRA_CFLAGS += -DDEBUG	-D_539A_

ifeq ($(CONFIG_NVT_HDAL_ISP_IQ)$(CONFIG_NVT_HDAL_BUILT_IN_WITH_539A_IPP),yy)
OBJ-TAG = $(CONFIG_NVT_HDAL_ISP_IQ)
endif

SRC += \
	iq_dbg.c \
	iq_main.c \
	iq_proc.c \
	iq_version.c \
	iq_flow/iq_alg.c \
	iq_flow/iq_msg.c \
	iq_flow/iq_common_param.c \
	iq_flow/iq_flow.c \
	iq_flow/iq_flow_ext.c \
	iq_flow/iqt_api.c \
	iq_flow/iqt_cfg.c \
	iq_flow/iqt_dtsi.c \
	iq_param/iq_param_default.c \
	iq_task/iq_task.c \
	iq_task/iq_task_id.c \
	iq_ui/iq_ui.c \
	iq_nnsc/iq_nnsc.c

SRC += \
	$(ISP_IQ_PRBUILT_DIR)/iq_lib/iq_lib.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

SRC += \
	iq_dbg.c \
	iq_sxcmd.c \
	iq_version.c \
	iq_flow/iq_alg.c \
	iq_flow/iq_msg.c \
	iq_flow/iq_common_param.c \
	iq_flow/iq_flow.c \
	iq_flow/iqt_api.c \
	iq_flow/iqt_cfg.c \
	iq_flow/iqt_dtsi.c \
	iq_param/iq_param_default.c \
	iq_task/iq_task.c \
	iq_task/iq_task_id.c \
	iq_ui/iq_ui.c \
	iq_nnsc/iq_nnsc.c

SRC += \
	$(ISP_IQ_PRBUILT_DIR)/iq_lib/iq_lib_rtos.c

endif

include $(NVT_HDAL_DIR)/driver.mk
