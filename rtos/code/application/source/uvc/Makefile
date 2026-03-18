MODULE_NAME = rtos-main

include $(NVT_PRJCFG_MODEL_CFG)
MODEL = $(shell echo $(NVT_PRJCFG_MODEL_CFG) | sed 's/.*\/configs\/rtos\/cfg_\([^\/]*\)\/ModelConfig.mk/\1/g')

.PHONY: all clean

# remove comment to force combo_flash support
#EMBMEM = EMBMEM_COMBO
#EMBMEM_BLK_SIZE = 0x20000

# remove comment to force partial load disabled
#FW_TYPE=FW_TYPE_COMPRESS

#######################################################################################
#--------- PACKAGE SELECTION ---------------------------------------------------------#
#######################################################################################
# 0: disable, 1: enable
PACKAGE_VIDEO    ?= 1
PACKAGE_AUDIO    ?= 0
PACKAGE_FILESYS  ?= 1
PACKAGE_SDCARD   ?= 1
PACKAGE_DISPLAY  ?= 0
PACKAGE_SAMPLES  ?= 1
PACKAGE_CMDSYS   ?= 1


#--------- PACKAGE SELECTION ---------------------------------------------------------

#######################################################################################
#--------- DO NOT EDIT ---------------------------------------------------------------#
#######################################################################################
BIN_INFO_OFS = 0x200
CODE_INFO_OFS = 0x3F0

# rtos-main.c always be compilied at last to update build date
MAIN_C = ./rtos-main.c

# caculation file offset of ecah field on bin_info and code_info
define ADD_HEX
$(shell printf "0x%X" $(shell expr $(shell printf "%d" $1) + $(shell printf "%d" $2)))
endef
BIN_INFO = $(call ADD_HEX, $(BOARD_RTOS_ADDR), $(BIN_INFO_OFS))
BIN_INFO_CHIPNAME = $(call ADD_HEX, $(BIN_INFO_OFS), 0x50)
BIN_INFO_LENGHT = $(call ADD_HEX, $(BIN_INFO_OFS), 0x68)
BIN_INFO_CHKSUM = $(call ADD_HEX, $(BIN_INFO_OFS), 0x6E)
BIN_INFO_BINCTRL = $(call ADD_HEX, $(BIN_INFO_OFS), 0x78)
CODE_INFO = $(call ADD_HEX, $(BOARD_RTOS_ADDR), $(CODE_INFO_OFS))
CODE_INFO_SECTION01_SIZE = $(call ADD_HEX, $(CODE_INFO_OFS), 0x1C)

# replace string on lds
LDS_REPLACE_DEFAULT  = \
	-e's/$$START_UP/$(BOARD_RTOS_ADDR)/g' \
	-e's/$$CODE_INFO/$(CODE_INFO)/g' \
	-e's/$$BIN_INFO/$(BIN_INFO)/g' \
	-e's/$$LDS_EXTERN/$(LDS_EXTERN)/g' \
	-e's/$$SENSOR1/libnvt_$(SENSOR1)/g' \
	-e's/$$SENSOR2/libnvt_$(SENSOR2)/g' \
	-e's/$$LCD1/lib$(LCD1)/g' \
	-e's/$$LCD2/lib$(LCD2)/g' \

#--------- END OF DO NOT EDIT ---------------------------------------------------------

#######################################################################################
#--------- ENVIRONMENT SETTING -------------------------------------------------------#
#######################################################################################
# DIRs
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
NVT_TOOLS_DIR = $(BUILD_DIR)/nvt-tools
RTOS_KERNEL_DIR = $(KERNELDIR)/lib/FreeRTOS
RTOS_LIB_DIR = $(KERNELDIR)/lib
RTOS_CURR_DEMO_DIR = $(KERNELDIR)/demos/novatek/$(SDK_CODENAME)
RTOS_POSIX_DIR = $(KERNELDIR)/lib/FreeRTOS-Plus-POSIX
RTOS_POSIX_SRC_DIR = $(RTOS_POSIX_DIR)/source
RTOS_LWIP_DIR = $(KERNELDIR)/lib/third_party/lwip/src
RTOS_LWIP_SRC_DIR = $(RTOS_LWIP_DIR)
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
EXT_DIR = $(NVT_HDAL_DIR)/ext_devices
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
OUTPUT_DIR = ./output
INSTALL_DIR = ../../output

