###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
SUBDIRS += \
	disp_off \
	display_panel \
	display_panel/disp_if8b_lcd1_aucn01 \
	display_panel/disp_if8b_lcd2_aucn01 \
	display_panel/disp_if8b_lcd1_pw35p00_hx8238d \
	display_panel/disp_if8b_lcd2_pw35p00_hx8238d \
	display_panel/disp_if8b_lcd1_zh020h2285a_st7789v \
	display_panel/disp_if8b_lcd2_rtp19qc008b_st7789v \
	display_panel/disp_if8b_lcd1_rt13qv053a_st7789v \
	display_panel/disp_ifdsi_lcd1_nt35510 \
	display_panel/disp_ifdsi_lcd2_nt35510 \
	display_panel/disp_ifpa_lcd1_wm08001_sn75lvds83b \
	display_panel/disp_ifmi_lcd1_nt75761 \
	display_panel/disp_ifmi_lcd1_tk043f1168 \
	display_panel/disp_ifmi_lcd2_nt75761 \
	display_panel/disp_ifdsi_lcd1_mi10 \
	display_panel/disp_ifdsi_lcd1_vivox50pro \
	display_panel/disp_if8b_lcd1_psd300_ili8961 \
	display_panel/disp_if8b_lcd2_psd300_ili8961 \
	display_panel/disp_if8b_lcd1_tp2803 \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

SUBDIRS := $(dir $(shell find . -name Makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)

endif

include $(NVT_HDAL_DIR)/drivers.mk

#dependency
$(filter-out display_panel, $(SUBDIRS)): display_panel