#include <stdio.h>
#include <unistd.h>
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include <kwrap/task.h>
#include <kwrap/examsys.h>
#include <nvtpack.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include <lz.h>
#include <bin_info.h>
#include <shm_info.h>
#include "prjcfg.h"
#include "sys_fastboot.h"
#include "sys_storage_partition.h"
#include "flow_updfw.h"

#if !defined(_NVT_DUAL_RTOS_ON_)
// stub functions
int flow_updfw_init_bank_info(void)
{
	return 0;
}

int flow_updfw(unsigned char *p_nvtpack, unsigned int size, BOOL *p_abort, VERSION_CTRL *p_verctrl)
{
	DBG_ERR("not support to NVT_DUAL_RTOS_OFF\n");
	return 0;
}

#elif defined(_NVT_DUAL_RTOS_ON_)

static VERSION_CTRL m_empty_bank_info = {0};
VERSION_CTRL m_boot_bank_info = {0}; //also indicate fw is valid

const static int bank_map[2] = {STRG_OBJ_FW_BANK, STRG_OBJ_FW_BANK1};
const static int rtos_map[2] = {STRG_OBJ_FW_RTOS, STRG_OBJ_FW_RTOS1};

#if defined(_EMBMEM_SPI_NAND_)
#define FDT_NVTPACK_PREFIX "/nand/nvtpack/index"
#elif defined(_EMBMEM_SPI_NOR_)
#define FDT_NVTPACK_PREFIX "/nor/nvtpack/index"
#elif defined(_EMBMEM_EMMC_)
#define FDT_NVTPACK_PREFIX "/mmc@f0510000/nvtpack/index"
#else
#error "unsupport _EMBMEM_?"
#endif

static int flow_updfw_erase_bank_if_necessary(void)
{
	// check if current boot is by uboot update firmware
	SHMINFO *p_shm;
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
	} else {
		DBG_DUMP("offset for  %s = %d \n", SHMEM_PATH, nodeoffset);
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return 0;
	} else {
		unsigned int *p_data = (unsigned int *)nodep;
		p_shm = (SHMINFO *)be32_to_cpu(p_data[0]);
		DBG_DUMP("p_shm = 0x%08X\n", (int)p_shm);
	}

	if ((p_shm->boot.LdCtrl2 & LDCF_UPDATE_FW) == 0) {
		// normal boot, no need to erase
		return 0;
	}

	// erase 2 bank partitions
	int i, er = 0;
	for (i = 0; i < 2; i++) {
		STORAGE_OBJ *p_strg = EMB_GETSTRGOBJ(bank_map[i]);
		p_strg->Lock();
		if (p_strg->Open() == 0) {
			unsigned char *p_tmp = (unsigned char *)malloc(_EMBMEM_BLK_SIZE_);
			memset(p_tmp, 0, _EMBMEM_BLK_SIZE_);
			if (p_strg->WrSectors((INT8 *)p_tmp, 0, 1) != 0) {
				DBG_ERR("clean bank[%d] failed.\r\n", i);
				er = -1;
			} else {
				DBG_DUMP("bank[%d] cleaned.\r\n", i);
			}
			free(p_tmp);
			p_strg->Close();
		}
		p_strg->Unlock();
	}
	return er;
}

static void init_thread(void *p_param)
{
	fastboot_wait_done(BOOT_INIT_OTHERS);

	extern BININFO bin_info;
	int boot_bank_from = bin_info.Resv[BININFO_RESV_IDX_BOOT_BANK_FROM];

	DBG_DUMP("boot from bank[%d]\n", boot_bank_from);

	if (flow_updfw_erase_bank_if_necessary() != 0) {
		THREAD_RETURN(-1);
	}

	int er = 0;
	STORAGE_OBJ *p_strg = EMB_GETSTRGOBJ(bank_map[boot_bank_from]);
	p_strg->Lock();
	if (p_strg->Open() == 0) {
		unsigned char *p_tmp = (unsigned char *)malloc(_EMBMEM_BLK_SIZE_);
		if (p_strg->RdSectors((INT8 *)p_tmp, 0, 1) != 0) {
			DBG_ERR("load uenv failed.\r\n");
			er = -1;
		} else {
			unsigned int fourcc = ((VERSION_CTRL *)p_tmp)->fourcc;
			if (fourcc == MAKEFOURCC('V', 'E', 'R', 'C')) {
				memcpy(&m_boot_bank_info, (unsigned char *)p_tmp, sizeof(VERSION_CTRL));
			} else {
				if (memcmp(&m_empty_bank_info, p_tmp, sizeof(VERSION_CTRL)) != 0) {
					// if bank_info not empty and invalid fourcc, show this warning
					DBG_WRN("invalid bank info fourcc %08X, use default.\n", fourcc);
				}
			}
		}
		free(p_tmp);
		p_strg->Close();
	}
	p_strg->Unlock();
	THREAD_RETURN(er);
}

