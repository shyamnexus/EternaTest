#include <stdio.h>
#include <stdlib.h>
#include <strg_def.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include <kwrap/debug.h>
#include <comm/shm_info.h>
#include <plat/top.h>
#include <plat/pll.h>
#include "prjcfg.h"
#include "sys_mempool.h"
#include "sys_storage_partition.h"

#if 0
/*******************************************************
 * User defined SPI NAND sample code
 ******************************************************/
#define SPI_NAND(MfgID, MemType, MemCapacity, TotalSize, SectorSize, BlockSize) \
{   .uiMfgID = (MfgID), \
    .uiMemType = (MemType), \
    .uiMemCapacity = (MemCapacity), \
    .uiTotalSize = (TotalSize), \
    .uiSectorSize = (SectorSize), \
    .uiBlockSize = (BlockSize), } \

SPI_FLASH_INFO nvt_nand_ids[] = {
	SPI_NAND(0xEF, 0xAA, 0x23, 0x20000000, 0x800, 0x20000),
};

static BOOL pmc_identifySpiNand(UINT32 uiMfgID, UINT32 uiTypeID, UINT32 uiThirdID, PSPIFLASH_IDENTIFY pIdentify)
{
    UINT32  ui_nand_ids;
    BOOL    nand_ids_found = FALSE;

	 for (ui_nand_ids = 0; ui_nand_ids < (sizeof(nvt_nand_ids) / sizeof(SPI_FLASH_INFO)); ui_nand_ids++) {
		// For SPI Nand flash only
		if (uiMfgID == nvt_nand_ids[ui_nand_ids].uiMfgID && uiTypeID == nvt_nand_ids[ui_nand_ids].uiMemType) {
		    if((!nvt_nand_ids[ui_nand_ids].uiMemCapacity) ||
				(nvt_nand_ids[ui_nand_ids].uiMemCapacity && (uiThirdID == nvt_nand_ids[ui_nand_ids].uiMemCapacity))) {
				pIdentify->uiSectorSize = nvt_nand_ids[ui_nand_ids].uiSectorSize;
				pIdentify->uiBlockSize = nvt_nand_ids[ui_nand_ids].uiBlockSize;
				pIdentify->uiTotalSize = nvt_nand_ids[ui_nand_ids].uiTotalSize;
				nand_ids_found = TRUE;
				break;
			}
		}
    }
    return nand_ids_found;
}
#endif

#if 0
/*******************************************************
 * User defined SPI NOR sample code
 ******************************************************/
#define SPI_NOR(MfgID, MemType, MemCapacity, TotalSize, SectorSize, BlockSize, flashwidth) \
{   .uiMfgID = (MfgID), \
    .uiMemType = (MemType), \
    .uiMemCapacity = (MemCapacity), \
    .uiTotalSize = (TotalSize), \
    .uiSectorSize = (SectorSize), \
    .uiBlockSize = (BlockSize), \
    .flashWidth = (flashwidth), \
	.uiSectorCnt = (TotalSize) / (SectorSize), \
	.bSupportEWSR = FALSE, \
	.bSupportAAI = FALSE, \
	.bSupportSecErase = TRUE, \
    .uiBlockEraseTime = 4000, \
    .uiSectorEraseTime = 300, \
    .uiPageProgramTime = 5, } \

SPI_FLASH_INFO nvt_nor_ids[] = {
	SPI_NOR(0xC2, 0x20, 0x19, 0x2000000, 0x10000, 0x10000, SPI_FLASH_BUSWIDTH_QUAD_TYPE1),
};

