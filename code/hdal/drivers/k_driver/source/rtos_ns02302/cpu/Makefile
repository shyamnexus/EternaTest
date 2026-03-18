MODULE_NAME = kdrv_cpu
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I $(KDRV_DIR)/include \
	-I$(KDRV_DIR)/source/include \
	-I$(CURR_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include/plat \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	dummy.c \
	dummy_core.c \


###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += -DDEBUG

ifeq ($(findstring x64,$(RTOS_CPU_TYPE)),x64)
ASM = \
	./cache_v8.S

SRC += \
	./cache_64.c \
	./cpu_cp_64.c

else
ASM = \
	./cpu_s.S \
	./power_L2.S

SRC += \
	./cache.c \
	./cpu_cp.c
endif

endif

include $(NVT_HDAL_DIR)/driver.mk
