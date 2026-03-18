MODULE_NAME = kdrv_jpg_old
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \

EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_$(SDK_CODENAME)

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
EXTRA_CFLAGS += -DDEBUG

SRC += \
	jpeg_main.c \
	jpeg_proc.c \
	jpeg_drv.c \
	jpeg_api.c \
	jpeg_int.c \
	jpeg.c \
	jpeg_file.c \
	jpeg_platform.c \
	kdrv_jpeg_queue.c \
	brc/jpegbrc_default.c \
	hal/jpg_header.c \
	hal/jpg_enc.c \
	hal/jpg_dec.c \
	hal/jpg_header.c \
	hal/jpg_parseheader.c \
	temp/dal_jpegenc.c \
	temp/dal_jpegdec.c \
	test/exam_jpg.c

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
EXTRA_CFLAGS += \
	-DDEBUG \
	-I$(CURDIR)/include \

SRC_SKIP_BUILD += $(wildcard /*.c)
SRC += \
	# jpeg_int.c \
	# jpeg.c \
	# jpeg_platform.c \
	# kdrv_jpeg_queue.c \
	# brc/jpegbrc_default.c \
	# hal/jpg_header.c \
	# hal/jpg_enc.c \
	# hal/jpg_dec.c \
	# hal/jpg_header.c \
	# hal/jpg_parseheader.c \
	# temp/dal_jpegenc.c \
	# temp/dal_jpegdec.c \

endif

include $(NVT_HDAL_DIR)/driver.mk
