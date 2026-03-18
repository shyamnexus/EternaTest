#
# arch/arm64/boot/Makefile
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies.
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 2012, ARM Ltd.
# Author: Will Deacon <will.deacon@arm.com>
#
# Based on the ia64 boot/Makefile.
#

OBJCOPYFLAGS_Image :=-O binary -R .note -R .note.gnu.build-id -R .comment -S

targets := Image Image.bz2 Image.gz Image.lz4 Image.lzma Image.lzo

# Latest kernel version does not have load and entry address offset.
UIMAGE_LOADADDR=0
UIMAGE_ENTRYADDR=0

$(obj)/Image: vmlinux FORCE
	$(call if_changed,objcopy)

$(obj)/Image.bz2: $(obj)/Image FORCE
	$(call if_changed,bzip2)

$(obj)/Image.gz: $(obj)/Image FORCE
	$(call if_changed,gzip)

$(obj)/Image.lz4: $(obj)/Image FORCE
	$(call if_changed,lz4)

$(obj)/Image.lzma: $(obj)/Image FORCE
	$(call if_changed,lzma)

$(obj)/Image.lzo: $(obj)/Image FORCE
	$(call if_changed,lzo)

install:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh $(KERNELRELEASE) \
	$(obj)/Image System.map "$(INSTALL_PATH)"

zinstall:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh $(KERNELRELEASE) \
	$(obj)/Image.gz System.map "$(INSTALL_PATH)"

$(obj)/uImage.bz2:  $(obj)/Image.bz2 FORCE
	@$(check_for_multiple_loadaddr)
	$(call if_changed,uimage)
	@echo $@ is ready

$(obj)/uImage.gz:  $(obj)/Image.gz FORCE
	@$(check_for_multiple_loadaddr)
	$(call if_changed,uimage)
	@echo $@ is ready

$(obj)/uImage.lz4:  $(obj)/Image.lz4 FORCE
	@$(check_for_multiple_loadaddr)
	$(call if_changed,uimage)
	@echo $@ is ready

$(obj)/uImage.lzma:  $(obj)/Image.lzma FORCE
	@$(check_for_multiple_loadaddr)
	$(call if_changed,uimage)
	@echo $@ is ready

$(obj)/uImage.lzo:  $(obj)/Image.lzo FORCE
	@$(check_for_multiple_loadaddr)
	$(call if_changed,uimage)
	@echo $@ is ready
