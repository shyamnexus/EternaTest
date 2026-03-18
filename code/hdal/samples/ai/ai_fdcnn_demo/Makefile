MODULE_NAME = ai_fdcnn_demo

HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
AI_LIB_DIR = $(NVT_HDAL_DIR)/vendor/ai3
AI_LIB = $(AI_LIB_DIR)/source

HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VOS_LIB_PATH = $(NVT_VOS_DIR)/output
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output
AI_LIB_INC_PATH = $(AI_LIB_DIR)/include

EXTRA_INCLUDE += -I$(HDAL_INC_PATH) -I$(NVT_HDAL_DIR)/source -I$(AI_LIB_INC_PATH)
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/include -I$(NVT_HDAL_DIR)/drivers/k_driver/include
EXTRA_INCLUDE += -I$(AI_LIB_DIR)/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/fdcnn/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/modules_lib/include
EXTRA_INCLUDE += -I$(NVT_VOS_DIR)/drivers/include

ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_LDFLAGS += -L$(HDAL_LIB_PATH) -lpthread -lhdal
EXTRA_LDFLAGS += -L$(VOS_LIB_PATH) -lvos
EXTRA_LDFLAGS += -L$(AI_LIB) -lvendor_ai3 -L$(AI_LIB)_pub -lvendor_ai3_pub
EXTRA_LDFLAGS += -L$(VENDOR_LIB_PATH) -lvendor_media
EXTRA_LDFLAGS += -lprebuilt_fdcnn -lprebuilt_modules_lib -lprebuilt_ai -lm -lstdc++
SRC += ai_fdcnn_demo.c
else ifeq ($(NVT_PRJCFG_CFG),rtos)
SRC += ai_fdcnn_demo.c
endif

include $(NVT_HDAL_DIR)/executable.mk
