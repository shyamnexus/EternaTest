#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/type.h>
#include <kwrap/perf.h>
#include <kwrap/cpu.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include <fdtfast.h>
#include "prjcfg.h"
#include "sys_fwload.h"
#include "sys_fastboot.h"
#include "sys_linuxboot.h"
#include "flow_boot_linux.h"
#include <kwrap/debug.h>
#include "vendor_videocapture.h"
#include "isp_api.h"
#include "sen_inc.h"
#include "kwrap/util.h"
#include <dispdevctrl.h>
#include <plat/nand.h>
#include "sys_storage_partition.h"
#include <MemCheck.h>
#include "kdrv_audioio/kdrv_audioio.h"
#include "isp_builtin.h" // NOTE: Include ISP builtin
#include <kdrv_gfx2d/kdrv_ise_ctl.h>
#include "fast_ipp.h"
#include "sys_fdt.h"
#include "sys_fastboot.h"
#include "kdrv_builtin/nvtmpp_init.h" // for nvtmpp_get_fastboot_mem
#include <kflow_common/nvtmpp.h>

#if (CFG_LOAD_AI_MODEL)
static int get_ai_model(unsigned int *p_addr, unsigned int *p_size, unsigned long long *p_flash_ofs)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	unsigned int partition_size;
	const void *nodep;  /* property node pointer */

	// check case of fastboot load ai model only
	nodeoffset = fdt_path_offset(p_fdt, "/ai-memory/ai_model");
	if (nodeoffset >= 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "load_dst", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access load_dst.\n");
			return -1;
		} else {
			unsigned int *p_data = (unsigned int *)nodep;
			*p_addr = be32_to_cpu(p_data[0]);
			*p_size = be32_to_cpu(p_data[1]);
		}

		nodeoffset = fdt_path_offset(p_fdt, PARTITION_PATH_AI);
		if (nodeoffset < 0) {
			DBG_ERR("no ai_flash\n");
			return 1;
		}

		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned long long *p_data = (unsigned long long *)nodep;
			*p_flash_ofs = be64_to_cpu(p_data[0]);
			partition_size = (unsigned int)be64_to_cpu(p_data[1]);
			if (partition_size < *p_size) {
				*p_size = partition_size;
				DBG_ERR("ai size(0x%X) > partition size (0x%X)\r\n", *p_size, partition_size);
				return -1;
			}
		}
		return 0;
	}

	DBG_DUMP("no ai\n");
	return 1; //no ai
}

int load_ai_model_nowait(LINUXBOOT_INFO *p_info)
{
	int er;
	UINT32 blksize = 0;
	unsigned long read_buf = 0x0;
	unsigned int  read_size = 0x0;
	unsigned long long read_ofs = 0x0;

	er = get_ai_model(&read_buf, &read_size, &read_ofs);

	// check if ai exists in fdt
	if (er == 1) {
		return 0;
	}

	if (er != 0) {
		return er;
	}

	STORAGE_OBJ *pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_ALL);
	if (pStrg == NULL) {
		DBG_ERR("pStrg is NULL.\n");
		return -1;
	}

	pStrg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&blksize, 0);
	if (blksize == 0) {
		DBG_ERR("blksize is 0.\n");
		return -1;
	}

	unsigned int blkcnt = ALIGN_CEIL(read_size, blksize) / blksize;

#if 0 //calc check sum
	pStrg->Lock();
	pStrg->Open();
	pStrg->RdSectors((INT8 *)read_buf, (UINT32)(read_ofs / blksize), (UINT32)(blkcnt));
	pStrg->Close();
	pStrg->Unlock();
	DBGD(MemCheck_CalcCheckSum16Bit((UINT32)read_buf, (UINT32)read_size));
	memset((INT8 *)read_buf, 0, read_size);
	vos_cpu_dcache_sync((VOS_ADDR)read_buf, read_size, VOS_DMA_TO_DEVICE);
#endif

#if defined(_EMBMEM_SPI_NOR_)
	if (is_fastboot()) {
		//notify linux-spinor in preload mode
		linuxboot_set_flash_preload(p_info);

		//uiSectorAddr and uiSectorSize are bytealignment but rather block number and block count
		NOR_READ_NONBLK_CFG nor_cfg = {0};
		nor_cfg.uiSectorAddr = (UINT32)(read_ofs);
		nor_cfg.uiSectorSize = blkcnt * blksize;
		nor_cfg.pBuf = (UINT8 *)read_buf;

		pStrg->Lock();
		pStrg->Open();
		if (pStrg->ExtIOCtrl(STRG_EXT_NOR_READ_NON_BLOCKING, (ULONG)&nor_cfg, 0) != E_OK) {
			DBG_ERR("failed to ExtIOCtrl\n");
			return -1;
		}
		//pStrg->Close(); //DO NOT close spi-nor for non-blocking load ai-model for spi-nor
		pStrg->Unlock();
	} else {
		DBG_WRN("not in fastboot, using safe-load-ai.\n");
		er = storage_partition_read_part(PARTITION_PATH_AI, (unsigned char *)read_buf, 0, (int)(blksize * blkcnt));
		if (er != 0) {
			return er;
		}
	}
#else
	er = storage_partition_read_part(PARTITION_PATH_AI, (unsigned char *)read_buf, 0, (int)(blksize * blkcnt));
	if (er != 0) {
		return er;
	}
#endif
	return 0;
}
#endif

#if (CFG_LOAD_AI_DETECT)
#if _TODO_FAST2A
#include <ai_ioctl.h>
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_ai.h"
#endif
#define AI_NEW_VER              0 //If 1, support kdrv_ai_drv_set_net_supported_num
#define AI_METHOD               0 //0: NULL, 1:NULL, 2:NULL, 3:PVD(3.1.1)+limit 4:PVD(3.1.5.001)+limit+fd
#define VENDOR_AI_CFG           0x000f0000  //ai project config
#if (AI_METHOD == 3)
#include "pvdcnn_lib.h"
#include "limit_fdet_lib.h"
#define MAX_DISTANCE_MODE       0
#define AI_VDO_W                PVD_YUV_WIDTH //320
#define AI_VDO_H                PVD_YUV_HEIGHT //320
#define PVD_MODEL_SIZE          (0x128680)
#define PVD_BASE_SIZE           (0x300000)//(0x2C1CA0)
#define LIMIT_FDET_MODEL_SIZE   (0x8A2C0)
#define LIMIT_FDET_BASE_SIZE    (0x100000)//(0x9BAE0)
#define OUT_BASE_SIZE           (0xe100)
#define AI_PVD_BUF_SIZE         PVD_BASE_SIZE + LIMIT_FDET_BASE_SIZE + OUT_BASE_SIZE + 0x400
#define mem_select(p)           AI_PVD_BUF_SIZE
#elif (AI_METHOD == 4)
#include "ext_module_api.h"
#include "pvdcnn_lib.h"
#include "limit_fdet_lib.h"
#define AI_VDO_W                 960
#define AI_VDO_H                 540
#define VENDOR_AI_CFG  			 0x000f0000  //vendor ai config
#define PRE_LOAD_PA 			 (0x7bff100)  // Preload model address, may need modification
#define USE_PRE_LOAD 			 DISABLE
#define AI_PVD_MODEL_SIZE        654976
#define AI_PVD_BUF_SIZE          1470272
#define AI_LIMIT_FDET_MODEL_SIZE 152832
#define AI_LIMIT_FDET_BUF_SIZE   213824
#define mem_select(p)            (AI_PVD_BUF_SIZE + AI_LIMIT_FDET_BUF_SIZE)
#define PVD_MAX_DISTANCE_MODE    0
#else
#include "hd_gfx.h"
#define AI_VDO_W                960
#define AI_VDO_H                540
#define mem_select(p)           (0x100000)
#endif
#define DUMP_AI_MODEL           1
#define DUMP_AI_MEMORY          1
#define DUMP_AI_IMAGE           0
#define STORE_AI_IMAGE          0
#define DEBUG_AI_MWP            0
#define LOOP_AI_PROC            0
typedef struct {
#if (AI_METHOD == 3)
	VENDOR_AI_BUF input_image;
	PVDCNN_MEM pvdcnn_mem;
	PVD_PROPOSAL_PARAM pvd_proposal_params;
	PVD_LAYER_PARAM PVD_params;
	PVD_LAYER_MEM PVD_det_mem;
	UINT32 outlayer_num;
	UINT32 layer_buffer; //va
	UINT32 out_layer_float; //va
	LIMIT_FDET_PARAM limit_fdet_params;
	LIMIT_FDET_MEM limit_fdet_mem;
	VENDOR_AIS_FLOW_MEM_PARM pred_cls_output_info;
#elif (AI_METHOD == 4)
	VENDOR_AI_BUF            input_image;
	VENDOR_AI_BUF            src_image;
	VENDOR_AIS_FLOW_MEM_PARM pvd_mem;
	VENDOR_AIS_FLOW_MEM_PARM limit_fdet_mem;
	VENDOR_AIS_FLOW_MEM_PARM pvd_model_mem;
	VENDOR_AIS_FLOW_MEM_PARM limit_model_mem;
#endif
	uintptr_t src_y;
	uintptr_t img_y;
	unsigned int detect_buf;
	unsigned int model_size;
	int image_init;
	int detect_num;
	int detect_done;
	int ai_det_model_exist;
} FASTBOOT_DETECT;
static FASTBOOT_DETECT g_detect_hdl = {0};

#if (DUMP_AI_MODEL) || (DUMP_AI_IMAGE)
static void dump_buffer(char *p, int len)
{
	int i, j;
	char  *pc = p;
	for (i = 0; i < len; i += 0x10) {
		//printf("ad = 0x%03x : ", i);
		for (j = 0; j < 0x10; j++) {
			printf("%02x ", *pc);
			pc++;
		}
		printf("\r\n");
	}
}
#endif

#if (DUMP_AI_MEMORY)
#include "kflow_common/nvtmpp.h"
static void dump_mem_range(void)
{
	nvtmpp_dump_mem_range(vk_printk);
}
#endif

