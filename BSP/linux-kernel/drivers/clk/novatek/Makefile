obj-$(CONFIG_COMMON_CLK_NVT)		+= nvt-clk.o nvt-clk-provider.o
obj-$(CONFIG_COMMON_CLK_NVT_EP)		+= nvt-clk-ep.o
obj-y					+= nvt-clk-dbg.o

ifdef CONFIG_FUNCTION_TRACER
ORIG_CFLAGS := $(KBUILD_CFLAGS)
KBUILD_CFLAGS = $(subst $(CC_FLAGS_FTRACE),,$(ORIG_CFLAGS))
endif

ifeq ($(CONFIG_NVT_IVOT_PLAT_NS02201),y)
obj-y += ns02201/pll.o_shipped
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51089),y)
obj-y += na51089/pll.o_shipped
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NS02301),y)
obj-y += ns02301/pll.o_shipped
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NS02302),y)
obj-y += ns02302/pll.o_shipped
endif
obj-$(CONFIG_COMMON_CLK_NA51055)        += nvt-im-clk-fr.o na51055-clk.o na51055/pll.o_shipped
obj-$(CONFIG_COMMON_CLK_NA51102)        += na51102/nvt-clk.o v2/nvt-im-clk-fr.o na51102/pll.o_shipped
obj-$(CONFIG_COMMON_CLK_NA51103)        += na51103/nvt-clk.o v2/nvt-im-clk-fr.o #na51103/pll.o_shipped
obj-$(CONFIG_COMMON_CLK_NA51089)        += nvt-im-clk-fr.o na51089-clk.o #na51089/pll.o_shipped
obj-$(CONFIG_COMMON_CLK_NA51090)        += na51090/na51090-clk.o v2/nvt-im-clk-fr.o
obj-$(CONFIG_COMMON_CLK_NA51090_EP)     += na51090/na51090-clk-ep.o v2/nvt-im-clk-fr.o
ifdef CONFIG_PCI
obj-$(CONFIG_COMMON_CLK_NA51090_EP)     += na51090/na51090-clk-ep-common.o
endif

