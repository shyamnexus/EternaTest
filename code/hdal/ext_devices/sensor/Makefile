###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
SUBDIRS += \
	sen_off \
	sen_ar0221 \
	sen_ar0237 \
	sen_ar0237ir \
	sen_ar0544 \
	sen_ar0144 \
	sen_ar2020 \
	sen_gc2053 \
	sen_gc2375 \
	sen_gc4c33 \
	sen_gc4653 \
	sen_gc4663 \
	sen_gc5035 \
	sen_gc5603 \
	sen_gc8034 \
	sen_gc8613 \
	sen_hi5011q \
	sen_ov12d \
	sen_ov13b10 \
	sen_ov2715 \
	sen_ov2718 \
	sen_ov2775 \
	sen_ov2736ir \
	sen_ov2740 \
	sen_ov48b \
	sen_ov64b \
	sen_ov9281 \
	sen_ov9281_slave \
	sen_ov12895 \
	sen_os02d10 \
	sen_os02g10 \
	sen_os02k10 \
	sen_os02h10 \
	sen_os03a10 \
	sen_os04a10 \
	sen_os04a10_slave \
	sen_os04b10 \
	sen_os04c10 \
	sen_os04c10_slave \
	sen_os05a10 \
	sen_os06a10 \
	sen_os08a10 \
	sen_os08b10 \
	sen_os12d40 \
	sen_ps5268 \
	sen_f35 \
	sen_f37 \
	sen_f37p \
	sen_f51 \
	sen_f53 \
	sen_imx123 \
	sen_imx178 \
	sen_imx265 \
	sen_imx307 \
	sen_imx307_slave \
	sen_imx226 \
	sen_imx230 \
	sen_imx258 \
	sen_imx290 \
	sen_imx305 \
	sen_imx317 \
	sen_imx322 \
	sen_imx326 \
	sen_imx327 \
	sen_imx334 \
	sen_imx335 \
	sen_imx335_slave \
	sen_imx347 \
	sen_imx385 \
	sen_imx415 \
	sen_imx464 \
	sen_imx485 \
	sen_imx477 \
	sen_imx577 \
	sen_imx586 \
	sen_imx636 \
	sen_imx678 \
	sen_jinglin_ti_module \
	sen_jinglin_ti_raw_module \
	sen_q03p \
	sen_rtd6122c \
	sen_s5k231y \
	sen_s5k4h7 \
	sen_sc1330 \
	sen_sc132gs \
	sen_sc200ai \
	sen_sc230ai \
	sen_sc231ai \
	sen_sc301iot \
	sen_sc350ai \
	sen_sc401ai \
	sen_sc430ai \
	sen_sc436sl \
	sen_sc450ai \
	sen_sc500ai \
	sen_sc501ai_slave \
	sen_sc500cs \
	sen_sc530ai \
	sen_sc531ai \
	sen_sc531ai_slave \
	sen_sc803h \
	sen_sc830ai \
	sen_sc835hai \
	sen_sc850sl \
	sen_sc910gs \
	sen_sc223a \
	sen_sc2232h \
	sen_sc2239 \
	sen_sc2310 \
	sen_sc2335 \
	sen_sc233a \
	sen_sc2335_slave \
	sen_sc2336 \
	sen_sc3332 \
	sen_sc3335 \
	sen_sc3336 \
	sen_sc3338 \
	sen_sc3338_slave \
	sen_sc4210 \
	sen_sc4238 \
	sen_sc4335 \
	sen_sc4336 \
	sen_sc5239 \
	sen_sc5239_slave \
	sen_sc8238 \
	sen_vd55g0 \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

SUBDIRS := $(dir $(shell find . -name Makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)

endif

include $(NVT_HDAL_DIR)/drivers.mk
