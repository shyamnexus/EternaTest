###############################################################################
# Common-Head Makefile                                                        #
###############################################################################
OBJ += $(SRC:.c=.o) $(SRC_CPP:.cpp=.o) $(ASM:.S=.o)
# variables for clean object
RM_OBJ = $(foreach n, $(SRC:.c=), $(if $(wildcard $(n).c),$(if $(wildcard $(n).o),$(n).o,),))
RM_OBJ += $(foreach n, $(SRC_CPP:.cpp=), $(if $(wildcard $(n).cpp),$(if $(wildcard $(n).o),$(n).o,),))
RM_OBJ += $(foreach n, $(ASM:.S=), $(if $(wildcard $(n).cpp),$(if $(wildcard $(n).o),$(n).o,),))
WILDCARD_SRC = $(strip $(foreach n, $(SRC) $(SRC_CPP) $(ASM), $(if $(wildcard $(n)),$(n),)))
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
STATIC_LIB = $(MODULE_NAME).a
DYNAMIC_LIB = $(MODULE_NAME).so
TARGETS = $(STATIC_LIB) $(DYNAMIC_LIB)
ifeq ($(findstring uclibc, $(CROSS_COMPILE)), uclibc)
ENV_LIBC=uclibc
else
ENV_LIBC=glibc
endif

EXTRA_INCLUDE += \
	-I.

C_PREDEFINEDS += \
	-D__LINUX_USER__ \
	-D__LINUX \

WARNINGS += \
	-Wall \
	-Wundef \
	-Wsign-compare \
	-Wno-missing-braces \
	-Werror \

COMPILE_OPTS += \
	-O3 \
	-fPIC \
	-ffunction-sections \
	-fdata-sections \

C_CFLAGS = \
	$(NVT_ASAN) \
	$(NVT_GCOV) \
	$(PLATFORM_CFLAGS) \
	$(COMPILE_OPTS) \
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

LD_FLAGS = \
	$(NVT_ASAN) \
	$(NVT_GCOV) \
	-fPIC \
	-shared \
	$(EXTRA_LDFLAGS)

all: $(STATIC_LIB) $(DYNAMIC_LIB)

ifneq ($(WILDCARD_SRC),)
$(STATIC_LIB): $(OBJ)
	@echo Creating $@ ...
	@$(AR) rcs $(MODULE_NAME).a $(OBJ)

$(DYNAMIC_LIB): $(OBJ)
	@echo Creating $@ ...
	@$(CC) -o $(MODULE_NAME).so $(OBJ) $(LD_FLAGS)
	@$(NM) -n $@ > $@.sym
else
$(STATIC_LIB):
	@echo "nothing to be done for '$@'"

$(DYNAMIC_LIB):
	@echo "nothing to be done for '$@'"
endif

clean:
ifneq ($(WILDCARD_SRC),)
# case 1: mixed .c and .o or full source
	@rm -f $(TARGETS) $(RM_OBJ)
else
# case 2: only remain .a and .so
	@rm -f $(OBJ)
endif
	@rm -f $(DYNAMIC_LIB).sym
	@rm -f $(addprefix $(OUTPUT_DIR)/, $(TARGETS))

install:
	@mkdir -p $(OUTPUT_DIR)
# target copy to output
ifeq ($(wildcard  $(TARGETS)), )
	@echo "nothing to copy for '$(TARGETS)'"
else
	@cp -avf -t $(OUTPUT_DIR) $(TARGETS)
endif
# copy prebuilt lib to output
ifneq ($(wildcard prebuilt/$(ENV_LIBC)),)
	@cp -avf -t $(OUTPUT_DIR) prebuilt/$(ENV_LIBC)/*.a  prebuilt/$(ENV_LIBC)/*.so
endif
# release prebuilt lib
ifneq ($(PREBUILT_DIR), )
ifeq ($(wildcard $(SRC)),$(strip $(SRC)))
	@mkdir -p $(PREBUILT_DIR)/$(ENV_LIBC)
	@cp -avf -t $(PREBUILT_DIR)/$(ENV_LIBC) $(TARGETS)
endif
endif

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
STATIC_LIB = $(MODULE_NAME).a
TARGETS = $(STATIC_LIB)
ifeq ($(findstring x64,$(RTOS_CPU_TYPE)),x64)
ENV_LIBC=rtos64
else
ENV_LIBC=rtos
endif

C_PREDEFINEDS += \

WARNINGS += \
	-Wno-format \

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

all: $(STATIC_LIB)

ifneq ($(WILDCARD_SRC),)
$(STATIC_LIB): $(OBJ)
	@echo Creating $@...
	@$(AR) rcsD $@ $(OBJ)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@
else
$(STATIC_LIB):
	@echo "nothing to be done for '$@'"
endif

clean:
ifneq ($(WILDCARD_SRC),)
# case 1: mixed .c and .o or full source
	@rm -f $(TARGETS) $(RM_OBJ)
else
# case 2: only remain .a and .so
	@rm -f $(OBJ)
endif
	@rm -f $(RM_OBJ)
	@rm -f $(addprefix $(OUTPUT_DIR)/, $(TARGETS))

install:
	@mkdir -p $(OUTPUT_DIR)
# target copy to output
ifeq ($(wildcard  $(TARGETS)), )
	@echo "nothing to copy for '$(TARGETS)'"
else
	@cp -avf -t $(OUTPUT_DIR) $(TARGETS)
endif
# copy extra files to output
ifneq ($(EXTRA_INSTALL),)
	@cp -avf -t $(OUTPUT_DIR) $(EXTRA_INSTALL)
endif
# copy prebuilt lib to output
ifneq ($(wildcard prebuilt/$(ENV_LIBC)),)
	@cp -avf -t $(OUTPUT_DIR) prebuilt/$(ENV_LIBC)/*.a
endif
# release prebuilt lib
ifneq ($(PREBUILT_DIR), )
ifeq ($(wildcard $(SRC)),$(strip $(SRC)))
	@mkdir -p $(PREBUILT_DIR)/$(ENV_LIBC)
	@cp -avf -t $(PREBUILT_DIR)/$(ENV_LIBC) $(TARGETS)
endif
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
