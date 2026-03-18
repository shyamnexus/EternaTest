ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51055),y)
obj-y					+= fmem_na51055.o
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51089),y)
obj-y					+= fmem_na51055.o
else
obj-y					+= fmem.o
endif
obj-y					+= nvtmem/
obj-$(CONFIG_NVT_PCIE_LIB)		+= nvt-pcie-lib/
obj-$(CONFIG_NVT_PCIE_CASCADE_DRV)	+= nvt-pcie-cascade-drv/
obj-$(CONFIG_NVT_PCIE_COMM_DRV_EP)	+= nvt-pcie-drv-ep/
obj-$(CONFIG_NVT_TIMER)			+= nvt-timer/
obj-$(CONFIG_NVT_INFO)			+= nvt-info.o
