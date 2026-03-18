MODULE_NAME = kflow_audioout
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/source/kflow_audioout/unit/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-Wno-missing-braces \
	-DDEBUG \

ifeq ($(CONFIG_NVT_HDAL_KFLOW_AUDIOOUT),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KFLOW_AUDIOOUT)
endif

SRC += \
	linux/isf_audout_main.c \
	linux/isf_audout_proc.c \
	linux/isf_audout_drv.c \
	isf_audout.c \
	isf_audout0.c \
	isf_audout1.c \
	isf_audout_verinfo.c \
	isf_audout_api.c \
	isf_audout_debug.c \
	isf_audout_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-Wno-format \

SRC += \
	isf_audout.c \
	isf_audout0.c \
	isf_audout1.c \
	isf_audout_api.c \
	isf_audout_debug.c\
	isf_audout_cert.c \
	rtos/isf_audout_drv.c \
	rtos/isf_audout_proc.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
