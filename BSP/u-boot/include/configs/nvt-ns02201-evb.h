/*
 * Copyright (C) 2016 Novatek Microelectronics Corp. All rights reserved.
 * Author: iVoT-IM <iVoT_MailGrp@novatek.com.tw>
 *
 * Configuration settings for the Novatek NS02201 SOC.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_NS02201_H
#define __CONFIG_NS02201_H

#include <linux/sizes.h>

/* #define CONFIG_DEBUG 1 */

#ifdef CONFIG_DEBUG
#define DEBUG                       1
#endif

/*
 * High Level Configuration Options
 */

#define CONFIG_SYS_NAND_BASE                            0xF0990000

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
 * DDR information.  If the CONFIG_NR_DRAM_BANKS is not defined,
 * we say (for simplicity) that we have 1 bank, always, even when
 * we have more.  We always start at 0x80000000, and we place the
 * initial stack pointer in our SRAM. Otherwise, we can define
 * CONFIG_NR_DRAM_BANKS before including this file.
 */

/*
 * To include nvt memory layout
 */
#include "novatek/ns02201_a32.h"
/*
 * Our DDR memory always starts at 0x00000000 and U-Boot shall have
 * relocated itself to higher in memory by the time this value is used.
 * However, set this to a 32MB offset to allow for easier Linux kernel
 * booting as the default is often used as the kernel load address.
 */
#define CONFIG_STANDALONE_LOAD_ADDR			0x1A000000
/* We set the max number of command args high to avoid HUSH bugs. */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE				(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE				CONFIG_SYS_CBSIZE
										/* even with bootdelay=0 */
/* MTD */
//#define CONFIG_CMD_MTDPARTS
#define CONFIG_SYS_BOOTM_LEN				(25 << 20)

#if (defined(CONFIG_CMD_DFU) && defined(_EMBMEM_EMMC_))
#define CONFIG_DFU_MMC  1
#ifndef CONFIG_FS_EXT4
#define CONFIG_FS_EXT4  1
#endif
#ifndef CONFIG_EXT4_WRITE
#define CONFIG_EXT4_WRITE   1
#endif
#define CONFIG_SYS_DFU_MAX_FILE_SIZE CONFIG_LINUX_SDRAM_SIZE

/* EMMC partition */
#define DFU_ALT_INFO \
			"rootfs ext4 2 1;" \
			"rootfs1 fat 2 2;" \
			"rootfs2 ext4 2 3;" \
			"mbr1 mbr 2 4;" \
			"rootfsl1 part 2 5"
#endif

#if (defined(CONFIG_CMD_DFU) && defined(CONFIG_NVT_SPI_NAND))
#define CONFIG_DFU_NAND 1
#define CONFIG_DFU_NAND_TRIMFFS 1

/* NAND partition */
#define DFU_ALT_INFO \
			"fdt part 0 1;" \
			"uboot part 0 2;" \
			"uenv part 0 3;" \
			"linux part 0 4;" \
			"rootfs0 partubi 0 5;" \
			"rootfs1 partubi 0 6;" \
			"app partubi 0 7"
#endif

#if (defined(CONFIG_CMD_DFU) && defined(CONFIG_NVT_SPI_NOR))
#define CONFIG_DFU_SF   1

/* NOR partition */
#define DFU_ALT_INFO \
			"fdt part 0 1;" \
			"uboot part 0 2;" \
			"uenv part 0 3;" \
			"linux part 0 4;" \
			"rootfs part 0 5;" \
			"app part 0 6"

#endif
#ifdef CONFIG_CMD_DFU
#define CONFIG_THOR_RESET_OFF
#endif

#define CONFIG_CMDLINE_TAG              1       /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS            1
#define CONFIG_INITRD_TAG               1
#define CONFIG_REVISION_TAG             1

#ifndef CONFIG_SYS_LOAD_ADDR
	#define CONFIG_SYS_LOAD_ADDR (CONFIG_SYS_TEXT_BASE - 0x1900000)
#endif

#define CONFIG_SYS_INIT_SP_ADDR CONFIG_SYS_TEXT_BASE

#endif /* __CONFIG_NS02201_H */
