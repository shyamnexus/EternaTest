MODULE_NAME = hd_video_record_with_osg

# Flag to enable/disable freetype support (0=disable, 1=enable)
USE_FREETYPE = 0

# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(VOS_INC_PATH) \


###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += $(NVT_ASAN)

# Add freetype support if enabled
ifeq ($(USE_FREETYPE),1)
EXTRA_CFLAGS += -DUSE_FREETYPE=1
EXTRA_INCLUDE += -I/usr/include/freetype2
EXTRA_LDFLAGS = $(NVT_ASAN) -L$(HDAL_LIB_PATH) -lhdal -lpthread -lfreetype
else
EXTRA_CFLAGS += -DUSE_FREETYPE=0
EXTRA_LDFLAGS = $(NVT_ASAN) -L$(HDAL_LIB_PATH) -lhdal -lpthread
endif

SRC += video_record_with_osg.c

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

# Add freetype support if enabled
ifeq ($(USE_FREETYPE),1)
EXTRA_CFLAGS += -DUSE_FREETYPE=1
else
EXTRA_CFLAGS += -DUSE_FREETYPE=0
endif

SRC += video_record_with_osg.c

endif

include $(NVT_HDAL_DIR)/executable.mk
