include $(NVT_PRJCFG_MODEL_CFG)

SUBDIRS := $(dir $(wildcard */Makefile */makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\/$$//g';)

.PHONY: all clean install $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

ifneq ($(NVT_SAMPLES_INSTALL),DISABLE)
install: $(SUBDIRS)
else
install:
	@echo NVT_SAMPLES_INSTALL is disabled to skip install samples
endif

clean: $(SUBDIRS)
	@rm output -rf
