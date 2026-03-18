current_mk_abspathname := $(abspath $(lastword $(MAKEFILE_LIST)))
current_mk_absdir := $(dir $(current_mk_abspathname))../../../../ai_sdk_env.mk
ifneq ("$(wildcard $(current_mk_absdir))", "")
include $(current_mk_absdir)
endif

# include
include $(NVT_PRJCFG_MODEL_CFG)

VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow_isp
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
NVT_KDRV_PATH = $(NVT_HDAL_DIR)/drivers/k_driver
#NVT_KFLOW_PATH = $(NVT_HDAL_DIR)/drivers/k_flow/$(NVT_SDK_TYPE)
NVT_KFLOW_PATH = $(NVT_HDAL_DIR)/drivers/k_flow/ipc

ifeq ("$(wildcard $(OWN_NVT_AI3_PATH))", "")
NVT_AI_KDRV_PATH = $(NVT_HDAL_DIR)/vendor/ai3/drivers/k_driver
NVT_AI_KFLOW_PATH = $(NVT_HDAL_DIR)/vendor/ai3/drivers/k_flow
else
NVT_AI_KFLOW_PATH=$(NVT_AI3_KFLOW_PATH)
CURR_DRIVER_INC_PATH=$(NVT_AI3_KFLOW_ISP_PATH)/include
endif

VG_MODULE_INC = $(NVT_KDRV_PATH)/source/include
VOS_MODULE_INC=$(NVT_VOS_DIR)/drivers/include

ifeq ($(SDK_CODENAME), ns02201)
EXTRA_CFLAGS += $(subst xvr, -D_NVT_NVR_SDK_, $(subst ipc, -D_NVT_IPC_SDK_,$(NVT_SDK_TYPE)))
endif

EXTRA_CFLAGS += -I$(VOS_MODULE_INC)
EXTRA_CFLAGS += -I$(NVT_HDAL_DIR)/include
EXTRA_CFLAGS += -I$(NVT_KDRV_PATH)/include -I$(NVT_KFLOW_PATH)/include -I$(VG_MODULE_INC) -I$(VG_MODULE_INC)/common -I$(VG_MODULE_INC)/util
EXTRA_CFLAGS += -I$(NVT_KFLOW_PATH)/include/kflow_common -I$(NVT_KFLOW_PATH)/source/include
EXTRA_CFLAGS += -I$(NVT_AI_KDRV_PATH)/include -I$(NVT_AI_KFLOW_PATH)/include -I$(NVT_KDRV_PATH)/source/kdrv_builtin/ai -I$(CURR_DRIVER_INC_PATH)

ifeq ($(SDK_CODENAME), ns02401)
EXTRA_CFLAGS += -Wno-enum-compare -Wno-date-time -D__LINUX -D__KERNEL__
else
EXTRA_CFLAGS += -Werror -Wno-enum-compare -Wno-date-time -D__LINUX -D__KERNEL__
endif


ifeq ($(NVT_VP_BUILD), Y)
EXTRA_CFLAGS += -D KFLOW_AI_VP_ENV
endif

MODULE_NAME = kflow_ai_isp
OBJ := \
	kflow_isp/kflow_ai_isp_ioctl.o \
	kflow_isp/kflow_ai_isp_platform.o \
	kflow_isp/kflow_ai_isp_net_flow.o  \
	kflow_isp/kflow_ai_isp.o \
	kflow_isp/kflow_ai_isp_sw_mode.o \
	kflow_isp/kflow_ai_isp_change_res.o \
	kflow_isp/kflow_ai_isp_build_instrs_sync_sdktool.o \
	kflow_isp/kflow_ai_isp_tsk.o \
	kflow_isp/kflow_ai_isp_proc.o
	
	
obj-m += $(MODULE_NAME).o
obj-m := $(sort $(obj-m))

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then includes this
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.
ifeq ($(CONFIG_FUNCTION_TRACER),y)
$(MODULE_NAME)-objs := $(OBJ)
else
$(MODULE_NAME)-objs := $(OBJ:.o=_no_tracer.o)
EXTRA_CFLAGS += $(shell for n in $(OBJ:.o=); do if [ -f $(src)/$$n.c ]; then ln -sf $$(basename $${n}.c) $(src)/$${n}_no_tracer.c; else rm -f $(src)/$${n}_no_tracer.c; fi done)
endif

