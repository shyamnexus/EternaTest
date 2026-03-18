#
# Makefile for the nvt platform of the linux kernel
#

ifdef CONFIG_ARM
obj-$(CONFIG_NVT_IVOT_PLAT_NA51000)			+= plat/reset.o
obj-$(CONFIG_NVT_IVOT_PLAT_NA51055)			+= plat/reset.o plat/pll_preset.o nvt-sramctl.o plat/fastboot_preload.o
obj-$(CONFIG_NVT_IVOT_PLAT_NA51089)			+= plat/reset.o nvt-sramctl.o plat/pll_preset.o plat/fastboot_preload.o
obj-$(CONFIG_NVT_IVOT_PLAT_NA51090)			+= plat/reset.o plat/nvt_mmcpinctl.o plat/chip_ver.o
obj-$(CONFIG_NVT_IVOT_PLAT_NA51102)			+= nvt-sramctl.o plat/reset.o plat/pll_preset.o plat/nvt_mmcpinctl.o plat/fastboot_preload.o
obj-$(CONFIG_NVT_IVOT_PLAT_NA51103)			+= plat/reset.o plat/nvt_mmcpinctl.o nvt-sramctl.o plat/chip_ver.o
obj-$(CONFIG_NVT_IVOT_PLAT_TC98110)			+= plat/reset.o
obj-$(CONFIG_NVT_IVOT_PLAT_NS02201)			+= nvt-sramctl.o
obj-$(CONFIG_NVT_IVOT_PLAT_NS02301)			+= plat/reset.o nvt-sramctl.o plat/nvt_mmcpinctl.o plat/fastboot_preload.o plat/pll_preset.o
obj-$(CONFIG_NVT_IVOT_PLAT_NS02302)			+= plat/reset.o nvt-sramctl.o plat/nvt_ns02302_sramctl.o
obj-$(CONFIG_SUSPEND)					+= plat/pm.o plat/sleep.o
else
obj-$(CONFIG_NVT_IVOT_PLAT_NA51000)			+=
obj-$(CONFIG_NVT_IVOT_PLAT_NA51090)			+= plat/reset.o plat/pll_preset.o plat/nvt_mmcpinctl.o nvt-sramctl.o plat/chip_ver.o
obj-$(CONFIG_NVT_IVOT_PLAT_NA51102)			+= plat/reset.o plat/pll_preset.o plat/nvt_mmcpinctl.o nvt-sramctl.o plat/fastboot_preload.o
obj-$(CONFIG_NVT_IVOT_PLAT_NA51103)			+= plat/reset.o plat/pll_preset.o plat/nvt_mmcpinctl.o nvt-sramctl.o plat/chip_ver.o
obj-$(CONFIG_NVT_IVOT_PLAT_NS02201)			+= plat/reset.o plat/pll_preset.o plat/nvt_mmcpinctl.o
obj-$(CONFIG_NVT_IVOT_PLAT_NS02201)                     += nvt-sramctl.o plat/fastboot_preload.o
obj-$(CONFIG_NVT_IVOT_PLAT_NS02301)			+= plat/reset.o plat/pll_preset.o plat/nvt_mmcpinctl.o
obj-$(CONFIG_NVT_IVOT_PLAT_NS02301)                     += nvt-sramctl.o plat/fastboot_preload.o
obj-$(CONFIG_NVT_IVOT_PLAT_NS02302)			+= plat/reset.o plat/nvt_mmcpinctl.o nvt-sramctl.o plat/fastboot_preload.o plat/pll_preset.o plat/nvt_ns02302_sramctl.o
obj-$(CONFIG_NVT_IVOT_PLAT_NS02401)			+= plat/reset.o plat/nvt_mmcpinctl.o nvt-sramctl.o
obj-$(CONFIG_SUSPEND)					+= plat/pm.o
endif
