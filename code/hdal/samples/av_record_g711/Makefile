# Makefile for Combined Audio/Video Recording with G.711 Encoding
# Builds two separate executables:
#   - av_record_g711_ulaw: Video + G.711 μ-law audio
#   - av_record_g711_alaw: Video + G.711 A-law audio

# Default module for backward compatibility
MODULE_NAME = av_record_g711_ulaw

# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output

# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output

# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(VOS_INC_PATH)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)

EXTRA_CFLAGS += $(NVT_ASAN) -D__LINUX
EXTRA_LDFLAGS = $(NVT_ASAN) -L$(HDAL_LIB_PATH) -lhdal -lpthread

# Source files for each target
SRC_ULAW = av_record_g711_ulaw.c
SRC_ALAW = av_record_g711_alaw.c

# Object files
OBJ_ULAW = $(SRC_ULAW:.c=.o)
OBJ_ALAW = $(SRC_ALAW:.c=.o)

# Targets
TARGET_ULAW = av_record_g711_ulaw
TARGET_ALAW = av_record_g711_alaw

# Default target - build both
.PHONY: all clean install

all: $(TARGET_ULAW) $(TARGET_ALAW)

$(TARGET_ULAW): $(OBJ_ULAW)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^ $(LDFLAGS) $(EXTRA_LDFLAGS)
	@echo "Built: $@"

$(TARGET_ALAW): $(OBJ_ALAW)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^ $(LDFLAGS) $(EXTRA_LDFLAGS)
	@echo "Built: $@"

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(EXTRA_INCLUDE) -c -o $@ $<

clean:
	rm -f $(OBJ_ULAW) $(OBJ_ALAW) $(TARGET_ULAW) $(TARGET_ALAW)

install: all
	@mkdir -p $(OUTPUT_DIR)
	cp -f $(TARGET_ULAW) $(OUTPUT_DIR)/
	cp -f $(TARGET_ALAW) $(OUTPUT_DIR)/
	@echo "Installed to: $(OUTPUT_DIR)"

# For compatibility with the SDK build system
SRC = $(SRC_ULAW)

###############################################################################
# RTOS Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

# DIRs
RTOS_KERNEL_DIR = $(KERNELDIR)/lib/FreeRTOS
RTOS_LIB_DIR = $(KERNELDIR)/lib
RTOS_CURR_DEMO_DIR = $(KERNELDIR)/demos/novatek/$(SDK_CODENAME)
RTOS_POSIX_DIR = $(KERNELDIR)/lib/FreeRTOS-Plus-POSIX
RTOS_POSIX_SRC_DIR = $(RTOS_POSIX_DIR)/source

# INCs for EXTRA_CFLAGS
EXTRA_INCLUDE += \
	-I$(RTOS_LIB_DIR) \
	-I$(RTOS_KERNEL_DIR)/portable/GCC/ARM_CA9 \
	-I$(RTOS_KERNEL_DIR)/include \
	-I$(RTOS_KERNEL_DIR)/include/private \
	-I$(RTOS_CURR_DEMO_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include/portable/novatek \
	-I$(LIBRARY_DIR)/include

EXTRA_CFLAGS += -Wno-format

# For RTOS, only build one at a time based on MODULE_NAME
ifeq ($(MODULE_NAME),av_record_g711_alaw)
SRC = av_record_g711_alaw.c
else
SRC = av_record_g711_ulaw.c
endif

endif

###############################################################################
# Include common build rules                                                  #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
# For Linux, we handle the build ourselves above
else
include $(NVT_HDAL_DIR)/samples/sample.mk
endif