#if (STORE_AI_IMAGE)
#include <FileSysTsk.h>
static void store_img(char *ptr, int len, int index)
{
	FILE *f_out;
	char file_path[40];

	snprintf(file_path, 40, "/mnt/sd/ai_img_%d.yuv", index);
	DBG_DUMP("save file (%s), ptr = 0x%x\r\n", file_path, ptr);
	if ((f_out = fopen(file_path, "wb")) == NULL) {
		DBG_ERR("open file (%s) fail....\r\n", file_path);
	}
	if (f_out) {
		fwrite(ptr, 1, len, f_out);
		fflush(f_out);
		fclose(f_out);
	}
}
#endif

#if (DEBUG_AI_MWP)
#include "comm/ddr_arb.h"
VOID fast_ai_write_protect(UINT32 addr, UINT32 size, DMA_WRITEPROT_SET wpset, DMA_PROT_MODE wpmode)
{
	DMA_WRITEPROT_ATTR attr = {0};
	UINT32 dram_mode = 0; //dram_1

	DBG_DUMP("wp_in_range_p(wpset[%d]wpmode[%d]): addr=0x%x size=0x%x\r\n", wpset, wpmode, addr, size);

	attr.level = DMA_WPLEL_DETECT;
	attr.protect_mode = wpmode;

	if (wpmode == DMA_PROT_IN) {
		memset((void *)&attr.mask, 0xff, sizeof(DMA_CH_MSK));
		attr.mask.CNN_0 = 0;  //< CNN_0 (in)
		attr.mask.CNN_1 = 0;  //< CNN_1 (in)
		attr.mask.CNN_2 = 0;  //< CNN_2 (in)
		attr.mask.CNN_3 = 0;  //< CNN_3 (in)
		attr.mask.CNN_4 = 0;  //< CNN_4 (out)
		attr.mask.CNN_5 = 0;  //< CNN_5 (out)
		attr.mask.CNN_6 = 0;  //< CNN_6 (out)
		attr.mask.NUE2_0 = 0;  //< NUE2_0 (in)
		attr.mask.NUE2_1 = 0;  //< NUE2_1 (in)
		attr.mask.NUE2_2 = 0;  //< NUE2_2 (in)
		attr.mask.NUE2_3 = 0;  //< NUE2_3 (out)
		attr.mask.NUE2_4 = 0;  //< NUE2_4 (out)
		attr.mask.NUE2_5 = 0;  //< NUE2_5 (out)
		attr.mask.NUE2_6 = 0;  //< NUE2_6 (out)
	}

	if (wpmode == DMA_PROT_OUT) {
		memset((void *)&attr.mask, 0x0, sizeof(DMA_CH_MSK));
		attr.mask.CNN_0 = 1;  //< CNN_0 (in)
		attr.mask.CNN_1 = 1;  //< CNN_1 (in)
		attr.mask.CNN_2 = 1;  //< CNN_2 (in)
		attr.mask.CNN_3 = 1;  //< CNN_3 (in)
		attr.mask.CNN_4 = 1;  //< CNN_4 (out)
		attr.mask.CNN_5 = 1;  //< CNN_5 (out)
		attr.mask.CNN_6 = 1;  //< CNN_6 (out)
		attr.mask.NUE2_0 = 1;  //< NUE2_0 (in)
		attr.mask.NUE2_1 = 1;  //< NUE2_1 (in)
		attr.mask.NUE2_2 = 1;  //< NUE2_2 (in)
		attr.mask.NUE2_3 = 1;  //< NUE2_3 (out)
		attr.mask.NUE2_4 = 1;  //< NUE2_4 (out)
		attr.mask.NUE2_5 = 1;  //< NUE2_5 (out)
		attr.mask.NUE2_6 = 1;  //< NUE2_6 (out)
	}

	attr.protect_rgn_attr[0].en = 1;
	attr.protect_rgn_attr[0].starting_addr = addr;  //physical addressA
	attr.protect_rgn_attr[0].size = size;
	arb_enable_wp(dram_mode, wpset, &attr);
}
#endif

HD_RESULT parse_fboot_hdal_mem_range(HD_COMMON_MEM_INIT_CONFIG *p_mem_config)
{
	// allocate fixed memory (nvtmpp is required for mempool_init)
	//nvtmpp_init(); // nvtmpp_init is in part-3

	UINT32 hdal_addr = 0;
	{
		unsigned char *p_fdt = (unsigned char *)fdt_get_base();

		if (p_fdt == NULL) {
			DBG_ERR("p_fdt is NULL.\n");
			return HD_ERR_NULL_PTR;
		}

		int len;
		int nodeoffset;
		const void *nodep;  /* property node pointer */

		// get linux space
		nodeoffset = fdt_path_offset(p_fdt, "/hdal-memory/media");
		if (nodeoffset < 0) {
			DBG_ERR("failed to offset for  /hdal-memory/media = %d \n", nodeoffset);
			return HD_ERR_NOT_AVAIL;
		}

		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return HD_ERR_NOT_AVAIL;
		} else {
			unsigned int *p_data = (unsigned int *)nodep;
			hdal_addr = (UINT32)be32_to_cpu(p_data[1]);
		}
	}

	UINT32 end_addr = 0;
	UINT32 blk_size = 0;
	{
		//=> |[comn_blk]|[misc_cpool]|[pv_pools]|
		NVTMPP_FASTBOOT_MEM_S *p_fastboot_mem;
		p_fastboot_mem = nvtmpp_get_fastboot_mem();
		if (NULL == p_fastboot_mem) {
			DBG_ERR("nvtmpp_get_fastboot_mem failed\r\n");
			return HD_ERR_NOMEM;
		}

		end_addr = ALIGN_CEIL(p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC_CTRL].addr + p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC_CTRL].size, 0x40);
		if (p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_0_MAX].addr) {
			end_addr = ALIGN_CEIL(p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_0_MAX].addr + p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_0_MAX].size, 0x40);
		}
		if (p_fastboot_mem->pv_pools[FBOOT_POOL_ACAP_CTRL].addr) {
			end_addr = ALIGN_CEIL(p_fastboot_mem->pv_pools[FBOOT_POOL_ACAP_CTRL].addr + p_fastboot_mem->pv_pools[FBOOT_POOL_ACAP_CTRL].size, 0x40);
		}
		if (p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].addr) {
			end_addr = ALIGN_CEIL(p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].addr + p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].size, 0x40);
		}
		if (p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC1_CTRL].addr) {
			end_addr = ALIGN_CEIL(p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC1_CTRL].addr + p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC1_CTRL].size, 0x40);
		}
		if (p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_2_MAX].addr) {
			end_addr = ALIGN_CEIL(p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_2_MAX].addr + p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_2_MAX].size, 0x40);
		}
		if (end_addr == 0) {
			end_addr = ALIGN_CEIL(hdal_addr, 0x40);
		}
	}
	blk_size = end_addr - hdal_addr;

	p_mem_config->pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	p_mem_config->pool_info[0].blk_size = blk_size;
	p_mem_config->pool_info[0].blk_cnt = 1;
	p_mem_config->pool_info[0].ddr_id = DDR_ID0;

	return HD_OK;
}

HD_COMMON_MEM_VB_BLK hd_common_mem_get_block_lite(HD_COMMON_MEM_POOL_TYPE pool_type, UINT32 blk_size, HD_COMMON_MEM_DDR_ID ddr)
{
	/**
	 * get fastboot memory from dts
	 * nvtmpp_parse_fastboot_mem_dt has called earlier on _fast_open_sie
	 * do not call it again, or will boot linux NG
	 */
	NVTMPP_FASTBOOT_MEM_S *p_fastboot_mem;
	p_fastboot_mem = nvtmpp_get_fastboot_mem();
	if (NULL == p_fastboot_mem) {
		DBG_ERR("nvtmpp_get_fastboot_mem failed\r\n");
		return -1;
	}
	/**
	 * get blk from cnn pool "HD_COMMON_MEM_CNN_POOL"
	 * only for ai cnn usage
	 */
	int path_id;
	for (path_id = 0; path_id < FBOOT_MISC_CPOOL_MAX; path_id++) {
		//int i;
		if (p_fastboot_mem->misc_cpool[path_id].pool_type == pool_type) {
			return (HD_COMMON_MEM_VB_BLK)p_fastboot_mem->misc_cpool[path_id].blk;
		}
	}
	return HD_COMMON_MEM_VB_INVALID_BLK;
}

UINTPTR hd_common_mem_blk2pa_lite(HD_COMMON_MEM_VB_BLK blk)
{
	/**
	 * get pa from cnn blk
	 * only for ai cnn usage
	 */
	return ((NVTMPP_FASTBOOT_BLK_S *)blk)->addr;
}

#if !(_PACKAGE_VIDEO_)
HD_RESULT hd_common_mem_init_lite(HD_COMMON_MEM_INIT_CONFIG *p_mem_config)
{
	NVTMPP_VB_CONF_S st_conf = {0};
	st_conf.max_pool_cnt = 64;
	st_conf.common_pool[0].blk_size = p_mem_config->pool_info[0].blk_size;
	st_conf.common_pool[0].blk_cnt = 1;
	st_conf.common_pool[0].ddr = NVTMPP_DDR_1;
	st_conf.common_pool[0].type = POOL_TYPE_COMMON;
	nvtmpp_vb_set_conf(&st_conf);
	nvtmpp_vb_init();

#if (DUMP_AI_MEMORY)
	dump_mem_range();
#endif
	return HD_OK;
}

// hd_common.c - lite for ai
int g_cfg_lite = 0;
int g_cfg1_lite = 0;
int g_cfg2_lite = 0;
HD_RESULT hd_common_init_lite(UINT32 sys_config_type)
{
	//config
	g_cfg_lite = sys_config_type;
	g_cfg1_lite = 0;
	g_cfg2_lite = 0;
	return HD_OK;
}
HD_RESULT hd_common_sysconfig_lite(UINT32 sys_config_type1, UINT32 sys_config_type2, UINT32 type1_mask, UINT32 type2_mask)
{
	UINT32 m2;
	m2 = (sys_config_type2 & type2_mask);
	// only for ai now
	if (m2 & VENDOR_AI_CFG) {
		g_cfg2_lite &= ~(m2 & VENDOR_AI_CFG);
		g_cfg2_lite |= (m2 & VENDOR_AI_CFG);
	}
	return HD_OK;
}

