MODULE_NAME = kdrv_videocapture
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \
	-I$(KDRV_DIR)/source/kdrv_builtin/include \
	-I$(KDRV_DIR)/source/kdrv_builtin/sie \
	-I$(KDRV_DIR)/source/kdrv_builtin/sie/eng/include \
	-I$(KDRV_DIR)/source/kdrv_builtin/vie \
	-I$(KDRV_DIR)/include/kdrv_videocapture \
	-I$(LIBRARY_DIR)/include \
	-I$(CURR_DRIVER_DIR)/source/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/kdrv_sie/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/kdrv_sie/kdrv_flow \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/kdrv_sie/kdrv_debug \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/kdrv_vie/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/kdrv_vie/kdrv_flow \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/kdrv_vie/kdrv_debug \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/kdrv_tge/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/kdrv_tge/eng/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/ssenif/hal/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/ssenif/hal \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/ssenif/senphy/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/ssenif/csi/include \
	-I$(CURR_DRIVER_DIR)/source/kdrv_videocapture/ssenif/lvds/include \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-DDEBUG \
	-D_GROUP_KO_ \

ifeq ($(CONFIG_NVT_HDAL_KDRV_VIDEOCAPTURE),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_VIDEOCAPTURE)
endif

SRC += \
	kdrv_videocapture_main.c \
	kdrv_sie/kdrv_sie_init.c \
	kdrv_sie/kdrv_sie_platform.c \
	kdrv_sie/kdrv_sie_main.c \
	kdrv_sie/kdrv_sie_proc.c \
	kdrv_sie/kdrv_sie_api.c \
	kdrv_sie/kdrv_sie_export.c \
	kdrv_sie/kdrv_flow/kdrv_sie.c \
	kdrv_sie/kdrv_flow/kdrv_sie_config.c \
	kdrv_sie/kdrv_flow/kdrv_sie_cert.c \
	kdrv_sie/kdrv_debug/kdrv_sie_debug_int.c \
	kdrv_vie/kdrv_vie_main.c \
	kdrv_vie/kdrv_vie_proc.c \
	kdrv_vie/kdrv_vie_api.c \
	kdrv_vie/kdrv_vie_export.c \
	kdrv_vie/kdrv_flow/kdrv_vie.c \
	kdrv_vie/kdrv_flow/kdrv_vie_config.c \
	kdrv_vie/kdrv_flow/kdrv_vie_cert.c \
	kdrv_vie/kdrv_debug/kdrv_vie_debug_int.c \
	kdrv_tge/eng/tge_int.c \
	kdrv_tge/eng/tge_lib.c \
	kdrv_tge/eng/tge_platform.c \
	kdrv_tge/kdrv_tge_main.c \
	kdrv_tge/kdrv_tge_proc.c \
	kdrv_tge/kdrv_tge_drv.c \
	kdrv_tge/kdrv_tge_api.c \
	kdrv_tge/kdrv_tge.c \
	kdrv_tge/kdrv_tge_export.c \
	kdrv_tge/kdrv_tge_ver.c \
	kdrv_tge/kdrv_tge_platform.c \
	kdrv_tge/kdrv_tge_platform_int.c \
	kdrv_tge/kdrv_tge_cert.c \
	ssenif/hal/ssenif_main.c \
	ssenif/hal/ssenif_proc.c \
	ssenif/hal/ssenif_drv.c \
	ssenif/hal/ssenif_api.c \
	ssenif/hal/kdrv_ssenif.c \
	ssenif/hal/dal_ssenif.c \
	ssenif/hal/serial_sen_intf_lvds.c \
	ssenif/hal/serial_sen_intf_lvds2.c \
	ssenif/hal/serial_sen_intf_lvds3.c \
	ssenif/hal/serial_sen_intf_lvds4.c \
	ssenif/hal/serial_sen_intf_csi.c \
	ssenif/hal/serial_sen_intf_csi2.c \
	ssenif/hal/serial_sen_intf_csi3.c \
	ssenif/hal/serial_sen_intf_csi4.c \
	ssenif/senphy/senphy_main.c \
	ssenif/senphy/senphy_proc.c \
	ssenif/senphy/senphy_drv.c \
	ssenif/senphy/senphy_api.c \
	ssenif/senphy/senphy.c \
	ssenif/senphy/senphy_ver.c \
	ssenif/csi/csi_main.c \
	ssenif/csi/csi_proc.c \
	ssenif/csi/csi_drv.c \
	ssenif/csi/csi_api.c \
	ssenif/csi/csi_obj.c \
	ssenif/csi/csi.c \
	ssenif/csi/csi2.c \
	ssenif/csi/csi3.c \
	ssenif/csi/csi4.c \
	ssenif/csi/csi_ver.c \
	ssenif/csi/csi_platform.c\
	ssenif/csi/csi_cert.c \
	ssenif/lvds/lvds_main.c \
	ssenif/lvds/lvds_proc.c \
	ssenif/lvds/lvds_drv.c \
	ssenif/lvds/lvds_api.c \
	ssenif/lvds/lvds.c \
	ssenif/lvds/lvds2.c \
	ssenif/lvds/lvds3.c \
	ssenif/lvds/lvds4.c \
	ssenif/lvds/lvdsobj.c \
	ssenif/lvds/lvds_ver.c \
	ssenif/lvds/lvds_platform.c \
	ssenif/lvds/lvds_cert.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

