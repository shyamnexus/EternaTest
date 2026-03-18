current_mk_abspathname := $(abspath $(lastword $(MAKEFILE_LIST)))
current_mk_absdir := $(dir $(current_mk_abspathname))../../../../../ai_sdk_env.mk
ifneq ("$(wildcard $(current_mk_absdir))", "")
include $(current_mk_absdir)
endif


ifeq ("$(wildcard $(OWN_NVT_AI3_PATH))", "")
KDRV_AI_MAKEFILE_PATH = $(NVT_HDAL_DIR)/vendor/ai3/drivers/k_driver/source/kdrv_ai/
else
KDRV_AI_MAKEFILE_PATH = $(NVT_AI_KDRV_PATH)/source/kdrv_ai/
endif


ifeq ($(SDK_CODENAME), ns02302)
include $(KDRV_AI_MAKEFILE_PATH)/Makefile_538
else ifeq  ($(SDK_CODENAME), ns02201)
include $(KDRV_AI_MAKEFILE_PATH)/Makefile_690
else ifeq  ($(SDK_CODENAME), ns02401)
include $(KDRV_AI_MAKEFILE_PATH)/Makefile_635
endif