// hd_common.c -> for ai flow
int nvtmpp_hdl = 0; // for vendor_common_mem_cache_sync
HD_RESULT hd_common_get_sysconfig(UINT32 *p_sys_config_type1, UINT32 *p_sys_config_type2)
{
	if (p_sys_config_type1 != 0) {
		p_sys_config_type1[0] = g_cfg1_lite;
	}
	if (p_sys_config_type2 != 0) {
		p_sys_config_type2[0] = g_cfg2_lite;
	}
	return HD_OK;
}
#define nvtmpp_sys_pa2va(pa) pa
void *hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE mem_type, UINTPTR phy_addr, UINT32 size)
{
	void        *map_addr;
	if (HD_COMMON_MEM_MEM_TYPE_NONCACHE == mem_type) {
		DBG_ERR("Not Support noncache map\r\n");
		return NULL;
	}
	if (0 == size) {
		DBG_ERR("size is 0\r\n");
		return NULL;
	}
	map_addr = (void *)nvtmpp_sys_pa2va(phy_addr);
	if ((UINTPTR)(-1) == (UINTPTR)map_addr) {
		DBG_ERR("mmap fail phy_addr 0x%lx, size 0x%lx\r\n", (unsigned long)phy_addr, (unsigned long)size);
		goto map_err;
	}
	return map_addr;
map_err:
	return NULL;
}
HD_RESULT hd_common_mem_munmap(void *virt_addr, unsigned int size)
{
	if (0 == size) {
		DBG_ERR("size is 0\r\n");
		return HD_ERR_INV;
	}
	return HD_OK;
}
HD_RESULT hd_common_mem_flush_cache(void *virt_addr, unsigned int size)
{
	if (0 == size) {
		DBG_ERR("size is 0\r\n");
		return HD_ERR_INV;
	}
	vos_cpu_dcache_sync((VOS_ADDR)virt_addr, size, VOS_DMA_BIDIRECTIONAL);
	return HD_OK;
}
HD_RESULT hd_common_mem_alloc(CHAR *name, UINTPTR *phy_addr, void **virt_addr, UINT32 size, HD_COMMON_MEM_DDR_ID ddr)
{
	UINTPTR                       pa;
	void                         *va;
	NVTMPP_VB_POOL pool;
	pool = nvtmpp_vb_create_pool(name, size, 1, ddr);
	if (pool == NVTMPP_VB_INVALID_POOL) {
		DBG_ERR("cre_pool_fail\r\n");
		return HD_ERR_NOMEM;
	}
	NVTMPP_VB_BLK blk;
#define USER_MODULE MAKE_NVTMPP_MODULE('u', 's', 'e', 'r', 0, 0, 0, 0)
	blk = nvtmpp_vb_get_block(USER_MODULE, pool, size, ddr);
	if (blk < 0) {
		DBG_ERR("get_blk_fail\r\n");
		return HD_ERR_NOMEM;
	}
	pa = nvtmpp_vb_blk2pa(blk);
	if (pa <= 0) {
		DBG_ERR("get_pa_fail\r\n");
		return HD_ERR_NOMEM;
	}
	va = (void *)pa;
	*phy_addr = pa;
	*virt_addr = va;
	return HD_OK;
}
HD_RESULT hd_common_mem_free(UINTPTR phy_addr, void *virt_addr)
{
	return HD_OK;
}
HD_RESULT hd_common_mem_cache_sync(void* virt_addr, unsigned int size, HD_COMMON_MEM_DMA_DIR dir)
{
	return 0;
}
// hd_util -> for ai flow
#include "comm/hwclock.h"
UINT32 hd_gettime_ms(VOID)
{
	return (UINT32)(hwclock_get_longcounter() / 1000);
}
UINT64 hd_gettime_us(VOID)
{
	return hwclock_get_longcounter();
}
// hd_gfx.c -> for ai flow
HD_RESULT hd_gfx_init_lite(VOID)
{
	return HD_OK;
}
HD_RESULT hd_gfx_scale(HD_GFX_SCALE *p_param)
{
	uintptr_t out_addr, in_addr;
	int out_lofs, out_w, out_h, in_lofs, in_w, in_h;
	int io_pack_fmt, scale_method;
	static int i = 0;

	if (i == 0) {
		vos_perf_list_mark("sc", __LINE__, i);
	}

	// not supported
	if (p_param->src_img.format != HD_VIDEO_PXLFMT_YUV420) {
		DBG_DUMP("[%s] src_img fmt(0x%x) not supported\r\n", __func__, p_param->src_img.format);
	}
	if (p_param->dst_img.format != HD_VIDEO_PXLFMT_YUV420) {
		DBG_DUMP("[%s] dst_img fmt(0x%x) not supported\r\n", __func__, p_param->dst_img.format);
	}

	// choose scale method
	if (p_param->src_region.w >= p_param->dst_region.w && p_param->src_region.h >= p_param->dst_region.h) {
		scale_method = ISE_DRV_SCALE_METHOD_ISD;
	} else {
		scale_method = ISE_DRV_SCALE_METHOD_BILINEAR;
	}
	DBG_IND("[scale] scale_method = %d\r\n", scale_method);

	// Y8
	io_pack_fmt = ISE_DRV_Y8_ONLY;
	out_addr = p_param->dst_img.p_phy_addr[0] + (p_param->dst_region.x + p_param->dst_img.lineoffset[0] * p_param->dst_region.y);
	out_lofs = p_param->dst_img.lineoffset[0];
	out_w = p_param->dst_region.w;
	out_h = p_param->dst_region.h;
	in_addr = p_param->src_img.p_phy_addr[0] + (p_param->src_region.x + p_param->src_img.lineoffset[0] * p_param->src_region.y);
	in_lofs = p_param->src_img.lineoffset[0];
	in_w = p_param->src_region.w;
	in_h = p_param->src_region.h;
	ise_scale(out_addr, out_lofs, out_w, out_h, in_addr, in_lofs, in_w, in_h, io_pack_fmt, scale_method);

	// UVP
	io_pack_fmt = ISE_DRV_UVP;
	out_addr = p_param->dst_img.p_phy_addr[1] + (p_param->dst_region.x + (p_param->dst_img.lineoffset[1] * (p_param->dst_region.y >> 1)));
	out_lofs = p_param->dst_img.lineoffset[1];
	out_w = p_param->dst_region.w >> 1; // UV packed the UV width should be the half of Y width
	out_h = p_param->dst_region.h >> 1;
	in_addr = p_param->src_img.p_phy_addr[1] + (p_param->src_region.x + (p_param->src_img.lineoffset[1] * (p_param->src_region.y >> 1)));
	in_lofs = p_param->src_img.lineoffset[1];
	in_w = p_param->src_region.w >> 1; // UV packed the UV width should be the half of Y width
	in_h = p_param->src_region.h >> 1;
	ise_scale(out_addr, out_lofs, out_w >> 1, out_h, in_addr, in_lofs, in_w >> 1, in_h, io_pack_fmt, scale_method);

	i++;
	vos_perf_list_mark("sc", __LINE__, i);

	return HD_OK;
}
#endif //!(_PACKAGE_VIDEO_)

static int get_ai_det_model(unsigned long *p_addr, unsigned long *p_size, unsigned long long *p_flash_ofs)
{
#if defined(FASTBOOT20)
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
#else
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
#endif

	if (p_fdt == NULL) {
		DBG_ERR("p_fdtfast is NULL.\n");
		return -1;
	}

	int len;
	int pool_idx, nodeoffset;
	unsigned int partition_size;
	const void *nodep;  /* property node pointer */

	// check case of fastboot load ai model only
	nodeoffset = fdt_path_offset(p_fdt, "/fastboot/hdal-mem");
	if (nodeoffset < 0) {
		DBG_ERR("unable to parse /fastboot/hdal-mem\n");
		return -1;
	}

	// loop misc_cpooln_pool_type to find type HD_COMMON_MEM_CNN_POOL
	char misc_cpooln_pool_type_name[32] = {0};
	for (pool_idx = 0; pool_idx < NVTMPP_VB_MAX_COMM_POOLS; pool_idx++) {
		snprintf(misc_cpooln_pool_type_name, 31, "misc_cpool%d_pool_type", pool_idx);
		nodep = fdt_getprop(p_fdt, nodeoffset, misc_cpooln_pool_type_name, &len);
		if (len == 0 || nodep == NULL) {
			DBG_DUMP("no ai\n");
			return 1;
		} else {
			unsigned int *p_data = (unsigned int *)nodep;
			int misc_cpooln_pool_type = be32_to_cpu(p_data[0]);
			if (misc_cpooln_pool_type == HD_COMMON_MEM_CNN_POOL) {
				break;
			}
		}
	}

	if (pool_idx == NVTMPP_VB_MAX_COMM_POOLS) {
		DBG_DUMP("no ai, no HD_COMMON_MEM_CNN_POOL pool\n");
		return 1;
	}

	snprintf(misc_cpooln_pool_type_name, 31, "misc_cpool%d_blk_0", pool_idx);
	nodep = fdt_getprop(p_fdt, nodeoffset, misc_cpooln_pool_type_name, &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("unable to find %s\n", misc_cpooln_pool_type_name);
		return -1;
	} else {
		unsigned long *p_data = (unsigned long *)nodep;
		*p_addr = be64_to_cpu(p_data[0]);
		*p_size = be64_to_cpu(p_data[1]);
	}

	// get ai model bin size
	unsigned int bin_size = 0;
	nodeoffset = fdt_path_offset(p_fdt, "/fastboot/ai_model");
	if (nodeoffset < 0) {
		DBG_ERR("no ai, /fastboot/ai_model is not existing");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "bin_size", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("unable to find bin_size\n");
		return -1;
	} else {
		unsigned int *p_data = (unsigned int *)nodep;
		bin_size = be32_to_cpu(p_data[0]);
	}

	if (ALIGN_CEIL(bin_size, _EMBMEM_BLK_SIZE_) > *p_size) {
		DBG_ERR("misc_cpool0_blk_0 is too small, require: %08X\n", ALIGN_CEIL(bin_size, _EMBMEM_BLK_SIZE_));
		return -1;
	}

	*p_size = bin_size;

	p_fdt = (unsigned char *)fdt_get_base();
	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	nodeoffset = fdt_path_offset(p_fdt, PARTITION_PATH_AI);
	if (nodeoffset < 0) {
		DBG_ERR("no ai_flash\n");
		return -1;
	}

	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return -1;
	} else {
		unsigned long long *p_data = (unsigned long long *)nodep;
		*p_flash_ofs = be64_to_cpu(p_data[0]);
		partition_size = (unsigned int)be64_to_cpu(p_data[1]);
		if (partition_size < *p_size) {
			*p_size = partition_size;
			DBG_ERR("ai size(0x%X) > partition size (0x%X)\r\n", *p_size, partition_size);
			return -1;
		}
	}
	return 0;
}

