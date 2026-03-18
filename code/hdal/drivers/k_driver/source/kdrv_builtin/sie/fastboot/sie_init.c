/**
    SIE module fast boot driver

    @file       sie_init.c
    @ingroup    mIIPPSIE

    Copyright   Novatek Microelectronics Corp. 2010.  All rights reserved.
*/
// include files for FW
#include "kwrap/util.h"
#include "kwrap/type.h"
#include <kdrv_builtin.h>
#include "sie_init.h"
#include "nvtmpp_init.h"
#include "sie_eng_base.h"
#include "sie_init_int.h"
#include "bridge.h"

#define SIE_FB_RING_BUF_NUM     2           //fastboot sie ouptut ring buffer number

//bridge info
#define SIE_SKIP_ID_BIT      0x42495353 //MAKEFOURCC('S', 'S', 'I', 'B');	frame count
#define SIE_FC_ID_1      	 0x31494653 //MAKEFOURCC('S', 'F', 'I', '1');
#define SIE_FC_ID_2      	 0x32494653 //MAKEFOURCC('S', 'F', 'I', '2');
#define SIE_FC_ID_3      	 0x33494653 //MAKEFOURCC('S', 'F', 'I', '3');
#define SIE_FC_ID_4      	 0x34494653 //MAKEFOURCC('S', 'F', 'I', '4');
#define SIE_FC_ID_5      	 0x35494653 //MAKEFOURCC('S', 'F', 'I', '5');

#define SIE_TS_ID_1          0x31495453 //MAKEFOURCC('S', 'T', 'I', '1');	timestamp
#define SIE_TS_ID_2          0x32495453 //MAKEFOURCC('S', 'T', 'I', '2');
#define SIE_TS_ID_3          0x33495453 //MAKEFOURCC('S', 'T', 'I', '3');
#define SIE_TS_ID_4          0x34495453 //MAKEFOURCC('S', 'T', 'I', '4');
#define SIE_TS_ID_5          0x35495453 //MAKEFOURCC('S', 'T', 'I', '5');

#define SIE_ID_1_ADDR_0      0x30413153 //MAKEFOURCC('S', '1', 'A', '0'); SIE1 out buf 0 LSB
#define SIE_ID_1_ADDR_1      0x31413153 //MAKEFOURCC('S', '1', 'A', '1'); SIE1 out buf 1 LSB
#define SIE_ID_1_ADDR_2      0x32413153 //MAKEFOURCC('S', '1', 'A', '2'); SIE1 out buf 0 MSB
#define SIE_ID_1_ADDR_3      0x33413153 //MAKEFOURCC('S', '1', 'A', '3'); SIE1 out buf 1 MSB

#define SIE_ID_2_ADDR_0      0x30413253 //MAKEFOURCC('S', '2', 'A', '0'); SIE2 out buf 0 LSB
#define SIE_ID_2_ADDR_1      0x31413253 //MAKEFOURCC('S', '2', 'A', '1'); SIE2 out buf 1 LSB
#define SIE_ID_2_ADDR_2      0x32413253 //MAKEFOURCC('S', '2', 'A', '2'); SIE2 out buf 0 MSB
#define SIE_ID_2_ADDR_3      0x33413253 //MAKEFOURCC('S', '2', 'A', '3'); SIE2 out buf 1 MSB

#define SIE_ID_3_ADDR_0      0x30413353 //MAKEFOURCC('S', '3', 'A', '0'); SIE3 out buf 0 LSB
#define SIE_ID_3_ADDR_1      0x31413353 //MAKEFOURCC('S', '3', 'A', '1'); SIE3 out buf 1 LSB
#define SIE_ID_3_ADDR_2      0x32413353 //MAKEFOURCC('S', '3', 'A', '2'); SIE3 out buf 0 MSB
#define SIE_ID_3_ADDR_3      0x33413353 //MAKEFOURCC('S', '3', 'A', '3'); SIE3 out buf 1 MSB

#define SIE_ID_4_ADDR_0      0x30413453 //MAKEFOURCC('S', '4', 'A', '0'); SIE4 out buf 0 LSB
#define SIE_ID_4_ADDR_1      0x31413453 //MAKEFOURCC('S', '4', 'A', '1'); SIE4 out buf 1 LSB
#define SIE_ID_4_ADDR_2      0x32413453 //MAKEFOURCC('S', '4', 'A', '2'); SIE4 out buf 0 MSB
#define SIE_ID_4_ADDR_3      0x33413453 //MAKEFOURCC('S', '4', 'A', '3'); SIE4 out buf 1 MSB

#define SIE_ID_5_ADDR_0      0x30413553 //MAKEFOURCC('S', '5', 'A', '0'); SIE5 out buf 0 LSB
#define SIE_ID_5_ADDR_1      0x31413553 //MAKEFOURCC('S', '5', 'A', '1'); SIE5 out buf 1 LSB
#define SIE_ID_5_ADDR_2      0x32413553 //MAKEFOURCC('S', '5', 'A', '2'); SIE5 out buf 0 MSB
#define SIE_ID_5_ADDR_3      0x33413553 //MAKEFOURCC('S', '5', 'A', '3'); SIE5 out buf 1 MSB

//offset 0x0 ctrl
#define SIE_FB_CTL_OFS          0x0         //ctl reg ofs
#define SIE_FB_CTL_RST_BIT      0x01        //reset bit
#define SIE_FB_CTL_LOAD_BIT     0x02        //load bit
#define SIE_FB_CTL_ACT_EN_BIT   0x04        //active enable bit

//offset 0x4    function
#define SIE_FB_FUNC_OFS         	0x4         //function reg ofs
#define SIE_FB_FUNC_IQ_OFF_BIT 		0x08808		//function disable bit(OB_AVG_EN bit3, DPC bit11, ECS bit15)
#define SIE_FB_FUNC_DVI_EN_BIT 		0x04      	//DVI function enable bit
#define SIE_FB_FUNC_RAWENC_EN_BIT 	0x80000		//Raw Encode function enable bit

//offset 0x8
#define SIE_FB_INTE_OFS     0x08            //interrupt enable reg ofs
#define SIE_FB_INTE_DFT_BIT SIE_INT_VD|SIE_INT_CROPEND|SIE_INT_DRAM_OUT0_END|SIE_INT_DRAM_OUT1_END|SIE_INT_DRAM_OUT2_END  //interrupt enable bit, VD and Crop End, OUT1_END, OUT2_END

//offset 0xc
#define SIE_FB_INTS_OFS     0x0c            //interrupt status reg ofs

#define SIE_SIM_BUF_CB 0

static  VK_DEFINE_SPINLOCK(my_lock);
#define sie_header_loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define sie_header_unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)

static volatile SIE_FB_BUF_ADDR_INFO sie_fb_rdy_addr[SIE_MAX_ENG_NUM][SIE_FB_OUT_CH_MAX][SIE_FB_BUF_IDX_MAX] = {0};
static volatile SIE_FB_BUF_ADDR_INFO sie_fb_ppb[SIE_MAX_ENG_NUM][SIE_FB_OUT_CH_MAX][SIE_FB_RING_BUF_NUM] = {0};

volatile BOOL sie_fb_first_vd[SIE_MAX_ENG_NUM] = {0};					//for isr process
volatile BOOL sie_fb_vd[SIE_MAX_ENG_NUM] = {0};							//for wait vd api using
volatile BOOL ctl_sie_fb_de_flg[SIE_MAX_ENG_NUM] = {TRUE, TRUE, TRUE};
static VOS_TICK sie_fb_vd_ts[SIE_MAX_ENG_NUM] = {0};
static SIE_BUILTIN_IOSIZE sie_fb_iosize[SIE_MAX_ENG_NUM] = {0};
static SIE_BUILTIN_IOSIZE sie_fb_rt_chg_size[SIE_MAX_ENG_NUM] = {0};	//sie run time change act/crp win by user
SIE_BUILTIN_INFO sie_builtin_info[SIE_MAX_ENG_NUM] = {0};
SIE_FB_BRIDGE_INFO bridge_info[SIE_MAX_ENG_NUM] = {0};
UINT32 streaming_id_bit = 0;

//debug
UINT32 sie_fb_dbg_dump_max_cnt = 5;
UINT32 sie_fb_dbg_dump_cnt[SIE_MAX_ENG_NUM] = {0};
UINT32 sie_fb_dbg_push_cnt[SIE_MAX_ENG_NUM] = {2, 2, 2};
SIE_FB_DBG_LVL sie_fb_dbg_lvl = SIE_FB_DBG_LVL_WRN;	//read from fastboot dtsi
SIE_BUILTIN_DBG_INFO sie_builtin_dbg_info = {0};

typedef struct {
	ULONG addr;
	UINT32 size;
} SIE_BUILTIN_BLK;

static void (*sie_fb_int_cb)(UINT32 id, UINT32 status);
static void (*sie_fb_buf_out_cb)(UINT32 id, SIE_BUILTIN_HEADER_INFO *info);
typedef void (*SIE_FB_INT_ISR_CB)(void *eng, UINT32 status, void *reserve);
#define SIE_BUILTIN_NODE_HEAD "/fastboot/sie"
#define FAST_BUILTIN_NODE_HEAD "/fastboot"
UINT32 fast_builtin_gen = 0;
static int nodeoffset_list[SIE_MAX_ENG_NUM][SIE_INIT_NODE_NUM];
static int nodeoffset_list_dbg[SIE_INIT_NODE_GLB_NUM];

typedef int (*SIE_BUILTIN_SSDRV_FP_SET_BOOL)(SIE_ENG_HANDLE *, BOOL);
static const SIE_BUILTIN_SSDRV_FP_SET_BOOL sie_builtin_fp_set_singleouten[SIE_FB_OUT_CH_MAX] = {
	NULL,	//base
	sie_eng_set_dramout0_singleout_en_buf_reg,
	sie_eng_set_dramout1_singleout_en_buf_reg,
	sie_eng_set_dramout2_singleout_en_buf_reg,
};

typedef int (*SIE_BUILTIN_SSDRV_FP_SET_DRAMOUT_ADDR)(SIE_ENG_HANDLE *, SIE_DRAM_ADDR *);
static const SIE_BUILTIN_SSDRV_FP_SET_DRAMOUT_ADDR sie_builtin_fp_set_dramoutmode[SIE_FB_OUT_CH_MAX] = {
	NULL,	//base
	sie_eng_set_dramout0_addr_buf_reg,
	sie_eng_set_dramout1_addr_buf_reg,
	sie_eng_set_dramout2_addr_buf_reg,
};

#if SIE_SIM_BUF_CB
static void sie_fb_buf_out_cb_sim_common(SIE_ENGINE_ID id, SIE_BUILTIN_HEADER_INFO *info)
{
	if (info != NULL) {
		if (info->buf_ctrl == SIE_BUILTIN_HEADER_CTL_PUSH) {
			nvtmpp_unlock_fastboot_blk(info->buf_addr);
		}
	}

}
#endif

static SIE_LOAD_SRC sie_fb_conv2_sie_ssdrv_load_src(UINT32 id)
{
	if (sie_builtin_info[id].ref_load_id == id) {
		return SIE_LOAD_DISABLE;
	} else {
		switch (sie_builtin_info[id].ref_load_id) {
		case SIE_ENG_ID_1:
			return SIE_LOAD_FROM_SIE1;
		case SIE_ENG_ID_2:
			return SIE_LOAD_FROM_SIE2;
		case SIE_ENG_ID_3:
			return SIE_LOAD_FROM_SIE3;
		case SIE_ENG_ID_4:
			return SIE_LOAD_FROM_SIE4;
		case SIE_ENG_ID_5:
			return SIE_LOAD_FROM_SIE5;
		default:
			sie_fb_dbg_err("Unknown ref_load_id %d, force to SIE_LOAD_DISABLE\r\n", (int)sie_builtin_info[id].ref_load_id);
			return SIE_LOAD_DISABLE;
		}
	}
}

