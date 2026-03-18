MODULE_NAME = ai_pvdcnn_demo

# DIRs
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
AI_LIB_DIR = $(NVT_HDAL_DIR)/vendor/ai3
AI_LIB = $(AI_LIB_DIR)/source
CV_LIB_DIR = $(NVT_HDAL_DIR)/vendor/cv

# INCs
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VOS_LIB_PATH = $(NVT_VOS_DIR)/output
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output
AI_LIB_INC_PATH = $(AI_LIB_DIR)/include
CV_LIB_INC_PATH = $(CV_LIB_DIR)/include
VENDOR_MEDIA_INC = $(NVT_HDAL_DIR)/vendor/media/include

# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(NVT_HDAL_DIR)/source \
	-I$(AI_LIB_INC_PATH) \
	-I$(CV_LIB_INC_PATH) \
	-I$(VENDOR_MEDIA_INC) \

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/include
EXTRA_INCLUDE += -I$(AI_LIB_DIR)/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/pvdcnn/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/modules_lib/include
EXTRA_INCLUDE += -I$(NVT_VOS_DIR)/drivers/include

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += $(NVT_ASAN)
EXTRA_LDFLAGS += $(NVT_ASAN)
EXTRA_LDFLAGS += \
	-L$(HDAL_LIB_PATH) \
	-lpthread \
	-lhdal \

EXTRA_LDFLAGS += \
	-L$(VOS_LIB_PATH) \
	-lvos \

EXTRA_LDFLAGS += \
	-L$(AI_LIB) \
	-lvendor_ai3 \

EXTRA_LDFLAGS += \
	-L$(AI_LIB)_pub \
	-lvendor_ai3_pub \

EXTRA_LDFLAGS += \
	-L$(VENDOR_LIB_PATH) \
	-lvendor_media \
	-lvendor_cv \
	-lprebuilt_cv \

EXTRA_LDFLAGS += -lprebuilt_pvdcnn
EXTRA_LDFLAGS += -lprebuilt_modules_lib
EXTRA_LDFLAGS += -lprebuilt_ai
EXTRA_LDFLAGS += -lm
EXTRA_LDFLAGS += -lstdc++

SRC += ai_pvdcnn_demo.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
RTOS_KERNEL_DIR = $(KERNELDIR)/lib/FreeRTOS
RTOS_LIB_DIR = $(KERNELDIR)/lib
RTOS_CURR_DEMO_DIR = $(KERNELDIR)/demos/novatek/$(SDK_CODENAME)
RTOS_POSIX_DIR = $(KERNELDIR)/lib/FreeRTOS-Plus-POSIX

EXTRA_INCLUDE += \
	-I$(RTOS_LIB_DIR) \
	-I$(RTOS_KERNEL_DIR)/portable/GCC/ARM_CA9) \
	-I$(RTOS_KERNEL_DIR)/include \
	-I$(RTOS_KERNEL_DIR)/include/private \
	-I$(RTOS_CURR_DEMO_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include/portable/novatek \
	-I$(LIBRARY_DIR)/include

EXTRA_CFLAGS += -Wno-format
SRC += ai_pvdcnn_demo.c

endif

include $(NVT_HDAL_DIR)/executable.mk
