MODULE_NAME = aov1
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
RTOS_OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
AI_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/ai/drivers
AI_LIB_DIR = $(NVT_HDAL_DIR)/vendor/ai3
CV_LIB_DIR = $(NVT_HDAL_DIR)/vendor/cv
VENDOR_LIB_DIR  = $(NVT_HDAL_DIR)/vendor
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VOS_LIB_PATH  = $(NVT_VOS_DIR)/output
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output
AI_LIB        = $(NVT_HDAL_DIR)/vendor/ai3/source
AI_LIB_INC_PATH = $(AI_LIB_DIR)/include
CV_LIB_INC_PATH = $(CV_LIB_DIR)/include
CV_LIB_LIB_PATH = $(CV_LIB_DIR)/source
ALG_CV_LIB_INC_PATH = $(CV_LIB_DIR)/include/libmd
ALG_CV_LIB_PATH = $(VENDOR_LIB_DIR)/output
uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)
ifeq ($(uclibc),)
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/third_party/pvdcnn/prebuilt/glibc
	PREBUILD_LIB_2=$(NVT_HDAL_DIR)/vendor/third_party/limit_fdet/prebuilt/glibc
	PREBUILD_LIB_3=$(NVT_HDAL_DIR)/vendor/third_party/modules_lib/prebuilt/glibc
else
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/third_party/pvdcnn/prebuilt/uclibc
	PREBUILD_LIB_2=$(NVT_HDAL_DIR)/vendor/third_party/limit_fdet/prebuilt/uclibc
	PREBUILD_LIB_3=$(NVT_HDAL_DIR)/vendor/third_party/modules_lib/prebuilt/uclibc
endif
# INC FLAGs
EXTRA_INCLUDE += -I$(HDAL_INC_PATH) -I$(VOS_INC_PATH)
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/isp/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/media/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai/drivers/k_driver/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/source/kflow_ai_net
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/limit_fdet/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/modules_lib/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/third_party/pvdcnn/include
EXTRA_INCLUDE += -I$(NVT_VOS_DIR)/drivers/include
EXTRA_INCLUDE += -I$(AI_DRIVER_DIR)/k_driver/include
EXTRA_INCLUDE += -I$(AI_DRIVER_DIR)/k_flow/include
EXTRA_INCLUDE += -I$(AI_LIB_INC_PATH)/vendor_ai
EXTRA_INCLUDE += -I$(CV_LIB_INC_PATH)
EXTRA_INCLUDE += -I$(ALG_CV_LIB_INC_PATH)
.PHONY: all clean
###############################################################################
# Linux Makefile                                                              #
###############################################################################
#--------- ENVIRONMENT SETTING --------------------
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes -Werror  -Wno-unused-variable -Wno-unused-function -Wno-unused-label
COMPILE_OPTS	=  -I. -O2 -fPIC -ffunction-sections -fdata-sections -D__LINUX
C_CFLAGS	= $(PLATFORM_CFLAGS) $(COMPILE_OPTS) $(WARNING) $(EXTRA_INCLUDE)
LD_FLAGS	= $(NVT_GCOV) $(NVT_ASAN) -L$(HDAL_LIB_PATH) -lhdal -lpthread
LD_FLAGS	+= -L$(NVT_HDAL_DIR)/vendor/output -lvendor_media -lvendor_isp
LD_FLAGS    += -L$(VOS_LIB_PATH) -lvos
LD_FLAGS 	+= -L$(AI_LIB) -lvendor_ai3
LD_FLAGS 	+= -L$(VENDOR_LIB_PATH) -lvendor_media
LD_FLAGS	+= -L$(PREBUILD_LIB_2) -lprebuilt_limit_fdet
LD_FLAGS	+= -L$(PREBUILD_LIB) -lprebuilt_pvdcnn
LD_FLAGS	+= -L$(PREBUILD_LIB_3) -lprebuilt_modules_lib
LD_FLAGS	+= -L$(ALG_CV_LIB_PATH) -lvendor_cv -lprebuilt_cv
LD_FLAGS 	+= -L$(NVT_HDAL_DIR)/vendor/ai3/source_pub -lvendor_ai3_pub
LD_FLAGS 	+= -L$(NVT_HDAL_DIR)/vendor/output -lprebuilt_ai -lstdc++
LD_FLAGS	+= -lm
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = $(MODULE_NAME)
SRC = main.c aov1_ctrl.c alg_md.c ai3_pvd.c aov1_filesave.c

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