int flow_updfw_init_bank_info(void)
{
	VK_TASK_HANDLE vkt  = vos_task_create(init_thread, NULL,  "init_bank_info",  10, 4096);
	vos_task_resume(vkt);
	return 0;
}

int get_rtos_id_form_nvtpack(void)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
	if (p_fdt == NULL) {
		return -1;
	}
	int nodeoffset = fdt_path_offset(p_fdt, FDT_NVTPACK_PREFIX);
	if (nodeoffset == -FDT_ERR_NOTFOUND) {
		DBG_ERR("unable to find %s\n", FDT_NVTPACK_PREFIX);
		return -1;
	}

	for (nodeoffset = fdt_first_subnode(p_fdt, nodeoffset);
		 (nodeoffset >= 0);
		 (nodeoffset = fdt_next_subnode(p_fdt, nodeoffset))) {
		int len;
		const struct fdt_property *prop_reg;
		if (!(prop_reg = fdt_get_property(p_fdt, nodeoffset, "partition_name", &len))) {
			continue;
		}
		if (strncmp(prop_reg->data, "rtos", 5) != 0) {
			continue;
		}

		const char *name = fdt_get_name(p_fdt, nodeoffset, &len);
		int id = atoi(name + 2); //ignore "id" prefix
		return id;
	}
	DBG_ERR("unable to find rtos id in nvtpack.\n");
	return -1;
}

