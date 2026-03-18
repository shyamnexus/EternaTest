###############################################################################
# Common-Head Makefile                                                        #
###############################################################################
ANSI_RED='\033[0;31m'
ANSI_YELLOW='\033[1;33m'
ANSI_RESET='\033[0m'

PWD := $(shell pwd)
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR=$(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR=$(NVT_HDAL_DIR)/drivers/k_flow
VENDOR_DIR=$(NVT_HDAL_DIR)/vendor
PANEL_DIR=$(NVT_HDAL_DIR)/ext_devices/panel

PWD_KDRV = hdal/drivers/k_driver
PWD_KFLOW = hdal/drivers/k_flow
PWD_VENDOR = hdal/vendor
PWD_EXTDEVICES = hdal/ext_devices
PWD_PANEL = hdal/ext_devices/panel
PWD_GYRO = hdal/ext_devices/gyro

KBUILD_EXTRA_SYMBOLS += $(shell find $(VOS_DRIVER_DIR) $(KDRV_DIR) -name Module.symvers)

ifeq ($(CURR_DRIVER_DIR),)
$(error CURR_DRIVER_DIR must be defined in Makefile.)
endif

ifeq ($(MODULE_NAME),)
$(error MODULE_NAME must be defined in Makefile.)
endif

ifeq ($(findstring $(PWD_KFLOW), $(CURR_DRIVER_DIR)), $(PWD_KFLOW))
KBUILD_EXTRA_SYMBOLS += $(shell find $(KFLOW_DIR) -name Module.symvers)
else ifeq ($(findstring $(PWD_VENDOR), $(CURR_DRIVER_DIR)), $(PWD_VENDOR))
KBUILD_EXTRA_SYMBOLS += $(shell find $(KFLOW_DIR) $(VENDOR_DIR) -name Module.symvers)
else ifeq ($(findstring $(PWD_EXTDEVICES), $(CURR_DRIVER_DIR)), $(PWD_EXTDEVICES))
KBUILD_EXTRA_SYMBOLS += $(shell find $(KFLOW_DIR) $(VENDOR_DIR) -name Module.symvers)
endif

ifeq ($(findstring $(PWD_PANEL), $(CURR_DRIVER_DIR)), $(PWD_PANEL))
KBUILD_EXTRA_SYMBOLS += $(wildcard $(NVT_HDAL_DIR)/ext_devices/panel/display_panel/Module.symvers)
endif

ifeq ($(findstring $(PWD_GYRO), $(CURR_DRIVER_DIR)), $(PWD_GYRO))
KBUILD_EXTRA_SYMBOLS += $(wildcard $(NVT_HDAL_DIR)/ext_devices/gyro/gyro_comm/Module.symvers)
endif

# global include
EXTRA_INCLUDE += \
	-I . \
	-I$(src)/include \
	-I$(NVT_HDAL_DIR)/include \
	-I$(VOS_DRIVER_DIR)/include \

OBJ += $(SRC:.c=.o) $(ASM:.S=.o) $(CPP_SRC:.cpp=.o)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
ifeq ($(KERNELRELEASE),)
# to solve make module_install unable to get CONFIG_FUNCTION_TRACER
-include $(KERNELDIR)/.config
endif
OBJ-TAG ?= m

C_PREDEFINEDS += \
	-D__LINUX \

WARNINGS += \
	-Werror \
	-Wno-date-time \

EXTRA_CFLAGS += \
	$(WARNINGS) \
	$(C_PREDEFINEDS) \
	$(EXTRA_INCLUDE)

obj-$(OBJ-TAG) += $(MODULE_NAME).o

ifeq ($(CONFIG_FUNCTION_TRACER),y)
$(MODULE_NAME)-objs = $(OBJ)
else
$(MODULE_NAME)-objs = $(OBJ:.o=_no_tracer.o)
EXTRA_CFLAGS += \
	$(shell \
		for n in $(OBJ:.o=); \
		do \
			if [ -f $(src)/$$n.c ]; then \
				ln -sf $$(basename $${n}.c) $(src)/$${n}_no_tracer.c; \
			else rm -f $(src)/$${n}_no_tracer.c; \
			fi \
		done \
	)
endif

MODPOST_OBJ = $(foreach n, $($(MODULE_NAME)-objs:.o=), $(if $(wildcard $(src)/$(n).c),, $(src)/$(n).o))
MODPOST_CMD = $(foreach n, $(MODPOST_OBJ), $(subst $(notdir $(n)),.$(notdir $(n)).cmd,$(n)))
EXTRA_CFLAGS += $(foreach n, $(MODPOST_CMD), $(shell touch $(n)))

# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then includes this
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.
ifeq ($(KERNELRELEASE),)
KERVER = $(shell cat ${KERNELDIR}/include/config/kernel.release)
KDIR = $(KERNELDIR)
MDIR = $(KERNELDIR)/_install_modules/lib/modules/$(KERVER)/hdal
ifeq ($(findstring $(PWD_EXTDEVICES), $(CURR_DRIVER_DIR)), $(PWD_EXTDEVICES))
# hdal/ext_devices/panel/display_panel/disp_if8b_lcd1_pw35p00_hx8238d -> display_panel/disp_if8b_lcd1_pw35p00_hx8238d
MODPATH ?= $(shell echo $(PWD) | awk -F'ext_devices/[^/]*/' '{print $$NF}';)
else
# hdal/drivers/k_driver/source/comm/usb2dev -> comm/usb2dev
MODPATH ?= $(shell echo $(PWD) | awk -F'source/' '{print $$NF}';)
endif
MODNAME = $(obj-m:.o=.ko)
# variables for clean object
RM_TRACER_OBJ = $(foreach n, $(OBJ:.o=), $(if $(wildcard $(n).c),$(if $(wildcard $(n).o),$(n).o,),))
RM_NO_TRACER_OBJ = $(foreach n, $(OBJ:.o=), $(if $(wildcard $(n).c),$(if $(wildcard $(n)_no_tracer.o),$(n)_no_tracer.o,),))
RM_TRACER_CMD =  $(foreach n, $(OBJ:.o=.o.cmd), $(join $(dir $(n)), $(addprefix .,$(notdir $(n)))))
RM_NO_TRACER_CMD =  $(foreach n, $(OBJ:.o=_no_tracer.o.cmd), $(join $(dir $(n)), $(addprefix .,$(notdir $(n)))))

CLEAN_ALL = \
 	$(RM_TRACER_OBJ) \
	$(RM_NO_TRACER_OBJ) \
	$(RM_TRACER_CMD) \
	$(RM_NO_TRACER_CMD) \
	$(OBJ:.o=_no_tracer.c) \
	$(MODULE_NAME).* \
	.$(MODULE_NAME).* \
	Module.symvers \
	.Module.symvers.cmd \
	modules.order \
	.modules.order.cmd \
	.tmp_versions \
	.built-in.a.cmd \
	built-in.a \

modules:
	@$(MAKE) -C $(KDIR) M=$(PWD) $(NVT_KGCOV) modules

# install to kernel folder
modules_install:
ifeq ($(wildcard $(MODNAME)), $(MODNAME))
ifeq ($(NVT_MOD_INSTALL),)
	@rm -f $(MDIR)/$(MODPATH)/$(MODNAME) && \
	install -m644 -b -D $(MODNAME) ${MDIR}/$(MODPATH)/$(MODNAME)
else
	@install -m644 -b -D $(MODNAME) $(NVT_MOD_INSTALL)/lib/modules/$(KERVER)/hdal/$(MODPATH)/$(MODNAME)
endif
endif
ifneq ($(PREBUILT_INSTALL), )
	@mkdir -p $(PREBUILT_DIR)
	@cp --parent -avf -t $(PREBUILT_DIR) $(PREBUILT_INSTALL)
endif

# install to rootfs folder
modules_install2:
ifneq ($(ROOTFS_DIR),)
	@echo install to $(ROOTFS_DIR)/rootfs/lib/modules/$(KERVER)/hdal/$(MODPATH)/$(MODNAME)
	@install -m644 -b -D $(MODNAME) $(ROOTFS_DIR)/rootfs/lib/modules/$(KERVER)/hdal/$(MODPATH)/$(MODNAME) && \
	depmod -b $(ROOTFS_DIR)/rootfs -a $(KERVER)
else
	@echo "ROOTFS_DIR is not defined"
endif

clean:
	@rm -rf $(CLEAN_ALL)

codesize:
	@echo $(MODULE_NAME)
	@$(OBJDUMP) -t $(MODULE_NAME).o > $(MODULE_NAME).sym && \
	$(BUILD_DIR)/nvt-tools/nvt-ld-op -j $(MODULE_NAME).sym

endif

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
OUTPUT_NAME = lib$(MODULE_NAME).a
OUTPUT_DIR = $(CURR_DRIVER_DIR)/output
ifeq ($(findstring x64,$(RTOS_CPU_TYPE)),x64)
ENV_LIBC=rtos64
else
ENV_LIBC=rtos
endif

C_PREDEFINEDS += \
	-DDEBUG \

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

RM_OBJ = $(foreach n, $(OBJ:.o=), $(if $(wildcard $(n).c),$(if $(wildcard $(n).o),$(n).o,),))
RM_OBJ += $(foreach n, $(OBJ:.o=), $(if $(wildcard $(n).S),$(if $(wildcard $(n).o),$(n).o,),))
RM_OBJ += $(foreach n, $(OBJ:.o=), $(if $(wildcard $(n).cpp),$(if $(wildcard $(n).o),$(n).o,),))

# compare 2 counts between expected .c + .o and real existed .c + .o
# also consider case of exited .o without .c
# if 2 counts is the same, the target can be built.
REQUIRE_SRC_CNT = $(words $(SRC) $(SRC_CPP) $(ASM))
EXIST_SRC_CNT = $(words $(filter-out $(RM_OBJ), $(wildcard $(SRC) $(SRC_CPP) $(ASM) $(OBJ))))

modules: $(OUTPUT_NAME)

%.o:%.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo Compiling $<
	@$(CXX) $(C_CXXFLAGS) -c $< -o $@

%.o: %.S
	@echo Assembling $<
	@$(CC) $(C_AFLAGS) -c $< -o $@


ifeq ($(REQUIRE_SRC_CNT), $(EXIST_SRC_CNT))
$(OUTPUT_NAME): $(OBJ)
	@echo Creating library $* ...
	@$(AR) rcsD $(OUTPUT_NAME) $(OBJ)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@
else
$(OUTPUT_NAME):
	@echo "nothing to be done for $(OUTPUT_NAME)"
endif

clean:
	@rm -f $(RM_OBJ)
ifeq ($(REQUIRE_SRC_CNT), $(EXIST_SRC_CNT))
	@rm -f $(OUTPUT_NAME)
else
	@echo ignore to remove $(OUTPUT_NAME)
endif


modules_install:
	@mkdir -p $(OUTPUT_DIR)
# target copy to output
ifeq ($(wildcard  $(OUTPUT_NAME)), )
	@echo "nothing to copy for '$(OUTPUT_NAME)'"
else
	@cp -avf -t $(OUTPUT_DIR) $(OUTPUT_NAME)
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
	@cp -avf -t $(PREBUILT_DIR)/prebuilt/$(ENV_LIBC) $(OUTPUT_NAME)
endif
endif

endif
###############################################################################
# Common-Tail Makefile                                                        #
###############################################################################
chk_cert:
ifeq ($(filter %_cert.c, $(SRC)), )
	@echo -e $(ANSI_RED)$(MODULE_NAME) has no cert.$(ANSI_RESET)
else
	@echo $(MODULE_NAME) has $(notdir $(filter %_cert.c, $(SRC)))
endif

.PHONY: modules modules_install modules_install2 clean publish chk_cert