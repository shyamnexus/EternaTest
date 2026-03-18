MODULE_NAME = hd_video_record_with_vsp_smart2_aiisp
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
RTOS_OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
VOS_LIB_PATH  = $(NVT_VOS_DIR)/output
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VENDOR_INC_PATH = $(NVT_HDAL_DIR)/vendor/media/include
NVTINFO_INC_PATH = $(LIBRARY_DIR)/source/nvtinfo
AI_LIB        = $(NVT_HDAL_DIR)/vendor/ai3/source
uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)
ifeq ($(uclibc),)
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai/source/prebuilt/lib/glibc
else
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai/source/prebuilt/lib/uclibc
endif
# INC FLAGs
EXTRA_INCLUDE += -I$(HDAL_INC_PATH) -I$(VOS_INC_PATH) -I$(NVT_HDAL_DIR)/vendor/isp/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/media/$(NVT_SDK_TYPE)/include -I$(VENDOR_INC_PATH) -I$(NVTINFO_INC_PATH) -I$(NVT_HDAL_DIR)/vendor/media/$(NVT_SDK_TYPE)/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/vendor_ai_cpu
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/vendor_ai_dsp
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_driver/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/source/kflow_ai_net

EXT_LIBRARY_DIR = -L$(LIBRARY_DIR)/output

.PHONY: all clean
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
#--------- ENVIRONMENT SETTING --------------------
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes -Werror -Wno-unused-function -Wno-unused-variable
COMPILE_OPTS	=  -I. -O2 -fPIC -ffunction-sections -fdata-sections -D__LINUX
C_CFLAGS	= $(NVT_GCOV) $(NVT_ASAN) $(PLATFORM_CFLAGS) $(COMPILE_OPTS) $(WARNING) $(EXTRA_INCLUDE)

# default LD_FLAGS
LD_FLAGS      = $(NVT_GCOV) $(NVT_ASAN) -L$(HDAL_LIB_PATH) -lhdal -lpthread -L$(NVT_HDAL_DIR)/vendor/output -lvendor_media -lvendor_isp -lprebuilt_smart_stitchv2 -lm -lstdc++
LD_FLAGS       += -L$(VOS_LIB_PATH) -lvos
LD_FLAGS 	+= -L$(AI_LIB) -lvendor_ai3
LD_FLAGS 	+= -L$(AI_LIB)_pub -lvendor_ai3_pub
LD_FLAGS	+= -L$(PREBUILD_LIB) -lprebuilt_ai
LD_FLAGS	+= -L$(NVT_HDAL_DIR)/vendor/output -lvendor_isp
# static?
##LD_FLAGS	= $(NVT_ASAN) -L$(HDAL_LIB_PATH) -lhdal -lpthread -L$(NVT_HDAL_DIR)/vendor/output -lvendor_media -lvendor_isp -l:libprebuilt_isp.a -lm -lstdc++

##CXX_LD_FLAGS	= $(EXT_LIBRARY_DIR) -lnvtinfo -lopencv_shape -lopencv_stitching  -lopencv_dnn -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core
CXX_LD_FLAGS	= $(EXT_LIBRARY_DIR) -lnvtinfo  -lopencv_imgproc -lopencv_core -lopencv_imgcodecs -lopencv_tracking -lopencv_video -lopencv_calib3d -lopencv_dnn -lopencv_features2d -lopencv_flann

#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = $(MODULE_NAME)
SRC = video_record_with_vsp_smart2_aiisp.c vsp_dec_func.c aiisp.c

OBJ = $(SRC:.c=.o)

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo "nothing to be done for '$(MODULE_NAME)'"
clean:
	@echo "nothing to be done for '$(MODULE_NAME)'"
else

ifeq ("$(wildcard $(LIBRARY_DIR)/output/libopencv_*.so*)","")
all:
	@echo "skip '$(MODULE_NAME)' before make install opencv lib"
else

all: $(LIB_NAME)

$(LIB_NAME): $(OBJ)
	@echo Creating $@...
	@$(CC) -o $@ $(OBJ) $(LD_FLAGS) $(CXX_LD_FLAGS)
	@$(NM) -n $@ > $@.sym
	@$(STRIP) $@
	@$(OBJCOPY) -R .comment -R .note.ABI-tag -R .gnu.version $@

%.o: %.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

endif

clean:
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so*
endif

install:
ifeq ("$(wildcard $(LIBRARY_DIR)/output/libopencv_*.so*)","")
	@echo "skip '$(MODULE_NAME)' before make install opencv lib"
else
	@cp -avf $(LIB_NAME) $(ROOTFS_DIR)/rootfs/usr/bin
endif
###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
#--------- ENVIRONMENT SETTING --------------------
# DIRs
RTOS_KERNEL_DIR = $(KERNELDIR)/lib/FreeRTOS
RTOS_LIB_DIR = $(KERNELDIR)/lib
RTOS_CURR_DEMO_DIR = $(KERNELDIR)/demos/novatek/$(SDK_CODENAME)
RTOS_POSIX_DIR = $(KERNELDIR)/lib/FreeRTOS-Plus-POSIX
RTOS_POSIX_SRC_DIR = $(RTOS_POSIX_DIR)/source

#INCs for C_CFLAGS

EXTRA_INCLUDE += \
	-I$(RTOS_LIB_DIR) \
	-I$(RTOS_KERNEL_DIR)/portable/GCC/ARM_CA9  \
	-I$(RTOS_KERNEL_DIR)/include \
	-I$(RTOS_KERNEL_DIR)/include/private \
	-I$(RTOS_CURR_DEMO_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include/portable/novatek \
	-I$(LIBRARY_DIR)/include

C_CFLAGS = $(PLATFORM_CFLAGS) $(EXTRA_INCLUDE) -Wno-format
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = lib$(MODULE_NAME).a
SRC =

OBJ = $(SRC:.c=.o)

all: $(LIB_NAME)

$(LIB_NAME): $(OBJ)
	@echo Creating $@...
	@$(AR) rcsD $@ $(OBJ)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@

%.o: %.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

clean:
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so*

install:
	@mkdir -p $(RTOS_OUTPUT_DIR)
	@cp -avf $(LIB_NAME) $(RTOS_OUTPUT_DIR)
endif
