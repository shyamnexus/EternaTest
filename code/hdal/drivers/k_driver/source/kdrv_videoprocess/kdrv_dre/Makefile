MODULE_NAME = kdrv_dre
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KDRV_INT_INC_PATH = $(KDRV_DIR)/source/include
CURR_DRV_INC_PATH = $(KDRV_INC_PATH)/kdrv_videoprocess/dre
CURR_DRV_INT_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_videoprocess/kdrv_dre/include
SSDRV_INC_PATH = $(CURR_DRIVER_DIR)/source/kdrv_videoprocess/kdrv_dre/eng/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRV_INT_INC_PATH) \
	-I$(KDRV_INT_INC_PATH) \
	-I$(SSDRV_INC_PATH) \
	-I$(CURR_DRV_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -D__section_name__=$(MODULE_NAME)
EXTRA_CFLAGS += \
	-DDEBUG \
	-D_GROUP_KO_ \
	-DKDRV_INTERFACE=1 \

ifeq ($(CONFIG_NVT_HDAL_KDRV_DRE),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_DRE)
endif

SRC += \
	dre_drv_main.c \
	dre_drv_init.c \
	dre_drv_proc.c \
	dre_drv_proc_api.c \
	dre_drv_util.c \
	dre_drv_ctl.c \
	dre_drv_ll.c \
	dre_drv_flow.c \
	dre_drv_tsk.c \
	dre_drv_mm.c \
	dre_drv_fusion.c \
	dre_drv_export.c \
	dre_drv_ver.c \
	kdrv_dre_ctl.c \
	eng/dre_eng.c \
	eng/dre_eng_int.c \
	eng/dre_eng_int_platform.c \
	eng/dre_eng_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

EXTRA_CFLAGS += \
	-DDEBUG \
	-D_GROUP_KO_ \
	-DKDRV_INTERFACE=1 \


SRC += \
	dre_drv_init.c \
	dre_drv_proc_api.c \
	dre_drv_util.c \
	dre_drv_ctl.c \
	dre_drv_ll.c \
	dre_drv_flow.c \
	dre_drv_tsk.c \
	dre_drv_mm.c \
	dre_drv_fusion.c \
	dre_drv_ver.c \
	kdrv_dre_ctl.c \
	eng/dre_eng.c \
	eng/dre_eng_int.c \
	eng/dre_eng_int_platform.c \
	eng/dre_eng_cert.c \


endif

include $(NVT_HDAL_DIR)/driver.mk
