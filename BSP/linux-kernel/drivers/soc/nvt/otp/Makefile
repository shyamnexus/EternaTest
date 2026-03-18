obj-$(CONFIG_NVT_OTP_SUPPORT) += nvt_otp.o

ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51055), y)
nvt_otp_protect := na51055
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51000), y)
nvt_otp_protect := na51000
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51089), y)
nvt_otp_protect := na51089
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51090), y)
nvt_otp_protect := na51090_a64
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51102), y)
nvt_otp_protect := na51102_a64
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NA51103), y)
nvt_otp_protect := na51103_a32
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NS02201), y)
nvt_otp_protect := ns02201_a64
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NS02302), y)
nvt_otp_protect := ns02302_a64
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NS02301), y)
nvt_otp_protect := ns02301_a32
else ifeq ($(CONFIG_NVT_IVOT_PLAT_NS02401), y)
nvt_otp_protect := ns02401_a64
else
nvt_otp_protect := na51068
endif

ifdef CONFIG_FUNCTION_TRACER
ORIG_CFLAGS := $(KBUILD_CFLAGS)
KBUILD_CFLAGS = $(subst $(CC_FLAGS_FTRACE),,$(ORIG_CFLAGS))
endif

ifneq ($(CONFIG_NVT_IVOT_PLAT_NA51055), y)
nvt_otp-objs := $(nvt_otp_protect)/otp_protected.o_shipped otp_drv.o otp_main.o otp_platform.o otp_proc.o nvt-otp-dummy.o
else
nvt_otp-objs := otp_main.o $(nvt_otp_protect)/otp_protected.o_shipped $(nvt_otp_protect)/otp_drv.o $(nvt_otp_protect)/otp_platform.o $(nvt_otp_protect)/otp_proc.o nvt-otp-dummy.o
endif

