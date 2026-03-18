MODULE_NAME = kdrv_trke
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
CURR_DRV_INC_PATH = $(VENDOR_KDRV_INC_PATH)/kdrv_trke
CURR_DRV_INT_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_trke/include
SSDRV_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_trke/eng/include
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
	trke_drv_main.c \
	trke_drv_init.c \
	trke_drv_proc.c \
	trke_drv_proc_api.c \
	trke_drv_util.c \
	trke_drv_ctl.c \
	trke_drv_ll.c \
	trke_drv_flow.c \
	trke_drv_tsk.c \
	trke_drv_mm.c \
	trke_drv_export.c \
	trke_drv_ver.c \
	trke_drv_ioctl.c \
	trke_drv_layer_proc.c \
	kdrv_trke_ctl.c \
	eng/trke_eng.c \
	eng/trke_eng_base.c \
	eng/trke_eng_dma_base.c \
	eng/trke_eng_platform.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

SRC += \
	trke_drv_init.c \
	trke_drv_proc_api.c \
	trke_drv_util.c \
	trke_drv_ctl.c \
	trke_drv_ll.c \
	trke_drv_flow.c \
	trke_drv_tsk.c \
	trke_drv_mm.c \
	trke_drv_ver.c \
	trke_drv_ioctl.c \
	trke_drv_layer_proc.c \
	kdrv_trke_ctl.c \
	eng/trke_eng.c \
	eng/trke_eng_base.c \
	eng/trke_eng_dma_base.c \
	eng/trke_eng_platform.c

endif

include $(NVT_HDAL_DIR)/driver.mk