static BOOL pmc_identifySpiNor(UINT32 uiMfgID, UINT32 uiTypeID, UINT32 uiThirdID, PSPIFLASH_IDENTIFY pIdentify)
{
    UINT32  ui_nand_ids;
    BOOL    nor_ids_found = FALSE;

	 for (ui_nand_ids = 0; ui_nand_ids < (sizeof(nvt_nor_ids) / sizeof(SPI_FLASH_INFO)); ui_nand_ids++) {
		// For SPI Nor flash only
		if (uiMfgID == nvt_nor_ids[ui_nand_ids].uiMfgID && uiTypeID == nvt_nor_ids[ui_nand_ids].uiMemType && uiThirdID == nvt_nor_ids[ui_nand_ids].uiMemCapacity) {
			pIdentify->uiSectorSize = nvt_nor_ids[ui_nand_ids].uiSectorSize;
			pIdentify->uiBlockSize = nvt_nor_ids[ui_nand_ids].uiBlockSize;
			pIdentify->uiTotalSize = nvt_nor_ids[ui_nand_ids].uiTotalSize;
			pIdentify->uiSectorCnt = nvt_nor_ids[ui_nand_ids].uiSectorCnt;
			pIdentify->flashWidth = nvt_nor_ids[ui_nand_ids].flashWidth;
			pIdentify->uiBlockEraseTime = nvt_nor_ids[ui_nand_ids].uiBlockEraseTime;
			pIdentify->uiSectorEraseTime = nvt_nor_ids[ui_nand_ids].uiSectorEraseTime;
			pIdentify->uiPageProgramTime = nvt_nor_ids[ui_nand_ids].uiPageProgramTime;
			nor_ids_found = TRUE;
			break;
		}
    }
    return nor_ids_found;
}
#endif

#define STRG_MEM_SIZE ALIGN_CEIL_32(_EMBMEM_BLK_SIZE_+(_EMBMEM_BLK_SIZE_>>2))

typedef enum _LDSTRG {
	LDSTRG_UNKNOWN = 0,
	LDSTRG_NAND,
	LDSTRG_SPI_NAND,
	LDSTRG_SPI_NOR,
	LDSTRG_EMMC,
	LDSTRG_COMBO,
	ENUM_DUMMY4WORD(LDSTRG)
} LDSTRG;

static void *strg_mem = NULL;
static SHMINFO *shm = NULL;

#if defined(_EMBMEM_COMBO_)
LDSTRG strg_type = LDSTRG_UNKNOWN;

static int init_partition_type(void)
{
	// check boot source
	BOOT_SRC_ENUM boot_src = (BOOT_SRC_ENUM)top_get_bs();

	switch (boot_src) {
	case BOOT_SRC_SPINAND:
	case BOOT_SRC_SPINAND_RS:
	case BOOT_SRC_SPINAND_4K:
	case BOOT_SRC_SPINAND_RS_4K:
		strg_type = LDSTRG_SPI_NAND;
		return 0;
	case BOOT_SRC_SPINOR:
		strg_type = LDSTRG_SPI_NOR;
		return 0;
	case BOOT_SRC_EMMC_4BIT:
	case BOOT_SRC_EMMC_8BIT:
		strg_type = LDSTRG_EMMC;
		return 0;
	default:
		break; // use LdStorage to indicate major flash type
	}

	strg_type = shm->boot.LdStorage;
	return 0;
}

PSTORAGE_OBJ storage_partition_getObject(STRG_OBJ_ID strgObjID)
{
	switch (strg_type) {
	case LDSTRG_SPI_NOR:
		return spiflash_getStorageObject(strgObjID);
	case LDSTRG_SPI_NAND:
		return nand_getStorageObject(strgObjID);
	case LDSTRG_EMMC:
		return sdio3_getStorageObject(strgObjID);
	default:
		DBG_ERR("unsupported strg_type: %d\n", strg_type);
		return NULL;
	}
}

char *storage_partition_make_fdt_path(char *name)
{
	static char path[64] = { 0 };

	switch (strg_type) {
	case LDSTRG_SPI_NOR:
		snprintf(path, sizeof(path) - 1, "%s%s", "/nor/partition_", name);
		break;
	case LDSTRG_SPI_NAND:
		snprintf(path, sizeof(path) - 1, "%s%s", "/nand/partition_", name);
		break;
	case LDSTRG_EMMC:
		snprintf(path, sizeof(path) - 1, "%s%s", "mmc2/partition_", name);
		break;
	default:
		DBG_ERR("unsupported strg_type: %d\n", strg_type);
		return NULL;
	}
	return path;
}
#endif


