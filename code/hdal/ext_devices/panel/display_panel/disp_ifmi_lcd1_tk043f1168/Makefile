include $(NVT_HDAL_DIR)/ext_devices/panel/display_panel/PanelDef.txt

KDRV_INC_PATH=$(NVT_HDAL_DIR)/drivers/k_driver

lcd = disp_ifmi_lcd1_tk043f1168
panel_type = dispdev_ifmi
DISPLCDSEL_PARAMETER += -DDISPLCDSEL_IFMI_TYPE=DISPLCDSEL_IFMI_LCD1
DISPDEV_PARAMETER += -DDISPDEV_IFMI_TYPE=DISPDEV_IFMI_TK043F1168

MODULE_NAME = disp_ifmi_lcd1_tk043f1168
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/ext_devices/panel
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_DIR)/source/kdrv_videoout/include \
	-I$(KDRV_DIR)/include/comm \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-I$(src)/../include \
	-DDEBUG \
	$(DISPLCDSEL_PARAMETER) \
	$(DISPDEV_PARAMETER) \


SRC += \
	dispdev_ifmi.c \
	panel.c \
	panel_ver.c \


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/../include \
	$(DISPLCDSEL_PARAMETER) \
	$(DISPDEV_PARAMETER) \


SRC += \
	dispdev_ifmi.c \
	panel.c \
	panel_ver.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
