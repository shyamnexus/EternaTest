###############################################################################
# Common-Head Makefile                                                        #
###############################################################################
OBJ += $(SRC:.c=.o) $(ASM:.S=.o) $(CPP_SRC:.cpp=.o)
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
TARGETS += $(MODULE_NAME)

EXTRA_INCLUDE += \
	-I. \

WARNINGS += \
	-Wall \
	-Wundef \
	-Wsign-compare \
	-Wno-missing-braces \
	-Wstrict-prototypes \
	-Werror \
	-Wno-sign-compare \

COMPILE_OPTS += \
	 -O3 \
	 -fPIC \
	 -ffunction-sections \
	 -fdata-sections \
	 -D__LINUX_USER__ \
	 -D__LINUX \

C_CFLAGS = \
	$(NVT_ASAN) \
	$(NVT_GCOV) \
	$(PLATFORM_CFLAGS) \
	$(COMPILE_OPTS) \
	$(WARNINGS) \
	$(C_PREDEFINEDS) \
	$(EXTRA_INCLUDE) \
	$(EXTRA_CFLAGS) \

LD_FLAGS = \
	$(NVT_GCOV) \
	$(NVT_ASAN) \
	$(EXTRA_LDFLAGS) \

all: $(TARGETS)

$(MODULE_NAME): $(OBJ)

$(TARGETS):
ifeq ($(strip $(OBJ)), )
	@echo "nothing to be done for '$@'"
else
	@echo Creating $@...
	@$(CC) -o $@ $(OBJ) $(LD_FLAGS)
	@$(NM) -n $@ > $@.sym
ifneq ($(NVT_BINARY_FILE_STRIP), no)
	@$(STRIP) $@
	@$(OBJCOPY) -R .comment -R .note.ABI-tag -R .gnu.version $@
endif
endif


clean:
	@rm -f $(OBJ) $(TARGETS) $(MODULE_NAME).sym
	@rm -f $(addprefix $(OUTPUT_DIR)/, $(TARGETS))

install: $(TARGETS)
	@mkdir -p $(OUTPUT_DIR)
ifeq ($(wildcard  $(TARGETS)), )
	@echo "nothing to copy for '$(TARGETS)'"
else
	@cp -avf -t $(OUTPUT_DIR) $(TARGETS)
	@cp -avf -t $(ROOTFS_DIR)/rootfs/usr/bin $(TARGETS)
endif

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
STATIC_LIB = lib$(MODULE_NAME).a
TARGETS += $(STATIC_LIB)

C_PREDEFINEDS += \

WARNINGS += \
	-Wno-format

C_CFLAGS = \
	$(PLATFORM_CFLAGS) \
	$(WARNINGS) \
	$(C_PREDEFINEDS) \
	$(EXTRA_INCLUDE) \
	$(EXTRA_CFLAGS) \

C_CXXFLAGS = \
	$(PLATFORM_CXXFLAGS) \
	$(WARNINGS) \
	$(C_PREDEFINEDS) \
	$(EXTRA_INCLUDE) \
	$(EXTRA_CPPFLAGS) \

C_AFLAGS = \
	$(PLATFORM_AFLAGS) \
	$(WARNINGS) \
	$(C_PREDEFINEDS) \
	$(EXTRA_INCLUDE) \
	$(EXTRA_AFLAGS) \

all: $(TARGETS)

$(STATIC_LIB): $(OBJ)

$(TARGETS):
ifeq ($(strip $(OBJ)), )
	@echo "nothing to be done for '$@'"
else
	@echo Creating $@...
	@$(AR) rcsD $@ $(OBJ)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@
endif

clean:
	@rm -f $(OBJ) $(TARGETS)
	@rm -f $(addprefix $(OUTPUT_DIR)/, $(TARGETS))

install: $(TARGETS)
	@mkdir -p $(OUTPUT_DIR)
ifeq ($(wildcard  $(TARGETS)), )
	@echo "nothing to copy for '$(TARGETS)'"
else
	@cp -avf -t $(OUTPUT_DIR) $(TARGETS)
endif

endif
###############################################################################
# Common-Tail Makefile                                                        #
###############################################################################
%.o:%.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo Compiling $<
	@$(CXX) $(C_CXXFLAGS) -c $< -o $@

%.o: %.S
	@echo Assembling $<
	@$(CC) $(C_AFLAGS) -c $< -o $@

.PHONY: all clean install