int operation_ai_prepare(int net_num)
{
	/* part-1: load ai model */
	int er;
	UINT32 blksize = 0;
	unsigned long read_buf = 0x0;
	unsigned long read_size = 0x0;
	unsigned long long read_ofs = 0x0;

	vos_perf_list_mark("ai", __LINE__, 0);

	er = get_ai_det_model(&read_buf, &read_size, &read_ofs);

	if (er == 0) {
		g_detect_hdl.ai_det_model_exist = 1;
	}

	fastboot_set_done(BOOT_FLOW_AI_DET_MODEL);

	// check if ai exists in fdt
	if (er == 1) {
		return 0; //no ai
	}

	if (er != 0) {
		DBG_ERR("ai er=%d\r\n", er);
		return er;
	}

	STORAGE_OBJ *pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_ALL);
	if (pStrg == NULL) {
		DBG_ERR("pStrg is NULL.\n");
		return -1;
	}

	pStrg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&blksize, 0);
	if (blksize == 0) {
		DBG_ERR("blksize is 0.\n");
		return -1;
	}

	unsigned int blkcnt = ALIGN_CEIL(read_size, blksize) / blksize;

#if 0 //calc check sum
	pStrg->Lock();
	pStrg->Open();
	pStrg->RdSectors((INT8 *)read_buf, (UINT32)(read_ofs / blksize), (UINT32)(blkcnt));
	pStrg->Close();
	pStrg->Unlock();
	DBGD(MemCheck_CalcCheckSum16Bit((UINT32)read_buf, (UINT32)read_size));
	memset((INT8 *)read_buf, 0, read_size);
	vos_cpu_dcache_sync((VOS_ADDR)read_buf, read_size, VOS_DMA_TO_DEVICE);
#endif

	er = storage_partition_read_part(PARTITION_PATH_AI, (unsigned char *)read_buf, 0, (int)(blksize * blkcnt));
	if (er != 0) {
		return er;
	}

	vos_perf_list_mark("ai", __LINE__, 1);

	#if (DUMP_AI_MODEL)
	dump_buffer((char *)read_buf, 256);
	#endif

	DBG_DUMP("[ai]modelbuf=0x%lx,size=0x%lx\r\n", (unsigned long)read_buf, (unsigned long)read_size);
	g_detect_hdl.model_size = (unsigned int)read_size;
	return 0;
}

int operation_ai_set_img(uintptr_t y, uintptr_t uv, unsigned int lofs_y, unsigned int w, unsigned int h, unsigned int fmt)
{
	if (g_detect_hdl.ai_det_model_exist == 0) {
		return 0; //no ai
	}

	if (g_detect_hdl.image_init == FALSE) {
		g_detect_hdl.src_y = y;
		if (g_detect_hdl.img_y == 0) {
			DBG_ERR("[ai][img] not alloc gximage buffer\r\n");
			return -1;
		}
#if (AI_METHOD == 3) || (AI_METHOD == 4)
		uintptr_t img_y = (g_detect_hdl.img_y) ? g_detect_hdl.img_y : y;
		g_detect_hdl.input_image.width    = AI_VDO_W;
		g_detect_hdl.input_image.height   = AI_VDO_H;
		g_detect_hdl.input_image.channel  = 2;
		g_detect_hdl.input_image.line_ofs = AI_VDO_W;
		g_detect_hdl.input_image.fmt      = fmt;
		g_detect_hdl.input_image.pa       = img_y;
		g_detect_hdl.input_image.va       = img_y; //rtos
		g_detect_hdl.input_image.sign     = MAKEFOURCC('A', 'B', 'U', 'F');
		g_detect_hdl.input_image.size     = AI_VDO_W * AI_VDO_H * 3 / 2;
		DBG_IND("[ai][img]w=%d,h=%d,fmt=0x%x,addr=0x%lx,loff=%d,size=0x%lx\r\n",
			(int)g_detect_hdl.input_image.width, (int)g_detect_hdl.input_image.height, (int)g_detect_hdl.input_image.fmt,
			(unsigned long)g_detect_hdl.input_image.pa, (int)g_detect_hdl.input_image.line_ofs, g_detect_hdl.input_image.size);
#if (DUMP_AI_IMAGE)
		dump_buffer((char *)img_y, 128);
#endif
#endif

#if (AI_METHOD == 4)
		g_detect_hdl.src_image.width    = w;
		g_detect_hdl.src_image.height   = h;
		g_detect_hdl.src_image.line_ofs = lofs_y;
		g_detect_hdl.src_image.fmt      = fmt;
		g_detect_hdl.src_image.pa       = y;
		g_detect_hdl.src_image.va       = y; //rtos
		g_detect_hdl.src_image.sign     = MAKEFOURCC('A', 'B', 'U', 'F');
		g_detect_hdl.src_image.size     = lofs_y * h * 3 / 2;
#endif
		g_detect_hdl.image_init = TRUE;
	}
	return 0;
}

int operation_sys_init_ai(int net_num)
{
#if _TODO_FAST2A
	HD_RESULT ret;

	// init ai
#if (AI_NEW_VER)
	extern INT32 kdrv_ai_drv_set_net_supported_num(UINT32 net_num);
	kdrv_ai_drv_set_net_supported_num(16); // config net num
#endif
	nvt_ai_drv_init_rtos();

	// init hdal
#if !(_PACKAGE_VIDEO_)
	ret = hd_common_init_lite(0);
#else
	ret = hd_common_init(0);
#endif
	if (ret != HD_OK) {
		DBG_ERR("common fail=%d\n", ret);
		return -1;
	}

	//set project config for AI
#if !(_PACKAGE_VIDEO_)
	hd_common_sysconfig_lite(0, (1 << 16), 0, VENDOR_AI_CFG); //enable AI engine
#else
	hd_common_sysconfig(0, (1 << 16), 0, VENDOR_AI_CFG); //enable AI engine
#endif

	//init gfx for img
#if !(_PACKAGE_VIDEO_)
	ret = hd_gfx_init_lite();
#else
	ret = hd_gfx_init();
#endif
	if (ret != HD_OK) {
		DBG_ERR("hd_gfx_init fail=%d\n", ret);
		return -1;
	}

	// config extend engine plugin, process scheduler
	UINT32 schd = VENDOR_AI_PROC_SCHD_FAIR;
	/* remove cpu plugin for codesize reduction */
	/* vendor_ai_cfg_set(VENDOR_AI_CFG_PLUGIN_ENGINE, vendor_ai_cpu1_get_engine()); */
	vendor_ai_cfg_set(VENDOR_AI_CFG_PROC_SCHD, &schd);
	if ((ret = vendor_ai_init()) != HD_OK) {
		DBG_ERR("vendor_ai_init fail=%d\n", ret);
		return -1;
	}
#endif
	return 0;
}

int operation_sys_init_mem(void)
{
	int ret;

	//mem init
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	ret = parse_fboot_hdal_mem_range(&mem_cfg);
	if (ret != HD_OK) {
		DBG_ERR("parse hdal mem fail=%d\n", ret);
		return -1;
	}
#if !(_PACKAGE_VIDEO_)
	ret = hd_common_mem_init_lite(&mem_cfg);
	if (ret != HD_OK) {
		DBG_ERR("mem_lite fail=%d\n", ret);
		return -1;
	}
#else
	ret = hd_common_mem_init(&mem_cfg);
	if (ret != HD_OK) {
		DBG_ERR("mem fail=%d\n", ret);
		return -1;
	}
#if (DUMP_AI_MEMORY)
	dump_mem_range();
#endif
#endif

	// nn get block ---
	UINTPTR                     pa, va;
	HD_COMMON_MEM_VB_BLK        blk;
#if !(_PACKAGE_VIDEO_) || (POWERON_MODE == POWERON_MODE_PREROLL)
	blk = hd_common_mem_get_block_lite(HD_COMMON_MEM_CNN_POOL, mem_select(), DDR_ID0);
#else
	blk = hd_common_mem_get_block(HD_COMMON_MEM_CNN_POOL, mem_select(), DDR_ID0);
#endif
	if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
		DBG_ERR("hd_common_mem_get_block fail\r\n");
		return -1;
	}
#if !(_PACKAGE_VIDEO_) || (POWERON_MODE == POWERON_MODE_PREROLL)
	pa = hd_common_mem_blk2pa_lite(blk);
#else
	pa = hd_common_mem_blk2pa(blk);
#endif
	if (pa == 0) {
		DBG_ERR("not get buffer, pa=%08lx\r\n", (unsigned long)pa);
		return -1;
	}
	va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, mem_select());
	if (va == 0) {
		ret = hd_common_mem_munmap((void *)va, mem_select());
		if (ret != HD_OK) {
			DBG_ERR("mem unmap fail\r\n");
			return ret;
		}
		return -1;
	}

	DBG_DUMP("[ai][sys]detect_buf=0x%lx\r\n", (unsigned long)va);
	g_detect_hdl.detect_buf = (unsigned int)va;

