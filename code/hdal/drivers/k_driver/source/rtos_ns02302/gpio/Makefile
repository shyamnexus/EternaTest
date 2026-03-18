MODULE_NAME = kdrv_gpio
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
SRC += \
	gpio.c \

endif

include $(NVT_HDAL_DIR)/driver.mk