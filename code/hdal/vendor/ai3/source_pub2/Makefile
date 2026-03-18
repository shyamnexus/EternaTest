current_mk_abspathname := $(abspath $(lastword $(MAKEFILE_LIST)))
current_mk_absdir := $(dir $(current_mk_abspathname))../../ai_sdk_env.mk
ifneq ("$(wildcard $(current_mk_absdir))", "")
include $(current_mk_absdir)
endif

CUR_PATH := $(shell pwd)

ifeq ("$(wildcard $(OWN_NVT_AI3_PATH))", "")
NVT_AI3_PATH = $(NVT_HDAL_DIR)/vendor/ai3
NVT_AI_KDRV_PATH=$(NVT_AI3_PATH)/drivers/k_driver
NVT_AI_KFLOW_PATH=$(NVT_AI3_PATH)/drivers/k_flow
else
NVT_AI_KFLOW_PATH=$(NVT_AI3_KFLOW_PATH)
endif

NVT_KDRV_PATH = $(NVT_HDAL_DIR)/drivers/k_driver
NVT_KFLOW_PATH = $(NVT_HDAL_DIR)/drivers/k_flow/$(NVT_SDK_TYPE)

OUTPUT_DIR = $(NVT_HDAL_DIR)/vendor/output

INCLUDE_PATH += -I$(NVT_VOS_DIR)/drivers/include
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/include -I$(NVT_KDRV_PATH)/include -I$(NVT_KDRV_PATH)/source/include/ -I$(NVT_KFLOW_PATH)/include/
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/include -I$(NVT_AI3_PATH)/include -I$(NVT_AI3_PATH)/source_pub2/include
INCLUDE_PATH += -I$(NVT_AI_KDRV_PATH)/include -I$(NVT_AI_KFLOW_PATH)/include
INCLUDE_PATH += -I$(NVT_AI3_PATH)/source_pub/prebuilt/include
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/dsp/include -I$(NVT_AI3_PATH)/source/include
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/media/$(NVT_SDK_TYPE)/include
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/ai3/include/vendor
INCLUDE_PATH += -I$(NVT_AI3_PATH)/source/include

ifeq ("$(wildcard $(OWN_NVT_AI3_PATH))", "")
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/prebuilt/include/nvtnn
else
INCLUDE_PATH += -I$(OWN_NVT_AI3_PATH)/source_pub/prebuilt/include/nvtnn
endif

nn_dli_dsp = no

#COPS = $(INCLUDE_PATH) -D ARM_ASM -O3 -Wall -Werror -finline-functions -msoft-float -fPIC -Wno-aggressive-loop-optimizations
COPS = $(INCLUDE_PATH) -D ARM_ASM -O3 -Wall -Werror -finline-functions -fPIC -Wno-aggressive-loop-optimizations $(NVT_ASAN)

CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
AR=$(CROSS_COMPILE)ar

CSRC += \
	vendor_ai_dsp/vendor_ai_dsp.c \
	vendor_ai_dsp/vendor_ai_dsp_custnn.c \
	vendor_ai_dsp/vendor_ai_dsp_custnn_pool.c \
	vendor_ai_dsp/vendor_ai_dsp_nvtnn.c

ifeq ($(nn_dli_dsp), yes)
COPS += -D NN_DLI_DSP_ACL
CPPSRC += \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_softmax.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_fc.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_dilatedconv.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_resize.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_deconvolution.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_deconvolution_depthwise.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_elementwise.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_instancenorm.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_layernorm.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_matmul.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_priorbox.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_norm.cpp \
	vendor_ai_dsp/nn_dli_dsp/vendor_ai_dli_dsp_utils.cpp
endif

HDAL_STATIC_LIB = libvendor_ai3_pub2.a
HDAL_DYNAMIC_LIB = libvendor_ai3_pub2.so

COBJS = ${CSRC:%.c=%.o} ${CPPSRC:%.cpp=%.o}

.PHONY: $(TARGETS)
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
TARGETS := $(HDAL_STATIC_LIB) $(HDAL_DYNAMIC_LIB)

all: $(TARGETS)

$(HDAL_STATIC_LIB) : $(COBJS) $(SOBJS)
	@$(AR) -r $@ $(COBJS)
#	mkdir -p ./lib
#	cp $@ ./lib/

$(HDAL_DYNAMIC_LIB) : $(COBJS) $(SOBJS)
	@$(CC) -shared -s -o $@ $(COBJS)
#	mkdir -p ../lib
#	cp $@ ./lib/

%.o: %.c
	@$(CC) -c $(COPS) $(CFLAGS) -I. -o $@ $<

%.o: %.cpp
	@$(CC) -c $(COPS) $(CFLAGS) -I. -o $@ $<

install:
	@cp *.so *.a $(OUTPUT_DIR)

clean :
	/bin/rm -f *.o
	/bin/rm -f */*.o
	/bin/rm -f */*/*.o
	/bin/rm -f $(HDAL_STATIC_LIB) $(HDAL_DYNAMIC_LIB)

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

ifeq ($(SDK_CODENAME), $(filter ns02302, $(SDK_CODENAME))) # 538 support RTOS

TARGETS := $(HDAL_STATIC_LIB)

ifeq ($(findstring x64,$(RTOS_CPU_TYPE)),x64)
ENV_LIBC=rtos64
else
ENV_LIBC=rtos
endif

C_PREDEFINEDS += \

WARNINGS += \
	-Wno-format \

C_CFLAGS = \
	$(PLATFORM_CFLAGS) \
	$(WARNINGS) \
	$(C_PREDEFINEDS) \
	$(EXTRA_INCLUDE) \
	$(EXTRA_CFLAGS) \

C_CFLAGS += -Wno-error=undef -Wno-undef

C_CXXFLAGS = \
	$(PLATFORM_CXXFLAGS) \
	$(WARNINGS) \
	$(C_PREDEFINEDS) \
	$(EXTRA_INCLUDE) \
	$(EXTRA_CPPFLAGS) \

C_AFLAGS = \
	$(PLATFORM_AFLAGS) \
	$(WARNINGS) \
	$(C_PREDEFINEDS) \
	$(EXTRA_INCLUDE) \
	$(EXTRA_AFLAGS) \

all: $(TARGETS)

$(HDAL_STATIC_LIB) : $(COBJS) $(SOBJS)
	@$(AR) rcsD $@ $(COBJS)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@

%.o: %.c
	@echo Compiling $<
	@$(CC) -c $(COPS) $(C_CFLAGS) $(CFLAGS) -I. -o $@ $<

%.o: %.cpp
	@echo Compiling $<
	@$(CC) -c $(COPS) $(C_CXXFLAGS) $(CFLAGS) -I. -o $@ $<

%.o: %.S
	@echo Assembling $<
	@$(CC) $(C_AFLAGS) -c $< -o $@

install:
	@cp *.a $(OUTPUT_DIR)

clean :
	/bin/rm -f *.o
	/bin/rm -f */*.o
	/bin/rm -f */*/*.o
	/bin/rm -f $(HDAL_STATIC_LIB)

else # other ic don't support RTOS

all:
	@echo "nothing to be done for '$(LIB_NAME)'"
clean:
	@echo "nothing to be done for '$(LIB_NAME)'"

install:
	@echo "nothing to be done for '$(LIB_NAME)'"

endif # ifeq ($(SDK_CODENAME), $(filter ns02302, $(SDK_CODENAME)))

endif
