MODULE_NAME = nvt_vencrc
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/media/drivers
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include/nvt_vencrc
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_DIR)/source/kdrv_builtin/nvt_vdocdc/vrc/inc \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_KDRV_VIDEOCODEC),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_VIDEOCODEC)
endif

ifeq ($(USR_RC_CFG),yes)
SRC += \
	nvt_vencrc_main_usr.c \
	main/h26xenc_rate_control_usr.c
	
else
SRC += \
	nvt_vencrc_main.c
endif

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
endif

include $(NVT_HDAL_DIR)/driver.mk
