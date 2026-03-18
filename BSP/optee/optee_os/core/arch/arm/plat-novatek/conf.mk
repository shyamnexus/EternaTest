PLATFORM_FLAVOR ?= na51055
core-platform-cppflags += -D__OPTEEOS

ifeq ($(PLATFORM_FLAVOR),na51055)
core-platform-cppflags += -D_BSP_NA51055_
$(call force,CFG_TEE_CORE_NB_CORE,CFG_TEE_CORE_NUM)
CFG_NUM_THREADS ?= 2

else ifeq ($(PLATFORM_FLAVOR),na51084)
core-platform-cppflags += -D_BSP_NA51084_
$(call force,CFG_TEE_CORE_NB_CORE,2)
CFG_NUM_THREADS ?= 2

else ifeq ($(PLATFORM_FLAVOR),na51089)
core-platform-cppflags += -D_BSP_NA51089_
$(call force,CFG_TEE_CORE_NB_CORE,1)
CFG_NUM_THREADS ?= 2
$(call force,CFG_CORE_ASLR,n)

else ifeq ($(PLATFORM_FLAVOR),na51090)
core-platform-cppflags += -D_BSP_NA51090_
$(call force,CFG_TEE_CORE_NB_CORE,CFG_TEE_CORE_NUM)
CFG_NUM_THREADS ?= CFG_TEE_CORE_NUM
$(call force,CFG_CORE_ARM64_PA_BITS,36)
$(call force,CFG_CORE_ASLR,n)

else ifeq ($(PLATFORM_FLAVOR),na51102)
core-platform-cppflags += -D_BSP_NA51102_
$(call force,CFG_TEE_CORE_NB_CORE,CFG_TEE_CORE_NUM)
CFG_NUM_THREADS ?= CFG_TEE_CORE_NUM
$(call force,CFG_CORE_ARM64_PA_BITS,36)
$(call force,CFG_CORE_ASLR,n)

else ifeq ($(PLATFORM_FLAVOR),na51103)
core-platform-cppflags += -D_BSP_NA51103_
$(call force,CFG_TEE_CORE_NB_CORE,CFG_TEE_CORE_NUM)
CFG_NUM_THREADS ?= CFG_TEE_CORE_NUM
$(call force,CFG_CORE_ASLR,n)

else ifeq ($(PLATFORM_FLAVOR),ns02201)
core-platform-cppflags += -D_BSP_NS02201_
$(call force,CFG_TEE_CORE_NB_CORE,CFG_TEE_CORE_NUM)
CFG_NUM_THREADS ?= CFG_TEE_CORE_NUM
$(call force,CFG_CORE_ARM64_PA_BITS,36)
$(call force,CFG_CORE_ASLR,n)

else ifeq ($(PLATFORM_FLAVOR),ns02301)
core-platform-cppflags += -D_BSP_NS02301_
$(call force,CFG_TEE_CORE_NB_CORE,CFG_TEE_CORE_NUM)
CFG_NUM_THREADS ?= CFG_TEE_CORE_NUM
$(call force,CFG_CORE_ASLR,n)

else ifeq ($(PLATFORM_FLAVOR),ns02302)
core-platform-cppflags += -D_BSP_NS02302_
$(call force,CFG_TEE_CORE_NB_CORE,CFG_TEE_CORE_NUM)
CFG_NUM_THREADS ?= CFG_TEE_CORE_NUM
$(call force,CFG_CORE_ARM64_PA_BITS,36)
$(call force,CFG_CORE_ASLR,n)

else ifeq ($(PLATFORM_FLAVOR),ns02401)
core-platform-cppflags += -D_BSP_NS02401_
$(call force,CFG_TEE_CORE_NB_CORE,CFG_TEE_CORE_NUM)
CFG_NUM_THREADS ?= CFG_TEE_CORE_NUM
$(call force,CFG_CORE_ARM64_PA_BITS,36)
$(call force,CFG_CORE_ASLR,n)
$(call force,CFG_CORE_CLUSTER_SHIFT,2) #2**(CFG_CORE_CLUSTER_SHIFT) = 4 cores per cluster
$(call force,CFG_CORE_THREAD_SHIFT,0)
$(call force,CFG_ARM_GICV3, y)

else
$(error Error: $(PLATFORM_FLAVOR) Not supported currently)
endif

ca7-flavorlist = \
        ns02301 \

