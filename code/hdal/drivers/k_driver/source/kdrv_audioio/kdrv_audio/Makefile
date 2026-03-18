MODULE_NAME = nvt_kdrv_audio
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_DIR)/source/include \
	-I$(KDRV_DIR)/include \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH)/kdrv_audioio \
	-I$(KDRV_DIR)/source/kdrv_audioio/audio_common/include \
	-I$(KDRV_DIR)/source/kdrv_builtin/audio/eac/include \
	-I$(KDRV_DIR)/source/kdrv_builtin/audio/dai/include \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-D_$(NVT_SDK_TYPE)_ \
	-DDEBUG \

ifeq ($(CONFIG_NVT_HDAL_KDRV_AUDIO),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_AUDIO)
endif

SRC += \
	kdrv_audioio_main.c \
	kdrv_audioio_drv.c \
	kdrv_audioio.c \
	kdrv_audioio_export.c \
	kdrv_audioio_cert.c \
	kdrv_audioio_proc.c \
	#kdrv_audioio_dai2/kdrv_audioio_dai2.c \
	#kdrv_audioio_dai2/dai_platform_dai2.c \
	#kdrv_audioio_dai2/audio_obj_tx_dai2.c \
	#kdrv_audioio_dai2/audio_obj_rx_dai2.c \
	#kdrv_audioio_dai2/audio_queue_dai2.c \
	#kdrv_audioio_dai2/audio_obj_txlb_dai2.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-D_$(NVT_SDK_TYPE)_ \
	-DDEBUG \
	-I$(CURDIR)/include \


SRC += \
	kdrv_audioio.c \
	kdrv_audioio_cert.c \
	#kdrv_audioio_dai2/kdrv_audioio_dai2.c \
	#kdrv_audioio_dai2/audio_queue_dai2.c \
	#kdrv_audioio_dai2/audio_obj_tx_dai2.c \
	#kdrv_audioio_dai2/audio_obj_rx_dai2.c \
	#kdrv_audioio_dai2/audio_obj_txlb_dai2.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
