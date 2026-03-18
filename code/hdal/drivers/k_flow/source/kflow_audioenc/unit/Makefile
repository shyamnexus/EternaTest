MODULE_NAME = kflow_audioenc
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/source/kflow_audioenc/unit/include_protected
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
	linux/isf_audenc_main.c \
	linux/isf_audenc_proc.c \
	linux/isf_audenc_drv.c \
	mp_encode_pcm/mp_pcm_encoder.c \
	mp_encode_pcm/mp_pcm_encoder_obj.c \
	mp_encode_ppcm/mp_ppcm_encoder.c \
	mp_encode_ppcm/mp_ppcm_encoder_obj.c \
	nmediarec_audenc/nmediarec_audenc_tsk.c \
	nmediarec_audenc/nvt_media_interface.c \
	isf_audenc/isf_audenc.c \
	isf_audenc/isf_audenc_debug.c \
	isf_audenc/isf_audenc_verinfo.c \
	isf_audenc/isf_audenc_cert.c \

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
	mp_encode_pcm/mp_pcm_encoder.c \
	mp_encode_pcm/mp_pcm_encoder_obj.c \
	mp_encode_ppcm/mp_ppcm_encoder.c \
	mp_encode_ppcm/mp_ppcm_encoder_obj.c \
	nmediarec_audenc/nmediarec_audenc_tsk.c \
	nmediarec_audenc/nvt_media_interface.c \
	isf_audenc/isf_audenc.c \
	isf_audenc/isf_audenc_debug.c \
	isf_audenc/isf_audenc_verinfo.c \
	rtos/isf_audenc_drv.c \
	rtos/isf_audenc_proc.c \
	isf_audenc/isf_audenc_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
