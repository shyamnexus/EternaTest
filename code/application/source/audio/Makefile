TOPTARGETS := all clean

SUBDIRS := $(wildcard */.)

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)

install:
	@for n in $(SUBDIRS); do \
		$(MAKE) -C $$n install; \
	done

clean:
	@for n in $(SUBDIRS); do \
		$(MAKE) -C $$n clean; \
	done

release:
	@for n in $(SUBDIRS); do \
		$(MAKE) -C $$n release; \
	done