# LIBs
GCC_LIB_PATH     = $(dir $(shell $(CC) $(PLATFORM_CFLAGS) -print-libgcc-file-name))
STDC_LIB_PATH    = $(dir $(shell $(CC) $(PLATFORM_CFLAGS) -print-file-name=libstdc++.a))
LIBC_LIB_PATH    = $(dir $(shell $(CC) $(PLATFORM_CFLAGS) -print-file-name=libc.a))

# LIB_DEPENDENCY to collect magic symbol and check if needing link again and
LIB_DEPENDENCY = \
	$(wildcard $(RTOS_CURR_DEMO_DIR)/output/*.a) \
	$(wildcard $(NVT_HDAL_DIR)/output/*.a) \
	$(wildcard $(NVT_HDAL_DIR)/vendor/output/*.a) \
	$(wildcard $(NVT_HDAL_DIR)/vendor/isp/drivers/output/*.a) \
	$(wildcard $(NVT_HDAL_DIR)/vendor/media/drivers/output/*.a) \
	$(wildcard $(HDAL_SAMPLE_DIR)/output/*.a) \
	$(wildcard $(LIBRARY_DIR)/output/*.a) \
	$(wildcard $(VOS_DRIVER_DIR)/output/*.a) \
	$(wildcard $(NVT_DRIVER_DIR)/output/*.a) \
	$(wildcard $(KDRV_DIR)/output/*.a) \
	$(wildcard $(EXT_DIR)/panel/output/*.a) \
	$(wildcard $(EXT_DIR)/sensor/output/*.a) \
	$(wildcard $(KFLOW_DIR)/output/*.a) \
	$(wildcard $(EXT_DIR)/audio/output/*.a) \

#LIB DIRs for C_LDFLAGS
EXTRA_LIB_DIR += \
	-L$(RTOS_CURR_DEMO_DIR)/output \
	-L$(NVT_HDAL_DIR)/output \
	-L$(NVT_HDAL_DIR)/vendor/output \
	-L$(NVT_HDAL_DIR)/vendor/media/drivers/output/ \
	-L$(NVT_HDAL_DIR)/vendor/isp/drivers/output/ \
	-L$(KDRV_DIR)/output \
	-L$(EXT_DIR)/panel/output \
	-L$(EXT_DIR)/sensor/output \
	-L$(EXT_DIR)/audio/output \
	-L$(KFLOW_DIR)/output \
	-L$(HDAL_SAMPLE_DIR)/output \
	-L$(LIBRARY_DIR)/output \
	-L$(VOS_DRIVER_DIR)/output \
	-L$(NVT_VOS_DIR)/output \
	-L$(NVT_DRIVER_DIR)/output \
	-L$(GCC_LIB_PATH) \
	-L$(STDC_LIB_PATH) \
	-L$(LIBC_LIB_PATH)

# FLAGs for Compiler, Assembler
BUILD_DATE = 20`date +'%y%m%d'`
BUILD_DATE_SEC = `date +'%s'`
C_PREDEFINED = \
	-DDEBUG \
	-D_MODEL_$(MODEL)_ \
	-D_BIN_NAME_='"$(BIN_NAME)"' \
	-D_BIN_NAME_T_='"$(BIN_NAME_T)"' \
	-D_BOARD_FDT_ADDR_=$(BOARD_FDT_ADDR) \
	-D_BOARD_SHMEM_ADDR_=$(BOARD_SHMEM_ADDR) \
	-D_BOARD_RTOS_ADDR_=$(BOARD_RTOS_ADDR) \
	-D_$(EMBMEM)_ \
	-D_EMBMEM_BLK_SIZE_=$(EMBMEM_BLK_SIZE) \
	-D_$(FW_TYPE)_ \
	-D_$(LCD1)_ \
	-D_$(SENSOR1)_ \
	-D_$(SENSOR2)_ \
	-D_SEN_SUFFIX_=$(SENSOR_CFG_SUFFIX) \
	-D_SEN1_="$(subst sen_,,$(SENSOR1))" \
	-D_SEN2_="$(subst sen_,,$(SENSOR2))" \
	-D_$(NVT_ROOTFS_TYPE)_ \
	-D_PACKAGE_VIDEO_=$(PACKAGE_VIDEO) \
	-D_PACKAGE_AUDIO_=$(PACKAGE_AUDIO) \
	-D_PACKAGE_FILESYS_=$(PACKAGE_FILESYS) \
	-D_PACKAGE_SDCARD_=$(PACKAGE_SDCARD) \
	-D_PACKAGE_DISPLAY_=$(PACKAGE_DISPLAY) \
	-D_$(NVT_LINUX_COMPRESS)_ \
	-D_$(NVT_LINUX_SMP)_ \
	-D_$(BOOT_OS)_ \
	-D_$(NVT_DUAL_RTOS)_\
	-D_BUILD_DATE_=$(BUILD_DATE) \
	-D_BUILD_DATE_SEC_=$(BUILD_DATE_SEC) \
	-D_BIN_INFO_OFS_=$(BIN_INFO_OFS) \

C_CFLAGS = $(PLATFORM_CFLAGS) $(EXTRA_INCLUDE) $(C_PREDEFINED) -Wno-format
C_CXXFLAGS = $(PLATFORM_CXXFLAGS) $(EXTRA_INCLUDE) $(C_PREDEFINED)
C_AFLAGS = $(PLATFORM_AFLAGS) $(EXTRA_INCLUDE)
C_LDFLAGS = \
	-Bstatic \
	-EL \
	--no-wchar-size-warning \
	--gc-sections \
	-T $(OUTPUT_DIR)/$(LDSCRIPT) \
	-Map $(MAP_NAME) \
	--start-group \
	$(EXTRA_LIB_DIR) \
	$(EXTRA_LIB) \
	--end-group

# FILEs
LDSCRIPT = rtos-main.lds
LDS_EXTERN = extern.lds
OUTPUT_NAME = $(OUTPUT_DIR)/$(MODULE_NAME).bin
IMG_NAME = $(OUTPUT_DIR)/$(MODULE_NAME).img
MAP_NAME = $(OUTPUT_DIR)/$(MODULE_NAME).map
SYM_NAME = $(OUTPUT_DIR)/$(MODULE_NAME).sym
DASM_NAME = $(OUTPUT_DIR)/$(MODULE_NAME).asm

# LDS
LDS_REPLACE = \
	$(LDS_REPLACE_DEFAULT) \

#--------- END OF ENVIRONMENT SETTING -------------------------------------------------

#######################################################################################
#--------- INCs FOR C_CFLAGS ---------------------------------------------------------#
#######################################################################################
# public includes
EXTRA_INCLUDE += \
	-I$(RTOS_LIB_DIR) \
	-I$(RTOS_KERNEL_DIR)/portable/GCC/ARM_CA9  \
	-I$(RTOS_KERNEL_DIR)/include \
	-I$(RTOS_KERNEL_DIR)/include/private \
	-I$(RTOS_CURR_DEMO_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include/portable/novatek \
	-I$(RTOS_LWIP_DIR)/include \
	-I$(RTOS_LWIP_SRC_DIR)/portable \
	-I$(RTOS_LWIP_SRC_DIR)/portable/arch \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(NVT_VOS_DIR)/include \
	-I$(NVT_HDAL_DIR)/include \
	-I$(NVT_HDAL_DIR)/vendor/isp/include \
	-I$(NVT_HDAL_DIR)/vendor/isp/drivers/include \
	-I$(KDRV_DIR)/include/plat \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/include \
	-I$(EXT_DIR)/panel/display_panel/include \
	-I$(EXT_DIR)/sensor/sen_common \
	-I$(EXT_DIR)/audio/aud_common/include \
	-I$(KFLOW_DIR)/include \
	-I$(NVT_DRIVER_DIR)/include \
	-I$(LIBRARY_DIR)/include \
	-I$(APP_DIR)/include \
	-I$(KDRV_DIR)/source/kdrv_gfx2d/kdrv_affine/include \
	-I$(KDRV_DIR)/source/include \
	-I$(NVT_HDAL_DIR)/vendor/media/include \

# application local includes
EXTRA_INCLUDE += \
	-I. \
	-I./system \
	-I./configs \
	-I./flow \
	-I./flow/CFU/verify/include \

#--------- END OF INCs FOR C_CFLAGS ---------------------------------------------------

#######################################################################################
#--------- LINKING LIBs FOR C_LDFLAGS ------------------------------------------------#
#######################################################################################

# kernel, vos, libc, others necessary
EXTRA_LIB = \
	-lfreertos \
	-ldrv_portable \
	-lc \
	-lm \
	-lgcc \
	-lstdc++ \
	-lkwrap \
	-lvos \
	-ldrv_drtc \
	-ldrv_rtc \
	-ldrv_serial \
	-ldrv_misc \
	-lker_pkg \
	-luvac \
	-lprebuilt_isp \

# kdrv
EXTRA_LIB += \
	-lkdrv_interrupt \
	-lkdrv_pll \
	-lkdrv_top \
	-lkdrv_usb2dev \
	-lkdrv_usb3dev \
	-lkdrv_comm \
	-lkdrv_gfx2d \
	-lkdrv_i2c \
	-lkdrv_gpio \
	-lkdrv_cpu \
	-lkdrv_sdp \
	-lnvt_uvcp \
	-lkdrv_vpe \
	-lnvt_sif \
	-lkdrv_jpg \
	-lkdrv_h26x \
	-lkdrv_rtosfdt \
	-lkdrv_adc \
	-lnvt_spi \
	-lkdrv_wdt \
	-lkdrv_videocapture \
	-lkdrv_hwcopy \


ifeq ($(PACKAGE_VIDEO),1)
EXTRA_LIB += -lkdrv_videoprocess -lnvt_tse -lkdrv_builtin
endif
ifeq ($(PACKAGE_AUDIO),1)
EXTRA_LIB += -lnvt_audio -lnvt_kdrv_audio -laud_aec -laud_aacd -laud_aace -laudlib_IIR
endif
ifeq ($(PACKAGE_DISPLAY),1)
EXTRA_LIB += -lkdrv_videoout -lnvt_ide
endif

# kflow
ifeq ($(PACKAGE_VIDEO),1)
EXTRA_LIB += \
	-lkflow_videoenc \
	-lkflow_videodec \
	-lkflow_common \
	-lkflow_videocapture \
	-lkflow_videoprocess \
	-lnvt_gfx \
	-lnvt_videosprite \

endif

ifeq ($(PACKAGE_AUDIO),1)
EXTRA_LIB += -lkflow_audiocap -lkflow_audioenc -lkflow_audiodec -lkflow_audioout
endif
ifeq ($(PACKAGE_DISPLAY),1)
EXTRA_LIB += -lkflow_videoout
endif

ifeq ($(PACKAGE_VIDEO),1)
# PQ
EXTRA_LIB += \
	-lvendor_isp \
	-lnvt_isp \
	-lnvt_ae \
	-lnvt_awb \
	-lnvt_iq \

endif

# ext_devices
EXTRA_LIB += \
	-lnvt_$(SENSOR1) \
	-lnvt_$(SENSOR2) \

# for fastboot with lcd, their always linked
# for normal rtos, if PACKAGE_AUDIO = 0, lcd driver will be optimized.
EXTRA_LIB += -lnvt_dispdev_panel -l$(LCD1)

ifeq ($(PACKAGE_AUDIO),1)
EXTRA_LIB += -lnvt_aud_emu -lnvt_aud_ac108
endif

ifeq ($(PACKAGE_VIDEO),1)
# hdal, vendor
EXTRA_LIB += \
	-lhdal \
	-lvendor_media \
	-lnvt_vencrc \

endif

# code/lib
EXTRA_LIB += \
	-lfwsrv \
	-lutility \
	-lzlib \
	-lping \
	-ldhcpd \
	-lvf_gfx \
	-lDbgUt \
	-lcmdmap \
	-lPStore \

ifeq ($(PACKAGE_FILESYS),1)
EXTRA_LIB += -lFileSys -lFsUitron
else
EXTRA_LIB += -lFileSysNull
endif

# code/driver
EXTRA_LIB += \
	-ldrv_devusb_msdc \
	-ldrv_storage \
	-lmsdcnvt \


# DO NOT DEFAULT TO LINK TEST_CASES LIBs


#--------- END of LINKING LIBs FOR C_LDFLAGS ------------------------------------------

#######################################################################################
#--------- SOURCEs FOR APPLICATION ---------------------------------------------------#
#######################################################################################
ASM = \
	./startup/code_info.S \
	./startup/smp_entry.S \
	./startup/clock_api.S \

# system
SRC = \
	./startup/bin_info.c \
	./startup/startup.c \
	./system/sys_fdt.c \
	./startup/power_down_isr.c \
	./system/sys_filesys.c \
	./system/sys_card.c \
	./system/sys_nvtmpp.c \
	./system/sys_mempool.c \
	./system/sys_fwload.c \
	./system/sys_fastboot.c \
	./system/sys_storage_partition.c \
	./system/sys_pstore.c \

CPP_SRC = \

# project dependency
SRC += \
	./flow/flow_uvc.c \
	./flow/uvc_slice.c \
	./flow/flow_updfw.c \
	./flow/flow_hid.c \
	./flow/flow_ct_pu.c \
	./flow/flow_eu.c \
	./flow/flow_uvc_info.c \
	./uiapp/msdcnvt/MsdcNvtCb.c \
	./uiapp/msdcnvt/MsdcNvtCbCom.c \
	./uiapp/msdcnvt/MsdcNvtCb_Adj.c \
	./uiapp/msdcnvt/MsdcNvtCb_CustomSi.c \
	./uiapp/msdcnvt/MsdcNvtCb_ISP.c \


DTS_SRC = \
	./dts/application.dts
#--------- END OF SOURCEs FOR APPLICATION ---------------------------------------------

#######################################################################################
#--------- DEVICE TREE CONFIGURATION -------------------------------------------------#
#######################################################################################
DTS_SENSOR = ./dts/sensor.dts
DTS_APP = ./dts/application.dts
DTB_SENSOR = $(DTS_SENSOR:.dts=.dtb)
DTB_APP = $(DTS_APP:.dts=.dtb)
DTB = $(DTB_SENSOR) $(DTB_APP)

SENSOR_CFG_DIR = $(NVT_HDAL_DIR)/ext_devices/sensor/configs
AD_CFG_DIR = $(NVT_HDAL_DIR)/ext_devices/ad/configs
MOTOR_CFG_DIR = $(NVT_HDAL_DIR)/ext_devices/motor/configs
ISP_CFG_DIR = $(NVT_HDAL_DIR)/vendor/isp/configs

DTB_INCLUDE = \
	-I$(SENSOR_CFG_DIR)/dtsi \
	-I$(AD_CFG_DIR)/dtsi \
	-I$(MOTOR_CFG_DIR)/dtsi \
	-I$(ISP_CFG_DIR)/dtsi \
	-I$(KERNELDIR)/include \
	-I$(KERNELDIR)/arch/arm/boot/dts/include  \
	-I$(CONFIG_DIR)/include \
	-I$(LINUX_BUILD_TOP)/base/linux-BSP/linux-kernel/include

#--------- END OF DEVICE TREE CONFIGURATION -------------------------------------------

#######################################################################################
#--------- COMPILING AND LINKING -----------------------------------------------------#
#######################################################################################
OBJ = $(SRC:.c=.o) $(ASM:.S=.o) $(CPP_SRC:.cpp=.o) $(DTS_SENSOR:.dts=.o)

all: $(DTB) $(OUTPUT_NAME)

$(DTB): $(wildcard $(dir $(DTS_SRC))*.dtsi)

$(LDS_EXTERN): $(OBJ) $(LIB_DEPENDENCY) $(LDSCRIPT) $(MAIN_C)
	@echo generate $@ ... && \
	echo "EXTERN(" > $@  && \
	$(OBJDUMP) -h $(LIB_DEPENDENCY) | grep "\.version\.info" | sed 's/[^.]*\.version\.info\.[^\.]*\.\([^\ ]*\).*/\1_LIBRARY_VERSION_INFO/g' >> $@
ifeq ($(PACKAGE_CMDSYS),1)
	@$(OBJDUMP) -h $(LIB_DEPENDENCY) | grep "\.cmdsys\.table" | sed 's/[^.]*\.cmdsys\.table\.\([^\ ]*\).*/\1_cmdsys_main/g' >> $@
endif
ifeq ($(PACKAGE_SAMPLES),1)
	@$(OBJDUMP) -h $(LIB_DEPENDENCY) | grep "\.examsys\.table" | sed 's/[^.]*\.examsys\.table\.\([^\ ]*\).*/\1_examsys_main/g' >> $@
endif
	@echo ")" >> $@

$(IMG_NAME): $(LDS_EXTERN)
	@echo Compiling $(MAIN_C) && \
	$(CC) $(C_CFLAGS) -c $(MAIN_C) -o $(MAIN_C:.c=.o) && \
	echo Creating $@... && \
	mkdir -p $(OUTPUT_DIR) && \
	sed $(LDS_REPLACE_DEFAULT) $(LDSCRIPT) > $(OUTPUT_DIR)/$(LDSCRIPT).in && \
	cpp -nostdinc -x assembler-with-cpp -C -P -E $(C_PREDEFINED) $(OUTPUT_DIR)/$(LDSCRIPT).in > $(OUTPUT_DIR)/$(LDSCRIPT) && \
	rm $(OUTPUT_DIR)/$(LDSCRIPT).in && \
	$(LD) -o $@ $(OBJ) $(MAIN_C:.c=.o) $(C_LDFLAGS) && \
	$(NM) -n $@ > $(SYM_NAME)

$(OUTPUT_NAME): $(IMG_NAME)
	@echo Creating executable $@ ... && \
	$(OBJCOPY) --gap-fill=0xff -O binary $< $@.tmp && \
	echo Encrypt binary file $@ ... && \
	$(NVT_TOOLS_DIR)/encrypt_bin SUM $@.tmp $(BIN_INFO_CHIPNAME) $(BIN_NAME)
ifeq ($(FW_TYPE),FW_TYPE_COMPRESS)
	@$(NVT_TOOLS_DIR)/bfc c lz $@.tmp $@.tmp.lz 0 0 -align && \
	cp -f $@.tmp.lz $@ && \
	rm -f $@.tmp.lz $@.tmp
else ifeq ($(FW_TYPE),FW_TYPE_PARTIAL_COMPRESS)
	@$(NVT_TOOLS_DIR)/bfc  c gz $@.tmp $@.tmp.lz 1 $(CODE_INFO_SECTION01_SIZE) $(BIN_INFO_LENGHT) $(BIN_INFO_BINCTRL) $(BIN_INFO_CHKSUM) $(EMBMEM_BLK_SIZE) && \
	cp -f $@.tmp.lz $@ && \
	rm -f $@.tmp.lz $@.tmp
else ifeq ($(FW_TYPE),FW_TYPE_LZMA)
	@$(NVT_TOOLS_DIR)/bfc c lzma $@.tmp $@.tmp.lz 0 0 -align && \
	cp -f $@.tmp.lz $@ && \
	rm -f $@.tmp.lz $@.tmp
else
	@cp -f $@.tmp $@ && \
	rm -f $@.tmp
endif


%.o: %.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo Compiling $<
	@$(CXX) $(C_CXXFLAGS) -c $< -o $@

%.o: %.S
	@echo Assembling $<
	@$(CC) $(C_AFLAGS) -c $< -o $@

%.dtb: %.dts
	@echo Compiling $<
	@cpp -nostdinc $(DTB_INCLUDE) -undef -x assembler-with-cpp $(C_PREDEFINED) $< > $<.tmp \
	&& dtc -O dtb -b 0 -o $@ $<.tmp \
	&& rm $<.tmp

%.o: %.dtb
	@echo Objectizing $< && \
	touch $<.tmp.c && \
	$(CC) $(C_CFLAGS) -c $<.tmp.c -o $@ && \
	$(OBJCOPY) --add-section .sensor=$< $@ && \
	rm $<.tmp.c

# cim is for coverity check
cim: $(OBJ)

clean:
	@rm -rf $(OBJ) $(MAIN_C:.c=.o) $(LDS_EXTERN) $(DTB) $(OUTPUT_DIR)

install: $(OUTPUT_NAME)
	@mkdir -p $(INSTALL_DIR)
	@cp -avf $(OUTPUT_NAME) $(INSTALL_DIR)
	@cp -avf $(DTB_APP) $(INSTALL_DIR)/$(notdir $(DTB_APP:.dtb=.bin))

dasm: $(IMG_NAME)
	@echo Disassembly $< to $(DASM_NAME)... \
	&& $(OBJDUMP) -D $(IMG_NAME) > $(DASM_NAME)

# codesize statistics
codesize: $(OUTPUT_NAME)
	@cd $(dir $(SYM_NAME)) && \
	$(BUILD_DIR)/nvt-tools/nvt-ld-op -r -s $(notdir $(SYM_NAME)) && \
	$(BUILD_DIR)/nvt-tools/nvt-ld-op -a $(notdir $(MAP_NAME)) && \
	echo see $(dir $(SYM_NAME))*.txt for the report
