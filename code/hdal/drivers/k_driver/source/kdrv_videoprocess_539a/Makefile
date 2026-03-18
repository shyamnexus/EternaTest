MODULE_NAME = kdrv_videoprocess_539a
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \
	-I$(KDRV_DIR)/include/comm \
	-I$(CURR_DRIVER_DIR)/source/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_ipp/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_ime/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_ime/eng/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_ipe/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_ipe/eng/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_pre/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_pre/eng/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_ife/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_ife/eng/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_builtin_539a/ipp/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess_539a/kdrv_ipp/eng/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videoprocess/kdrv_sde/include \
	-I$(KDRV_DIR)/include/kdrv_builtin \
	-I$(KDRV_DIR)/include/comm/util \
	-I$(KDRV_DIR)/source/comm/util

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \
	-D_GROUP_KO_ \
	-D_539A_ \

ifeq ($(CONFIG_NVT_HDAL_KDRV_VIDEOPROCESS)$(CONFIG_NVT_HDAL_BUILT_IN_WITH_539A_IPP),yy)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_VIDEOPROCESS)
endif

SRC += \
	kdrv_videoprocess_main.c \
	kdrv_pre/kdrv_pre_main.c \
	kdrv_pre/kdrv_pre_proc.c \
	kdrv_pre/kdrv_pre_init.c \
	kdrv_pre/kdrv_pre_api.c \
	kdrv_pre/kdrv_pre_platform.c \
	kdrv_pre/kdrv_pre_cert.c \
	kdrv_pre/kdrv_pre.c \
	kdrv_ife/kdrv_ife_main.c \
	kdrv_ife/kdrv_ife_proc.c \
	kdrv_ife/kdrv_ife_init.c \
	kdrv_ife/kdrv_ife_api.c \
	kdrv_ife/kdrv_ife_platform.c \
	kdrv_ife/kdrv_ife_cert.c \
	kdrv_ife/kdrv_ife.c \
	kdrv_ime/kdrv_ime_init.c \
	kdrv_ime/kdrv_ime_main.c \
	kdrv_ime/kdrv_ime_proc.c \
	kdrv_ime/kdrv_ime_api.c \
	kdrv_ime/kdrv_ime_platform.c \
	kdrv_ime/kdrv_ime_cert.c \
	kdrv_ime/kdrv_ime.c \
	kdrv_ime/eng/ime_eng.c \
	kdrv_ime/eng/ime_eng_3dnr_base.c \
	kdrv_ime/eng/ime_eng_compression_base.c \
	kdrv_ime/eng/ime_eng_control_base.c \
	kdrv_ime/eng/ime_eng_dbcs_base.c \
	kdrv_ime/eng/ime_eng_in_out_path_base.c \
	kdrv_ime/eng/ime_eng_cal.c \
	kdrv_ime/eng/ime_eng_pm_base.c \
	kdrv_ime/eng/ime_eng_fisheye_mask_base.c \
	kdrv_ime/eng/ime_eng_yuvcvt_base.c \
	kdrv_ime/eng/ime_eng_lca_base.c \
	kdrv_ime/eng/ime_eng_shp_base.c \
	kdrv_ime/eng/ime_eng_nn_isp_base.c \
	kdrv_ime/eng/ime_eng_limt.c \
	kdrv_ime/eng/ime_eng_int_platform.c \
	kdrv_ime/eng/ime_eng_cert.c \
	kdrv_ipe/eng/ipe_eng_platform.c \
	kdrv_ipe/eng/ipe_eng_int.c \
	kdrv_ipe/eng/ipe_eng_base.c \
	kdrv_ipe/eng/ipe_eng.c \
	kdrv_ipe/eng/ipe_eng_cert.c \
	kdrv_ipe/eng/ipe_eng_limt.c \
	kdrv_ipe/kdrv_ipe_init.c \
	kdrv_ipe/kdrv_ipe_main.c \
	kdrv_ipe/kdrv_ipe_proc.c \
	kdrv_ipe/kdrv_ipe_api.c \
	kdrv_ipe/kdrv_ipe_platform.c \
	kdrv_ipe/kdrv_ipe_cert.c \
	kdrv_ipe/kdrv_ipe.c \
	kdrv_ipp/kdrv_ipp_main.c \
	kdrv_ipp/kdrv_ipp_proc.c \
	kdrv_ipp/kdrv_ipp_api.c \
	kdrv_ipp/kdrv_ipp_int.c \
	kdrv_ipp/kdrv_ipp_util.c \
	kdrv_ipp/kdrv_ipp_export.c \
	kdrv_ipp/kdrv_ipp_platform.c \
	kdrv_ipp/kdrv_ipp.c \
	kdrv_ipp/kdrv_ipp_cert.c \
	kdrv_ipp/kdrv_ipp_verinfo.c \
	kdrv_ipp/eng/ipp_eng.c \
	kdrv_pre/eng/pre_eng_platform.c \
	kdrv_pre/eng/pre_eng.c \
	kdrv_pre/eng/pre_eng_base.c \
	kdrv_pre/eng/pre_eng_int.c \
	kdrv_pre/eng/pre_eng_cert.c \
	kdrv_ife/eng/ife_eng_platform.c \
	kdrv_ife/eng/ife_eng.c \
	kdrv_ife/eng/ife_eng_base.c \
	kdrv_ife/eng/ife_eng_int.c \
	kdrv_ife/eng/ife_eng_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += \
	-I$(KDRV_DIR)/include/plat \
	-I$(NVT_DRIVER_DIR)/include

