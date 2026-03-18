/**
    NVT common header file
    Define common functions
    @file       nvt_common.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __ARCH_COMMON_NVT_COMMON_H__
#define __ARCH_COMMON_NVT_COMMON_H__

#include <asm/io.h>
#include <asm/nvt-common/modelext/emb_partition_info.h>
#include <asm/nvt-common/nvt_types.h>
#include <common.h>
#include <image-sparse.h>
#include <asm/gpio.h>

extern unsigned long nvt_tm0_cnt_beg;
extern unsigned long nvt_tm0_cnt_end;
extern unsigned long nvt_shminfo_comm_uboot_boot_func;
extern unsigned long nvt_shminfo_comm_core1_start;
extern unsigned long nvt_shminfo_comm_core2_start;
extern unsigned long nvt_shminfo_comm_fw_update_addr;
extern unsigned long nvt_shminfo_comm_fw_update_len;
extern unsigned int nvt_shminfo_boot_fdt_addr;
extern unsigned long nvt_ramdisk_addr;
extern unsigned long nvt_ramdisk_size;
extern unsigned long nvt_ker_img_addr;
extern unsigned long nvt_ker_img_size;
extern uint8_t *nvt_fdt_buffer;
extern void nvt_rtc_init(void);
extern char *nvt_bin_name;
extern char *nvt_bin_name_t;
extern char *nvt_bin_name_r;
extern char *nvt_bin_name_modelext;
extern u32 nvt_flash_ecc_err_pages[100];
extern u32 uart_disable_anchor;
extern u32 rtos_disable_anchor;
extern EMB_PARTITION_FDT_TRANSLATE_TABLE emb_part_fdt_map[EMBTYPE_TOTAL_SIZE];
extern EMB_PARTITION emb_partition_info_data_curr[EMB_PARTITION_INFO_COUNT];
extern EMB_PARTITION emb_partition_info_data_new[EMB_PARTITION_INFO_COUNT];

enum {
	MEMTYPE_DRAM,
	MEMTYPE_CORE2ENTRY1,
	MEMTYPE_CORE2ENTRY2,
	MEMTYPE_FDT,
	MEMTYPE_SHMEM,
	MEMTYPE_LOADER,
	MEMTYPE_SHMEM_AMP,
	MEMTYPE_ATF,
	MEMTYPE_TEEOS,
	MEMTYPE_NSMEM,
	MEMTYPE_LINUXTMP,
	MEMTYPE_UBOOT,
	MEMTYPE_RTOS,
	MEMTYPE_BRIDGE,
	MEMTYPE_LINUX,
	MEMTYPE_ALL_IN_ONE,
	MEMTYPE_KERNEL_IMG,
	MEMTYPE_DSP,
	MEMTYPE_RTOS_S0,
	MEMTYPE_RTOS_S1,
	MEMTYPE_RTOS_S2,
	MEMTYPE_LOGO_FB,
	MEMTYPE_LOGO2_FB,
	MEMTYPE_SIZE,
};

#define MEMTYPE_TOTAL_SIZE (MEMTYPE_SIZE)
typedef struct _EMB_MEM_FDT_TRANSLATE_TABLE {
	char fdt_node_name[30];
	unsigned short emb_type;
} EMB_MEM_FDT_TRANSLATE_TABLE;

typedef struct _nvt_mem_region {
	unsigned long addr;
	unsigned long size;
} nvt_mem_region;

extern EMB_MEM_FDT_TRANSLATE_TABLE emb_mem_fdt_map[MEMTYPE_TOTAL_SIZE];
extern nvt_mem_region nvt_memory_cfg[MEMTYPE_TOTAL_SIZE];

int nvt_get_memory_by_dts(const void *fdt_addr);
void nvt_print_dram_setting(void);

/* print color setting */
#define ANSI_COLOR_RED			"\x1b[31m"
#define ANSI_COLOR_GREEN		"\x1b[32m"
#define ANSI_COLOR_YELLOW		"\x1b[33m"
#define ANSI_COLOR_BLUE			"\x1b[34m"
#define ANSI_COLOR_MAGENTA		"\x1b[35m"
#define ANSI_COLOR_CYAN			"\x1b[36m"
#define ANSI_COLOR_WHITE		"\x1b[37m"
#define ANSI_COLOR_RESET		"\x1b[0m"

