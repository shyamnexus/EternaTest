MODULE_NAME = libvendor_mau
# DIRs
MAU_DRIVER_DIR = $(NVT_HDAL_DIR)/drivers/k_driver/source/comm/ddr_arb
MAU_LIB_DIR = $(NVT_HDAL_DIR)/vendor/mau
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
OUTPUT_DIR = $(NVT_HDAL_DIR)/vendor/output
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers

# INCs
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
MAU_DRIVER_INC_PATH = $(MAU_DRIVER_DIR)/include
MAU_LIB_INC_PATH = $(MAU_LIB_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include

# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(MAU_DRIVER_INC_PATH) \
	-I$(MAU_LIB_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(VOS_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += $(NVT_ASAN)
EXTRA_LDFLAGS = $(NVT_ASAN)
SRC += \
	vendor_mau.c

###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
SRC += \

endif

include $(NVT_HDAL_DIR)/library.mk