static int init_partition(STRG_OBJ_ID strg_id, const char *fdt_path)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	UINT32 blksize;
	unsigned long long partition_ofs, partition_size;

	nodeoffset = fdt_path_offset(p_fdt, fdt_path);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", fdt_path, nodeoffset);
		return -1;
	}

	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return -1;
	} else {
		unsigned long long *p_data = (unsigned long long *)nodep;
		partition_ofs = be64_to_cpu(p_data[0]);
		partition_size = be64_to_cpu(p_data[1]);
		//DBG_DUMP("partition reg = <0x%llX 0x%llX> \n", partition_ofs, partition_size);
	}


	STORAGE_OBJ *pStrg = EMB_GETSTRGOBJ(strg_id);
	if (pStrg == NULL) {
		DBG_ERR("pStrg is NULL.\n");
		return -1;
	}

	//for user defined spi nand
	//pStrg->ExtIOCtrl(STRG_EXT_CMD_SPI_IDENTIFY_CB, (ULONG)&pmc_identifySpiNand, (ULONG)NULL);

	//for user defined spi nor
	//pStrg->ExtIOCtrl(STRG_EXT_CMD_SPI_IDENTIFY_CB, (ULONG)&pmc_identifySpiNor, (ULONG)NULL);


	pStrg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&blksize, 0); //get block size
#if defined(_EMBMEM_COMBO_)
	if ((blksize > _EMBMEM_BLK_SIZE_) || (_EMBMEM_BLK_SIZE_ % blksize) != 0) {
		DBG_ERR("combo-storage block size is not match. %X(driver) : %X(dts)\n", (int)blksize, (int)_EMBMEM_BLK_SIZE_);
		return -1;
	}
#else
	if (blksize != _EMBMEM_BLK_SIZE_) {
		DBG_ERR("storage block size is not match. %X(driver) : %X(dts)\n", (int)blksize, (int)_EMBMEM_BLK_SIZE_);
		return -1;
	}
#endif
	pStrg->SetParam(STRG_SET_MEMORY_REGION, (ULONG)strg_mem, STRG_MEM_SIZE);
	pStrg->SetParam(STRG_SET_PARTITION_SECTORS, (UINT32)(partition_ofs / blksize), (UINT32)(partition_size / blksize));

#if defined(_EMBMEM_SPI_NOR_)
	if ((shm->boot.LdCtrl2 & LDCF_DTR80_BOOT)) {
		if (pStrg->ExtIOCtrl(STRG_EXT_CMD_SPI_DTR, 1, 0) != 0) {
			DBG_ERR("failed to STRG_EXT_CMD_SPI_DTR\n");
			return -1;
		}
	}
#endif
	return 0;
}

#if defined(_EMBMEM_EMMC_) || defined(_EMBMEM_COMBO_)
BOOL emmc_card_detect(void)
{
	return TRUE;
}

BOOL emmc_card_wp(void)
{
	return FALSE;
}

