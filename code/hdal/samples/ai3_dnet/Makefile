MODULE_NAME = ai3_dnet
# DIRs
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
ECOS_OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
# INCs
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VOS_LIB_PATH  = $(NVT_VOS_DIR)/output
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output
AI_LIB        = $(NVT_HDAL_DIR)/vendor/ai3/source

uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)
ifeq ($(uclibc),)
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai3/source_pub/prebuilt/lib/glibc
else
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai3/source_pub/prebuilt/lib/uclibc
endif
# INC FLAGs
EXTRA_INCLUDE += -I$(HDAL_INC_PATH) -I$(NVT_HDAL_DIR)/source
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_driver/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/source/kflow_ai_net
EXTRA_INCLUDE += -I$(NVT_VOS_DIR)/drivers/include
.PHONY: all clean
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
#--------- ENVIRONMENT SETTING --------------------
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes -Werror
COMPILE_OPTS	=  -I. -O2 -fPIC -ffunction-sections -fdata-sections -D__LINUX
C_CFLAGS	 = $(NVT_GCOV) $(NVT_ASAN) $(PLATFORM_CFLAGS) $(COMPILE_OPTS) $(WARNING) $(EXTRA_INCLUDE)
#C_CFLAGS        += $(subst xvr, -D_NVT_NVR_SDK_, $(subst ipc, -D_NVT_IPC_SDK_,$(NVT_SDK_TYPE)))
LD_FLAGS   	+= $(NVT_GCOV) $(NVT_ASAN) -L$(HDAL_LIB_PATH) -lpthread -lhdal
LD_FLAGS       += -L$(VOS_LIB_PATH) -lvos
LD_FLAGS 	+= -L$(AI_LIB) -lvendor_ai3
LD_FLAGS 	+= -L$(AI_LIB)_pub -lvendor_ai3_pub
LD_FLAGS 	+= -L$(VENDOR_LIB_PATH) -lvendor_media
LD_FLAGS	+= -L$(PREBUILD_LIB) -lprebuilt_ai
LD_FLAGS	+= -lm
LD_FLAGS	+= -L$(LIBRARY_DIR) -lstdc++
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = $(MODULE_NAME)
SRC = ai3_dnet.c ai_cpu_postproc.c

OBJ = $(SRC:.c=.o)

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
clean:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
else
all: $(LIB_NAME)

$(LIB_NAME): $(OBJ)
	@echo Creating $@...
	@$(CC) -o $@ $(OBJ) $(LD_FLAGS)
	@$(NM) -n $@ > $@.sym
	@$(STRIP) $@
	@$(OBJCOPY) -R .comment -R .note.ABI-tag -R .gnu.version $@

%.o: %.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

clean:
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so*
endif

install:
	@cp -avf $(LIB_NAME) $(ROOTFS_DIR)/rootfs/usr/bin

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

# DIRs
RTOS_KERNEL_DIR = $(KERNELDIR)/lib/FreeRTOS
RTOS_LIB_DIR = $(KERNELDIR)/lib
RTOS_CURR_DEMO_DIR = $(KERNELDIR)/demos/novatek/$(SDK_CODENAME)
RTOS_POSIX_DIR = $(KERNELDIR)/lib/FreeRTOS-Plus-POSIX
RTOS_POSIX_SRC_DIR = $(RTOS_POSIX_DIR)/source

OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output

#INCs for EXTRA_CFLAGS

EXTRA_INCLUDE += \
	-I$(RTOS_LIB_DIR) \
	-I$(RTOS_KERNEL_DIR)/portable/GCC/ARM_CA9  \
	-I$(RTOS_KERNEL_DIR)/include \
	-I$(RTOS_KERNEL_DIR)/include/private \
	-I$(RTOS_CURR_DEMO_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include/portable/novatek \
	-I$(LIBRARY_DIR)/include

EXTRA_CFLAGS += -Wno-format

SRC = ai3_dnet.c ai_cpu_postproc.c

include $(NVT_HDAL_DIR)/executable.mk
endif
