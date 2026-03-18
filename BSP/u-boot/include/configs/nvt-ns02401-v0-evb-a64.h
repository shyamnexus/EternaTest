/*
 * Copyright (C) 2023 Novatek Microelectronics Corp. All rights reserved.
 * Author: iVoT-IM <iVoT_MailGrp@novatek.com.tw>
 *
 * Configuration settings for the Novatek NS02401 SOC.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_NS02401_V0_H
#define __CONFIG_NS02401_V0_H

#include <linux/sizes.h>

/* #define CONFIG_DEBUG 1 */

#ifdef CONFIG_DEBUG
#define DEBUG                       1
#endif

/*
 * High Level Configuration Options
 */

#define CONFIG_SYS_NAND_BASE                            0x2F0350000

#if defined(CONFIG_USB_EHCI_NVTIVOT) || defined(CONFIG_USB_EHCI_NVTIVOT_A64)
#define CONFIG_EHCI_IS_TDI
#endif
/*-----------------------------------------------------------------------
 * IP address configuration
 */
#ifdef CONFIG_NOVATEK_MAC_ENET
#define CFG_ETHNET
#endif

#define FIXED_ETH_PARAMETER

#ifdef FIXED_ETH_PARAMETER
#ifdef CFG_ETHNET
#define CFG_ETHADDR				{0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x01}
#define CFG_ETH1ADDR				{0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x02}
#define CONFIG_IPADDR				192.168.1.99	/* Target IP address */
#define CONFIG_NETMASK				255.255.255.0
#define CONFIG_SERVERIP				192.168.1.11	/* Server IP address */
#define CONFIG_GATEWAYIP			192.168.1.254
#define CONFIG_HOSTNAME				"soclnx"
#endif
#endif

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
 * NVT LED CONFIG
 *
 * LED GPIO selection
 * C_GPIO(x)
 * P_GPIO(x)
 * S_GPIO(x)
 * L_GPIO(x)
 * D_GPIO(x)
 * Duration Unit: ms
 */
#ifdef CONFIG_NVT_FW_UPDATE_LED
#ifdef CONFIG_NVT_PWM
#define PWM_SIGNAL_NORMAL 0
#define PWM_SIGNAL_INVERT 1
#define NVT_PWMLED (PWMID_0 | PWMID_1)
#define PWM_SIGNAL_TYPE PWM_SIGNAL_INVERT
#define	PWM_LED_ERASE 50
#define	PWM_LED_PROGRAM 5
#else
#define NVT_LED_PIN P_GPIO(12)
#define NVT_LED_ERASE_DURATION 30
#define NVT_LED_PROGRAM_DURATION 10
#endif
#endif

#ifdef CONFIG_USB_GADGET_NVTIVOT
#define CONFIG_USBD_HS
#endif


/*
 * SATA Configuration
 */
#ifdef CONFIG_AHCI_NVT  // CONFIG_AHCI_NVT

#ifndef CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SCSI_AHCI_PLAT
#endif
#ifndef CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI
#endif
#ifndef CONFIG_SYS_SCSI_MAX_SCSI_ID
#define CONFIG_SYS_SCSI_MAX_SCSI_ID    1
#endif
#ifndef CONFIG_SYS_SCSI_MAX_LUN
#define CONFIG_SYS_SCSI_MAX_LUN                1
#endif
#ifndef CONFIG_SYS_SCSI_MAX_DEVICE
#define CONFIG_SYS_SCSI_MAX_DEVICE     (CONFIG_SYS_SCSI_MAX_SCSI_ID * \
                                       CONFIG_SYS_SCSI_MAX_LUN)
#endif
#ifndef CONFIG_LIBATA
#define CONFIG_LIBATA
#endif

//#define CONFIG_CMD_SATA
//#define CONFIG_SYS_SATA_MAX_DEVICE CONFIG_SYS_SCSI_MAX_DEVICE
#ifndef CONFIG_CMD_SCSI
#define CONFIG_CMD_SCSI
#endif
#ifndef CONFIG_SCSI
#define CONFIG_SCSI
#endif
//#define CONFIG_DOS_PARTITION
/*
#define CONFIG_PARTITION_UUIDS
#define CONFIG_CMD_PART
*/
#ifndef CONFIG_FS_EXT4
#define CONFIG_FS_EXT4
#endif
#ifndef CONFIG_EXT4_WRITE
#define CONFIG_EXT4_WRITE
#endif
#ifndef CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4
#endif

#ifndef CONFIG_AHCI
#define CONFIG_AHCI
#endif
#ifndef CONFIG_DM_SCSI
#define CONFIG_DM_SCSI
#endif

#endif  // CONFIG_AHCI_NVT

/*
 * To include nvt memory layout
 */
#include "novatek/ns02401_v0_a64.h"

/*
 * Our DDR memory always starts at 0x00000000 and U-Boot shall have
 * relocated itself to higher in memory by the time this value is used.
 * However, set this to a 32MB offset to allow for easier Linux kernel
 * booting as the default is often used as the kernel load address.
 */
#define CONFIG_STANDALONE_LOAD_ADDR			0x1A000000

