MODULE_NAME = libvendor_cv
# DIRs
CV_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/cv/drivers
CV_LIB_DIR = $(NVT_HDAL_DIR)/vendor/cv
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
OUTPUT_DIR = $(NVT_HDAL_DIR)/vendor/output
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
CV_LIB_SRC_PATH = $(CV_LIB_DIR)/source
ISP_LIB_DIR = $(NVT_HDAL_DIR)/vendor/isp
ISP_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/isp/drivers

# INCs
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
CV_DRIVER_INC_PATH = $(CV_DRIVER_DIR)/include
CV_LIB_INC_PATH = $(CV_LIB_DIR)/include
CV_LIB_VER_INC_PATH = $(CV_LIB_DIR)/source/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
ISP_LIB_INC_PATH = $(ISP_LIB_DIR)/include
ISP_DRIVER_INC_PATH = $(ISP_DRIVER_DIR)/include
ISP_KFLOW_INC_PATH = $(NVT_HDAL_DIR)/drivers/k_flow/include
ISP_KDRIVER_INC_PATH = $(NVT_HDAL_DIR)/drivers/k_driver/include

# INC FLAGs
EXTRA_INCLUDE += \
	-I$(HDAL_INC_PATH) \
	-I$(CV_DRIVER_INC_PATH) \
	-I$(CV_LIB_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(VOS_INC_PATH) \

EXTRA_INCLUDE += -I$(CV_DRIVER_INC_PATH)/kdrv_md

EXTRA_INCLUDE += -I$(CV_LIB_INC_PATH)/vendor_dis

EXTRA_INCLUDE += -I$(CV_DRIVER_INC_PATH)/kdrv_ive

EXTRA_INCLUDE += -I$(CV_LIB_INC_PATH)/libive

EXTRA_INCLUDE += -I$(CV_LIB_INC_PATH)/libmd

EXTRA_INCLUDE += -I$(CV_LIB_SRC_PATH)/include

EXTRA_INCLUDE += -I$(CV_DRIVER_INC_PATH)/kdrv_trke

EXTRA_INCLUDE += -I$(CV_LIB_INC_PATH)/libtrke

EXTRA_INCLUDE += -I$(CV_LIB_INC_PATH)/libvqa

EXTRA_INCLUDE += -I$(CV_LIB_VER_INC_PATH)

EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(ISP_LIB_INC_PATH) \
	-I$(ISP_DRIVER_INC_PATH) \
	-I$(ISP_KFLOW_INC_PATH) \
	-I$(ISP_KDRIVER_INC_PATH) \

EXTRA_INCLUDE += -I$(CV_LIB_INC_PATH)/libdis

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += $(NVT_ASAN)
EXTRA_LDFLAGS = $(NVT_ASAN)    -lm
SRC += \
	vendor_md/vendor_md.c \
	vendor_ive/vendor_ive.c \
	vendor_trke/vendor_trke.c \
	vendor_dis/vendor_dis.c \
	vendor_dis/vendor_dis_version.c \
	libive/nvt_ive_filter.c \
	libive/nvt_ive_erode.c \
	libive/nvt_ive_dilate.c \
	libive/nvt_ive_thresh.c \
	libive/nvt_ive_canny.c \
	libive/nvt_ive_sobel.c \
	libive/nvt_ive_16bit_to_8bit.c \
	libive/nvt_ive_integral.c \
	libive/nvt_ive_median.c \
	libive/nvt_ive_min.c \
	libive/nvt_ive_max.c \
	libive/nvt_ive_stcorner.c \
	libive/nvt_ive_add.c \
	libive/nvt_ive_sub.c \
	libive/nvt_ive_and.c \
	libive/nvt_ive_or.c \
	libive/nvt_ive_xor.c \
	libive/nvt_ive_histo.c \
	libive/nvt_ive_ncc.c \
	libive/nvt_ive_dma.c \
	libive/nvt_ive_equalhisto.c \
	libive/nvt_ive_util.c \
	libive/nvt_ive_lbp.c \
	libive/nvt_ive_map.c \
	libive/nvt_ive_va.c \
	libive/nvt_ive_planar_to_pack.c \
	libtrke/nvt_trke.c \
	libmd/nvt_md_mdbc.c \
	libvqa/nvt_vqa.c \
	libvqa/nvt_vqa_clrcst.c \

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
# DIRs
RTOS_KERNEL_DIR = $(KERNELDIR)/lib/FreeRTOS
RTOS_LIB_DIR = $(KERNELDIR)/lib
RTOS_CURR_DEMO_DIR = $(KERNELDIR)/demos/novatek/$(SDK_CODENAME)
RTOS_POSIX_DIR = $(KERNELDIR)/lib/FreeRTOS-Plus-POSIX
RTOS_POSIX_SRC_DIR = $(RTOS_POSIX_DIR)/source
CV_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/cv/drivers
# INCs
CV_DRIVER_INC_PATH = $(CV_DRIVER_DIR)/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(RTOS_LIB_DIR) \
	-I$(CV_DRIVER_INC_PATH) \
	-I$(RTOS_KERNEL_DIR)/portable/GCC/ARM_CA9  \
	-I$(RTOS_KERNEL_DIR)/include \
	-I$(RTOS_KERNEL_DIR)/include/private \
	-I$(RTOS_CURR_DEMO_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include \
	-I$(RTOS_POSIX_DIR)/include/portable/novatek
SRC += \
	vendor_md/vendor_md.c \
	vendor_ive/vendor_ive.c \
	vendor_trke/vendor_trke.c \
	libvqa/nvt_vqa_clrcst.c \
	libvqa/nvt_vqa.c \

ifeq ($(findstring x64,$(RTOS_CPU_TYPE)),x64)
else
endif

endif

include $(NVT_HDAL_DIR)/library.mk
