###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
SUBDIRS += \
	kdrv_builtin \
	kdrv_builtin_539a \
	comm \
	comm/dummy \
	comm/usb_chrgdet \
	comm/usb3dev \
	comm/tse \
	comm/sdp \
	comm/gyro_spi \
	comm/uvcp \
	kdrv_gfx2d \
	kdrv_videocapture \
	kdrv_videoprocess \
	kdrv_videoprocess_539a \
	kdrv_videoout/mi \
	kdrv_videoout/dsi \
	kdrv_videoout/ide \
	kdrv_videoout/dsiphy \
	kdrv_videoout/csi_tx \
	kdrv_videoout/display_obj \
	kdrv_videoout/fbdev \
	kdrv_videocodec \
	kdrv_videojpeg \
	kdrv_videojpeg/decoder_lib \
	kdrv_videoprocess/kdrv_vpe \
	kdrv_videoprocess/kdrv_dre \
	kdrv_audioio/audio_common \
	kdrv_audioio/kdrv_audio \
	kdrv_rpc \
	#comm/drvdump \
	#comm/usb2dev \
	#kdrv_videodec \
	#kdrv_videoprocess/kdrv_ipp_sim \
	#kdrv_audioio/audlib_aec \
	#kdrv_audioio/audlib_agc \
	#kdrv_audioio/audlib_anr \
	#kdrv_audioio/audlib_filt \
	#kdrv_audioio/audlib_src \
	#kdrv_audioio/audlib_adpcm \
	#kdrv_audioio/audlib_g711 \
	#kdrv_audioio/audlib_aac \
	#kdrv_audioio/audio_device/ac108 \
	#kdrv_audioio/audio_device/emu \


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

SUBDIRS := $(dir $(shell find . -name Makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)

RTOS_SUBDIRS := $(dir $(shell find rtos_$(SDK_CODENAME) -name Makefile))
RTOS_SUBDIRS := $(shell echo $(RTOS_SUBDIRS) | sed -e 's/\.\///g';)

FILTER_OUT = $(foreach v,$(2),$(if $(findstring $(1),$(v)),,$(v)))
SUBDIRS := $(call FILTER_OUT,rtos_ns,$(SUBDIRS)) $(RTOS_SUBDIRS)

ifeq ("${OLD_IPP}","ON")
SUBDIRS := $(call FILTER_OUT,kdrv_videoprocess,$(SUBDIRS))
else
SUBDIRS := $(call FILTER_OUT,emu_kdrv_vproc,$(SUBDIRS))
endif

endif

include $(NVT_HDAL_DIR)/drivers.mk

# dependency for multi-thread
comm: kdrv_builtin kdrv_builtin_539a
kdrv_gfx2d: comm
kdrv_videocapture: comm
kdrv_videoprocess: kdrv_gfx2d
kdrv_videoprocess_539a: kdrv_gfx2d
kdrv_videoout/display_obj: kdrv_videoout/mi kdrv_videoout/ide kdrv_videoout/dsi
kdrv_videoout/fbdev: kdrv_videoout/display_obj
kdrv_videoout/csi_tx: kdrv_videoout/dsiphy
kdrv_videoout/dsi: kdrv_videoout/dsiphy
kdrv_videocodec: comm
kdrv_videojpeg: comm
kdrv_videojpeg/decoder_lib: comm
kdrv_videoprocess/kdrv_vpe: kdrv_videoprocess kdrv_videoprocess_539a
kdrv_videoprocess/kdrv_dre: kdrv_videoprocess kdrv_videoprocess_539a
kdrv_audioio/audio_common: comm
kdrv_audioio/kdrv_audio: kdrv_audioio/audio_common
kdrv_rpc: comm
comm/gyro_spi: comm

