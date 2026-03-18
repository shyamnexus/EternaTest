$(call force,CFG_ARM32_core,y)
$(call force,CFG_ARM64_core,n)
$(call force,CFG_WITH_LPAE,n)
$(call force,CFG_HWSUPP_MEM_PERM_WXN,n)
$(call force,CFG_HWSUPP_MEM_PERM_PXN,n)
$(call force,CFG_SECURE_TIME_SOURCE_CNTPCT,n)
arm32-platform-cpuarch 	:= cortex-a53
arm32-platform-cflags 	+= -march=armv7-a
arm32-platform-aflags 	+= -march=armv7-a
arm32-platform-cxxflags += -march=armv7-a
# Program flow prediction may need manual enablement
CFG_ENABLE_SCTLR_Z ?= y