static void sie_fb_dump_init_info(SIE_ENG_ID_T id)
{
	SIE_ACT_WIN_PARAM act_win = {0};
	SIE_CRP_WIN_PARAM crp_win = {0};
	SIE_BS_H_PARAM bs_h = {0};
	SIE_BS_V_PARAM bs_v = {0};
	UINT32 func;

	sie_fb_dbg_ind("\r\n------------------------- sie id %d builtin init dump begin ------------------------------\r\n", id);
	sie_eng_get_act_win_buf_reg(sie_builtin_info[id].ssdrv_hdl, &act_win);
	sie_fb_dbg_ind("act_win: (%d, %d, %d, %d), cfapat: %d\r\n",
					act_win.st_x, act_win.st_y, act_win.sz_x, act_win.sz_y, act_win.cfa_pat);

	sie_eng_get_crop_win_buf_reg(sie_builtin_info[id].ssdrv_hdl, &crp_win);
	sie_fb_dbg_ind("crp_win: (%d, %d, %d, %d), cfapat: %d\r\n",
					crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y, crp_win.cfa_pat);

	sie_eng_get_bsh_buf_reg(sie_builtin_info[id].ssdrv_hdl, &bs_h);
	sie_eng_get_bsv_buf_reg(sie_builtin_info[id].ssdrv_hdl, &bs_v);
	sie_fb_dbg_ind("scl_out: (%d, %d)\r\n", bs_h.out_sz, bs_v.out_sz);

	sie_fb_dbg_ind("sen_out_dest:      0x%x\r\n", sie_builtin_info[id].sen_out_dest);
	sie_fb_dbg_ind("out_dest:          0x%x\r\n", sie_builtin_info[id].out_dest);

	sie_fb_dbg_ind("dupl_src_id:       %d\r\n", sie_builtin_info[id].dupl_src_id);
	sie_fb_dbg_ind("ref_load_id:       %d\r\n", sie_builtin_info[id].ref_load_id);
	sie_fb_dbg_ind("shdr_en:           %d\r\n", sie_builtin_info[id].shdr_en);
	sie_fb_dbg_ind("low latency:       %d\r\n", sie_builtin_info[id].low_latency_en);

	sie_fb_dbg_ind("dbg isp bypass:    %d\r\n", sie_builtin_dbg_info.isp_bypass_en);
	sie_fb_dbg_ind("dbg out two frame: %d\r\n", sie_builtin_dbg_info.out_two_frame_only);

	sie_eng_get_function_enable_buf_reg(sie_builtin_info[id].ssdrv_hdl, &func);
	sie_fb_dbg_ind("function:          0x%x\r\n", func);
	sie_fb_dbg_ind("fastbuilt_gen:     %d\r\n", fast_builtin_gen);
	sie_fb_dbg_ind("b_start:           %d\r\n", sie_builtin_info[id].b_start);

	sie_fb_dbg_ind("\r\n-------------------------- sie id %d builtin init dump end -------------------------------\r\n\r\n", id);
}

void sie_fb_upd_timestp(UINT32 id)
{
	unsigned long flags;

	sie_header_loc_cpu(flags);
	vos_perf_mark(&sie_fb_vd_ts[id]);
	sie_header_unl_cpu(flags);
}

ER sie_fb_set_ppb(SIE_ENG_ID_T id, SIE_BUILTIN_INIT_INFO *init_info)
{
	ER rt = E_OK;
	ULONG vcap_out_addr1 = 0, vcap_out_addr2 = 0;
	UINT32 ch0_size = 0, ch1_size = 0, ch2_size = 0, total_buf_size = 0;
	UINT32 ring_buf_sz = 0, ring_buf_en = 0, out_dest = 0;

	if (sie_init_plat_read_dtsi_array(nodeoffset_list[id][SIE_INIT_NODE_CTRL], "out0_sz", &ch0_size, 1) != E_OK) {
		DBG_ERR("Failed to get out_0 size %d\r\n", ch0_size);
		return E_SYS;
	}

	if (sie_init_plat_read_dtsi_array(nodeoffset_list[id][SIE_INIT_NODE_CTRL], "out1_sz", &ch1_size, 1) != E_OK) {
		DBG_ERR("Failed to get out_1 size %d\r\n", ch1_size);
		return E_SYS;
	}

	if (sie_init_plat_read_dtsi_array(nodeoffset_list[id][SIE_INIT_NODE_CTRL], "out2_sz", &ch2_size, 1) != E_OK) {
		DBG_ERR("Failed to get out_2 size\r\n");
		return E_SYS;
	}

	if (sie_init_plat_read_dtsi_array(nodeoffset_list[id][SIE_INIT_NODE_CTRL], "out_dest", &out_dest, 1) != E_OK) {
		DBG_ERR("Failed to get out dest\r\n");
		return E_SYS;
	} else {
		sie_builtin_info[id].out_dest = out_dest;
	}

	if (sie_init_plat_read_dtsi_array(nodeoffset_list[id][SIE_INIT_NODE_CTRL], "ring_buf_en", &ring_buf_en, 1) != E_OK) {
		DBG_ERR("Failed to get ring buf en\r\n");
		return E_SYS;
	}

	if (sie_init_plat_read_dtsi_array(nodeoffset_list[id][SIE_INIT_NODE_CTRL], "ring_buf_sz", &ring_buf_sz, 1) != E_OK) {
		DBG_ERR("Failed to get ring buf size\r\n");
		return E_SYS;
	}

	total_buf_size = ch0_size + ch1_size + ch2_size;
	if (total_buf_size) {

		if ((1<<id) & streaming_id_bit) {
			vcap_out_addr1 = nvtmpp_get_fastboot_blk(total_buf_size);

			if (sie_builtin_info[id].one_buf_en) {	//one buffer mode
				vcap_out_addr2 = vcap_out_addr1;	//set addr2 = addr and lock buf again
				nvtmpp_lock_fastboot_blk(vcap_out_addr1);	//lock buffer for addr2 buffer release
			} else {
				vcap_out_addr2 = nvtmpp_get_fastboot_blk(total_buf_size);
			}
			sie_builtin_info[id].bridge_info.buf_addr_0 = vcap_out_addr1;
			sie_builtin_info[id].bridge_info.buf_addr_1 = vcap_out_addr2;
			sie_fb_dbg_ind("get comm buf(size 0x%x) 0x%lx, 0x%lx\r\n", total_buf_size, vcap_out_addr1, vcap_out_addr2);
		} else {
			//sie out address from rtos bridge info is phy addr
			vcap_out_addr1 = sie_builtin_info[id].bridge_info.buf_addr_0;
			if (vcap_out_addr1 != 0) {
				sie_fb_dbg_ind("lock comm buf pa 0x%lx\r\n", vcap_out_addr1);
				vcap_out_addr1 = nvtmpp_buitin_sys_pa2va(vcap_out_addr1);
				nvtmpp_lock_fastboot_blk(vcap_out_addr1);
			} else {
				sie_fb_dbg_wrn("get bridge buf1 0\r\n");
				vcap_out_addr1 = nvtmpp_get_fastboot_blk(total_buf_size);
			}

			vcap_out_addr2 = sie_builtin_info[id].bridge_info.buf_addr_1;
			if (vcap_out_addr2 != 0) {
				sie_fb_dbg_ind("lock comm buf pa 0x%lx\r\n", vcap_out_addr2);
				vcap_out_addr2 = nvtmpp_buitin_sys_pa2va(vcap_out_addr2);
				nvtmpp_lock_fastboot_blk(vcap_out_addr2);
			} else {
				sie_fb_dbg_wrn("get bridge buf2 0\r\n");
				if (sie_builtin_info[id].one_buf_en) {
					vcap_out_addr2 = vcap_out_addr1;
					nvtmpp_lock_fastboot_blk(vcap_out_addr2);
				} else {
					vcap_out_addr2 = nvtmpp_get_fastboot_blk(total_buf_size);
				}
			}
		}
		if ((vcap_out_addr1 == 0) || (vcap_out_addr2 == 0)) {
			DBG_ERR("id=%d,alloc buf sz 0x%x fail (0x%lx,0x%lx)\r\n", id, total_buf_size, vcap_out_addr1, vcap_out_addr2);
			return E_NOMEM;
		} else {
			sie_fb_dbg_ind("id=%d,alloc buf sz 0x%x ok (0x%lx,0x%lx)\r\n", id, total_buf_size, vcap_out_addr1, vcap_out_addr2);
		}

		sie_fb_ppb[id][SIE_FB_OUT_BASE][0].va = vcap_out_addr1;
		sie_fb_ppb[id][SIE_FB_OUT_BASE][0].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_BASE][0].va);
		sie_fb_ppb[id][SIE_FB_OUT_BASE][1].va = vcap_out_addr2;
		sie_fb_ppb[id][SIE_FB_OUT_BASE][1].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_BASE][1].va);

		if (ch0_size != 0) {    // raw (dram mode only)
			sie_builtin_info[id].sin_out_en[SIE_FB_OUT_CH0] = TRUE;
			sie_fb_ppb[id][SIE_FB_OUT_CH0][0].va = vcap_out_addr1;
			sie_fb_ppb[id][SIE_FB_OUT_CH0][1].va = vcap_out_addr2;
			sie_fb_ppb[id][SIE_FB_OUT_CH0][0].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_CH0][0].va);
			sie_fb_ppb[id][SIE_FB_OUT_CH0][1].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_CH0][1].va);

		} else {
			sie_builtin_info[id].sin_out_en[SIE_FB_OUT_CH0] = FALSE;
			sie_fb_ppb[id][SIE_FB_OUT_CH0][0].va = 0;
			sie_fb_ppb[id][SIE_FB_OUT_CH0][1].va = 0;
			sie_fb_ppb[id][SIE_FB_OUT_CH0][0].pa = 0;
			sie_fb_ppb[id][SIE_FB_OUT_CH0][1].pa = 0;
		}

		if (ch1_size != 0) {    // CA
			sie_builtin_info[id].sin_out_en[SIE_FB_OUT_CH1] = TRUE;
			sie_fb_ppb[id][SIE_FB_OUT_CH1][0].va = vcap_out_addr1 + ch0_size;
			sie_fb_ppb[id][SIE_FB_OUT_CH1][1].va = vcap_out_addr2 + ch0_size;
			sie_fb_ppb[id][SIE_FB_OUT_CH1][0].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_CH1][0].va);
			sie_fb_ppb[id][SIE_FB_OUT_CH1][1].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_CH1][1].va);

		} else {
			sie_builtin_info[id].sin_out_en[SIE_FB_OUT_CH1] = FALSE;
			sie_fb_ppb[id][SIE_FB_OUT_CH1][0].va = 0;
			sie_fb_ppb[id][SIE_FB_OUT_CH1][1].va = 0;
			sie_fb_ppb[id][SIE_FB_OUT_CH1][0].pa = 0;
			sie_fb_ppb[id][SIE_FB_OUT_CH1][1].pa = 0;
		}

		if (ch2_size != 0) {    // LA
			sie_builtin_info[id].sin_out_en[SIE_FB_OUT_CH2] = TRUE;
			sie_fb_ppb[id][SIE_FB_OUT_CH2][0].va = vcap_out_addr1 + ch0_size + ch1_size;
			sie_fb_ppb[id][SIE_FB_OUT_CH2][1].va = vcap_out_addr2 + ch0_size + ch1_size;
			sie_fb_ppb[id][SIE_FB_OUT_CH2][0].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_CH2][0].va);
			sie_fb_ppb[id][SIE_FB_OUT_CH2][1].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_CH2][1].va);

		} else {
			sie_builtin_info[id].sin_out_en[SIE_FB_OUT_CH2] = FALSE;
			sie_fb_ppb[id][SIE_FB_OUT_CH2][0].va = 0;
			sie_fb_ppb[id][SIE_FB_OUT_CH2][1].va = 0;
			sie_fb_ppb[id][SIE_FB_OUT_CH2][0].pa = 0;
			sie_fb_ppb[id][SIE_FB_OUT_CH2][1].pa = 0;
		}
	}

	//SIE2 set ring buffer
	if (id == SIE_ENG_ID_2 && out_dest == SIE_FB_OUT_DIRECT && ring_buf_en == 1) {
		if (init_info->ring_buf_blk_size >= ring_buf_sz) {
			sie_fb_ppb[id][SIE_FB_OUT_CH0][0].va = init_info->ring_buf_blk_addr;
			sie_fb_ppb[id][SIE_FB_OUT_CH0][1].va = init_info->ring_buf_blk_addr;
			sie_fb_ppb[id][SIE_FB_OUT_CH0][0].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_CH0][0].va);
			sie_fb_ppb[id][SIE_FB_OUT_CH0][1].pa = nvtmpp_sys_va2pa(sie_fb_ppb[id][SIE_FB_OUT_CH0][1].va);
			sie_builtin_info[id].sin_out_en[SIE_FB_OUT_CH0] = TRUE;
		} else {
			DBG_ERR("init ring buf fail, need %x > input %x\r\n", (unsigned int)ring_buf_sz, (unsigned int)init_info->ring_buf_blk_size);
			rt = E_NOMEM;
		}
	}
	return rt;
}