#if (CFG_LOAD_AI_DETECT)
	// allocate mem for gximage
	if (g_detect_hdl.img_y == 0) {
		void *va;
		uintptr_t addr;
		ret = hd_common_mem_alloc("fast_ai", &addr, (void **)&va, (AI_VDO_W * AI_VDO_H * 2), DDR_ID0);
		if (ret != HD_OK) {
			printf("err1:alloc size 0x%x, ddr %d\r\n", (AI_VDO_W * AI_VDO_H * 2), DDR_ID0);
			return 0;
		}
		vos_cpu_dcache_sync((VOS_ADDR)addr, (AI_VDO_W * AI_VDO_H * 2), VOS_DMA_BIDIRECTIONAL);
		g_detect_hdl.img_y = (uintptr_t)addr;
		// not memcpy, use ise_scale later
	}
#endif
	return 0;
}

#if (AI_METHOD == 3)
static HD_RESULT pvdcnn_preset(PVDCNN_MEM *pvdcnn_mem, VENDOR_AIS_FLOW_MEM_PARM *buf, PVD_PROPOSAL_PARAM *pvd_proposal_params)
{
	HD_RESULT ret;
	UINT32 proc_id = pvd_proposal_params->run_id;
	DBG_DUMP("proc_id=%d\r\n", proc_id);

	VENDOR_AIS_FLOW_MEM_PARM pvdmodel_buf = {0};
	pvdmodel_buf.pa = g_detect_hdl.detect_buf;
	pvdmodel_buf.va = g_detect_hdl.detect_buf;
	pvdmodel_buf.size = PVD_MODEL_SIZE + LIMIT_FDET_MODEL_SIZE; //for offset

	VENDOR_AI_NET_CFG_BUF_OPT cfg_buf_opt = {0};
	cfg_buf_opt.method = VENDOR_AI_NET_BUF_OPT_NONE; //VENDOR_AI_NET_BUF_OPT_SHRINK_O1;
	cfg_buf_opt.ddr_id = DDR_ID0;
	vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_BUF_OPT, &cfg_buf_opt);

	VENDOR_AI_NET_CFG_JOB_OPT cfg_job_opt = {0};
	//#if (STORE_AI_IMAGE)
	cfg_job_opt.method = VENDOR_AI_NET_JOB_OPT_LINEAR;
	cfg_job_opt.wait_ms = -1;
	//#else
	//cfg_job_opt.method = VENDOR_AI_NET_JOB_OPT_LINEAR_O1;
	//cfg_job_opt.wait_ms = 0;
	//#endif
	cfg_job_opt.schd_parm = VENDOR_AI_FAIR_CORE_ALL; //FAIR dispatch to ALL core
	vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_JOB_OPT, &cfg_job_opt);

	UINT32 model_size = (UINT32)PVD_MODEL_SIZE + LIMIT_FDET_MODEL_SIZE; //get from dtsi
	if (model_size <= 0) {
		DBG_ERR("pvdcnn input model not exist\r\n");
		return HD_ERR_NOT_OPEN;
	}
	DBG_DUMP("[model_mem]size=0x%lx\r\n", (unsigned long)model_size);
	ret = get_pvd_mem(&pvdmodel_buf, &(pvdcnn_mem->model_mem), model_size, 32);
	if (ret != HD_OK) {
		DBG_ERR("pvdcnn get model mem fail (%d)!!\r\n", ret);
		return ret;
	}
	DBG_DUMP("[model_mem]addr=0x%lx,size=0x%lx\r\n", (unsigned long)pvdcnn_mem->model_mem.va, (unsigned long)model_size);

	//ret = load_model(pvd_model_name, pvdcnn_mem->model_mem.va);
	///=> not need to load_model
	//#if (DUMP_AI_MODEL)
	//dump_buffer((char *)pvdcnn_mem->model_mem.va, 256);
	//#endif

	pvdcnn_get_version();
	ret = pvdcnn_version_check(&(pvdcnn_mem->model_mem));
	if (ret != HD_OK) {
		DBG_ERR("pvdcnn version check fail (%d)!!\r\n", ret);
		return ret;
	}

	// set model
	ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_MODEL, &(pvdcnn_mem->model_mem));
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_net_set fail (%d)!!\r\n", ret);
		return ret;
	}
	// open
	ret = vendor_ai_net_open(proc_id);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai_net_open fail (%d)!!\r\n", ret);
		return ret;
	}
#if (DUMP_AI_MEMORY)
	dump_mem_range();
#endif

	//set work buf and assign pvdcnn mem
	VENDOR_AI_NET_CFG_WORKBUF wbuf = {0};
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &wbuf);
	if (ret != HD_OK) {
		DBG_ERR("pvdcnn get VENDOR_AI_NET_PARAM_CFG_WORKBUF fail\r\n");
		return ret;
	}
	DBG_DUMP("[io_mem]size=0x%lx\r\n", (unsigned long)wbuf.size);
	ret = get_pvd_mem(buf, &(pvdcnn_mem->io_mem), wbuf.size, 32);
	if (ret != HD_OK) {
		DBG_ERR("pvdcnn get io_mem fail (%d)!!\r\n", ret);
		return ret;
	}
	DBG_DUMP("[io_mem]addr=0x%lx,size=0x%lx\r\n", (unsigned long)pvdcnn_mem->io_mem.va, (unsigned long)wbuf.size);

#if MAX_DISTANCE_MODE
	/*get max distance buf*/
	ret = get_pvd_mem(buf, &(pvdcnn_mem->scale_buf), (PVD_YUV_WIDTH * PVD_YUV_HEIGHT * 3 / 2), 32);
	if (ret != HD_OK) {
		DBG_ERR("pvdcnn get scale_buf fail (%d)!!\r\n", ret);
		return ret;
	}
#endif
	ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &(pvdcnn_mem->io_mem));
	if (ret != HD_OK) {
		DBG_ERR("pvdcnn set VENDOR_AI_NET_PARAM_CFG_WORKBUF fail (%d)\r\n", ret);
		return ret;
	}
	return HD_OK;
}
static HD_RESULT limit_init(LIMIT_FDET_MEM *limit_fdet_mem, LIMIT_FDET_PARAM *limit_fdet_params, VENDOR_AIS_FLOW_MEM_PARM pred_cls_input_info, VENDOR_AIS_FLOW_MEM_PARM *pred_cls_output_info)
{
	HD_RESULT ret;

	unsigned int detect_buf = g_detect_hdl.detect_buf;
	unsigned int shift;
	// | 0x128680      | 0x8A2C0         | 0x2C1CA0   | 0x9BAE0      |
	// |---------------|-----------------|------------|--------------|
	// | pvd model bin | limit model bin | pvdcnn buf | limitcnn buf |
	shift = PVD_MODEL_SIZE;
	VENDOR_AIS_FLOW_MEM_PARM limitmodel_buf = {0};
	limitmodel_buf.pa = detect_buf + shift;
	limitmodel_buf.va = detect_buf + shift;
	limitmodel_buf.size = LIMIT_FDET_MODEL_SIZE;
	shift = PVD_MODEL_SIZE + LIMIT_FDET_MODEL_SIZE + (PVD_BASE_SIZE - PVD_MODEL_SIZE);
	VENDOR_AIS_FLOW_MEM_PARM limitcnn_buf = {0};
	limitcnn_buf.pa = detect_buf + shift;
	limitcnn_buf.va = detect_buf + shift;
	limitcnn_buf.size = LIMIT_FDET_BASE_SIZE - LIMIT_FDET_MODEL_SIZE;
	shift = LIMIT_FDET_BASE_SIZE - LIMIT_FDET_MODEL_SIZE;

	limit_fdet_params->run_id = 3;
	limit_fdet_params->cls_det_threshold = 0.993;
	limit_fdet_params->cls_threshold = 0.7;
	limit_fdet_params->cls_debug_print = 0;
	limit_fdet_params->ratiow = 1.0; //(FLOAT)src_img.width  / (FLOAT)src_img.width;      //(FLOAT)PD_YUV_WIDTH;
	limit_fdet_params->ratioh = 1.0; //(FLOAT)src_img.height / (FLOAT)src_img.height;     //(FLOAT)PD_YUV_HEIGHT;
	limit_fdet_params->cls_id = 6;
	limit_fdet_params->pvdcls = 1;
	UINT32 size_final_predx = 100 * sizeof(PVDCNN_RESULT) + 32;
	ret = get_pvd_mem(&limitcnn_buf, pred_cls_output_info, size_final_predx, 32);
	limit_fdet_mem->det_in_mem = pred_cls_input_info;      //pvdcnn_mem.out_result;
	limit_fdet_mem->det_out_mem = *pred_cls_output_info;

	// ai init
	VENDOR_AI_NET_CFG_BUF_OPT cfg_buf_opt = {0};
	cfg_buf_opt.method = VENDOR_AI_NET_BUF_OPT_NONE;//VENDOR_AI_NET_BUF_OPT_SHRINK_O1;
	cfg_buf_opt.ddr_id = DDR_ID0;
	vendor_ai_net_set(limit_fdet_params->run_id, VENDOR_AI_NET_PARAM_CFG_BUF_OPT, &cfg_buf_opt);

	VENDOR_AI_NET_CFG_JOB_OPT cfg_job_opt = {0};
	cfg_job_opt.method = VENDOR_AI_NET_JOB_OPT_LINEAR;//VENDOR_AI_NET_JOB_OPT_LINEAR_O1;
	cfg_job_opt.wait_ms = -1;//0;
	cfg_job_opt.schd_parm = VENDOR_AI_FAIR_CORE_ALL; //FAIR dispatch to ALL core
	vendor_ai_net_set(limit_fdet_params->run_id, VENDOR_AI_NET_PARAM_CFG_JOB_OPT, &cfg_job_opt);

	ret = get_limit_fdet_mem(&limitcnn_buf, &(limit_fdet_mem->input_mem), LIMIT_FDET_WIDTH * LIMIT_FDET_WIDTH * 3 / 2, 32);
	if (ret != HD_OK) {
		printf("ERR: limit_fdet get input_mem fail (%d)!!\n", ret);
		return ret;
	}

	unsigned int limit_net_size = 565924;
	ret = get_limit_fdet_mem(&limitmodel_buf, &(limit_fdet_mem->model_mem), limit_net_size, 32);
	if (ret != HD_OK) {
		printf("ERR: limit_fdet get model mem fail (%d)!!\n", ret);
		return ret;
	}

	///=> not need to load_model
#if (DUMP_AI_MODEL)
	dump_buffer((char *)limit_fdet_mem->model_mem.va, 256);
#endif

	ret = limit_fdet_version_check(&(limit_fdet_mem->model_mem));
	if (ret != HD_OK) {
		printf("ERR: limit_fdet version check fail (%d)!!\n", ret);
		return ret;
	}

	// set model
	ret = vendor_ai_net_set(limit_fdet_params->run_id, VENDOR_AI_NET_PARAM_CFG_MODEL, &(limit_fdet_mem->model_mem));
	if (ret != HD_OK) {
		printf("ERR: limit model vendor_ai_net_set fail (%d)!!\n", ret);
		return ret;
	}
	// open
	ret = vendor_ai_net_open(limit_fdet_params->run_id);
	if (ret != HD_OK) {
		printf("ERR: limit model vendor_ai_net_open fail (%d)!!\n", ret);
		return ret;
	}

	if (ret != HD_OK) {
		printf("ERR: limit_fdet open fail !!\n");
		return ret;
	}

	//set cls work buf
	VENDOR_AI_NET_CFG_WORKBUF limit_fdet_wbuf = {0};
	ret = vendor_ai_net_get(limit_fdet_params->run_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &limit_fdet_wbuf);
	if (ret != HD_OK) {
		printf("ERR: limit_fdet get VENDOR_AI_NET_PARAM_CFG_WORKBUF fail (%d)\n", ret);
		return ret;
	}

	ret = get_limit_fdet_mem(&limitcnn_buf, &(limit_fdet_mem->io_mem), limit_fdet_wbuf.size, 32);
	if (ret != HD_OK) {
		printf("ERR: limit_fdet get io_mem fail (%d)!!\n", ret);
		return ret;
	}

	ret = vendor_ai_net_set(limit_fdet_params->run_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &(limit_fdet_mem->io_mem));
	if (ret != HD_OK) {
		printf("ERR: limit_fdet set VENDOR_AI_NET_PARAM_CFG_WORKBUF fail (%d)\n", ret);
		return ret;
	}
	//printf("cls model_mem: %ld, cls work buf: %ld, model_mem+work_buf= %ld\n", limit_net_size,  limit_fdet_wbuf.size, limit_net_size + limit_fdet_wbuf.size);

	ret = vendor_ai_net_start(limit_fdet_params->run_id);
	if (HD_OK != ret) {
		printf("ERR: limit_fdet start fail!!\n");
		return ret;
	}
	return HD_OK;
}
#endif

