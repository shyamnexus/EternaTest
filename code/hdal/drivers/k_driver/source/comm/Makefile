MODULE_NAME = kdrv_comm
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
EXTRA_INCLUDE += \
	-I$(VOS_DRIVER_DIR)/include \
	-I$(KDRV_DIR)/include \
	-I$(KDRV_DIR)/include/plat \
	-I$(CURR_DRIVER_DIR)/source/include \
	-I$(CURR_DRIVER_DIR)/source/comm/ddr_arb/include \
	-I$(CURR_DRIVER_DIR)/source/comm/drv_sys/include \
	-I$(CURR_DRIVER_DIR)/source/comm/pwm/include \
	-I$(CURR_DRIVER_DIR)/source/comm/sif/include \
	-I$(CURR_DRIVER_DIR)/source/comm/hwcopy/include \
	-I$(KDRV_DIR)/include/comm \
	-I$(KDRV_DIR)/include/comm/util \

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += \
	-Wno-sign-compare \
	-DDEBUG \
	-D_GROUP_KO_ \
	-D_$(NVT_SDK_TYPE)_ \

ifeq ($(CONFIG_NVT_HDAL_KDRV_COMM),y)
OBJ-TAG = $(CONFIG_NVT_HDAL_KDRV_COMM)
endif

SRC += \
	kdrv_comm_main.c \
	drv_sys/timer_main.c \
	drv_sys/timer_proc.c \
	drv_sys/timer_drv.c \
	drv_sys/timer_api.c \
	drv_sys/timer_platform.c \
	drv_sys/timer.c \
	drv_sys/timer_ioctl.c \
	drv_sys/timer_export.c \
	drv_sys/hwclock.c \
	drv_sys/hwclock_proc.c \
	drv_sys/heavyload.c \
	drv_sys/heavyload_proc.c \
	drv_sys/nvt_jiffies.c \
	drvdump/nvt_drvdump.c \
	ddr_arb/ddr_arb_main.c \
	ddr_arb/ddr_arb_proc.c \
	ddr_arb/ddr_arb_drv.c \
	ddr_arb/ddr_arb_api.c \
	ddr_arb/ddr_arb_platform.c \
	ddr_arb/arb.c \
	ddr_arb/emu_arb.c \
	ddr_arb/arb_ver.c \
	ddr_arb/ddr_arb_export.c \
	hwcopy/hwcopy_main.c \
	hwcopy/hwcopy_proc.c \
	hwcopy/hwcopy_drv.c \
	hwcopy/hwcopy_api.c \
	hwcopy/hwcopy.c \
	hwcopy/hwcopy_platform.c \
	hwcopy/hwcopy_cert.c	\
	hwcopy/emu_hwcopy.c \
	hwcopy/kdrv_hwcopy_if.c \
	hwcopy/hwcopy_export.c \
	pwm/pwm_main.c \
	pwm/pwm_proc.c \
	pwm/pwm_drv.c \
	pwm/pwm_api.c \
	pwm/pwm_platform.c \
	pwm/pwm.c \
	pwm/pwm_ver.c \
	pwm/pwm_export.c \
	pwm/pwm_cert.c \
	sif/sif_main.c \
	sif/sif_proc.c \
	sif/sif_drv.c \
	sif/sif_api.c \
	sif/sif.c \
	sif/sif_export.c \
	util/info.c \
	util/vg_debug.c \
	util/vg_lock.c \
	util/vg_log.c \
	util/vg_memcpy.c \
	util/vg_semaphore.c \
	util/vg_timer.c \
	util/vg_log_core.c \
	util/vg_log_pub.c \
	util/vg_log_pif.c \
	util/isf_debug_log.c \
	nvt_dmasys/dma_nvt.c \
	nvt_dmasys/dma_util.c \
	nvt_dmasys/dma_proc.c \
	fdt/fdt_export.c \

ifneq ($(CONFIG_NVT_HDAL_KDRV_COMM),y)
SRC += \
	fdt/fdt.c \
	fdt/fdt_ro.c \
	fdt/fdt_wip.c \
	fdt/fdt_sw.c \
	fdt/fdt_rw.c \
	fdt/fdt_strerror.c \
	fdt/fdt_empty_tree.c \
	fdt/fdt_addresses.c \
	fdt/fdt_overlay.c \

endif

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
include $(NVT_PRJCFG_MODEL_CFG)
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat

EXTRA_CFLAGS += \
	-Wno-sign-compare \
	-DDEBUG \
	-D_$(CPU2_TYPE)_ \


SRC += \
	kdrv_comm_main.c \
	drv_sys/dummy_core.c \
	fdt/fdt.c \
	fdt/fdt_ro.c \
	fdt/fdt_wip.c \
	fdt/fdt_sw.c \
	fdt/fdt_rw.c \
	fdt/fdt_strerror.c \
	fdt/fdt_empty_tree.c \
	fdt/fdt_addresses.c \
	fdt/fdt_overlay.c \
	drv_sys/timer_platform.c \
	drv_sys/timer.c \
	drv_sys/timer_drv.c \
	drv_sys/timer_freertos.c \
	drv_sys/hwclock.c \
	drv_sys/hwclock_freertos.c \
	drv_sys/delay.c \
	drv_sys/heavyload.c \
	ddr_arb/arb.c\
	ddr_arb/ddr_arb_platform.c\
	ddr_arb/arb_ver.c \
	pwm/pwm.c \
	pwm/pwm_platform.c \
	pwm/pwm_ver.c \
	pwm/pwm_cert.c \
	drvdump/nvt_drvdump.c \
	util/vg_log_rtos.c \
	util/vg_log_core.c \
	util/vg_log_pub.c \
	util/vg_log_pif.c \
	util/isf_debug_log.c \


endif

include $(NVT_HDAL_DIR)/driver.mk