MODPOST_OBJ = $(foreach n, $($(MODULE_NAME)-objs:.o=), $(if $(wildcard $(src)/$(n).c),, $(src)/$(n).o))
MODPOST_CMD = $(foreach n, $(MODPOST_OBJ), $(subst $(notdir $(n)),.$(notdir $(n)).cmd,$(n)))
EXTRA_CFLAGS += $(foreach n, $(MODPOST_CMD), $(shell touch $(n)))

ifeq ($(KERNELRELEASE),)
PWD := $(shell pwd)
KERVER ?= $(NVT_LINUX_VER)
KDIR ?= $(KERNELDIR)
MDIR ?= $(KERNELDIR)/_install_modules/lib/modules/$(KERVER)/hdal
#MODPATH := $(shell echo $(PWD) | awk -F'source/' '{print $$NF}')
MODPATH :=	kflow_ai_isp
MODNAME := $(shell echo $(obj-m:.o=.ko))
# variables for clean object
RM_TRACER_OBJ = $(foreach n, $(OBJ:.o=), $(if $(wildcard $(n).c),$(if $(wildcard $(n).o),$(n).o,),))
RM_NO_TRACER_OBJ = $(foreach n, $(OBJ:.o=), $(if $(wildcard $(n).c),$(if $(wildcard $(n)_no_tracer.o),$(n)_no_tracer.o,),))
export KBUILD_EXTRA_SYMBOLS += $(shell find $(VOS_DRIVER_DIR) -name Module.symvers)
export KBUILD_EXTRA_SYMBOLS += $(shell find $(NVT_KDRV_PATH) -name Module.symvers)
export KBUILD_EXTRA_SYMBOLS += $(shell find $(NVT_AI_KDRV_PATH) -name Module.symvers)
export KBUILD_EXTRA_SYMBOLS += $(shell find $(NVT_AI_KFLOW_PATH) -name Module.symvers)

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) $(NVT_KGCOV) modules

modules_install:
	@rm -f $(MDIR)/$(MODPATH)/$(MODNAME); \
	install -m644 -b -D $(MODNAME) ${MDIR}/$(MODPATH)/$(MODNAME); \
	cd $(KDIR)/_install_modules/lib/modules/$(KERVER)/; depmod -b $(KDIR)/_install_modules/ -a $(KERVER)
	@if [ -z $(NVT_MOD_INSTALL) ]; then \
		rm -f $(MDIR)/$(MODPATH)/$(MODNAME); \
		install -m644 -b -D $(MODNAME) ${MDIR}/$(MODPATH)/$(MODNAME); \
		cd $(KDIR)/_install_modules/lib/modules/$(KERVER)/; depmod -b $(KDIR)/_install_modules/ -a $(KERVER); \
	else \
		mkdir -p $(NVT_MOD_INSTALL)/lib/modules/$(KERVER); \
		install -m644 -b -D $(MODNAME) $(NVT_MOD_INSTALL)/lib/modules/$(KERVER)/hdal/$(MODPATH)/$(MODNAME); \
	fi
	
	
clean:
	@rm -rf $(MODULE_NAME).* .$(MODULE_NAME).* $(OBJ:.o=_no_tracer.c) Module.symvers modules.order .tmp_versions
	@rm -f `find . -type f -name ".*.cmd" -o -name ".*.d"`
	@rm -f $(RM_TRACER_OBJ) $(RM_NO_TRACER_OBJ)
codesize:
	@echo $(MODULE_NAME)
	@$(OBJDUMP) -t $(MODULE_NAME).o > $(MODULE_NAME).sym && \
	$(BUILD_DIR)/nvt-tools/nvt-ld-op -j $(MODULE_NAME).sym
.PHONY: modules modules_install clean

endif
###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
OUTPUT_NAME = lib$(MODULE_NAME).a
modules:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"

modules_install:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"

clean:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
endif