int operation_cnn_init(int net_num)
{
#if (AI_METHOD == 3)
	//unsigned int detect_buf = g_detect_hdl.detect_buf;
	// | 0x128680      | 0x8A2C0         | 0x2C1CA0   | 0x9BAE0      |
	// |---------------|-----------------|------------|--------------|
	// | pvd model bin | limit model bin | pvdcnn buf | limitcnn buf |
	unsigned int detect_buf = g_detect_hdl.detect_buf;
	HD_RESULT ret = HD_OK;
	VENDOR_AIS_FLOW_MEM_PARM pvdcnn_buf = {0};
	pvdcnn_buf.pa = detect_buf + PVD_MODEL_SIZE + LIMIT_FDET_MODEL_SIZE;
	pvdcnn_buf.va = detect_buf + PVD_MODEL_SIZE + LIMIT_FDET_MODEL_SIZE;
	pvdcnn_buf.size = PVD_BASE_SIZE - PVD_MODEL_SIZE;
	PVDCNN_MEM pvdcnn_mem = {0};
	PVD_PROPOSAL_PARAM *p_pvd_proposal_params = (PVD_PROPOSAL_PARAM *) & (g_detect_hdl.pvd_proposal_params);
	p_pvd_proposal_params->run_id = 4; //0;
	if ((ret = pvdcnn_preset(&pvdcnn_mem, &pvdcnn_buf, p_pvd_proposal_params)) != HD_OK) {
		DBG_ERR("pvd_preset fail=%d\n", (int)ret);
		return 0;
	}
	memcpy((void *)&g_detect_hdl.pvdcnn_mem, (void *)&pvdcnn_mem, sizeof(pvdcnn_mem));

	// start pdcnn network
	if ((ret = vendor_ai_net_start(p_pvd_proposal_params->run_id)) != HD_OK) {
		DBG_ERR("vendor_ai_net_start fail=%d\n", (int)ret);
		return 0;
	}

	// get outlayer
	CHAR out_layer_name[2][128] = { "layer115-conv", "layer125-conv" };
	UINT32 outlayer_path_list[256] = { 0 };
	UINT32 outlayer_path_list_tmp[256] = { 0 };
	UINT32 outlayer_num = 0;
	VENDOR_AI_NET_INFO net_info = {0};
	if ((ret = vendor_ai_net_get(p_pvd_proposal_params->run_id, VENDOR_AI_NET_PARAM_INFO, &net_info)) != HD_OK) {
		DBG_ERR("vendor_ai_net_get fail=%d\n", (int)ret);
		return 0;
	}
	outlayer_num = net_info.out_buf_cnt;
	g_detect_hdl.outlayer_num = outlayer_num;
	DBG_DUMP("outlayer_num=%d\r\n", outlayer_num);
	if ((ret = vendor_ai_net_get(p_pvd_proposal_params->run_id, VENDOR_AI_NET_PARAM_OUT_PATH_LIST, outlayer_path_list_tmp)) != HD_OK) {
		DBG_ERR("get outlayer_path_list fail=%d\n", (int)ret);
		return 0;
	}
	memcpy(outlayer_path_list, outlayer_path_list_tmp, sizeof(UINT32) * outlayer_num);
	for (UINT32 i = 0; i < outlayer_num; i++) {
		VENDOR_AI_BUF layer_tmp;
		// get out buf by path_id
		if ((ret = vendor_ai_net_get(p_pvd_proposal_params->run_id, outlayer_path_list_tmp[i], &layer_tmp)) != HD_OK) {
			DBG_ERR("pvd get AI_OUTBUF fail !!\n");
			return 0;
		}
		for (UINT32 k = 0; k < outlayer_num; k++) {
			if (strncmp(out_layer_name[k], layer_tmp.name, strlen(out_layer_name[k])) == 0) {
				outlayer_path_list[k] = outlayer_path_list_tmp[i];
				break;
			}
		}
	}

	VENDOR_AIS_FLOW_MEM_PARM layer_buffer_fix;
	ret = get_pvd_mem(&pvdcnn_buf, &layer_buffer_fix, outlayer_num * sizeof(VENDOR_AI_BUF), 32);
	DBG_DUMP("layer_buffer=0x%lx,size=0x%lx\r\n", (unsigned long)layer_buffer_fix.va, (unsigned long)outlayer_num * sizeof(VENDOR_AI_BUF));
	VENDOR_AI_BUF *layer_buffer = (VENDOR_AI_BUF *)layer_buffer_fix.va;
	VENDOR_AIS_FLOW_MEM_PARM out_layer_float_fix;
	ret = get_pvd_mem(&pvdcnn_buf, &out_layer_float_fix, outlayer_num * sizeof(FLOAT), 32);
	DBG_DUMP("out_layer_float_fix=0x%lx,size=0x%lx\r\n", (unsigned long)out_layer_float_fix.va, (unsigned long)outlayer_num * sizeof(FLOAT));
	FLOAT **out_layer_float = (FLOAT **)out_layer_float_fix.va;
	UINT32 outlayer_bufsize = 0;
	INT32 max_map[2] = {0};
	for (UINT32 i = 0; i < outlayer_num; i++) {
		ret = vendor_ai_net_get(p_pvd_proposal_params->run_id, outlayer_path_list[i], &(layer_buffer[i]));
		if (HD_OK != ret) {
			printf("pvd get AI_OUTBUF fail !!\n");
			break;
		}
		DBG_DUMP("[%d]width=0x%x,height=0x%x,channel=0x%x,batch_num=0x%x\r\n", i, layer_buffer[i].width, layer_buffer[i].height, layer_buffer[i].channel, layer_buffer[i].batch_num);
		UINT32 length = layer_buffer[i].width * layer_buffer[i].height * layer_buffer[i].channel * layer_buffer[i].batch_num;
		VENDOR_AIS_FLOW_MEM_PARM out_layer_float_fix_fix;
		ret = get_pvd_mem(&pvdcnn_buf, &out_layer_float_fix_fix, sizeof(FLOAT) * length, 32);
		DBG_DUMP("out_layer_float_fix_fix=0x%lx,size=0x%lx\r\n", (unsigned long)out_layer_float_fix_fix.va, (unsigned long)sizeof(FLOAT) * length);
		out_layer_float[i] = (FLOAT *)out_layer_float_fix_fix.va;
		outlayer_bufsize += sizeof(FLOAT) * length;
		if (i == outlayer_num - 1) {
			max_map[0] = (INT32)layer_buffer[i].width;
			max_map[1] = (INT32)layer_buffer[i].height;
		}
	}
	g_detect_hdl.layer_buffer = layer_buffer_fix.va;
	g_detect_hdl.out_layer_float = out_layer_float_fix.va;

	//pvd param
	PVD_LAYER_PARAM *p_pvd_params = (PVD_LAYER_PARAM *) & (g_detect_hdl.PVD_params);
#if 1
	p_pvd_params->run_id = p_pvd_proposal_params->run_id;
#endif
	p_pvd_params->nms_threshold = 0.5;
	p_pvd_params->confindence_threshold = 0.38;
	p_pvd_params->pd_limit_param.limit_module = 2;
	//p_pvd_params->vd_limit_param.limit_module = 2;
	p_pvd_params->pd_limit_param.sm_thr_num = 2;
	//p_pvd_params->vd_limit_param.sm_thr_num = 2;
	p_pvd_params->cls_score_thresh = 0.98;
	// pvd detection out layer include params! default
	p_pvd_params->stable_bbox_param.TRACKER_ENBALE = FALSE;
	pvdcnn_getparam(p_pvd_params, max_map, NULL);
	INT8 pvd_light_night = 9; // >0 light;    <0 night
	pdcnn_para_init(p_pvd_params, pvd_light_night); //
	//vdcnn_para_init(p_pvd_params, pvd_light_night); //
	if (pvd_light_night < 0) { //night
		p_pvd_params->cls_score_thresh = 1.0;
		printf("pvd dynamic threshold is in night model now\n");
	}

	// confindence_threshold: PVD ��X > confindence_threshold �~�|��X�� limit �ҫ�����
	// cls_score_thresh: PVD ��X > cls_score_thresh �N���������������� PD, ���� limit model

	PVD_LAYER_MEM *p_pvd_det_mem = (PVD_LAYER_MEM *) & (g_detect_hdl.PVD_det_mem);
	VENDOR_AIS_FLOW_MEM_PARM size_swap_data_fix;
	ret = get_pvd_mem(&pvdcnn_buf, &size_swap_data_fix, p_pvd_params->size_swap_data, 32);
	DBG_DUMP("size_swap_data_fix=0x%lx,size=0x%lx\r\n", (unsigned long)size_swap_data_fix.va, (unsigned long)p_pvd_params->size_swap_data);
	p_pvd_det_mem->swap_data = (FLOAT *)size_swap_data_fix.va;
	VENDOR_AIS_FLOW_MEM_PARM size_class_score_fix;
	ret = get_pvd_mem(&pvdcnn_buf, &size_class_score_fix, p_pvd_params->size_class_score, 32);
	DBG_DUMP("size_class_score_fix=0x%lx,size=0x%lx\r\n", (unsigned long)size_class_score_fix.va, (unsigned long)p_pvd_params->size_class_score);
	p_pvd_det_mem->class_score = (FLOAT *)size_class_score_fix.va;
	VENDOR_AIS_FLOW_MEM_PARM size_predicts_init_fix;
	ret = get_pvd_mem(&pvdcnn_buf, &size_predicts_init_fix, p_pvd_params->size_predicts_init, 32);
	DBG_DUMP("size_predicts_init_fix=0x%lx,size=0x%lx\r\n", (unsigned long)size_predicts_init_fix.va, (unsigned long)p_pvd_params->size_predicts_init);
	p_pvd_det_mem->predicts_init = (PVD_PREDICT *)size_predicts_init_fix.va;
	VENDOR_AIS_FLOW_MEM_PARM size_predicts_initvd_fix;
	ret = get_pvd_mem(&pvdcnn_buf, &size_predicts_initvd_fix, p_pvd_params->size_predicts_init, 32);
	DBG_DUMP("size_predicts_initvd_fix=0x%lx,size=0x%lx\r\n", (unsigned long)size_predicts_initvd_fix.va, (unsigned long)p_pvd_params->size_predicts_init);
	p_pvd_det_mem->predicts_initVD = (PVD_PREDICT *)size_predicts_initvd_fix.va;
	VENDOR_AIS_FLOW_MEM_PARM size_final_pred_fix;
	ret = get_pvd_mem(&pvdcnn_buf, &size_final_pred_fix, p_pvd_params->size_final_pred, 32);
	DBG_DUMP("size_final_pred_fix=0x%lx,size=0x%lx\r\n", (unsigned long)size_final_pred_fix.va, (unsigned long)p_pvd_params->size_final_pred);
	p_pvd_det_mem->final_pred = (PVDCNN_RESULT *)size_final_pred_fix.va;
	memset(p_pvd_det_mem->swap_data, 0, p_pvd_params->size_swap_data);
	memset(p_pvd_det_mem->class_score, 0, p_pvd_params->size_class_score);
	memset(p_pvd_det_mem->predicts_init, 0, p_pvd_params->size_predicts_init);
	memset(p_pvd_det_mem->final_pred, 0, p_pvd_params->size_final_pred);

	VENDOR_AIS_FLOW_MEM_PARM pred_pvd_output_info = {0};
	pred_pvd_output_info.pa = size_final_pred_fix.pa;
	pred_pvd_output_info.va = size_final_pred_fix.va;
	pred_pvd_output_info.size = size_final_pred_fix.size;

	// cls model initial
	LIMIT_FDET_PARAM *p_limit_fdet_params = (LIMIT_FDET_PARAM *) & (g_detect_hdl.limit_fdet_params);
	LIMIT_FDET_MEM *p_limit_fdet_mem = (LIMIT_FDET_MEM *) & (g_detect_hdl.limit_fdet_mem);
	VENDOR_AIS_FLOW_MEM_PARM *p_pred_cls_output_info = (VENDOR_AIS_FLOW_MEM_PARM *) & (g_detect_hdl.pred_cls_output_info);
	ret = limit_init(p_limit_fdet_mem, p_limit_fdet_params, pred_pvd_output_info, p_pred_cls_output_info);
	if (ret != HD_OK) {
		printf("ERR: limit_init fail (%d)!!\n", ret);
		return 0;
	}
#endif
	DBG_DUMP("[ai][cnn]-done\r\n");
	return 0;
}