ca9-flavorlist = \
        na51055 \
		na51084 \
		na51089 \

ca53-flavorlist = \
		na51103 \

armv8-flavorlist = \
        na51090 \
        na51102 \
        ns02201 \
        ns02302 \
        ns02401 \

ifneq (,$(filter $(PLATFORM_FLAVOR),$(ca9-flavorlist)))
$(call force,CFG_ARM32_core,y)
$(call force,CFG_PL310,y)
$(call force,CFG_PL310_LOCKED,y)
$(call force,CFG_WITH_LPAE,n)
$(call force,CFG_GIC,y)
$(call force,CFG_WITH_ARM_TRUSTED_FW,n)
CFG_BOOT_SECONDARY_REQUEST ?= y
CFG_INIT_CNTVOFF ?= y
include core/arch/arm/cpu/cortex-a9.mk
else ifneq (,$(filter $(PLATFORM_FLAVOR),$(ca7-flavorlist)))
$(call force,CFG_ARM32_core,y)
$(call force,CFG_PL310,n)
$(call force,CFG_PL310_LOCKED,n)
$(call force,CFG_WITH_LPAE,n)
$(call force,CFG_GIC,y)
$(call force,CFG_WITH_ARM_TRUSTED_FW,n)
CFG_BOOT_SECONDARY_REQUEST ?= y
CFG_INIT_CNTVOFF ?= y
include core/arch/arm/cpu/cortex-a7.mk
else ifneq (,$(filter $(PLATFORM_FLAVOR),$(ca53-flavorlist)))
$(call force,CFG_ARM64_core,n)
$(call force,CFG_ARM32_core,y)
$(call force,CFG_GIC,y)
$(call force,CFG_WITH_LPAE,n)
$(call force,CFG_WITH_ARM_TRUSTED_FW,n)
CFG_USER_TA_TARGETS ?= ta_arm32
CFG_BOOT_SECONDARY_REQUEST ?= y
CFG_INIT_CNTVOFF ?= y
include core/arch/arm/cpu/cortex-armv7-0.mk
else ifneq (,$(filter $(PLATFORM_FLAVOR),$(armv8-flavorlist)))
$(call force,CFG_ARM64_core,y)
$(call force,CFG_ARM32_core,n)
$(call force,CFG_GIC,y)
$(call force,CFG_WITH_LPAE,y)
$(call force,CFG_WITH_ARM_TRUSTED_FW,y)
CFG_USER_TA_TARGETS ?= ta_arm64
include core/arch/arm/cpu/cortex-armv8-0.mk
else
$(error Unsupported PLATFORM_FLAVOR "$(PLATFORM_FLAVOR)")
endif

$(call force,CFG_SECURE_TIME_SOURCE_REE,y)
$(call force,CFG_CACHE_API,y)
$(call force,CFG_PM_STUBS,y)
$(call force,CFG_GENERIC_BOOT,y)
$(call force,CFG_TEE_CORE_DEBUG,n)
$(call force,CFG_8250_UART,y)
$(call force,CFG_TEE_LOAD_ADDR,y)
$(call force,CFG_NVT_IVOT,y)
$(call force,CFG_NVT_TZPC,y)
ifeq ($(PLATFORM_FLAVOR),na51102)
$(call force,CFG_NVT_CRYPTO,y)
$(call force,CFG_NVT_HASH,y)
$(call force,CFG_NVT_RSA,y)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
else ifeq ($(PLATFORM_FLAVOR),na51090)
$(call force,CFG_NVT_CRYPTO,y)
$(call force,CFG_NVT_HASH,y)
$(call force,CFG_NVT_RSA,y)
$(call force,CFG_NVT_ECDSA,n)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
else ifeq ($(PLATFORM_FLAVOR),na51103)
$(call force,CFG_NVT_CRYPTO,y)
$(call force,CFG_NVT_HASH,y)
$(call force,CFG_NVT_RSA,y)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
else ifeq ($(PLATFORM_FLAVOR),ns02201)
$(call force,CFG_NVT_CRYPTO,y)
$(call force,CFG_NVT_HASH,y)
$(call force,CFG_NVT_RSA,y)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
else ifeq ($(PLATFORM_FLAVOR),na51055)
$(call force,CFG_NVT_CRYPTO,n)
$(call force,CFG_NVT_HASH,n)
$(call force,CFG_NVT_RSA,n)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
else ifeq ($(PLATFORM_FLAVOR),na51089)
$(call force,CFG_NVT_CRYPTO,y)
$(call force,CFG_NVT_HASH,y)
$(call force,CFG_NVT_RSA,y)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
else ifeq ($(PLATFORM_FLAVOR),ns02301)
$(call force,CFG_NVT_CRYPTO,y)
$(call force,CFG_NVT_HASH,y)
$(call force,CFG_NVT_RSA,y)
$(call force,CFG_NVT_ECDSA,y)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
else ifeq ($(PLATFORM_FLAVOR),ns02302)
$(call force,CFG_NVT_CRYPTO,y)
$(call force,CFG_NVT_HASH,y)
$(call force,CFG_NVT_RSA,y)
$(call force,CFG_NVT_ECDSA,y)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
else ifeq ($(PLATFORM_FLAVOR),ns02401)
$(call force,CFG_NVT_CRYPTO,y)
$(call force,CFG_NVT_HASH,y)
$(call force,CFG_NVT_RSA,y)
$(call force,CFG_NVT_ECDSA,y)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
else
$(call force,CFG_NVT_CRYPTO,y)
$(call force,CFG_NVT_HASH,y)
$(call force,CFG_NVT_RSA,y)
$(call force,CFG_NVT_TZASC,y)
$(call force,CFG_NVT_OPT,y)
endif
$(call force,CFG_CRYPTO_DRIVER,y)
$(call force,CFG_CRYPTO_DRV_HASH,y)
$(call force,CFG_CRYPTO_DRV_HMAC,y)
$(call force,CFG_CRYPTO_DRV_CIPHER,y)