/* RTOS codeinfo */
#define RTOS_CODEINFO_OFFSET		0x3F0
#if defined(__aarch64__)
#define RTOS_CODEINFO_SUB_ZI_LIMIT	0x18 	//(image_general_zi_zi_limit,uboot_end)
#else
#define RTOS_CODEINFO_SUB_ZI_LIMIT	0x14 	//(image_general_zi_zi_limit,uboot_end)
#endif
/* Error code for update all-in-one */
#define ERR_NVT_UPDATE_FAILED		-1
#define ERR_NVT_UPDATE_OPENFAILED	-2
#define ERR_NVT_UPDATE_READ_FAILED	-3
#define ERR_NVT_UPDATE_NO_NEED		-4

/*Bininfo boot info. */
#define COMM_UBOOT_BOOT_FUNC_BOOT_UPDFIRM	1
#define COMM_UBOOT_BOOT_FUNC_BOOT_UPD_FRM_USB	2
#define COMM_UBOOT_BOOT_FUNC_BOOT_UPD_FRM_ETH	3
#define COMM_UBOOT_BOOT_FUNC_BOOT_FORMAT_ROOTFS	4
#define COMM_UBOOT_BOOT_FUNC_BOOT_RECOVERY_SYS	5
#define COMM_UBOOT_BOOT_FUNC_BOOT_REASON_MASK	0xff << 0
#define COMM_UBOOT_BOOT_FUNC_BOOT_DONE		1 << 8
#define COMM_UBOOT_BOOT_FUNC_BOOT_NG		2 << 8
#define COMM_UBOOT_BOOT_FUNC_BOOT_DONE_MASK	0xff << 8
#define COMM_UBOOT_BOOT_FUNC_LOAD_DSP_MASK	 0xff << 16
#define COMM_UBOOT_BOOT_FUNC_LOAD_DSP_DONE		1 << 16
#define COMM_UBOOT_BOOT_FUNC_LOAD_DSP_NG		2 << 16

#define nvt_readb(offset)			readb(offset)
#define nvt_readw(offset)			readw(offset)
#define nvt_readl(offset)			readl(offset)

#define nvt_writeb(val, offset)		writeb(val, offset)
#define nvt_writew(val, offset)		writew(val, offset)
#define nvt_writel(val, offset)		writel(val, offset)

#define NVT_FDT_MEM_SIZE			(SZ_1K * 10)
#define FDT_SIZE				SZ_128K

#ifdef CONFIG_NVT_IVOT_NVT_DBG
#define NVT_DBG_FATAL		0
#define NVT_DBG_ERR		1
#define NVT_DBG_WRN		2
#define NVT_DBG_UNIT		3
#define NVT_DBG_FUNC		4
#define NVT_DBG_IND		5
#define NVT_DBG_MSG		6
#define NVT_DBG_VALUE		7
#define NVT_DBG_USER		8