int operation_ai_detect(int net_num)
{
	int wait = 1;

	vos_perf_list_mark("de", __LINE__, 0);

	/**
	 * operation_sys_init_ai needs many time ms to init
	 * so run operation_sys_init_ai() first and then check if ai exist
	 */

	if (0 != operation_sys_init_ai(net_num)) {
		printf("operation_sys_init_ai failed\r\n");
		goto exit_detect;
	}

	fastboot_wait_done(BOOT_FLOW_AI_DET_MODEL);

	if (g_detect_hdl.ai_det_model_exist == 0) {
		return 0; //no ai
	}

	if (0 != operation_sys_init_mem()) {
		printf("operation_sys_init_mem failed\r\n");
		goto exit_detect;
	}

	if (wait) {
		while (g_detect_hdl.detect_buf == 0 || g_detect_hdl.model_size == 0) {
			vos_util_delay_ms(1);
		}
	}
	if (g_detect_hdl.detect_buf == 0 || g_detect_hdl.model_size == 0) {
		goto exit_detect;
	}

	if (0 != operation_cnn_init(net_num)) {
		printf("cnn_init failed\r\n");
		goto exit_detect;
	}

	// extern void libc_heap_dump(void);
	// libc_heap_dump();

	if (wait) {
		while (g_detect_hdl.image_init == FALSE) {
			vos_util_delay_ms(1);
		}
	}
	if (g_detect_hdl.image_init == FALSE) {
		goto exit_detect;
	}

#if (AI_METHOD == 3)
	// use ise_scale after image_init
	ise_scale(g_detect_hdl.img_y, AI_VDO_W, AI_VDO_W, AI_VDO_H, g_detect_hdl.src_y, 1920, 1920, 1080, KDRV_ISE_Y8, KDRV_ISE_INTEGRATION);
	ise_scale(g_detect_hdl.img_y + AI_VDO_W * AI_VDO_H, AI_VDO_W, AI_VDO_W, AI_VDO_H >> 1, g_detect_hdl.src_y + 1920 * 1080, 1920, 1920, 1080 >> 1, KDRV_ISE_UVP, KDRV_ISE_INTEGRATION);
#elif (AI_METHOD == 4)
	ise_scale(g_detect_hdl.img_y,
				AI_VDO_W,
				AI_VDO_W,
				AI_VDO_H,
				g_detect_hdl.src_y,
				g_detect_hdl.src_image.line_ofs,
				g_detect_hdl.src_image.width,
				g_detect_hdl.src_image.height,
				ISE_DRV_Y8_ONLY,
				ISE_DRV_SCALE_METHOD_ISD);
	ise_scale(g_detect_hdl.img_y + AI_VDO_W * AI_VDO_H,
				AI_VDO_W,
				AI_VDO_W >> 1,
				AI_VDO_H >> 1,
				g_detect_hdl.src_image.pa + g_detect_hdl.src_image.line_ofs * g_detect_hdl.src_image.height,
				g_detect_hdl.src_image.line_ofs,
				g_detect_hdl.src_image.width >> 1,
				g_detect_hdl.src_image.height >> 1,
				ISE_DRV_UVP,
				ISE_DRV_SCALE_METHOD_ISD);
#endif

	vos_perf_list_mark("de", __LINE__, 1);

#if (STORE_AI_IMAGE)
	DBG_DUMP("wait filesys-b\r\n");
	vos_util_delay_ms(1200);
	DBG_DUMP("wait filesys-e\r\n");
	DBG_DUMP("[ai][img]img_y=0x%lx\r\n", (unsigned long)g_detect_hdl.input_image.va);
	store_img((char *)g_detect_hdl.input_image.va, g_detect_hdl.input_image.size, 0);
#endif

#if (DEBUG_AI_MWP)
	UINT32 wp_det_addr1 = (UINT32)(g_detect_hdl.detect_buf);
	UINT32 wp_det_size1 = (UINT32)(mem_select());
	fast_ai_write_protect(wp_det_addr1, wp_det_size1, WPSET_0, DMA_PROT_IN);
	vos_util_delay_ms(100);
	UINT32 wp_det_addr2 = (UINT32)(g_detect_hdl.detect_buf);
	UINT32 wp_det_size2 = (UINT32)(mem_select());
	fast_ai_write_protect(wp_det_addr2, wp_det_size2, WPSET_1, DMA_PROT_OUT);
	vos_util_delay_ms(100);
#endif

#if (AI_METHOD == 3)
	HD_RESULT ret;
	PVDCNN_MEM *p_pvdcnn_mem = (PVDCNN_MEM *) & (g_detect_hdl.pvdcnn_mem);
	PVD_PROPOSAL_PARAM *p_proposal_params = (PVD_PROPOSAL_PARAM *) & (g_detect_hdl.pvd_proposal_params);
	UINT32 ai_pd_frame = 0;
	UINT32 outlayer_num = g_detect_hdl.outlayer_num;
	PVD_LAYER_PARAM *p_pvd_params = (PVD_LAYER_PARAM *) & (g_detect_hdl.PVD_params);
	PVD_LAYER_MEM *p_pvd_det_mem = (PVD_LAYER_MEM *) & (g_detect_hdl.PVD_det_mem);
	VENDOR_AI_BUF *p_src_img = (VENDOR_AI_BUF *) & (g_detect_hdl.input_image);
	VENDOR_AI_BUF *layer_buffer = (VENDOR_AI_BUF *)g_detect_hdl.layer_buffer;
	FLOAT **out_layer_float = (FLOAT **)g_detect_hdl.out_layer_float;
	UINT32 pvdcnn_num = 0;
	LIMIT_FDET_PARAM *p_limit_fdet_params = (LIMIT_FDET_PARAM *) & (g_detect_hdl.limit_fdet_params);
	LIMIT_FDET_MEM *p_limit_fdet_mem = (LIMIT_FDET_MEM *) & (g_detect_hdl.limit_fdet_mem);
	VENDOR_AIS_FLOW_MEM_PARM *p_pred_cls_output_info = (VENDOR_AIS_FLOW_MEM_PARM *) & (g_detect_hdl.pred_cls_output_info);

#if (LOOP_AI_PROC)
	while (1) {
#endif

		/* execution */
		PVDCNN_RESULT *objs_info = p_pvd_det_mem->final_pred;
		if ((ret = pvdcnn_netprocess(p_proposal_params, p_pvdcnn_mem, p_src_img, layer_buffer, p_pvd_params,
						out_layer_float, p_pvd_det_mem, objs_info, ai_pd_frame, outlayer_num, (UINT32)MAX_DISTANCE_MODE, PVDCNN_PD_DET_SW)) != HD_OK) {
			DBG_ERR("failed to pvdcnn_netprocess, er=%d\n", (int)ret);
			goto stop_detext;
		}
		DBG_DUMP("[PVD1] ----------- num : %ld ----------- \n", p_pvdcnn_mem->out_num);

		/*
		 * detect time finetune:
		 * (1) check the highest one of pvdcnn result is lower cls_det_threshold or not.
		 * (2) config max_roi_num and change to use limit_fdet_process_getone to reduce ops.
		 */
		UINT32 max_roi_num = 3;
		p_limit_fdet_params->cls_det_threshold = 0.98;
		objs_info = (PVDCNN_RESULT *)p_limit_fdet_mem->det_in_mem.va;
		if (objs_info[0].score < p_limit_fdet_params->cls_det_threshold) { // the highest one

			p_limit_fdet_params->proc_num = p_pvdcnn_mem->out_num;
			if ((ret = limit_fdet_process_getone(p_limit_fdet_params, p_limit_fdet_mem, p_src_img, max_roi_num)) != HD_OK) {
				DBG_ERR("failed to limit_fdet_process, er=%d\n", (int)ret);
				goto stop_detext;
			}
			p_pvdcnn_mem->out_num = p_limit_fdet_params->proc_num;
			DBG_DUMP("[PVD2] ----------- num : %ld ----------- \n", p_pvdcnn_mem->out_num);

			objs_info = (PVDCNN_RESULT *)p_pred_cls_output_info->va;
			INT8 pvd_cls = 9;
			if (p_pvd_params->pd_limit_param.limit_module != 0) {
				p_pvdcnn_mem->out_num = dynamic_limit(p_pvdcnn_mem->out_num, &p_pvd_params->pd_limit_param, p_pvd_params->cls_score_thresh,
						pvd_cls, objs_info, 1, p_src_img->width, p_src_img->height);
			}
			DBG_DUMP("[PVD3] ----------- num : %ld ----------- \n", p_pvdcnn_mem->out_num);
		}

		/* result */
		pvdcnn_num = p_pvdcnn_mem->out_num;
		DBG_DUMP("[PVD] ----------- num : %ld ----------- \n", pvdcnn_num);

#if (LOOP_AI_PROC)
	}
#endif

	g_detect_hdl.detect_num = pvdcnn_num;
#elif (AI_METHOD == 4)
	HD_RESULT ret = 0;
	INT32 pvd_out_num = 0;
	UINT32 pvdcnn_debug = DISABLE;
	static NN_FILE_PATH pvd_files = { 0 };
	static PVD_ADJUSTABLE_PARAM pvd_params = {0};
	static NN_FILE_PATH limit_files = { 0 };
	static LIMIT_FDET_ADJUSTABLE_PARAM limit_fdet_params = {0};
	NN_RESULT_M *pvd_final_result = NULL;

	g_detect_hdl.pvd_model_mem.pa = g_detect_hdl.detect_buf;
	g_detect_hdl.pvd_model_mem.va = g_detect_hdl.pvd_model_mem.pa;
	g_detect_hdl.pvd_model_mem.size = ALIGN_CEIL(AI_PVD_MODEL_SIZE, 2048); // PVD model bin

	g_detect_hdl.limit_model_mem.pa = g_detect_hdl.pvd_model_mem.pa + g_detect_hdl.pvd_model_mem.size;
	g_detect_hdl.limit_model_mem.va = g_detect_hdl.pvd_model_mem.va + g_detect_hdl.pvd_model_mem.size;
	g_detect_hdl.limit_model_mem.size = ALIGN_CEIL(AI_LIMIT_FDET_MODEL_SIZE, 2048); // Limit fdet model bin

	g_detect_hdl.pvd_mem.pa = g_detect_hdl.limit_model_mem.pa + g_detect_hdl.limit_model_mem.size;
	g_detect_hdl.pvd_mem.va = g_detect_hdl.limit_model_mem.va + g_detect_hdl.limit_model_mem.size;
	g_detect_hdl.pvd_mem.size = AI_PVD_BUF_SIZE - ALIGN_CEIL(AI_PVD_MODEL_SIZE, 2048); // PVD working buffer

	g_detect_hdl.limit_fdet_mem.pa = g_detect_hdl.pvd_mem.pa + g_detect_hdl.pvd_mem.size;
	g_detect_hdl.limit_fdet_mem.va = g_detect_hdl.pvd_mem.va + g_detect_hdl.pvd_mem.size;
	g_detect_hdl.limit_fdet_mem.size = AI_LIMIT_FDET_BUF_SIZE - ALIGN_CEIL(AI_LIMIT_FDET_MODEL_SIZE, 2048); // Limit fdet model bin

	snprintf(pvd_files.model_file, PATH_LENGTH_M, "%s", "");
	snprintf(pvd_files.para_file, PATH_LENGTH_M, "%s", "");
	ret = pvdcnn_init(g_detect_hdl.pvd_mem, (UINT32)0, PVD_MAX_DISTANCE_MODE, &(g_detect_hdl.pvd_model_mem), (VOID *)NULL, &pvd_files, pvdcnn_debug);
	if (ret != HD_OK){
		printf("[pvdcnn] Failed to init pvdcnn \r\n");
		goto stop_detext;
	}

	ret = pvdcnn_params_get(g_detect_hdl.pvd_mem, &pvd_params);
	pvd_params.TRACKER_ENBALE = 0;
	pvd_params.confindence_threshold = 0.3;
	ret = pvdcnn_params_set(g_detect_hdl.pvd_mem, &pvd_params);

	// Limit Fdet Init
	snprintf(limit_files.model_file, PATH_LENGTH_M, "%s", "");
	snprintf(limit_files.para_file, PATH_LENGTH_M, "%s", "");
	ret = limit_fdet_init(g_detect_hdl.limit_fdet_mem, (UINT32)1, &(g_detect_hdl.limit_model_mem), (VOID *)NULL, &limit_files, pvdcnn_debug);
	if (ret != HD_OK){
		printf("[limit_fdet] Failed to init limit_fdet \r\n");
		goto stop_detext;
	}
	ret = limit_fdet_params_get(g_detect_hdl.limit_fdet_mem, &limit_fdet_params);
	limit_fdet_params.detection_threshold = 0.95f;
	limit_fdet_params.cls_threshold[0] = 0.5f; // Increase people detection rate
	ret = limit_fdet_params_set(g_detect_hdl.limit_fdet_mem, &limit_fdet_params);

#if (LOOP_AI_PROC)
	while (1) {
#endif
		ret = pvdcnn_process_inplace(g_detect_hdl.pvd_mem, &(g_detect_hdl.input_image), &(g_detect_hdl.limit_fdet_mem), &pvd_final_result, &pvd_out_num, (PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
		if(ret){
			DBG_ERR("pvdcnn_process_inplace() fail with %d\n", ret);
			goto stop_detext;
		}
#if (LOOP_AI_PROC)
	}
#endif

	g_detect_hdl.detect_num = pvd_out_num;
#else
	UINT32 null_num = 0;
#if (LOOP_AI_PROC)
	while (1) {
		DBG_DUMP("[DE] ----------- num : %ld ----------- \n", null_num);
	}
#endif
	g_detect_hdl.detect_num = null_num;
#endif

#if (AI_METHOD == 3)
stop_detext:
	vendor_ai_net_stop(p_proposal_params->run_id);
	vendor_ai_net_close(p_proposal_params->run_id);
	vendor_ai_net_stop(p_limit_fdet_params->run_id);
	vendor_ai_net_close(p_limit_fdet_params->run_id);
	vendor_ai_uninit();
#elif (AI_METHOD == 4)
stop_detext:
	pvdcnn_uninit(g_detect_hdl.pvd_mem);
	limit_fdet_uninit(g_detect_hdl.limit_fdet_mem);
	vendor_ai_uninit();
#endif

#if (DEBUG_AI_MWP)
	arb_disable_wp(DDR_ARB_1, WPSET_0);
	arb_disable_wp(DDR_ARB_1, WPSET_1);
#endif

	vos_perf_list_mark("de", __LINE__, 2);

exit_detect:
	g_detect_hdl.detect_done = 1;
	return g_detect_hdl.detect_num;
}

int operation_ai_get_result(void)
{
	fastboot_wait_done(BOOT_FLOW_AI_DET_MODEL);
	if (g_detect_hdl.ai_det_model_exist == 0) {
		return 0; //no ai
	}

	int wait = 1;
	if (wait) {
		while (g_detect_hdl.detect_done == 0) {
			vos_util_delay_ms(1);
		}
	}
	return g_detect_hdl.detect_num;
}
#endif
