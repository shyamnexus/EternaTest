MODULE_NAME = kdrv_builtin
VEMU = no

# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
EXTRA_INCLUDE += \
	-I$(NVT_HDAL_DIR)/include \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/include/kdrv_builtin \
	-I$(KDRV_DIR)/include/kdrv_gfx2d \
	-I$(KDRV_DIR)/source/include \
	-I$(KDRV_DIR)/source/kdrv_videoprocess/kdrv_ime/eng/include \
	-I$(KDRV_DIR)/source/kdrv_audioio/kdrv_audio/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/ai \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/ipp/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/isp \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/sie/eng/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/vpu/inc \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/vrc/inc \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/h26x/inc \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/h26x/api \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/h264/inc \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/h264/api \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/h265/inc \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/h265/api \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/audio/dai/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/audio/eac/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_jpg/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videojpeg/decoder_lib \
	-I$(KDRV_DIR)/include/kdrv_ai \
	-I$(KDRV_DIR)/include/kdrv_videoenc \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/osg \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/kdrv_gfx2d/kdrv_grph/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin/kdrv_gfx2d/kdrv_grph_if/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -D_KDRV_BUILTIN_
ifeq ($(CONFIG_NVT_HDAL_KDRV_BUILTIN),y)
EXTRA_CFLAGS += -D__KDRV_BUILTIN_USE_INITCALL
endif
EXTRA_CFLAGS += \
	-Wno-sign-compare \
	-D_GROUP_KO_ \


ifeq ($(CONFIG_NVT_HDAL_KDRV_BUILTIN),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_BUILTIN)
endif

vpu-src += \
	nvt_vdocdc/vpu/src/h26x.c \
	nvt_vdocdc/vpu/src/vtrc.c \
	nvt_vdocdc/vpu/src/h26x_cert.c \

vrc-src += \
	nvt_vdocdc/vrc/src/h26xenc_rc.c \
	nvt_vdocdc/vrc/src/h26xenc_rate_control.c \

h26x-src += \
	nvt_vdocdc/h26x/src/h26x_bitstream.c \
	nvt_vdocdc/h26x/src/h26x_common.c \
	nvt_vdocdc/h26x/src/h26xenc_api.c \
	nvt_vdocdc/h26x/src/h26xenc_int.c \
	nvt_vdocdc/h26x/src/h26xenc_wrap.c \

h264-src += \
	nvt_vdocdc/h264/src/h264enc_api.c \
	nvt_vdocdc/h264/src/h264enc_header.c \
	nvt_vdocdc/h264/src/h264enc_int.c \
	nvt_vdocdc/h264/src/h264enc_wrap.c \

h265-src += \
	nvt_vdocdc/h265/src/h265enc_api.c \
	nvt_vdocdc/h265/src/h265enc_header.c \
	nvt_vdocdc/h265/src/h265enc_int.c \
	nvt_vdocdc/h265/src/h265enc_wrap.c \

h26xm-src += \
	nvt_vdocdc/nvt_vdocdc_export.c \
	nvt_vdocdc/nvt_vdocdc_platform.c \

ifeq ($(NVT_SDK_TYPE),NVT_IPC_SDK)
h26xm-src += \
	nvt_vdocdc/nvt_vdocdc_main.c \
	nvt_vdocdc/nvt_vdocdc_drv.c \
	nvt_vdocdc/nvt_vdocdc_dbg.c \

endif
jpeg-src += \
	nvt_jpg/nvt_jpeg_main.c \
	nvt_jpg/nvt_jpeg_proc.c \
	nvt_jpg/nvt_jpeg_drv.c \
	nvt_jpg/nvt_jpeg_api.c \
	nvt_jpg/nvt_jpeg_platform.c \
	nvt_jpg/nvt_jpeg.c \
	nvt_jpg/nvt_jpeg_queue.c \
	nvt_jpg/nvt_jpg_enc.c \
	nvt_jpg/nvt_jpg_header.c \
	nvt_jpg/nvt_jpeg_int.c \
	nvt_jpg/nvt_jpegbrc.c \
	nvt_jpg/nvt_jpg_parseheader.c \
	nvt_jpg/nvt_jpeg_kdrv.c \
	nvt_jpg/nvt_jpeg_cert.c \

kdrv_ai-src += \
	kdrv_ai/kdrv_ai_builtin.c

ifeq ($(shell if [ -d $(src)/isp/builtin ]; then echo y; else echo n; fi),y)
isp-builtin-dir += isp/builtin
else
isp-builtin-dir += isp
endif

isp-builtin-src = \
	$(isp-builtin-dir)/ae_builtin_nvt.c \
	$(isp-builtin-dir)/ae_builtin_alg.c \
	$(isp-builtin-dir)/ae_common.c \
	$(isp-builtin-dir)/awb_builtin_nvt.c \
	$(isp-builtin-dir)/awb_builtin_alg.c \
	$(isp-builtin-dir)/iq_lib_nvt.c

