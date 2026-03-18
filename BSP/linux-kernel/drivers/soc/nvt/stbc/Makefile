obj-$(CONFIG_NVT_STBC_SUPPORT) += nvt_stbc.o

ifdef CONFIG_FUNCTION_TRACER
ORIG_CFLAGS := $(KBUILD_CFLAGS)
KBUILD_CFLAGS = $(subst $(CC_FLAGS_FTRACE),,$(ORIG_CFLAGS))
endif

nvt_stbc-objs := stbc_drv.o stbc_main.o stbc_proc.o
ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51102), y)
nvt_stbc-objs += na51102/stbc.o na51102/stbc_platform.o
else
nvt_stbc-objs += ns02302/stbc.o ns02302/stbc_platform.o
endif
