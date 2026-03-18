/**
    @brief Source file of kflow_ai_net.

    @file kflow_ai_isp.c

    @ingroup kflow_ai_net

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/

#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kwrap/file.h"
#include "kwrap/platform.h"
#include "kwrap/semaphore.h"
#include "kwrap/cpu.h"

#include "kdrv_ai.h"
#include "kflow_ai_net/kflow_ai_net_platform.h"
#include "kflow_isp/nn_isp.h"
//=============================================================
#define __CLASS__ 				"[ai][kflow_isp][isp]"
#include "kflow_ai_isp_debug.h"
//=============================================================
#include "kflow_isp/kflow_isp_platform.h"
#include "kflow_isp/kflow_isp_change_res.h"
#include "kflow_ai_isp_build_instrs_sync_sdktool.h"


#define AIISP_DYNAMIC_CHANGE_RESOLUTION_EN      1
#define AIISP_DYNAMIC_RESOLUTION_DEBUG          0
#define AIISP_FRAME_MODE_DEBUG                  0


//local api
ER convert_regiobuf2drvframe(JMISP_REG_IOBUF* p_regiobuf, KDRV_AI_JMISP_FRAME_INFO* p_o_frame);
ER convert_regiobuf2drvpingpong(JMISP_REG_IOBUF* p_regiobuf, KDRV_AI_JMISP_PINGPONG_INFO* p_o_pingpong);
uint64_t ispframemode_cal_blk_rel_addr(JMISP_REG_IOBUF* p_iobuf_reg, uint64_t blk_x, uint64_t blk_y, uint64_t stripe_x, uint64_t slice_y);
ER ispframemode_cal_substripe_frame(JMISP_REG_IOBUF* p_iobuf_reg, KDRV_AI_JMISP_FRAME_INFO* p_out_frame_info, uint32_t* p_substripe_blk_x_cnt_array
									, uint64_t start_addr, uint64_t substripe_x, uint64_t slice_y);
ER ispframemode_cal_substripe_pingpong(JMISP_REG_IOBUF* p_iobuf_reg, KDRV_AI_JMISP_PINGPONG_INFO* p_out_pingpong_info, uint32_t* p_substripe_blk_x_cnt_array
									, uint64_t start_addr, uint64_t substripe_x, uint64_t slice_y);
ER ispframemode_upd_pingpong_addr(VOID *p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
									, const uint64_t pp_addr0, const uint64_t pp_addr1, const bool sub_stripe_en, const uint32_t core_num);
ER ispframemode_upd_ringbuf_addr(VOID *p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
									, bool ring_en, const uint64_t ring_saddr, const uint64_t ring_eaddr, const bool sub_stripe_en, const uint32_t core_num);


e_AI_JMISP_DRV_IO_CONST convert_regbuf2drvio_type(int32_t type) {
    e_AI_JMISP_REG_IO_CONST reg_type = (e_AI_JMISP_REG_IO_CONST)type;
    switch (reg_type) {
    case e_AI_JMISP_REG_IO_RING_BUF0:
        return e_AI_JMISP_DRV_IO_RING_BUF0;
    case e_AI_JMISP_REG_IO_RING_BUF1:
        return e_AI_JMISP_DRV_IO_RING_BUF1;
    case e_AI_JMISP_REG_IO_FRAME_BUF0:
        return e_AI_JMISP_DRV_IO_FRAME_BUF0;
    case e_AI_JMISP_REG_IO_FRAME_BUF1:
        return e_AI_JMISP_DRV_IO_FRAME_BUF1;
    case e_AI_JMISP_REG_IO_FRAME_BUF2:
        return e_AI_JMISP_DRV_IO_FRAME_BUF2;
    case e_AI_JMISP_REG_IO_FRAME_BUF3:
        return e_AI_JMISP_DRV_IO_FRAME_BUF3;
    case e_AI_JMISP_REG_IO_PINGPONG_BUF0:
        return e_AI_JMISP_DRV_IO_PINGPONG_BUF0;
    case e_AI_JMISP_REG_IO_PINGPONG_BUF1:
        return e_AI_JMISP_DRV_IO_PINGPONG_BUF1;
    case e_AI_JMISP_REG_IO_PINGPONG_BUF2:
        return e_AI_JMISP_DRV_IO_PINGPONG_BUF2;
    case e_AI_JMISP_REG_IO_CUSTPIPE_BUF0:
        return e_AI_JMISP_DRV_IO_PIPE_BUF0;
    case e_AI_JMISP_REG_IO_CUSTPIPE_BUF1:
        return e_AI_JMISP_DRV_IO_PIPE_BUF1;
    default:
        return e_AI_JMISP_DRV_IO_UNKNOWN;
    }
}

e_AI_JMISP_DRV_IO_CONST convert_iobuf2drvio_type(AI_JMISP_IOBUF_INFO* p_iobuf_info, e_AI_JMISP_IOBUF_CONST iobuf_type) {
    return convert_regbuf2drvio_type(convert_iobuf2regbuf_type(p_iobuf_info, iobuf_type));
}

uint32_t get_ppu_num(AI_JMISP_INFO* p_info) {
	uint32_t ppu_id_array[e_AI_IOBUF_NUM];
	uint32_t ppu_num=0;
	uint32_t i=0, j=0;
	
	if (p_info == NULL) {
		return ppu_num;
	}

	//init
	for (i=0; i<e_AI_IOBUF_NUM; ++i) {
		ppu_id_array[i] = -1;
	}

	//find number of PPU
	for (i=0; i<e_AI_IOBUF_NUM; ++i) {
		BOOL repeat_ppu = FALSE;
		int32_t match_ppu_id = p_info->iobuf[i].match_ppu_id;
		if ((match_ppu_id < 0)) {
			continue;
		}
		for (j=0; j<ppu_num; ++j) {
			if (match_ppu_id == ppu_id_array[j]) {
				repeat_ppu = TRUE;
				break;
			}
		}
		if (!repeat_ppu) {
			ppu_id_array[ppu_num++] = match_ppu_id;
		}
	}

	return ppu_num;
}

ER convert_dyframe2jmisp_info(const IPP_INFO* p_ipp_info, AI_JMISP_INFO* p_ori_info, AI_JMISP_INFO* p_out_info) {

    const e_AI_JMISP_IOBUF_CONST bm_iobuf_type = get_benchmark_iobuf_type();
    AI_JMISP_IOBUF_INFO* p_bm_iobuf = get_benchmark_iobuf(p_ori_info);
    int32_t resol_w_lower_bound, resol_h_lower_bound;

    // for resolution changed
    if ((p_ipp_info == NULL) || (p_ori_info == NULL) || (p_out_info == NULL) || (p_bm_iobuf == NULL)) {
        return E_NOEXS;
    }

    resol_w_lower_bound = (p_bm_iobuf->blockInfo.block_width - p_bm_iobuf->blockInfo.block_overlap_width) * 3;
    resol_h_lower_bound = (p_bm_iobuf->blockInfo.block_height - p_bm_iobuf->blockInfo.block_overlap_height) * 3;
    if ((p_bm_iobuf->frameInfo.frame_width < p_ipp_info->frame_width)
            || (p_bm_iobuf->frameInfo.frame_height < p_ipp_info->frame_height)
            || (resol_w_lower_bound > p_ipp_info->frame_width)
            || (resol_h_lower_bound > p_ipp_info->frame_height)
    ) {
        DBG_ERR("Resolution(%d, %d) exceeds model limits(%d~%d, %d~%d).\r\n"
                , p_ipp_info->frame_width, p_ipp_info->frame_height
                , resol_w_lower_bound, p_bm_iobuf->frameInfo.frame_width
                , resol_h_lower_bound, p_bm_iobuf->frameInfo.frame_height);
        return E_RES;
    }

    // copy original block info
    *p_out_info = *p_ori_info;


#if AIISP_DYNAMIC_CHANGE_RESOLUTION_EN

	// update frame info
	{
		uint32_t idx;
		if ((p_bm_iobuf->frameInfo.frame_width <= 0) || (p_bm_iobuf->frameInfo.frame_height <= 0)) {
			return E_CTX;
		}

		p_out_info->iobuf[bm_iobuf_type].frameInfo.frame_width = p_ipp_info->frame_width;
		p_out_info->iobuf[bm_iobuf_type].frameInfo.frame_height = p_ipp_info->frame_height;
		for (idx=1; idx<e_AI_IOBUF_NUM; ++idx) {
            int32_t ratio_scale_x, ratio_shift_x, ratio_scale_y, ratio_shift_y;
			if ((!is_jmisp_sup_buf(&p_out_info->iobuf[idx], idx)) || (p_out_info->iobuf[idx].buffer_sz <= 0)) {
                continue;
            }
            ratio_scale_x = p_out_info->iobuf[idx].ratioInfo.scale_x;
			ratio_shift_x = p_out_info->iobuf[idx].ratioInfo.shift_x;
			ratio_scale_y = p_out_info->iobuf[idx].ratioInfo.scale_y;
			ratio_shift_y = p_out_info->iobuf[idx].ratioInfo.shift_y;
			p_out_info->iobuf[idx].frameInfo.frame_width = p_out_info->iobuf[bm_iobuf_type].frameInfo.frame_width * ratio_scale_x >> ratio_shift_x;
			p_out_info->iobuf[idx].frameInfo.frame_height = p_out_info->iobuf[bm_iobuf_type].frameInfo.frame_height * ratio_scale_y >> ratio_shift_y;
		}
	}
	
	// update stripe info
	{
		int32_t check_stripe_num=0;
		uint32_t idx;
		
		// check stripe num
		if  (p_ipp_info->first_stripe_width == 0) {
			DBG_ERR("AI first stripe width can't be 0\r\n");
			return E_PAR;
		} else if (p_ipp_info->last_stripe_width == 0) {
			check_stripe_num=1;
		} else if (p_ipp_info->mid_stripe_width == 0) {
			check_stripe_num=2;
		} else {
			int32_t mid_frame_width = p_ipp_info->frame_width - p_ipp_info->first_stripe_width - p_ipp_info->last_stripe_width + p_ipp_info->stripe_overlap_width;
			int32_t stripe_width_without_overlap = p_ipp_info->mid_stripe_width - p_ipp_info->stripe_overlap_width;
			check_stripe_num = mid_frame_width / stripe_width_without_overlap + 2;
		}
		if (p_ipp_info->stripe_num != check_stripe_num) {
			DBG_ERR("AI stripe num (%d) not same with ISP (%d)...\r\n", check_stripe_num, p_ipp_info->stripe_num);
			return E_PAR;
		}

		p_out_info->iobuf[bm_iobuf_type].stripeInfo.first_stripe_width = p_ipp_info->first_stripe_width;
		p_out_info->iobuf[bm_iobuf_type].stripeInfo.mid_stripe_width = p_ipp_info->mid_stripe_width;
		p_out_info->iobuf[bm_iobuf_type].stripeInfo.last_stripe_width = p_ipp_info->last_stripe_width;
		p_out_info->iobuf[bm_iobuf_type].stripeInfo.stripe_overlap_width = p_ipp_info->stripe_overlap_width;
		for (idx=1; idx<e_AI_IOBUF_NUM; ++idx) {
			int32_t ratio_scale, ratio_shift;
            if ((!is_jmisp_sup_buf(&p_out_info->iobuf[idx], idx)) || (p_out_info->iobuf[idx].buffer_sz <= 0)) {
                continue;
            }
            ratio_scale = p_out_info->iobuf[idx].ratioInfo.scale_x;
            ratio_shift = p_out_info->iobuf[idx].ratioInfo.shift_x;
			p_out_info->iobuf[idx].stripeInfo.first_stripe_width = p_out_info->iobuf[bm_iobuf_type].stripeInfo.first_stripe_width * ratio_scale >> ratio_shift;
			p_out_info->iobuf[idx].stripeInfo.mid_stripe_width = p_out_info->iobuf[bm_iobuf_type].stripeInfo.mid_stripe_width * ratio_scale >> ratio_shift;
			p_out_info->iobuf[idx].stripeInfo.last_stripe_width = p_out_info->iobuf[bm_iobuf_type].stripeInfo.last_stripe_width * ratio_scale >> ratio_shift;
			p_out_info->iobuf[idx].stripeInfo.stripe_overlap_width = p_out_info->iobuf[bm_iobuf_type].stripeInfo.stripe_overlap_width * ratio_scale >> ratio_shift;
		}
	}

	// update slice info
	{ 
		bool is_one_slice;
		uint32_t idx;
		
		if (p_bm_iobuf->stripeInfo.output_slice_height == p_bm_iobuf->frameInfo.frame_height) {
			if (p_bm_iobuf->stripeInfo.slice_overlap_height != 0) {
				DBG_ERR("slice overlap should be 0 if only one slice.\r\n");
			}
			is_one_slice = true;
		} else {
			is_one_slice = false;
		}


#if 0	// change slice height to fill slice buffer
        int32_t max_new_stripe_w, new_slice_h, ori_slice_buf_sz;

        ori_slice_buf_sz = max(p_bm_iobuf->stripeInfo.first_stripe_width, p_bm_iobuf->stripeInfo.mid_stripe_width);
        ori_slice_buf_sz = max(ori_slice_buf_sz, p_bm_iobuf->stripeInfo.last_stripe_width);
        ori_slice_buf_sz *= p_bm_iobuf->stripeInfo.output_slice_height;

        max_new_stripe_w = max(p_out_info->iobuf[bm_iobuf_type].stripeInfo.first_stripe_width, p_out_info->iobuf[bm_iobuf_type].stripeInfo.mid_stripe_width);
        max_new_stripe_w = max(max_new_stripe_w, p_out_info->iobuf[bm_iobuf_type].stripeInfo.last_stripe_width);

        new_slice_h = (ori_slice_buf_sz / max_new_stripe_w) >> 1 << 1;
        p_out_info->iobuf[bm_iobuf_type].stripeInfo.slice_overlap_height = p_bm_iobuf->stripeInfo.slice_overlap_height;
        p_out_info->iobuf[bm_iobuf_type].stripeInfo.output_slice_height = new_slice_h;
        for (idx=1; idx<e_AI_IOBUF_NUM; ++idx) {
            if ((!is_jmisp_sup_buf(&p_out_info->iobuf[idx], idx)) || (p_out_info->iobuf[idx].buffer_sz <= 0)) {
                continue;
            }
            int32_t ratio_scale = p_out_info->iobuf[idx].ratioInfo.scale_y;
            int32_t ratio_shift = p_out_info->iobuf[idx].ratioInfo.shift_y;
            p_out_info->iobuf[idx].stripeInfo.slice_overlap_height = p_ori_info->iobuf[idx].stripeInfo.slice_overlap_height;
            p_out_info->iobuf[idx].stripeInfo.output_slice_height = p_out_info->iobuf[bm_iobuf_type].stripeInfo.output_slice_height * ratio_scale >> ratio_shift;
        }

#else	// for debug
        for (idx=0; idx<e_AI_IOBUF_NUM; ++idx) {
            if ((!is_jmisp_sup_buf(&p_out_info->iobuf[idx], idx)) || (p_out_info->iobuf[idx].buffer_sz <= 0)) {
                continue;
            }
            p_out_info->iobuf[idx].stripeInfo.slice_overlap_height = p_ori_info->iobuf[idx].stripeInfo.slice_overlap_height;
            if (is_one_slice) {
                p_out_info->iobuf[idx].stripeInfo.output_slice_height = p_out_info->iobuf[idx].frameInfo.frame_height;
            } else {
                p_out_info->iobuf[idx].stripeInfo.output_slice_height = p_ori_info->iobuf[idx].stripeInfo.output_slice_height;
            }
        }
#endif
	}

#endif


#if AIISP_DYNAMIC_RESOLUTION_DEBUG
    {
        uint32_t idx=0;
        DBG_DUMP("-------- Change resolution AI_JMISP_INFO debug start --------\n");
        DBG_DUMP("work buffer size: (%llu, %llu)\n", p_ori_info->working_buf_size, p_out_info->working_buf_size);
        for (idx=0; idx<e_AI_IOBUF_NUM; ++idx) {
            const e_AI_JMISP_IOBUF_CONST type = (e_AI_JMISP_IOBUF_CONST)idx;
            const AI_JMISP_IOBUF_INFO* p_ori_iobuf = &p_ori_info->iobuf[type];
            const AI_JMISP_IOBUF_INFO* p_out_iobuf = &p_out_info->iobuf[type];
            if ((!is_jmisp_sup_buf(&p_out_info->iobuf[idx], idx)) || (p_out_info->iobuf[idx].buffer_sz <= 0)) {
                continue;
            }
            DBG_DUMP("======= io buf[%d] =======\n", idx);
            DBG_DUMP("frame w : (%d, %d)\n", p_ori_iobuf->frameInfo.frame_width, p_out_iobuf->frameInfo.frame_width);
            DBG_DUMP("frame h : (%d, %d)\n", p_ori_iobuf->frameInfo.frame_height, p_out_iobuf->frameInfo.frame_height);
            DBG_DUMP("first stripe w : (%d, %d)\n", p_ori_iobuf->stripeInfo.first_stripe_width, p_out_iobuf->stripeInfo.first_stripe_width);
            DBG_DUMP("mid stripe w : (%d, %d)\n", p_ori_iobuf->stripeInfo.mid_stripe_width, p_out_iobuf->stripeInfo.mid_stripe_width);
            DBG_DUMP("last stripe w : (%d, %d)\n", p_ori_iobuf->stripeInfo.last_stripe_width, p_out_iobuf->stripeInfo.last_stripe_width);
            DBG_DUMP("overlap stripe w : (%d, %d)\n", p_ori_iobuf->stripeInfo.stripe_overlap_width, p_out_iobuf->stripeInfo.stripe_overlap_width);
            DBG_DUMP("out slice h : (%d, %d)\n", p_ori_iobuf->stripeInfo.output_slice_height, p_out_iobuf->stripeInfo.output_slice_height);
            DBG_DUMP("overlap slice h : (%d, %d)\n", p_ori_iobuf->stripeInfo.slice_overlap_height, p_out_iobuf->stripeInfo.slice_overlap_height);
            DBG_DUMP("block w : (%d, %d)\n", p_ori_iobuf->blockInfo.block_width, p_out_iobuf->blockInfo.block_width);
            DBG_DUMP("overlap block w : (%d, %d)\n", p_ori_iobuf->blockInfo.block_overlap_width, p_out_iobuf->blockInfo.block_overlap_width);
            DBG_DUMP("block h : (%d, %d)\n", p_ori_iobuf->blockInfo.block_height, p_out_iobuf->blockInfo.block_height);
            DBG_DUMP("overlap block h : (%d, %d)\n", p_ori_iobuf->blockInfo.block_overlap_height, p_out_iobuf->blockInfo.block_overlap_height);
            DBG_DUMP("io base addr type : (%d, %d)\n", p_ori_iobuf->addrType, p_out_iobuf->addrType);
            DBG_DUMP("io base addr ofs : (%llx, %llx)\n", p_ori_iobuf->pa, p_out_iobuf->pa);
            DBG_DUMP("match PPU ID : (%d, %d)\n", p_ori_iobuf->match_ppu_id, p_out_iobuf->match_ppu_id);
            DBG_DUMP("bitdepth : (%u, %u)\n", p_ori_iobuf->bitdepth, p_out_iobuf->bitdepth);
            DBG_DUMP("buffer size : (%u, %u)\n", p_ori_iobuf->buffer_sz, p_out_iobuf->buffer_sz);
            DBG_DUMP("ratio scale x : (%d, %d)\n", p_ori_iobuf->ratioInfo.scale_x, p_out_iobuf->ratioInfo.scale_x);
            DBG_DUMP("ratio shift x : (%d, %d)\n", p_ori_iobuf->ratioInfo.shift_x, p_out_iobuf->ratioInfo.shift_x);
            DBG_DUMP("ratio scale y : (%d, %d)\n", p_ori_iobuf->ratioInfo.scale_y, p_out_iobuf->ratioInfo.scale_y);
            DBG_DUMP("ratio shift y : (%d, %d)\n", p_ori_iobuf->ratioInfo.shift_y, p_out_iobuf->ratioInfo.shift_y);
        }
        DBG_DUMP("-------- Change resolution AI_JMISP_INFO debug end --------\n");
    }
#endif

	return E_OK;
}

ER parsing_ppu_instrs(AI_JMISP_INFO* p_info, uintptr_t ppu_addr) {
	
#if AIISP_DYNAMIC_CHANGE_RESOLUTION_EN
	{
		// for resolution changed
		JMISP_REG_INFO* p_reg_info;
		uint32_t ppu_num, idx;
		uintptr_t ppu_head_addr;

		p_reg_info = nvt_ai_mem_alloc(sizeof(JMISP_REG_INFO));
		if (p_reg_info == NULL) {
			DBG_ERR("p_reg_info alloc fail\r\n");
			return E_NOMEM;
		} else if ((p_info == NULL) || (ppu_addr == 0)) {
			nvt_ai_mem_free(p_reg_info);
			//p_reg_info = 0;
			return E_NOEXS;
		}

		gen_jmisp_reg_para(p_info, p_reg_info);
		
		// get # of PPU with check match_ppu_id in AI_JMISP_INFO
		ppu_num = get_ppu_num(p_info);
		
		// ppu_addr : PPU_HEAD, PPU_INSTRS, PPU_HEAD, PPU_INSTRS, ...
		ppu_head_addr = ppu_addr;
		for (idx=0; idx<ppu_num; ++idx) {
			JMISP_INSTR_BLOCK_CASE_INFO* p_blk_case_info;
			JMISPPPU_HEAD* p_ppu_head = (JMISPPPU_HEAD*)ppu_head_addr;
			p_blk_case_info = nvt_ai_mem_alloc(sizeof(JMISP_INSTR_BLOCK_CASE_INFO));
			if (p_blk_case_info == NULL) {
				DBG_ERR("p_blk_case_info alloc fail\r\n");
				continue;
			}
			
			gen_ppu_blockcase_info(p_info, p_reg_info, p_ppu_head, p_blk_case_info);
			upd_ppu_all_blockcase_instr(p_blk_case_info, p_reg_info, p_ppu_head, p_info);
        #if 1
            { // fix broken image
                const uintptr_t aligned_addr = ALIGN_FLOOR_64(ppu_head_addr);
                const uint32_t fill_size = ppu_head_addr - aligned_addr;
                vos_cpu_dcache_sync(aligned_addr, p_ppu_head->parm_size + fill_size, VOS_DMA_TO_DEVICE);
            }
        #endif
			ppu_head_addr += (sizeof(JMISPPPU_HEAD) + p_ppu_head->parm_size);
			nvt_ai_mem_free(p_blk_case_info);
			p_blk_case_info = 0;
		}

		nvt_ai_mem_free(p_reg_info);
		//p_reg_info = 0;
	}
#endif

#if AIISP_DYNAMIC_RESOLUTION_DEBUG
    {
        uint32_t ppu_num = get_ppu_num(p_info);
        uintptr_t ppu_head_addr = ppu_addr;
        uint32_t idx;

        for (idx=0; idx<ppu_num; ++idx) {
            JMISPPPU_HEAD* p_ppu_head = (JMISPPPU_HEAD*)ppu_head_addr;
            const uint32_t case_num = gen_block_case_num(p_ppu_head, p_info);
            uintptr_t ppu_parm_addr = p_ppu_head->parm_addr;
            uint32_t per_llcmd_sz = p_ppu_head->parm_size / case_num;
            uint32_t case_idx, ll_idx;
                    
            DBG_DUMP("ppu#[%u]\n", idx);
            for (case_idx=0; case_idx<case_num; ++case_idx) {
                uintptr_t per_ppu_ll_addr = ppu_parm_addr + case_idx * per_llcmd_sz;
                DBG_DUMP("Conditional: %u\n", case_idx);
                
                for (ll_idx=0 ; ll_idx < per_llcmd_sz ; ll_idx += sizeof(UINT64)) {
                    UINT64* p_ll_cmd_ptr = (UINT64*)(per_ppu_ll_addr + ll_idx);
                    DBG_DUMP("    (%4d) 0x%016llx", (int)ll_idx, *p_ll_cmd_ptr);
                    if (NN_LL_CMD_GET_MODE(*p_ll_cmd_ptr) == 1) {
                        UINT reg = (*p_ll_cmd_ptr << 44) >> 52;
                        UINT value = (*p_ll_cmd_ptr) >> 32;
                        DBG_DUMP(" => cmd(UPD     ) ,Reg(0x%03x), Value(0x%08x)\n", reg, value);
                    } else if (NN_LL_CMD_GET_MODE(*p_ll_cmd_ptr) == 3) {
                        DBG_DUMP("   => cmd(NEXT_LL ) , TableIndex(%d)\n", (int)((*p_ll_cmd_ptr << 48) >> 56));
                    } else if (NN_LL_CMD_GET_MODE(*p_ll_cmd_ptr) == 2) {
                        DBG_DUMP("   => cmd(JUMP    ) , addr_type(%u), msb_addr(0x%04x), Addr(0x%08x)\n", (UINT)((*p_ll_cmd_ptr) >> 60), (UINT)((*p_ll_cmd_ptr << 12) >> 60), (UINT)((*p_ll_cmd_ptr << 16) >> 32));
                    } else if (NN_LL_CMD_GET_MODE(*p_ll_cmd_ptr) == 0) {
                        DBG_DUMP(" => cmd(NULL    ) ,TableIndex(%d)\n", (int)((*p_ll_cmd_ptr << 48) >> 56));
                    } else {
                        DBG_DUMP(" => cmd(Unknwon )\n");
                    }
                }
                DBG_DUMP("\n");
            }
            DBG_DUMP("----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

            ppu_head_addr += (sizeof(JMISPPPU_HEAD) + p_ppu_head->parm_size);
        }
    }
#endif

	return E_OK;
}

ER change_pou_inputfeat_seg(AI_JMISP_INFO* p_info, uintptr_t ppu_addr, AI_TUNING_PARA* p_tuning_para) {
    JMISP_REG_INFO* p_reg_info;
    uint32_t ppu_num, idx;
    uintptr_t ppu_head_addr;
    JMISP_INSTR_TUNING_INFO tuning_info;
    AI_TUNING_PARA *p_input_para = (AI_TUNING_PARA *)p_tuning_para;
    ER er_code = E_OK;

    p_reg_info = nvt_ai_mem_alloc(sizeof(JMISP_REG_INFO));
    if (p_reg_info == NULL) {
        DBG_ERR("p_reg_info alloc fail\r\n");
        return E_NOMEM;
    } else if ((p_info == NULL) || (ppu_addr == 0) || (p_tuning_para == NULL)) {
        er_code = E_NOEXS;
        goto exit;
    }
    {
        tuning_info.str = p_input_para->tuning_para;
    }

    gen_jmisp_reg_para(p_info, p_reg_info);

    // get # of PPU with check match_ppu_id in AI_JMISP_INFO
    ppu_num = get_ppu_num(p_info);

    // ppu_addr : PPU_HEAD, PPU_INSTRS, PPU_HEAD, PPU_INSTRS, ...
    ppu_head_addr = ppu_addr;
    for (idx=0; idx<ppu_num; ++idx) {
        JMISPPPU_HEAD* p_ppu_head = (JMISPPPU_HEAD*)ppu_head_addr;
        if ((p_ppu_head->config.bit.unit_mode != e_UNIT_SEL_POU)
            || (p_ppu_head->match_mctrl_idx != p_input_para->mctrl_id)) {
            ppu_head_addr += (sizeof(JMISPPPU_HEAD) + p_ppu_head->parm_size);
            continue;
        }

        upd_tuning_in_all_instr(&tuning_info, p_reg_info, p_ppu_head, p_info);

    #if 1
        { // fix broken image
            const uintptr_t aligned_addr = ALIGN_FLOOR_64(ppu_head_addr);
            const uint32_t fill_size = ppu_head_addr - aligned_addr;
            vos_cpu_dcache_sync(aligned_addr, p_ppu_head->parm_size + fill_size, VOS_DMA_TO_DEVICE);
        }
    #endif
        ppu_head_addr += (sizeof(JMISPPPU_HEAD) + p_ppu_head->parm_size);
    }

exit:
    nvt_ai_mem_free(p_reg_info);
    //p_reg_info = 0;
    return E_OK;
}

ER get_ppu_quan_param(AI_JMISP_INFO* p_info, uintptr_t ppu_addr, AI_TUNING_QUAN_PARAM* o_para) {
    JMISP_REG_INFO* p_reg_info;
    uint32_t ppu_num, idx;
    uintptr_t ppu_head_addr;
    JMISP_INSTR_GAIN_INFO gain_info;
    bool is_found = false;
    AI_TUNING_QUAN_PARAM *p_output_para = o_para;
    ER er_code = E_OK;

    memset(&gain_info, 0, sizeof(gain_info));
    p_reg_info = nvt_ai_mem_alloc(sizeof(JMISP_REG_INFO));
    if (p_reg_info == NULL) {
        DBG_ERR("p_reg_info alloc fail\r\n");
        return E_NOMEM;
    } else if ((p_info == NULL) || (ppu_addr == 0) || (p_output_para == NULL)) {
        er_code = E_NOEXS;
        goto exit;
    }

    gen_jmisp_reg_para(p_info, p_reg_info);

    // get # of PPU with check match_ppu_id in AI_JMISP_INFO
    ppu_num = get_ppu_num(p_info);

    // ppu_addr : PPU_HEAD, PPU_INSTRS, PPU_HEAD, PPU_INSTRS, ...
    ppu_head_addr = ppu_addr;
    for (idx=0; idx<ppu_num; ++idx) {
        JMISPPPU_HEAD* p_ppu_head = (JMISPPPU_HEAD*)ppu_head_addr;
        if ((p_ppu_head->config.word != 0 && p_ppu_head->config.bit.unit_mode != e_UNIT_SEL_PPU) // note: 690/538 old model doesn't have unit_mode, and therefore will be 0 ... skip check for 690/538
            || (p_ppu_head->match_mctrl_idx != p_output_para->mctrl_id)) {
            ppu_head_addr += (sizeof(JMISPPPU_HEAD) + p_ppu_head->parm_size);
            continue;
        }

        get_gain_in_all_instr(&gain_info, p_reg_info, p_ppu_head, p_info);
        ppu_head_addr += (sizeof(JMISPPPU_HEAD) + p_ppu_head->parm_size);
        is_found = true;
        break;
    }
    if (!is_found) {
        DBG_ERR("not get the matching mctrl\r\n");
        er_code = E_NOEXS;
        goto exit;
    }
    for (idx=0; idx<3; ++idx) {
        p_output_para->quan_out[idx].dir = gain_info.quan_out[idx].dir;
        p_output_para->quan_out[idx].scale = gain_info.quan_out[idx].scale;
        p_output_para->quan_out[idx].shift = gain_info.quan_out[idx].shift;
    }

exit:
    nvt_ai_mem_free(p_reg_info);
    //p_reg_info = 0;
    return er_code;
}

ER set_ppu_quan_param(AI_JMISP_INFO* p_info, uintptr_t ppu_addr, AI_TUNING_QUAN_PARAM* p_para) {
    JMISP_REG_INFO* p_reg_info;
    uint32_t ppu_num, idx;
    uintptr_t ppu_head_addr;
    JMISP_INSTR_GAIN_INFO gain_info;
    AI_TUNING_QUAN_PARAM *p_input_para = p_para;
    ER er_code = E_OK;

    p_reg_info = nvt_ai_mem_alloc(sizeof(JMISP_REG_INFO));
    if (p_reg_info == NULL) {
        DBG_ERR("p_reg_info alloc fail\r\n");
        return E_NOMEM;
    } else if ((p_info == NULL) || (ppu_addr == 0) || (p_para == NULL)) {
        er_code = E_NOEXS;
        goto exit;
    }
    for (idx=0; idx<3; ++idx) {
        gain_info.quan_out[idx].dir = p_input_para->quan_out[idx].dir;
        gain_info.quan_out[idx].scale = p_input_para->quan_out[idx].scale;
        gain_info.quan_out[idx].shift = p_input_para->quan_out[idx].shift;
    }

    gen_jmisp_reg_para(p_info, p_reg_info);

    // get # of PPU with check match_ppu_id in AI_JMISP_INFO
    ppu_num = get_ppu_num(p_info);

    // ppu_addr : PPU_HEAD, PPU_INSTRS, PPU_HEAD, PPU_INSTRS, ...
    ppu_head_addr = ppu_addr;
    for (idx=0; idx<ppu_num; ++idx) {
        JMISPPPU_HEAD* p_ppu_head = (JMISPPPU_HEAD*)ppu_head_addr;
        if ((p_ppu_head->config.word != 0 && p_ppu_head->config.bit.unit_mode != e_UNIT_SEL_PPU) // note: 690/538 old model doesn't have unit_mode, and therefore will be 0 ... skip check for 690/538
            || (p_ppu_head->match_mctrl_idx != p_input_para->mctrl_id)) {
            ppu_head_addr += (sizeof(JMISPPPU_HEAD) + p_ppu_head->parm_size);
            continue;
        }

        set_gain_in_all_instr(&gain_info, p_reg_info, p_ppu_head, p_info);

        { // fix broken image
            const uintptr_t aligned_addr = ALIGN_FLOOR_64(ppu_head_addr);
            const uint32_t fill_size = ppu_head_addr - aligned_addr;
            vos_cpu_dcache_sync(aligned_addr, p_ppu_head->parm_size + fill_size, VOS_DMA_TO_DEVICE);
        }

        ppu_head_addr += (sizeof(JMISPPPU_HEAD) + p_ppu_head->parm_size);
    }

exit:
    nvt_ai_mem_free(p_reg_info);
    //p_reg_info = 0;
    return er_code;
}

ER update_gain(const AI_TUNING_QUAN_PARAM* i_para, AI_TUNING_QUAN_PARAM* o_para, const AI_FLOAT_FORMAT gain) {
#define GAIN_SCALE_MAX      ((1<<16) - 1)
#define GAIN_SHIFT_MAX      ((1<<5) - 1)
#define UINT8_MAX           (0xff)
#define UINT16_MAX          (0xffff)

    uint32_t idx;
    if ((i_para == NULL) || (o_para == NULL)
        || (gain.value == 0) || ((gain.frac_bit + gain.int_bit) > 32)) {
        DBG_ERR("Input null\r\n");
        return E_NOEXS;
    } else if ((i_para->mctrl_id < 0) || (i_para->mctrl_id != o_para->mctrl_id)) {
        DBG_ERR("Not correct mctrl\r\n");
        return E_CTX;
    }
    for (idx=0; idx<3; ++idx) {
        const AI_PPU_QUAN_SCALE_SHIFT* p_ori_gain = &i_para->quan_out[idx];
        AI_PPU_QUAN_SCALE_SHIFT* p_upd_gain = &o_para->quan_out[idx];
        AI_PPU_QUAN_SCALE_SHIFT gain_fix;
        AI_PPU_QUAN_SCALE_SHIFT* p_gain_fix = &gain_fix;
        if (p_ori_gain->scale == 0) {
            DBG_ERR("quan[%d] scale can't be zero\r\n", idx);
            return E_CTX;
        }

        { // float to quan
            p_gain_fix->dir = 0;
            p_gain_fix->shift = gain.frac_bit;
            p_gain_fix->scale = gain.value;
        }

        { //update value
            p_upd_gain->scale = p_ori_gain->scale * p_gain_fix->scale;
            if (p_ori_gain->dir == p_gain_fix->dir) {
                p_upd_gain->dir = p_ori_gain->dir;
                p_upd_gain->shift = p_ori_gain->shift + p_gain_fix->shift;
            } else if (p_ori_gain->shift >= p_gain_fix->shift) {
                p_upd_gain->dir = p_ori_gain->dir;
                p_upd_gain->shift = p_ori_gain->shift - p_gain_fix->shift;
            } else {
                p_upd_gain->dir = p_gain_fix->dir;
                p_upd_gain->shift = p_gain_fix->shift - p_ori_gain->shift;
            }
        }

        { //normalized
            uint8_t tmp_dir = p_upd_gain->dir;
            uint32_t tmp_shift = p_upd_gain->shift;
            uint32_t tmp_scale = p_upd_gain->scale;
            uint32_t j;
            for (j=0; j<GAIN_SCALE_MAX; ++j) {
                if (tmp_scale <= GAIN_SCALE_MAX) {
                    break;
                }

                if (tmp_shift == 0) {
                    tmp_dir = 1;
                    tmp_shift += 1;
                    tmp_scale >>= 1;
                } else if (tmp_dir == 1) {
                    tmp_shift += 1;
                    tmp_scale >>= 1;
                } else {
                    tmp_shift -= 1;
                    tmp_scale >>= 1;
                }
            }

            if (tmp_shift > GAIN_SHIFT_MAX) {
                const uint32_t fix_shift = tmp_shift - GAIN_SHIFT_MAX;
                tmp_shift -= fix_shift;
                if (tmp_dir == 1) {
                    tmp_scale >>= fix_shift;
                } else {
                    tmp_scale <<= fix_shift;
                }
            }

            p_upd_gain->dir = tmp_dir;
            p_upd_gain->shift = (tmp_shift > UINT8_MAX) ? UINT8_MAX : tmp_shift;
            p_upd_gain->scale = (tmp_scale > UINT16_MAX) ? UINT16_MAX : tmp_scale;
        }

        { //check boundary
            if ((p_upd_gain->scale > GAIN_SCALE_MAX) || (p_upd_gain->shift > GAIN_SHIFT_MAX)) {
                DBG_ERR("out[%d] of scale... in(%d, %d, %d) out(%d, %d, %d) gain(%d, %d, %d)\r\n"
                        , idx
                        , p_ori_gain->dir, p_ori_gain->shift, p_ori_gain->scale
                        , p_upd_gain->dir, p_upd_gain->shift, p_upd_gain->scale
                        , p_gain_fix->dir, p_gain_fix->shift, p_gain_fix->scale
                        );
                return E_CTX;
            }
        }
    }
    return E_OK;
}

ER convert_regiobuf2drvframe(JMISP_REG_IOBUF* p_regiobuf, KDRV_AI_JMISP_FRAME_INFO* p_o_frame) {
	if ((p_regiobuf == NULL) || (p_o_frame == NULL)) {
		return E_NOEXS;
	}
	
	p_o_frame->ring_en = p_regiobuf->is_from_ringbuf;
	
	p_o_frame->first_stripe_x_ofs = p_regiobuf->first_stripe_x_ofs;
	p_o_frame->middle_stripe_x_ofs = p_regiobuf->mid_stripe_x_ofs;
	p_o_frame->first_slice_y_ofs = p_regiobuf->first_slice_y_ofs;
	p_o_frame->middle_slice_y_ofs = p_regiobuf->mid_slice_y_ofs;
	p_o_frame->blk_ofs_info.blk_x_first_ofs = p_regiobuf->first_block_x_ofs;
	p_o_frame->blk_ofs_info.blk_x_middle_ofs = p_regiobuf->mid_block_x_ofs;
	p_o_frame->blk_ofs_info.blk_y_first_ofs = p_regiobuf->first_block_y_ofs;
	p_o_frame->blk_ofs_info.blk_y_middle_ofs = p_regiobuf->mid_block_y_ofs;
	
	return E_OK;
}

ER convert_regiobuf2drvpingpong(JMISP_REG_IOBUF* p_regiobuf, KDRV_AI_JMISP_PINGPONG_INFO* p_o_pingpong) {
	if ((p_regiobuf == NULL) || (p_o_pingpong == NULL)) {
		return E_NOEXS;
	}
	
	p_o_pingpong->blk_ofs_info.blk_x_first_ofs = p_regiobuf->first_block_x_ofs;
	p_o_pingpong->blk_ofs_info.blk_x_middle_ofs = p_regiobuf->mid_block_x_ofs;
	p_o_pingpong->blk_ofs_info.blk_y_first_ofs = p_regiobuf->first_block_y_ofs;
	p_o_pingpong->blk_ofs_info.blk_y_middle_ofs = p_regiobuf->mid_block_y_ofs;
	
	return E_OK;
}

ER update_jmisp_kdrv_resolution(AI_JMISP_INFO* p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_para) {
    JMISP_REG_INFO* p_reg_info;
    ER er_code = E_OK;

    p_reg_info = nvt_ai_mem_alloc(sizeof(JMISP_REG_INFO));
    if (p_reg_info == NULL) {
        DBG_ERR("p_reg_info alloc fail\r\n");
        return E_NOMEM;
    }
    else if ((p_info == NULL) || (p_drv_para == NULL)) {
        er_code = E_NOEXS;
        goto exit;
    }

    gen_jmisp_reg_para(p_info, p_reg_info);

#if AIISP_DYNAMIC_RESOLUTION_DEBUG
	{
		uint32_t i=0;
		DBG_DUMP("====== jmisp reg debug start ======\n");
		DBG_DUMP("path_id: %u\n", p_drv_para->path_id);
		DBG_DUMP("signal_mode: %u\n", p_reg_info->signal_mode);
		DBG_DUMP("stripe_x_num: %u\n", p_reg_info->stripe_x_num);
		DBG_DUMP("slice_y_num: %u\n", p_reg_info->slice_y_num);
		DBG_DUMP("first_stripe_block_x_num: %u\n", p_reg_info->first_stripe_block_x_num);
		DBG_DUMP("mid_stripe_block_x_num: %u\n", p_reg_info->mid_stripe_block_x_num);
		DBG_DUMP("last_stripe_block_x_num: %u\n", p_reg_info->last_stripe_block_x_num);
		DBG_DUMP("first_slice_block_y_num: %u\n", p_reg_info->first_slice_block_y_num);
		DBG_DUMP("mid_slice_block_y_num: %u\n", p_reg_info->mid_slice_block_y_num);
		DBG_DUMP("last_slice_block_y_num: %u\n", p_reg_info->last_slice_block_y_num);
		for (i=0; i<e_AI_JMISP_REG_IO_NUM; ++i) {
			JMISP_REG_IOBUF* pbuf = &p_reg_info->iobuf_reg[i];
			DBG_DUMP("iobuf[ %u ]\n", i);
			DBG_DUMP("is_from_ringbuf: %d\n", pbuf->is_from_ringbuf);
			DBG_DUMP("block_lofs: %d\n", pbuf->block_lofs);
			DBG_DUMP("first_stripe_x_ofs: %d\n", pbuf->first_stripe_x_ofs);
			DBG_DUMP("mid_stripe_x_ofs: %d\n", pbuf->mid_stripe_x_ofs);
			DBG_DUMP("first_slice_y_ofs: %d\n", pbuf->first_slice_y_ofs);
			DBG_DUMP("mid_slice_y_ofs: %d\n", pbuf->mid_slice_y_ofs);
			DBG_DUMP("first_block_x_ofs: %d\n", pbuf->first_block_x_ofs);
			DBG_DUMP("mid_block_x_ofs: %d\n", pbuf->mid_block_x_ofs);
			DBG_DUMP("first_block_y_ofs: %d\n", pbuf->first_block_y_ofs);
			DBG_DUMP("mid_block_y_ofs: %d\n", pbuf->mid_block_y_ofs);
			DBG_DUMP("first_stripe_w: %d\n", pbuf->first_stripe_w);
			DBG_DUMP("mid_stripe_w: %d\n", pbuf->mid_stripe_w);
			DBG_DUMP("last_stripe_w: %d\n", pbuf->last_stripe_w);
			DBG_DUMP("first_slice_h: %d\n", pbuf->first_slice_h);
			DBG_DUMP("mid_slice_h: %d\n", pbuf->mid_slice_h);
			DBG_DUMP("last_slice_h: %d\n", pbuf->last_slice_h);
		}
		DBG_DUMP("====== jmisp reg debug end ======\n");
	}
#endif
    {
        uint32_t i=0;
        for (i=e_AI_JMISP_REG_IO_RING_BUF_BEGIN; i<e_AI_JMISP_REG_IO_RING_BUF_END; ++i) {
            JMISP_REG_IOBUF* pbuf = &p_reg_info->iobuf_reg[i];
            e_AI_JMISP_DRV_IO_CONST drv_io_type = convert_regbuf2drvio_type(i);
            if (drv_io_type >= 0) {
                convert_regiobuf2drvframe(pbuf, &p_drv_para->frame[drv_io_type]);
            }
        }
        for (i=e_AI_JMISP_REG_IO_FRAME_BUF_BEGIN; i<e_AI_JMISP_REG_IO_FRAME_BUF_END; ++i) {
            JMISP_REG_IOBUF* pbuf = &p_reg_info->iobuf_reg[i];
            e_AI_JMISP_DRV_IO_CONST drv_io_type = convert_regbuf2drvio_type(i);
            if (drv_io_type >= 0) {
                convert_regiobuf2drvframe(pbuf, &p_drv_para->frame[drv_io_type]);
            }
        }
        for (i=e_AI_JMISP_REG_IO_PINGPONG_BUF_BEGIN; i<e_AI_JMISP_REG_IO_PINGPONG_BUF_END; ++i) {
            JMISP_REG_IOBUF* pbuf = &p_reg_info->iobuf_reg[i];
            e_AI_JMISP_DRV_IO_CONST drv_io_type = convert_regbuf2drvio_type(i);
            if (drv_io_type >= 0) {
                convert_regiobuf2drvpingpong(pbuf, &p_drv_para->pingpong[drv_io_type]);
            }
        }
    }

    p_drv_para->stripe_num = p_reg_info->stripe_x_num;
    p_drv_para->slice_num = p_reg_info->slice_y_num;
    p_drv_para->signal_mode_en = p_reg_info->signal_mode;
    p_drv_para->blk_num_info.blk_x_first_num = p_reg_info->first_stripe_block_x_num;
    p_drv_para->blk_num_info.blk_x_middle_num = p_reg_info->mid_stripe_block_x_num;
    p_drv_para->blk_num_info.blk_x_last_num = p_reg_info->last_stripe_block_x_num;
    p_drv_para->blk_num_info.blk_y_first_num = p_reg_info->first_slice_block_y_num;
    p_drv_para->blk_num_info.blk_y_middle_num = p_reg_info->mid_slice_block_y_num;
    p_drv_para->blk_num_info.blk_y_last_num = p_reg_info->last_slice_block_y_num;

exit:
    nvt_ai_mem_free(p_reg_info);
    //p_reg_info = 0;
    return er_code;
    }


//for frame mode wo PPU pipe
uint64_t ispframemode_cal_blk_rel_addr(JMISP_REG_IOBUF* p_iobuf_reg, uint64_t blk_x, uint64_t blk_y, uint64_t stripe_x, uint64_t slice_y) {

	uint64_t rel_addr = 0;

	if (p_iobuf_reg == NULL) {
		return 0;
	}

	if (blk_y==0) {
		rel_addr += 0;
	} else {
		if (slice_y == 0) {
			rel_addr += p_iobuf_reg->first_block_y_ofs;
		} else {
			rel_addr += p_iobuf_reg->mid_block_y_ofs;
		}
		rel_addr += p_iobuf_reg->mid_block_y_ofs * (blk_y - 1);
	}

	if (blk_x==0) {
		rel_addr += 0;
	} else {
		rel_addr += p_iobuf_reg->first_block_x_ofs;
		rel_addr += p_iobuf_reg->mid_block_x_ofs * (blk_x - 1);
	}

	return rel_addr;
}

uint32_t ispframemode_get_total_blk_num(VOID* p_info) {
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t x_num, y_num;
	if (p_reg_info == NULL) {
		return 0;
	}
	
	if (p_reg_info->stripe_x_num > 1) {
		x_num = p_reg_info->first_stripe_block_x_num + p_reg_info->last_stripe_block_x_num + p_reg_info->mid_stripe_block_x_num*(p_reg_info->stripe_x_num-2);
	} else {
		x_num = p_reg_info->first_stripe_block_x_num;
	}
	
	if (p_reg_info->slice_y_num > 1) {
		y_num = p_reg_info->first_slice_block_y_num + p_reg_info->last_slice_block_y_num + p_reg_info->mid_slice_block_y_num*(p_reg_info->slice_y_num-2);
	} else {
		y_num = p_reg_info->first_slice_block_y_num;
	}

	return (x_num * y_num);
}

uint32_t ispframemode_get_max_blk_num_in_a_slice(VOID* p_info) {
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t x_num, y_num;
	if (p_reg_info == NULL) {
		return 0;
	}

	x_num = max(max(p_reg_info->first_stripe_block_x_num, p_reg_info->last_stripe_block_x_num), p_reg_info->mid_stripe_block_x_num);
	y_num = max(max(p_reg_info->first_slice_block_y_num, p_reg_info->last_slice_block_y_num), p_reg_info->mid_slice_block_y_num);

	return (x_num * y_num);
}

uint32_t ispframemode_get_blk_num_in_a_slice(VOID* p_info, uint32_t slice_idx) {
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t stripe_x_num, slice_y_num, stripe_x_idx, slice_y_idx;
	uint32_t x_num, y_num;
	if ((p_reg_info == NULL)
		|| (slice_idx >= ispframemode_get_total_slice_num(p_reg_info))) {
		return 0;
	}

	stripe_x_num = p_reg_info->stripe_x_num;
	slice_y_num = p_reg_info->slice_y_num;
	stripe_x_idx = slice_idx / slice_y_num;
	slice_y_idx = slice_idx % slice_y_num;

	if (stripe_x_idx == 0) {
		x_num = p_reg_info->first_stripe_block_x_num;
	} else if (stripe_x_idx == (stripe_x_num - 1)) {
		x_num = p_reg_info->last_stripe_block_x_num;
	} else {
		x_num = p_reg_info->mid_stripe_block_x_num;
	}

	if (slice_y_idx == 0) {
		y_num = p_reg_info->first_slice_block_y_num;
	} else if (slice_y_idx == (slice_y_num - 1)) {
		y_num = p_reg_info->last_slice_block_y_num;
	} else {
		y_num = p_reg_info->mid_slice_block_y_num;
	}

	return (x_num * y_num);
}

ER ispframemode_create_blknum_array(VOID *p_info, uint32_t* p_blknum, const uint32_t cnt) {
	
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t stripe_num, slice_num, stripe_x = 0, slice_y = 0, idx=0;
	if ((p_reg_info == NULL) || (p_blknum == NULL) || (cnt == 0)) {
		return E_NOEXS;
	}
	
	stripe_num = p_reg_info->stripe_x_num;
	slice_num = p_reg_info->slice_y_num;
	if (ispframemode_get_total_slice_num(p_info) > cnt) {
		DBG_ERR("array not enough\r\n");
		return E_PAR;
	}

	idx = 0;
	for (stripe_x=0; stripe_x<stripe_num; ++stripe_x) {
		uint32_t blk_x_num;
		if (stripe_x == 0) {
			blk_x_num = p_reg_info->first_stripe_block_x_num;
		} else if (stripe_x == (stripe_num - 1)) {
			blk_x_num = p_reg_info->last_stripe_block_x_num;
		} else {
			blk_x_num = p_reg_info->mid_stripe_block_x_num;
		}
		
		for (slice_y=0; slice_y<slice_num; ++slice_y) {
			uint32_t blk_y_num;
			if (slice_y == 0) {
				blk_y_num = p_reg_info->first_slice_block_y_num;
			} else if (slice_y == (slice_num - 1)) {
				blk_y_num = p_reg_info->last_slice_block_y_num;
			} else {
				blk_y_num = p_reg_info->mid_slice_block_y_num;
			}

			p_blknum[idx] = blk_x_num * blk_y_num;
			++idx;
		}
	}
	return E_OK;
}

ER ispframemode_create_blkinfo_array(AI_JMISP_INFO* p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt) {
	AI_JMISP_IOBUF_INFO *p_tmp_iobuf;
	JMISP_REG_INFO *p_reg_info;
	uint32_t x = 0, y = 0, stripe_x = 0, slice_y = 0;
	uint32_t idx=0, j=0;
    const uint32_t tmp_buf_sz = sizeof(AI_JMISP_IOBUF_INFO) * e_AI_IOBUF_NUM;
	ER err;

	p_reg_info = nvt_ai_mem_alloc(sizeof(JMISP_REG_INFO));
    p_tmp_iobuf = nvt_ai_mem_alloc(tmp_buf_sz);
	if ((p_reg_info == NULL) || (p_info == NULL) || (p_drv_array == NULL) || (cnt == 0) || (p_tmp_iobuf == NULL)) {
		DBG_ERR("p_reg_info alloc fail\r\n");
		err = E_NOEXS;
		goto exit;
	}

	gen_jmisp_reg_para(p_info, p_reg_info);
	if (p_reg_info->stripe_x_num > 1) {
		DBG_ERR("not support frame mode if stripe > 1\r\n");
		err = E_PAR;
		goto exit;
	}

	if (ispframemode_get_total_blk_num(p_reg_info) != cnt) {
		DBG_ERR("drv array not enough\r\n");
		err = E_PAR;
		goto exit;
	}

	idx = 0;
	for (stripe_x=0; stripe_x<p_reg_info->stripe_x_num; ++stripe_x) {
		uint32_t blk_x_cnt;
		if (stripe_x == 0) {
			blk_x_cnt = p_reg_info->first_stripe_block_x_num;
		} else if (stripe_x == (p_reg_info->stripe_x_num - 1)) {
			blk_x_cnt = p_reg_info->last_stripe_block_x_num;
		} else {
			blk_x_cnt = p_reg_info->mid_stripe_block_x_num;
		}
		
		//init
		memcpy(p_tmp_iobuf, p_info->iobuf, tmp_buf_sz);
		if (stripe_x > 0) {
			uint32_t st_idx=0;

            for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                    continue;
                }
                p_tmp_iobuf[iobuf_type].pa += p_reg_info->iobuf_reg[regbuf_type].first_stripe_x_ofs;
            }
            for (st_idx=1; st_idx<stripe_x; ++st_idx) {
                for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                    const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                    const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                    if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                        continue;
                    }
                    p_tmp_iobuf[iobuf_type].pa += p_reg_info->iobuf_reg[regbuf_type].mid_stripe_x_ofs;
                }
            }
        }
		
		for (slice_y=0; slice_y<p_reg_info->slice_y_num; ++slice_y) {
			uint32_t blk_y_cnt;
			if (slice_y == 0) {
				blk_y_cnt = p_reg_info->first_slice_block_y_num;
			} else if (slice_y == (p_reg_info->slice_y_num - 1)) {
				blk_y_cnt = p_reg_info->last_slice_block_y_num;
			} else {
				blk_y_cnt = p_reg_info->mid_slice_block_y_num;
			}

			if (slice_y == 0) {
			} else if (slice_y == 1) {
                for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                    const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                    const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                    if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                        continue;
                    }
                    p_tmp_iobuf[iobuf_type].pa += p_reg_info->iobuf_reg[regbuf_type].first_slice_y_ofs;
                }
			} else {
                for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                    const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                    const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                    if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                        continue;
                    }
                    p_tmp_iobuf[iobuf_type].pa += p_reg_info->iobuf_reg[regbuf_type].mid_slice_y_ofs;
                }
			}


			for (y=0; y<blk_y_cnt; ++y) {
				for (x=0; x<blk_x_cnt; ++x, ++idx) {

					KDRV_AI_JMISP_PATH_INFO* p_drv = &p_drv_array[idx];
					uint64_t cur_addr = 0;
					uint32_t blk_case = 0, is_top, is_bottom, is_left, is_right;
					uint32_t stripe_case = 0;
					if (idx >= cnt) {
						break;
					}

					p_drv->handshake = KDRV_AI_JMISP_HANDSHAKE_CPU;
					p_drv->signal_mode_en = false;
					//p_drv->path_id;

					p_drv->stripe_num = 1;
					p_drv->slice_num = 1;
					p_drv->blk_num_info.blk_x_first_num = 1;
					p_drv->blk_num_info.blk_x_middle_num = 1;
					p_drv->blk_num_info.blk_x_last_num = 1;
					p_drv->blk_num_info.blk_y_first_num = 1;
					p_drv->blk_num_info.blk_y_middle_num = 1;
					p_drv->blk_num_info.blk_y_last_num = 1;

					//init addr
					memset(p_drv->frame, 0, sizeof(KDRV_AI_JMISP_FRAME_INFO) * KDRV_AI_JMISP_MAX_FRAME_NUM);
					memset(p_drv->pingpong, 0, sizeof(KDRV_AI_JMISP_PINGPONG_INFO) * KDRV_AI_JMISP_MAX_PINGPONG_NUM);
					memset(p_drv->ppu_pipebuf_lsbaddr, 0, sizeof(p_drv->ppu_pipebuf_lsbaddr));

                    for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                        const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                        const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                        const e_AI_JMISP_DRV_IO_CONST drvio_type = convert_regbuf2drvio_type(regbuf_type);
                        if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                            continue;
                        }
                        cur_addr = ispframemode_cal_blk_rel_addr(&p_reg_info->iobuf_reg[regbuf_type], x, y, stripe_x, slice_y);
                        if ((regbuf_type >= e_AI_JMISP_REG_IO_PINGPONG_BUF_BEGIN) && (regbuf_type < e_AI_JMISP_REG_IO_PINGPONG_BUF_END)) {
                            p_drv->pingpong[drvio_type].pingpong_addr0 = p_tmp_iobuf[iobuf_type].pa + cur_addr;
					        p_drv->pingpong[drvio_type].pingpong_addr1 = p_drv->pingpong[drvio_type].pingpong_addr0;
                        } else {
                            p_drv->frame[drvio_type].frame_addr = p_tmp_iobuf[iobuf_type].pa + cur_addr;
                        }
                    }

					is_top = (y == 0) && (slice_y == 0);
					is_bottom = (y==(blk_y_cnt - 1)) && (slice_y == (p_reg_info->slice_y_num - 1));
					is_left = (x==0);
					is_right = (x == (blk_x_cnt - 1));
					if (is_left && is_top) {
						blk_case = 0;	//left-top
					} else if (is_left && is_bottom) {
						blk_case = 6;	//left-bottom
					} else if (is_right && is_top) {
						blk_case = 2;	//right-top
					} else if (is_right && is_bottom) {
						blk_case = 8;	//right-bottom
					} else if (is_top) {
						blk_case = 1;	//top
					} else if (is_bottom) {
						blk_case = 7;	//bottom
					} else if (is_left) {
						blk_case = 3;	//left
					} else if (is_right) {
						blk_case = 5;	//right
					} else {
						blk_case = 4;	//mid
					}

					if (stripe_x==0) {
						stripe_case = 0;	//left
					} else if (stripe_x == (p_reg_info->stripe_x_num-1)) {
						stripe_case = 2;	//right
					} else {
						stripe_case = 1;	//mid
					}

					//0x2C0: record block case
					p_drv->cust[0] = (stripe_case << 24) + blk_case;
					//0x2C4: record index
					p_drv->cust[1] = ((slice_y & 0xff) << 24) + ((stripe_x & 0xff) << 16) + ((y & 0xff) << 8) + ((x & 0xff));
				}
			}
		}

	}

#if AIISP_FRAME_MODE_DEBUG
DBG_DUMP("=========== ispframemode_create_blkinfo_array debug start ============  \n" );
{
	DBG_DUMP("signal_mode=%d; stripe_x_num=%d; slice_y_num=%d;\n", p_reg_info->signal_mode, p_reg_info->stripe_x_num, p_reg_info->slice_y_num);
	DBG_DUMP("first_stripe_block_x_num=%d; mid_stripe_block_x_num=%d; last_stripe_block_x_num=%d;\n", p_reg_info->first_stripe_block_x_num, p_reg_info->mid_stripe_block_x_num, p_reg_info->last_stripe_block_x_num);
	DBG_DUMP("first_slice_block_y_num=%d; mid_slice_block_y_num=%d; last_slice_block_y_num=%d;\n", p_reg_info->first_slice_block_y_num, p_reg_info->mid_slice_block_y_num, p_reg_info->last_slice_block_y_num);
	for (idx=0; idx<e_AI_JMISP_REG_IO_NUM; ++idx) {
		JMISP_REG_IOBUF* p_iobuf = &p_reg_info->iobuf_reg[idx];
		DBG_DUMP("[%d] is_from_ringbuf=%d; block_lofs=%d;\n", idx, p_iobuf->is_from_ringbuf, p_iobuf->block_lofs);
		DBG_DUMP("---- first_stripe_x_ofs=%d; mid_stripe_x_ofs=%d;\n", p_iobuf->first_stripe_x_ofs, p_iobuf->mid_stripe_x_ofs);
		DBG_DUMP("---- first_slice_y_ofs=%d; mid_slice_y_ofs=%d;\n", p_iobuf->first_slice_y_ofs, p_iobuf->mid_slice_y_ofs);
		DBG_DUMP("---- first_block_x_ofs=%d; mid_block_x_ofs=%d;\n", p_iobuf->first_block_x_ofs, p_iobuf->mid_block_x_ofs);
		DBG_DUMP("---- first_block_y_ofs=%d; mid_block_y_ofs=%d;\n", p_iobuf->first_block_y_ofs, p_iobuf->mid_block_y_ofs);

		DBG_DUMP("---- first_stripe_w=%d; mid_stripe_w=%d;last_stripe_w=%d\n", p_iobuf->first_stripe_w, p_iobuf->mid_stripe_w, p_iobuf->last_stripe_w);
		DBG_DUMP("---- first_slice_h=%d; mid_slice_h=%d;last_slice_h=%d\n", p_iobuf->first_slice_h, p_iobuf->mid_slice_h, p_iobuf->last_slice_h);
	}
}
{
	idx = 0;
	for (stripe_x=0; stripe_x<p_reg_info->stripe_x_num; ++stripe_x) {
		uint32_t blk_x_cnt;
		if (stripe_x == 0) {
			blk_x_cnt = p_reg_info->first_stripe_block_x_num;
		} else if (stripe_x == (p_reg_info->stripe_x_num - 1)) {
			blk_x_cnt = p_reg_info->last_stripe_block_x_num;
		} else {
			blk_x_cnt = p_reg_info->mid_stripe_block_x_num;
		}

		for (slice_y=0; slice_y<p_reg_info->slice_y_num; ++slice_y) {
			uint32_t blk_y_cnt;
			if (slice_y == 0) {
				blk_y_cnt = p_reg_info->first_slice_block_y_num;
			} else if (slice_y == (p_reg_info->slice_y_num - 1)) {
				blk_y_cnt = p_reg_info->last_slice_block_y_num;
			} else {
				blk_y_cnt = p_reg_info->mid_slice_block_y_num;
			}
		
			for (y=0; y<blk_y_cnt; ++y) {
				for (x=0; x<blk_x_cnt; ++x, ++idx) {
					uint32_t i=0;
					KDRV_AI_JMISP_PATH_INFO* p_drv = &p_drv_array[idx];

					if ((x > 0) && (x < (blk_x_cnt - 1))) {
						continue;
					} else if ((y > 0) && (y < (blk_y_cnt - 1))) {
						continue;
					}
					
					DBG_DUMP("--- stripe = %d; slice = %d; x = %d; y = %d; idx = %d; ---\n", (int)stripe_x, (int)slice_y, (int)x, (int)y, (int)idx);
					for (i=0; i<KDRV_AI_JMISP_MAX_PINGPONG_NUM; ++i) {
						DBG_DUMP("p_drv->pingpong[%d].pingpong_addr0  = 0x%016lx \n",  (int)i, (ULONG)p_drv->pingpong[i].pingpong_addr0 );
						DBG_DUMP("p_drv->pingpong[%d].pingpong_addr1  = 0x%016lx\n",   (int)i, (ULONG)p_drv->pingpong[i].pingpong_addr1);
					}
					for (i=0; i<KDRV_AI_JMISP_MAX_FRAME_NUM; ++i) {
						DBG_DUMP("p_drv->frame[%d].ring_start_addr  = 0x%016lx \n",  (int)i, (ULONG)p_drv->frame[i].ring_start_addr );
						DBG_DUMP("p_drv->frame[%d].ring_end_addr 0x%016lx\n",        (int)i, (ULONG)p_drv->frame[i].ring_end_addr);
						DBG_DUMP("p_drv->frame[%d].ring_en   = %u \n",          (int)i,        p_drv->frame[i].ring_en );
						DBG_DUMP("p_drv->frame[%d].frame_addr  = 0x%016lx \n",       (int)i, (ULONG)p_drv->frame[i].frame_addr);
					}
					DBG_DUMP( "p_drv->path_id = %d \n",       (int)p_drv->path_id);
					DBG_DUMP( "p_drv->handshake = %d \n", (int) p_drv->handshake);

					DBG_DUMP( "p_drv->signal_mode_en = %d \n",       (int)p_drv->signal_mode_en);
					DBG_DUMP( "p_drv->stripe_num = %d; p_drv->slice_num = %d;\n", (int) p_drv->stripe_num, (int) p_drv->slice_num);
					DBG_DUMP( "p_drv->blk_num_info: x_first_num = %d; x_middle_num = %d; x_last_num = %d;\n", (int) p_drv->blk_num_info.blk_x_first_num, (int) p_drv->blk_num_info.blk_x_middle_num, (int) p_drv->blk_num_info.blk_x_last_num);
					DBG_DUMP( "p_drv->blk_num_info: y_first_num = %d; y_middle_num = %d; y_last_num = %d;\n", (int) p_drv->blk_num_info.blk_y_first_num, (int) p_drv->blk_num_info.blk_y_middle_num, (int) p_drv->blk_num_info.blk_y_last_num);
					for (i=0; i<KDRV_AI_JMISP_MAX_CUST_NUM; ++i) {
						DBG_DUMP("p_drv-> cust[%d] = 0x%08x;\n", (int)i, p_drv->cust[i]);
					}
					for (i=0; i<KDRV_AI_JMISP_MAX_PIPEBUFF_NUM; ++i) {
						DBG_DUMP("p_drv-> pipebuf[%d] = 0x%08x;\n", (int)i, p_drv->ppu_pipebuf_lsbaddr[i]);
					}
				}
			}
		}
	}
}
DBG_DUMP("=========== ispframemode_create_blkinfo_array debug end ============  \n" );
#endif

	err = E_OK;


exit:
    if (p_reg_info != NULL) {
        nvt_ai_mem_free(p_reg_info);
        //p_reg_info = 0;
    }
    if (p_tmp_iobuf != NULL) {
        nvt_ai_mem_free(p_tmp_iobuf);
        //p_tmp_iobuf = 0;
    }
    return err;
}

ER ispframemode_upd_blk_pingpong_addr(VOID *p_reg_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
										, const uint64_t pp_addr0, const uint64_t pp_addr1) {
	ispframemode_upd_pingpong_addr(p_reg_info, p_drv_array,cnt, type, pp_addr0, pp_addr1, 0, 0);
	return E_OK;
}

ER ispframemode_upd_blk_ringbuf_addr(VOID *p_reg_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
										, bool ring_en, const uint64_t ring_saddr, const uint64_t ring_eaddr) {
	ispframemode_upd_ringbuf_addr(p_reg_info, p_drv_array, cnt, type, ring_en, ring_saddr, ring_eaddr, 0, 0);
	return E_OK;
}


//for frame mode with PPU pipe
ER ispframemode_cal_substripe_frame(JMISP_REG_IOBUF* p_iobuf_reg, KDRV_AI_JMISP_FRAME_INFO* p_out_frame_info, uint32_t* p_substripe_blk_x_cnt_array, uint64_t start_addr, uint64_t substripe_x, uint64_t slice_y) {

	uint64_t cur_addr=0;
	if ((p_iobuf_reg == NULL) || (p_out_frame_info == NULL) || (p_substripe_blk_x_cnt_array == NULL)) {
		return E_NOEXS;
	}
	if (substripe_x > 0) {
		uint32_t i=0;
		for (i=0; i<substripe_x; ++i) {
			const uint64_t blk_cnt = p_substripe_blk_x_cnt_array[i];
			if (i == 0) {
				cur_addr += p_iobuf_reg->first_block_x_ofs;
			} else {
				cur_addr += p_iobuf_reg->mid_block_x_ofs;
			}
			if (blk_cnt == 0) {
				DBG_ERR("block num can't be 0 in substripe[%d]\r\n", i);
				continue;
			}
			cur_addr += (uint64_t)p_iobuf_reg->mid_block_x_ofs * (blk_cnt - 1);
		}
	}

	p_out_frame_info->frame_addr = start_addr + cur_addr;
	p_out_frame_info->blk_ofs_info.blk_x_first_ofs = (substripe_x==0) ? p_iobuf_reg->first_block_x_ofs : p_iobuf_reg->mid_block_x_ofs;
	p_out_frame_info->blk_ofs_info.blk_x_middle_ofs = p_iobuf_reg->mid_block_x_ofs;
	p_out_frame_info->blk_ofs_info.blk_y_first_ofs = (slice_y==0) ? p_iobuf_reg->first_block_y_ofs : p_iobuf_reg->mid_block_y_ofs;
	p_out_frame_info->blk_ofs_info.blk_y_middle_ofs = p_iobuf_reg->mid_block_y_ofs;
	return E_OK;
}

ER ispframemode_cal_substripe_pingpong(JMISP_REG_IOBUF* p_iobuf_reg, KDRV_AI_JMISP_PINGPONG_INFO* p_out_pingpong_info, uint32_t* p_substripe_blk_x_cnt_array, uint64_t start_addr, uint64_t substripe_x, uint64_t slice_y) {

	uint64_t cur_addr=0;
	if ((p_iobuf_reg == NULL) || (p_out_pingpong_info == NULL) || (p_substripe_blk_x_cnt_array == NULL)) {
		return E_NOEXS;
	}
	if (substripe_x > 0) {
		uint32_t i=0;
		for (i=0; i<substripe_x; ++i) {
			const uint64_t blk_cnt = p_substripe_blk_x_cnt_array[i];
			if (i == 0) {
				cur_addr += p_iobuf_reg->first_block_x_ofs;
			} else {
				cur_addr += p_iobuf_reg->mid_block_x_ofs;
			}
			if (blk_cnt == 0) {
				DBG_ERR("block num can't be 0 in substripe[%d]\r\n", i);
				continue;
			}
			cur_addr += (uint64_t)p_iobuf_reg->mid_block_x_ofs * (blk_cnt - 1);
		}
	}

	p_out_pingpong_info->pingpong_addr0 = start_addr + cur_addr;
	p_out_pingpong_info->pingpong_addr1 = p_out_pingpong_info->pingpong_addr0;
	p_out_pingpong_info->blk_ofs_info.blk_x_first_ofs = (substripe_x==0) ? p_iobuf_reg->first_block_x_ofs : p_iobuf_reg->mid_block_x_ofs;
	p_out_pingpong_info->blk_ofs_info.blk_x_middle_ofs = p_iobuf_reg->mid_block_x_ofs;
	p_out_pingpong_info->blk_ofs_info.blk_y_first_ofs = (slice_y==0) ? p_iobuf_reg->first_block_y_ofs : p_iobuf_reg->mid_block_y_ofs;
	p_out_pingpong_info->blk_ofs_info.blk_y_middle_ofs = p_iobuf_reg->mid_block_y_ofs;
	return E_OK;
}

uint32_t ispframemode_get_total_substripe_num(VOID* p_info, const uint32_t core_num) {

	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t x_num;

	if (p_reg_info == NULL) {
		return 0;
	}

	x_num = min((uint32_t)p_reg_info->first_stripe_block_x_num, core_num);
	if (p_reg_info->stripe_x_num > 1) {
		x_num += min((uint32_t)p_reg_info->last_stripe_block_x_num, core_num);
		x_num += min((uint32_t)p_reg_info->mid_stripe_block_x_num, core_num) * (p_reg_info->stripe_x_num - 2);
	}

	return x_num * p_reg_info->slice_y_num;
}

uint32_t ispframemode_get_max_substripe_num_in_a_slice(VOID* p_info, const uint32_t core_num) {
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t x_num, first_stripe_substripe_num, mid_stripe_substripe_num, last_stripe_substripe_num;

	if (p_reg_info == NULL) {
		return 0;
	}

	first_stripe_substripe_num = min((uint32_t)p_reg_info->first_stripe_block_x_num, core_num);
	mid_stripe_substripe_num = min((uint32_t)p_reg_info->mid_stripe_block_x_num, core_num);
	last_stripe_substripe_num = min((uint32_t)p_reg_info->last_stripe_block_x_num, core_num);
	x_num = max(max( first_stripe_substripe_num, last_stripe_substripe_num), mid_stripe_substripe_num);

	return x_num;
}

uint32_t ispframemode_get_substripe_num_in_a_slice(VOID* p_info, uint32_t slice_idx, const uint32_t core_num) {
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t stripe_x_num, slice_y_num, stripe_x_idx;
	uint32_t x_num;

	if ((p_reg_info == NULL)
		|| (slice_idx >= ispframemode_get_total_slice_num(p_reg_info))) {
		return 0;
	}

	stripe_x_num = p_reg_info->stripe_x_num;
	slice_y_num = p_reg_info->slice_y_num;
	stripe_x_idx = slice_idx / slice_y_num;

	if (stripe_x_idx == 0) {
		x_num = min((uint32_t)p_reg_info->first_stripe_block_x_num, core_num);
	} else if (stripe_x_idx == (stripe_x_num - 1)) {
		x_num = min((uint32_t)p_reg_info->last_stripe_block_x_num, core_num);
	} else {
		x_num = min((uint32_t)p_reg_info->mid_stripe_block_x_num, core_num);
	}

	return x_num;
}

ER ispframemode_create_substripe_num_array(VOID *p_info, uint32_t* p_substripe_num, const uint32_t cnt, const uint32_t core_num) {

	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t stripe_num, slice_num, stripe_x = 0, slice_y = 0, idx=0;

	if ((p_reg_info == NULL) || (p_substripe_num == NULL) || (cnt == 0)) {
		return E_NOEXS;
	}

	stripe_num = p_reg_info->stripe_x_num;
	slice_num = p_reg_info->slice_y_num;
	if (ispframemode_get_total_slice_num(p_info) > cnt) {
		DBG_ERR("array not enough\r\n");
		return E_PAR;
	}

	idx = 0;
	for (stripe_x=0; stripe_x<stripe_num; ++stripe_x) {
		uint32_t substripe_num;
		if (stripe_x == 0) {
			substripe_num = min((uint32_t)p_reg_info->first_stripe_block_x_num, core_num);
		} else if (stripe_x == (stripe_num - 1)) {
			substripe_num = min((uint32_t)p_reg_info->last_stripe_block_x_num, core_num);
		} else {
			substripe_num = min((uint32_t)p_reg_info->mid_stripe_block_x_num, core_num);
		}

		for (slice_y=0; slice_y<slice_num; ++slice_y) {
			p_substripe_num[idx] = substripe_num;
			++idx;
		}
	}

	return E_OK;
}

ER ispframemode_create_substripe_array(AI_JMISP_INFO* p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, const uint32_t core_num) {
	AI_JMISP_IOBUF_INFO *p_tmp_iobuf;
	JMISP_REG_INFO *p_reg_info;
	uint32_t substripe_x = 0, stripe_x = 0, slice_y = 0;
	uint32_t idx=0, j=0;
    const uint32_t tmp_buf_sz = sizeof(AI_JMISP_IOBUF_INFO) * e_AI_IOBUF_NUM;
	ER err;

    p_tmp_iobuf = nvt_ai_mem_alloc(tmp_buf_sz);
	p_reg_info = nvt_ai_mem_alloc(sizeof(JMISP_REG_INFO));
	if ((p_reg_info == NULL) || (p_info == NULL) || (p_drv_array == NULL) || (cnt == 0) || (p_tmp_iobuf == NULL)) {
		DBG_ERR("p_reg_info alloc fail\r\n");
		err = E_NOEXS;
		goto exit;
	}

	gen_jmisp_reg_para(p_info, p_reg_info);
	if (p_reg_info->stripe_x_num > 1) {
		DBG_ERR("not support frame mode if stripe > 1\r\n");
		err = E_PAR;
		goto exit;
	}

	if (ispframemode_get_total_substripe_num(p_reg_info, core_num) != cnt) {
		DBG_ERR("drv array not enough\r\n");
		err = E_PAR;
		goto exit;
	}

    {
        uint32_t total_blk_x_cnt, total_blk_y_cnt;

        total_blk_x_cnt = p_reg_info->first_stripe_block_x_num;
        if (p_reg_info->stripe_x_num > 1) {
            total_blk_x_cnt += p_reg_info->last_stripe_block_x_num;
        }
        if (p_reg_info->stripe_x_num > 2) {
            total_blk_x_cnt += p_reg_info->mid_stripe_block_x_num;
        }

        total_blk_y_cnt = p_reg_info->first_slice_block_y_num;
        if (p_reg_info->slice_y_num > 1) {
            total_blk_y_cnt += p_reg_info->last_slice_block_y_num;
        }
        if (p_reg_info->slice_y_num > 2) {
            total_blk_y_cnt += p_reg_info->mid_slice_block_y_num;
        }

        if ((total_blk_x_cnt < 3) || (total_blk_y_cnt < 3)) {
            DBG_WRN("May use wrong ppu case if block num(%d, %d) not enough\n", total_blk_x_cnt, total_blk_y_cnt);
        }
    }

	idx = 0;
	for (stripe_x=0; stripe_x<p_reg_info->stripe_x_num; ++stripe_x) {
		uint32_t blk_x_cnt, substripe_cnt;
		uint32_t* sub_blk_x_cnt_array;

		if (stripe_x == 0) {
			blk_x_cnt = p_reg_info->first_stripe_block_x_num;
		} else if (stripe_x == (p_reg_info->stripe_x_num - 1)) {
			blk_x_cnt = p_reg_info->last_stripe_block_x_num;
		} else {
			blk_x_cnt = p_reg_info->mid_stripe_block_x_num;
		}
		substripe_cnt = min(blk_x_cnt, core_num);

		sub_blk_x_cnt_array = nvt_ai_mem_alloc(sizeof(uint32_t) * substripe_cnt);
		for (substripe_x=0; substripe_x<substripe_cnt; ++substripe_x) {
			uint32_t cnt = blk_x_cnt / substripe_cnt;
			if (substripe_x < (blk_x_cnt % substripe_cnt)) {
				++cnt;
			}
			sub_blk_x_cnt_array[substripe_x] = cnt;
		}

		//init
		memcpy(p_tmp_iobuf, p_info->iobuf, tmp_buf_sz);
		if (stripe_x > 0) {
			uint32_t st_idx=0;
            for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                    continue;
                }
                p_tmp_iobuf[iobuf_type].pa += p_reg_info->iobuf_reg[regbuf_type].first_stripe_x_ofs;
            }
			for (st_idx=1; st_idx<stripe_x; ++st_idx) {
                for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                    const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                    const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                    if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                        continue;
                    }
                    p_tmp_iobuf[iobuf_type].pa += p_reg_info->iobuf_reg[regbuf_type].mid_stripe_x_ofs;
                }
			}
		}

		for (slice_y=0; slice_y<p_reg_info->slice_y_num; ++slice_y) {
			uint32_t blk_y_cnt;
			if (slice_y == 0) {
				blk_y_cnt = p_reg_info->first_slice_block_y_num;
			} else if (slice_y == (p_reg_info->slice_y_num - 1)) {
				blk_y_cnt = p_reg_info->last_slice_block_y_num;
			} else {
				blk_y_cnt = p_reg_info->mid_slice_block_y_num;
			}

			if (slice_y == 1) {
                for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                    const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                    const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                    if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                        continue;
                    }
                    p_tmp_iobuf[iobuf_type].pa += p_reg_info->iobuf_reg[regbuf_type].first_slice_y_ofs;
                }
			} else if (slice_y > 1) {
                for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                    const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                    const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                    if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                        continue;
                    }
                    p_tmp_iobuf[iobuf_type].pa += p_reg_info->iobuf_reg[regbuf_type].mid_slice_y_ofs;
                }
			}


			for (substripe_x=0; substripe_x<substripe_cnt; ++substripe_x, ++idx) {

				KDRV_AI_JMISP_PATH_INFO* p_drv = &p_drv_array[idx];
				KDRV_AI_JMISP_FRAME_INFO* p_frame_info;
				KDRV_AI_JMISP_PINGPONG_INFO* p_pingpong_info;
				JMISP_REG_IOBUF* p_iobuf_reg;
				uint32_t substripe_case = 0, slice_case = 0;
				if (idx >= cnt) {
					break;
				}

				p_drv->handshake = KDRV_AI_JMISP_HANDSHAKE_CPU;
				p_drv->signal_mode_en = false;
				//p_drv->path_id;

				p_drv->stripe_num = 1;
				p_drv->slice_num = 1;
				p_drv->blk_num_info.blk_x_first_num = sub_blk_x_cnt_array[substripe_x];
				p_drv->blk_num_info.blk_x_middle_num = 1;
				p_drv->blk_num_info.blk_x_last_num = 1;
				p_drv->blk_num_info.blk_y_first_num = blk_y_cnt;
				p_drv->blk_num_info.blk_y_middle_num = 1;
				p_drv->blk_num_info.blk_y_last_num = 1;

				//init addr
				memset(p_drv->frame, 0, sizeof(KDRV_AI_JMISP_FRAME_INFO) * KDRV_AI_JMISP_MAX_FRAME_NUM);
				memset(p_drv->pingpong, 0, sizeof(KDRV_AI_JMISP_PINGPONG_INFO) * KDRV_AI_JMISP_MAX_PINGPONG_NUM);
				memset(p_drv->ppu_pipebuf_lsbaddr, 0, sizeof(p_drv->ppu_pipebuf_lsbaddr));

                for (j=0; j<e_AI_IOBUF_NUM; ++j) {
                    const e_AI_JMISP_IOBUF_CONST iobuf_type = (e_AI_JMISP_REG_IO_CONST)j;
                    const e_AI_JMISP_REG_IO_CONST regbuf_type = convert_iobuf2regbuf_type(&p_tmp_iobuf[iobuf_type], iobuf_type);
                    const e_AI_JMISP_DRV_IO_CONST drvio_type = convert_regbuf2drvio_type(regbuf_type);
                    if ((!is_jmisp_sup_buf(&p_tmp_iobuf[iobuf_type], iobuf_type)) || (p_tmp_iobuf[iobuf_type].buffer_sz <= 0)) {
                        continue;
                    }

                    p_iobuf_reg = &p_reg_info->iobuf_reg[regbuf_type];
                    if ((regbuf_type >= e_AI_JMISP_REG_IO_PINGPONG_BUF_BEGIN) && (regbuf_type < e_AI_JMISP_REG_IO_PINGPONG_BUF_END)) {
                        p_pingpong_info = &p_drv->pingpong[drvio_type];
                        ispframemode_cal_substripe_pingpong(p_iobuf_reg, p_pingpong_info, sub_blk_x_cnt_array, p_tmp_iobuf[iobuf_type].pa, substripe_x, slice_y);
                    } else {
                        p_frame_info = &p_drv->frame[drvio_type];
                        ispframemode_cal_substripe_frame(p_iobuf_reg, p_frame_info, sub_blk_x_cnt_array, p_tmp_iobuf[iobuf_type].pa, substripe_x, slice_y);
                    }
                }

				if (substripe_x==0) {
					substripe_case = 0;	//left
				} else if (substripe_x == (substripe_cnt-1)) {
					substripe_case = 2;	//right
				} else {
					substripe_case = 1;	//mid
				}

				if (slice_y==0) {
					slice_case = 0;	//top
				} else if (slice_y == (p_reg_info->slice_y_num-1)) {
					slice_case = 2;	//bottom
				} else {
					slice_case = 1;	//mid
				}

				//0x2C0: record block case
				p_drv->cust[0] = (substripe_case << 24) + (slice_case << 28);
				//0x2C4: record index
				p_drv->cust[1] = ((slice_y & 0xff) << 24) + ((stripe_x & 0xff) << 16) + ((0 & 0xff) << 8) + ((substripe_x & 0xff));
			}

		}


		if (sub_blk_x_cnt_array != NULL) {
			nvt_ai_mem_free(sub_blk_x_cnt_array);
			//sub_blk_x_cnt_array = 0;
		}
	}

#if AIISP_FRAME_MODE_DEBUG
DBG_DUMP("=========== ispframemode_create_substripe_array debug start ============  \n" );
{
	DBG_DUMP("signal_mode=%d; stripe_x_num=%d; slice_y_num=%d;\n", p_reg_info->signal_mode, p_reg_info->stripe_x_num, p_reg_info->slice_y_num);
	DBG_DUMP("first_stripe_block_x_num=%d; mid_stripe_block_x_num=%d; last_stripe_block_x_num=%d;\n", p_reg_info->first_stripe_block_x_num, p_reg_info->mid_stripe_block_x_num, p_reg_info->last_stripe_block_x_num);
	DBG_DUMP("first_slice_block_y_num=%d; mid_slice_block_y_num=%d; last_slice_block_y_num=%d;\n", p_reg_info->first_slice_block_y_num, p_reg_info->mid_slice_block_y_num, p_reg_info->last_slice_block_y_num);
	for (idx=0; idx<e_AI_JMISP_REG_IO_NUM; ++idx) {
		JMISP_REG_IOBUF* p_iobuf = &p_reg_info->iobuf_reg[idx];
		DBG_DUMP("[%d] is_from_ringbuf=%d; block_lofs=%d;\n", idx, p_iobuf->is_from_ringbuf, p_iobuf->block_lofs);
		DBG_DUMP("---- first_stripe_x_ofs=%d; mid_stripe_x_ofs=%d;\n", p_iobuf->first_stripe_x_ofs, p_iobuf->mid_stripe_x_ofs);
		DBG_DUMP("---- first_slice_y_ofs=%d; mid_slice_y_ofs=%d;\n", p_iobuf->first_slice_y_ofs, p_iobuf->mid_slice_y_ofs);
		DBG_DUMP("---- first_block_x_ofs=%d; mid_block_x_ofs=%d;\n", p_iobuf->first_block_x_ofs, p_iobuf->mid_block_x_ofs);
		DBG_DUMP("---- first_block_y_ofs=%d; mid_block_y_ofs=%d;\n", p_iobuf->first_block_y_ofs, p_iobuf->mid_block_y_ofs);

		DBG_DUMP("---- first_stripe_w=%d; mid_stripe_w=%d;last_stripe_w=%d\n", p_iobuf->first_stripe_w, p_iobuf->mid_stripe_w, p_iobuf->last_stripe_w);
		DBG_DUMP("---- first_slice_h=%d; mid_slice_h=%d;last_slice_h=%d\n", p_iobuf->first_slice_h, p_iobuf->mid_slice_h, p_iobuf->last_slice_h);
	}
}
{
	idx = 0;
	for (stripe_x=0; stripe_x<p_reg_info->stripe_x_num; ++stripe_x) {
		uint32_t blk_x_cnt, substripe_cnt;
		if (stripe_x == 0) {
			blk_x_cnt = p_reg_info->first_stripe_block_x_num;
		} else if (stripe_x == (p_reg_info->stripe_x_num - 1)) {
			blk_x_cnt = p_reg_info->last_stripe_block_x_num;
		} else {
			blk_x_cnt = p_reg_info->mid_stripe_block_x_num;
		}
		substripe_cnt = min(blk_x_cnt, core_num);
		DBG_DUMP("substripe_cnt = %d\n", substripe_cnt);

		for (slice_y=0; slice_y<p_reg_info->slice_y_num; ++slice_y) {

			for (substripe_x=0; substripe_x<substripe_cnt; ++substripe_x, ++idx) {
				uint32_t i=0;
				KDRV_AI_JMISP_PATH_INFO* p_drv = &p_drv_array[idx];

				if (substripe_x > (substripe_cnt - 1)) {
					continue;
				}

				DBG_DUMP("--- stripe = %d; slice = %d; substripe_x = %d; idx = %d; ---\n", (int)stripe_x, (int)slice_y, (int)substripe_x, (int)idx);
				for (i=0; i<KDRV_AI_JMISP_MAX_PINGPONG_NUM; ++i) {
					DBG_DUMP("p_drv->pingpong[%d].pingpong_addr0  = 0x%08lx \n",  (int)i, (ULONG)p_drv->pingpong[i].pingpong_addr0 );
					DBG_DUMP("p_drv->pingpong[%d].pingpong_addr1  = 0x%08lx\n",   (int)i, (ULONG)p_drv->pingpong[i].pingpong_addr1);
				}
				for (i=0; i<KDRV_AI_JMISP_MAX_FRAME_NUM; ++i) {
					DBG_DUMP("p_drv->frame[%d].ring_start_addr  = 0x%08lx \n",  (int)i, (ULONG)p_drv->frame[i].ring_start_addr );
					DBG_DUMP("p_drv->frame[%d].ring_end_addr 0x%08lx\n",        (int)i, (ULONG)p_drv->frame[i].ring_end_addr);
					DBG_DUMP("p_drv->frame[%d].ring_en   = %u \n",          (int)i,        p_drv->frame[i].ring_en );
					DBG_DUMP("p_drv->frame[%d].frame_addr  = 0x%08lx \n",       (int)i, (ULONG)p_drv->frame[i].frame_addr);
					DBG_DUMP("p_drv->frame[%d].blk_ofs_info first_x_ofs = %u, mid_x_ofs = %u \n",       (int)i, p_drv->frame[i].blk_ofs_info.blk_x_first_ofs, p_drv->frame[i].blk_ofs_info.blk_x_middle_ofs);
					DBG_DUMP("p_drv->frame[%d].blk_ofs_info first_y_ofs = %u, mid_y_ofs = %u \n",       (int)i, p_drv->frame[i].blk_ofs_info.blk_y_first_ofs, p_drv->frame[i].blk_ofs_info.blk_y_middle_ofs);
				}
				DBG_DUMP( "p_drv->path_id = %d \n",       (int)p_drv->path_id);
				DBG_DUMP( "p_drv->handshake = %d \n", (int) p_drv->handshake);

				DBG_DUMP( "p_drv->signal_mode_en = %d \n",       (int)p_drv->signal_mode_en);
				DBG_DUMP( "p_drv->stripe_num = %d; p_drv->slice_num = %d;\n", (int) p_drv->stripe_num, (int) p_drv->slice_num);
				DBG_DUMP( "p_drv->blk_num_info: x_first_num = %d; x_middle_num = %d; x_last_num = %d;\n", (int) p_drv->blk_num_info.blk_x_first_num, (int) p_drv->blk_num_info.blk_x_middle_num, (int) p_drv->blk_num_info.blk_x_last_num);
				DBG_DUMP( "p_drv->blk_num_info: y_first_num = %d; y_middle_num = %d; y_last_num = %d;\n", (int) p_drv->blk_num_info.blk_y_first_num, (int) p_drv->blk_num_info.blk_y_middle_num, (int) p_drv->blk_num_info.blk_y_last_num);
				for (i=0; i<KDRV_AI_JMISP_MAX_CUST_NUM; ++i) {
					DBG_DUMP("p_drv-> cust[%d] = 0x%08lx;\n", (int)i, (ULONG)p_drv->cust[i]);
				}
				for (i=0; i<KDRV_AI_JMISP_MAX_PIPEBUFF_NUM; ++i) {
					DBG_DUMP("p_drv-> pipebuf[%d] = 0x%08lx;\n", (int)i, (ULONG)p_drv->ppu_pipebuf_lsbaddr[i]);
				}
			}

		}
	}
}
DBG_DUMP("=========== ispframemode_create_substripe_array debug end ============  \n" );
#endif

	err = E_OK;


exit:
    if (p_reg_info != NULL) {
        nvt_ai_mem_free(p_reg_info);
        //p_reg_info = 0;
    }
	if (p_tmp_iobuf != NULL) {
		nvt_ai_mem_free(p_tmp_iobuf);
		//p_tmp_iobuf = 0;
	}
    return err;
}


ER ispframemode_upd_substripe_pingpong_addr(VOID *p_reg_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
											, const uint64_t pp_addr0, const uint64_t pp_addr1, const uint32_t core_num) {
	ispframemode_upd_pingpong_addr(p_reg_info, p_drv_array,cnt, type, pp_addr0, pp_addr1, 1, core_num);
	return E_OK;
}


ER ispframemode_upd_substripe_ringbuf_addr(VOID *p_reg_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
											, bool ring_en, const uint64_t ring_saddr, const uint64_t ring_eaddr, const uint32_t core_num) {
	ispframemode_upd_ringbuf_addr(p_reg_info, p_drv_array, cnt, type, ring_en, ring_saddr, ring_eaddr, 1, core_num);
	return E_OK;
}


//for frame mode
uintptr_t ispframemode_get_jmisp_null_pa_pos(uintptr_t jmisp_addr, uintptr_t parm_va_ofs, uintptr_t parm_pa_ofs, UINT32 size) {
	UINT32 i = 0;
    UINT64 *temp_ptr = 0;
	uintptr_t null_pa_addr = jmisp_addr - parm_va_ofs + parm_pa_ofs;

	if ((jmisp_addr == 0) || (size == 0) || (parm_va_ofs == 0) || (parm_pa_ofs == 0)) {
		return E_NOEXS;
	}

    while(i < size) {
		temp_ptr = (UINT64 *)(jmisp_addr + i);
        if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 0) {
			null_pa_addr = (uintptr_t)temp_ptr - parm_va_ofs + parm_pa_ofs;
			i += 4;	
			break;	//return E_OK;

        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 1) {
            i += 8;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 2) {
            i += 8;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 4) {
            i += 12;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 5) {
            i += 4;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 6) {
            i += 12;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 7) {
            i += 4;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 8) {
            i += 12;
        } else if (NN_JOBLL_CMD_GET_MODE(*temp_ptr) == 15) {
            i += 8;
        } else {
			DBG_ERR("unknown job list cmd mode : %d\r\n", (int)NN_JOBLL_CMD_GET_MODE(*temp_ptr));
		}
    }

	return null_pa_addr;
}

uint32_t ispframemode_get_total_slice_num(VOID* p_info) {
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t slice_num = 0;
	if (p_reg_info == NULL) {
		return 0;
	}
	slice_num = p_reg_info->stripe_x_num * p_reg_info->slice_y_num;
	return slice_num;
}


ER ispframemode_upd_pingpong_addr(VOID *p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
									, const uint64_t pp_addr0, const uint64_t pp_addr1, const bool sub_stripe_en, const uint32_t core_num) {
	
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t* p_num = NULL;
	uint32_t slice_cnt=0;
	uint32_t stripe_x = 0, slice_y = 0;
	uint32_t slice_idx=0, idx=0;
	const int32_t pingpong_buf_sz = pp_addr1 - pp_addr0;
	ER status = E_OK;

	if ((p_reg_info==NULL) || (p_drv_array==NULL) || (cnt==0) || (pp_addr0 == 0) || (pp_addr1 == 0) || (pingpong_buf_sz <= 0)) {
		status = E_NOEXS;
		goto exit;
	} else if ((type < e_AI_JMISP_DRV_IO_PINGPONG_BUF0) || (type > e_AI_JMISP_DRV_IO_PINGPONG_BUF2)) {
		DBG_ERR("not support type (%d)\r\n", type);
		status = E_PAR;
		goto exit;
	}

	slice_cnt = ispframemode_get_total_slice_num(p_info);
	p_num = nvt_ai_mem_alloc(sizeof(uint32_t) * slice_cnt);

	if (p_num == NULL) {
		DBG_ERR("p_num alloc fail\r\n");
		status = E_NOMEM;
		goto exit;
	}
	if (sub_stripe_en) {
		status = ispframemode_create_substripe_num_array(p_info, p_num, slice_cnt, core_num);
	} else {
		status = ispframemode_create_blknum_array(p_info, p_num, slice_cnt);
	}
	if (status < 0) {
		DBG_ERR("array alloc fail\r\n");
		goto exit;
	}

	slice_idx=0;
	idx=0;
	for (stripe_x=0; stripe_x<p_reg_info->stripe_x_num; ++stripe_x) {
		for (slice_y=0; slice_y<p_reg_info->slice_y_num; ++slice_y, ++slice_idx) {
			uint32_t i=0;
			for (i=0; i<p_num[slice_idx]; ++i, ++idx) {
				KDRV_AI_JMISP_PINGPONG_INFO* p_info = &(p_drv_array[idx].pingpong[type]);
				uint64_t pingpong_s_addr, pingpong_e_addr;

				pingpong_s_addr = (slice_y & 0x1) ? pp_addr1 : pp_addr0;
				pingpong_e_addr = pingpong_s_addr + pingpong_buf_sz;
				while (p_info->pingpong_addr0 >= pingpong_e_addr) {
					p_info->pingpong_addr0 += pingpong_s_addr;
					p_info->pingpong_addr0 -= pingpong_e_addr;
				}
				p_info->pingpong_addr1 = p_info->pingpong_addr0;

			}
		}
	}

#if AIISP_FRAME_MODE_DEBUG
DBG_DUMP("=========== ispframemode_upd_pingpong_addr debug start ============  \n" );
{
    slice_idx=0;
    idx=0;
    for (stripe_x=0; stripe_x<p_reg_info->stripe_x_num; ++stripe_x) {
        for (slice_y=0; slice_y<p_reg_info->slice_y_num; ++slice_y, ++slice_idx) {
            uint32_t i=0;
            for (i=0; i<p_num[slice_idx]; ++i, ++idx) {
                KDRV_AI_JMISP_PINGPONG_INFO* p_info = &(p_drv_array[idx].pingpong[type]);
                DBG_DUMP("stripe x: %u; slice y: %u; idx: %u\n", stripe_x, slice_y, i);
                DBG_DUMP("pingpong0=0x%016llx; pingpong1=0x%016llx;\n", p_info->pingpong_addr0, p_info->pingpong_addr1);
            }
        }
    }
}
DBG_DUMP("=========== ispframemode_upd_pingpong_addr debug end ============  \n" );
#endif

exit:
	if (p_num != NULL) {
		nvt_ai_mem_free(p_num);
		//p_num = 0;
	}

	return status;
}

ER ispframemode_upd_ringbuf_addr(VOID *p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, e_AI_JMISP_DRV_IO_CONST type
								, bool ring_en, const uint64_t ring_saddr, const uint64_t ring_eaddr, const bool sub_stripe_en, const uint32_t core_num) {
	
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t* p_num = NULL;
	uint32_t slice_cnt=0;
	uint32_t stripe_x = 0, slice_y = 0;
	uint32_t slice_idx=0, idx=0;
	ER status = E_OK;

	if ((p_reg_info==NULL) || (p_drv_array==NULL) || (cnt==0) || (ring_saddr >= ring_eaddr)) {
		status = E_NOEXS;
		goto exit;
	} else if ((type < e_AI_JMISP_DRV_IO_RING_BUF0) || (type > e_AI_JMISP_DRV_IO_FRAME_BUF3)) {
		DBG_ERR("not support type (%d)\r\n", type);
		status = E_PAR;
		goto exit;
	}

	slice_cnt = ispframemode_get_total_slice_num(p_info);
	p_num = nvt_ai_mem_alloc(sizeof(uint32_t) * slice_cnt);
	if (p_num == NULL) {
		DBG_ERR("p_num alloc fail\r\n");
		status = E_NOMEM;
		goto exit;
	}
	if (sub_stripe_en) {
		status = ispframemode_create_substripe_num_array(p_info, p_num, slice_cnt, core_num);
	} else {
		status = ispframemode_create_blknum_array(p_info, p_num, slice_cnt);
	}
	if (status < 0) {
		DBG_ERR("array alloc fail\r\n");
		goto exit;
	}
	
	slice_idx=0;
	idx=0;
	for (stripe_x=0; stripe_x<p_reg_info->stripe_x_num; ++stripe_x) {
		for (slice_y=0; slice_y<p_reg_info->slice_y_num; ++slice_y, ++slice_idx) {
			uint32_t i=0;
			for (i=0; i<p_num[slice_idx]; ++i, ++idx) {
				KDRV_AI_JMISP_FRAME_INFO* p_info = &p_drv_array[idx].frame[type];
				p_info->ring_en = ring_en;
				if (!ring_en) {
					continue;
				}
				p_info->ring_start_addr = ring_saddr;
				p_info->ring_end_addr = ring_eaddr;
				while (p_info->frame_addr >= ring_eaddr) {
					p_info->frame_addr += ring_saddr;
					p_info->frame_addr -= ring_eaddr;
				}
				if (p_info->frame_addr < ring_saddr) {
					DBG_ERR("udpate frame addrss < ring start address\r\n");
				}
			}
		}
	}
#if AIISP_FRAME_MODE_DEBUG
DBG_DUMP("=========== ispframemode_upd_ringbuf_addr debug start ============  \n" );
{
    slice_idx=0;
    idx=0;
    for (stripe_x=0; stripe_x<p_reg_info->stripe_x_num; ++stripe_x) {
        for (slice_y=0; slice_y<p_reg_info->slice_y_num; ++slice_y, ++slice_idx) {
            uint32_t i=0;
            for (i=0; i<p_num[slice_idx]; ++i, ++idx) {
                KDRV_AI_JMISP_FRAME_INFO* p_info = &p_drv_array[idx].frame[type];

                DBG_DUMP("stripe x: %u; slice y: %u; idx: %u\n", stripe_x, slice_y, i);
                DBG_DUMP("ring[0x%016llx, 0x%016llx]; frame addr=0x%016llx\n", p_info->ring_start_addr, p_info->ring_end_addr, p_info->frame_addr);
            }
        }
    }
}
DBG_DUMP("=========== ispframemode_upd_ringbuf_addr debug end ============  \n" );
#endif

exit:
	if (p_num != NULL) {
		nvt_ai_mem_free(p_num);
		//p_num = 0;
	}

	return status;
}

ER ispframemode_upd_substripe_ppupipebuf_addr(VOID *p_info, KDRV_AI_JMISP_PATH_INFO* p_drv_array, const uint32_t cnt, const uint32_t core_num
									, const uint64_t ppubuf_addr0, const uint64_t ppubuf_addr1, const uint32_t ppubuf_sz) {
	JMISP_REG_INFO* p_reg_info = p_info;
	uint32_t* p_substripe_num = NULL;
	uint32_t slice_cnt, slice_idx, substripe_idx;
	uint32_t each_substripe_ppubuf_sz;
	ER status = E_OK;

	if ((p_reg_info==NULL) || (p_drv_array==NULL) || (cnt==0) || (core_num==0) || (ppubuf_addr0==0) || (ppubuf_addr1==0) || (ppubuf_sz==0)) {
		DBG_ERR("null args\r\n");
		status = E_NOEXS;
		goto exit;
	}
    each_substripe_ppubuf_sz = ppubuf_sz / core_num;

	slice_cnt = ispframemode_get_total_slice_num(p_info);
	p_substripe_num = nvt_ai_mem_alloc(sizeof(uint32_t) * slice_cnt);
	if (p_substripe_num == NULL) {
		DBG_ERR("p_substripe_num alloc fail\r\n");
		status = E_NOMEM;
		goto exit;
	}
	status = ispframemode_create_substripe_num_array(p_info, p_substripe_num, slice_cnt, core_num);
	if (status < 0) {
		DBG_ERR("array alloc fail\r\n");
		goto exit;
	}

    substripe_idx=0;
	for (slice_idx=0; slice_idx<slice_cnt; ++slice_idx) {
		uint32_t i=0;
		uint64_t addr_ofs=0;
		for (i=0; i<p_substripe_num[slice_idx]; ++i, ++substripe_idx) {
			p_drv_array[substripe_idx].ppu_pipebuf_lsbaddr[e_AI_JMISP_DRV_IO_PIPE_BUF0] = ppubuf_addr0 + addr_ofs;
			p_drv_array[substripe_idx].ppu_pipebuf_lsbaddr[e_AI_JMISP_DRV_IO_PIPE_BUF1] = ppubuf_addr1 + addr_ofs;
			addr_ofs += each_substripe_ppubuf_sz;
		}
	}

#if AIISP_FRAME_MODE_DEBUG
DBG_DUMP("=========== ispframemode_upd_substripe_ppupipebuf_addr debug start ============  \n" );
{
    substripe_idx=0;
    for (slice_idx=0; slice_idx<slice_cnt; ++slice_idx) {
        uint32_t i=0;
        for (i=0; i<p_substripe_num[slice_idx]; ++i, ++substripe_idx) {
            DBG_DUMP("slice idx: %u; substripe: %u;\n", slice_idx, i);
            DBG_DUMP("pipe buf0: 0x%08x\n", p_drv_array[substripe_idx].ppu_pipebuf_lsbaddr[e_AI_JMISP_DRV_IO_PIPE_BUF0]);
            DBG_DUMP("pipe buf1: 0x%08x\n", p_drv_array[substripe_idx].ppu_pipebuf_lsbaddr[e_AI_JMISP_DRV_IO_PIPE_BUF1]);
        }
    }
}
DBG_DUMP("=========== ispframemode_upd_substripe_ppupipebuf_addr debug end ============  \n" );
#endif

exit:
	if (p_substripe_num != NULL) {
		nvt_ai_mem_free(p_substripe_num);
		//p_substripe_num = 0;
	}

	return status;
}