SUBDIRS := $(dir $(shell find source -maxdepth 2 -name Makefile))
SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)
SUBDIRS += external
OUTPUT_DIR = output

.PHONY: all clean install $(SUBDIRS)

all: $(SUBDIRS)

$(OUTPUT_DIR):
	@mkdir -p $@

$(SUBDIRS): $(OUTPUT_DIR)
	@$(MAKE) -C $@ $(MAKECMDGOALS) $(NVT_MULTI_CORES_FLAG);

install: $(SUBDIRS)

clean: $(SUBDIRS)
	@rm $(OUTPUT_DIR) -rf
