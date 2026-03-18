MODULE_NAME = ai_heat_map_demo

CV_LIB_DIR = $(NVT_HDAL_DIR)/vendor/cv
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VOS_LIB_PATH = $(NVT_VOS_DIR)/output
CV_LIB_INC_PATH = $(CV_LIB_DIR)/include
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output

EXTRA_INCLUDE += -I$(HDAL_INC_PATH) -I$(CV_LIB_INC_PATH) -I$(NVT_VOS_DIR)/drivers/include

ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_LDFLAGS += -L$(HDAL_LIB_PATH) -lhdal -lpthread
EXTRA_LDFLAGS += -L$(VOS_LIB_PATH) -lvos
EXTRA_LDFLAGS += -L$(VENDOR_LIB_PATH) -lvendor_cv -lprebuilt_cv -lvendor_media
SRC += ai_heat_map_demo.c
else ifeq ($(NVT_PRJCFG_CFG),rtos)
SRC += ai_heat_map_demo.c
endif

include $(NVT_HDAL_DIR)/executable.mk
