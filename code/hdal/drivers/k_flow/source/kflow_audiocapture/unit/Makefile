MODULE_NAME = kflow_audiocap
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
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_DIR)/source/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-Wno-missing-braces \
	-DDEBUG \

ifeq ($(CONFIG_NVT_HDAL_KFLOW_AUDIOCAPTURE),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KFLOW_AUDIOCAPTURE)
endif

SRC += \
	wavstudio/wavstudio_api.c \
	wavstudio/wavstudio_aud_intf.c \
	wavstudio/wavstudio_id.c \
	wavstudio/wavstudio_playtsk.c \
	wavstudio/wavstudio_playtsk2.c \
	wavstudio/wavstudio_recordtsk.c \
	wavstudio/wavstudio_util.c \
	wavstudio/wavstudio_sxcmd.c \
	wavstudio/ctl_aud.c \
	wavstudio/wavstudio_cert.c \
	nvtaec/nvtaec.c \
	linux/isf_audcap_main.c \
	linux/isf_audcap_proc.c \
	linux/isf_audcap_drv.c \
	isf_audcap/isf_audcap.c \
	isf_audcap/isf_audcap0.c \
	isf_audcap/isf_audcap_verinfo.c \
	isf_audcap/isf_audcap_api.c \
	isf_audcap/isf_audcap_debug.c \
	isf_audcap/isf_audcap_export.c \
	isf_audcap/isf_audcap_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-Wno-format \

SRC += \
	wavstudio/wavstudio_api.c \
	wavstudio/wavstudio_aud_intf.c \
	wavstudio/wavstudio_id.c \
	wavstudio/wavstudio_playtsk.c \
	wavstudio/wavstudio_playtsk2.c \
	wavstudio/wavstudio_recordtsk.c \
	wavstudio/wavstudio_util.c \
	wavstudio/wavstudio_sxcmd.c \
	wavstudio/ctl_aud.c \
	wavstudio/wavstudio_cert.c \
	nvtaec/nvtaec.c \
	isf_audcap/isf_audcap.c \
	isf_audcap/isf_audcap0.c \
	isf_audcap/isf_audcap_api.c \
	isf_audcap/isf_audcap_debug.c \
	isf_audcap/isf_audcap_cert.c \
	rtos/isf_audcap_drv.c \
	rtos/isf_audcap_proc.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
