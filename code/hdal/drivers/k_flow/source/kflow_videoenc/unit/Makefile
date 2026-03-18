MODULE_NAME = kflow_videoenc
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR =$(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR =$(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
KDRV_COMM_INC_PATH = $(KDRV_DIR)/include/comm
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(KDRV_COMM_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DVDOENC_LL
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/source/kflow_videoenc/unit/include_protected

# This line should be removed later ( now still use dal_h26x API & nvt_h26x.ko first, will use kdrv API later )
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_videocodec/include

# Need to include h265_def.h for tile rule
#EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_videocodec/h265/api

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_builtin/nvt_vdocdc/h265/api

EXTRA_CFLAGS += \
	-Wno-missing-braces \
	-DDEBUG \

ifeq ($(CONFIG_NVT_HDAL_KFLOW_VIDEOENC),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KFLOW_VIDEOENC)
endif

SRC += \
	linux/isf_vdoenc_main.c \
	linux/isf_vdoenc_proc.c \
	linux/isf_vdoenc_drv.c \
	mp_vcodec_mjpg/mp_mjpg_encoder.c \
	mp_vcodec_mjpg/mp_mjpg_encoder_cert.c \
	mp_vcodec_h264/mp_h264_encoder.c \
	mp_vcodec_h264/mp_h264_encoder_obj.c \
	mp_vcodec_h264/mp_h264_encoder_cert.c \
	mp_vcodec_h265/mp_h265_encoder.c \
	mp_vcodec_h265/mp_h265_encoder_obj.c \
	mp_vcodec_h265/mp_h265_encoder_cert.c \
	nmediarec_vdoenc/nvt_media_interface.c \
	nmediarec_vdoenc/nmediarec_vdoenc_tsk.c \
	nmediarec_vdoenc/nmediarec_vdotrig_tsk.c \
	nmediarec_vdoenc/nmediarec_vdoenc_platform.c \
	nmediarec_vdoenc/nmediarec_vdoenc_export.c \
	nmediarec_vdoenc/nmediarec_vdoenc_cert.c \
	isf_vdoenc/isf_vdoenc.c \
	isf_vdoenc/isf_vdoenc_input_mask.c \
	isf_vdoenc/isf_vdoenc_input_osd.c \
	isf_vdoenc/isf_vdoenc_debug.c \
	isf_vdoenc/isf_vdoenc_verinfo.c \
	isf_vdoenc/isf_vdoenc_platform.c \
	isf_vdoenc/isf_vdoenc_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += -DVDOENC_LL
EXTRA_CFLAGS += \
	-DDEBUG \
	-Wno-format \

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/source/kflow_videoenc/unit/include_protected

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_videocodec/include

# Need to include h265_def.h for tile rule
#EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_videocodec/h265/api

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_builtin/nvt_vdocdc/h265/api

EXTRA_INCLUDE += -I$(KDRV_DIR)/source/include

SRC += \
	rtos/isf_vdoenc_drv.c \
	rtos/isf_vdoenc_proc.c \
	mp_vcodec_mjpg/mp_mjpg_encoder.c \
	mp_vcodec_mjpg/mp_mjpg_encoder_cert.c\
	mp_vcodec_h264/mp_h264_encoder.c \
	mp_vcodec_h264/mp_h264_encoder_obj.c \
	mp_vcodec_h264/mp_h264_encoder_cert.c \
	mp_vcodec_h265/mp_h265_encoder.c \
	mp_vcodec_h265/mp_h265_encoder_obj.c \
	mp_vcodec_h265/mp_h265_encoder_cert.c \
	nmediarec_vdoenc/nvt_media_interface.c \
	nmediarec_vdoenc/nmediarec_vdoenc_tsk.c \
	nmediarec_vdoenc/nmediarec_vdotrig_tsk.c \
	nmediarec_vdoenc/nmediarec_vdoenc_platform.c \
	nmediarec_vdoenc/nmediarec_vdoenc_cert.c \
	isf_vdoenc/isf_vdoenc.c \
	isf_vdoenc/isf_vdoenc_input_mask.c \
	isf_vdoenc/isf_vdoenc_input_osd.c \
	isf_vdoenc/isf_vdoenc_debug.c \
	isf_vdoenc/isf_vdoenc_verinfo.c \
	isf_vdoenc/isf_vdoenc_platform.c \
	isf_vdoenc/isf_vdoenc_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
