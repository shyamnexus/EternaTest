MODULE_NAME = hd_demo_turnkey
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
AI_LIB_DIR = $(NVT_HDAL_DIR)/vendor/ai3
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
AI_LIB_INC_PATH = $(AI_LIB_DIR)/include

# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(VOS_INC_PATH) \
	-I$(AI_LIB_INC_PATH) \
	-I$(NVT_HDAL_DIR)/vendor/media/include \

EXTRA_INCLUDE += -I$(AI_LIB_DIR)/drivers/k_flow/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/modules_lib/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/samples_ipc_lib/include

EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/pvdcnn/include

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)

EXTRA_CFLAGS += $(NVT_ASAN)

EXTRA_LDFLAGS = $(NVT_ASAN) \
	-L$(HDAL_LIB_PATH) \
	-lhdal -lpthread

EXTRA_LDFLAGS += \
	-L$(VENDOR_LIB_PATH) \
	-lvendor_media \

EXTRA_LDFLAGS += -lprebuilt_samples_ipc_lib

EXTRA_LDFLAGS += -lprebuilt_modules_lib

SRC += hd_demo_turnkey.c

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
endif

include $(NVT_HDAL_DIR)/executable.mk
