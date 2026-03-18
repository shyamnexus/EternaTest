MAKE_PID := $(shell echo $$PPID)
JOB_FLAG := $(filter -j%, $(subst -j ,-j,$(shell ps T | grep "^\s*$(MAKE_PID).*$(MAKE)")))
JOBS     := $(subst -j,,$(JOB_FLAG))

NVT_VENDOR_AI3_PATH=$(NVT_HDAL_DIR)/vendor/ai3
NVT_KFLOW_PATH=$(NVT_VENDOR_AI3_PATH)/drivers/k_flow/source
NVT_KFLOW_ISP_PATH=$(NVT_VENDOR_AI3_PATH)/drivers/k_flow_isp/source
NVT_SOURCE_PATH=$(NVT_VENDOR_AI3_PATH)/source
NVT_SOURCE_PUB_PATH=$(NVT_VENDOR_AI3_PATH)/source_pub
NVT_SOURCE_PUB2_PATH=$(NVT_VENDOR_AI3_PATH)/source_pub2
NVT_KDRV_PATH=$(NVT_VENDOR_AI3_PATH)/drivers/k_driver/source
#NVT_SAMPLE_PATH=$(NVT_VENDOR_AI3_PATH)/samples

all:
	@$(MAKE) -C $(NVT_KDRV_PATH) -j$(JOBS)
	@$(MAKE) -C $(NVT_KFLOW_PATH) -j$(JOBS)
ifneq ($(SDK_CODENAME), ns02401)
	@$(MAKE) -C $(NVT_KFLOW_ISP_PATH) -j$(JOBS)
endif
	@$(MAKE) -C $(NVT_SOURCE_PATH) -j$(JOBS)
	@$(MAKE) -C $(NVT_SOURCE_PUB_PATH) -j$(JOBS)
	@$(MAKE) -C $(NVT_SOURCE_PUB2_PATH) -j$(JOBS)
	#@$(MAKE) -C $(NVT_SAMPLE_PATH) -j$(JOBS)

install:
	@$(MAKE) -C $(NVT_KDRV_PATH) modules_install
	@$(MAKE) -C $(NVT_KFLOW_PATH) modules_install
ifneq ($(SDK_CODENAME), ns02401)
	@$(MAKE) -C $(NVT_KFLOW_ISP_PATH) modules_install
endif
	@$(MAKE) -C $(NVT_SOURCE_PATH) install
	@$(MAKE) -C $(NVT_SOURCE_PUB_PATH) install
	@$(MAKE) -C $(NVT_SOURCE_PUB2_PATH) install

clean:
	@$(MAKE) -C $(NVT_KDRV_PATH) clean
	@$(MAKE) -C $(NVT_KFLOW_PATH) clean
ifneq ($(SDK_CODENAME), ns02401)
	@$(MAKE) -C $(NVT_KFLOW_ISP_PATH) clean
endif
	@$(MAKE) -C $(NVT_SOURCE_PATH) clean
	@$(MAKE) -C $(NVT_SOURCE_PUB_PATH) clean
	@$(MAKE) -C $(NVT_SOURCE_PUB2_PATH) clean
	#@$(MAKE) -C $(NVT_SAMPLE_PATH) clean

codesize:
	@$(MAKE) -C $(NVT_KDRV_PATH) codesize
	@$(MAKE) -C $(NVT_KFLOW_PATH) codesize
ifneq ($(SDK_CODENAME), ns02401)
	@$(MAKE) -C $(NVT_KFLOW_ISP_PATH) codesize
endif
	#@$(MAKE) -C $(NVT_SOURCE_PATH) codesize
	#@$(MAKE) -C $(NVT_SOURCE_PUB_PATH) codesize
