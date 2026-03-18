MODULE_NAME = apm_test
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
RTOS_OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
# INC FLAGs
EXTRA_INCLUDE += -I$(HDAL_INC_PATH) -I$(VOS_INC_PATH) -I include
.PHONY: all clean release_clean release
###############################################################################
# Linux Makefile                                                              #
###############################################################################
linux-v510=$(shell echo $(CROSS_COMPILE)|grep aarch64-ca53-linux-gnueabihf-10.4.0)
uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)


ifeq ($(uclibc),)
    ifeq ($(linux-v510),)
        ENV_LIBC=glibc
    else
        ENV_LIBC=glibc_v510
    endif
else
    ENV_LIBC=uclibc
endif

ifeq ($(NVT_PRJCFG_CFG),Linux)
#--------- ENVIRONMENT SETTING --------------------
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes -Werror -Wformat
COMPILE_OPTS	=  -I. -O2 -fPIC -ffunction-sections -fdata-sections -D__LINUX
C_CFLAGS	= $(PLATFORM_CFLAGS) $(COMPILE_OPTS) $(WARNING) $(EXTRA_INCLUDE) 
LD_FLAGS	= -L$(HDAL_LIB_PATH) -L$(NVT_HDAL_DIR)/vendor/media/source -lpthread -Llib/$(ENV_LIBC)/static -lapm -lstdc++ -lm 
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = $(MODULE_NAME)
SRC = apm_test.c

OBJ = $(SRC:.c=.o)

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
clean:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
else
all: $(LIB_NAME)

$(LIB_NAME): $(OBJ)
ifeq ($(shell test -e ./build.sh && echo yes),yes)
	./build.sh
endif
	@echo Creating $@...
	@$(CC) -o $@ $(OBJ) $(LD_FLAGS)
	@$(NM) -n $@ > $@.sym
	@$(STRIP) $@
	@$(OBJCOPY) -R .comment -R .note.ABI-tag -R .gnu.version $@
	@if [ -d scripts ];then cp $@ scripts; fi

%.o: %.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

clean:
ifeq ($(shell test -e ./build.sh && echo yes),yes)
	./build.sh clean
endif
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so*

release_clean:
	@rm -f $(OBJ) $(LIB_NAME).sym *.o *.a *.so* *.sh

release: all release_clean

install:
	@cp -avf $(LIB_NAME) $(ROOTFS_DIR)/rootfs/usr/bin

endif
endif
