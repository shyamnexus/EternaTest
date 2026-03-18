# lookup table for $RTOS_CPU_TYPE -> threadx ports name

PORT_NAME = $($(RTOS_CPU_TYPE))
# mark it for non-smp
#SMP_SUFFIX = -SMP

ifeq ($(SDK_CODENAME),)
$(error $$SDK_CODENAME is not found)
endif

DIR_KERNEL      = ./FreeRTOS-Kernel$(SMP_SUFFIX)
DIR_KERNEL_PORT = ./novatek/$(SDK_CODENAME)$(SMP_SUFFIX)
DIR_POSIX       = ./Lab-Project-FreeRTOS-POSIX
DIR_POSIX_PORT  = ./novatek/posix
DIR_TCP         = ./FreeRTOS-Plus-TCP
DIR_TCP_PORT    = ./novatek/tcp
DIR_LWIP        = ./lwip
DIR_LWIP_PORT   = ./novatek/lwip

DIR_INC_KERNEL      = $(DIR_KERNEL)/include
DIR_INC_KERNEL_PORT = $(DIR_KERNEL_PORT)/include
DIR_INC_POSIX       = $(DIR_POSIX)/include
DIR_INC_POSIX_PORT  = $(DIR_POSIX_PORT)
DIR_INC_TCP         = $(DIR_TCP)/include
DIR_INC_TCP_PORT    = $(DIR_TCP_PORT)
DIR_INC_LWIP        = $(DIR_LWIP)/src/include
DIR_INC_LWIP_PORT   = $(DIR_LWIP_PORT)

SYMLINK_KERNEL         = ./include/freertos
SYMLINK_KERNEL_PORT    = ./include/freertos-port
SYMLINK_POSIX          = ./include/posix
SYMLINK_POSIX_PORT     = ./include/posix-port
SYMLINK_TCP            = ./include/tcp
SYMLINK_TCP_PORT       = ./include/tcp-port
SYMLINK_LWIP           = ./include/lwip
SYMLINK_LWIP_PORT      = ./include/lwip-port
SYMLINK_INC            = \
	$(SYMLINK_KERNEL) \
	$(SYMLINK_KERNEL_PORT) \
	$(SYMLINK_POSIX) \
	$(SYMLINK_POSIX_PORT) \
	$(SYMLINK_TCP) \
	$(SYMLINK_TCP_PORT) \
	$(SYMLINK_LWIP) \
	$(SYMLINK_LWIP_PORT)

.PHONY: all clean

all: $(SYMLINK_INC)
ifneq ($(wildcard $(DIR_KERNEL)/Makefile),)
	@$(MAKE) -C $(DIR_KERNEL) $(NVT_MULTI_CORES_FLAG)
endif
ifneq ($(wildcard $(DIR_POSIX)/Makefile),)
	@$(MAKE) -C $(DIR_POSIX) $(NVT_MULTI_CORES_FLAG)
endif
ifneq ($(wildcard $(DIR_TCP)/Makefile),)
	@$(MAKE) -C $(DIR_TCP) $(NVT_MULTI_CORES_FLAG)
endif
ifneq ($(wildcard $(DIR_LWIP)/Makefile),)
	@$(MAKE) -C $(DIR_LWIP) $(NVT_MULTI_CORES_FLAG)
endif

install:
ifneq ($(wildcard $(DIR_KERNEL)/Makefile),)
	@$(MAKE) -C $(DIR_KERNEL) install
endif
ifneq ($(wildcard $(DIR_POSIX)/Makefile),)
	@$(MAKE) -C $(DIR_POSIX) install
endif
ifneq ($(wildcard $(DIR_TCP)/Makefile),)
	@$(MAKE) -C $(DIR_TCP) install
endif
ifneq ($(wildcard $(DIR_LWIP)/Makefile),)
	@$(MAKE) -C $(DIR_LWIP) install
endif


clean:
ifneq ($(wildcard $(DIR_KERNEL)/Makefile),)
	@$(MAKE) -C $(DIR_KERNEL) clean
endif
ifneq ($(wildcard $(DIR_POSIX)/Makefile),)
	@$(MAKE) -C $(DIR_POSIX) clean
endif
ifneq ($(wildcard $(DIR_TCP)/Makefile),)
	@$(MAKE) -C $(DIR_TCP) clean
endif
ifneq ($(wildcard $(DIR_LWIP)/Makefile),)
	@$(MAKE) -C $(DIR_LWIP) clean
endif
	@rm -rf $(SYMLINK_INC)

$(SYMLINK_INC):
ifeq ($(wildcard $(DIR_KERNEL_PORT)/.*),)
	$(error unable to find $(PWD)/$(DIR_KERNEL_PORT))
endif
ifeq ($(wildcard $(SYMLINK_KERNEL)),)
	@ln -s ../$(DIR_INC_KERNEL) $(SYMLINK_KERNEL)
endif
ifeq ($(wildcard $(SYMLINK_KERNEL_PORT)),)
	@ln -s ../$(DIR_INC_KERNEL_PORT) $(SYMLINK_KERNEL_PORT)
endif
ifeq ($(wildcard $(SYMLINK_POSIX)),)
	@ln -s ../$(DIR_INC_POSIX) $(SYMLINK_POSIX)
endif
ifeq ($(wildcard $(SYMLINK_POSIX_PORT)),)
	@ln -s ../$(DIR_INC_POSIX_PORT) $(SYMLINK_POSIX_PORT)
endif
ifeq ($(wildcard $(SYMLINK_TCP)),)
	@ln -s ../$(DIR_INC_TCP) $(SYMLINK_TCP)
endif
ifeq ($(wildcard $(SYMLINK_TCP_PORT)),)
	@ln -s ../$(DIR_INC_TCP_PORT) $(SYMLINK_TCP_PORT)
endif
ifeq ($(wildcard $(SYMLINK_LWIP)),)
	@ln -s ../$(DIR_INC_LWIP) $(SYMLINK_LWIP)
endif
ifeq ($(wildcard $(SYMLINK_LWIP_PORT)),)
	@ln -s ../$(DIR_INC_LWIP_PORT) $(SYMLINK_LWIP_PORT)
endif

symbolic: $(SYMLINK_INC)