int flow_updfw(unsigned char *p_nvtpack, unsigned int size, BOOL *p_abort, VERSION_CTRL *p_verctrl)
{
	int er;
	STORAGE_OBJ *p_strg = NULL;

	/**
	 * init_partition has checked if necessary partitions existing
	 */

	if (p_verctrl == NULL) {
		DBG_ERR("p_verctrl is NULL\r\n");
		return -1;
	}

	// check sanity
	NVTPACK_ER nvtpack_er;
	NVTPACK_VERIFY_OUTPUT verify = {0};
	NVTPACK_MEM mem_nvtpack = {p_nvtpack, size};
	if ((nvtpack_er = nvtpack_verify(&mem_nvtpack, &verify)) != 0) {
		DBG_ERR("nvtpack_verify failed, er = %d\r\n", nvtpack_er);
		return -1;
	}

	// get rtos from nvtpack
	int rtos_nvtpack_idx = get_rtos_id_form_nvtpack();
	if (rtos_nvtpack_idx < 0) {
		return -1;
	}

	NVTPACK_GET_PARTITION_INPUT get_input = {0};
	NVTPACK_MEM mem_rtos = {0};
	get_input.id = rtos_nvtpack_idx;
	get_input.mem = mem_nvtpack;
	if ((nvtpack_er = nvtpack_get_partition(&get_input, &mem_rtos)) != NVTPACK_ER_SUCCESS) {
		DBG_ERR("failed to nvtpack_get_partition, er = %d\r\n", nvtpack_er);
		return -1;
	}

	// check abort
	if (*p_abort) {
		DBG_ERR("user abort.\n");
		return -1;
	}

	unsigned char *p_flash_block = (unsigned char *)malloc(_EMBMEM_BLK_SIZE_);

	// clean bank info
	DBG_DUMP("bank_info[%d] will be clean.\r\n", p_verctrl->bank_id);
	p_strg = EMB_GETSTRGOBJ(bank_map[p_verctrl->bank_id]);
	p_strg->Lock();
	if (p_strg->Open() == 0) {
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(0);
#endif
		memset(p_flash_block, 0, _EMBMEM_BLK_SIZE_);
		if ((er = p_strg->WrSectors((INT8 *)p_flash_block, 0, 1)) != 0) {
			DBG_ERR("rtos[%d] write partition info failed. er=%d\r\n", p_verctrl->bank_id, er);
			free(p_flash_block);
			p_strg->Close();
			p_strg->Unlock();
			return -1;
		}
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(1);
#endif
	} else {
		DBG_ERR("open bank partition failed.\r\n");
		free(p_flash_block);
		p_strg->Unlock();
		return -1;
	}
	p_strg->Close();
	p_strg->Unlock();

	// write fw
	DBG_DUMP("rtos[%d] will be updated.\r\n", p_verctrl->bank_id);
	int blk_cnts = ALIGN_CEIL(mem_rtos.len, _EMBMEM_BLK_SIZE_) / _EMBMEM_BLK_SIZE_;
	unsigned char *p = mem_rtos.p_data;
	p_strg = EMB_GETSTRGOBJ(rtos_map[p_verctrl->bank_id]);
	p_strg->Lock();
	if (p_strg->Open() == 0) {
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(0);
#endif
#if defined(_EMBMEM_SPI_NOR_) || defined(_EMBMEM_EMMC_)
		int i_blk = 0;
		int remain_bytes = mem_rtos.len;
		while (blk_cnts--) {
			INT8 *p_data = (INT8 *)p;
			if (remain_bytes < _EMBMEM_BLK_SIZE_) {
				memset(p_flash_block, 0, _EMBMEM_BLK_SIZE_);
				memcpy(p_flash_block, p, remain_bytes);
				p_data = (INT8 *)p_flash_block;
			}
			if ((er = p_strg->WrSectors(p_data, i_blk, 1)) != 0) {
				DBG_ERR("rtos[%d] write partition failed on block %d.\r\n", p_verctrl->bank_id, i_blk);
				p_strg->Close();
				p_strg->Unlock();
				break;
			}

			//check abort
			if (*p_abort) {
				DBG_ERR("user abort.\n");
				p_strg->Close();
				p_strg->Unlock();
				return -1;
			}

			p += _EMBMEM_BLK_SIZE_;
			remain_bytes -= _EMBMEM_BLK_SIZE_;
			i_blk++;
		}
#else
		if ((er = p_strg->WrSectors((INT8 *)p, 0, blk_cnts)) != 0) {
			DBG_ERR("rtos[%d] write partition failed. er=%d\r\n", p_verctrl->bank_id, er);
			p_strg->Close();
			p_strg->Unlock();
			return -1;
		}
#endif
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(1);
#endif
	} else {
		DBG_ERR("rtos[%d] open partition failed.\r\n", p_verctrl->bank_id);
	}
	p_strg->Close();
	p_strg->Unlock();

	// update bank info
	DBG_DUMP("bank_info[%d] will be updated.\r\n", p_verctrl->bank_id);
	p_strg = EMB_GETSTRGOBJ(bank_map[p_verctrl->bank_id]);
	p_strg->Lock();
	if (p_strg->Open() == 0) {
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(0);
#endif
		memset(p_flash_block, 0, _EMBMEM_BLK_SIZE_);
		memcpy(p_flash_block, p_verctrl, sizeof(VERSION_CTRL));
		if ((er = p_strg->WrSectors((INT8 *)p_flash_block, 0, 1)) != 0) {
			DBG_ERR("bank[%d] write partition info failed. er=%d\r\n", p_verctrl->bank_id, er);
			free(p_flash_block);
			p_strg->Close();
			p_strg->Unlock();
			return -1;
		}
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(1);
#endif
	} else {
		DBG_ERR("open bank partition failed.\r\n");
		free(p_flash_block);
		p_strg->Unlock();
	}
	p_strg->Close();
	p_strg->Unlock();
	free(p_flash_block);

	/**
	 * update uboot, only for debug
	 * needing modify uboot_partition_id and fdt path of storage_partition_write_part
	 */
	if (0) {
		int uboot_partition_id = 4;
		// get uboot from nvtpack
		NVTPACK_GET_PARTITION_INPUT get_input_uboot = {0};
		NVTPACK_MEM mem_uboot = {0};
		get_input_uboot.id = uboot_partition_id;
		get_input_uboot.mem = mem_nvtpack;
		if ((nvtpack_er = nvtpack_get_partition(&get_input_uboot, &mem_uboot)) != NVTPACK_ER_SUCCESS) {
			DBG_ERR("failed to nvtpack_get_partition, er = %d\r\n", nvtpack_er);
			return -1;
		}
		storage_partition_write_part("/nor/partition_uboot", (unsigned char *)mem_uboot.p_data, 0x0, mem_uboot.len);
	}

	return 0;
}

