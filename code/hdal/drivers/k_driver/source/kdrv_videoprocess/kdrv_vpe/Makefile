MODULE_NAME = kdrv_vpe
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KDRV_INC_COMM_PATH = $(KDRV_DIR)/include/comm
KDRV_INT_INC_PATH = $(KDRV_DIR)/source/include
CURR_DRV_INC_PATH = $(KDRV_INC_PATH)/kdrv_videoprocess/vpe
CURR_DRV_INT_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_videoprocess/kdrv_vpe/include
SSDRV_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_videoprocess/kdrv_vpe/eng/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRV_INT_INC_PATH) \
	-I$(KDRV_INT_INC_PATH) \
	-I$(SSDRV_INC_PATH) \
	-I$(CURR_DRV_INC_PATH) \
	-I$(KDRV_INC_COMM_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -D__section_name__=$(MODULE_NAME)
EXTRA_CFLAGS += \
	-DDEBUG \
	-DKDRV_INTERFACE=1 \

ifeq ($(CONFIG_NVT_HDAL_KDRV_VPE),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_VPE)
endif

SRC += \
	vpe_drv_main.c \
	vpe_drv_init.c \
	vpe_drv_proc.c \
	vpe_drv_proc_api.c \
	vpe_drv_util.c \
	vpe_drv_ctl.c \
	vpe_drv_ll.c \
	vpe_drv_flow.c \
	vpe_drv_pipe.c \
	vpe_drv_sca.c \
	vpe_drv_shpen.c \
	vpe_drv_pal.c \
	vpe_drv_tsk.c \
	vpe_drv_mm.c \
	vpe_drv_column.c \
	vpe_drv_dce.c \
	vpe_drv_dctg.c \
	vpe_drv_cgain.c \
	vpe_drv_mask.c \
	vpe_drv_export.c \
	vpe_drv_ver.c \
	kdrv_vpe_ctl.c \
	eng/vpe_eng.c \
	eng/vpe_eng_int_platform.c \
	eng/vpe_eng_ctrl_base.c \
	eng/vpe_eng_dma_base.c \
	eng/vpe_eng_pal_base.c \
	eng/vpe_eng_sca_base.c \
	eng/vpe_eng_shp_base.c \
	eng/vpe_eng_int_column_cal.c \
	eng/vpe_eng_dce_base.c \
	eng/vpe_eng_mask_base.c	\
	eng/vpe_eng_cert.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

EXTRA_CFLAGS += \
	-DDEBUG \
	-DKDRV_INTERFACE=1 \

SRC += \
	eng/vpe_eng.c \
	eng/vpe_eng_int_platform.c \
	eng/vpe_eng_ctrl_base.c \
	eng/vpe_eng_dma_base.c \
	eng/vpe_eng_pal_base.c \
	eng/vpe_eng_sca_base.c \
	eng/vpe_eng_shp_base.c \
	eng/vpe_eng_int_column_cal.c \
	eng/vpe_eng_dce_base.c \
	eng/vpe_eng_mask_base.c \
	eng/vpe_eng_cert.c \
	vpe_drv_init.c \
	vpe_drv_proc_api.c \
	vpe_drv_util.c \
	vpe_drv_ctl.c \
	vpe_drv_ll.c \
	vpe_drv_flow.c \
	vpe_drv_pipe.c \
	vpe_drv_sca.c \
	vpe_drv_shpen.c \
	vpe_drv_pal.c \
	vpe_drv_tsk.c \
	vpe_drv_mm.c \
	vpe_drv_column.c \
	vpe_drv_dce.c \
	vpe_drv_dctg.c \
	vpe_drv_mask.c \
	vpe_drv_cgain.c \
	vpe_drv_ver.c \
	kdrv_vpe_ctl.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