#define nvt_dbg(level, fmt, args...)										\
do {														\
	if (unlikely(NVT_DBG_##level <= CONFIG_NVT_IVOT_DEBUG_LEVEL)) {						\
		if (NVT_DBG_##level < NVT_DBG_WRN)								\
			printf("%s %s: %s" fmt, ANSI_COLOR_RED, __func__, ANSI_COLOR_RESET, ##args);		\
		else if (NVT_DBG_##level < NVT_DBG_IND)								\
			printf("%s %s: %s" fmt, ANSI_COLOR_YELLOW, __func__, ANSI_COLOR_RESET, ##args);		\
		else												\
			printf("%s: " fmt, __func__, ##args);							\
	}													\
} while (0)
#else
#define nvt_dbg(level, fmt, args...)				\
do {								\
} while (0)
#endif

typedef enum
{
	NVT_BIN_NAME_TYPE_FW = 1,
	NVT_BIN_NAME_TYPE_RUNFW,
	NVT_BIN_NAME_TYPE_RECOVERY_FW,
	NVT_BIN_NAME_TYPE_MODELEXT,
} NVT_BIN_NAME_TYPE;

typedef struct _FASTBOOTPSTORE
{
	UINT32 uiDataSize;	///<section size (without header)
	UINT32 uiDataAddr;	///<section addr
	UINT32 uiCheckSum1;	///<make sure this header is intact. value = (uiDataSize | uiDataAddr)
	UINT32 uiCheckSum2;	///<make sure section buffer is intact
} FASTBOOTPSTORE, *PFASTBOOTPSTORE;

int nvt_fw_update(bool firm_upd_firm);
int nvt_chk_all_in_one_valid(unsigned short EmbType, unsigned int addr, unsigned int size, unsigned int id);
int nvt_fw_load_tbin(void);
int nvt_process_all_in_one(ulong addr, unsigned int size, int firm_upd_firm);
int nvt_fdt_init(bool reload);
int nvt_check_isfdt(ulong addr);
int nvt_chk_mtd_fdt_is_null(void);
int nvt_board_init(void);
int nvt_board_init_early(void);
int nvt_detect_fw_tbin(void);
void nvt_modelext_bininfo_init(void);
void nvt_ipc_init(void);
unsigned long get_nvt_timer0_cnt(void);
void lz_uncompress(unsigned char *in, unsigned char *out, unsigned int insize);
int get_part_and_flash_type(const char *partname, loff_t *off, loff_t *maxsize, nvt_flash_type_t* flash_type);
#define get_part( partname_ptr, off_ptr, maxsize_ptr) get_part_and_flash_type( partname_ptr, off_ptr, maxsize_ptr, NULL)
void nvt_disable_mem_protect(void);
int nvt_part_config(char *p_cmdline, EMB_PARTITION *partition_ptr);
char *get_nvt_bin_name(NVT_BIN_NAME_TYPE type);
int nvt_check_should_rename_updfw(void);
int nvt_rename_updfw(int reverse);
int nvt_create_delfw(void);
int nvt_delete_delfw(void);
int nvt_process_rootfs_format(void);
int nvt_check_is_fw_update_fw(void);
int nvt_check_is_fomat_rootfs(void);
int nvt_fs_set_blk_dev(void);
int nvt_process_sys_recovery(void);
int nvt_flash_mark_bad(void);
void nvt_shminfo_init(void);
int nvt_check_isfdt(ulong addr);
int nvt_enum_fdt_props(ulong fdt_addr, char *part_node_name, void *pdata,
			int (*enum_funcs)(ulong fdt_addr, const char *node_name, const char *prop_name, char *prop_val, void *pdata));
int nvt_fdt_embpart_lookup(char *part_node_name, unsigned short *embtype);
/* To get partition table from dts. */
int nvt_getfdt_emb(ulong fdt_addr, EMB_PARTITION *pemb);
int nvt_getfdt_rootfs_mtd_num(ulong fdt_addr, unsigned int *mtd_num, uint32_t *ro_attr);
int nvt_dts_config_parsing(void);
int nvt_sparse_image_update(u64 addr, u64 off, u64 size, u64 part_size);
int nvt_sparse_image_readback(u64 addr, u64 off, u64 size, u64 part_size);

extern int nvt_ivot_hw_init(void);
extern int nvt_ivot_hw_init_early(void);
extern void nvt_ivot_reset_cpu(void);
extern int nvt_ivot_set_cpuclk(void);
#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT

int nvt_dts_optee_nsmem(unsigned long* addr, unsigned long* size);

#endif

#ifdef CONFIG_DFU_MMC
lbaint_t nvt_mmc_sparse_write(struct sparse_storage *info, lbaint_t blk, lbaint_t blkcnt, const void *buffer);
lbaint_t nvt_mmc_sparse_read(struct sparse_storage *info, lbaint_t blk, lbaint_t blkcnt, const void *buffer);
lbaint_t nvt_mmc_sparse_reserve(struct sparse_storage *info, lbaint_t blk, lbaint_t blkcnt);
void nvt_mmc_sparse_msg_print(const char *str, char *response);
#endif

/*** NAND Util ***/
uint32_t nand_get_page_size(void);
uint32_t nand_get_block_size(void);
uint32_t nand_total_blocks(void);

/*** DDR Util ***/
#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64) || defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64) || defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64) || defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64) || defined(CONFIG_TARGET_NS02302_A64))
void nvt_ddr_scan(u32 type);
#else
void nvt_ddr_scan(void);
#endif
void nvt_stbc_pmc(UINT32 debug);

#if defined(CONFIG_NVT_IVOT_DUAL_RTOS)
extern int nvt_decide_dual_rtos_idx(void);
extern void nvt_update_dual_rtos_info(void *p_bininfo, int boot_bank);
#endif

#ifdef CONFIG_NVT_IVOT_SECBOOT_SUPPORT
int nvt_decrypt_aes_on_uboot(unsigned long addr);
int nvt_chk_signature_on_uboot(ulong addr, unsigned int size);
#elif defined(CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT)
int nvt_decrypt_aes(unsigned long addr);
int nvt_chk_signature(ulong addr, unsigned int size);
#endif

#endif /* __ARCH_COMMON_NVT_COMMON_H__ */