ER sie_fb_set_iosize(UINT32 id, SIE_BUILTIN_IOSIZE iosize)
{
	SIE_ACT_WIN_PARAM act_win = {0};
	SIE_CRP_WIN_PARAM crp_win = {0};
	SIE_BS_PARAM bs_param = {0};
	SIE_CA_WIN_PARAM ca_win;
	SIE_LA_WIN_PARAM la_win;
	SIE_CA_CROP_PARAM ca_crp;
	SIE_LA_CROP_PARAM la_crp;
	SIE_STCS_CALASIZE_INFO cala_size;
	SIE_LA_WIN_PARAM la_win_param;
	UINT32 i, update_idx;

	sie_fb_dbg_ind("set_iosize: id %d, upadte to act_win(%d, %d, %d, %d), crp_win(%d, %d, %d, %d), sie_out (%d ,%d)\r\n",id,
		iosize.act_win.x, iosize.act_win.y, iosize.act_win.w, iosize.act_win.h,
		iosize.crp_win.x, iosize.crp_win.y, iosize.crp_win.w, iosize.crp_win.h,
		sie_builtin_info[id].out_size.w, sie_builtin_info[id].out_size.h);

	if (iosize.act_win.x == 0 && iosize.act_win.y == 0 && iosize.act_win.w == 0 && iosize.act_win.h == 0) {
		iosize.act_win = sie_fb_iosize[id].act_win;
	}

	if (iosize.crp_win.x == 0 && iosize.crp_win.y == 0 && iosize.crp_win.w == 0 && iosize.crp_win.h == 0) {
		iosize.crp_win = sie_fb_iosize[id].crp_win;
	}

	if (sie_builtin_info[id].shdr_en)  {
		update_idx = sie_builtin_info[id].sen_out_dest;
		if (id != sie_builtin_info[id].dupl_src_id) {
			iosize = sie_fb_rt_chg_size[sie_builtin_info[id].dupl_src_id];
		}

		if (sie_builtin_info[id].b_start && sie_builtin_info[sie_builtin_info[id].ref_load_id].b_start)  {
			sie_fb_dbg_ind("wait ref_load_id %d vd for sync\r\n", sie_builtin_info[id].ref_load_id);
			sie_eng_wait_event_timeout(sie_builtin_info[sie_builtin_info[id].ref_load_id].ssdrv_hdl, SIE_ENG_EVENT_VD, TRUE, 1000);
		}
	} else {
		update_idx = 1 << id;
	}

	for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
		if (update_idx & 1 << i) {
			if (sie_builtin_info[i].fb_streaming &&
				iosize.act_win.w != 0 && iosize.act_win.h != 0 &&
				iosize.crp_win.w != 0 && iosize.crp_win.h != 0) {

				//active/crop win
				if (iosize.act_win.w < (iosize.crp_win.x + iosize.crp_win.w) || iosize.act_win.h < (iosize.crp_win.y + iosize.crp_win.h)) {
					DBG_ERR("id %d, active window size (%d, %d) need >= crop window (%d, %d, %d, %d)\r\n",id,
					iosize.act_win.w, iosize.act_win.h,
					iosize.crp_win.x, iosize.crp_win.y, iosize.crp_win.w, iosize.crp_win.h);
					return E_SYS;
				}

				sie_eng_get_act_win_buf_reg(sie_builtin_info[i].ssdrv_hdl, &act_win);
				sie_eng_get_crop_win_buf_reg(sie_builtin_info[i].ssdrv_hdl, &crp_win);
				sie_fb_dbg_ind("id %d, cur act_win(%d, %d, %d, %d), crp_win(%d, %d, %d, %d)\r\n", id, act_win.st_x, act_win.st_y, act_win.sz_x, act_win.sz_y, crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y);
				//active and crop window
				act_win.st_x = iosize.act_win.x;
				act_win.st_y = iosize.act_win.y;
				act_win.sz_x = iosize.act_win.w;
				act_win.sz_y = iosize.act_win.h;
				sie_eng_set_act_win_buf_reg(sie_builtin_info[i].ssdrv_hdl, &act_win);

				crp_win.st_x = iosize.crp_win.x;
				crp_win.st_y = iosize.crp_win.y;
				crp_win.sz_x = iosize.crp_win.w;
				crp_win.sz_y = iosize.crp_win.h;
				sie_eng_set_crop_win_buf_reg(sie_builtin_info[i].ssdrv_hdl, &crp_win);

				//scaling factor
				bs_param.in_sz_x = iosize.crp_win.w;
				bs_param.in_sz_y = iosize.crp_win.h;
				bs_param.out_sz_x = sie_builtin_info[i].out_size.w;
				bs_param.out_sz_y = sie_builtin_info[i].out_size.h;
				bs_param.lpf = 50;
				bs_param.bin_pwr = 0;
				bs_param.adaptivelpf_en = 1;

				if (bs_param.in_sz_x > bs_param.out_sz_x || bs_param.in_sz_y > bs_param.out_sz_y) {
					sie_eng_set_bs_buf_reg(sie_builtin_info[i].ssdrv_hdl, &bs_param);
				}

				//ca/la
				sie_eng_get_ca_crop_buf_reg(sie_builtin_info[i].ssdrv_hdl, &ca_crp);
				ca_crp.st_x = 0;
				ca_crp.st_y = 0;
				ca_crp.sz_x = iosize.crp_win.w;
				ca_crp.sz_y = iosize.crp_win.h;
				sie_eng_set_ca_crop_buf_reg(sie_builtin_info[i].ssdrv_hdl, &ca_crp);

				la_crp.st_x = 0;
				la_crp.st_y = 0;
				if (ca_crp.sz_x > SIE_CA_SMPL_MAX_W) {
					la_crp.sz_x = SIE_CA_SMPL_MAX_W / 2;
				} else {
					la_crp.sz_x = ca_crp.sz_x / 2;
				}
				if (ca_crp.sz_y > SIE_CA_SMPL_MAX_H) {
					la_crp.sz_y = SIE_CA_SMPL_MAX_H / 2;
				} else {
					la_crp.sz_y = ca_crp.sz_y / 2;
				}
				sie_eng_set_la_crop_buf_reg(sie_builtin_info[i].ssdrv_hdl, &la_crp);

				sie_eng_get_ca_win_buf_reg(sie_builtin_info[i].ssdrv_hdl, &ca_win);
				cala_size.ca_cfa_pat = ca_crp.cfa_pat;
				cala_size.ca_roi_stx = ca_crp.st_x;
				cala_size.ca_roi_sty = ca_crp.st_y;
				cala_size.ca_roi_szx = ca_crp.sz_x;
				cala_size.ca_roi_szy = ca_crp.sz_y;
				cala_size.ca_win_numx = ca_win.win_numx;
				cala_size.ca_win_numy = ca_win.win_numy;

				sie_eng_get_la_win_buf_reg(sie_builtin_info[i].ssdrv_hdl, &la_win);
				cala_size.la_roi_stx = la_crp.st_x;
				cala_size.la_roi_sty = la_crp.st_y;
				cala_size.la_roi_szx = la_crp.sz_x;
				cala_size.la_roi_szy = la_crp.sz_y;
				cala_size.la_win_numx = la_win.win_numx;
				cala_size.la_win_numy = la_win.win_numy;
				sie_eng_set_cala_size_buf_reg(sie_builtin_info[i].ssdrv_hdl, &cala_size, &la_win_param);
				if (sie_builtin_info[i].b_start == TRUE) {
					sie_eng_set_load_hw_reg(sie_builtin_info[i].ssdrv_hdl);
				}
			}
			sie_fb_rt_chg_size[i] = iosize;
			sie_fb_dbg_ind("rt chg: id %d, shdr_en %d, (src_id: %d, ref_load_id: %d), update_idx %x,upadte to act_win(%d, %d, %d, %d), crp_win(%d, %d, %d, %d)\r\n",
				i,	sie_builtin_info[i].shdr_en, sie_builtin_info[i].dupl_src_id,sie_builtin_info[i].ref_load_id,update_idx,
				iosize.act_win.x, iosize.act_win.y, iosize.act_win.w, iosize.act_win.h,
				iosize.crp_win.x, iosize.crp_win.y, iosize.crp_win.w, iosize.crp_win.h);
		}
	}
	return E_OK;
}

ER sie_fb_get_clk(UINT32 id, UINT32 *clk_src, UINT32 *clk_rate)
{
	SIE_BUILTIN_RESOURCE *builtin_resource;

	if ((builtin_resource = sie_builtin_resource_get(id)) == NULL) {
		return E_PAR;
	} else {
		if (clk_src != NULL) {
			*clk_src  = builtin_resource->clk_src;
		}
		if (clk_rate != NULL) {
			*clk_rate  = builtin_resource->clk_rate;
		}
		return E_OK;
	}
}

void sie_fb_get_rdy_addr(UINT32 id, ULONG *addr_ch0, ULONG *addr_ch1, ULONG *addr_ch2)
{
	if (addr_ch0 != NULL) {
		*addr_ch0 = sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY].va;
	}

	if (addr_ch1 != NULL) {
		*addr_ch1 = sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY].va;
	}

	if (addr_ch2 != NULL) {
		*addr_ch2 = sie_fb_rdy_addr[id][SIE_FB_OUT_CH2][SIE_FB_BUF_IDX_RDY].va;
	}

	if (sie_fb_dbg_dump_cnt[id] <= sie_fb_dbg_dump_max_cnt) {
		sie_fb_dbg_ind("id %d, fc %d, get rdy address 0x%lx, 0x%lx, 0x%lx\r\n", id, sie_builtin_info[id].header_info.count,
		sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY].va,
		sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY].va,
		sie_fb_rdy_addr[id][SIE_FB_OUT_CH2][SIE_FB_BUF_IDX_RDY].va);
	}
}

USIZE sie_fb_get_out_size(UINT32 id)
{
	SIE_BS_H_PARAM bs_h = {0};
	SIE_BS_V_PARAM bs_v = {0};
	USIZE out_size;

	sie_eng_get_bsh_buf_reg(sie_builtin_info[id].ssdrv_hdl, &bs_h);
	out_size.w = bs_h.out_sz;
	sie_eng_get_bsv_buf_reg(sie_builtin_info[id].ssdrv_hdl, &bs_v);
	out_size.h = bs_v.out_sz;

	return out_size;
}

void sie_fb_buf_ctrl(UINT32 id, BOOL int_cb_trig, UINT32 status, SIE_FB_BUF_IDX buf_idx, UINT32 buf_io_ctl)
{
	unsigned long flags;
	SIE_CRP_WIN_PARAM crp_win = {0};
	UINT32 func;
	VOS_TICK ts;
	UINT32 i;

	if (int_cb_trig && sie_fb_int_cb) {
		sie_fb_int_cb(id, status);
	}

	sie_eng_get_crop_win_buf_reg(sie_builtin_info[id].ssdrv_hdl, &crp_win);
	sie_eng_get_function_enable_buf_reg(sie_builtin_info[id].ssdrv_hdl, &func);
	vos_perf_mark(&ts);

	if (sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].va != 0) {
		if (sie_builtin_info[id].out_dest == SIE_FB_OUT_DIRECT || sie_fb_buf_out_cb == NULL) {
			// direct or NULL buf_cb, sie unlock buffer
			nvtmpp_unlock_fastboot_blk(sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].va);
			sie_fb_dbg_ind("[SIE FB] buf_rls id: %d, idx %d, comm_buf_addr va:0x%lx, dram_en %d, func: 0x%x, crp_win(%d, %d, %d, %d), buf_io_ctl: %d, ts: %d\r\n",
							id, buf_idx, sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].va, sie_builtin_info[id].out_dest, func,
							crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y, buf_io_ctl, sie_builtin_info[id].header_info.timestamp);
		} else if (sie_builtin_info[id].out_dest == SIE_FB_OUT_DRAM) {
			// dram mode, ipp unlock buffer (raw/ca/la)
			if (sie_fb_buf_out_cb) {
				if (buf_io_ctl == SIE_BUILTIN_HEADER_CTL_PUSH) {
					sie_header_loc_cpu(flags);
					sie_builtin_info[id].header_info.buf_ctrl = SIE_BUILTIN_HEADER_CTL_PUSH;
					sie_builtin_info[id].header_info.buf_addr = sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].va;
					sie_builtin_info[id].header_info.buf_addr_pa = sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].pa;
					sie_builtin_info[id].header_info.addr_ch0 = sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][buf_idx].va;
					sie_builtin_info[id].header_info.addr_ch0_pa = sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][buf_idx].pa;
					sie_builtin_info[id].header_info.addr_ch1 = sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][buf_idx].va;
					sie_builtin_info[id].header_info.addr_ch1_pa = sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][buf_idx].pa;
					sie_builtin_info[id].header_info.timestamp = sie_fb_vd_ts[id];

					if (sie_builtin_info[id].shdr_en) {
						if (sie_builtin_info[id].dupl_src_id == id) {
							for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
								if (sie_builtin_info[id].sen_out_dest & (1 << i))  {
									sie_builtin_info[id].header_info.frm2_buf_addr = sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][buf_idx].va;
									sie_builtin_info[id].header_info.frm2_buf_addr_pa = sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][buf_idx].pa;
									sie_builtin_info[id].header_info.frm2_addr_ch0 = sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][buf_idx].va;
									sie_builtin_info[id].header_info.frm2_addr_ch0_pa = sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][buf_idx].pa;
								}
							}

							sie_fb_buf_out_cb(id, &sie_builtin_info[id].header_info);
							sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].va = 0;
							sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].pa = 0;
							sie_fb_dbg_ind("[SIE FB] buf_rls id: %d, idx %d, comm_buf_addr va:0x%lx(frm2: 0x%lx), dram_en %d, func: 0x%x, crp_win(%d, %d, %d, %d), buf_io_ctl: %d, ts: %d\r\n",
											id, buf_idx, sie_builtin_info[id].header_info.buf_addr, sie_builtin_info[id].header_info.frm2_buf_addr, sie_builtin_info[id].out_dest, func,
											crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y, buf_io_ctl, sie_builtin_info[id].header_info.timestamp);
						}
					} else {
						sie_fb_buf_out_cb(id, &sie_builtin_info[id].header_info);
						sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].va = 0;
						sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].pa = 0;
						sie_fb_dbg_ind("[SIE FB] buf_rls id: %d, idx %d, comm_buf_addr va:0x%lx(frm2: 0x%lx), dram_en %d, func: 0x%x, crp_win(%d, %d, %d, %d), buf_io_ctl: %d, ts: %d\r\n",
										id, buf_idx, sie_builtin_info[id].header_info.buf_addr, sie_builtin_info[id].header_info.frm2_buf_addr, sie_builtin_info[id].out_dest, func,
										crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y, buf_io_ctl, sie_builtin_info[id].header_info.timestamp);
					}

					sie_header_unl_cpu(flags);
				} else if (buf_io_ctl == SIE_BUILTIN_HEADER_CTL_UNLOCK) {
					nvtmpp_unlock_fastboot_blk(sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].va);
					sie_fb_dbg_ind("[SIE FB] buf_rls id: %d, idx %d, comm_buf_addr va:0x%lx, dram_en %d, func: 0x%x, crp_win(%d, %d, %d, %d), buf_io_ctl: %d, ts: %d\r\n",
									id, buf_idx, sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][buf_idx].va, sie_builtin_info[id].out_dest, func,
									crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y, buf_io_ctl, sie_builtin_info[id].header_info.timestamp);
				}
			}
		}
	}
}

