MODULE_NAME = nvt_ipc
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR=$(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR=$(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \
	-I$(KFLOW_DIR)/include \
	-I$(KFLOW_DIR)/kflow_rpc/include \
	-I$(KDRV_DIR)/source/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \
	-D_GROUP_KO_ \
	-Wno-missing-braces \

SRC += \
	nvt_ipc_main.c \
	nvt_ipc_drv.c \
	nvt_ipc_task.c \
	nvt_ipc_api.c \
	nvt_ipc_msg_que.c \
	nvt_ipc_proc.c \
	nvt_ipc_ioctl.c \
	nvt_ipc_platform.c \
	nvt_ipc_ver.c \
	nvt_ipc_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	nvt_ipc_drv.c \
	nvt_ipc_task.c \
	nvt_ipc_api.c \
	nvt_ipc_msg_que.c \
	nvt_ipc_ioctl.c \
	nvt_ipc_platform.c \
	nvt_ipc_ver.c \
	nvt_ipc_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
