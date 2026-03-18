MODULE_NAME = kdrv_rpc
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \
	-I$(KDRV_DIR)/source/include \
	-I$(KDRV_DIR)/include/kdrv_rpc \
	-I$(LIBRARY_DIR)/include \
	-I$(CURR_DRIVER_DIR)/source/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \

ifeq ($(CONFIG_NVT_HDAL_KDRV_RPC),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_RPC)
endif

SRC += \
	kdrv_rpc_main.c \
	cc.c \
	kdrv_rpc_drv.c \
	kdrv_rpc_queue.c \
	kdrv_rpc_proc.c \
	kdrv_rpc_debug_cmd.c \
	kdrv_rpc_platform.c \
	kdrv_rpc_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

EXTRA_CFLAGS += -DDEBUG

SRC += \
	cc.c \
	kdrv_rpc_drv.c \
	kdrv_rpc_queue.c \
	kdrv_rpc_debug_cmd.c \
	kdrv_rpc_platform.c \
	kdrv_rpc_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
