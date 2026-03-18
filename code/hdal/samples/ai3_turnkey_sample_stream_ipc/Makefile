MODULE_NAME = ai3_turnkey_sample_stream_ipc

# DIRs
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
AI_LIB_DIR = $(NVT_HDAL_DIR)/vendor/ai3
AI_LIB = $(AI_LIB_DIR)/source
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VOS_LIB_PATH = $(NVT_VOS_DIR)/output
# INCs
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
AI_LIB_INC_PATH = $(AI_LIB_DIR)/include

# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(AI_LIB_INC_PATH) \
	-I$(NVT_HDAL_DIR)/vendor/media/include \

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/include

EXTRA_INCLUDE += -I$(AI_LIB_DIR)/drivers/k_flow/include

EXTRA_INCLUDE += -I$(NVT_VOS_DIR)/drivers/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/limit_fdet/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/pvdcnn/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/fdcnn/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/modules_lib/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/sw_tracker/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/samples_ipc_lib/include

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)

EXTRA_CFLAGS += $(NVT_ASAN)

EXTRA_LDFLAGS = $(NVT_ASAN) \
	-L$(HDAL_LIB_PATH) \
	-lhdal \
	-lm \
	-lpthread \

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

EXTRA_LDFLAGS += -lprebuilt_limit_fdet
EXTRA_LDFLAGS += -lprebuilt_pvdcnn
EXTRA_LDFLAGS += -lprebuilt_fdcnn
EXTRA_LDFLAGS += -lprebuilt_modules_lib
EXTRA_LDFLAGS += -lprebuilt_samples_ipc_lib
EXTRA_LDFLAGS += -lprebuilt_sw_tracker
EXTRA_LDFLAGS += -lprebuilt_ai
EXTRA_LDFLAGS += -lstdc++

SRC += ai3_turnkey_sample_stream_ipc.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

# DIRs
RTOS_KERNEL_DIR = $(KERNELDIR)/lib/FreeRTOS
RTOS_LIB_DIR = $(KERNELDIR)/lib
RTOS_CURR_DEMO_DIR = $(KERNELDIR)/demos/novatek/$(SDK_CODENAME)
RTOS_POSIX_DIR = $(KERNELDIR)/lib/FreeRTOS-Plus-POSIX
RTOS_POSIX_SRC_DIR = $(RTOS_POSIX_DIR)/source

#INCs for EXTRA_CFLAGS

EXTRA_INCLUDE += \
	-I$(RTOS_LIB_DIR) \
	-I$(RTOS_KERNEL_DIR)/portable/GCC/ARM_CA9  \
	-I$(RTOS_KERNEL_DIR)/include \
	-I$(RTOS_KERNEL_DIR)/include/private \
	-I$(RTOS_CURR_DEMO_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include/portable/novatek \
	-I$(LIBRARY_DIR)/include

EXTRA_CFLAGS += -Wno-format

SRC += \

endif

include $(NVT_HDAL_DIR)/executable.mk
