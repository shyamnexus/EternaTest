MODULE_NAME = aec_test
# DIRs
RTOS_OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
RTOS_OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
VOS_LIB_PATH  = $(NVT_VOS_DIR)/output
# INCs
LIBRARY_INC_PATH = $(LIBRARY_DIR)/include
LIBRARY_LIB_PATH = $(LIBRARY_DIR)/output
THIS_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
# INC FLAGs
EXTRA_INCLUDE += -I$(LIBRARY_INC_PATH) -I../include -I$(HDAL_INC_PATH) -I$(VOS_INC_PATH) -I$(NVT_HDAL_DIR)/vendor/media/include -I include -I$(LIBRARY_DIR)/include 
.PHONY: all clean
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
#--------- ENVIRONMENT SETTING --------------------
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Werror
COMPILE_OPTS	=  -I. -O2 -fPIC -ffunction-sections -fdata-sections -D__LINUX
C_CFLAGS	= $(PLATFORM_CFLAGS) $(COMPILE_OPTS) $(WARNING) $(EXTRA_INCLUDE)
#C_CFLAGS += -march=armv7-a -DARMV7 -mfloat-abi=hard
C_CFLAGS += -D__arm__
C_CFLAGS += -DARMV7
C_CFLAGS += -D_ARMV7_
LD_FLAGS	= -L$(HDAL_LIB_PATH) -L$(NVT_HDAL_DIR)/vendor/media/source -lhdal -lpthread -lvendor_media -L$(LIBRARY_DIR)/output -laec -lgcc_s -Llib/static -lstdc++ -lm -L$(VOS_LIB_PATH) -lvos
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = $(MODULE_NAME)
SRC = aec_test.c wavreader.c wavwriter.c


OBJ = $(SRC:.c=.o)

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
clean:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
else
all: $(LIB_NAME) $(DTB)

#Because kernel .dts depend on .dtsi inclusion, they have to be preprocessed first with
#the C preprocessor (cpp). The dtc tool can convert between .dts and .dtb:
$(DTB): $(DTS)
	cpp -nostdinc -I include -undef -x assembler-with-cpp $< > $<.tmp \
	&& dtc -O dtb -b 0 -o $@ $<.tmp
	rm $<.tmp

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
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.so* $(DTB)
endif

install:
	@cp -avf $(LIB_NAME) $(ROOTFS_DIR)/rootfs/usr/bin

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
#--------- ENVIRONMENT SETTING --------------------
C_CFLAGS = $(PLATFORM_CFLAGS) $(EXTRA_INCLUDE)
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = lib$(MODULE_NAME).a
SRC = testecho.c


OBJ = $(SRC:.c=.o)

all: $(LIB_NAME) $(DTB)

#Because kernel .dts depend on .dtsi inclusion, they have to be preprocessed first with
#the C preprocessor (cpp). The dtc tool can convert between .dts and .dtb:
$(DTB): $(DTS)
	cpp -nostdinc -I include -undef -x assembler-with-cpp $< > $<.tmp \
	&& dtc -O dtb -b 0 -o $@ $<.tmp
	rm $<.tmp

$(LIB_NAME): $(OBJ)
	@echo Creating $@...
	@$(AR) rcsD $@ $(OBJ)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@

%.o: %.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

clean:
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so* $(DTB)

install:
	@mkdir -p $(RTOS_OUTPUT_DIR)
	@cp -avf $(LIB_NAME) $(RTOS_OUTPUT_DIR)

endif
