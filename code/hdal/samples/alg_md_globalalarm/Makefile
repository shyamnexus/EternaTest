MODULE_NAME = alg_md_globalalarm
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
CV_LIB_DIR = $(NVT_HDAL_DIR)/vendor/cv
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
VOS_LIB_PATH = $(NVT_VOS_DIR)/output
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
CV_LIB_INC_PATH = $(CV_LIB_DIR)/include
CV_LIB_OUT_PATH = $(NVT_HDAL_DIR)/vendor/output
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(VOS_INC_PATH) \
	-I$(CV_LIB_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += $(NVT_ASAN)
EXTRA_LDFLAGS += $(NVT_ASAN) -L$(HDAL_LIB_PATH) -lhdal -lpthread
EXTRA_LDFLAGS += \
	-L$(CV_LIB_OUT_PATH) \
	-lvendor_cv \
	-lprebuilt_cv \

EXTRA_LDFLAGS += \
	-L$(VOS_LIB_PATH) \
	-lvos \

SRC += alg_md_globalalarm.c

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
SRC += alg_md_globalalarm.c

endif

include $(NVT_HDAL_DIR)/executable.mk
