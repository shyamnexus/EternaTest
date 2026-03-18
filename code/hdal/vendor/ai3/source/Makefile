current_mk_abspathname := $(abspath $(lastword $(MAKEFILE_LIST)))
current_mk_absdir := $(dir $(current_mk_abspathname))../../ai_sdk_env.mk
ifneq ("$(wildcard $(current_mk_absdir))", "")
include $(current_mk_absdir)
endif

CUR_PATH := $(shell pwd)
NVT_KDRV_PATH = $(NVT_HDAL_DIR)/drivers/k_driver
NVT_KFLOW_PATH = $(NVT_HDAL_DIR)/drivers/k_flow/ipc
#NVT_KFLOW_PATH = $(NVT_HDAL_DIR)/drivers/k_flow/$(NVT_SDK_TYPE)

ifeq ("$(wildcard $(OWN_NVT_AI3_PATH))", "")
NVT_AI3_PATH = $(NVT_HDAL_DIR)/vendor/ai3
NVT_AI_KDRV_PATH=$(NVT_AI3_PATH)/drivers/k_driver
NVT_AI_KFLOW_ISP_PATH=$(NVT_AI3_PATH)/drivers/k_flow_isp
NVT_AI_KFLOW_PATH=$(NVT_AI3_PATH)/drivers/k_flow
else
NVT_AI_KFLOW_PATH=$(NVT_AI3_KFLOW_PATH)
NVT_AI_KFLOW_ISP_PATH=$(NVT_AI3_KFLOW_ISP_PATH)
endif

NVT_AI_LIB_PATH=$(NVT_AI3_PATH)/source
OUTPUT_DIR = $(NVT_HDAL_DIR)/vendor/output

INCLUDE_PATH += -I$(NVT_VOS_DIR)/include
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/include
INCLUDE_PATH += -I$(NVT_KDRV_PATH)/include -I$(NVT_KDRV_PATH)/source/include/ -I$(NVT_KFLOW_PATH)/include/
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/include -I$(NVT_AI_LIB_PATH)/include -I$(NVT_AI3_PATH)/include -I$(NVT_AI_KDRV_PATH)/include -I$(NVT_AI_KFLOW_PATH)/include -I $(NVT_AI_KFLOW_ISP_PATH)/include
INCLUDE_PATH += -I$(NVT_AI3_PATH)/source_pub

ifeq ("$(wildcard $(OWN_NVT_AI3_PATH))", "")
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/prebuilt/include
else
INCLUDE_PATH += -I$(OWN_NVT_AI3_PATH)/source_pub/prebuilt/include 
endif

INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/cv/include -I$(NVT_HDAL_DIR)/vendor/isp/include
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/dsp/include


ifeq ($(SDK_CODENAME), ns02201)
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/media/$(NVT_SDK_TYPE)/include
else ifeq  ($(SDK_CODENAME), ns02302)
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/media/include
else ifeq  ($(SDK_CODENAME), ns02401)
INCLUDE_PATH += -I$(NVT_HDAL_DIR)/vendor/media/include
endif

CFLAGS += '-Wno-address-of-packed-member'

#COPS = $(INCLUDE_PATH) -D ARM_ASM -O3 -Wall -Werror -finline-functions -msoft-float -fPIC -Wno-aggressive-loop-optimizations
COPS = $(INCLUDE_PATH) -D ARM_ASM -O3 -Wall -Werror -Werror=sign-compare -finline-functions -fPIC -Wno-aggressive-loop-optimizations $(NVT_ASAN)

ifeq ("$(wildcard $(OWN_NVT_AI3_PATH))", "")
uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)
ifeq ($(uclibc),)
    PREBUILD_DIR = $(NVT_AI3_PATH)/source_pub/prebuilt/lib/glibc
    PREBUILD_LIB = $(NVT_AI3_PATH)/source_pub/prebuilt/lib/$(SDK_CODENAME)/glibc
