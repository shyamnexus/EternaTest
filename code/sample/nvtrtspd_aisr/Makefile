#--------- ENVIRONMENT SETTING --------------------
AI_LIB        = $(NVT_HDAL_DIR)/vendor/ai2/source
uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)
ifeq ($(uclibc),)
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai3/source_pub/prebuilt/lib/glibc
else
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai3/source_pub/prebuilt/lib/uclibc
endif
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output
VOS_LIB_PATH  = $(NVT_VOS_DIR)/output

INCLUDES	= -I$(NVT_HDAL_DIR)/include -I$(LIBRARY_DIR)/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_driver/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/source/kflow_ai_net
INCLUDES += -I$(NVT_VOS_DIR)/drivers/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/isp/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/media/include
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes
COMPILE_OPTS	= $(INCLUDES) -I. -O2 -fPIC -ffunction-sections -fdata-sections
CPPFLAGS	=
CFLAGS		= $(PLATFORM_CFLAGS) $(PRJCFG_CFLAGS) -D__LINUX520 -D__LINUX -D_NETWORK_ON_CPU1_
C_FLAGS		= $(NVT_ASAN) $(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS) $(WARNING)
LD_FLAGS	= $(NVT_ASAN) -L$(LIBRARY_DIR)/output -L$(NVT_HDAL_DIR)/output -lstdc++ -lpthread -lm -lrt -lnvtlive555 -lhdal -L$(NVT_HDAL_DIR)/vendor/output -lvendor_isp -lprebuilt_isp -lvendor_media
LD_FLAGS    += -L$(VOS_LIB_PATH) -lvos
LD_FLAGS 	+= -L$(AI_LIB) -lvendor_ai3
LD_FLAGS 	+= -L$(AI_LIB)_pub -lvendor_ai3_pub
LD_FLAGS 	+= -L$(VENDOR_LIB_PATH) -lvendor_media
LD_FLAGS	+= -L$(PREBUILD_LIB) -lprebuilt_ai

#--------- END OF ENVIRONMENT SETTING -------------
DEP_LIBRARIES := nvt@lib@nvtlive555
#--------- Compiling -------------------
BIN = nvtrtspd_aisr
SRC = nvtrtspd.c video_record.c ai_cpu_post_accuracy.c ai3_net.c thermal_lib.c
HEADER = $(shell find . -name "*.h")

OBJ = $(SRC:.c=.o)

.PHONY: all clean install

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo ">>> Skip"
clean:
	@echo ">>> Skip"

else
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LD_FLAGS)
	$(NM) -n $@ > $@.sym
	$(STRIP) $@
	$(OBJCOPY) -R .comment -R .note.ABI-tag -R .gnu.version $@

%.o: %.c $(HEADER)
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	rm -vf $(BIN) $(OBJ) $(BIN).sym *.o *.a *.so*
endif

install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@mkdir -p $(APP_DIR)/output
	@cp -avf $(BIN) $(APP_DIR)/output
	@cp -avf ${BIN} ${ROOTFS_DIR}/rootfs/bin/
