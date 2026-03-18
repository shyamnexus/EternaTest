#include <string.h>
#include <plat/sdio.h>
#include <plat/strg_def.h>
#include <hdal.h>
#include <FileSysTsk.h>
#include <kwrap/task.h>
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include "kwrap/cpu.h"
#include "sys_fastboot.h"
#include "sys_mempool.h"
#include "sys_fwload.h"
#include <FreeRTOS.h>
#include <task.h>
#include "kdrv_builtin/nvtmpp_init.h"
#include "kdrv_builtin/kdrv_ipp_builtin.h"
#include "kdrv_builtin/vdoenc_builtin.h"
#include "kdrv_builtin/sie_init.h"
#include <kdrv_videoprocess/kdrv_ime.h>
#include <kdrv_videoenc/kdrv_videoenc_lmt.h>
#include "fast_venc.h"
#include "bridge_fourcc.h"
#include "bridge_mem.h"

void get_enc_obj(UINT32 *enc_obj)
{
	vdoenc_builtin_get_enc_obj_addr(enc_obj);
	return;
}

void get_enc_var(UINT32 *enc_var)
{
	vdoenc_builtin_get_enc_var_addr(enc_var);
	return;
}

int venc_fast_init(void)
{
	VDOENC_BUILTIN_INIT_INFO venc_info[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
	int i = 0, ret = 0;
//	int venc_en[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
//	int codec_type[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
	NVTMPP_FASTBOOT_MEM_S *p_fastboot_mem;

	THREAD_ENTRY();

	fwload_wait_done(CODE_SECTION_05);

	// init nvtmpp memory
	if (nvtmpp_init_mmz() < 0) {
		DBG_ERR("init_mmz fail\r\n");
		return -1;
	}
	if (nvtmpp_parse_fastboot_mem_dt() < 0) {
		DBG_ERR("parse fastboot_mem fail\r\n");
		return -1;
	}

	p_fastboot_mem = nvtmpp_get_fastboot_mem();
	vdoenc_builtin_get_of_node_venc();
	for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
		ret = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_ENC_EN);
		if (ret == 1) {
			venc_info[i].width = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_WIDTH);
			venc_info[i].height = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_HEIGHT);
		}
	}
	venc_info[BUILTIN_VDOENC_PATH_ID_0].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_0_MAX].addr;
	venc_info[BUILTIN_VDOENC_PATH_ID_0].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_0_MAX].size;
	venc_info[BUILTIN_VDOENC_PATH_ID_1].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_1_MAX].addr;
	venc_info[BUILTIN_VDOENC_PATH_ID_1].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_1_MAX].size;
	venc_info[BUILTIN_VDOENC_PATH_ID_2].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_2_MAX].addr;
	venc_info[BUILTIN_VDOENC_PATH_ID_2].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_2_MAX].size;
	venc_info[BUILTIN_VDOENC_PATH_ID_3].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_3_MAX].addr;
	venc_info[BUILTIN_VDOENC_PATH_ID_3].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_3_MAX].size;
	venc_info[BUILTIN_VDOENC_PATH_ID_4].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_4_MAX].addr;
	venc_info[BUILTIN_VDOENC_PATH_ID_4].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_4_MAX].size;
	venc_info[BUILTIN_VDOENC_PATH_ID_5].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_5_MAX].addr;
	venc_info[BUILTIN_VDOENC_PATH_ID_5].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_5_MAX].size;

	{
		// sw workaround for h26x
		VDOENC_BUILTIN_INIT_INFO venc_info_lite = {0};
		venc_info_lite.width = H265E_WIDTH_MIN_538;
		venc_info_lite.height = H265E_HEIGHT_MIN_538;
		venc_info_lite.max_blk_addr = venc_info[BUILTIN_VDOENC_PATH_ID_0].max_blk_addr;
		venc_info_lite.max_blk_size = venc_info[BUILTIN_VDOENC_PATH_ID_0].max_blk_size;
		VdoEnc_builtin_init_lite(&venc_info_lite);
	}

	VdoEnc_builtin_init(venc_info);

	return 0;
}

int bridge_mem_plan_venc(void)
{
	UINT32 i = 0;
	UINT32 enc_var = 0;
	UINT32 enc_obj = 0;
	UINT32 enable = 0;
	UINT32 pre_num = 0;

	bridge_mem_add_tag(VENC_PRE_INIT, 1);

	get_enc_obj(&enc_obj);
	if (enc_obj != 0) {
		bridge_mem_add_tag(VENC_PRE_OBJ, enc_obj);
	}

	get_enc_var(&enc_var);
	if (enc_var != 0) {
		bridge_mem_add_tag(VENC_PRE_VAR, enc_var);
	}

	vdoenc_builtin_get_en_lnx_h26x_int_first(&enable);
	bridge_mem_add_tag(VENC_EN_INT_FIRST, enable);

	for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
		vdoenc_builtin_get_pre_num(i, &pre_num);
		bridge_mem_add_tag(VENC_PRE_NUM_0 + i, pre_num);
	}

	return 0;
}