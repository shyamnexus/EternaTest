MODULE_NAME = vendor_chksum
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
KDRV_INC_PATH = $(NVT_HDAL_DIR)/drivers/k_driver/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += $(NVT_ASAN)
EXTRA_LDFLAGS = $(NVT_ASAN) -L$(HDAL_LIB_PATH) -lhdal
SRC += vendor_chksum.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
SRC += vendor_chksum.c

endif

include $(NVT_HDAL_DIR)/executable.mk
