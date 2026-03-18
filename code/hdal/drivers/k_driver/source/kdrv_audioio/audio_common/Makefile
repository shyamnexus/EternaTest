MODULE_NAME = nvt_audio
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_DIR)/source/include \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_DIR)/source/kdrv_builtin/audio/dai/include \
	-I$(KDRV_DIR)/source/kdrv_builtin/audio/eac/include \

EXTRA_INCLUDE += -I$(KDRV_DIR)/source/rtos_$(SDK_CODENAME)/include

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_KDRV_AUDIO_COMMON),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_AUDIO_COMMON)
endif

SRC += \
	audio_api.c \
	audio_main.c \
	audio_proc.c \
	audio_drv.c \
	Audio.c \
	audio_platform.c \
	audio_export.c \
	audio_i2s.c \
	audio_queue.c \
	audio_tstx1.c \
	audio_tstx2.c \
	audio_tstxlb.c \
	audio_tsrx.c \
	AudCodecEmbd/AudCodecEmbd.c \
	audio_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \

SRC += \
	Audio.c \
	audio_i2s.c \
	audio_queue.c \
	audio_tstx1.c \
	audio_tstx2.c \
	audio_tstxlb.c \
	audio_tsrx.c \
	AudCodecEmbd/AudCodecEmbd.c \
	audio_cert.c \
	#audlib_aec/aec.c \
	#audlib_aac/aud_aacd_api.c \
	#audlib_aac/aud_aace_api.c \
	#audlib_adpcm/ADPCM.c \
	#audlib_agc/agc.c \
	#audlib_anr/anr.c \
	#audlib_anr/fxpt_fft_2.c \
	#audlib_anr/fxpt_sqrt.c \
	#audlib_doa/doa.c \
	#audlib_doa/doa_fft.c \
	#audlib_doa/vad.c \
	#audlib_filt/audFilt.c \
	#audlib_g711/g711.c \
	#audlib_src/aud_src.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
