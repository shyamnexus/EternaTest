MODULE_NAME = kdrv_h26x
# DEFINE
VSIM = no
VEMU = no
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
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \

EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(CURR_DRIVER_DIR)/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocodec \
	-I$(CURR_DRIVER_DIR)/source/include \

EXTRA_INCLUDE += -I$(LIBRARY_DIR)/include

EXTRA_INCLUDE += -I$(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/kdrv_videoenc

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/kdrv_videodec

vpu_dir = $(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/vpu
vrc_dir = $(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/vrc
h26x_dir = $(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/h26x
h264e_dir = $(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/h264
h264d_dir = $(CURR_DRIVER_DIR)/source/kdrv_videocodec/h264
h265e_dir = $(CURR_DRIVER_DIR)/source/kdrv_builtin/nvt_vdocdc/h265
h265d_dir = $(CURR_DRIVER_DIR)/source/kdrv_videocodec/h265
plat_dir = $(CURR_DRIVER_DIR)/source/kdrv_videocodec/plat_ipc
EXTRA_INCLUDE += \
	-I$(vpu_dir)/inc \
	-I$(vrc_dir)/inc \
	-I$(h26x_dir)/inc \
	-I$(h26x_dir)/api \

EXTRA_INCLUDE += \
	-I$(h264e_dir)/inc \
	-I$(h264e_dir)/api \
	-I$(h265e_dir)/inc \
	-I$(h265e_dir)/api \

EXTRA_INCLUDE += \
	-I$(h264d_dir)/inc \
	-I$(h264d_dir)/api \
	-I$(h265d_dir)/inc \
	-I$(h265d_dir)/api \

EXTRA_INCLUDE += -I$(plat_dir)/inc

ifeq ($(VEMU),no)
sim_dir = $(CURR_DRIVER_DIR)/source/kdrv_videocodec/sim_ipc
emode_dir = $(CURR_DRIVER_DIR)/source/kdrv_videocodec/emode
EXTRA_INCLUDE += \
	-I$(sim_dir)/inc \
	-I$(emode_dir)/inc \
	-I./include \
	-I$(NVT_HDAL_DIR)/vendor/media/drivers/include/nvt_vencrc \

else
emu_dir = $(CURR_DRIVER_DIR)/source/kdrv_videocodec/emu
EXTRA_INCLUDE += -I$(emu_dir)/inc

endif
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DVDOCDC_LL

EXTRA_CFLAGS += -DDEBUG

h264-src += \
	h264/src/h264dec_api.c \
	h264/src/h264dec_header.c \
	h264/src/h264dec_int.c \
	h264/src/h264dec_cfg.c \

h265-src += \
	h265/src/h265dec_api.c \
	h265/src/h265dec_header.c \
	h265/src/h265dec_int.c \
	h265/src/h265dec_cfg.c \

#plat-src := plat_ipc/src/kdrv_vdocdc_main.c plat_ipc/src/kdrv_vdocdc_dbg.c plat_ipc/src/kdrv_vdocdc_ioctl.c plat_ipc/src/kdrv_vdocdc_emode.c plat_ipc/src/kdrv_videoenc.c plat_ipc/src/kdrv_vdocdc_proc.c plat_ipc/src/kdrv_vdocdc_api.c plat_ipc/src/kdrv_vdocdc_thread.c plat_ipc/src/kdrv_vdocdc_export.c
plat-src := plat_ipc/src/kdrv_vdocdc_main.c plat_ipc/src/kdrv_vdocdc_dbg.c plat_ipc/src/kdrv_vdocdc_ioctl.c plat_ipc/src/kdrv_vdocdc_emode.c plat_ipc/src/kdrv_videoenc.c plat_ipc/src/kdrv_videodec.c plat_ipc/src/kdrv_vdocdc_proc.c plat_ipc/src/kdrv_vdocdc_api.c plat_ipc/src/kdrv_vdocdc_export.c plat_ipc/src/kdrv_vdocdc_thread.c
emode-src := emode/src/vemd_cfg.c emode/src/vemd_api.c
ifeq ($(VSIM),yes)
EXTRA_CFLAGS += -DVDOCDC_SIM
sim-src := sim_ipc/src/sim_vdocdc_mem.c sim_ipc/src/sim_vdocdc_file.c sim_ipc/src/sim_avc_main.c sim_ipc/src/sim_hevc_main.c
#sim-src += sim_jpeg_main.c
endif
SRC += $(h264-src) $(h265-src) $(sim-src) $(plat-src) $(emode-src)

ifeq ($(CONFIG_NVT_HDAL_KDRV_VIDEOCODEC),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_VIDEOCODEC)
endif

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += -DVDOCDC_LL

#SRC += $(wildcard $(h264d_dir)/src/*.c)
#SRC += $(wildcard $(h265d_dir)/src/*.c)
#SRC += $(wildcard $(emode_dir)/src/*.c)

SRC += \
        h264/src/h264dec_api.c \
        h264/src/h264dec_header.c \
        h264/src/h264dec_int.c \
        h264/src/h264dec_cfg.c \

SRC += \
        h265/src/h265dec_api.c \
        h265/src/h265dec_header.c \
        h265/src/h265dec_int.c \
        h265/src/h265dec_cfg.c \

ifeq ($(VEMU),no)
#SRC += $(wildcard $(emode_dir)/src/*.c)
SRC += emode/src/vemd_cfg.c emode/src/vemd_api.c
#SRC += $(plat_dir)/src/kdrv_videoenc.c $(plat_dir)/src/kdrv_vdocdc_dbg.c $(plat_dir)/src/kdrv_vdocdc_emode.c $(plat_dir)/src/kdrv_vdocdc_thread.c
SRC += $(plat_dir)/src/kdrv_videoenc.c $(plat_dir)/src/kdrv_videodec.c $(plat_dir)/src/kdrv_vdocdc_dbg.c $(plat_dir)/src/kdrv_vdocdc_emode.c $(plat_dir)/src/kdrv_vdocdc_thread.c
EXTRA_CFLAGS += -DDEBUG

ifeq ($(VSIM),yes)
EXTRA_CFLAGS += -DVDOCDC_SIM
#SRC += $(wildcard $(sim_dir)/src/*.c)
SRC += sim_ipc/src/sim_vdocdc_mem.c sim_ipc/src/sim_vdocdc_file.c sim_ipc/src/sim_avc_main.c sim_ipc/src/sim_hevc_main.c
endif
else
EXTRA_CFLAGS += -DVDOCDC_EMU
#SRC += $(wildcard $(emu_dir)/src/*.c)
SRC += \
       emu/src/emu_h26x_common.c \
       emu/src/emu_h26x_file.c \
       emu/src/emu_h26x_job.c \
       emu/src/emu_h26x_mem.c \
       emu/src/emu_h264_dec.c \
       emu/src/emu_h264_enc.c \
       emu/src/emu_h265_check.c \
       emu/src/emu_h265_dec.c \
       emu/src/emu_h265_enc.c \

SRC += $(plat_dir)/src/kdrv_vdocdc_dbg.c $(plat_dir)/src/kdrv_vdocdc_emode.c
EXTRA_CFLAGS += -I$(APP_DIR)/source/test/emulation
EXTRA_CFLAGS += -DDEBUG
endif

endif

include $(NVT_HDAL_DIR)/driver.mk