static UINT32 sie_fb_get_frame_end_sts(SIE_ENG_ID_T id)
{
	if (sie_builtin_info[id].out_dest == SIE_FB_OUT_DIRECT || sie_builtin_dbg_info.out_two_frame_only == TRUE) {
		return SIE_INT_CROPEND;
	} else {
		return SIE_INT_DRAM_OUT0_END;
	}
}

void sie_fb_common_isr(SIE_ENG_ID_T id, void *eng, UINT32 status, void *reserve)
{
	SIE_DRAM_ADDR addr[SIE_FB_OUT_CH_MAX] = {0};
	unsigned long flags;
	SIE_CRP_WIN_PARAM crp_win;
	UINT32 func;
	UINT32 ch;
	VOS_TICK ts = 0;
	static VOS_TICK prv_ts[SIE_MAX_ENG_NUM] = {0};
	ULONG ch_out_paddr;
	SIE_ENGINE_STATUS_CB *ssdrv_cb = (SIE_ENGINE_STATUS_CB *)reserve;
	UINT32 kdrv_status = 0;
	UINT32 i;
	SIE_DRAM_OUT0_PARAM out_ch0_param;

#if defined(__FREERTOS)	//check vd for first interrupt status
	if ((status & SIE_INT_VD) && (sie_fb_first_vd[id] == FALSE)) {
		sie_fb_first_vd[id] = TRUE;
	}
#else
	sie_fb_first_vd[id] = TRUE;
	sie_builtin_info[id].b_start = TRUE;
#endif

	if (sie_fb_first_vd[id] == FALSE || sie_builtin_info[id].b_start == FALSE) {
		return;
	}

	//check kdrv isr cb reg
	if (sie_builtin_info[id].fastboot_last_frame == TRUE) {
		kdrv_status = status;
		sie_builtin_info[id].fatboot_isr_proc = FALSE;
		sie_builtin_info[id].kdrv_isr_proc = TRUE;
	} else {
		// low latency process
		if (sie_builtin_info[id].low_latency_en) {
			status &= ~(sie_fb_get_frame_end_sts(id));
			if (status & SIE_INT_BP3) {
				status |= sie_fb_get_frame_end_sts(id);
			}
		}

		/*
			check fastboot last frame when
			case 1: shdr disable
			case 2: shdr enable and id = ref_load_id (long exp frame)
			case 3: shdr enable and ref_load_id fastboot_last_frame is TRUE (long exp frame need switch to kdrv first)
		*/
		if ((status & sie_fb_get_frame_end_sts(id)) &&
			(sie_builtin_info[id].kdrv_cb) &&
			((sie_builtin_info[id].shdr_en == 0) ||	//case1
			(sie_builtin_info[id].shdr_en && sie_builtin_info[id].ref_load_id == id) ||	//case2
			(sie_builtin_info[id].shdr_en && sie_builtin_info[sie_builtin_info[id].ref_load_id].fastboot_last_frame == TRUE))) {	//cas3
			sie_builtin_info[id].fastboot_last_frame = TRUE;
			sie_builtin_info[id].fatboot_isr_proc = TRUE;
			status &= ~SIE_INT_VD;
            /* Frame(N)-Crop_End + Frame(N+1)-VD */
            if (status & SIE_INT_VD) {
				kdrv_status = (status & ~(sie_fb_get_frame_end_sts(id)));	///< clear CROP_END status for Frame(N+1) callback
            	sie_builtin_info[id].kdrv_isr_proc = TRUE;
            } else {
            	sie_builtin_info[id].kdrv_isr_proc = FALSE;
            }
	    } else {
			sie_builtin_info[id].fastboot_last_frame = FALSE;
			sie_builtin_info[id].fatboot_isr_proc = TRUE;
			sie_builtin_info[id].kdrv_isr_proc = FALSE;
	    }
	}

	if (sie_builtin_info[id].fatboot_isr_proc == TRUE) {
		sie_header_loc_cpu(flags);

		//update ping pong buffer
		if (status & sie_fb_get_frame_end_sts(id)) { //set ready adderss
			if (sie_builtin_info[id].shdr_en && sie_builtin_info[id].out_dest == SIE_FB_OUT_DRAM) {
				if (sie_builtin_info[id].dupl_src_id == id) {
					for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
						if (sie_builtin_info[id].sen_out_dest & (1 << i))  {
							sie_eng_get_function_enable_buf_reg(sie_builtin_info[i].ssdrv_hdl, &func);
							ctl_sie_fb_de_flg[i] = TRUE;
							sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT];
							sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[i][SIE_FB_OUT_BASE][sie_builtin_info[i].frame_count % SIE_FB_RING_BUF_NUM];    //for hdal buf release

							sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT];
							sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[i][SIE_FB_OUT_CH0][sie_builtin_info[i].frame_count % SIE_FB_RING_BUF_NUM];    //for hdal buf release

							if (func & SIE_FB_FUNC_DVI_EN_BIT) {	//update ch1 address when dvi enable
								sie_fb_rdy_addr[i][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[i][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_CUR_OUT];
								sie_fb_rdy_addr[i][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[i][SIE_FB_OUT_CH1][sie_builtin_info[i].frame_count % SIE_FB_RING_BUF_NUM];    //for hdal buf release
							}

							sie_builtin_info[i].header_info.buf_addr = sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY].va;
							sie_builtin_info[i].header_info.buf_addr_pa = sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY].pa;
							sie_builtin_info[i].header_info.addr_ch0 = sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY].va;
							sie_builtin_info[i].header_info.addr_ch0_pa = sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY].pa;
							sie_builtin_info[i].header_info.addr_ch1 = sie_fb_rdy_addr[i][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY].va;
							sie_builtin_info[i].header_info.addr_ch1_pa = sie_fb_rdy_addr[i][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY].pa;
							sie_builtin_info[i].header_info.count = sie_builtin_info[i].frame_count;
							sie_builtin_info[i].header_info.timestamp = sie_fb_vd_ts[i];

							//set src id header frame2 addr info
							sie_builtin_info[id].header_info.frm2_buf_addr = sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY].va;
							sie_builtin_info[id].header_info.frm2_buf_addr_pa = sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY].pa;
							sie_builtin_info[id].header_info.frm2_addr_ch0 = sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY].va;
							sie_builtin_info[id].header_info.frm2_addr_ch0_pa = sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY].pa;
						}
					}
				}
			} else {
				ctl_sie_fb_de_flg[id] = TRUE;
				sie_eng_get_function_enable_buf_reg(sie_builtin_info[id].ssdrv_hdl, &func);
				sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT];
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT];
				if (func & SIE_FB_FUNC_DVI_EN_BIT) {	//update ch1 address when dvi enable
					sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_CUR_OUT];
					sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH1][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM];    //for hdal buf release
				}

				sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_BASE][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM];    //for hdal buf release
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH0][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM];    //for hdal buf release

				sie_builtin_info[id].header_info.buf_addr = sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY].va;
				sie_builtin_info[id].header_info.buf_addr_pa = sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY].pa;
				sie_builtin_info[id].header_info.addr_ch0 = sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY].va;
				sie_builtin_info[id].header_info.addr_ch0_pa = sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_RDY].pa;
				sie_builtin_info[id].header_info.addr_ch1 = sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY].va;
				sie_builtin_info[id].header_info.addr_ch1_pa = sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY].pa;
				sie_builtin_info[id].header_info.count = sie_builtin_info[id].frame_count;
				sie_builtin_info[id].header_info.timestamp = sie_fb_vd_ts[id];
			}
		}

		//update ping pong buffer
		if (status & SIE_INT_DRAM_OUT1_END) {   //set ready adderss
			sie_eng_get_function_enable_buf_reg(sie_builtin_info[id].ssdrv_hdl, &func);
			if (!(func & SIE_FB_FUNC_DVI_EN_BIT)) {	//update ch1 address when dvi disable, ca output
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_CUR_OUT];
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH1][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM];    //for hdal buf release
			}
			if (sie_fb_dbg_dump_cnt[id] <= sie_fb_dbg_dump_max_cnt) {
				sie_fb_dbg_ind("id %d, fc %d, out1 rdy addr 0x%lx\r\n", id,
					sie_builtin_info[id].frame_count,
					sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_RDY].va);
			}
		}

		if (status & SIE_INT_DRAM_OUT2_END) {   //set ready adderss
			sie_fb_rdy_addr[id][SIE_FB_OUT_CH2][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[id][SIE_FB_OUT_CH2][SIE_FB_BUF_IDX_CUR_OUT];
			sie_fb_rdy_addr[id][SIE_FB_OUT_CH2][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH2][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM];    //for hdal buf release
			if (sie_fb_dbg_dump_cnt[id] <= sie_fb_dbg_dump_max_cnt) {
				sie_fb_dbg_ind("id %d, fc %d, out2 rdy addr 0x%lx\r\n", id,
					sie_builtin_info[id].frame_count,
					sie_fb_rdy_addr[id][SIE_FB_OUT_CH2][SIE_FB_BUF_IDX_RDY].va);
			}
		}

		if (status & SIE_INT_VD) {
			sie_fb_dbg_dump_cnt[id]++;
			if (sie_builtin_info[id].ref_load_id == id) {
				sie_builtin_info[id].b_ref_load_id_vd = TRUE;
			}

			if (sie_builtin_info[id].shdr_en && sie_builtin_info[id].out_dest == SIE_FB_OUT_DRAM) {
				if (sie_builtin_info[id].dupl_src_id == id) {
					for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
						if (sie_builtin_info[id].sen_out_dest & (1 << i)) {
							if (ctl_sie_fb_de_flg[i] == FALSE) {
								sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT];
							}
							sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[i][SIE_FB_OUT_BASE][sie_builtin_info[i].frame_count % SIE_FB_RING_BUF_NUM];
							sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[i][SIE_FB_OUT_CH0][sie_builtin_info[i].frame_count % SIE_FB_RING_BUF_NUM];
							sie_fb_rdy_addr[i][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[i][SIE_FB_OUT_CH1][sie_builtin_info[i].frame_count % SIE_FB_RING_BUF_NUM];
							sie_fb_rdy_addr[i][SIE_FB_OUT_CH2][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[i][SIE_FB_OUT_CH2][sie_builtin_info[i].frame_count % SIE_FB_RING_BUF_NUM];
#if defined(__FREERTOS)
							sie_builtin_info[i].bridge_info.frame_cnt = sie_builtin_info[i].frame_count;	//save frame cnt for rtos to linux
#endif
							sie_builtin_info[i].frame_count++;	//for setting next frame address
							sie_eng_get_function_enable_buf_reg(sie_builtin_info[i].ssdrv_hdl, &func);
							sie_eng_get_crop_win_buf_reg(sie_builtin_info[i].ssdrv_hdl, &crp_win);
							if ((sie_fb_iosize[i].crp_win.w != crp_win.sz_x) || (sie_fb_iosize[i].crp_win.h != crp_win.sz_y)) {
								//set out size to 0 for ipp to skip this frame
								sie_builtin_info[i].header_info.out_size.w = 0;
								sie_builtin_info[i].header_info.out_size.h = 0;
							} else {
								sie_builtin_info[i].header_info.out_size = sie_fb_get_out_size(i);
								if (sie_builtin_info[i].header_info.out_size.w == 0 && sie_builtin_info[i].header_info.out_size.h == 0) {
									sie_builtin_info[i].header_info.out_size.w = sie_fb_iosize[i].crp_win.w;
									sie_builtin_info[i].header_info.out_size.h = sie_fb_iosize[i].crp_win.h;
								}
							}

							for (ch = 0; ch < SIE_FB_OUT_CH_MAX; ch++) {
								if (sie_builtin_fp_set_dramoutmode[ch] != NULL) {
									ch_out_paddr = sie_fb_ppb[i][ch][sie_builtin_info[i].frame_count % SIE_FB_RING_BUF_NUM].pa;
									if (ch_out_paddr != 0) {
										addr[ch].addr_pa = ch_out_paddr;
										sie_builtin_fp_set_dramoutmode[ch](sie_builtin_info[i].ssdrv_hdl, &addr[ch]);
									}
								}
								//debug code for skip single out setting when sie output 2 frame
								if (sie_builtin_fp_set_singleouten[ch] != NULL) {
									if (sie_builtin_dbg_info.out_two_frame_only == TRUE && sie_builtin_info[i].header_info.out_size.w != 0 && sie_builtin_info[i].header_info.out_size.h != 0) {
										if (ch == 1 && sie_fb_dbg_push_cnt[i] > 0) {
											sie_fb_dbg_push_cnt[i]--;
											sie_eng_get_dramout0_buf_reg(sie_builtin_info[i].ssdrv_hdl, &out_ch0_param);
											DBG_DUMP("id %d: raw addr_va 0x%lx addr_pa 0x%lx, size(%d %d), lofs %d, enc: %d\r\n", i,
											sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT].va,
											sie_fb_rdy_addr[i][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT].pa,
											sie_builtin_info[i].header_info.out_size.w, sie_builtin_info[i].header_info.out_size.h,
											out_ch0_param.lofs, (func & SIE_FB_FUNC_RAWENC_EN_BIT?1:0));
											if (sie_fb_dbg_push_cnt[id] != 0) {
												sie_builtin_fp_set_singleouten[ch](sie_builtin_info[i].ssdrv_hdl, sie_builtin_info[i].sin_out_en[ch]);
											}
										}
									} else {
										sie_builtin_fp_set_singleouten[ch](sie_builtin_info[i].ssdrv_hdl, sie_builtin_info[i].sin_out_en[ch]);
									}
								}
							}
							ctl_sie_fb_de_flg[i] = FALSE;
							sie_eng_set_load_hw_reg(sie_builtin_info[i].ssdrv_hdl);

							vos_perf_mark(&ts);
							if (sie_fb_dbg_dump_cnt[id] <= sie_fb_dbg_dump_max_cnt) {
								sie_fb_dbg_ind("[FB] vd: id: %d, cur_adr: 0x%lx, func: 0x%x, crp_win(%d, %d, %d, %d), ts: %d\r\n",
												i, sie_fb_rdy_addr[i][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT].va, func,
												crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y, ts);
							}
						}
					}
				}
			} else {
				if (ctl_sie_fb_de_flg[id] == FALSE) {
					sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY] = sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT];
				}
				sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_BASE][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM];
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH0][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM];
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH1][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM];
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH2][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH2][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM];
#if defined(__FREERTOS)
				sie_builtin_info[id].bridge_info.frame_cnt = sie_builtin_info[id].frame_count;	//save frame cnt for rtos to linux