SRC += \
	kdrv_builtin_main.c \
	kdrv_builtin_util.c \
	kdrv_builtin_ver.c \
	bridge.c \
	fdtfast.c \
	unlzma.c \
	nvtmpp_init.c \
	nvtmpp_init_platform.c \
	isp/ae_builtin.c \
	isp/ae_dtsi.c \
	isp/ae_param.c \
	isp/awb_builtin.c \
	isp/awb_dtsi.c \
	isp/awb_param.c \
	isp/iq_builtin.c \
	isp/iq_dtsi.c \
	isp/iq_param.c \
	isp/isp_builtin.c \
	isp/sensor_builtin.c \
	isp/sensor_builtin_os04c10.c \
	ipp/ime_builtin_main.c \
	ipp/ime_builtin.c \
	ipp/kdrv_ipp_builtin.c \
	ipp/kdrv_ipp_builtin_platform.c \
	ipp/kdrv_ipp_builtin_dbg.c \
	ipp/kdrv_ipp_builtin_version.c \
	ipp/kdrv_ipp_builtin_cert.c \
	sie/eng/sie_eng.c \
	sie/eng/sie_eng_int.c \
	sie/eng/sie_eng_base.c \
	sie/eng/sie_eng_platform.c \
	sie/eng/sie_eng_cert.c \
	sie/fastboot/sie_init_main.c \
	sie/fastboot/sie_init.c \
	sie/fastboot/sie_init_platform.c \
	sie/fastboot/sie_export.c \
	vdoenc_builtin.c \
	vdoenc_builtin_export.c \
	vdoenc_builtin_platform.c \
	vdoenc_builtin_cert.c \
	vie/vie_main.c \
	vie/vie_drv.c \
	vie/vie_int.c \
	vie/vie_lib.c \
	vie/vie_platform.c \
	vie/vie_cert.c \
	audio/dai/dai.c \
	audio/dai/dai_api.c \
	audio/dai/dai_main.c \
	audio/dai/dai_drv.c \
	audio/dai/dai_proc.c \
	audio/dai/dai_platform.c \
	audio/dai/dai_export.c \
	audio/eac/eac.c \
	audio/eac/eac_api.c \
	audio/eac/eac_main.c \
	audio/eac/eac_drv.c \
	audio/eac/eac_proc.c \
	audio/eac/eac_platform.c \
	audio/eac/eac_export.c \
	audcap_builtin.c \
	audcap_builtin_platform.c \
	audcap_builtin_export.c \
	audcap_builtin_cert.c \
	osg/osg.c \
	osg/osg_demo.c \
	kdrv_gfx2d/kdrv_grph/graphic_builtin_main.c \
	kdrv_gfx2d/kdrv_grph/graphic_builtin_proc.c \
	kdrv_gfx2d/kdrv_grph/graphic_builtin_drv.c \
	kdrv_gfx2d/kdrv_grph/graphic_builtin_api.c \
	kdrv_gfx2d/kdrv_grph/grph_builtin.c \
	kdrv_gfx2d/kdrv_grph/grph_builtin_jobs.c \
	kdrv_gfx2d/kdrv_grph/grph_builtin_cert.c \
	kdrv_gfx2d/kdrv_grph/grph_builtin_platform.c \
	kdrv_gfx2d/kdrv_grph/emu_grph_builtin.c \
	kdrv_gfx2d/kdrv_grph/kdrv_builtin_graphic.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_main.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_init.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_proc.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_proc_api.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_util.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_ctl.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_flow.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_tsk.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_mm.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_export.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_ver.c \
	kdrv_gfx2d/kdrv_grph_if/kdrv_grph_ctl.c \

SRC += $(vpu-src)
SRC += $(vrc-src)
SRC += $(h26x-src)
SRC += $(h264-src)
SRC += $(h265-src)
SRC += $(h26xm-src)
SRC += $(jpeg-src)
SRC += $(isp-builtin-src)
SRC += $(kdrv_ai-src)

ifneq ($(CONFIG_NVT_HDAL_KDRV_BUILTIN),y)
SRC += \
	fdt/fdt.c \
	fdt/fdt_ro.c \
	fdt/fdt_wip.c \
	fdt/fdt_sw.c \
	fdt/fdt_rw.c \
	fdt/fdt_strerror.c \
	fdt/fdt_empty_tree.c \
	fdt/fdt_addresses.c \
	fdt/fdt_overlay.c \

endif

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += \
	-I$(KDRV_DIR)/include/plat \
	-I$(KDRV_DIR)/include \

EXTRA_CFLAGS += \
	-Wno-sign-compare \
	-DDEBUG \


ifeq ($(VEMU),yes)
EXTRA_CFLAGS += -DVDOCDC_EMU
endif
#EXTRA_CFLAGS += -DVDOCDC_LL
ifeq ($(shell if [ -d ./isp/builtin ]; then echo y; else echo n; fi),y)
isp-builtin-dir += isp/builtin
else
isp-builtin-dir += isp/prebuilt_rtos
endif

