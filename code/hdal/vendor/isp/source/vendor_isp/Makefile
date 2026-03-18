MODULE_NAME = libvendor_isp
# DIRs
VOS_DIR = $(NVT_VOS_DIR)
ISP_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers
ISP_LIB_DIR = $(NVT_HDAL_DIR)/vendor/isp
OUTPUT_DIR = $(NVT_HDAL_DIR)/vendor/output
# INCs
VOS_INC_PATH = $(VOS_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
ISP_DRIVER_INC_PATH = $(ISP_DRIVER_DIR)/include
ISP_LIB_INC_PATH = $(ISP_LIB_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(HDAL_INC_PATH) \
	-I$(ISP_DRIVER_INC_PATH) \
	-I$(ISP_LIB_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += $(NVT_ASAN)
EXTRA_LDFLAGS = $(NVT_ASAN)
SRC += \
	vendor_isp.c \
	vendor_iq.c \
	vendor_3a.c \
	vendor_vpe.c

###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \

SRC += \
	vendor_isp_rtos.c \
	vendor_vpe_rtos.c

endif

include $(NVT_HDAL_DIR)/library.mk