EXAMFUNC_ENTRY(test_destroy_fw, argc, argv)
{
	int er;
	extern BININFO bin_info;

	if (argc < 2) {
		DBG_DUMP("usage: test_desotry_fw [curr/next]\r\n");
		return 0;
	}


	int boot_bank_from = bin_info.Resv[BININFO_RESV_IDX_BOOT_BANK_FROM];
	int destory_idx = boot_bank_from;
	if (strncmp(argv[1], "next", 5) == 0) {
		destory_idx ^= 1;
	}

	DBG_DUMP("curr:%d, destroy:%d\r\n", boot_bank_from, destory_idx);

	STORAGE_OBJ *p_strg = EMB_GETSTRGOBJ(rtos_map[destory_idx]);
	p_strg->Lock();
	if (p_strg->Open() == 0) {
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(0);
#endif
		//just destory 1 blocks
		if ((er = p_strg->WrSectors((INT8 *)0x0, 1, 1)) != 0) {
			DBG_ERR("rtos[%d] write partition failed. er=%d\r\n", destory_idx, er);
			p_strg->Close();
			p_strg->Unlock();
			return -1;
		}
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(1);
#endif
	} else {
		DBG_ERR("rtos[%d] open partition failed.\r\n", destory_idx);
	}
	p_strg->Close();
	p_strg->Unlock();
	return 0;
}

EXAMFUNC_ENTRY(test_destroy_bank, argc, argv)
{
	int er;
	extern BININFO bin_info;
	int boot_bank_from = bin_info.Resv[BININFO_RESV_IDX_BOOT_BANK_FROM];
	int destory_idx = boot_bank_from;

	if (strncmp(argv[1], "next", 5) == 0) {
		destory_idx ^= 1;
	}

	DBG_DUMP("curr:%d, destroy:%d\r\n", boot_bank_from, destory_idx);

	STORAGE_OBJ *p_strg = EMB_GETSTRGOBJ(bank_map[destory_idx]);
	p_strg->Lock();
	if (p_strg->Open() == 0) {
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(0);
#endif
		//just destory 1 blocks
		if ((er = p_strg->WrSectors((INT8 *)0x0, 0, 1)) != 0) {
			DBG_ERR("rtos[%d] write partition failed. er=%d\r\n", destory_idx, er);
			p_strg->Close();
			p_strg->Unlock();
			return -1;
		}
#if defined(_EMBMEM_SPI_NOR_)
		//spi_nor_write_protect(1);
#endif
	} else {
		DBG_ERR("rtos[%d] open partition failed.\r\n", destory_idx);
	}
	p_strg->Close();
	p_strg->Unlock();
	return 0;
}

#endif

#if defined(_NVT_DUAL_RTOS_ON_) && (_PACKAGE_FILESYS_)
#include <FileSysTsk.h>
#define LOADER_UPD_FW_PATH "A:\\"_BIN_NAME_".BIN"

static VERSION_CTRL m_test_bank_info = {0}; // do not set it local, because it is large.

