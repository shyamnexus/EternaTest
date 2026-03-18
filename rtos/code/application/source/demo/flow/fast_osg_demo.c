#include <stdio.h>
#include <string.h>
#include "kwrap/debug.h"
#include "prjcfg.h"
#include "kdrv_builtin.h"
#include "kwrap/type.h"
#include "sys_fwload.h"
#include <kdrv_builtin/osg_builtin.h>
#include <kdrv_builtin/nvtmpp_init.h>
#include <grph_plat.h>
#include <kdrv_gfx2d/kdrv_grph_ctl.h>
#include <hdal.h>

#define USE_EXT_OSG DISABLE

#define __ALIGN_CEIL_128(a)       ((a+127) & ~0x07f)

#define STAMP_WIDTH   416
#define STAMP_HEIGHT  128

extern unsigned short video_frm_416_128_1_argb4444[];

typedef struct {
	uintptr_t  pa;
	UINT32     size;
} OSD_BUFFER_S;

static OSD_BUFFER_S hd_common_mem_get_block_lite(HD_COMMON_MEM_POOL_TYPE pool_type, UINT32 nvtmpp_buf_idx)
{
	NVTMPP_FASTBOOT_MEM_S *p_fastboot_mem;
	OSD_BUFFER_S          buffer = {0};

	p_fastboot_mem = nvtmpp_get_fastboot_mem();
	if (NULL == p_fastboot_mem) {
		DBG_ERR("nvtmpp_get_fastboot_mem failed\r\n");
		return buffer;
	}

	int path_id;
	for (path_id = 0; path_id < FBOOT_MISC_CPOOL_MAX; path_id++) {
		if (p_fastboot_mem->misc_cpool[path_id].pool_type == pool_type) {
			if(p_fastboot_mem->misc_cpool[path_id].blk_cnt >= nvtmpp_buf_idx){
				buffer.pa   = p_fastboot_mem->misc_cpool[path_id].blk[nvtmpp_buf_idx].pa;
				buffer.size = p_fastboot_mem->misc_cpool[path_id].blk[nvtmpp_buf_idx].size;
				return buffer;
			}else{
				DBG_ERR("buf idx(%u) > dtsi(%u)\n", nvtmpp_buf_idx, p_fastboot_mem->misc_cpool[path_id].blk_cnt);
				return buffer;
			}
		}
	}

	return buffer;
}

static int setup_osg_stamp(UINT32 path, OSD_BUFFER_S *pBuffer)
{
	OSG                   osg;
	OSG_STAMP             stamp = {0};
	int                   ret = -1;
	
	if(pBuffer == NULL){
		DBG_ERR("pBuffer is NULL\r\n");
		return -1;
	}

	stamp.buf.type        = OSG_BUF_TYPE_PING_PONG;
	stamp.buf.size        = pBuffer->size;
	stamp.buf.ddr_id      = 0;
	stamp.buf.p_addr      = pBuffer->pa; //should be 128 aligned
	stamp.img.fmt         = OSG_PXLFMT_ARGB4444;
	stamp.img.dim.w       = STAMP_WIDTH;
	stamp.img.dim.h       = STAMP_HEIGHT;
	stamp.attr.alpha      = 255;
	stamp.attr.position.x = 0;
	stamp.attr.position.y = 240;
	stamp.attr.layer      = 0;
	stamp.attr.region     = 0;

	osg.num = 1;
	osg.stamp = &stamp;

#if USE_EXT_OSG
	if(vds_set_early_ext_osg(path, &osg)){
		DBG_ERR("vds_set_early_ext_osg() fail\n");
		goto out;
	}

	if(vds_update_early_ext_osg(path, 0, (uintptr_t)video_frm_416_128_1_argb4444)){
		DBG_ERR("vds_update_early_ext_osg(0, 0) fail\n");
		goto out;
	}
#else
	if(vds_set_early_osg(path, &osg)){
		DBG_ERR("vds_set_early_osg() fail\n");
		goto out;
	}

	if(vds_update_early_osg(path, 0, (uintptr_t)video_frm_416_128_1_argb4444)){
		DBG_ERR("vds_update_early_osg(0, 0) fail\n");
		goto out;
	}
#endif

	ret = 0;

out:

	return ret;
}

int venc_osg_demo_init(void)
{
	OSD_BUFFER_S buffer1 = {0}, buffer2 = {0};

	buffer1 = hd_common_mem_get_block_lite(HD_COMMON_MEM_OSG_POOL, 0);
	if (buffer1.pa == 0) {
		return 0;
	}

	fwload_wait_done(CODE_SECTION_04);

#if USE_EXT_OSG
	grph_platform_init();
	kdrv_graph_rtos_init();
	if(fastboot_osg_register_grph_cb(kdrv_graph_open, kdrv_graph_set, kdrv_graph_trigger, kdrv_graph_close)){
		DBG_ERR("fail to register graphic callback\n");
		return -1;
	}
#endif

	if(setup_osg_stamp(0, &buffer1)){
		DBG_ERR("fail to setup h26x stamp\n");
		return -1;
	}

	buffer2 = hd_common_mem_get_block_lite(HD_COMMON_MEM_OSG_POOL, 1);
	if (buffer2.pa == 0) {
		return 0;
	}

	if(setup_osg_stamp(1, &buffer2)){
		DBG_ERR("fail to setup jpeg stamp\n");
		return -1;
	}

	return 0;
}