EXTRA_CFLAGS += -DDEBUG

SRC += \
	kdrv_ime/eng/ime_eng.c \
	kdrv_ime/eng/ime_eng_3dnr_base.c \
	kdrv_ime/eng/ime_eng_compression_base.c \
	kdrv_ime/eng/ime_eng_control_base.c \
	kdrv_ime/eng/ime_eng_dbcs_base.c \
	kdrv_ime/eng/ime_eng_in_out_path_base.c \
	kdrv_ime/eng/ime_eng_cal.c \
	kdrv_ime/eng/ime_eng_pm_base.c \
	kdrv_ime/eng/ime_eng_fisheye_mask_base.c \
	kdrv_ime/eng/ime_eng_yuvcvt_base.c \
	kdrv_ime/eng/ime_eng_lca_base.c \
	kdrv_ime/eng/ime_eng_shp_base.c \
	kdrv_ime/eng/ime_eng_nn_isp_base.c \
	kdrv_ime/eng/ime_eng_limt.c \
	kdrv_ime/eng/ime_eng_int_platform.c \
	kdrv_ime/eng/ime_eng_cert.c \
	kdrv_ime/kdrv_ime_init.c \
	kdrv_ime/kdrv_ime_api.c \
	kdrv_ime/kdrv_ime_platform.c \
	kdrv_ime/kdrv_ime_cert.c \
	kdrv_ime/kdrv_ime.c \
	kdrv_pre/kdrv_pre_init.c \
	kdrv_pre/kdrv_pre_api.c \
	kdrv_pre/kdrv_pre_platform.c \
	kdrv_pre/kdrv_pre_cert.c \
	kdrv_pre/kdrv_pre.c \
	kdrv_ife/kdrv_ife_init.c \
	kdrv_ife/kdrv_ife_api.c \
	kdrv_ife/kdrv_ife_platform.c \
	kdrv_ife/kdrv_ife_cert.c \
	kdrv_ife/kdrv_ife.c \
	kdrv_ipe/kdrv_ipe_init.c \
	kdrv_ipe/kdrv_ipe_api.c \
	kdrv_ipe/kdrv_ipe_platform.c \
	kdrv_ipe/kdrv_ipe_cert.c \
	kdrv_ipe/kdrv_ipe.c \
	kdrv_ipp/kdrv_ipp_api.c \
	kdrv_ipp/kdrv_ipp_int.c \
	kdrv_ipp/kdrv_ipp_util.c \
	kdrv_ipp/kdrv_ipp_platform.c \
	kdrv_ipp/kdrv_ipp.c \
	kdrv_ipp/kdrv_ipp_cert.c \
	kdrv_ipp/kdrv_ipp_verinfo.c \
	kdrv_ipp/eng/ipp_eng.c \
	kdrv_pre/eng/pre_eng.c \
	kdrv_pre/eng/pre_eng_base.c \
	kdrv_pre/eng/pre_eng_int.c \
	kdrv_pre/eng/pre_eng_platform.c \
	kdrv_pre/eng/pre_eng_cert.c \
	kdrv_ife/eng/ife_eng.c \
	kdrv_ife/eng/ife_eng_base.c \
	kdrv_ife/eng/ife_eng_int.c \
	kdrv_ife/eng/ife_eng_platform.c \
	kdrv_ife/eng/ife_eng_cert.c \
	kdrv_ipe/eng/ipe_eng_platform.c \
	kdrv_ipe/eng/ipe_eng_int.c \
	kdrv_ipe/eng/ipe_eng_base.c \
	kdrv_ipe/eng/ipe_eng.c \
	kdrv_ipe/eng/ipe_eng_cert.c \
	kdrv_ipe/eng/ipe_eng_limt.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
