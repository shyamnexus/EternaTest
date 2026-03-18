MODULE_NAME = libvendor_eis
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
SRC += \
	vendor_eis.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
RTOS_KERNEL_DIR = $(KERNELDIR)/lib/FreeRTOS
RTOS_LIB_DIR = $(KERNELDIR)/lib
RTOS_CURR_DEMO_DIR = $(KERNELDIR)/demos/novatek/$(SDK_CODENAME)
RTOS_POSIX_DIR = $(KERNELDIR)/lib/FreeRTOS-Plus-POSIX
RTOS_POSIX_SRC_DIR = $(RTOS_POSIX_DIR)/source
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(RTOS_LIB_DIR) \
	-I$(RTOS_KERNEL_DIR)/portable/GCC/ARM_CA9  \
	-I$(RTOS_KERNEL_DIR)/include \
	-I$(RTOS_KERNEL_DIR)/include/private \
	-I$(RTOS_CURR_DEMO_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include/portable/novatek \
	-I$(LIBRARY_DIR)/include

SRC += \
	vendor_eis.c \

endif

include $(NVT_HDAL_DIR)/library.mk