static void emmc_set_freq(int MHz)
{
	sdio3_setConfig(SDIO_CONFIG_ID_MMC_FAST_BOOT, TRUE);
	switch (MHz) {
	case 48:
		sdio3_setConfig(SDIO_CONFIG_ID_BUS_SPEED_MODE, SDIO_BUS_SPEED_MODE_DS | SDIO_BUS_SPEED_MODE_HS);
		sdio3_setConfig(SDIO_CONFIG_ID_DS_DRIVING, 3); //level 3
		sdio3_setConfig(SDIO_CONFIG_ID_HS_DRIVING, 3); //level 3
		sdio3_setConfig(SDIO_CONFIG_ID_DRIVING_SINK_EN, TRUE);
		sdio3_setConfig(SDIO_CONFIG_ID_BUS_WIDTH, SD_HOST_BUS_4_BITS);
		break;
	case 96:
		sdio3_setConfig(SDIO_CONFIG_ID_BUS_SPEED_MODE, SDIO_BUS_SPEED_MODE_DS | SDIO_BUS_SPEED_MODE_HS | SDIO_BUS_SPEED_MODE_HS200);
		sdio3_setConfig(SDIO_CONFIG_ID_DS_DRIVING, 3); //level 3
		sdio3_setConfig(SDIO_CONFIG_ID_HS_DRIVING, 3); //level 3
		sdio3_setConfig(SDIO_CONFIG_ID_HS200_DRIVING, 3); //level 3
		sdio3_setConfig(SDIO_CONFIG_ID_DRIVING_SINK_EN, TRUE);
		sdio3_setConfig(SDIO_CONFIG_ID_BUS_WIDTH, SD_HOST_BUS_8_BITS/*SD_HOST_BUS_8_BITS*/);
		break;
	case 120:
		sdio3_setConfig(SDIO_CONFIG_ID_BUS_SPEED_MODE, SDIO_BUS_SPEED_MODE_DS | SDIO_BUS_SPEED_MODE_HS | SDIO_BUS_SPEED_MODE_HS400);
		sdio3_setConfig(SDIO_CONFIG_ID_BUS_WIDTH, SD_HOST_BUS_8_BITS);
		sdio3_setConfig(SDIO_CONFIG_ID_SRCLK, SDIO_SRC_CLK_PLL6);
		sdio3_setConfig(SDIO_CONFIG_ID_HS400_MAX_CLK, 120 * 1000000);
		break;
	default:
		DBG_ERR("unsupported emmc %d MHz.\n", MHz);
	}
}
#endif
static int init_shm(void)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	// read SHMEM_PATH
	nodeoffset = fdt_path_offset(p_fdt, SHMEM_PATH);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", SHMEM_PATH, nodeoffset);
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return -1;
	}

	unsigned int *p_data = (unsigned int *)nodep;
	shm = (SHMINFO *)(uintptr_t)be32_to_cpu(p_data[0]);

	return 0;
}
static int init_configs(void)
{
	int er;

	if ((er = init_shm()) != 0) {
		return er;
	}
#if defined(_EMBMEM_SPI_NAND_)
	nand_setConfig(NAND_CONFIG_ID_NAND_TYPE, NANDCTRL_SPI_NAND_TYPE);
#elif defined(_EMBMEM_SPI_NOR_)
	nand_setConfig(NAND_CONFIG_ID_NAND_TYPE, NANDCTRL_SPI_NOR_TYPE);
#elif defined(_EMBMEM_EMMC_)
	sdio3_setCallBack(SDIO_CALLBACK_CARD_DETECT, (SDIO_CALLBACK_HDL)emmc_card_detect);
	sdio3_setCallBack(SDIO_CALLBACK_WRITE_PROTECT, (SDIO_CALLBACK_HDL)emmc_card_wp);
	emmc_set_freq(120);
#elif defined(_EMBMEM_COMBO_)
	if (init_partition_type() != 0) {
		return -1;
	}
	switch (strg_type) {
	case LDSTRG_SPI_NOR:
		nand_setConfig(NAND_CONFIG_ID_NAND_TYPE, NANDCTRL_SPI_NOR_TYPE);
		break;
	case LDSTRG_SPI_NAND:
		nand_setConfig(NAND_CONFIG_ID_NAND_TYPE, NANDCTRL_SPI_NAND_TYPE);
		break;
	case LDSTRG_EMMC:
		sdio3_setCallBack(SDIO_CALLBACK_CARD_DETECT, (SDIO_CALLBACK_HDL)emmc_card_detect);
		sdio3_setCallBack(SDIO_CALLBACK_WRITE_PROTECT, (SDIO_CALLBACK_HDL)emmc_card_wp);
		emmc_set_freq(120);
		break;
	default:
		DBG_ERR("unsupported strg_type: %d\n", strg_type);
		return -1;
	}
#endif
	if (strg_mem == NULL) {
		strg_mem = malloc(STRG_MEM_SIZE);
	}
	return 0;
}

int storage_partition_init1(void)
{
	init_configs();
	// setting partition info
	// init rtos partition only for fastboot
	init_partition(STRG_OBJ_FW_RTOS, PARTITION_PATH_RTOS);

	// for emmc, to reduce 400KHz init too many time, we open a dummy partition
#if (!POWERON_EXAM_FLASH_LOAD) // to avoid POWERON_EXAM_FLASH_LOAD use the same STRG_OBJ_FW_RSV7
#if defined(_EMBMEM_EMMC_)
	UINT32 totoal_sectors = 0;
	STORAGE_OBJ *pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_ALL);
	pStrg->SetParam(STRG_SET_MEMORY_REGION, (ULONG)strg_mem, STRG_MEM_SIZE);
	pStrg->Open(); //for initial emmc speed up, should open first, and then set partition
	pStrg->GetParam(STRG_GET_DEVICE_PHY_SECTORS, (ULONG)&totoal_sectors, 0);
	if (totoal_sectors == 0) {
		DBG_ERR("totoal_sectors couldn't be zero\n");
		return -1;
	}
	pStrg->SetParam(STRG_SET_PARTITION_SECTORS, 0, totoal_sectors);
