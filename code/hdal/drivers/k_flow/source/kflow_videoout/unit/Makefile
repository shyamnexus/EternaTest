MODULE_NAME = kflow_videoout
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_DIR)/source/include \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-Wno-missing-braces \
	-DDEBUG \

ifeq ($(CONFIG_NVT_HDAL_KFLOW_VIDEOOUT),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KFLOW_VIDEOOUT)
endif

SRC += \
	linux/isf_vdoout_main.c \
	linux/isf_vdoout_proc.c \
	linux/isf_vdoout_drv.c \
	isf_vdoout.c \
	isf_vdoout1.c \
	isf_vdoout0.c \
	isf_vdoout_wnd.c \
	isf_vdoout_api.c \
	isf_vdoout_debug.c \
	isf_vdoout_output_osd.c \
	isf_vdoout_output_mask.c \
	isf_vdoout_verinfo.c \
	isf_vdoout_platform.c \
	isf_vdoout_cert.c \
	vdodisp/vdodisp_tsk.c \
	vdodisp/vdodisp_id.c \
	vdodisp/vdodisp_int.c \
	vdodisp/vdodisp_cmd.c \
	vdodisp/vdodisp_sx_cmd.c \
	vdodisp/vdodisp_api.c \
	vdodisp/vdodisp_run.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
SRC += \
	rtos/isf_vdoout_proc.c \
	rtos/isf_vdoout_drv.c \
	isf_vdoout.c \
	isf_vdoout1.c \
	isf_vdoout0.c \
	isf_vdoout_wnd.c \
	isf_vdoout_api.c \
	isf_vdoout_debug.c \
	isf_vdoout_output_osd.c \
	isf_vdoout_output_mask.c \
	vdodisp/vdodisp_tsk.c \
	vdodisp/vdodisp_id.c \
	vdodisp/vdodisp_int.c \
	vdodisp/vdodisp_cmd.c \
	vdodisp/vdodisp_api.c \
	vdodisp/vdodisp_run.c \
	vdodisp/vdodisp_sx_cmd.c \
	isf_vdoout_verinfo.c \
	isf_vdoout_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
