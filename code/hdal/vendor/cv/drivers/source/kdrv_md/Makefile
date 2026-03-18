MODULE_NAME = kdrv_md
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/cv/drivers
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include/kdrv_md
CURR_KDRIVER_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_md/include
CURR_SSDRIVER_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_md/eng/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(CURR_KDRIVER_INC_PATH) \
	-I$(CURR_SSDRIVER_INC_PATH) \
	-I$(KDRV_INC_PATH)/comm

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	md_drv_main.c \
	md_drv_init.c \
	md_drv_proc.c \
	md_drv_proc_api.c \
	md_drv_util.c \
	md_drv_ctl.c \
	md_drv_ll.c \
	md_drv_flow.c \
	md_drv_tsk.c \
	md_drv_mm.c \
	md_drv_export.c \
	md_drv_ver.c \
	md_drv_ioctl.c \
	md_drv_motion_det.c \
	kdrv_md_ctl.c \
	kdrv_md_ctl.c \
	eng/md_eng.c \
	eng/md_eng_base.c \
	eng/md_eng_dma_base.c \
	eng/md_eng_platform.c


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

SRC += \
	md_drv_init.c \
	md_drv_proc_api.c \
	md_drv_util.c \
	md_drv_ctl.c \
	md_drv_ll.c \
	md_drv_flow.c \
	md_drv_tsk.c \
	md_drv_mm.c \
	md_drv_ver.c \
	md_drv_ioctl.c \
	md_drv_motion_det.c \
	kdrv_md_ctl.c \
	kdrv_md_ctl.c \
	eng/md_eng.c \
	eng/md_eng_base.c \
	eng/md_eng_dma_base.c \
	eng/md_eng_platform.c

endif

include $(NVT_HDAL_DIR)/driver.mk