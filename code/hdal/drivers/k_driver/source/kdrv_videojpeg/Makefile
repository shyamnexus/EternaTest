MODULE_NAME = kdrv_jpg
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
BUILTIN_INC_PATH = $(KDRV_DIR)/source/kdrv_builtin/nvt_jpg/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \
	-I$(UTIL_INC_PATH) \
	-I$(BUILTIN_INC_PATH) \

EXTRA_INCLUDE += \
	-I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME) \
	-I$(NVT_DRIVER_DIR)/include \


###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_KDRV_VIDEOJPEG),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_VIDEOJPEG)
endif

SRC += \
	jpeg_main.c \
	jpeg_proc.c \
	jpeg_enc_sim.c \
	jpeg_dec_sim.c \
	kdrv_videoenc_jpeg.c \
	kdrv_videodec_jpeg.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
include $(NVT_PRJCFG_MODEL_CFG)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \

SRC += \
	jpeg_main.c \
	jpeg_proc.c \
	kdrv_videoenc_jpeg.c \
	kdrv_videodec_jpeg.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
