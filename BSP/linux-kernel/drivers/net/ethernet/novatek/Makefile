#
# Makefile for the Novatek device drivers.
#

obj-$(CONFIG_NET_VENDOR_NOVATEK) += ntkimethmac.o
ntkimethmac-objs := DWC_ETH_QOS.o DWC_ETH_QOS_mdio.o \
			DWC_ETH_QOS_dev.o DWC_ETH_QOS_desc.o \
			DWC_ETH_QOS_drv.o DWC_ETH_QOS_ethtool.o \
			DWC_ETH_QOS_debug_operation.o DWC_ETH_QOS_ptp.o DWC_ETH_QOS_platform.o
ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51103),y)
	ntkimethmac-objs += DWC_ETH_QOS_proc.o
endif
