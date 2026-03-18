# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2001
# Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.

#ccflags-y += -DDEBUG

obj-$(CONFIG_DM_PWM)		+= pwm-uclass.o

obj-$(CONFIG_PWM_CROS_EC)	+= cros_ec_pwm.o
obj-$(CONFIG_PWM_EXYNOS)	+= exynos_pwm.o
obj-$(CONFIG_PWM_IMX)		+= pwm-imx.o pwm-imx-util.o
obj-$(CONFIG_PWM_MESON)		+= pwm-meson.o
obj-$(CONFIG_PWM_MTK)		+= pwm-mtk.o
ifneq ($(CONFIG_TARGET_NA51055)$(CONFIG_TARGET_NA51055_A64),)
obj-$(CONFIG_PWM_NVT)		+= nvt_pwm.o nvt_pwm_na51055.o
else ifneq ($(CONFIG_TARGET_NA51089)$(CONFIG_TARGET_NA51089_A64),)
obj-$(CONFIG_PWM_NVT)		+= nvt_pwm.o nvt_pwm_na51089.o
else ifneq ($(CONFIG_TARGET_NA51102)$(CONFIG_TARGET_NA51102_A64),)
obj-$(CONFIG_PWM_NVT)		+= nvt_pwm.o nvt_pwm_na51102.o
else ifneq ($(CONFIG_TARGET_NA51103)$(CONFIG_TARGET_NA51103_A64),)
obj-$(CONFIG_PWM_NVT)		+= nvt_pwm.o nvt_pwm_na51103.o
else ifneq ($(CONFIG_TARGET_NS02201)$(CONFIG_TARGET_NS02201_A64),)
obj-$(CONFIG_PWM_NVT)		+= nvt_pwm.o nvt_pwm_ns02201.o
else ifneq ($(CONFIG_TARGET_NS02301)$(CONFIG_TARGET_NS02301_A64),)
obj-$(CONFIG_PWM_NVT)		+= nvt_pwm.o nvt_pwm_ns02301.o
else ifneq ($(CONFIG_TARGET_NS02302)$(CONFIG_TARGET_NS02302_A64),)
obj-$(CONFIG_PWM_NVT)		+= nvt_pwm.o nvt_pwm_ns02302.o
endif
obj-$(CONFIG_PWM_ROCKCHIP)	+= rk_pwm.o
obj-$(CONFIG_PWM_SANDBOX)	+= sandbox_pwm.o
obj-$(CONFIG_PWM_SIFIVE)	+= pwm-sifive.o
obj-$(CONFIG_PWM_TEGRA)		+= tegra_pwm.o
obj-$(CONFIG_PWM_SUNXI)		+= sunxi_pwm.o
obj-$(CONFIG_PWM_TI_EHRPWM)	+= pwm-ti-ehrpwm.o