isp-builtin-src = \
	$(isp-builtin-dir)/ae_builtin_nvt.c \
	$(isp-builtin-dir)/ae_builtin_alg.c \
	$(isp-builtin-dir)/ae_common.c \
	$(isp-builtin-dir)/awb_builtin_nvt.c \
	$(isp-builtin-dir)/awb_builtin_alg.c \
	$(isp-builtin-dir)/iq_lib_nvt.c

SRC += \
	kdrv_builtin_main.c \
	kdrv_builtin_ver.c \
	nvtmpp_init.c \
	nvtmpp_init_platform.c \
	fdtfast.c \
	ipp/ime_builtin.c \
	ipp/kdrv_ipp_rtos.c \
	ipp/kdrv_ipp_builtin.c \
	ipp/kdrv_ipp_builtin_platform.c \
	ipp/kdrv_ipp_builtin_dbg.c \
	ipp/kdrv_ipp_builtin_version.c \
	ipp/kdrv_ipp_builtin_cert.c \
	isp/ae_builtin.c \
	isp/ae_dtsi.c \
	isp/ae_param.c \
	isp/awb_builtin.c \
	isp/awb_dtsi.c \
	isp/awb_param.c \
	isp/iq_builtin.c \
	isp/iq_dtsi.c \
	isp/iq_param.c \
	isp/isp_builtin.c \
	isp/sensor_builtin.c \
	isp/sensor_builtin_os04c10.c \
	audio/dai/dai.c \
	audio/eac/eac.c \
	unlzma.c \
	sie/eng/sie_eng.c \
	sie/eng/sie_eng_int.c \
	sie/eng/sie_eng_base.c \
	sie/eng/sie_eng_platform.c \
	sie/eng/sie_eng_cert.c \
	sie/fastboot/sie_init_main.c \
	sie/fastboot/sie_init.c \
	sie/fastboot/sie_init_platform.c \
	sie/fastboot/sie_export.c \
	vdoenc_builtin.c \
	vdoenc_builtin_platform.c \
	vdoenc_builtin_cert.c \
	vie/vie_int.c \
	vie/vie_lib.c \
	vie/vie_platform.c \
	vie/vie_cert.c \
	osg/osg.c \
	osg/osg_demo.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_init.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_proc_api.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_util.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_ctl.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_flow.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_tsk.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_mm.c \
	kdrv_gfx2d/kdrv_grph_if/grph_drv_ver.c \
	kdrv_gfx2d/kdrv_grph_if/kdrv_grph_ctl.c \

SRC += $(wildcard nvt_vdocdc/vpu/src/*.c)
SRC += $(wildcard nvt_vdocdc/h26x/src/*.c)
SRC += $(wildcard nvt_vdocdc/h264/src/*.c)
SRC += $(wildcard nvt_vdocdc/h265/src/*.c)
SRC += $(wildcard nvt_vdocdc/vrc/src/*.c)
SRC += \
	nvt_vdocdc/nvt_vdocdc_drv.c \
	nvt_vdocdc/nvt_vdocdc_dbg.c \

SRC_SKIP_BUILD = \
	kdrv_ai/ai_api.c \
	kdrv_ai/ai_lib.c \
	kdrv_ai/kdrv_ai_int.c \
	kdrv_ai/kdrv_ai_config.c \
	kdrv_ai/kdrv_ai_api.c \
	kdrv_ai/ai_ioctl.c \
	kdrv_ai/nue2/nue2_int.c \
	kdrv_ai/nue2/nue2_lib.c \
	kdrv_ai/nue2/nue2_platform.c \
	kdrv_ai/cnn/cnn_int.c \
	kdrv_ai/cnn/cnn_lib.c \
	kdrv_ai/cnn/cnn_platform.c \
	kdrv_ai/cnn/cnn_ll_cmd.c \
	kdrv_ai/nue/nue_ll_cmd.c \
	kdrv_ai/nue/nue_int.c \
	kdrv_ai/nue/nue_lib.c \
	kdrv_ai/nue/nue_platform.c \

SRC_SKIP_BUILD += $(wildcard nvt_jpg/*.c)
SRC += \
	nvt_jpg/nvt_jpeg.c \
	nvt_jpg/nvt_jpg_enc.c \
	nvt_jpg/nvt_jpeg_platform.c \
	nvt_jpg/nvt_jpg_header.c \
	nvt_jpg/nvt_jpeg_queue.c \
	nvt_jpg/nvt_jpegbrc.c \
	nvt_jpg/nvt_jpg_parseheader.c \
	nvt_jpg/nvt_jpeg_main.c \
	nvt_jpg/nvt_jpeg_int.c \
	nvt_jpg/nvt_jpeg_kdrv.c \
	nvt_jpg/nvt_jpeg_cert.c \

SRC += $(isp-builtin-src)

endif

include $(NVT_HDAL_DIR)/driver.mk
