#
# Makefile for the hdal built-in
#

VOS_DRIVER_SRC_DIR = vos/drivers/source
KDRV_SRC_DIR = hdal/drivers/k_driver/source
KFLOW_SRC_DIR = hdal/drivers/k_flow/source
VENDOR_ISP_DIR = hdal/vendor/isp/drivers/source
VENDOR_MEDIA_DIR = hdal/vendor/media/drivers/source

# to avoid make clean error (when make cleaning, CONFIG_NVT_VOS_KWRAP != y)
ifeq ($(CONFIG_NVT_VOS_KWRAP),y)
obj-$(CONFIG_NVT_VOS_KWRAP) += $(VOS_DRIVER_SRC_DIR)/kwrap/
endif

# to avoid make clean error (when make cleaning, CONFIG_NVT_HDAL_KDRV_BUILTIN != y)
ifeq ($(CONFIG_NVT_HDAL_KDRV_BUILTIN),y)
# special case for ns02302 between 538 and 539a series to use different ipp
ifneq ($(CONFIG_NVT_HDAL_BUILT_IN_WITH_539A_IPP),y)
obj-$(CONFIG_NVT_HDAL_KDRV_BUILTIN) += $(KDRV_SRC_DIR)/kdrv_builtin/
else
obj-$(CONFIG_NVT_HDAL_KDRV_BUILTIN) += $(KDRV_SRC_DIR)/kdrv_builtin_539a/
endif
obj-$(CONFIG_NVT_HDAL_KDRV_COMM) += $(KDRV_SRC_DIR)/comm/
obj-$(CONFIG_NVT_HDAL_KDRV_GFX2D) += $(KDRV_SRC_DIR)/kdrv_gfx2d/
obj-$(CONFIG_NVT_HDAL_KDRV_VIDEOCAPTURE) += $(KDRV_SRC_DIR)/kdrv_videocapture/
ifneq ($(CONFIG_NVT_HDAL_BUILT_IN_WITH_539A_IPP),y)
obj-$(CONFIG_NVT_HDAL_KDRV_VIDEOPROCESS) += $(KDRV_SRC_DIR)/kdrv_videoprocess/
else
# special case for ns02302 between 538 and 539a series to use different ipp
obj-$(CONFIG_NVT_HDAL_KDRV_VIDEOPROCESS) += $(KDRV_SRC_DIR)/kdrv_videoprocess_539a/
endif
obj-$(CONFIG_NVT_HDAL_KDRV_DRE) += $(KDRV_SRC_DIR)/kdrv_videoprocess/kdrv_dre/
obj-$(CONFIG_NVT_HDAL_KDRV_VPE) += $(KDRV_SRC_DIR)/kdrv_videoprocess/kdrv_vpe/
obj-$(CONFIG_NVT_HDAL_KDRV_VIDEOCODEC) += $(KDRV_SRC_DIR)/kdrv_videocodec/
obj-$(CONFIG_NVT_HDAL_KDRV_VIDEOJPEG) += $(KDRV_SRC_DIR)/kdrv_videojpeg/
obj-$(CONFIG_NVT_HDAL_KDRV_IDE) += $(KDRV_SRC_DIR)/kdrv_videoout/ide/
obj-$(CONFIG_NVT_HDAL_KDRV_VIDEOOUT) += $(KDRV_SRC_DIR)/kdrv_videoout/display_obj/
obj-$(CONFIG_NVT_HDAL_KDRV_MI) += $(KDRV_SRC_DIR)/kdrv_videoout/mi/
obj-$(CONFIG_NVT_HDAL_KDRV_DSI) += $(KDRV_SRC_DIR)/kdrv_videoout/dsi/
obj-$(CONFIG_NVT_HDAL_KDRV_DSIPHY) += $(KDRV_SRC_DIR)/kdrv_videoout/dsiphy/
obj-$(CONFIG_NVT_HDAL_KDRV_AUDIO_COMMON) += $(KDRV_SRC_DIR)/kdrv_audioio/audio_common/
obj-$(CONFIG_NVT_HDAL_KDRV_AUDIO) += $(KDRV_SRC_DIR)/kdrv_audioio/kdrv_audio/
obj-$(CONFIG_NVT_HDAL_KFLOW_COMMON) += $(KFLOW_SRC_DIR)/kflow_common/
obj-$(CONFIG_NVT_HDAL_KFLOW_VIDEOCAPTURE) += $(KFLOW_SRC_DIR)/kflow_videocapture/
obj-$(CONFIG_NVT_HDAL_KFLOW_VIDEOSPRITE) += $(KFLOW_SRC_DIR)/kflow_gfx/videosprite/
ifneq ($(CONFIG_NVT_HDAL_BUILT_IN_WITH_539A_IPP),y)
obj-$(CONFIG_NVT_HDAL_KFLOW_VIDEOPROCESS) += $(KFLOW_SRC_DIR)/kflow_videoprocess/
else
# special case for ns02302 between 538 and 539a series to use different ipp
obj-$(CONFIG_NVT_HDAL_KFLOW_VIDEOPROCESS) += $(KFLOW_SRC_DIR)/kflow_videoprocess_539a/
endif
obj-$(CONFIG_NVT_HDAL_KFLOW_VIDEOENC) += $(KFLOW_SRC_DIR)/kflow_videoenc/unit/
obj-$(CONFIG_NVT_HDAL_KFLOW_VIDEOOUT) += $(KFLOW_SRC_DIR)/kflow_videoout/unit/
obj-$(CONFIG_NVT_HDAL_KFLOW_GFX2D) += $(KFLOW_SRC_DIR)/kflow_gfx/
obj-$(CONFIG_NVT_HDAL_KFLOW_AUDIOCAPTURE) += $(KFLOW_SRC_DIR)/kflow_audiocapture/unit/
obj-$(CONFIG_NVT_HDAL_KFLOW_AUDIOOUT) += $(KFLOW_SRC_DIR)/kflow_audioout/unit/
obj-$(CONFIG_NVT_HDAL_KDRV_VIDEOCODEC) += $(VENDOR_MEDIA_DIR)/nvt_vencrc/
ifneq ($(CONFIG_NVT_HDAL_BUILT_IN_WITH_539A_IPP),y)
obj-$(CONFIG_NVT_HDAL_ISP_ISP) += $(VENDOR_ISP_DIR)/isp/
else
# special case for ns02302 between 538 and 539a series to use different ipp
obj-$(CONFIG_NVT_HDAL_ISP_ISP) += $(VENDOR_ISP_DIR)/isp_539a/
endif
obj-$(CONFIG_NVT_HDAL_ISP_AE) += $(VENDOR_ISP_DIR)/ae/
obj-$(CONFIG_NVT_HDAL_ISP_AWB) += $(VENDOR_ISP_DIR)/awb/
ifneq ($(CONFIG_NVT_HDAL_BUILT_IN_WITH_539A_IPP),y)
obj-$(CONFIG_NVT_HDAL_ISP_IQ) += $(VENDOR_ISP_DIR)/iq/
else
# special case for ns02302 between 538 and 539a series to use different ipp
obj-$(CONFIG_NVT_HDAL_ISP_IQ) += $(VENDOR_ISP_DIR)/iq_539a/
endif
obj-$(CONFIG_NVT_HDAL_ISP_VPE) += $(VENDOR_ISP_DIR)/vpe/
endif

