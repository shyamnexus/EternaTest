MODULE_NAME = kdrv_gfx2d
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/include/kdrv_gfx2d \
	-I$(CURR_DRIVER_DIR)/source/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_gfx2d/kdrv_grph/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_gfx2d/kdrv_grph_if/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_gfx2d/kdrv_hwcopy/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_gfx2d/kdrv_ise/eng/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_gfx2d/kdrv_ise/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_gfx2d/kdrv_ise/eng/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_gfx2d/kdrv_rotation/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \
	-D_GROUP_KO_ \


ifeq ($(CONFIG_NVT_HDAL_KDRV_GFX2D),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_GFX2D)
endif

SRC += \
	kdrv_gfx2d_main.c \
	kdrv_ise/ise_drv_main.c \
	kdrv_ise/ise_drv_init.c \
	kdrv_ise/ise_drv_proc.c \
	kdrv_ise/ise_drv_proc_api.c \
	kdrv_ise/ise_drv_util.c \
	kdrv_ise/ise_drv_ctl.c \
	kdrv_ise/ise_drv_ll.c \
	kdrv_ise/ise_drv_sca.c \
	kdrv_ise/ise_drv_flow.c \
	kdrv_ise/ise_drv_tsk.c \
	kdrv_ise/ise_drv_mm.c \
	kdrv_ise/ise_drv_export.c \
	kdrv_ise/ise_drv_ver.c \
	kdrv_ise/kdrv_ise_ctl.c \
	kdrv_ise/eng/ise_eng.c \
	kdrv_ise/eng/ise_eng_base.c \
	kdrv_ise/eng/ise_eng_cert.c \
	kdrv_ise/eng/ise_eng_platform.c \
	kdrv_gfx_if/kdrv_gfx_if.c \
	kdrv_gfx_if/kdrv_gfx_if_export.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

EXTRA_CFLAGS += -DDEBUG

SRC += \
	kdrv_gfx2d_main.c \
	kdrv_grph/grph.c \
	kdrv_grph/grph_cert.c \
	kdrv_grph/grph_jobs.c \
	kdrv_grph/grph_platform.c \
	kdrv_grph/emu_grph.c \
	kdrv_grph/kdrv_graphic.c \
	kdrv_ise/ise_drv_init.c \
	kdrv_ise/ise_drv_proc_api.c \
	kdrv_ise/ise_drv_util.c \
	kdrv_ise/ise_drv_ctl.c \
	kdrv_ise/ise_drv_ll.c \
	kdrv_ise/ise_drv_sca.c \
	kdrv_ise/ise_drv_flow.c \
	kdrv_ise/ise_drv_tsk.c \
	kdrv_ise/ise_drv_mm.c \
	kdrv_ise/ise_drv_ver.c \
	kdrv_ise/kdrv_ise_ctl.c \
	kdrv_ise/eng/ise_eng.c \
	kdrv_ise/eng/ise_eng_cert.c \
	kdrv_ise/eng/ise_eng_base.c \
	kdrv_ise/eng/ise_eng_platform.c \
	kdrv_gfx_if/kdrv_gfx_if.c

endif

include $(NVT_HDAL_DIR)/driver.mk