EXTRA_CFLAGS += -DDEBUG

SRC += \
	kdrv_videocapture_main.c \
	kdrv_sie/kdrv_flow/kdrv_sie.c \
	kdrv_sie/kdrv_flow/kdrv_sie_config.c \
	kdrv_sie/kdrv_flow/kdrv_sie_cert.c \
	kdrv_sie/kdrv_debug/kdrv_sie_debug_int.c \
	kdrv_sie/kdrv_sie_api.c \
	kdrv_sie/kdrv_sie_init.c \
	kdrv_sie/kdrv_sie_platform.c \
	kdrv_vie/kdrv_flow/kdrv_vie.c \
	kdrv_vie/kdrv_flow/kdrv_vie_config.c \
	kdrv_vie/kdrv_flow/kdrv_vie_cert.c \
	kdrv_vie/kdrv_debug/kdrv_vie_debug_int.c \
	kdrv_vie/kdrv_vie_api.c \
	kdrv_tge/eng/tge_int.c \
	kdrv_tge/eng/tge_lib.c \
	kdrv_tge/eng/tge_platform.c \
	kdrv_tge/kdrv_tge_api.c \
	kdrv_tge/kdrv_tge.c \
	kdrv_tge/kdrv_tge_platform.c \
	kdrv_tge/kdrv_tge_platform_int.c \
	kdrv_tge/kdrv_tge_cert.c \
	ssenif/hal/kdrv_ssenif.c \
	ssenif/hal/dal_ssenif.c \
	ssenif/hal/serial_sen_intf_lvds.c \
	ssenif/hal/serial_sen_intf_lvds2.c \
	ssenif/hal/serial_sen_intf_lvds3.c \
	ssenif/hal/serial_sen_intf_lvds4.c \
	ssenif/hal/serial_sen_intf_csi.c \
	ssenif/hal/serial_sen_intf_csi2.c \
	ssenif/hal/serial_sen_intf_csi3.c \
	ssenif/hal/serial_sen_intf_csi4.c \
	ssenif/senphy/senphy.c \
	ssenif/senphy/senphy_ver.c \
	ssenif/csi/csi.c \
	ssenif/csi/csi2.c \
	ssenif/csi/csi3.c \
	ssenif/csi/csi4.c \
	ssenif/csi/csi_obj.c \
	ssenif/csi/csi_ver.c \
	ssenif/csi/csi_platform.c\
	ssenif/csi/csi_cert.c \
	ssenif/lvds/lvds.c \
	ssenif/lvds/lvds2.c \
	ssenif/lvds/lvds3.c \
	ssenif/lvds/lvds4.c \
	ssenif/lvds/lvdsobj.c \
	ssenif/lvds/lvds_ver.c \
	ssenif/lvds/lvds_platform.c \
	ssenif/lvds/lvds_cert.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
