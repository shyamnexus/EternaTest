MODULE_NAME = speakerphone_test
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
RTOS_OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
# INCs
LIBRARY_INC_PATH = $(LIBRARY_DIR)/include
THIS_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
# libs
LIBRARY_LIB_PATH = $(LIBRARY_DIR)/output

VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
# INC FLAGs
EXTRA_INCLUDE += -I$(LIBRARY_INC_PATH) -I$(HDAL_INC_PATH) -I$(VOS_INC_PATH) -I$(NVT_HDAL_DIR)/vendor/media/include -I../include -I../apm_test/include/ -I../
.PHONY: all clean release
###############################################################################
# Linux Makefile                                                              #
###############################################################################
uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)
ifeq ($(uclibc),)
    ENV_LIBC=glibc
else
    ENV_LIBC=uclibc
endif

ifeq ($(NVT_PRJCFG_CFG),Linux)
#--------- ENVIRONMENT SETTING --------------------
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Werror
COMPILE_OPTS	=  -I. -O2 -fPIC -ffunction-sections -fdata-sections -D__LINUX
C_CFLAGS	= $(PLATFORM_CFLAGS) $(COMPILE_OPTS) $(WARNING) $(EXTRA_INCLUDE)
LD_FLAGS	= -L$(LIBRARY_LIB_PATH) -L$(HDAL_LIB_PATH) -L$(LIBRARY_LIB_PATH) -L$(HDAL_LIB_PATH) -L$(NVT_HDAL_DIR)/vendor/media/source -lhdal -lpthread -laec -lstdc++ -lm  -Llib/$(ENV_LIBC)/static -lapm  -lvendor_media -lagc -laudfilt
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = $(MODULE_NAME)
SRC = speakerphone_test.c ns_func.c agc_func.c eq_func.c
#SRC = eq_func.c

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
	@find -name __pycache__ | xargs -I {} rm -rf {}
	@rm -f $(OBJ) $(LIB_NAME).sym *.o *.a *.so* *.sh $(MODULE_NAME)

install:
	@cp -avf $(LIB_NAME) $(ROOTFS_DIR)/rootfs/usr/bin

endif

endif
