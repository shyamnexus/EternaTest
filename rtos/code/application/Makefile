include $(NVT_PRJCFG_MODEL_CFG)

TARGET_DIR = ./source/$(RTOS_APP_MAIN)

SUBDIRS := $(dir $(shell find . -name Makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)
OUTPUT_DIR = output

all: $(OUTPUT_DIR)
ifeq ($(RTOS_APP_MAIN),)
	@echo ERROR: RTOS_MAIN cannot be empty in nvt-xxx-info.dtsi
	@exit 1;
else
	@$(MAKE) -C $(TARGET_DIR)
endif

$(OUTPUT_DIR):
	@mkdir -p $@

cim:
ifeq ($(RTOS_APP_MAIN),)
	@echo ERROR: RTOS_MAIN cannot be empty in nvt-xxx-info.dtsi
	@exit 1;
else
	@$(MAKE) -C $(TARGET_DIR) cim
endif

clean:
	@for n in $(SUBDIRS); do \
		$(MAKE) -C $$n clean $(NVT_MULTI_CORES_FLAG); \
	done
	@rm $(OUTPUT_DIR) -rf

install:
	@$(MAKE) -C $(TARGET_DIR) install
