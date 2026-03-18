MODULE_NAME = kflow_videocapture
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR=$(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR=$(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/source/include \
	-I$(KDRV_DIR)/source/kdrv_videocapture/ssenif \
	-I$(KFLOW_DIR)/include \
	-I$(KDRV_DIR)/include/kdrv_videocapture \
	-I$(KFLOW_DIR)/include/kflow_videocapture \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_sen/lib \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_sen/include \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_sen/sen_debug \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_sen/sen_utility \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_sen/sen_flow \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_sie/include \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_sie/sie_debug \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_sie/sie_utility \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_sie/sie_flow \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_vie/include \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_vie/vie_debug \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_vie/vie_utility \
	-I$(CURR_DRIVER_DIR)/source/kflow_videocapture/ctl_vie/vie_flow \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \
	-D_GROUP_KO_ \
	-Wno-missing-braces \

ifeq ($(CONFIG_NVT_HDAL_KFLOW_VIDEOCAPTURE),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KFLOW_VIDEOCAPTURE)
endif

SRC += \
	kflow_videocapture_main.c \
	ctl_sie/ctl_sie_main.c \
	ctl_sie/ctl_sie_proc.c \
	ctl_sie/ctl_sie_drv.c \
	ctl_sie/ctl_sie_api.c \
	ctl_sie/ctl_sie_export.c \
	ctl_sie/sie_flow/kdf_sie.c \
	ctl_sie/sie_utility/ctl_sie_event_id.c \
	ctl_sie/sie_utility/ctl_sie_event.c \
	ctl_sie/sie_utility/ctl_sie_utility_int.c \
	ctl_sie/sie_utility/ctl_sie_cert.c \
	ctl_sie/sie_debug/ctl_sie_debug_int.c \
	ctl_sie/sie_flow/ctl_sie_buf.c \
	ctl_sie/sie_flow/ctl_sie_iosize.c \
	ctl_sie/sie_flow/ctl_sie_id.c \
	ctl_sie/sie_flow/ctl_sie_isp_task.c \
	ctl_sie/sie_flow/ctl_sie.c \
	ctl_sie/sie_flow/ctl_sie_isp.c \
	ctl_sie/ctl_sie_ver.c \
	ctl_vie/ctl_vie_main.c \
	ctl_vie/ctl_vie_proc.c \
	ctl_vie/ctl_vie_drv.c \
	ctl_vie/ctl_vie_api.c \
	ctl_vie/ctl_vie_export.c \
	ctl_vie/vie_flow/kdf_vie.c \
	ctl_vie/vie_utility/ctl_vie_event_id.c \
	ctl_vie/vie_utility/ctl_vie_event.c \
	ctl_vie/vie_utility/ctl_vie_utility_int.c \
	ctl_vie/vie_utility/ctl_vie_cert.c \
	ctl_vie/vie_debug/ctl_vie_debug_int.c \
	ctl_vie/vie_flow/ctl_vie_buf.c \
	ctl_vie/vie_flow/ctl_vie_iosize.c \
	ctl_vie/vie_flow/ctl_vie_id.c \
	ctl_vie/vie_flow/ctl_vie_isp_task.c \
	ctl_vie/vie_flow/ctl_vie.c \
	ctl_vie/vie_flow/ctl_vie_isp.c \
	ctl_vie/ctl_vie_ver.c \
	ctl_sen/ctl_sen_main.c \
	ctl_sen/ctl_sen_proc.c \
	ctl_sen/ctl_sen_api.c \
	ctl_sen/ctl_sen_config.c \
	ctl_sen/ctl_sen_export.c \
	ctl_sen/ctl_sen_ver.c \
	ctl_sen/sen_flow/ctl_sen.c \
	ctl_sen/sen_flow/ctl_sen_clk.c \
	ctl_sen/sen_flow/ctl_sen_if.c \
	ctl_sen/sen_utility/ctl_sen_utility.c \
	ctl_sen/sen_utility/ctl_sen_cert.c \
	ctl_sen/sen_debug/ctl_sen_debug_infor.c \
	unit/linux/isf_vdocap_main.c \
	unit/linux/isf_vdocap_proc.c \
	unit/linux/isf_vdocap_drv.c \
	unit/isf_vdocap_api.c \
	unit/isf_vdocap.c \
	unit/isf_vdocap_debug.c \
	unit/isf_vdocap_id.c \
	unit/isf_vdocap_output.c \
	unit/isf_vdocap_pull.c \
	unit/isf_vdocap0.c \
	unit/isf_vdocap1.c \
	unit/isf_vdocap2.c \
	unit/isf_vdocap3.c \
	unit/isf_vdocap4.c \
	unit/isf_vdocap5.c \
	unit/isf_vdocap6.c \
	unit/isf_vdocap7.c \
	unit/isf_vdocap8.c \
	unit/isf_vdocap_verinfo.c \
	unit/isf_vdocap_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	kflow_videocapture_main.c \
	ctl_sen/ctl_sen_config.c \
	ctl_sen/ctl_sen_api.c \
	ctl_sen/sen_flow/ctl_sen.c \
	ctl_sen/sen_flow/ctl_sen_clk.c \
	ctl_sen/sen_flow/ctl_sen_if.c \
	ctl_sen/sen_utility/ctl_sen_utility.c \
	ctl_sen/sen_utility/ctl_sen_cert.c \
	ctl_sen/sen_debug/ctl_sen_debug_infor.c \
	ctl_sie/ctl_sie_api.c \
	ctl_sie/sie_flow/kdf_sie.c \
	ctl_sie/sie_utility/ctl_sie_event_id.c \
	ctl_sie/sie_utility/ctl_sie_event.c \
	ctl_sie/sie_utility/ctl_sie_utility_int.c \
	ctl_sie/sie_utility/ctl_sie_cert.c \
	ctl_sie/sie_debug/ctl_sie_debug_int.c \
	ctl_sie/sie_flow/ctl_sie_buf.c \
	ctl_sie/sie_flow/ctl_sie_iosize.c \
	ctl_sie/sie_flow/ctl_sie_id.c \
	ctl_sie/sie_flow/ctl_sie_isp_task.c \
	ctl_sie/sie_flow/ctl_sie.c \
	ctl_sie/sie_flow/ctl_sie_isp.c \
	ctl_sie/ctl_sie_ver.c \
	ctl_vie/ctl_vie_api.c \
	ctl_vie/vie_flow/kdf_vie.c \
	ctl_vie/vie_utility/ctl_vie_event_id.c \
	ctl_vie/vie_utility/ctl_vie_event.c \
	ctl_vie/vie_utility/ctl_vie_utility_int.c \
	ctl_vie/vie_utility/ctl_vie_cert.c \
	ctl_vie/vie_debug/ctl_vie_debug_int.c \
	ctl_vie/vie_flow/ctl_vie_buf.c \
	ctl_vie/vie_flow/ctl_vie_iosize.c \
	ctl_vie/vie_flow/ctl_vie_id.c \
	ctl_vie/vie_flow/ctl_vie_isp_task.c \
	ctl_vie/vie_flow/ctl_vie.c \
	ctl_vie/vie_flow/ctl_vie_isp.c \
	ctl_vie/ctl_vie_ver.c \
	unit/rtos/isf_vdocap_drv.c \
	unit/rtos/isf_vdocap_proc.c \
	unit/isf_vdocap_api.c \
	unit/isf_vdocap.c \
	unit/isf_vdocap_debug.c \
	unit/isf_vdocap_id.c \
	unit/isf_vdocap_output.c \
	unit/isf_vdocap_pull.c \
	unit/isf_vdocap0.c \
	unit/isf_vdocap1.c \
	unit/isf_vdocap2.c \
	unit/isf_vdocap3.c \
	unit/isf_vdocap4.c \
	unit/isf_vdocap5.c \
	unit/isf_vdocap6.c \
	unit/isf_vdocap7.c \
	unit/isf_vdocap8.c \
	unit/isf_vdocap_verinfo.c \
	unit/isf_vdocap_cert.c

endif

include $(NVT_HDAL_DIR)/driver.mk