#elif defined(_EMBMEM_COMBO_)
	if (init_partition_type() == LDSTRG_EMMC) {
		UINT32 totoal_sectors = 0;
		STORAGE_OBJ *pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_ALL);
		pStrg->SetParam(STRG_SET_MEMORY_REGION, (ULONG)strg_mem, STRG_MEM_SIZE);
		pStrg->Open(); //for initial emmc speed up, should open first, and then set partition
		pStrg->GetParam(STRG_GET_DEVICE_PHY_SECTORS, (ULONG)&totoal_sectors, 0);
		if (totoal_sectors == 0) {
			DBG_ERR("totoal_sectors couldn't be zero\n");
			return -1;
		}
		pStrg->SetParam(STRG_SET_PARTITION_SECTORS, 0, totoal_sectors);
	}
#else
	init_partition(STRG_OBJ_FW_ALL, PARTITION_PATH_ALL);
#endif
#endif
	return 0;
}

int storage_partition_init2(void)
{
	// init others partitions after fastboot
	init_partition(STRG_OBJ_FW_FDT, PARTITION_PATH_FDT);
	init_partition(STRG_OBJ_FW_APP, PARTITION_PATH_APP);
#if defined(_BOOT_OS_RTOS_BOOT_LINUX_)
	init_partition(STRG_OBJ_FW_LINUX, PARTITION_PATH_LINUX);
	init_partition(STRG_OBJ_FW_ROOTFS, PARTITION_PATH_ROOTFS);
#endif
	return 0;
}

int storage_partition_read_part(const char *pathname, unsigned char *buff, int offset, int size)
{
	int er;
	UINT32 blksize;
	STORAGE_OBJ *pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_ALL);

	if (pStrg == NULL) {
		DBG_ERR("failed to get STRG_OBJ_FW_ALL");
		return -1;
	}

	pStrg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&blksize, 0); //get block size

	if ((offset % blksize) != 0) {
		DBG_ERR("block offset must be block alignment.");
		return -1;
	}

	if ((size % blksize) != 0) {
		DBG_ERR("buf size must be block alignment.");
		return -1;
	}

	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */
	unsigned long long partition_ofs, partition_size;

	nodeoffset = fdt_path_offset(p_fdt, pathname);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", pathname, nodeoffset);
		return -1;
	}

	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return -1;
	} else {
		unsigned long long *p_data = (unsigned long long *)nodep;
		partition_ofs = be64_to_cpu(p_data[0]);
		partition_size = be64_to_cpu(p_data[1]);
	}

	if (offset + size > (int)partition_size) {
		DBG_ERR("flash access is out of range\n");
		return -1;
	}

	// for spi-nand, because bad block existing any partition, we cannot map whole flash partition to access partition data
	// for example, partition[2] offset is 0x80000, but there is bad block on 0x20000
	// if RdSector access 0x80000, the data will be on 0xa0000
#if defined(_EMBMEM_SPI_NAND_)
	pStrg->Lock();
	pStrg->SetParam(STRG_SET_PARTITION_SECTORS, (UINT32)partition_ofs / blksize, (UINT32)partition_size / blksize);
	pStrg->Open();
	er = pStrg->RdSectors((INT8 *)buff, (UINT32)offset / blksize, (UINT32)size / blksize);
	pStrg->Close();
	pStrg->Unlock();
#elif defined(_EMBMEM_COMBO_)
	if (init_partition_type() == LDSTRG_SPI_NAND) {
		pStrg->Lock();
		pStrg->SetParam(STRG_SET_PARTITION_SECTORS, (UINT32)partition_ofs / blksize, (UINT32)partition_size / blksize);
		pStrg->Open();
		er = pStrg->RdSectors((INT8 *)buff, (UINT32)offset / blksize, (UINT32)size / blksize);
		pStrg->Close();
		pStrg->Unlock();
	} else {
		int complete_offset = (int)partition_ofs + offset;
		pStrg->Lock();
		pStrg->Open();
		er = pStrg->RdSectors((INT8 *)buff, complete_offset / blksize, size / blksize);
		pStrg->Close();
		pStrg->Unlock();
	}
