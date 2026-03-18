MODULE_NAME = nvt-fastboot.bin
include $(NVT_PRJCFG_MODEL_CFG)

DTC_VER = $(shell ver=`dtc -v | awk -F' ' '{print $$NF}' | awk -F'-' '{print $$(NF-1)}'`; echo "$$ver" | sed 's/[^a-zA-Z0-9]//g')

ifeq ($(shell expr $(DTC_VER) \< 145), 1)
$(error Bad dtc version, please install v1.4.5 or later)
endif

EMBMEM_DTR ?= EMBMEM_DTR_NONE

DTC_CFLAGS = $(shell echo "-W no-unit_address_format -W no-unit_address_vs_reg")

DTS_SRC = nvt-fastboot.dts
DTB = $(DTS_SRC:.dts=.dtb)

DTB_INCLUDE = \
	-I$(CONFIG_DIR)/include \

C_DEFINE_EXT = \
	-D_INDEPENDENT_FDT_ \

DTB_CFLAGS = \
	$(DTB_INCLUDE) \
	$(C_DEFINE_EXT) \
	-undef -x assembler-with-cpp \

.PHONY: all clean install

all: $(MODULE_NAME)

%.dtb: %.dts
	@echo Compling $<
	@cpp -nostdinc $(DTB_CFLAGS) $< > $<.tmp \
	&& dtc -@ -O dtb -b 0 $(DTC_CFLAGS) -o $@ $<.tmp \
	&& $(BUILD_DIR)/nvt-tools/mkfastboot.py -i $<.tmp -o fastboot -d $(EMBMEM_DTR) \
	&& rm $<.tmp

$(DTB): $(filter-out $(DTS_SRC), $(wildcard *.dtsi) $(wildcard *.dts) $(wildcard $(CONFIG_DIR)/include/*/*.dtsi))

$(MODULE_NAME): $(DTB) $(NVT_PRJCFG_MODEL_CFG)
ifeq ($(EMBMEM_DTR), EMBMEM_DTR_4_BIT)
	@echo DTR-4BIT
	@cat flash_tunning_pattern_4bit.bin $< > $@
else ifeq ($(EMBMEM_DTR), EMBMEM_DTR_8_BIT)
	@echo DTR-8BIT
	@cat flash_tunning_pattern_8bit.bin $< > $@
else
	@cp -f $< $@
endif

install: $(MODULE_NAME)
	@cp $< $(OUTPUT_DIR)

clean:
	@rm -f $(MODULE_NAME) $(DTB)
	@rm -rf fastboot

