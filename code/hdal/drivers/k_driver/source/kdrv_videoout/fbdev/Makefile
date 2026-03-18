MODULE_NAME = nvt_fb
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
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_DIR)/source/kdrv_videoout/include \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

ifeq ($(NVT_PRJCFG_CFG),Linux)

SRC += \
	nvt_fb_na51102.c \
	nvt_fb_osd.c \
	nvt_fb_video.c \

EXTRA_CFLAGS += -I$(NVT_DRIVER_DIR)/include
EXTRA_CFLAGS += -I$(VOS_INC_PATH)
EXTRA_CFLAGS += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/kdrv_videoout/include
EXTRA_CFLAGS += -I$(NVT_HDAL_DIR)/drivers/k_driver/include/
else ifeq ($(NVT_PRJCFG_CFG),rtos)

endif

include $(NVT_HDAL_DIR)/driver.mk
