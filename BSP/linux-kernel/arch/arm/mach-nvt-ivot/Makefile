#
# Makefile for NVT processors family
#

ccflags-$(CONFIG_ARCH_NVT_IVOT_V7) := -I$(srctree)/drivers/soc/nvt/plat-novatek/include

obj-$(CONFIG_ARCH_NVT_IVOT_V7)		+= board-dt.o io.o

obj-$(CONFIG_SMP)			+= platsmp.o headsmp.o

obj-$(CONFIG_HOTPLUG_CPU)		+= hotplug.o

obj-$(CONFIG_TEE)			+= smc.o
