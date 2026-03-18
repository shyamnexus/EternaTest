MODULE_NAME = kdrv_usb3dev
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I $(KDRV_DIR)/include \
	-I$(KDRV_DIR)/source/include \
	-I$(CURR_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include/plat \
	-I$(NVT_DRIVER_DIR)/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	dummy.c \
	dummy_core.c \


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-D_$(MSIX_TYPE)_ \
	-D_$(PCIE_EMU_TYPE)_ \

include $(NVT_PRJCFG_MODEL_CFG)
SRC += \
	usb3dev.c \
	usb3dev_command.c \
	usb3dev_stdrequest.c \
	usb3dev_test.c \
	usb3dev_interrupt.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
