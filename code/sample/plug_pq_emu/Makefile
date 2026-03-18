include $(NVT_PRJCFG_MODEL_CFG)

#--------- ENVIRONMENT SETTING --------------------
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VOS_LIB_PATH  = $(NVT_VOS_DIR)/output
AI_LIB        = $(NVT_HDAL_DIR)/vendor/ai3/source

uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)
ifeq ($(uclibc),)
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai/source/prebuilt/lib/glibc
else
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai/source/prebuilt/lib/uclibc
endif

INCLUDES = -I$(LIBRARY_DIR)/include -I$(HDAL_INC_PATH) -I$(VOS_INC_PATH) -I$(NVT_HDAL_DIR)/vendor/isp/include -I$(NVT_HDAL_DIR)/vendor/media/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/vendor_ai_cpu
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/source_pub/vendor_ai_dsp
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_driver/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/include
INCLUDES += -I$(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow/source/kflow_ai_net
WARNING = -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes
COMPILE_OPTS = $(INCLUDES) -I. -O2 -fPIC -ffunction-sections -fdata-sections
CPPFLAGS =
CFLAGS = $(PLATFORM_CFLAGS) $(PRJCFG_CFLAGS) -D__LINUX520 -D__LINUX -D_NETWORK_ON_CPU1_ -D_BOARD_DRAM_SIZE_=$(BOARD_DRAM_SIZE) -D_CHIP_NT98538_
C_FLAGS = $(NVT_ASAN) $(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS) $(WARNING)
LD_FLAGS = $(NVT_ASAN) -L$(LIBRARY_DIR)/output -L$(NVT_HDAL_DIR)/output -lstdc++ -lpthread -lm -lrt -lhdal -lvendor_isp -lvendor_media -L$(NVT_HDAL_DIR)/vendor/output
LD_FLAGS += -L$(VOS_LIB_PATH) -lvos -L$(AI_LIB) -lvendor_ai3 -L$(AI_LIB)_pub -lvendor_ai3_pub -L$(PREBUILD_LIB) -lprebuilt_ai
#--------- END OF ENVIRONMENT SETTING -------------
BIN = plug_pq_emu
SRC = plug_pq_emu.c ai.c isp.c
SRC_CPP = aiisp_pq.cpp
HEADER = $(shell find . -name "*.h")

OBJ = $(SRC:.c=.o) $(SRC_CPP:.cpp=.o)

.PHONY: all clean install

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo ">>> Skip"
clean:
	@echo ">>> Skip"

else ifeq ($(NVT_SDK_TYPE), xvr)
all:
	@echo "$(MODULE_NAME) is not built. It's only valid for ipc"
clean:
	@echo "$(MODULE_NAME) is not built. It's only valid for ipc"

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

ifeq ($(NVT_SDK_TYPE), xvr)
install:
	@echo "$(MODULE_NAME) is not installed. It's only valid for ipc"
else
install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@mkdir -p $(APP_DIR)/output
	@cp -avf $(BIN) $(APP_DIR)/output
	@cp -avf ${BIN} ${ROOTFS_DIR}/rootfs/bin/
endif
