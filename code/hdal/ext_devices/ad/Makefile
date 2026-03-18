###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
SUBDIRS += \
	sen_ad_dummy \
	sen_ad_tc358743 \
	sen_ad_tc358840 \
	sen_ad_nvp6124b \
	sen_ad_nvp6188 \
	sen_ad_tp2830 \
	sen_ad_tp2854 \
	sen_ad_tp2855 \
	sen_ad_tp9930 \
	sen_ad_tp9950 \
	sen_ad_tp9963 \
	sen_ad_pr2100k \
	sen_ad_pr2020k \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

SUBDIRS := $(dir $(shell find . -name Makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)

endif

include $(NVT_HDAL_DIR)/drivers.mk