#endif
				sie_builtin_info[id].frame_count++;	//for setting next frame address

				sie_eng_get_function_enable_buf_reg(sie_builtin_info[id].ssdrv_hdl, &func);
				sie_eng_get_crop_win_buf_reg(sie_builtin_info[id].ssdrv_hdl, &crp_win);
				if ((sie_fb_iosize[id].crp_win.w != crp_win.sz_x) || (sie_fb_iosize[id].crp_win.h != crp_win.sz_y)) {
					sie_builtin_info[id].header_info.out_size.w = 0;
					sie_builtin_info[id].header_info.out_size.h = 0;
				} else {
					sie_builtin_info[id].header_info.out_size = sie_fb_get_out_size(id);
					if (sie_builtin_info[id].header_info.out_size.w == 0 && sie_builtin_info[id].header_info.out_size.h == 0) {
						sie_builtin_info[id].header_info.out_size.w = sie_fb_iosize[id].crp_win.w;
						sie_builtin_info[id].header_info.out_size.h = sie_fb_iosize[id].crp_win.h;
					}
				}
				for (ch = 0; ch < SIE_FB_OUT_CH_MAX; ch++) {
					if (sie_builtin_fp_set_dramoutmode[ch] != NULL) {
						ch_out_paddr = sie_fb_ppb[id][ch][sie_builtin_info[id].frame_count % SIE_FB_RING_BUF_NUM].pa;
						if (ch_out_paddr != 0) {
							addr[ch].addr_pa = ch_out_paddr;
							sie_builtin_fp_set_dramoutmode[ch](sie_builtin_info[id].ssdrv_hdl, &addr[ch]);
						}
					}
					//debug code for skip single out setting when sie output 2 frame
					if (sie_builtin_fp_set_singleouten[ch] != NULL) {
						if (sie_builtin_dbg_info.out_two_frame_only == TRUE && sie_builtin_info[id].header_info.out_size.w != 0 && sie_builtin_info[id].header_info.out_size.h != 0) {
							if (ch == 1 && sie_fb_dbg_push_cnt[id] > 0) {
								sie_fb_dbg_push_cnt[id]--;
								sie_eng_get_dramout0_buf_reg(sie_builtin_info[id].ssdrv_hdl, &out_ch0_param);
								DBG_DUMP("id %d: raw addr_va 0x%lx addr_pa 0x%lx, size(%d %d), lofs %d, enc: %d\r\n", id,
									sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT].va,
									sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT].pa,
									sie_builtin_info[id].out_size.w, sie_builtin_info[id].out_size.h,
									out_ch0_param.lofs, (func & SIE_FB_FUNC_RAWENC_EN_BIT?1:0));
								if (sie_fb_dbg_push_cnt[id] != 0) {
									sie_builtin_fp_set_singleouten[ch](sie_builtin_info[id].ssdrv_hdl, sie_builtin_info[id].sin_out_en[ch]);
								}
							}
						} else {
							sie_builtin_fp_set_singleouten[ch](sie_builtin_info[id].ssdrv_hdl, sie_builtin_info[id].sin_out_en[ch]);
						}
					}
				}
				ctl_sie_fb_de_flg[id] = FALSE;
				sie_eng_set_load_hw_reg(sie_builtin_info[id].ssdrv_hdl);

				vos_perf_mark(&ts);
				if (sie_fb_dbg_dump_cnt[id] <= sie_fb_dbg_dump_max_cnt) {
					sie_fb_dbg_ind("[FB] vd: id: %d, cur_adr:0x%lx, head adr: 0x%lx, fc %d, crp_win(%d, %d, %d, %d), ts: %d\r\n",
									id, sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT].va, sie_builtin_info[id].header_info.buf_addr, sie_builtin_info[id].header_info.count,
									crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y, ts);
				}
			}

			//update timestamp
			vos_perf_mark(&sie_fb_vd_ts[id]);
			sie_builtin_info[id].bridge_info.timestamp = sie_fb_vd_ts[id];

			if (ssdrv_cb != NULL) {
#if 0
				if (ssdrv_cb->status.dram_in1_udfl || ssdrv_cb->status.dram_in2_udfl || ssdrv_cb->status.dram_out0_err ||
					ssdrv_cb->status.dram_out0_lnbufovfl || ssdrv_cb->status.dram_out1_lnbufovfl ||
					ssdrv_cb->status.dram_out2_lnbufovfl || ssdrv_cb->status.dram_out3_lnbufovfl) {
					sie_fb_dbg_err("id %d in_udfl(%d,%d), out0_err %d, out_ch_ovfl(%d, %d, %d, %d)\r\n", id,
						ssdrv_cb->status.dram_in1_udfl, ssdrv_cb->status.dram_in1_udfl, ssdrv_cb->status.dram_out0_err,
						ssdrv_cb->status.dram_out0_lnbufovfl, ssdrv_cb->status.dram_out1_lnbufovfl, ssdrv_cb->status.dram_out2_lnbufovfl, ssdrv_cb->status.dram_out3_lnbufovfl);
				}
#endif
			}
			sie_fb_vd[id] = TRUE;
		}

		sie_header_unl_cpu(flags);
		//callback to interrupt cb_fp(isp)
		if (sie_fb_int_cb && (sie_builtin_dbg_info.isp_bypass_en == 0) ) {
			if (sie_builtin_info[id].shdr_en) {
				if (sie_builtin_info[sie_builtin_info[id].ref_load_id].b_ref_load_id_vd == TRUE) {
					sie_fb_int_cb(id, status);
				}
			} else {
				sie_fb_int_cb(id, status);
			}
		}

		//release buffer
		if (status & sie_fb_get_frame_end_sts(id)) {
			if (sie_builtin_info[id].out_dest == SIE_FB_OUT_DRAM) {
				//dram mode
				if (sie_fb_buf_out_cb) {
					if (ssdrv_cb != NULL && sie_builtin_info[id].fastboot_last_frame == TRUE) {
						sie_builtin_info[id].header_info.buf_ctrl = SIE_BUILTIN_HEADER_CTL_PUSH;	//trigger ipp process and unlock
					} else {
						sie_builtin_info[id].header_info.buf_ctrl = SIE_BUILTIN_HEADER_CTL_LOCK;	//trigger ipp process only
					}

					if ((sie_builtin_info[id].shdr_en == 0) || (sie_builtin_info[id].shdr_en && sie_builtin_info[id].dupl_src_id == id)) {
						vos_perf_mark(&ts);
						sie_eng_get_crop_win_buf_reg(sie_builtin_info[id].ssdrv_hdl, &crp_win);
						sie_eng_get_function_enable_buf_reg(sie_builtin_info[id].ssdrv_hdl, &func);
						if (sie_builtin_info[id].header_info.buf_ctrl == SIE_BUILTIN_HEADER_CTL_PUSH) {
							sie_fb_dbg_ind("[FB] ce: id: %d, out_adr: 0x%lx(pa: 0x%lx),func: 0x%x, crp_win(%d, %d, %d, %d), buf_io: %d, fc: %d, ts: %d\r\n",
											id, sie_builtin_info[id].header_info.buf_addr, sie_builtin_info[id].header_info.buf_addr_pa,
											func, crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y,
											sie_builtin_info[id].header_info.buf_ctrl, sie_builtin_info[id].header_info.count, sie_builtin_info[id].header_info.timestamp);
						}
						if (sie_builtin_dbg_info.push_ts_print == TRUE) {
							vos_perf_mark(&ts);
							DBG_DUMP("%d, %d, %d, +%d\r\n", id, sie_builtin_info[id].header_info.timestamp/1000, ts/1000, (sie_builtin_info[id].header_info.timestamp-prv_ts[id])/1000);
							prv_ts[id] = sie_builtin_info[id].header_info.timestamp;
						}
						sie_fb_buf_out_cb(id, &sie_builtin_info[id].header_info);
					}
				}
			} else {
				if (sie_builtin_dbg_info.push_ts_print == TRUE) {
					vos_perf_mark(&ts);
					DBG_DUMP("%d, %d, %d, +%d\r\n", id, sie_builtin_info[id].header_info.timestamp/1000, ts/1000, (sie_builtin_info[id].header_info.timestamp-prv_ts[id])/1000);
					prv_ts[id] = sie_builtin_info[id].header_info.timestamp;
				}
				//unlock buffer when direct mode + fastboot last frame process
				if (sie_builtin_info[id].fastboot_last_frame == TRUE) {
					sie_fb_dbg_ind("[FB] id: %d, unlock out_adr: 0x%lx(pa: 0x%lx)\r\n",
								id, sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY].va, sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY].pa);
					nvtmpp_unlock_fastboot_blk(sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_RDY].va);
				}
			}
		}
	}

	if (sie_builtin_info[id].kdrv_cb != NULL && sie_builtin_info[id].kdrv_isr_proc == TRUE) {
		sie_builtin_info[id].kdrv_cb(eng, kdrv_status, reserve);
	}
}

static void sie1_fb_isr(void *eng, UINT32 status, void *reserve)
{
	sie_fb_common_isr(SIE_ENG_ID_1, eng, status, reserve);
}

static void sie2_fb_isr(void *eng, UINT32 status, void *reserve)
{
	sie_fb_common_isr(SIE_ENG_ID_2, eng, status, reserve);
}

static void sie3_fb_isr(void *eng, UINT32 status, void *reserve)
{
	sie_fb_common_isr(SIE_ENG_ID_3, eng, status, reserve);
}

static void sie4_fb_isr(void *eng, UINT32 status, void *reserve)
{
	sie_fb_common_isr(SIE_ENG_ID_4, eng, status, reserve);
}

static void sie5_fb_isr(void *eng, UINT32 status, void *reserve)
{
	sie_fb_common_isr(SIE_ENG_ID_5, eng, status, reserve);
}

