obj-$(CONFIG_NVT_OTP_EP_SUPPORT) += nvt_otp_ep.o

ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51055), y)
nvt_otp_protect := na51055
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51000), y)
nvt_otp_protect := na51000
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51090), y)
nvt_otp_protect := na51090_a64
else
nvt_otp_protect := na51068
endif

ifdef CONFIG_FUNCTION_TRACER
ORIG_CFLAGS := $(KBUILD_CFLAGS)
KBUILD_CFLAGS = $(subst $(CC_FLAGS_FTRACE),,$(ORIG_CFLAGS))
endif

nvt_otp_ep-objs := $(nvt_otp_protect)/otp_protected.o_shipped otp_ep_drv.o otp_ep_main.o otp_ep_platform.o otp_ep_proc.o
