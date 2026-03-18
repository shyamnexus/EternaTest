#ifndef __ARCH_COMMON_EMB_PARTITION_INFO_H
#define __ARCH_COMMON_EMB_PARTITION_INFO_H

#define EMB_PARTITION_INFO_VER 0x16072117 ///< YYYY/MM/DD HH

/**
    Partition Infomation
    This is common header used between firmware of rtos, eCos, Linux, DSP
    so !!!!!! DO NOT modify it !!!!!!
*/

#define EMB_PARTITION_INFO_COUNT  20

typedef enum {
	FLASHTYPE_NOR    = 0x00,
	FLASHTYPE_NAND   = 0x01,
	FLASHTYPE_EMMC   = 0x02,
	FLASHTYPE_SIZE,
} nvt_flash_type_t;

typedef enum {
	FS_RAW           = 0x00,
	FS_SUPPORT       = 0x01,
	FS_JFFS2		 = 0x02,
	FS_SQUASH        = 0x03,
	FS_UBI           = 0x04,
	FS_EXT4          = 0x05,
	FS_FAT			 = 0x06,
	FS_SIZE,
} nvt_file_system_t;

enum {
	EMBTYPE_UNKNOWN   = 0x00,
	EMBTYPE_LOADER    = 0x01,  /* loader must always put in partition[0] */
	EMBTYPE_FDT       = 0x02,  /* modelext must always put in partition[1] */
	EMBTYPE_OBSOLETE1 = 0x03,
	EMBTYPE_OBSOLETE2 = 0x04,
	EMBTYPE_UBOOT     = 0x05,
	EMBTYPE_LINUX     = 0x06,
	EMBTYPE_DSP       = 0x07,
	EMBTYPE_PSTORE    = 0x08,
	EMBTYPE_FAT       = 0x09,
	EMBTYPE_EXFAT     = 0x0A,
	EMBTYPE_ROOTFS    = 0x0B,
	EMBTYPE_RAMFS     = 0x0C,
	EMBTYPE_UENV      = 0x0D, /* u-boot environment data */
	EMBTYPE_MBR       = 0x0E, /* for emmc partition, mbr always put in partition[0] instead of loader */
	EMBTYPE_ROOTFSL   = 0x0F, /* for emmc logical partition */
	EMBTYPE_RTOS      = 0x10, /* freertos */
	EMBTYPE_APP       = 0x11, /* for APP , like IQ configs */
	EMBTYPE_TEEOS     = 0x12, /* for secure os*/
	EMBTYPE_AI        = 0x13, /* for fastboot-ai*/
	EMBTYPE_ATF       = 0x14, /* for ATF */
	EMBTYPE_BANK      = 0x15, /* for dual rtos, extra partition called bank */
	EMBTYPE_MCU       = 0x16, /* MCU firmware, such as 8051, andes */
	EMBTYPE_FDTFAST   = 0x17, /* an extend fdt for fastboot only */
	EMBTYPE_SIZE,
};
/**
    customer defined data partition format
*/
enum {
	EMBTYPE_USER	= 0x80,
	EMBTYPE_USERRAW	= 0x81,
	EMBTYPE_USR_SIZE,
};

#define GET_USER_PART_NUM(m)	((m) - 0x80)

#define EMBTYPE_TOTAL_SIZE	(EMBTYPE_SIZE + EMBTYPE_USR_SIZE - 0x80)
/* for reason of compatiable linux, we use original type to decalre */

typedef struct _EMB_PARTITION {
    unsigned short		FlashType;       /* FLASHTYPE_ */
	unsigned short		EmbType;         /* EMBTYPE_ */
	unsigned short		OrderIdx;        /* Order index of the same EmbType based on '0' */
	unsigned long long	PartitionOffset; /* Phyical offset of partition */
	unsigned long long	PartitionSize;   /* Size of this partition */
	unsigned long long	ReversedSize;    /* Reserved size for bad block */
	unsigned short		FileExist;       /* File name if exist */
} EMB_PARTITION, *PEMB_PARTITION;

typedef struct _EMB_PARTITION_FDT_TRANSLATE_TABLE {
	char fdt_node_name[30];
	unsigned short emb_type;
} EMB_PARTITION_FDT_TRANSLATE_TABLE;
#endif /* __ARCH_COMMON_EMB_PARTITION_INFO_H */