EXAMFUNC_ENTRY(test_dualfw, argc, argv)
{
	BOOL b_abort = FALSE;
	// get file size first to calc working buffer
	unsigned int nvtpack_len = (unsigned int)FileSys_GetFileLen(LOADER_UPD_FW_PATH);
	unsigned char *p_nvtpack = (unsigned char *)malloc(nvtpack_len);
	if (p_nvtpack == NULL) {
		DBG_ERR("malloc failed.\n");
		return -1;
	}
	// read file
	FST_FILE h_file = FileSys_OpenFile(LOADER_UPD_FW_PATH, FST_OPEN_READ);
	if (h_file == 0) {
		DBG_ERR("FW bin open fail\r\n");
		free(p_nvtpack);
		return -1;
	}

	if (FileSys_ReadFile(h_file, (UINT8 *)p_nvtpack, (UINT32 *)&nvtpack_len, 0, NULL) != 0) {
		DBG_ERR("FW bin read fail\r\n");
		free(p_nvtpack);
		FileSys_CloseFile(h_file);
		return -1;
	}

	FileSys_CloseFile(h_file);

	// check sanity
	NVTPACK_ER nvtpack_er;
	NVTPACK_VERIFY_OUTPUT verify = {0};
	NVTPACK_MEM mem_nvtpack = {0};
	mem_nvtpack.p_data = p_nvtpack;
	mem_nvtpack.len = nvtpack_len;
	if ((nvtpack_er = nvtpack_verify(&mem_nvtpack, &verify)) != 0) {
		DBG_ERR("nvtpack_verify failed, er = %d\r\n", nvtpack_er);
		free(p_nvtpack);
		return -1;
	}

	// get rtos from nvtpack
	int rtos_nvtpack_idx = get_rtos_id_form_nvtpack();
	if (rtos_nvtpack_idx < 0) {
		free(p_nvtpack);
		return -1;
	}
	NVTPACK_GET_PARTITION_INPUT get_input = {0};
	NVTPACK_MEM mem_rtos = {0};
	get_input.id = rtos_nvtpack_idx;
	get_input.mem = mem_nvtpack;
	if ((nvtpack_er = nvtpack_get_partition(&get_input, &mem_rtos)) != NVTPACK_ER_SUCCESS) {
		DBG_ERR("failed to nvtpack_get_partition, er = %d\r\n", nvtpack_er);
		free(p_nvtpack);
		return -1;
	}

	// get rtos version from nvtpack
	int nvtpack_rtos_fw_type = 0;
	unsigned int nvtpack_rtos_date = 0;
	unsigned int nvtpack_rtos_ver = 0;
	NVTPACK_BFC_HDR *p_nvtpack_rtos_bfc = (NVTPACK_BFC_HDR *)mem_rtos.p_data;
	if (p_nvtpack_rtos_bfc->uiFourCC != MAKEFOURCC('B', 'C', 'L', '1')) {
		DBG_ERR("nvtpack-rtos invalid bfc.\r\n");
		free(p_nvtpack);
		return -1;
	} else {
		const int unlz_preload_size = ALIGN_CEIL_4(_BIN_INFO_OFS_ + sizeof(BININFO));
		const int unlz_tmp_size = ALIGN_CEIL_4(128 * 1024); //128KB enough
		unsigned char *p_unlz_tmp_fw = (unsigned char *)malloc(unlz_tmp_size);

		if (p_unlz_tmp_fw == NULL) {
			DBG_ERR("alloc unlz_tmp_fw failed.\n");
			free(p_nvtpack);
		}

		//unlzma some to get version
		if ((be32_to_cpu(p_nvtpack_rtos_bfc->uiAlgorithm) & 0xFF) == 11) {
			DBG_ERR("nvtpack-rtos  unsupported lzma data.\r\n");
			free(p_unlz_tmp_fw);
			free(p_nvtpack);
			return -1;
		} else {
			int decoded = LZ_Uncompress((unsigned char *)&p_nvtpack_rtos_bfc[1], p_unlz_tmp_fw, unlz_preload_size);
			if (decoded > unlz_tmp_size) {
				DBG_ERR("unlz_tmp_size is too small, require: %d.\n", decoded);
			}
		}

		BININFO *p_bininfo = (BININFO *)(p_unlz_tmp_fw + _BIN_INFO_OFS_);
		nvtpack_rtos_ver = p_bininfo->Resv[BININFO_RESV_IDX_VER];
		DBG_DUMP("nvtpack-rtos ver: %08X\r\n", nvtpack_rtos_ver);

		nvtpack_rtos_date = p_bininfo->Resv[BININFO_RESV_IDX_DATE_SEC];
		DBG_DUMP("nvtpack-rtos date: %u\r\n", nvtpack_rtos_date);

		nvtpack_rtos_fw_type = p_bininfo->Resv[BININFO_RESV_IDX_RELEASE_BUILD];
		DBG_DUMP("nvtpack-rtos fw_type: %u\r\n", nvtpack_rtos_fw_type);

		free(p_unlz_tmp_fw);
	}

	//fill bank info
	memset(&m_test_bank_info, 0, sizeof(m_test_bank_info));
	m_test_bank_info.fourcc = MAKEFOURCC('V', 'E', 'R', 'C');
	m_test_bank_info.bank_id = m_boot_bank_info.bank_id ^ 1;
	m_test_bank_info.fw_type = nvtpack_rtos_fw_type;
	m_test_bank_info.fw_version = nvtpack_rtos_ver;
	m_test_bank_info.fw_date =  nvtpack_rtos_date;
	m_test_bank_info.update_cnt = m_boot_bank_info.update_cnt + 1;

	flow_updfw(p_nvtpack, nvtpack_len, &b_abort, &m_test_bank_info);
	free(p_nvtpack);
	return 0;
}

#endif
