MODULE_NAME = kflow_videodec
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR =$(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR =$(NVT_HDAL_DIR)/drivers/k_flow
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

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DVDODEC_LL
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/source/kflow_videodec/unit/include_protected

# This line should be removed later ( now still use dal_h26x API & nvt_h26x.ko first, will use kdrv API later )
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_videocodec/include

EXTRA_CFLAGS += \
	-Wno-missing-braces \
	-DDEBUG \

SRC += \
	linux/isf_vdodec_main.c \
	linux/isf_vdodec_proc.c \
	linux/isf_vdodec_drv.c \
	mp_decode_mjpg/mp_mjpg_decoder.c \
	mp_decode_mjpg/mp_mjpg_decoder_cert.c \
	mp_decode_h264/mp_h264_decoder.c \
	mp_decode_h264/mp_h264_reorder.c \
	mp_decode_h264/mp_h264_decoder_obj.c \
	mp_decode_h264/mp_h264_decoder_cert.c \
	mp_decode_h265/mp_h265_decoder.c \
	mp_decode_h265/mp_h265_reorder.c \
	mp_decode_h265/mp_h265_decoder_obj.c \
	mp_decode_h265/mp_h265_decoder_cert.c \
	nmediaplay_vdodec/nmediaplay_vdodec_tsk.c \
	nmediaplay_vdodec/nvt_media_interface.c \
	nmediaplay_vdodec/nmediaplay_vdodec_platform.c \
	nmediaplay_vdodec/nmediaplay_vdodec_cert.c \
	isf_vdodec/isf_vdodec.c \
	isf_vdodec/isf_vdodec_debug.c \
	isf_vdodec/isf_vdodec_platform.c \
	isf_vdodec/isf_vdodec_verinfo.c \
	isf_vdodec/isf_vdodec_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += -DVDODEC_LL
EXTRA_CFLAGS += \
	-DDEBUG \
	-Wno-format \

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/source/kflow_videodec/unit/include_protected

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_videocodec/include

EXTRA_INCLUDE += -I$(KDRV_DIR)/source/include

SRC += \
	rtos/isf_vdodec_drv.c \
	rtos/isf_vdodec_proc.c \
	mp_decode_mjpg/mp_mjpg_decoder.c \
	mp_decode_mjpg/mp_mjpg_decoder_cert.c \
	mp_decode_h264/mp_h264_decoder.c \
	mp_decode_h264/mp_h264_reorder.c \
	mp_decode_h264/mp_h264_decoder_obj.c \
	mp_decode_h264/mp_h264_decoder_cert.c \
	mp_decode_h265/mp_h265_decoder.c \
	mp_decode_h265/mp_h265_reorder.c \
	mp_decode_h265/mp_h265_decoder_obj.c \
	mp_decode_h265/mp_h265_decoder_cert.c \
	nmediaplay_vdodec/nmediaplay_vdodec_tsk.c \
	nmediaplay_vdodec/nvt_media_interface.c \
	nmediaplay_vdodec/nmediaplay_vdodec_platform.c \
	nmediaplay_vdodec/nmediaplay_vdodec_cert.c \
	isf_vdodec/isf_vdodec.c \
	isf_vdodec/isf_vdodec_debug.c \
	isf_vdodec/isf_vdodec_verinfo.c \
	isf_vdodec/isf_vdodec_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