void sie_fb_wait_vd(UINT32 id)
{
	UINT32 chk_cnt = 1000;

	sie_fb_vd[id] = FALSE;
	while (1) {
		if (sie_fb_vd[id] == TRUE) {
			return;
		}
		vos_util_delay_ms(1);
		chk_cnt--;
		if (chk_cnt == 0) {
			sie_fb_dbg_err("wait vd timeout for %d ms\r\n", chk_cnt);
			return;
		}
	}
}

static UINT32 sie_fb_id_bit = 0;
static BOOL sie_fb_id_chk(UINT32 id)
{
	if ((sie_fb_id_bit) & (1 << id)) {
		return TRUE; // is fastboot id
	}
	return FALSE; // not fastboot id
}
ER sie_builtin_rst(UINT32 id_bit)
{
#if defined(__KERNEL__)
	int fastboot_nodeoffset;
	UINT32 tmp_u32;
#endif
	UINT32 id;

#if defined(__KERNEL__)
	if (sie_init_plat_get_nodeoffset(FAST_BUILTIN_NODE_HEAD, &fastboot_nodeoffset) == 0) {
		if (sie_init_plat_read_dtsi_array(fastboot_nodeoffset, "generation", &tmp_u32, 1) == E_OK) {
			fast_builtin_gen = tmp_u32;
		}
	}
#else
	fast_builtin_gen = 0;
#endif

	if (fast_builtin_gen != 2) {
		/* rst sie module, avoid rtos to linux crash in interrupt handle */
		for (id = 0; id < SIE_MAX_ENG_NUM; id++) {
			if ((id_bit) & (1 << id))  {
				sie_eng_write_hw_reg(sie_builtin_info[id].ssdrv_hdl, SIE_FB_CTL_OFS, SIE_FB_CTL_RST_BIT); // rst module
				sie_eng_write_hw_reg(sie_builtin_info[id].ssdrv_hdl, SIE_FB_INTE_OFS, 0); // close interrupt enable
				sie_eng_write_hw_reg(sie_builtin_info[id].ssdrv_hdl, SIE_FB_INTE_OFS, 0xffffffff);	 // clear interrupt status
			}
		}
	}
	return E_OK;
}

ER sie_builtin_init(SIE_BUILTIN_INIT_INFO *info)
{
	SIE_DRAM_ADDR addr[SIE_FB_OUT_CH_MAX] = {0};
	UINT32 tmp_u32;
	UINT32 id = 0, i = 0, j, ch;
	UINT32 *reg_value;
	SIE_BS_H_PARAM bs_h = {0};
	SIE_BS_V_PARAM bs_v = {0};
	SIE_CRP_WIN_PARAM crp_win = {0};
	SIE_START_CFG ssdrv_trig_param = {0};
	VOS_TICK ts;
	const SIE_FB_INT_ISR_CB sie_isr_fp[] = {
		sie1_fb_isr,
		sie2_fb_isr,
		sie3_fb_isr,
		sie4_fb_isr,
		sie5_fb_isr,
	};
	int nodeoffset = 0, nodeoffset_curr = 0;
#if defined(__KERNEL__)
	UINT32 bridge_val = 0;
#endif

	// init offset value
	for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
		for (j = 0; j < SIE_INIT_NODE_NUM; j++) {
			nodeoffset_list[i][j] = -FDT_ERR_NOTFOUND;
		}
	}
	for (i = 0; i < SIE_INIT_NODE_GLB_NUM; i++) {
		nodeoffset_list_dbg[i] = -FDT_ERR_NOTFOUND;
	}

	if (sie_init_plat_get_nodeoffset(SIE_BUILTIN_NODE_HEAD, &nodeoffset) < 0) {
		// error handling
		DBG_ERR("%s get nodeofs %d fail\r\n", SIE_BUILTIN_NODE_HEAD, nodeoffset);
		return E_SYS;
	}

	// list subnode offset, avoid dummy fdt search
	sie_init_plat_get_nodeoffset_list(nodeoffset, &nodeoffset_list[0][0], &nodeoffset_list_dbg[0]);

	//get debug level here for msg print
	nodeoffset_curr = nodeoffset_list_dbg[SIE_INIT_NODE_GLB_MSG];
	if (nodeoffset_curr != -FDT_ERR_NOTFOUND) {
		if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "dbg_msg_lvl", &tmp_u32, 1) == E_OK) {
			sie_fb_dbg_lvl = (BOOL)tmp_u32;
		} else {
			sie_fb_dbg_lvl = SIE_FB_DBG_LVL_WRN;
		}

		if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "dbg_func_en", &tmp_u32, 1) != E_OK) {
			sie_fb_dbg_ind("-------sie dbg_func_en disable------\r\n");
		} else {
			sie_fb_dbg_ind("-------sie dbg_func_en %d------\r\n", tmp_u32);
			if(tmp_u32 & KDRV_SIE_BUILTIN_FUNC_ALGO_BY_PASS){
				sie_builtin_dbg_info.isp_bypass_en = 1;
				sie_fb_dbg_ind("-------sie dbg: skip isp callback enable ------\r\n");
			}

			if(tmp_u32 & KDRV_SIE_BUILTIN_FUNC_CSI_PAT_GEN) {
#if defined(__FREERTOS)
				//csi1 pattern gen
				*(volatile UINT32 *)(0x2f0280090) = 0x01000051;	//h_v increase
				//csi2 pattern gen
				*(volatile UINT32 *)(0x2f0330090) = 0x01000041;	//h increase
				sie_fb_dbg_ind("-------sie dbg: csi1/csi2 pattern enable ------\r\n");
#endif
			}

			// check for sie output 2 frame only for first two image debug
			if(tmp_u32 & KDRV_SIE_BUILTIN_FUNC_OUT_TWO_FRAME) {
				sie_builtin_dbg_info.out_two_frame_only = TRUE;
				sie_fb_dbg_ind("-------sie dbg: output 2 frame only enable ------\r\n");
			}

			// check for sie output 2 frame only for first two image debug
			if(tmp_u32 & KDRV_SIE_BUILTIN_PUSH_TS) {
				sie_builtin_dbg_info.push_ts_print = TRUE;
				sie_fb_dbg_ind("-------sie dbg: dump push out timestamp enable ------\r\n");
			}
		}
	}

	vos_perf_mark(&ts);
#if defined(__FREERTOS)
	if (sie_builtin_dbg_info.push_ts_print == TRUE) {
		DBG_DUMP("\r\n------------------- rtos sie built_in init (ts %d) -------------------\r\n\r\n", ts);
	} else {
		sie_fb_dbg_ind("\r\n------------------- rtos sie built_in init (ts %d) -------------------\r\n\r\n", ts);
	}
	sie_builtin_rtos_init();
	sie_builtin_module_init();
#else
	if (sie_builtin_dbg_info.push_ts_print == TRUE) {
		DBG_DUMP("\r\n------------------- linux sie built_in init (ts %d) -------------------\r\n\r\n", ts);
	} else {
		sie_fb_dbg_ind("\r\n------------------- linux sie built_in init (ts %d) -------------------\r\n\r\n", ts);
	}
#endif

#if SIE_SIM_BUF_CB
	if (sie_fb_reg_buf_out_cb(sie_fb_buf_out_cb_sim_common) != E_OK) {
		DBG_ERR("sie_fb_reg_buf_out_cb fail\r\n");
	}
#endif

#if defined(__KERNEL__)
	kdrv_bridge_get_tag(SIE_FC_ID_1, &sie_builtin_info[SIE_FB_ID_1].bridge_info.frame_cnt);
	kdrv_bridge_get_tag(SIE_TS_ID_1, &sie_builtin_info[SIE_FB_ID_1].bridge_info.timestamp);
	//LSB
	kdrv_bridge_get_tag(SIE_ID_1_ADDR_0, &bridge_val);
	sie_builtin_info[SIE_FB_ID_1].bridge_info.buf_addr_0 = bridge_val;
	kdrv_bridge_get_tag(SIE_ID_1_ADDR_1, &bridge_val);
	sie_builtin_info[SIE_FB_ID_1].bridge_info.buf_addr_1 = bridge_val;
	//MSB
	kdrv_bridge_get_tag(SIE_ID_1_ADDR_2, &bridge_val);
	sie_builtin_info[SIE_FB_ID_1].bridge_info.buf_addr_0 |= ((ULONG)bridge_val<<32);
	kdrv_bridge_get_tag(SIE_ID_1_ADDR_3, &bridge_val);
	sie_builtin_info[SIE_FB_ID_1].bridge_info.buf_addr_1 |= ((ULONG)bridge_val<<32);

	kdrv_bridge_get_tag(SIE_FC_ID_2, &sie_builtin_info[SIE_FB_ID_2].bridge_info.frame_cnt);
	kdrv_bridge_get_tag(SIE_TS_ID_2, &sie_builtin_info[SIE_FB_ID_2].bridge_info.timestamp);
	//LSB
	kdrv_bridge_get_tag(SIE_ID_2_ADDR_0, &bridge_val);
	sie_builtin_info[SIE_FB_ID_2].bridge_info.buf_addr_0 = bridge_val;
	kdrv_bridge_get_tag(SIE_ID_2_ADDR_1, &bridge_val);
	sie_builtin_info[SIE_FB_ID_2].bridge_info.buf_addr_1 = bridge_val;
	//MSB
	kdrv_bridge_get_tag(SIE_ID_2_ADDR_2, &bridge_val);
	sie_builtin_info[SIE_FB_ID_2].bridge_info.buf_addr_0 |= ((ULONG)bridge_val<<32);
	kdrv_bridge_get_tag(SIE_ID_2_ADDR_3, &bridge_val);
	sie_builtin_info[SIE_FB_ID_2].bridge_info.buf_addr_1 |= ((ULONG)bridge_val<<32);

	kdrv_bridge_get_tag(SIE_FC_ID_3, &sie_builtin_info[SIE_FB_ID_3].bridge_info.frame_cnt);
	kdrv_bridge_get_tag(SIE_TS_ID_3, &sie_builtin_info[SIE_FB_ID_3].bridge_info.timestamp);
	//LSB
	kdrv_bridge_get_tag(SIE_ID_3_ADDR_0, &bridge_val);
	sie_builtin_info[SIE_FB_ID_3].bridge_info.buf_addr_0 = bridge_val;
	kdrv_bridge_get_tag(SIE_ID_3_ADDR_1, &bridge_val);
	sie_builtin_info[SIE_FB_ID_3].bridge_info.buf_addr_1 = bridge_val;
	//MSB
	kdrv_bridge_get_tag(SIE_ID_3_ADDR_2, &bridge_val);
	sie_builtin_info[SIE_FB_ID_3].bridge_info.buf_addr_0 |= ((ULONG)bridge_val<<32);
	kdrv_bridge_get_tag(SIE_ID_3_ADDR_3, &bridge_val);
	sie_builtin_info[SIE_FB_ID_3].bridge_info.buf_addr_1 |= ((ULONG)bridge_val<<32);

	kdrv_bridge_get_tag(SIE_FC_ID_4, &sie_builtin_info[SIE_FB_ID_4].bridge_info.frame_cnt);
	kdrv_bridge_get_tag(SIE_TS_ID_4, &sie_builtin_info[SIE_FB_ID_4].bridge_info.timestamp);
	//LSB
	kdrv_bridge_get_tag(SIE_ID_4_ADDR_0, &bridge_val);
	sie_builtin_info[SIE_FB_ID_4].bridge_info.buf_addr_0 = bridge_val;
	kdrv_bridge_get_tag(SIE_ID_4_ADDR_1, &bridge_val);
	sie_builtin_info[SIE_FB_ID_4].bridge_info.buf_addr_1 = bridge_val;
	//MSB
	kdrv_bridge_get_tag(SIE_ID_4_ADDR_2, &bridge_val);
	sie_builtin_info[SIE_FB_ID_4].bridge_info.buf_addr_0 |= ((ULONG)bridge_val<<32);
	kdrv_bridge_get_tag(SIE_ID_4_ADDR_3, &bridge_val);
	sie_builtin_info[SIE_FB_ID_4].bridge_info.buf_addr_1 |= ((ULONG)bridge_val<<32);

	kdrv_bridge_get_tag(SIE_FC_ID_5, &sie_builtin_info[SIE_FB_ID_5].bridge_info.frame_cnt);
	kdrv_bridge_get_tag(SIE_TS_ID_5, &sie_builtin_info[SIE_FB_ID_5].bridge_info.timestamp);
	//LSB
	kdrv_bridge_get_tag(SIE_ID_5_ADDR_0, &bridge_val);
	sie_builtin_info[SIE_FB_ID_5].bridge_info.buf_addr_0 = bridge_val;
	kdrv_bridge_get_tag(SIE_ID_5_ADDR_1, &bridge_val);
	sie_builtin_info[SIE_FB_ID_5].bridge_info.buf_addr_1 = bridge_val;
	//MSB
	kdrv_bridge_get_tag(SIE_ID_5_ADDR_2, &bridge_val);
	sie_builtin_info[SIE_FB_ID_5].bridge_info.buf_addr_0 |= ((ULONG)bridge_val<<32);
	kdrv_bridge_get_tag(SIE_ID_5_ADDR_3, &bridge_val);
	sie_builtin_info[SIE_FB_ID_5].bridge_info.buf_addr_1 |= ((ULONG)bridge_val<<32);

	sie_fb_dbg_ind("sie bridge info:\r\n id0: fc %8d, ts %d, addr 0x%lx, 0x%lx \r\n id1: fc %8d, ts %d, addr 0x%lx, 0x%lx \r\n id2: fc %8d, ts %d, addr 0x%lx, 0x%lx \r\n id3: fc %8d, ts %d, addr 0x%lx, 0x%lx \r\n id4: fc %8d, ts %d, addr 0x%lx, 0x%lx \r\n\r\n",
		sie_builtin_info[SIE_FB_ID_1].bridge_info.frame_cnt, sie_builtin_info[SIE_FB_ID_1].bridge_info.timestamp, sie_builtin_info[SIE_FB_ID_1].bridge_info.buf_addr_0, sie_builtin_info[SIE_FB_ID_1].bridge_info.buf_addr_1,
		sie_builtin_info[SIE_FB_ID_2].bridge_info.frame_cnt, sie_builtin_info[SIE_FB_ID_2].bridge_info.timestamp, sie_builtin_info[SIE_FB_ID_2].bridge_info.buf_addr_0, sie_builtin_info[SIE_FB_ID_2].bridge_info.buf_addr_1,
		sie_builtin_info[SIE_FB_ID_3].bridge_info.frame_cnt, sie_builtin_info[SIE_FB_ID_3].bridge_info.timestamp, sie_builtin_info[SIE_FB_ID_3].bridge_info.buf_addr_0, sie_builtin_info[SIE_FB_ID_3].bridge_info.buf_addr_1,
		sie_builtin_info[SIE_FB_ID_4].bridge_info.frame_cnt, sie_builtin_info[SIE_FB_ID_4].bridge_info.timestamp, sie_builtin_info[SIE_FB_ID_4].bridge_info.buf_addr_0, sie_builtin_info[SIE_FB_ID_4].bridge_info.buf_addr_1,
		sie_builtin_info[SIE_FB_ID_5].bridge_info.frame_cnt, sie_builtin_info[SIE_FB_ID_5].bridge_info.timestamp, sie_builtin_info[SIE_FB_ID_5].bridge_info.buf_addr_0, sie_builtin_info[SIE_FB_ID_5].bridge_info.buf_addr_1
	);

	//linux streaming id (skip rtos streaming id)
	streaming_id_bit = info->sie_id_bit & (~info->rtos_streaming_id_bit);
