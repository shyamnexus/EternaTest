MODULE_NAME = nvt_usb3dev
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
ifeq ($(CONFIG_NVT_HDAL_KDRV_BUILTIN),y)
EXTRA_CFLAGS += -D__KDRV_BUILTIN_USE_INITCALL
endif

EXTRA_CFLAGS += -DDEBUG

ifeq ($(CONFIG_NVT_HDAL_USBDEV_BUILTIN),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_USBDEV_BUILTIN)
endif

SRC += \
	usb3dev_main.c \
	usb3dev_proc.c \
	usb3dev_drv.c \
	usb3dev_api.c \
	usb3dev.c \
	usb3dev_command.c \
	usb3dev_interrupt.c \
	usb3dev_stdrequest.c \
	usb3dev_ver.c \


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

endif

include $(NVT_HDAL_DIR)/driver.mk
