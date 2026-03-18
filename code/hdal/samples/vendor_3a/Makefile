MODULE_NAME = vendor_3a_sample
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
	-I$(NVT_HDAL_DIR)/vendor/isp/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += $(NVT_ASAN)
EXTRA_LDFLAGS = \
	$(NVT_ASAN) \
	-L$(HDAL_LIB_PATH) -lhdal \
	-lpthread \
	-L$(NVT_HDAL_DIR)/vendor/output -lvendor_isp
	
SRC += \
	sample_awb.c \
	sample_ae.c \
	sample_af.c \
	vendor_3a_sample.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
endif

include $(NVT_HDAL_DIR)/executable.mk
