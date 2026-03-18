MODULE_NAME = kflow_audiodec
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/source/kflow_audiodec/unit/include_protected
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/kdrv_audioio

EXTRA_INCLUDE += -I$(KDRV_DIR)/source/kdrv_audioio

EXTRA_INCLUDE += -I$(KFLOW_INC_PATH)/kflow_videoenc

EXTRA_CFLAGS += \
	-Wno-missing-braces \
	-DDEBUG \

SRC += \
	linux/isf_auddec_main.c \
	linux/isf_auddec_proc.c \
	linux/isf_auddec_drv.c \
	mp_decode_pcm/mp_pcm_decoder.c \
	mp_decode_pcm/mp_pcm_decoder_obj.c \
	nmediaplay_auddec/nmediaplay_auddec_tsk.c \
	nmediaplay_auddec/nvt_media_interface.c \
	isf_auddec/isf_auddec.c \
	isf_auddec/isf_auddec_debug.c \
	isf_auddec/isf_auddec_verinfo.c \
	isf_auddec/isf_auddec_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/kdrv_audioio

EXTRA_INCLUDE += -I$(KDRV_DIR)/source/kdrv_audioio

EXTRA_INCLUDE += -I$(KFLOW_INC_PATH)/kflow_videoenc

EXTRA_CFLAGS += \
	-DDEBUG \
	-Wno-format \

SRC += \
	mp_decode_pcm/mp_pcm_decoder.c \
	mp_decode_pcm/mp_pcm_decoder_obj.c \
	nmediaplay_auddec/nmediaplay_auddec_tsk.c \
	nmediaplay_auddec/nvt_media_interface.c \
	isf_auddec/isf_auddec.c \
	isf_auddec/isf_auddec_debug.c \
	isf_auddec/isf_auddec_verinfo.c \
	rtos/isf_auddec_drv.c \
	rtos/isf_auddec_proc.c \
	isf_auddec/isf_auddec_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
