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
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/include -I$(NVT_AI3_PATH)/include -I$(NVT_AI3_PATH)/source_pub/include
INCLUDE_PATH += -I$(NVT_AI_KDRV_PATH)/include -I$(NVT_AI_KFLOW_PATH)/include
INCLUDE_PATH += -I$(NVT_AI3_PATH)/source_pub/prebuilt/include
INCLUDE_PATH += -I$(NVT_AI3_PATH)/source_pub/vendor_ai_cpu/nn_dli_cpu
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/media/$(NVT_SDK_TYPE)/include
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/ai3/include/vendor
INCLUDE_PATH += -I$(NVT_AI3_PATH)/source/include

ifeq ("$(wildcard $(OWN_NVT_AI3_PATH))", "")
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/prebuilt/include/nvtnn
else
INCLUDE_PATH += -I$(OWN_NVT_AI3_PATH)/source_pub/prebuilt/include/nvtnn
endif

uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)

#ifeq ($(uclibc),) # ACL library doesn't support uclibc
#nn_dli_cpu_acl = no
#endif
nn_dli_cpu_acl = no

#nn_dli_cpu_nvtnn = yes
nn_dli_cpu_nvtnn = no

nn_dli_cpu_acl_softmax = no

#nn_dli_cpu_nvtnn_softmax = yes
nn_dli_cpu_nvtnn_softmax = no

#ifeq ($(findstring$(nn_dli_cpu_acl)$(nn_dli_cpu_nvtnn), yes, yes), )
# Path of ACL's header
#INCLUDE_PATH += -I$(NVT_AI3_PATH)/source_pub/vendor_ai_cpu/nn_dli_cpu/prebuilt/include/ComputeLibrary-21.11
#nn_dli_cpu = yes
#endif
nn_dli_cpu = no

ifeq ($(nn_dli_cpu), yes)
COPS += -D NN_DLI_CPU
endif


#COPS = $(INCLUDE_PATH) -D ARM_ASM -O3 -Wall -Werror -finline-functions -msoft-float -fPIC -Wno-aggressive-loop-optimizations
COPS = $(INCLUDE_PATH) -D ARM_ASM -O3 -Wall -Werror -finline-functions -fPIC -Wno-aggressive-loop-optimizations $(NVT_ASAN)
ifeq ($(nn_dli_cpu_acl), yes)
COPS += -D NN_DLI_CPU_ACL
# prebuilt ACL library
PREBUILD_DLI_CPU_LIB=$(NVT_AI3_PATH)/source_pub/vendor_ai_cpu/nn_dli_cpu/prebuilt/lib/glibc
endif

ifeq ($(nn_dli_cpu_acl_softmax), yes)
COPS += -D NN_DLI_CPU_ACL_SOFTMAX
endif

ifeq ($(nn_dli_cpu_nvtnn), yes)
COPS += -D NN_DLI_CPU_NVTNN
# prebuilt ACL library
PREBUILD_DLI_CPU_LIB=$(NVT_AI3_PATH)/source_pub/vendor_ai_cpu/nn_dli_cpu/prebuilt/lib/glibc
endif

ifeq ($(nn_dli_cpu_nvtnn_softmax), yes)
COPS += -D NN_DLI_CPU_NVTNN_SOFTMAX
endif

CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
AR=$(CROSS_COMPILE)ar

CSRC += \
	vendor_ai_cpu/vendor_ai_cpu.c \
	vendor_ai_cpu/vendor_ai_cpu_custnn.c \
	vendor_ai_cpu/vendor_ai_cpu_custnn_eltwise.c \
	vendor_ai_cpu/vendor_ai_cpu_custnn_concat.c \
	vendor_ai_cpu/vendor_ai_cpu_custnn_slice.c \
	vendor_ai_cpu/vendor_ai_cpu_nvtnn.c \
	vendor_ai_cpu/vendor_ai_cpu_prelu.c \
	vendor_ai_cpu/vendor_ai_cpu_proposal.c \
	vendor_ai_cpu/vendor_ai_cpu_softmax.c \
	vendor_ai_cpu/vendor_ai_cpu_util.c \

CPPSRC += \
	vendor_ai_cpu/vendor_ai_cpu_custnn_pool.cpp

ifeq ($(nn_dli_cpu), yes)
CPPSRC += \
	vendor_ai_cpu/nn_dli_cpu/vendor_ai_dli_cpu.cpp \
	vendor_ai_cpu/nn_dli_cpu/vendor_ai_dli_cpu_utils.cpp
endif

ifeq ($(nn_dli_cpu_acl), yes)
CPPSRC += \
	vendor_ai_cpu/nn_dli_cpu/vendor_ai_dli_cpu_elementwise.cpp \
	vendor_ai_cpu/nn_dli_cpu/vendor_ai_dli_cpu_resize.cpp
ifeq ($(nn_dli_cpu_acl_softmax), yes)
CPPSRC += \
	vendor_ai_cpu/nn_dli_cpu/vendor_ai_dli_cpu_softmax.cpp
endif
endif

ifeq ($(nn_dli_cpu_nvtnn), yes)
CPPSRC += \
	vendor_ai_cpu/nn_dli_cpu/vendor_ai_dli_cpu_pooling.cpp \
	vendor_ai_cpu/nn_dli_cpu/vendor_ai_dli_cpu_reverse.cpp \
	vendor_ai_cpu/nn_dli_cpu/vendor_ai_dli_cpu_permute.cpp
ifeq ($(nn_dli_cpu_nvtnn_softmax), yes)
CPPSRC += \
	vendor_ai_cpu/nn_dli_cpu/vendor_ai_dli_cpu_softmax.cpp
endif
endif

HDAL_STATIC_LIB = libvendor_ai3_pub.a
HDAL_DYNAMIC_LIB = libvendor_ai3_pub.so

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
ifeq ($(nn_dli_cpu_acl), yes)
	@cp $(PREBUILD_DLI_CPU_LIB)/*.so $(OUTPUT_DIR)
endif

ifeq ($(nn_dli_cpu_nvtnn), yes)
	@cp $(PREBUILD_DLI_CPU_LIB)/*.so $(OUTPUT_DIR)
endif

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
ifeq ($(nn_dli_cpu_acl), yes)
	@cp $(PREBUILD_DLI_CPU_LIB)/*.a $(OUTPUT_DIR)
endif

ifeq ($(nn_dli_cpu_nvtnn), yes)
	@cp $(PREBUILD_DLI_CPU_LIB)/*.a $(OUTPUT_DIR)
endif

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
