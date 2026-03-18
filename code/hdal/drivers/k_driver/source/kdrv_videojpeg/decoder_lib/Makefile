MODULE_NAME = kdrv_jpgdec
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
UTIL_INC_PATH = $(KDRV_DIR)/include/comm/util
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH)/kdrv_videodec \
	-I$(UTIL_INC_PATH) \
	-I$(KDRV_DIR)/source/kdrv_builtin/nvt_jpg/include \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	djpeg.c \
	   jdmarker.c \
	   jdapimin.c \
	   jdinput.c \
	   jutils.c \
	   jcomapi.c \
	   wrbmp.c \
	   jmemmgr.c \
	   jmemnobs.c \
	   jerror.c \
	   jdapistd.c \
	   jdatasrc.c \
	   jdmaster.c \
	   jdsample.c \
	   jdarith.c \
	   jdhuff.c \
	   jdcoefct.c \
	   jddctmgr.c \
	   jidctint.c \
	   jidctfst.c \
	   jquant1.c \
	   jdpostct.c \
	   jdmerge.c \
	   jdmainct.c \
	   jquant2.c \
	   jdcolor.c \
	   jaricom.c \
	   jidctflt.c \
	   sw_dec_proc.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
include $(NVT_PRJCFG_MODEL_CFG)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \

endif

include $(NVT_HDAL_DIR)/driver.mk