#else
	//rtos streaming id
	streaming_id_bit = info->rtos_streaming_id_bit;
#endif


	sie_fb_dbg_ind("sie_fb_id = 0x%x, rtos_stream_id = 0x%x, stream_id = 0x%x\r\n", (unsigned int)info->sie_id_bit, (unsigned int)info->rtos_streaming_id_bit, (unsigned int)streaming_id_bit);
	// update sie_fb_id_bit
	sie_fb_id_bit |= (info->sie_id_bit); // main sie id
	for (id = 0; id < SIE_MAX_ENG_NUM; id++) {
		nodeoffset_curr = nodeoffset_list[id][SIE_INIT_NODE_CTRL];
		if (nodeoffset_curr != -FDT_ERR_NOTFOUND) {
			sie_builtin_info[id].b_start = TRUE;
			sie_builtin_info[id].b_ref_load_id_vd = TRUE;
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "sen_out_dest", &sie_builtin_info[id].sen_out_dest, 1) != E_OK) {
				sie_builtin_info[id].sen_out_dest = 1<<id;
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "shdr_en", &sie_builtin_info[id].shdr_en, 1) != E_OK) {
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "low_latency", &sie_builtin_info[id].low_latency_en, 1) != E_OK) {
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "one_buf_en", &sie_builtin_info[id].one_buf_en, 1) != E_OK) {
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "dupl_src_id", &sie_builtin_info[id].dupl_src_id, 1) != E_OK) {
				DBG_ERR("Failed to read SIE %d dupl_src_id\r\n", (int)id + 1);
				return E_SYS;
			} else {
				//set total fb id
				if ((sie_fb_id_bit) & (1 << sie_builtin_info[id].dupl_src_id)) {
					sie_fb_id_bit |= (1 << id); 	// duplicate sie id
				}
				//set rtos or linux fb id
				if ((streaming_id_bit) & (1 << sie_builtin_info[id].dupl_src_id)) {
					streaming_id_bit |= (1 << id);	// duplicate sie id
				}
			}
			sie_fb_dbg_ind("sie_fb_id = 0x%x, rtos_stream_id = 0x%x, stream_id = 0x%x\r\n", (unsigned int)info->sie_id_bit, (unsigned int)info->rtos_streaming_id_bit, (unsigned int)streaming_id_bit);

			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "ref_load_id", &sie_builtin_info[id].ref_load_id, 1) != E_OK) {
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "act_x", &tmp_u32, 1) == E_OK) {
				sie_fb_iosize[id].act_win.x = tmp_u32;
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "act_y", &tmp_u32, 1) == E_OK) {
				sie_fb_iosize[id].act_win.y = tmp_u32;
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "act_w", &tmp_u32, 1) == E_OK) {
				sie_fb_iosize[id].act_win.w = tmp_u32;
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "act_h", &tmp_u32, 1) == E_OK) {
				sie_fb_iosize[id].act_win.h = tmp_u32;
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "crp_x", &tmp_u32, 1) == E_OK) {
				sie_fb_iosize[id].crp_win.x = tmp_u32;
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "crp_y", &tmp_u32, 1) == E_OK) {
				sie_fb_iosize[id].crp_win.y = tmp_u32;
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "crp_w", &tmp_u32, 1) == E_OK) {
				sie_fb_iosize[id].crp_win.w = tmp_u32;
			}
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "crp_h", &tmp_u32, 1) == E_OK) {
				sie_fb_iosize[id].crp_win.h = tmp_u32;
			}
			sie_fb_dbg_ind("id %d, act_win(%d, %d, %d, %d), crp_win(%d, %d, %d, %d)\r\n", id,
			sie_fb_iosize[id].act_win.x,sie_fb_iosize[id].act_win.y,sie_fb_iosize[id].act_win.w,sie_fb_iosize[id].act_win.h,
			sie_fb_iosize[id].crp_win.x,sie_fb_iosize[id].crp_win.y,sie_fb_iosize[id].crp_win.w,sie_fb_iosize[id].crp_win.h);
		}
	}

	for (id = 0; id < SIE_MAX_ENG_NUM; id++) {
		if (sie_fb_id_chk(id)) {
			sie_fb_dbg_ind("config sie id %d\r\n", id);
			if (sie_fb_set_ppb(id, info) != E_OK) {
				DBG_ERR("Failed to set SIE %d PPB\r\n", (int)id);
				return E_SYS;
			}

			reg_value = (UINT32 *)kdrv_sie_builtin_plat_malloc(SIE_ENG_REG_NUM * 2 * sizeof(UINT32)); // 2 : addr & value

			if (reg_value == NULL) {
				DBG_ERR("NULL for reg_value\r\n");
				return E_SYS;
			}

			nodeoffset_curr = nodeoffset_list[id][SIE_INIT_NODE_REG];
			if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "reg_start", &reg_value[0], SIE_ENG_REG_NUM * 2) != E_OK) {
				DBG_ERR("Failed to read sie reg\r\n");
				kdrv_sie_builtin_plat_free((void *)reg_value);
				return E_SYS;
			}
			sie_fb_dbg_ind("id %d, reg_num %d, stream_bit %x\r\n", id, SIE_ENG_REG_NUM, streaming_id_bit);

			if ((sie_builtin_info[id].ssdrv_hdl = sie_eng_get_handle(KDRV_CHIP0, id)) == NULL) {
				return E_SYS;
			}

			if ((1<<id) & streaming_id_bit) {
				sie_builtin_info[id].b_start = FALSE;
				sie_builtin_info[id].b_ref_load_id_vd = FALSE;
				/* ssdrv - open */
				if (sie_eng_open(sie_builtin_info[id].ssdrv_hdl) != 0) {
					DBG_ERR("id %d ssdrv fail\r\n", id);
					return E_SYS;
				}

				nodeoffset_curr = nodeoffset_list[id][SIE_INIT_NODE_CLK];
				if (nodeoffset_curr != -FDT_ERR_NOTFOUND) {
					if (sie_init_plat_read_dtsi_array(nodeoffset_curr, "intclk_src", &sie_builtin_info[id].int_clk_src, 1) == E_OK) {
						sie_eng_set_intclock(sie_builtin_info[id].ssdrv_hdl, sie_builtin_info[id].int_clk_src);
						sie_fb_dbg_ind("config sie %d int clk src %d\r\n", id, sie_builtin_info[id].int_clk_src);
					}
				}

				/* ssdrv - open */
#if defined(__KERNEL__)
				if (sie_eng_fastboot_open(sie_builtin_info[id].ssdrv_hdl) != 0) {
					DBG_ERR("id %d ssdrv fail\r\n", id);
					return E_SYS;
				}
#endif

				for (i = 0; i <= SIE_ENG_REG_NUM; i++) {
					if (reg_value[i * 2] == SIE_FB_CTL_OFS) {
						reg_value[i * 2 + 1] &= ~SIE_FB_CTL_ACT_EN_BIT;
						reg_value[i * 2 + 1] &= ~SIE_FB_CTL_LOAD_BIT;
						reg_value[i * 2 + 1] |= SIE_FB_CTL_RST_BIT;
					}
					//enable VD/CROP_END interrupt enable for flow ctrl
					if (reg_value[i * 2] == SIE_FB_INTE_OFS) {
						reg_value[i * 2 + 1] |= SIE_FB_INTE_DFT_BIT;
					}
					//disable iq function
					if (reg_value[i * 2] == SIE_FB_FUNC_OFS) {
						reg_value[i * 2 + 1] &= ~SIE_FB_FUNC_IQ_OFF_BIT;
					}
					sie_eng_write_hw_reg(sie_builtin_info[id].ssdrv_hdl, reg_value[i * 2], reg_value[i * 2 + 1]);
				}
				sie_eng_get_bsh_buf_reg(sie_builtin_info[id].ssdrv_hdl, &bs_h);
				sie_builtin_info[id].out_size.w = bs_h.out_sz;
				sie_eng_get_bsv_buf_reg(sie_builtin_info[id].ssdrv_hdl, &bs_v);
				sie_builtin_info[id].out_size.h = bs_v.out_sz;

				if (bs_h.out_sz == 0 || bs_v.out_sz == 0) {
					sie_eng_get_crop_win_buf_reg(sie_builtin_info[id].ssdrv_hdl, &crp_win);
					if (bs_h.out_sz == 0) {
						sie_builtin_info[id].out_size.w = crp_win.sz_x;
					}
					if (bs_v.out_sz == 0) {
						sie_builtin_info[id].out_size.h = crp_win.sz_y;
					}
					sie_fb_dbg_ind("cur crp win (%d, %d, %d, %d)\r\n", crp_win.st_x, crp_win.st_y, crp_win.sz_x, crp_win.sz_y);
				}
			}

			kdrv_sie_builtin_plat_free((void *)reg_value);
			sie_builtin_info[id].fb_streaming = TRUE;
			if ((1<<id) & streaming_id_bit) {
				// update iosize
//				sie_eng_set_load_hw_reg(sie_builtin_info[id].ssdrv_hdl);
				sie_fb_set_iosize(id, sie_fb_rt_chg_size[id]);

				//reset flg for isp
				if (sie_fb_int_cb) {
					sie_fb_int_cb(id, SIE_INT_ALL);
				}
			}

			//set first frame output address
#if defined(__KERNEL__)
			if (sie_builtin_info[id].bridge_info.frame_cnt != 0) {
				sie_fb_rdy_addr[id][SIE_FB_OUT_BASE][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_BASE][sie_builtin_info[id].bridge_info.frame_cnt % SIE_FB_RING_BUF_NUM];
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH0][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH0][sie_builtin_info[id].bridge_info.frame_cnt % SIE_FB_RING_BUF_NUM];
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH1][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH1][sie_builtin_info[id].bridge_info.frame_cnt % SIE_FB_RING_BUF_NUM];
				sie_fb_rdy_addr[id][SIE_FB_OUT_CH2][SIE_FB_BUF_IDX_CUR_OUT] = sie_fb_ppb[id][SIE_FB_OUT_CH2][sie_builtin_info[id].bridge_info.frame_cnt % SIE_FB_RING_BUF_NUM];

				sie_fb_vd_ts[id] = sie_builtin_info[id].bridge_info.timestamp;
				sie_builtin_info[id].frame_count = sie_builtin_info[id].bridge_info.frame_cnt+1;//cnt+1 for next vd update cur addr
			} else {
				for (ch = 0; ch < SIE_FB_OUT_CH_MAX; ch++) {
					if (sie_builtin_fp_set_singleouten[ch] != NULL) {
						sie_builtin_fp_set_singleouten[ch](sie_builtin_info[id].ssdrv_hdl, sie_builtin_info[id].sin_out_en[ch]);
					}
					if (sie_builtin_fp_set_dramoutmode[ch] != NULL && sie_fb_ppb[id][ch][0].pa != 0) {
						addr[ch].addr_pa = sie_fb_ppb[id][ch][0].pa;
						sie_builtin_fp_set_dramoutmode[ch](sie_builtin_info[id].ssdrv_hdl, &addr[ch]);
					}
				}
			}
