obj-y += basic/
obj-y += plat-novatek/
obj-$(CONFIG_NVT_OTP_SUPPORT) 	+= otp/
obj-$(CONFIG_NVT_STBC_SUPPORT) 	+= stbc/
obj-$(CONFIG_NVT_OTP_EP_SUPPORT) += otp_ep/
obj-$(CONFIG_NVT_HDAL_BUILD_IN) += nvt_hdal/
obj-$(CONFIG_NVT_PROFILER) 	+= nvt_profiler.o
obj-$(CONFIG_NVT_LOGFILE) 	+= nvt_logfile/
obj-$(CONFIG_NVT_CC) 	      += nvt_cc/
obj-$(CONFIG_NVT_ACP)		+= nvt_acp/
ifeq ($(CONFIG_NVT_PERF_MEASURE), y)
obj-y += nvt_perf/
endif
ifeq ($(CONFIG_NVT_IVOT_PLAT_NS02302), y)
obj-y += nvt_pd/
endif

ifeq ($(CONFIG_NVT_IVOT_PLAT_NS02301), y)
obj-y += nvt_pd/
endif