CFG_CRYPTO_DRIVER_DEBUG ?= 0
CFG_WARN_INSECURE ?= n
CFG_WITH_PAGER ?= n
CFG_BOOT_SYNC_CPU ?= n
CFG_TEE_CORE_EMBED_INTERNAL_TESTS ?= n
CFG_WITH_STACK_CANARIES ?= y
CFG_WITH_STATS ?= y
CFG_WITH_SOFTWARE_PRNG ?= n
CFG_STIH_UART ?= y
CFG_ENABLE_SCTLR_RR ?= y
CFG_UNWIND ?= y

##################################################################################################
# TEE/TZ RAM layout:
#			+---------------------------------------+  <- SHMEM_SIZE
#  	^		|     Non secure     |  SHM             |   ^
#  	|		|   shared memory    |                  |   |
#  	|		|                    |                  |   | SHMEM_START
#  	|		+---------------------------------------+  <- TZDRAM_SIZE
#  	|		| TEE private secure |  TA_RAM          |   ^
#  	|		|   external memory  +------------------+   | CFG_TEE_RAM_VA_SIZE
#  	|		|                    |  TEE_RAM         |   |
#  	| 		+---------------------------------------+   | TZDRAM_BASE
#
#
#  TEE_RAM : 2MByte
#  TA_RAM  : 6MByte
#  PUB_RAM : 4MByte
#
##################################################################################################

ifneq (,$(filter $(PLATFORM_FLAVOR),na51055 na51084 na51089 ns02301))
CFG_TZDRAM_START ?= 0x02000000
CFG_TZDRAM_SIZE ?= 0x00800000
CFG_CORE_HEAP_SIZE  ?= 0x000A0000
CFG_TEE_RAM_VA_SIZE := 0x00180000
CFG_TEE_LOAD_ADDR := 0x02000000
CFG_SHMEM_START ?= 0x02800000
CFG_SHMEM_SIZE ?= 0x00400000

CFG_IN_TREE_EARLY_TAS += avb/023f8f1a-292a-432b-8fc4-de8471358067
else ifneq (,$(filter $(PLATFORM_FLAVOR), na51090 na51102 na51103 ns02201 ns02302 ns02401))
CFG_TZDRAM_START ?= 0x02000000
CFG_TZDRAM_SIZE ?= 0x00800000
CFG_CORE_HEAP_SIZE  ?= 0x000A0000
CFG_TEE_RAM_VA_SIZE := 0x00180000
CFG_TEE_LOAD_ADDR := 0x02000000
CFG_SHMEM_START ?= 0x02800000
CFG_SHMEM_SIZE ?= 0x00400000

else
$(error Error: $(PLATFORM_FLAVOR) Not supported currently)
endif

CFG_MMAP_REGIONS ?= 24
