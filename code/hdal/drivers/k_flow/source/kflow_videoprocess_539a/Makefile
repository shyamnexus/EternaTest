MODULE_NAME = kflow_videoprocess_539a
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR=$(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR=$(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(KFLOW_DIR)
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/source/include \
	-I$(KFLOW_DIR)/include \
	-I$(KFLOW_DIR)/include/kflow_videoprocess \
	-I$(CURR_DRIVER_DIR)/source/kflow_videoprocess_539a/ctl_ipp/include \
	-I$(CURR_DRIVER_DIR)/source/kflow_videoprocess_539a/ctl_ipp/ipp_debug \
	-I$(CURR_DRIVER_DIR)/source/kflow_videoprocess_539a/ctl_ipp/ipp_utility \
	-I$(CURR_DRIVER_DIR)/source/kflow_videoprocess_539a/ctl_ipp/ipp_flow \
	-I$(CURR_DRIVER_DIR)/source/kflow_videoprocess_539a/ctl_vpe/include \
	-I$(CURR_DRIVER_DIR)/source/kflow_videoprocess_539a/ctl_dre/include \
	-I$(CURR_DRIVER_DIR)/source/kflow_videoprocess_539a/ctl_ise/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \
	-D_GROUP_KO_ \
	-Wno-missing-braces \
	-D_539A_ \

ifeq ($(CONFIG_NVT_HDAL_KFLOW_VIDEOPROCESS)$(CONFIG_NVT_HDAL_BUILT_IN_WITH_539A_IPP),yy)
OBJ-TAG = $(CONFIG_NVT_HDAL_KFLOW_VIDEOPROCESS)
endif

SRC += \
	kflow_videoprocess_main.c \
	ctl_ipp/ctl_ipp_main.c \
	ctl_ipp/ctl_ipp_proc.c \
	ctl_ipp/ctl_ipp_drv.c \
	ctl_ipp/ctl_ipp_api.c \
	ctl_ipp/ctl_ipp_verinfo.c \
	ctl_ipp/ipp_utility/ctl_ipp_util.c \
	ctl_ipp/ipp_utility/ipp_event_id.c \
	ctl_ipp/ipp_utility/ipp_event.c \
	ctl_ipp/ipp_flow/ctl_ipp_isp_int.c \
	ctl_ipp/ipp_flow/ctl_ipp_isp.c \
	ctl_ipp/ipp_flow/ctl_ipp_buf.c \
	ctl_ipp/ipp_flow/ctl_ipp_ai.c \
	ctl_ipp/ipp_flow/ctl_ipp_id.c \
	ctl_ipp/ipp_flow/ctl_ipp_int.c \
	ctl_ipp/ipp_flow/ctl_ipp.c \
	ctl_ipp/ipp_debug/ipp_debug_int.c \
	ctl_ipp/ctl_ipp_export.c \
	ctl_ipp/ctl_ipp_platform.c \
	ctl_ipp/ctl_ipp_cert.c \
	ctl_ipp/ipp_exam/ctl_ipp_exam.c \
	ctl_vpe/ctl_vpe_main.c \
	ctl_vpe/ctl_vpe_proc.c \
	ctl_vpe/ctl_vpe_drv.c \
	ctl_vpe/ctl_vpe_api.c \
	ctl_vpe/ctl_vpe_ver.c \
	ctl_vpe/vpe_flow/ctl_vpe_isp.c \
	ctl_vpe/vpe_flow/ctl_vpe_int.c \
	ctl_vpe/ctl_vpe_cert.c \
	ctl_vpe/vpe_flow/ctl_vpe.c \
	ctl_vpe/ctl_vpe_export.c \
	ctl_ise/ctl_ise_main.c \
	ctl_ise/ctl_ise_proc.c \
	ctl_ise/ctl_ise_drv.c \
	ctl_ise/ctl_ise_api.c \
	ctl_ise/ctl_ise_ver.c \
	ctl_ise/ise_flow/ctl_ise_int.c \
	ctl_ise/ctl_ise_cert.c \
	ctl_ise/ise_flow/ctl_ise.c \
	ctl_ise/ctl_ise_export.c \
	ctl_dre/ctl_dre_main.c \
	ctl_dre/ctl_dre_proc.c \
	ctl_dre/ctl_dre_drv.c \
	ctl_dre/ctl_dre_api.c \
	ctl_dre/ctl_dre_ver.c \
	ctl_dre/dre_flow/ctl_dre_isp.c \
	ctl_dre/dre_flow/ctl_dre_int.c \
	ctl_dre/ctl_dre_cert.c \
	ctl_dre/dre_flow/ctl_dre.c \
	ctl_dre/ctl_dre_export.c \
	unit/linux/isf_vdoprc_main.c \
	unit/linux/isf_vdoprc_proc.c \
	unit/linux/isf_vdoprc_drv.c \
	unit/isf_vdoprc_id.c \
	unit/isf_vdoprc_input.c \
	unit/isf_vdoprc_input_mask.c \
	unit/isf_vdoprc_input_osd.c \
	unit/isf_vdoprc_input_vpe.c \
	unit/isf_vdoprc_input_ise.c \
	unit/isf_vdoprc_process.c \
	unit/isf_vdoprc_output.c \
	unit/isf_vdoprc_output_ext.c \
	unit/isf_vdoprc_output_ext_tsk.c \
	unit/isf_vdoprc_output_mask.c \
	unit/isf_vdoprc_output_osd.c \
	unit/isf_vdoprc_output_vpe.c \
	unit/isf_vdoprc_output_ise.c \
	unit/isf_vdoprc_pull.c \
	unit/isf_vdoprc_debug.c \
	unit/isf_vdoprc0.c \
	unit/isf_vdoprc1.c \
	unit/isf_vdoprc2.c \
	unit/isf_vdoprc3.c \
	unit/isf_vdoprc4.c \
	unit/isf_vdoprc5.c \
	unit/isf_vdoprc6.c \
	unit/isf_vdoprc7.c \
	unit/isf_vdoprc8.c \
	unit/isf_vdoprc9.c \
	unit/isf_vdoprc10.c \
	unit/isf_vdoprc11.c \
	unit/isf_vdoprc12.c \
	unit/isf_vdoprc13.c \
	unit/isf_vdoprc14.c \
	unit/isf_vdoprc15.c \
	unit/isf_vdoprc16.c \
	unit/isf_vdoprc17.c \
	unit/isf_vdoprc18.c \
	unit/isf_vdoprc19.c \
	unit/isf_vdoprc_verinfo.c \
	unit/isf_vdoprc_cert.c \
	unit/isf_vdoprc.c \
	unit/isf_vdoprc_api.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	kflow_videoprocess_main.c \
	ctl_ipp/ipp_debug/ipp_debug_int.c \
	ctl_ipp/ipp_utility/ctl_ipp_util.c \
	ctl_ipp/ipp_utility/ipp_event_id.c \
	ctl_ipp/ipp_utility/ipp_event.c \
	ctl_ipp/ipp_flow/ctl_ipp_isp_int.c \
	ctl_ipp/ipp_flow/ctl_ipp_isp.c \
	ctl_ipp/ipp_flow/ctl_ipp_buf.c \
	ctl_ipp/ipp_flow/ctl_ipp_ai.c \
	ctl_ipp/ipp_flow/ctl_ipp_id.c \
	ctl_ipp/ipp_flow/ctl_ipp_int.c \
	ctl_ipp/ipp_flow/ctl_ipp.c \
	ctl_ipp/ctl_ipp_api.c \
	ctl_ipp/ctl_ipp_platform.c \
	ctl_ipp/ctl_ipp_cert.c \
	ctl_ipp/ipp_exam/ctl_ipp_exam.c \
	ctl_vpe/ctl_vpe_api.c \
	ctl_vpe/ctl_vpe_ver.c \
	ctl_vpe/vpe_flow/ctl_vpe_isp.c \
	ctl_vpe/vpe_flow/ctl_vpe_int.c \
	ctl_vpe/vpe_flow/ctl_vpe.c \
	ctl_vpe/ctl_vpe_cert.c \
	ctl_dre/ctl_dre_api.c \
	ctl_dre/ctl_dre_ver.c \
	ctl_dre/dre_flow/ctl_dre_isp.c \
	ctl_dre/dre_flow/ctl_dre_int.c \
	ctl_dre/dre_flow/ctl_dre.c \
	ctl_dre/ctl_dre_cert.c \
	ctl_ise/ctl_ise_api.c \
	ctl_ise/ctl_ise_ver.c \
	ctl_ise/ise_flow/ctl_ise_int.c \
	ctl_ise/ise_flow/ctl_ise.c \
	ctl_ise/ctl_ise_cert.c \
	unit/rtos/isf_vdoprc_drv.c \
	unit/rtos/isf_vdoprc_proc.c \
	unit/isf_vdoprc_id.c \
	unit/isf_vdoprc_input.c \
	unit/isf_vdoprc_input_mask.c \
	unit/isf_vdoprc_input_osd.c \
	unit/isf_vdoprc_input_vpe.c \
	unit/isf_vdoprc_input_ise.c \
	unit/isf_vdoprc_process.c \
	unit/isf_vdoprc_output.c \
	unit/isf_vdoprc_output_ext.c \
	unit/isf_vdoprc_output_ext_tsk.c \
	unit/isf_vdoprc_output_mask.c \
	unit/isf_vdoprc_output_osd.c \
	unit/isf_vdoprc_output_vpe.c \
	unit/isf_vdoprc_output_ise.c \
	unit/isf_vdoprc_pull.c \
	unit/isf_vdoprc_debug.c \
	unit/isf_vdoprc0.c \
	unit/isf_vdoprc1.c \
	unit/isf_vdoprc2.c \
	unit/isf_vdoprc3.c \
	unit/isf_vdoprc4.c \
	unit/isf_vdoprc5.c \
	unit/isf_vdoprc6.c \
	unit/isf_vdoprc7.c \
	unit/isf_vdoprc8.c \
	unit/isf_vdoprc9.c \
	unit/isf_vdoprc10.c \
	unit/isf_vdoprc11.c \
	unit/isf_vdoprc12.c \
	unit/isf_vdoprc13.c \
	unit/isf_vdoprc14.c \
	unit/isf_vdoprc15.c \
	unit/isf_vdoprc16.c \
	unit/isf_vdoprc17.c \
	unit/isf_vdoprc18.c \
	unit/isf_vdoprc19.c \
	unit/isf_vdoprc_cert.c \
	unit/isf_vdoprc_verinfo.c \
	unit/isf_vdoprc.c \
	unit/isf_vdoprc_api.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