#else
			for (ch = 0; ch < SIE_FB_OUT_CH_MAX; ch++) {
				if (sie_builtin_fp_set_singleouten[ch] != NULL) {
					sie_builtin_fp_set_singleouten[ch](sie_builtin_info[id].ssdrv_hdl, sie_builtin_info[id].sin_out_en[ch]);
				}

				if (sie_builtin_fp_set_dramoutmode[ch] != NULL && sie_fb_ppb[id][ch][0].pa != 0) {
					addr[ch].addr_pa = sie_fb_ppb[id][ch][0].pa;
					sie_builtin_fp_set_dramoutmode[ch](sie_builtin_info[id].ssdrv_hdl, &addr[ch]);
				}
			}
#endif
			sie_fb_dump_init_info(id);
		}
	}

	//sie start flow for 690/538/567: NA51107-3287
	for (id = 0; id < SIE_MAX_ENG_NUM; id++) {
		nodeoffset_curr = nodeoffset_list[id][SIE_INIT_NODE_REG];
		if ((nodeoffset_curr != -FDT_ERR_NOTFOUND) && sie_fb_id_chk(id)) {
			sie_eng_reg_isr_callback(sie_builtin_info[id].ssdrv_hdl, sie_isr_fp[id]);
			if ((1<<(id)) & streaming_id_bit) {
				sie_fb_dbg_ind("trigger start sie id %d, out_dest %d\r\n", id, sie_builtin_info[id].out_dest);
				sie_fb_dbg_ind("step1: set sie id %d, ref load disable\r\n", id);
				//step1: set ssdrv global src to disable
				sie_eng_set_global_load_src_hw_reg(sie_builtin_info[id].ssdrv_hdl, SIE_LOAD_DISABLE);
				/*
				step2:
					2-1: set act_en to 0
					2-2: reset to 0
				*/
				sie_fb_dbg_ind("step2: set sie id %d, act_and reset to 0\r\n", id);
				sie_eng_set_act_enable_buf_reg(sie_builtin_info[id].ssdrv_hdl, 0);
				ssdrv_trig_param.act_en = DISABLE;
				ssdrv_trig_param.act2_en = DISABLE;
				sie_eng_trig_hw_reg(sie_builtin_info[id].ssdrv_hdl, &ssdrv_trig_param);

				if (sie_builtin_info[id].ref_load_id != id) {
					//step3: set global load back for all SIE != global load src
					sie_fb_dbg_ind("step3: set sie id %d, ref load idx %d\r\n", id, sie_fb_conv2_sie_ssdrv_load_src(id));
					sie_eng_set_global_load_src_hw_reg(sie_builtin_info[id].ssdrv_hdl, sie_fb_conv2_sie_ssdrv_load_src(id));
					//step4: set active enable for all SIE != global load src
					sie_fb_dbg_ind("step4: trigger start sie id %d, ctl reg 0x%x, func 0x%x\r\n", id,
					sie_eng_read_hw_reg(sie_builtin_info[id].ssdrv_hdl, 0),
					sie_eng_read_hw_reg(sie_builtin_info[id].ssdrv_hdl, 4));
					ssdrv_trig_param.act_en = ENABLE;
					ssdrv_trig_param.act2_en = ENABLE;
					sie_eng_trig_hw_reg(sie_builtin_info[id].ssdrv_hdl, &ssdrv_trig_param);
					sie_builtin_info[id].b_start = TRUE;
				}
			}
		}
	}

	//step5: set act enable for global load src id
	for (id = 0; id < SIE_MAX_ENG_NUM; id++) {
		nodeoffset_curr = nodeoffset_list[id][SIE_INIT_NODE_REG];
		if ((nodeoffset_curr != -FDT_ERR_NOTFOUND) && sie_fb_id_chk(id)) {
			if ((1<<(id)) & streaming_id_bit) {
				if (sie_builtin_info[id].ref_load_id == id) {
					sie_fb_dbg_ind("step5: trigger start sie id %d, ctl reg 0x%x, func 0x%x\r\n", id,
					sie_eng_read_hw_reg(sie_builtin_info[id].ssdrv_hdl, 0),
					sie_eng_read_hw_reg(sie_builtin_info[id].ssdrv_hdl, 4));
					ssdrv_trig_param.act_en = ENABLE;
					ssdrv_trig_param.act2_en = ENABLE;
					sie_eng_trig_hw_reg(sie_builtin_info[id].ssdrv_hdl, &ssdrv_trig_param);
					sie_builtin_info[id].b_start = TRUE;
				}
				sie_eng_set_load_hw_reg(sie_builtin_info[id].ssdrv_hdl);
			}
		}
	}
	return E_OK;
}

ER sie_fb_reg_kdrv_isr_Cb(UINT32 id, SIE_FB_KDRV_ISR_FP fp)
{
	if (fp) {
		sie_builtin_info[id].kdrv_cb = fp;
		return E_OK;
	}
	return E_NOEXS;
}

ER sie_fb_reg_isr_Cb(SIE_FB_ISR_FP fp)
{
	if (fp) {
		sie_fb_int_cb = fp;
		return E_OK;
	}
	return E_NOEXS;
}

ER sie_fb_reg_buf_out_cb(SIE_FB_BUF_OUT_FP fp)
{
	if (fp) {
		sie_fb_buf_out_cb = fp;
		return E_OK;
	}
	return E_NOEXS;
}


UINT32 sie_set_dbg_func(UINT32 id, UINT32 dbg_func_en, BOOL en){

	sie_fb_dbg_ind("--set dbg_func: %x,%x,%x--\n",id,dbg_func_en,en);

	if(en == ENABLE){
		if (dbg_func_en & KDRV_SIE_BUILTIN_FUNC_ALGO_BY_PASS) {
			sie_builtin_dbg_info.isp_bypass_en = 1;
		}
	}else{
		if (dbg_func_en & KDRV_SIE_BUILTIN_FUNC_ALGO_BY_PASS) {
			sie_builtin_dbg_info.isp_bypass_en = 0;
		}
	}

	return 0;
}

void sie_fb_get_bridge_info(UINT32 id, SIE_FB_BRIDGE_INFO *brg_info)
{
	brg_info->frame_cnt = sie_builtin_info[id].bridge_info.frame_cnt;
	brg_info->timestamp = sie_builtin_info[id].bridge_info.timestamp;
	brg_info->buf_addr_0 = sie_builtin_info[id].bridge_info.buf_addr_0;
	brg_info->buf_addr_1 = sie_builtin_info[id].bridge_info.buf_addr_1;

	sie_fb_dbg_ind("id %d get bridge info fc %d, ts %d, addr(0x%lx, 0x%lx)\n",id, brg_info->frame_cnt, brg_info->timestamp, brg_info->buf_addr_0, brg_info->buf_addr_1);
	return;
}

ER sie_builtin_set_start(UINT32 id)
{
	UINT32 i = 0;
	SIE_START_CFG ssdrv_trig_param = {0};
	UINT32 ch;
	ULONG ch_out_paddr;
	SIE_DRAM_ADDR addr[SIE_FB_OUT_CH_MAX] = {0};

	sie_fb_dbg_ind("set trigger start id %d\r\n", id);
	//sie start flow for 690/538/567: NA51107-3287
	for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
		if ((i == id && sie_builtin_info[i].b_start == FALSE) || (sie_builtin_info[id].ref_load_id == i)) {
			//update first frame address
			for (ch = 0; ch < SIE_FB_OUT_CH_MAX; ch++) {
				if (sie_builtin_fp_set_dramoutmode[ch] != NULL) {
					ch_out_paddr = sie_fb_ppb[i][ch][sie_builtin_info[i].frame_count % SIE_FB_RING_BUF_NUM].pa;
					if (ch_out_paddr != 0) {
						addr[ch].addr_pa = ch_out_paddr;
						sie_builtin_fp_set_dramoutmode[ch](sie_builtin_info[i].ssdrv_hdl, &addr[ch]);
					}
				}
				if (sie_builtin_fp_set_singleouten[ch] != NULL) {
					sie_builtin_fp_set_singleouten[ch](sie_builtin_info[i].ssdrv_hdl, sie_builtin_info[i].sin_out_en[ch]);
				}
			}
			sie_eng_set_load_hw_reg(sie_builtin_info[i].ssdrv_hdl);
			if (sie_fb_dbg_dump_cnt[i] <= sie_fb_dbg_dump_max_cnt) {
				sie_fb_dbg_ind("[FB] start: id: %d, set_adr: 0x%lx, 0x%lx, 0x%lx\r\n",
								i, addr[SIE_FB_OUT_CH0].addr_pa, addr[SIE_FB_OUT_CH1].addr_pa, addr[SIE_FB_OUT_CH2].addr_pa);
			}

			sie_fb_dbg_ind("trigger start sie id %d, out_dest %d\r\n", i, sie_builtin_info[i].out_dest);
			sie_fb_dbg_ind("step1: set sie id %d, ref load disable\r\n", i);
			//step1: set ssdrv global src to disable
			sie_eng_set_global_load_src_hw_reg(sie_builtin_info[i].ssdrv_hdl, SIE_LOAD_DISABLE);
			/*
			step2:
				2-1: set act_en to 0
				2-2: reset to 0
			*/
			sie_fb_dbg_ind("step2: set sie id %d, act_and reset to 0\r\n", i);
			sie_eng_set_act_enable_buf_reg(sie_builtin_info[i].ssdrv_hdl, 0);
			ssdrv_trig_param.act_en = DISABLE;
			ssdrv_trig_param.act2_en = DISABLE;
			sie_eng_trig_hw_reg(sie_builtin_info[i].ssdrv_hdl, &ssdrv_trig_param);

			if (sie_builtin_info[i].ref_load_id != i) {
				//step3: set global load back for all SIE != global load src
				sie_fb_dbg_ind("step3: set sie id %d, ref load idx %d\r\n", i, sie_fb_conv2_sie_ssdrv_load_src(i));
				sie_eng_set_global_load_src_hw_reg(sie_builtin_info[i].ssdrv_hdl, sie_fb_conv2_sie_ssdrv_load_src(i));
				//step4: set active enable for all SIE != global load src
				sie_fb_dbg_ind("step4: trigger start sie id %d, ctl reg 0x%x, func 0x%x\r\n", i,
				sie_eng_read_hw_reg(sie_builtin_info[i].ssdrv_hdl, 0),
				sie_eng_read_hw_reg(sie_builtin_info[i].ssdrv_hdl, 4));
				ssdrv_trig_param.act_en = ENABLE;
				ssdrv_trig_param.act2_en = ENABLE;
				sie_eng_trig_hw_reg(sie_builtin_info[i].ssdrv_hdl, &ssdrv_trig_param);
				sie_builtin_info[i].b_start = TRUE;
			}
		}
	}

	//step5: set act enable for global load src id
	for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
		if ((i == id) || (sie_builtin_info[id].ref_load_id == i)) {
			if (sie_builtin_info[i].ref_load_id == i) {
				sie_fb_dbg_ind("step5: trigger start sie id %d, ctl reg 0x%x, func 0x%x\r\n", i,
				sie_eng_read_hw_reg(sie_builtin_info[i].ssdrv_hdl, 0),
				sie_eng_read_hw_reg(sie_builtin_info[i].ssdrv_hdl, 4));
				ssdrv_trig_param.act_en = ENABLE;
				ssdrv_trig_param.act2_en = ENABLE;
				sie_eng_trig_hw_reg(sie_builtin_info[i].ssdrv_hdl, &ssdrv_trig_param);
				sie_builtin_info[i].b_start = TRUE;
			}
			sie_eng_set_load_hw_reg(sie_builtin_info[i].ssdrv_hdl);
		}
	}
	return E_OK;
}

ER sie_builtin_set_stop(UINT32 id)
{
	UINT32 i;

	sie_fb_dbg_ind("set trigger stop id %d\r\n", id);
	for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
		if ((i == id && sie_builtin_info[i].b_start == TRUE) || (sie_builtin_info[id].ref_load_id == i)) {
			sie_fb_dbg_ind("trigger stop sie %d\r\n", i);
			sie_eng_stop_hw_reg(sie_builtin_info[i].ssdrv_hdl);
			sie_builtin_info[i].b_start = FALSE;
			sie_fb_first_vd[i] = FALSE;
			sie_fb_dbg_dump_cnt[i] = 0;
		}
	}

	return E_OK;
}

void sie_fb_get_dbg_info(SIE_BUILTIN_DBG_INFO *dbg_info)
{
	*dbg_info = sie_builtin_dbg_info;
	return;
}
//@}