#else
	int complete_offset = (int)partition_ofs + offset;
	pStrg->Lock();
#if !defined(_EMBMEM_EMMC_)
	pStrg->Open(); // for emmc speed up, dont open, close again
#endif
	er = pStrg->RdSectors((INT8 *)buff, complete_offset / blksize, size / blksize);
#if !defined(_EMBMEM_EMMC_)
	pStrg->Close();
#endif
	pStrg->Unlock();
#endif

	return er;
}


int storage_partition_write_part(const char *pathname, unsigned char *buff, int offset, int size)
{
	int er;
	UINT32 blksize;
	STORAGE_OBJ *pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_ALL);

	if (pStrg == NULL) {
		DBG_ERR("failed to get STRG_OBJ_FW_ALL");
		return -1;
	}

	pStrg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&blksize, 0); //get block size

	if ((offset % blksize) != 0) {
		DBG_ERR("block offset must be block alignment.");
		return -1;
	}

#if defined(_EMBMEM_SPI_NAND_)
	if (offset != 0) {
		DBG_ERR("spi-nand not support offset write");
		return -1;
	}
#elif defined(_EMBMEM_COMBO_)
	if (init_partition_type() == LDSTRG_SPI_NAND) {
		DBG_ERR("spi-nand not support offset write");
		return -1;
	}
#endif

	if ((size % blksize) != 0) {
		DBG_ERR("buf size must be block alignment.");
		return -1;
	}

	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */
	unsigned long long partition_ofs, partition_size;

	nodeoffset = fdt_path_offset(p_fdt, pathname);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", pathname, nodeoffset);
		return -1;
	}

	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return -1;
	} else {
		unsigned long long *p_data = (unsigned long long *)nodep;
		partition_ofs = be64_to_cpu(p_data[0]);
		partition_size = be64_to_cpu(p_data[1]);
	}

	if (offset + size > (int)partition_size) {
		DBG_ERR("flash access is out of range\n");
		return -1;
	}

	// for spi-nand, because bad block existing any partition, we cannot map whole flash partition to access partition data
	// for example, partition[2] offset is 0x80000, but there is bad block on 0x20000
	// if RdSector access 0x80000, the data will be on 0xa0000
#if defined(_EMBMEM_SPI_NAND_)
	pStrg->Lock();
	pStrg->SetParam(STRG_SET_PARTITION_SECTORS, (UINT32)partition_ofs / blksize, (UINT32)partition_size / blksize);
	pStrg->Open();
	er = pStrg->WrSectors((INT8 *)buff, (UINT32)offset / blksize, (UINT32)size / blksize);
	pStrg->Close();
	pStrg->Unlock();
#elif defined(_EMBMEM_COMBO_)
	if (init_partition_type() == LDSTRG_SPI_NAND) {
		pStrg->Lock();
		pStrg->SetParam(STRG_SET_PARTITION_SECTORS, (UINT32)partition_ofs / blksize, (UINT32)partition_size / blksize);
		pStrg->Open();
		er = pStrg->WrSectors((INT8 *)buff, (UINT32)offset / blksize, (UINT32)size / blksize);
		pStrg->Close();
		pStrg->Unlock();
	} else {
		int complete_offset = (int)partition_ofs + offset;
		pStrg->Lock();
		pStrg->Open();
		er = pStrg->WrSectors((INT8 *)buff, complete_offset / blksize, size / blksize);
		pStrg->Close();
		pStrg->Unlock();
	}
#else
	int complete_offset = (int)partition_ofs + offset;
	pStrg->Lock();
#if !defined(_EMBMEM_EMMC_)
	pStrg->Open(); // for emmc speed up, dont open, close again
#endif
	er = pStrg->WrSectors((INT8 *)buff, complete_offset / blksize, size / blksize);
#if !defined(_EMBMEM_EMMC_)
	pStrg->Close();
#endif
	pStrg->Unlock();
#endif


	return er;
}