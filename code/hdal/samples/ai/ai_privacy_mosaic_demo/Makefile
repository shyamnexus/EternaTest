MODULE_NAME = ai_privacy_mosaic_demo

HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VOS_LIB_PATH = $(NVT_VOS_DIR)/output
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output

EXTRA_INCLUDE += -I$(HDAL_INC_PATH) -I$(NVT_VOS_DIR)/drivers/include

ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_LDFLAGS += -L$(HDAL_LIB_PATH) -lhdal -lpthread
EXTRA_LDFLAGS += -L$(VOS_LIB_PATH) -lvos
EXTRA_LDFLAGS += -L$(VENDOR_LIB_PATH) -lvendor_media
SRC += ai_privacy_mosaic_demo.c
else ifeq ($(NVT_PRJCFG_CFG),rtos)
SRC += ai_privacy_mosaic_demo.c
endif

include $(NVT_HDAL_DIR)/executable.mk
