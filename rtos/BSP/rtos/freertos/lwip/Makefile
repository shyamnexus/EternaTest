MODULE_NAME = freertos-lwip
# DIRs
KERNELDIR = ..
OUTPUT_DIR = $(KERNELDIR)/output
# INC FLAGs
EXTRA_INCLUDE += \
	-I${KERNELDIR}/include/freertos \
	-I${KERNELDIR}/include/freertos-port \
	-I${KERNELDIR}/include/lwip \
	-I${KERNELDIR}/include/lwip-port \
	-I./src/include \

.PHONY: all clean
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
#--------- ENVIRONMENT SETTING --------------------
WARNING	= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes -Werror
COMPILE_OPTS	= -I. -O2 -fPIC -ffunction-sections -fdata-sections -D__LINUX_USER__
C_CFLAGS	= $(PLATFORM_CFLAGS) $(COMPILE_OPTS) $(WARNING) $(EXTRA_INCLUDE)
LD_FLAGS	= -fPIC -shared
#--------- END OF ENVIRONMENT SETTING -------------
SRC = \

OBJ = $(SRC:.c=.o)

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
clean:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
else
all: $(OBJ)
	@echo Creating library $(MODULE_NAME) ...
	@$(CC) -o $(MODULE_NAME).so $(OBJ) $(LD_FLAGS)
	@$(AR) rcs $(MODULE_NAME).a $(OBJ)

%.o:%.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) $< -c -o $@

clean:
	@rm -f $(OBJ) *.o *.a *.so*
	@rm -rf $(OUTPUT_DIR)
endif

install:
	@mkdir -p $(OUTPUT_DIR)
	@cp -avf *.so* *.a $(OUTPUT_DIR)

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
include src/Filelists.mk
LWIPDIR = src

#--------- ENVIRONMENT SETTING --------------------
# -Wno-implicit-function-declaration is for original freertos's warning
C_CFLAGS = \
	$(PLATFORM_CFLAGS) \
	$(EXTRA_INCLUDE) \
	-Wno-unused-but-set-variable \
	-Wno-unused-variable \

C_AFLAGS = $(PLATFORM_AFLAGS)
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = lib$(MODULE_NAME).a

ASM = \

SRC = \
	$(LWIPNOAPPSFILES)

OBJ = $(ASM:.S=.o) $(SRC:.c=.o)

all: $(LIB_NAME)

ifneq ("$(wildcard */*/*.c)","")
$(LIB_NAME): $(OBJ)
	@echo Creating $@...
	@$(AR) rcsD $@ $(OBJ)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@

%.o: %.c
	@echo Compiling $(notdir $<)
	@$(CC) $(C_CFLAGS) -c $< -o $@

%.o: %.S
	@echo Assembling $(notdir $<)
	@$(CC) $(C_AFLAGS) -c $< -o $@

clean:
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so*
	@rm -rf $(OUTPUT_DIR)

install:
	@mkdir -p $(OUTPUT_DIR)
	@cp -avf $(LIB_NAME) $(OUTPUT_DIR)

else
$(LIB_NAME):
	@echo nothing to be done for $(LIB_NAME)

clean:
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so*

install:
	@echo nothing to be done for $(LIB_NAME)

endif
endif