/* We set the max number of command args high to avoid HUSH bugs. */
#define CONFIG_SYS_MAXARGS				64

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE				1024
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE				(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE				CONFIG_SYS_CBSIZE
										/* even with bootdelay=0 */
/* MTD */
#define CONFIG_SYS_BOOTM_LEN				(25 << 20)

/* DFU */
#define CONFIG_SET_DFU_ALT_BUF_LEN      (SZ_1K)

#if (defined(CONFIG_CMD_DFU) && defined(_EMBMEM_EMMC_))
#define CONFIG_DFU_MMC  1
#ifndef CONFIG_FS_EXT4
#define CONFIG_FS_EXT4  1
#endif
#ifndef CONFIG_EXT4_WRITE
#define CONFIG_EXT4_WRITE   1
#endif

/* EMMC partition */
#if defined(_NVT_ROOTFS_TYPE_RAMDISK_)
#define DFU_ALT_INFO \
		"loader raw 0 0x10000 mmcpart 1;" \
		"fdt part 2 1;" \
		"atf part 2 3;" \
		"uboot part 2 4;" \
		"linux part 2 6;" \
		"rootfs part 2 7;" \
		"rootfs1 ext4 2 8;" \
		"rootfs2 ext4 2 9"
#else
#define DFU_ALT_INFO \
		"loader raw 0 0x10000 mmcpart 1;" \
		"fdt part 2 1;" \
		"atf part 2 3;" \
		"uboot part 2 4;" \
		"linux part 2 6;" \
		"rootfs ext4 2 7;" \
		"rootfs1 part 2 8;" \
		"rootfs2 ext4 2 9"
#endif
#endif


#if (defined(CONFIG_CMD_DFU) && defined(CONFIG_NVT_SPI_NAND))
#define CONFIG_DFU_NAND 1
#define CONFIG_DFU_NAND_TRIMFFS 1

/* NAND partition */

#if defined(CONFIG_NVT_IVOT_OPTEE_SUPPORT)
/* For OPTEE */
#define DFU_ALT_INFO \
			"loader raw 0x0 0x40000;" \
			"fdt part 0 1;" \
			"atf part 0 2;" \
			"optee part 0 3;" \
			"uboot part 0 4;" \
			"linux part 0 6;" \
			"rootfs partubi 0 7;" \
			"rootfs1 partubi 0 8;" \
			"rootfs2 part 0 9"

#else
#if defined(_NVT_ROOTFS_TYPE_NAND_SQUASH_)
#define DFU_ALT_INFO \
			"loader raw 0x0 0x40000;" \
			"fdt part 0 1;" \
			"atf part 0 2;" \
			"uboot part 0 3;" \
			"linux part 0 5;" \
			"rootfs partubi 0 6;" \
			"rootfs1 partubi 0 7;" \
			"app partubi 0 8"
#elif defined(_NVT_ROOTFS_TYPE_NAND_SQUASH_NEW_)
#define DFU_ALT_INFO \
			"loader raw 0x0 0x40000;" \
			"fdt part 0 1;" \
			"atf part 0 2;" \
			"uboot part 0 3;" \
			"linux part 0 5;" \
			"rootfs part 0 6;" \
			"app partubi 0 7"
#else
#define DFU_ALT_INFO \
			"loader raw 0x0 0x40000;" \
			"fdt part 0 1;" \
			"atf part 0 2;" \
			"uboot part 0 3;" \
			"linux part 0 5;" \
			"rootfs part 0 6;" \
			"rootfs1 partubi 0 7;" \
			"app partubi 0 8"
#endif
#endif
#endif

#if (defined(CONFIG_CMD_DFU) && defined(CONFIG_NVT_SPI_NOR))
#define CONFIG_DFU_SF   1

/* NOR partition */
#define DFU_ALT_INFO \
			"loader raw 0x0 0x10000;" \
			"fdt part 0 1;" \
			"atf part 0 2;" \
			"uboot part 0 3;" \
			"linux part 0 5;" \
			"rootfs part 0 6;" \
			"app part 0 7"

#endif

#if (defined(CONFIG_CMD_DFU) && defined(CONFIG_NVT_SPI_NOR_NAND))
#define CONFIG_DFU_NAND 1
#define CONFIG_DFU_NAND_TRIMFFS 1
#define CONFIG_DFU_SF   1


/* NOR partition */
#define DFU_ALT_INFO \
			"loader raw 0x0 0x10000;" \
			"fdt part 0 1;" \
			"atf part 0 2;" \
			"uboot part 0 3"

/* NAND partition */
#define DFU_ALT_INFO_NAND \
			"linux part 0 1;" \
			"rootfs part 0 2;" \
			"rootfs1 partubi 0 3;" \
			"app partubi 0 4"
#endif

#if (defined(CONFIG_CMD_DFU) && defined(CONFIG_DFU_RAM))
#define DFU_ALT_INFO_RAM \
			"all-in-one ram 0x14f00000 0x7800000"

#endif
#ifdef CONFIG_CMD_DFU
#define CONFIG_THOR_RESET_OFF
#endif

#ifndef CONFIG_SYS_LOAD_ADDR
	#define CONFIG_SYS_LOAD_ADDR (CONFIG_SYS_TEXT_BASE - 0x1900000)
#endif

#endif /* __CONFIG_NS02401_V0_H */
