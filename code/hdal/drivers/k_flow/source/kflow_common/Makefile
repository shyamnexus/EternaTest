MODULE_NAME = kflow_common
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR=$(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR=$(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include/plat \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/include \
	-I$(KFLOW_DIR)/include \
	-I$(KDRV_DIR)/source/kdrv_gfx2d/kdrv_ise/include \
	-I$(KDRV_DIR)/source/kdrv_gfx2d/kdrv_ise/eng/include \
	-I$(KDRV_DIR)/source/kdrv_videoout/include \
	-I$(KDRV_DIR)/include/kdrv_builtin \
	-I$(KDRV_DIR)/source/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \
	-D_GROUP_KO_ \

ifeq ($(CONFIG_NVT_HDAL_KFLOW_COMMON),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KFLOW_COMMON)
endif

SRC += \
	kflow_common_main.c \
	nvtmpp/nvtmpp_api.c \
	nvtmpp/nvtmpp_pool.c \
	nvtmpp/nvtmpp_heap2.c \
	nvtmpp/nvtmpp_blk.c \
	nvtmpp/nvtmpp_id.c \
	nvtmpp/nvtmpp_log.c \
	nvtmpp/nvtmpp_module.c \
	nvtmpp/nvtmpp_main.c \
	nvtmpp/nvtmpp_proc.c \
	nvtmpp/nvtmpp_ioctl.c \
	nvtmpp/nvtmpp_debug_cmd.c \
	nvtmpp/nvtmpp_export.c \
	nvtmpp/nvtmpp_platform.c \
	fastboot/fastboot_main.c \
	fastboot/fastboot_proc.c \
	fastboot/fastboot_fdt.c \
	isf_flow/linux/isf_flow_main.c \
	isf_flow/linux/isf_flow_proc.c \
	isf_flow/linux/isf_flow_drv.c \
	isf_flow/linux/isf_flow_debug_log.c \
	isf_flow/isf_flow_api.c \
	isf_flow/isf_flow_core.c \
	isf_flow/isf_flow_queue.c \
	isf_flow/isf_flow_debug.c \
	isf_flow/isf_flow_id.c \
	isf_flow/isf_flow_unit_data.c \
	isf_flow/isf_flow_unit_param.c \
	isf_flow/isf_flow_unit_state.c \
	isf_flow/isf_flow_data_common.c \
	isf_flow/isf_flow_data_vdofm.c \
	isf_flow/isf_flow_data_vdobs.c \
	isf_flow/isf_flow_data_audfm.c \
	isf_flow/isf_flow_data_audbs.c \
	isf_flow/isf_flow_verinfo.c \
	gximage/gximage.c \
	gximage/gximage_open.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
# -Wno-format for Jeah's isf_flow compiler error
EXTRA_CFLAGS += \
	-DDEBUG \
	-Wno-format \

SRC += \
	kflow_common_main.c \
	nvtmpp/nvtmpp_api.c \
	nvtmpp/nvtmpp_pool.c \
	nvtmpp/nvtmpp_heap2.c \
	nvtmpp/nvtmpp_blk.c \
	nvtmpp/nvtmpp_id.c \
	nvtmpp/nvtmpp_log.c \
	nvtmpp/nvtmpp_module.c \
	nvtmpp/nvtmpp_ioctl.c \
	nvtmpp/nvtmpp_debug_cmd.c \
	nvtmpp/nvtmpp_platform.c \
	gximage/gximage.c \
	gximage/gximage_open.c \
	isf_flow/rtos/isf_flow_drv.c \
	isf_flow/rtos/isf_flow_debug_log.c \
	isf_flow/isf_flow_api.c \
	isf_flow/isf_flow_core.c \
	isf_flow/isf_flow_queue.c \
	isf_flow/isf_flow_debug.c \
	isf_flow/isf_flow_id.c \
	isf_flow/isf_flow_unit_data.c \
	isf_flow/isf_flow_unit_param.c \
	isf_flow/isf_flow_unit_state.c \
	isf_flow/isf_flow_data_common.c \
	isf_flow/isf_flow_data_vdofm.c \
	isf_flow/isf_flow_data_vdobs.c \
	isf_flow/isf_flow_data_audfm.c \
	isf_flow/isf_flow_data_audbs.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