else
    PREBUILD_DIR = $(NVT_AI3_PATH)/source_pub/prebuilt/lib/uclibc
    PREBUILD_LIB = $(NVT_AI3_PATH)/source_pub/prebuilt/lib/$(SDK_CODENAME)/uclibc
endif
endif

CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
AR=$(CROSS_COMPILE)ar

CSRC += \
	vendor_ai_main.c \
	vendor_ai_comm.c \
	vendor_ai_comm_flow.c \
	vendor_ai_net/vendor_ai_net_gen.c \
	vendor_ai_net/vendor_ai_net_fc.c \
	vendor_ai_net/vendor_ai_net_flow.c \
	vendor_ai_net/vendor_ai_net.c \
	vendor_ai_net/vendor_ai_net_mem.c \
	vendor_ai_net/vendor_ai_net_group.c \
	vendor_ai_net/vendor_ai_net_debug.c \
	vendor_ai_net/vendor_ai_net_cmd.c \
	vendor_ai_net/vendor_ai_net_cat.c \
	vendor_ai_net/vendor_ai_genll.c \
	vendor_ai_net/vendor_ai_op.c \
	vendor_ai_net/debug_util/graph_debug_core.c \
	vendor_ai_net/debug_util/graph_debug_buffer.c \
	vendor_ai_net/debug_util/graph_debug_schedule.c \
	vendor_ai_net/debug_util/graph_debug_log.c \
	vendor_ai_cpu/vendor_ai_cpu_thread.c \
	vendor_ai_dsp/vendor_ai_dsp_thread.c \
	vendor_ai_dla/vendor_ai_dla.c

HDAL_STATIC_LIB = libvendor_ai3.a
HDAL_DYNAMIC_LIB = libvendor_ai3.so

COBJS = ${CSRC:%.c=%.o} ${CPPSRC:%.cpp=%.o}

.PHONY: $(TARGETS)
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
TARGETS := $(HDAL_STATIC_LIB) $(HDAL_DYNAMIC_LIB)
COPS += -D__LINUX_USER__

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
clean:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
codesize:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
else
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

clean :
	/bin/rm -f *.o
	/bin/rm -f */*.o
	/bin/rm -f */*/*.o
	/bin/rm -f $(HDAL_STATIC_LIB) $(HDAL_DYNAMIC_LIB)
	/bin/rm -f $(PREBUILD_DIR)/*.a
	/bin/rm -f $(PREBUILD_DIR)/*.so
endif

install:
#	@cp *.so *.a $(ROOTFS_DIR)/rootfs/lib
	@cp *.so *.a $(OUTPUT_DIR)
#	@cp $(PREBUILD_LIB)/*.so $(PREBUILD_LIB)/*.a $(ROOTFS_DIR)/rootfs/lib
	@cp $(PREBUILD_LIB)/*.so $(PREBUILD_LIB)/*.a $(OUTPUT_DIR)
	@mkdir -p $(PREBUILD_DIR)
	@cp $(PREBUILD_LIB)/*.so $(PREBUILD_LIB)/*.a $(PREBUILD_DIR)

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

clean :
	/bin/rm -f *.o
	/bin/rm -f */*.o
	/bin/rm -f */*/*.o
	/bin/rm -f $(HDAL_STATIC_LIB)
	/bin/rm -f $(PREBUILD_DIR)/*.a

install:
	@cp *.a $(OUTPUT_DIR)
	@cp $(PREBUILD_LIB)/*.a $(OUTPUT_DIR)
	@mkdir -p $(PREBUILD_DIR)
	@cp $(PREBUILD_LIB)/*.a $(PREBUILD_DIR)

else # other ic don't support RTOS

all:
	@echo "nothing to be done for '$(LIB_NAME)'"
clean:
	@echo "nothing to be done for '$(LIB_NAME)'"

install:
	@echo "nothing to be done for '$(LIB_NAME)'"

endif # ifeq ($(SDK_CODENAME), $(filter ns02302, $(SDK_CODENAME)))

endif
