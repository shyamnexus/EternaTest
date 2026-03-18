MODULE_NAME = kdrv_ive
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/cv/drivers
VENDOR_KDRV_DIR = $(CURR_DRIVER_DIR)
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow/$(NVT_SDK_TYPE)
KFLOW_SHARED_DIR = $(NVT_HDAL_DIR)/drivers/k_flow/shared
RTOS_OUTPUT_DIR = $(CURR_DRIVER_DIR)/output
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
VENDOR_KDRV_INC_PATH = $(VENDOR_KDRV_DIR)/include
VENDOR_KDRV_INT_INC_PATH = $(VENDOR_KDRV_DIR)/source/include
CURR_DRV_INC_PATH = $(VENDOR_KDRV_INC_PATH)/kdrv_ive
CURR_DRV_INT_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_ive/include
SSDRV_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_ive/eng/include
KDRV_COMM_INC_PATH = $(KDRV_DIR)/include/comm
VG_MODULE_INC = $(NVT_KDRV_PATH)/source/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
KFLOW_SHARED_INC_PATH = $(KFLOW_SHARED_DIR)/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
# INC FLAGs
EXTRA_INCLUDE += -I$(KDRV_SRC_INC_PATH) -I$(KFLOW_INC_PATH) -I$(KFLOW_SHARED_INC_PATH) -I$(VG_MODULE_INC) -I$(VOS_INC_PATH) -I$(KDRV_INC_PATH) -I$(VENDOR_KDRV_INC_PATH) -I$(CURR_DRV_INT_INC_PATH) -I$(VENDOR_KDRV_INT_INC_PATH) -I$(SSDRV_INC_PATH) -I$(CURR_DRV_INC_PATH) -I$(KDRV_COMM_INC_PATH)


###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	ive_drv_main.c \
	ive_drv_init.c \
	ive_drv_proc.c \
	ive_drv_proc_api.c \
	ive_drv_util.c \
	ive_drv_ctl.c \
	ive_drv_ll.c \
	ive_drv_flow.c \
	ive_drv_tsk.c \
	ive_drv_mm.c \
	ive_drv_export.c \
	ive_drv_ver.c \
	ive_drv_ioctl.c \
	ive_drv_16to8.c \
	ive_drv_canny_edge.c \
	ive_drv_histo.c \
	ive_drv_ncc.c \
	ive_drv_dma_copy.c \
	ive_drv_thres_lut.c \
	ive_drv_img_op.c \
	ive_drv_st_corner.c \
	ive_drv_va.c \
	ive_drv_lbp.c \
	ive_drv_packed.c \
	kdrv_ive_ctl.c \
	eng/ive_eng.c \
	eng/ive_eng_base.c \
	eng/ive_eng_dma_base.c \
	eng/ive_eng_platform.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

SRC += \
	eng/ive_eng.c \
	eng/ive_eng_base.c \
	eng/ive_eng_dma_base.c \
	eng/ive_eng_platform.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
