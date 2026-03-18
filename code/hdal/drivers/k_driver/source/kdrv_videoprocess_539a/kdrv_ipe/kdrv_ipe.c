#if defined(__LINUX)
#elif defined(__FREERTOS)
#include <string.h>
#endif
#include "kwrap/util.h"
#include "comm/hwclock.h"
#include "kdrv_ipe_int.h"
#include "kdrv_ipp_builtin.h"
#include "drv_lmt.h"
#include "kdrv_ipp_dbg_log.h"
#include "ipp_eng.h"

#define KDRV_IME_ALLOC_ENG_REG_BUFFER (1)
#define IPE_ABS(x)	((x)>=0 ? (x):(-(x)))

extern KDRV_IPE_HANDLE* (*kdrv_ipe_int_get_handle)(UINT32 chip_id, UINT32 eng_id);

KDRV_IPE_CTL g_kdrv_ipe_ctl;

#define KDRV_IPE_ENG_BASE KDRV_VIDEOPROCS_IPE_ENGINE0



void kdrv_ipe_dump_info(void)
{
	UINT32 i;
	KDRV_IPE_HANDLE *handle;

	if ((g_kdrv_ipe_ctl.p_hdl == NULL) || (g_kdrv_ipe_ctl.total_ch == 0)) {
		DBG_ERR("error g_kdrv_ipe_ctl.p_hdl(0x%lx) total ch = %d\r\n", (ULONG)g_kdrv_ipe_ctl.p_hdl, g_kdrv_ipe_ctl.total_ch);
		return;
	}

	DBG_DUMP("===================================================================================\n");
	DBG_DUMP("eng chip_num = %d\n", g_kdrv_ipe_ctl.chip_num);
	DBG_DUMP("eng eng_num = %d\n", g_kdrv_ipe_ctl.eng_num);
	DBG_DUMP("eng total ch = %d\n", g_kdrv_ipe_ctl.total_ch);
	DBG_DUMP("===================================================================================\n");
	DBG_DUMP("chip        eng eng_handle   callback\n");

	for (i = 0; i < g_kdrv_ipe_ctl.total_ch; i ++) {
		handle = &g_kdrv_ipe_ctl.p_hdl[i];
		if (handle) {
			DBG_DUMP("%4d 0x%.8x 0x%.8lx 0x%.8lx\r\n", handle->chip_id, handle->eng_id, (ULONG)handle->p_eng, (ULONG)handle->cb);
		}
	}
}

static ULONG kdrv_ipe_int_get_eng_reg_buf_addr(KDRV_IPE_HANDLE *p_hdl)
{
	return (ULONG)p_hdl->p_eng_reg_buf;
}

static ULONG kdrv_ipe_int_get_eng_flg_buf_addr(KDRV_IPE_HANDLE *p_hdl)
{
	return (ULONG)p_hdl->p_eng_flg_buf;
}

UINT32 kdrv_ipe_isr_state_machine(KDRV_IPE_HANDLE *p_hdl, UINT32 status)
{
	/* return interrupt status that should be process first base on cur state */
	switch (p_hdl->isr_state) {
	case KDRV_IPE_ISR_STATE_UNKNOWN:
		if (status & KDRV_IPE_INTERRUPT_FMS) {
			p_hdl->isr_state = KDRV_IPE_ISR_STATE_START;
			return KDRV_IPE_INTERRUPT_FMS;
		}
		break;

	case KDRV_IPE_ISR_STATE_START:
		if (status & KDRV_IPE_INTERRUPT_FMD) {
			p_hdl->isr_state = KDRV_IPE_ISR_STATE_END;
			return KDRV_IPE_INTERRUPT_FMD;
		}

		if (status & KDRV_IPE_INTERRUPT_LL_DONE) {
			p_hdl->isr_state = KDRV_IPE_ISR_STATE_END;
			return KDRV_IPE_INTERRUPT_LL_DONE;
		}
		break;

	case KDRV_IPE_ISR_STATE_END:
		if (status & KDRV_IPE_INTERRUPT_FMS) {
			p_hdl->isr_state = KDRV_IPE_ISR_STATE_START;
			return KDRV_IPE_INTERRUPT_FMS;
		}
		break;

	default:
		DBG_ERR("unknown state %d\r\n", (int)p_hdl->isr_state);
		break;
	}

	return 0;
}

void kdrv_ipe_isr(void *eng, UINT32 status, void *reserve)
{
	IPE_ENG_HANDLE *p_eng;
	KDRV_IPE_HANDLE *p_hdl;
	UINT32 proc_sts;

	if (eng == NULL) {
		return ;
	}

	p_eng = (IPE_ENG_HANDLE *)eng;
	p_hdl = kdrv_ipe_int_get_handle(p_eng->chip_id, p_eng->eng_id);

	if (p_hdl == NULL) {
		return ;
	}

	if (p_hdl->cb == NULL) {
		return ;
	}

	if(status & KDRV_IPE_INTERRUPT_FMD){

		p_hdl->p_ipe_dbg_info->ipe_frm_ed_cnt++;
	}

	if(status & KDRV_IPE_INTERRUPT_FMS){
		p_hdl->p_ipe_dbg_info->ipe_frm_st_cnt++;
	}

	if(status & KDRV_IPE_INTERRUPT_STP){

		p_hdl->p_ipe_dbg_info->ipe_strp_done_cnt++;
	}

	if(status & KDRV_IPE_INTERRUPT_LL_DONE){
		p_hdl->p_ipe_dbg_info->ipe_ll_ed_cnt++;
	}

	if(status & KDRV_IPE_INTERRUPT_LL_ERR){

		p_hdl->p_ipe_dbg_info->ipe_ll_error_cnt++;
	}

	if(status & KDRV_IPE_INTERRUPT_LL_ERR2){

		p_hdl->p_ipe_dbg_info->ipe_ll_error2_cnt++;
	}

	if(status & KDRV_IPE_INTERRUPT_FRAMEERR){

		p_hdl->p_ipe_dbg_info->ipe_frame_err_cnt++;
	}

	if(status & KDRV_IPE_INTERRUPT_YCC_OUT_END){

		p_hdl->p_ipe_dbg_info->ipe_ycbcr_out_ed_cnt++;
	}

	if(status & KDRV_IPE_INTERRUPT_GAMMA_IN_END){

		p_hdl->p_ipe_dbg_info->ipe_load_gamma_ed_cnt++;
	}

	if(status & KDRV_IPE_INTERRUPT_DEFOG_SUBOUT_END){

		p_hdl->p_ipe_dbg_info->ipe_defog_out_ed_cnt++;
	}

#if(IPE_538_KDRV_FEATURE == 0)
	if(status & KDRV_IPE_INTERRUPT_LCE_SUBOUT_END){

		p_hdl->p_ipe_dbg_info->ipe_lce_out_ed_cnt++;
	}
#endif


	if(status & KDRV_IPE_INTERRUPT_CFA_SUBOUT_END){

		p_hdl->p_ipe_dbg_info->ipe_cfa_out_ed_cnt++;
	}


	if(status & KDRV_IPE_INTERRUPTT_VA_OUT_END){

		p_hdl->p_ipe_dbg_info->ipe_va_out_ed_cnt++;
	}

	if(status & KDRV_IPE_INTE_ENC_OUTOVFL){

		p_hdl->p_ipe_dbg_info->ipe_enc_out_ovfl_cnt++;
	}

	if(status & KDRV_IPE_INTE_MOTION_END){

		p_hdl->p_ipe_dbg_info->ipe_motion_end_cnt++;
	}

	while (status != 0) {
		proc_sts = kdrv_ipe_isr_state_machine(p_hdl, status);
		if (proc_sts) {
			status &= ~(proc_sts);
			p_hdl->cb(p_hdl->chip_id, p_hdl->eng_id, proc_sts, NULL, NULL);
		} else {
			// coverity[assigned_value]
			status = 0;
			break;
		}
	}
}

INT32 kdrv_ipe_sys_init(UINT32 chip_num, UINT32 eng_num)
{
	KDRV_IPE_HANDLE *p_hdl;
	UINT32 i;
	UINT32 want_size = 0;
	UINT32 total_ch;

	if (g_kdrv_ipe_ctl.p_hdl != NULL) {
		DBG_ERR("reinit\r\n");
		return -1;
	}

	total_ch = chip_num * eng_num;
	g_kdrv_ipe_ctl.p_hdl = kdrv_ipe_os_malloc_wrap(sizeof(KDRV_IPE_HANDLE) * total_ch);
	if (g_kdrv_ipe_ctl.p_hdl == NULL) {
		DBG_ERR("alloc buf(%lu) failed\r\n", (ULONG)(sizeof(KDRV_IPE_HANDLE) * total_ch));
		return -1;
	}

	/* init kdrv ctl */
	g_kdrv_ipe_ctl.chip_num = chip_num;
	g_kdrv_ipe_ctl.eng_num = eng_num;
	g_kdrv_ipe_ctl.total_ch = total_ch;

	/* init kdrv handle */
	for (i = 0; i < g_kdrv_ipe_ctl.total_ch; i++) {
		p_hdl = &g_kdrv_ipe_ctl.p_hdl[i];
		memset((void *)p_hdl, 0, sizeof(KDRV_IPE_HANDLE));
		p_hdl->chip_id = KDRV_CHIP0;
		p_hdl->eng_id = KDRV_VIDEOPROCS_IPE_ENGINE0 + i;
		vk_spin_lock_init(&p_hdl->lock);
		SEM_CREATE(p_hdl->sem, 1);

		/* init debug information structure*/
		p_hdl->p_ipe_dbg_info = kdrv_ipe_os_malloc_wrap(sizeof(KDRV_IPE_DBG_INFO));
		if (p_hdl->p_ipe_dbg_info == NULL) {
			DBG_ERR("alloc p_ipe_dbg_info failed\r\n");
			kdrv_ipe_sys_uninit();
			return -1;
		}
		memset((void *)p_hdl->p_ipe_dbg_info, 0, sizeof(KDRV_IPE_DBG_INFO));

		//suspend buff
		p_hdl->reg_data = kdrv_ipe_os_malloc_wrap(ipe_eng_get_reg_base_buf_size());
		if (p_hdl->reg_data == NULL) {
			DBG_ERR("ipe kmalloc reg_data buf failed\r\n");
			kdrv_ipe_sys_uninit();
			// coverity[leaked_storage]
			return -1;
		}

		/* init working buffer for ime_eng */
		#if IPE_SSDRV_SUPPORT
		want_size = ipe_eng_get_reg_flag_buf_size();
		#endif
		p_hdl->p_eng_flg_buf = kdrv_ipe_os_malloc_wrap(want_size);
		if (p_hdl->p_eng_flg_buf == NULL) {
			#if IPE_SSDRV_SUPPORT
			DBG_ERR("alloc eng flg working buf %d failed\r\n", ipe_eng_get_reg_flag_buf_size());
			#endif
			kdrv_ipe_sys_uninit();
			// coverity[leaked_storage]
			return -1;
		}

		#if KDRV_IME_ALLOC_ENG_REG_BUFFER
		#if IPE_SSDRV_SUPPORT
		want_size = ipe_eng_get_reg_base_buf_size();
		p_hdl->p_eng_reg_buf = kdrv_ipe_os_malloc_wrap(want_size);
		if (p_hdl->p_eng_reg_buf == NULL) {
			DBG_ERR("alloc eng reg working buf %d failed\r\n", ipe_eng_get_reg_base_buf_size());
			kdrv_ipe_sys_uninit();
			// coverity[leaked_storage]
			return -1;
		}
		#endif
		#else
		g_kdrv_ipe_ctl.p_eng_reg_buf = NULL;
		#endif
	}
	// coverity[leaked_storage]
	return 0;
}

INT32 kdrv_ipe_sys_uninit()
{
	KDRV_IPE_HANDLE *p_hdl;
	UINT32 i;

	if (g_kdrv_ipe_ctl.p_hdl) {
		for (i = 0; i < g_kdrv_ipe_ctl.total_ch; i++) {
			p_hdl = &g_kdrv_ipe_ctl.p_hdl[i];
			SEM_DESTROY(p_hdl->sem);

			if(p_hdl->p_ipe_dbg_info){
				kdrv_ipe_os_mfree_wrap(p_hdl->p_ipe_dbg_info);
				p_hdl->p_ipe_dbg_info = NULL;
			}

			//suspend buff
			if(p_hdl->reg_data) {
				kdrv_ipe_os_mfree_wrap(p_hdl->reg_data);
				p_hdl->reg_data = NULL;
			}

			if (p_hdl->p_eng_reg_buf) {
				kdrv_ipe_os_mfree_wrap(p_hdl->p_eng_reg_buf);
				p_hdl->p_eng_reg_buf = NULL;
			}

			if (p_hdl->p_eng_flg_buf) {
				kdrv_ipe_os_mfree_wrap(p_hdl->p_eng_flg_buf);
				p_hdl->p_eng_flg_buf = NULL;
			}

		}
		kdrv_ipe_os_mfree_wrap(g_kdrv_ipe_ctl.p_hdl);
		g_kdrv_ipe_ctl.p_hdl = NULL;
	}

	memset((void *)&g_kdrv_ipe_ctl, 0, sizeof(KDRV_IPE_CTL));
	return 0;
}

#if 0
#endif

INT32 kdrv_ipe_open(UINT32 chip, UINT32 engine)
{
	INT32 rt = 0;
	KDRV_IPE_HANDLE *p_hdl;

	p_hdl = kdrv_ipe_int_get_handle(chip, engine);
	if (p_hdl == NULL) {
		DBG_ERR("open failed, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_PAR;
	}

	if (0 /*SEM_WAIT_TIMEOUT(p_hdl->sem, vos_util_msec_to_tick(KDRV_IPE_TIMEOUT_MS))*/) {
		DBG_ERR("chip 0x%.8x, engine 0x%.8x, open timeout(%d ms)\r\n", chip, engine, KDRV_IPE_TIMEOUT_MS);
		rt = E_SYS;
	} else {
		#if IPE_SSDRV_SUPPORT
		p_hdl->isr_state = KDRV_IPE_ISR_STATE_UNKNOWN;

		p_hdl->p_eng = ipe_eng_get_handle(chip, engine);
		if (ipe_eng_open(p_hdl->p_eng) != E_OK) {
			DBG_ERR("ipe eng open fail\r\n");
		}
		ipe_eng_reg_isr_callback(p_hdl->p_eng, kdrv_ipe_isr);
		if (p_hdl->p_eng == NULL) {
			rt = E_SYS;
			SEM_SIGNAL(p_hdl->sem);
		} else {
			rt = E_OK;
		}
		#endif
	}

	return rt;
}

INT32 kdrv_ipe_close(UINT32 chip, UINT32 engine)
{
	KDRV_IPE_HANDLE *p_hdl;

	p_hdl = kdrv_ipe_int_get_handle(chip, engine);
	if (p_hdl == NULL) {
		DBG_ERR("close failed, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_PAR;
	}

	if (p_hdl->p_eng == NULL) {
		DBG_ERR("must open before close\r\n");
		return E_SYS;
	}
#if IPE_SSDRV_SUPPORT
	if (ipe_eng_close(p_hdl->p_eng) != E_OK) {
		DBG_ERR("ipe eng close fail\r\n");
	}
#endif
	p_hdl->p_eng = NULL;
	SEM_SIGNAL(p_hdl->sem);

	return 0;
}

#ifdef CONFIG_PM
INT32 kdrv_ipe_suspend(UINT32 chip, UINT32 engine)
{
	KDRV_IPE_HANDLE *p_hdl;
	UINT32 i;
	UINT32 reg_ofs;
	uintptr_t ipe_reg_io_base = 0;

	if (!g_kdrv_ipe_ctl.total_ch)
		return E_OK;

	p_hdl = kdrv_ipe_int_get_handle(chip, engine);


#if 0
	if (g_kdrv_ipe_ctl.p_hdl) {
		for (i = 0; i < g_kdrv_ipe_ctl.total_ch; i++) {
			p_hdl = &g_kdrv_ipe_ctl.p_hdl[i];
		}
	}
#endif

	if (p_hdl == NULL) {
		DBG_IND("ipe driver is not init, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_OK;
	}

	if (p_hdl->p_eng == NULL) {
		DBG_IND("ipe is init but not open\r\n");
		return E_OK;
	}

	if (p_hdl->reg_data == NULL) {
		DBG_ERR("HDL tempory register buffer is not init\r\n");
		return E_SYS;
	}

	ipe_reg_io_base = p_hdl->p_eng->reg_io_base;
	DBG_IND("ipe_reg_io_base_pa %lx for suspend\r\n", vos_cpu_get_phy_addr(ipe_reg_io_base));

	if (!ipe_reg_io_base) {
		DBG_ERR("ipe base addr get fail!\r\n");
		return E_SYS;
	}

	/* flush output buffer */
	vos_cpu_dcache_sync(ipe_reg_io_base, ipe_eng_get_reg_base_buf_size(), VOS_DMA_FROM_DEVICE);

	for (i = 0; i < IPE_ENG_REG_NUM; i++) {
		reg_ofs = i << 2;

		p_hdl->reg_data[i] = kdrv_ipe_eng_getreg(ipe_reg_io_base + reg_ofs);
	}

	if (!IS_ERR(p_hdl->p_eng->pclk)) {
		ipe_eng_platform_disable_clk(p_hdl->p_eng);
		ipe_eng_platform_unprepare_clk(p_hdl->p_eng);

	}

	if (clk_get_phase(p_hdl->p_eng->pclk)) {
		p_hdl->auto_gating = 1;
		clk_set_phase(p_hdl->p_eng->pclk, 0);
	} else
		p_hdl->auto_gating = 0;

#if 0
	#if defined(__FREERTOS)
		debug_dumpmem(p_hdl->p_eng->reg_io_base, ipe_eng_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
	#else
		debug_dumpmem(vos_cpu_get_phy_addr(p_hdl->p_eng->reg_io_base), ipe_eng_get_reg_base_buf_size());
	#endif
#endif
	return 0;
}

INT32 kdrv_ipe_resume(UINT32 chip, UINT32 engine)
{
	KDRV_IPE_HANDLE *p_hdl;
	UINT32 i;
	UINT32 reg_ofs;
	uintptr_t ipe_reg_io_base = 0;

	if (!g_kdrv_ipe_ctl.total_ch)
		return E_OK;

	p_hdl = kdrv_ipe_int_get_handle(chip, engine);

#if 0
	if (g_kdrv_ipe_ctl.p_hdl) {
		for (i = 0; i < g_kdrv_ipe_ctl.total_ch; i++) {
			p_hdl = &g_kdrv_ipe_ctl.p_hdl[i];
		}
	}
#endif

	if (p_hdl == NULL) {
		DBG_IND("ipe driver is not init, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_OK;
	}

	if (p_hdl->p_eng == NULL) {
		DBG_IND("ipe is init but not open\r\n");
		return E_OK;
	}

	if (p_hdl->reg_data == NULL) {
		DBG_ERR("HDL tempory register buffer is not init\r\n");
		return E_SYS;
	}


	//set clock
	if (!IS_ERR(p_hdl->p_eng->pclk)) {
		ipe_eng_platform_set_clk_rate(p_hdl->p_eng); //including set parent
		ipe_eng_platform_prepare_clk(p_hdl->p_eng);
		ipe_eng_platform_enable_clk(p_hdl->p_eng);
	}
	if (p_hdl->auto_gating) {
		clk_set_phase(p_hdl->p_eng->pclk, 1);
	}

	ipe_reg_io_base = p_hdl->p_eng->reg_io_base;
	DBG_IND("ipe_reg_io_base_pa %lx for resume\r\n", vos_cpu_get_phy_addr(ipe_reg_io_base));

	if (!ipe_reg_io_base) {
		DBG_ERR("ipe base addr get fail!\r\n");
		return E_SYS;
	}

	for (i = 0; i < IPE_ENG_REG_NUM; i++) {
		reg_ofs = i << 2;

		kdrv_ipe_eng_setreg(ipe_reg_io_base + reg_ofs, p_hdl->reg_data[i]);
	}

	/* flush input buffer */
	vos_cpu_dcache_sync(ipe_reg_io_base, ipe_eng_get_reg_base_buf_size(), VOS_DMA_TO_DEVICE);
#if 0
	#if defined(__FREERTOS)
			debug_dumpmem(p_hdl->p_eng->reg_io_base, ipe_eng_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
	#else
			debug_dumpmem(vos_cpu_get_phy_addr(p_hdl->p_eng->reg_io_base), ipe_eng_get_reg_base_buf_size());
	#endif
#endif
	return 0;
}
#endif

#if 0
#endif

#if IPE_SSDRV_SUPPORT
static IPE_ENG_OPMODE kdrv_ipe_typecast_opmode(KDRV_IPP_OPMODE mode)
{
	switch (mode) {
	case KDRV_IPP_OPMODE_D2D:
		return IPE_ENG_OPMODE_D2D;

	case KDRV_IPP_OPMODE_IFE2IME:
	case KDRV_IPP_OPMODE_DCE2IME:
	case KDRV_IPP_OPMODE_IFE2IPE:
	case KDRV_IPP_OPMODE_PRE2IME:
	case KDRV_IPP_OPMODE_PRE2IPE:
		return IPE_ENG_OPMODE_DIRECT;

	case KDRV_IPP_OPMODE_SIE2IME:
		return IPE_ENG_OPMODE_ALL_DIRECT;

	default:
		DBG_ERR("unknown mode %d \r\n", (int)mode);
		return -1;
	}
}

static IPE_ENG_FMT kdrv_ipe_typecast_io_fmt(KDRV_IPP_FMT fmt)
{
	switch (fmt) {
	case KDRV_IPP_FMT_YUV444:
		return IPE_ENG_FMT_Y_PACK_UV444;

	case KDRV_IPP_FMT_YUV422:
		return IPE_ENG_FMT_Y_PACK_UV422;

	case KDRV_IPP_FMT_YUV420:
		return IPE_ENG_FMT_Y_PACK_UV420;

	case KDRV_IPP_FMT_Y8	:
		return IPE_ENG_FMT_Y;

	default:
		DBG_ERR("unknown iofmt 0x%.8x\r\n", (unsigned int)fmt);
		return -1;
	}
}

static IPE_CC_OFSSEL kdrv_ipe_typecast_cc_ofssel(KDRV_IPP_FRAME in_frm)
{

	if (KDRV_IPP_FMT_CLASS(in_frm.fmt) != KDRV_IPP_FMT_CLASS_YUV) {
		return CC_OFS_BYPASS;
	}

	switch (in_frm.pxl_fmt) {
	case KDRV_IPP_PXLFMT_YCC_FULL:
		return CC_OFS_Y_FULL;

	case KDRV_IPP_PXLFMT_YCC_BT601:
	case KDRV_IPP_PXLFMT_YCC_BT709:
		return CC_OFS_Y_BTU;

	default:
		DBG_ERR("unknown format 0x%.8x\r\n", (UINT32)in_frm.pxl_fmt);
		return -1;
	}
}

static IPE_ENG_RAW_FMT kdrv_ipe_typecast_bayer(KDRV_IPP_FMT fmt)
{
	KDRV_IPP_FMT fmt_without_lsb = fmt & (~KDRV_IPP_FMT_PACK_LSB); // remove lsb bit to prevent format issue

	switch (fmt_without_lsb & KDRV_IPP_FMT_BAYER_TYPE_MASK) {
	case KDRV_IPP_FMT_RGGB:
		return IPE_ENG_BAYER_2X2;
	case KDRV_IPP_FMT_RGBIR:
	case KDRV_IPP_FMT_RGBIR44:
		return IPE_ENG_RGBIR_4X4;
	default:
		DBG_ERR("Unknown bayer format 0x%.8x\n",(unsigned int)fmt);
		return -1;
	}
}

static IPE_ENG_CFA_PAT kdrv_ipe_typecast_cfa(KDRV_IPP_FMT fmt)
{
	UINT32 stpx = KDRV_IPP_FMT_STPX(fmt);

	switch (stpx) {
	case KDRV_IPP_FMT_RGGB_R:
		return IPE_ENG_CFA_RGGB;

	case KDRV_IPP_FMT_RGGB_GR:
		return IPE_ENG_CFA_GRBG;

	case KDRV_IPP_FMT_RGGB_GB:
		return IPE_ENG_CFA_GBRG;

	case KDRV_IPP_FMT_RGGB_B:
		return IPE_ENG_CFA_BGGR;

	case KDRV_IPP_FMT_RGBIR44_RGBG_GIGI:
		return IPE_ENG_CFA_RGBG_GIGI;

	case KDRV_IPP_FMT_RGBIR44_GBGR_IGIG:
		return IPE_ENG_CFA_GBGR_IGIG;

	case KDRV_IPP_FMT_RGBIR44_GIGI_BGRG:
		return IPE_ENG_CFA_GIGI_BGRG;

	case KDRV_IPP_FMT_RGBIR44_IGIG_GRGB:
		return IPE_ENG_CFA_IGIG_GRGB;

	case KDRV_IPP_FMT_RGBIR44_BGRG_GIGI:
		return IPE_ENG_CFA_BGRG_GIGI;

	case KDRV_IPP_FMT_RGBIR44_GRGB_IGIG:
		return IPE_ENG_CFA_GRGB_IGIG;

	case KDRV_IPP_FMT_RGBIR44_GIGI_RGBG:
		return IPE_ENG_CFA_GIGI_RGBG;

	case KDRV_IPP_FMT_RGBIR44_IGIG_GBGR:
		return IPE_ENG_CFA_IGIG_GBGR;

	default:
		DBG_ERR("Unknown stpx 0x%.8x\r\n", (unsigned int)stpx);
		return -1;
	}
}

static IPE_ENG_CFA_SUBOUT_CH_SEL kdrv_ipe_typecast_cfa_sub_ch(KDRV_IPE_CFA_SUBOUT_CH_SEL ch_sel)
{
	switch (ch_sel) {
	case KDRV_IPE_CFA_SUBOUT_CH0:
		return IPE_ENG_CFA_SUBOUT_CH0;

	case KDRV_IPE_CFA_SUBOUT_CH1:
		return IPE_ENG_CFA_SUBOUT_CH1;

	case KDRV_IPE_CFA_SUBOUT_CH2:
		return IPE_ENG_CFA_SUBOUT_CH2;

	case KDRV_IPE_CFA_SUBOUT_CH3:
		return IPE_ENG_CFA_SUBOUT_CH3;
	default:
		DBG_ERR("unknown cfa subout ch %d \r\n", ch_sel);
		return -1;
	}
}

static IPE_ENG_CFA_SUBOUT_BYTE kdrv_ipe_typecast_cfa_sub_byte(KDRV_IPE_CFA_SUBOUT_BYTE byte)
{
	switch (byte) {
	case KDRV_IPE_CFA_SUBOUT_1BYTE:
		return IPE_ENG_CFA_SUBOUT_1BYTE;

	case KDRV_IPE_CFA_SUBOUT_2BYTE:
		return IPE_ENG_CFA_SUBOUT_2BYTE;

	default:
		DBG_ERR("unknown cfa subout byte %d \r\n", byte);
		return -1;
	}
}

static void kdrv_ipe_parsing_io_info(KDRV_IPE_IO_CFG* io_cfg, KDRV_IPE_IO_INFO* info)
{
	if (io_cfg->mode == KDRV_IPP_OPMODE_D2D || io_cfg->mode == KDRV_IPP_OPMODE_IFE2IPE || io_cfg->mode == KDRV_IPP_OPMODE_PRE2IPE) {
		info->to_ime= FALSE;
	} else {
		info->to_ime = TRUE;
	}

	if (io_cfg->dma_out_en) {
		info->to_dma= TRUE;
	} else {
		info->to_dma = FALSE;
	}

	if (io_cfg->dma_out_en) {
		info->out_sel = IPE_ENG_OUT_ORIGINAL;
	}  /*else {
		info->out_sel = IPE_ENG_OUT_ORIGINAL;
	}*/

	if ((io_cfg->mode == KDRV_IPP_OPMODE_IFE2IME) || (io_cfg->mode == KDRV_IPP_OPMODE_PRE2IPE) || (io_cfg->mode == KDRV_IPP_OPMODE_PRE2IME)
			||(io_cfg->mode == KDRV_IPP_OPMODE_DCE2IME) || (io_cfg->mode == KDRV_IPP_OPMODE_SIE2IME) || (io_cfg->mode == KDRV_IPP_OPMODE_IFE2IPE)) {

		if( (KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW) ||  (KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX))
			info->in_fmt = IPE_ENG_BAYER/*IPE_ENG_FMT_Y_PACK_UV444*/;
		else if(KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_YUV)
			info->in_fmt = IPE_ENG_FMT_Y_PACK_UV444;
		else
			DBG_ERR("unknown ipe in fmt\n");

		info->out_fmt = /*IPE_ENG_FMT_Y*/IPE_ENG_FMT_Y_PACK_UV420/*IPE_ENG_FMT_Y_PACK_UV444*/;
	} else {
		info->in_fmt = kdrv_ipe_typecast_io_fmt(io_cfg->in_frm.fmt);
		info->out_fmt = kdrv_ipe_typecast_io_fmt(io_cfg->out_frm.fmt);
	}
}

#endif

static BOOL kdrv_ipe_chk_align(UINT32 val, UINT32 align)
{
	if (align == 0) {
		return TRUE;
	}else if(align == 2){
		return ( (val & 0x1) == 0);
	}else if(align == 4){
		return ( (val & 0x3) == 0);
	}else if(align == 8){
		return ( (val & 0x7) == 0);
	}else if(align == 16){
		return ( (val & 0xf) == 0);
	}else if(align == 32){
		return ( (val & 0x1f) == 0);
	}else{
		return ((val % align) == 0);
	}


}

static INT32 kdrv_ipe_int_get_limitation(KDRV_IPP_FMT fmt, KDRV_IPE_LMT *p_lmt)
{
	switch (fmt) {

		case KDRV_IPP_FMT_YUV420:

			p_lmt->in_w_min = IPE_SSDRV_SRCBUF_IPP_WMIN;
			p_lmt->in_w_max = IPE_SSDRV_SRCBUF_IPP_WMAX;
			p_lmt->in_w_align = IPE_SSDRV_SRCBUF_IPP_WALIGN;
			p_lmt->in_h_min = IPE_SSDRV_SRCBUF_IPP_HMIN;
			p_lmt->in_h_max = IPE_SSDRV_SRCBUF_IPP_HMAX;
			p_lmt->in_h_align = IPE_SSDRV_SRCBUF_IPP_HALIGN;
			p_lmt->in_y_lofs_align = IPE_SSDRV_SRCBUF_IPP_LOFF_ALIGN;
			p_lmt->in_uv_lofs_align = IPE_SSDRV_SRCBUF_IPP_LOFF_ALIGN;
			p_lmt->in_y_addr_align = IPE_SSDRV_SRCBUF_IPP_ADDR_ALIGN;
			p_lmt->in_uv_addr_align = IPE_SSDRV_SRCBUF_IPP_ADDR_ALIGN;

			p_lmt->out_w_min = IPE_SSDRV_SRCBUF_IPP_WMIN;
			p_lmt->out_w_max = IPE_SSDRV_SRCBUF_IPP_WMAX;
			p_lmt->out_w_align = IPE_SSDRV_SRCBUF_IPP_WALIGN;
			p_lmt->out_h_min = IPE_SSDRV_SRCBUF_IPP_HMIN;
			p_lmt->out_h_max = IPE_SSDRV_SRCBUF_IPP_HMAX;
			p_lmt->out_h_align = IPE_SSDRV_SRCBUF_IPP_HALIGN;
			p_lmt->out_y_lofs_align = IPE_SSDRV_SRCBUF_IPP_LOFF_ALIGN;
			p_lmt->out_uv_lofs_align = IPE_SSDRV_SRCBUF_IPP_LOFF_ALIGN;
			p_lmt->out_y_addr_align = IPE_SSDRV_SRCBUF_IPP_ADDR_ALIGN;
			p_lmt->out_uv_addr_align = IPE_SSDRV_SRCBUF_IPP_ADDR_ALIGN;

			p_lmt->defog_subimg_w_min = IPE_SSDRV_DEFOG_WMIN;
			p_lmt->defog_subimg_w_max = IPE_SSDRV_DEFOG_WMAX;
			p_lmt->defog_subimg_w_align = 1; //no alignment restriction
			p_lmt->defog_subimg_h_min = IPE_SSDRV_DEFOG_HMIN;
			p_lmt->defog_subimg_h_max = IPE_SSDRV_DEFOG_HMAX;
			p_lmt->defog_subimg_h_align = 1; //no alignment restriction
			p_lmt->defog_subimg_lofs_align = IPE_SSDRV_DEFOG_LOFF_ALIGN;
			p_lmt->defog_subimg_addr_align = IPE_SSDRV_DEFOG_ADDR_ALIGN;

			p_lmt->lce_subimg_w_min = IPE_SSDRV_DEFOG_WMIN;
			p_lmt->lce_subimg_w_max = IPE_SSDRV_DEFOG_WMAX;
			p_lmt->lce_subimg_w_align = 1; //no alignment restriction
			p_lmt->lce_subimg_h_min = IPE_SSDRV_DEFOG_HMIN;
			p_lmt->lce_subimg_h_max = IPE_SSDRV_DEFOG_HMAX;
			p_lmt->lce_subimg_h_align = 1; //no alignment restriction
			p_lmt->lce_subimg_lofs_align = IPE_SSDRV_DEFOG_LOFF_ALIGN;
			p_lmt->lce_subimg_addr_align = IPE_SSDRV_DEFOG_ADDR_ALIGN;

		break;

		default:
			p_lmt->defog_subimg_w_min = IPE_SSDRV_DEFOG_WMIN;
			p_lmt->defog_subimg_w_max = IPE_SSDRV_DEFOG_WMAX;
			p_lmt->defog_subimg_w_align = 1; //no alignment restriction
			p_lmt->defog_subimg_h_min = IPE_SSDRV_DEFOG_HMIN;
			p_lmt->defog_subimg_h_max = IPE_SSDRV_DEFOG_HMAX;
			p_lmt->defog_subimg_h_align = 1; //no alignment restriction
			p_lmt->defog_subimg_lofs_align = IPE_SSDRV_DEFOG_LOFF_ALIGN;
			p_lmt->defog_subimg_addr_align = IPE_SSDRV_DEFOG_ADDR_ALIGN;

			p_lmt->lce_subimg_w_min = 1;
			p_lmt->lce_subimg_w_max = 128;
			p_lmt->lce_subimg_w_align = 1; //no alignment restriction
			p_lmt->lce_subimg_h_min = 1;
			p_lmt->lce_subimg_h_max = 128;
			p_lmt->lce_subimg_h_align = 1; //no alignment restriction
			p_lmt->lce_subimg_lofs_align = 4;
			p_lmt->lce_subimg_addr_align = 4;

		break;
	}

	return E_OK;
}

static INT32 kdrv_ipe_int_check_limit(KDRV_IPE_JOB_CFG *p_cfg, KDRV_IPP_PXLFMT pxl_fmt)
{
	KDRV_IPE_LMT lmt = {0};

	if (p_cfg->p_iocfg->mode >= KDRV_IPP_OPMODE_MAX) {
		DBG_ERR("unsupport mode %d\r\n", (int)p_cfg->p_iocfg->mode);
		return E_PAR;
	}

	if (p_cfg->p_iocfg->mode == KDRV_IPP_OPMODE_D2D) {
		if (p_cfg->p_iocfg->in_frm.fmt != KDRV_IPP_FMT_YUV444 &&
			p_cfg->p_iocfg->in_frm.fmt != KDRV_IPP_FMT_YUV422 &&
			p_cfg->p_iocfg->in_frm.fmt != KDRV_IPP_FMT_YUV420 &&
			p_cfg->p_iocfg->in_frm.fmt != KDRV_IPP_FMT_Y8) {
			DBG_ERR("unsupport in format 0x%.8x\r\n", (unsigned int)p_cfg->p_iocfg->in_frm.fmt);
			return E_PAR;
		}

		if (p_cfg->p_iocfg->out_frm.fmt != KDRV_IPP_FMT_YUV444 &&
			p_cfg->p_iocfg->out_frm.fmt != KDRV_IPP_FMT_YUV422 &&
			p_cfg->p_iocfg->out_frm.fmt != KDRV_IPP_FMT_YUV420 &&
			p_cfg->p_iocfg->out_frm.fmt != KDRV_IPP_FMT_Y8) {
			DBG_ERR("unsupport out format 0x%.8x\r\n", (unsigned int)p_cfg->p_iocfg->out_frm.fmt);
			return E_PAR;
		}
	}

	if (kdrv_ipe_int_get_limitation(p_cfg->p_iocfg->in_frm.fmt, &lmt) != E_OK) {
		DBG_ERR("get ipe limit fail\n");
		return E_PAR;
	}

	if (KDRV_IPP_FMT_CLASS(p_cfg->p_iocfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_YUV) {
		UINT32 fmt_ycc = pxl_fmt;

		if (fmt_ycc != KDRV_IPP_PXLFMT_YCC_FULL &&
			fmt_ycc != KDRV_IPP_PXLFMT_YCC_BT601 &&
			fmt_ycc != KDRV_IPP_PXLFMT_YCC_BT709) {
			DBG_ERR("unsupport ycc format 0x%.8x\r\n", (unsigned int)p_cfg->p_iocfg->in_frm.fmt);
			return E_PAR;
		}
	}

	/*-----------input buf limitation-----------*/
	if(p_cfg->p_iocfg->mode == KDRV_IPP_OPMODE_D2D){

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->in_frm.size.w, lmt.in_w_align) != TRUE) {
			DBG_ERR("in_width %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->in_frm.size.w, (UINT32)lmt.in_w_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->in_frm.size.h, lmt.in_h_align) != TRUE) {
			DBG_ERR("in_height %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->in_frm.size.h, (UINT32)lmt.in_h_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->in_frm.lofs[0], lmt.in_y_lofs_align) != TRUE) {
			DBG_ERR("in_y_lofs %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->in_frm.lofs[0], (UINT32)lmt.in_y_lofs_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->in_frm.lofs[1], lmt.in_uv_lofs_align) != TRUE) {
			DBG_ERR("in_uv_lofs %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->in_frm.lofs[1], (UINT32)lmt.in_uv_lofs_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->in_frm.phyaddr[0], lmt.in_y_addr_align) != TRUE) {
			DBG_ERR("in_y_addr 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->in_frm.phyaddr[0], (UINT32)lmt.in_y_addr_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->in_frm.phyaddr[1], lmt.in_uv_addr_align) != TRUE) {
			DBG_ERR("in_uv_addr 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->in_frm.phyaddr[1], (UINT32)lmt.in_uv_addr_align);
			return E_PAR;
		}
	}
	/*-----------output buf limitation-----------*/
	if(p_cfg->p_iocfg->dma_out_en){

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->out_frm.size.w, lmt.out_w_align) != TRUE) {
			DBG_ERR("out_width %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->out_frm.size.w, (UINT32)lmt.out_w_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->out_frm.size.h, lmt.out_h_align) != TRUE) {
			DBG_ERR("out_height %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->out_frm.size.h, (UINT32)lmt.out_h_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->out_frm.lofs[0], lmt.out_y_lofs_align) != TRUE) {
			DBG_ERR("out_y_lofs %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->out_frm.lofs[0], (UINT32)lmt.out_y_lofs_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->out_frm.lofs[1], lmt.out_uv_lofs_align) != TRUE) {
			DBG_ERR("out_uv_lofs %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->out_frm.lofs[1], (UINT32)lmt.out_uv_lofs_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->out_frm.phyaddr[0], lmt.out_y_addr_align) != TRUE) {
			DBG_ERR("out_y_addr 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->out_frm.phyaddr[0], (UINT32)lmt.out_y_addr_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->out_frm.phyaddr[1], lmt.out_uv_addr_align) != TRUE) {
			DBG_ERR("out_uv_addr 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->out_frm.phyaddr[1], (UINT32)lmt.out_uv_addr_align);
			return E_PAR;
		}
	}
	/*-----------defog sub img limitation-----------*/
	if(p_cfg->p_iocfg->sub_out_en && p_cfg->p_iqcfg->defog.enable){

		if(p_cfg->p_iqcfg->subimg.subimg_size.h_size < lmt.defog_subimg_w_min){
			DBG_ERR("defog_subimg w:%d < min:%d\n", p_cfg->p_iqcfg->subimg.subimg_size.h_size, lmt.defog_subimg_w_min);
			return E_PAR;
		}

		if(p_cfg->p_iqcfg->subimg.subimg_size.h_size > lmt.defog_subimg_w_max){
			DBG_ERR("defog_subimg w:%d > max:%d\n", p_cfg->p_iqcfg->subimg.subimg_size.h_size, lmt.defog_subimg_w_max);
			return E_PAR;
		}

		if(p_cfg->p_iqcfg->subimg.subimg_size.v_size < lmt.defog_subimg_h_min){
			DBG_ERR("defog_subimg h:%d < min:%d\n", p_cfg->p_iqcfg->subimg.subimg_size.v_size, lmt.defog_subimg_h_min);
			return E_PAR;
		}

		if(p_cfg->p_iqcfg->subimg.subimg_size.v_size > lmt.defog_subimg_h_max){
			DBG_ERR("defog_subimg h:%d > max:%d\n", p_cfg->p_iqcfg->subimg.subimg_size.v_size, lmt.defog_subimg_h_max);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->sub_in_addr.pa, lmt.defog_subimg_addr_align) != TRUE) {
			DBG_ERR("defog_subimg_in_addr 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->sub_in_addr.pa, (UINT32)lmt.defog_subimg_addr_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->sub_out_addr.pa, lmt.defog_subimg_addr_align) != TRUE) {
			DBG_ERR("defog_subimg_out_addr 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->sub_out_addr.pa, (UINT32)lmt.defog_subimg_addr_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->subimg_lofs_in, lmt.defog_subimg_lofs_align) != TRUE) {
			DBG_ERR("defog_subimg_in_lofs %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->subimg_lofs_in, (UINT32)lmt.defog_subimg_lofs_align);
			return E_PAR;
		}

		if (kdrv_ipe_chk_align(p_cfg->p_iocfg->subimg_lofs_out, lmt.defog_subimg_lofs_align) != TRUE) {
			DBG_ERR("defog_subimg_out_lofs %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->subimg_lofs_out, (UINT32)lmt.defog_subimg_lofs_align);
			return E_PAR;
		}
	}

	/*-----------y_curve_sel limitation-----------*/
	if(p_cfg->p_iocfg->mode == KDRV_IPP_OPMODE_D2D && p_cfg->p_iqcfg->y_curve.enable == TRUE && p_cfg->p_iqcfg->y_curve.ycurve_sel != 2)
		DBG_ERR("ipe d2d mode on and y curve sel is not 2\n"); //When IPE run D2D mode, y_curve_sel only can be 2

	/*-----------edge motion limitation-----------*/
	if (p_cfg->p_iqcfg->eext.motion_enable && p_cfg->p_iocfg->in_frm.size.w > MAX_IPP_STRIPE_SIZE) {
		DBG_ERR("ipe motion map not support MST (in_width %d > %d)\r\n", (UINT32)p_cfg->p_iocfg->in_frm.size.w, MAX_IPP_STRIPE_SIZE);
		return E_PAR;
	}

	return E_OK;
}

#if IPE_SSDRV_SUPPORT
static INT32 kdrv_ipe_int_cfg_rgblpf(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_RGBLPF) {

		IPE_RGBLPF_PARAM rgblpf_info;

		rgblpf_info.lpf_r_info.filt_size = p_iq_cfg->rgb_lpf.lpf_param_r.filt_size;
		rgblpf_info.lpf_r_info.lpfw = p_iq_cfg->rgb_lpf.lpf_param_r.lpf_w;
		rgblpf_info.lpf_r_info.range_th0 = p_iq_cfg->rgb_lpf.lpf_param_r.range_th0;
		rgblpf_info.lpf_r_info.range_th1 = p_iq_cfg->rgb_lpf.lpf_param_r.range_th1;
		rgblpf_info.lpf_r_info.sonly_w = p_iq_cfg->rgb_lpf.lpf_param_r.s_only_w;

		rgblpf_info.lpf_g_info.filt_size = p_iq_cfg->rgb_lpf.lpf_param_g.filt_size;
		rgblpf_info.lpf_g_info.lpfw = p_iq_cfg->rgb_lpf.lpf_param_g.lpf_w;
		rgblpf_info.lpf_g_info.range_th0 = p_iq_cfg->rgb_lpf.lpf_param_g.range_th0;
		rgblpf_info.lpf_g_info.range_th1 = p_iq_cfg->rgb_lpf.lpf_param_g.range_th1;
		rgblpf_info.lpf_g_info.sonly_w = p_iq_cfg->rgb_lpf.lpf_param_g.s_only_w;

		rgblpf_info.lpf_b_info.filt_size = p_iq_cfg->rgb_lpf.lpf_param_b.filt_size;
		rgblpf_info.lpf_b_info.lpfw = p_iq_cfg->rgb_lpf.lpf_param_b.lpf_w;
		rgblpf_info.lpf_b_info.range_th0 = p_iq_cfg->rgb_lpf.lpf_param_b.range_th0;
		rgblpf_info.lpf_b_info.range_th1 = p_iq_cfg->rgb_lpf.lpf_param_b.range_th1;
		rgblpf_info.lpf_b_info.sonly_w = p_iq_cfg->rgb_lpf.lpf_param_b.s_only_w;

		ipe_eng_set_rgblpf_buf_reg(p_hdl->p_eng, &rgblpf_info);
		ipe_eng_set_rgb_lpf_enable_buf_reg(p_hdl->p_eng, p_iq_cfg->rgb_lpf.enable);
	}

	return E_OK;
}

#if (DUAL_IPP == 0)
static INT32 kdrv_ipe_int_cfg_lce(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg, IPE_ENG_DMA_OUT_INFO* p_single_out)
{
	KDRV_IPE_IQ_LCE *p_lce_info = NULL;
	KDRV_IPE_IQ_SUBIMG *p_sub_img_info = NULL;
	KDRV_IPE_IO_CFG* p_io_cfg = NULL;

	p_lce_info = &p_cfg->p_iqcfg->lce;
	p_sub_img_info = &p_cfg->p_iqcfg->subimg;
	p_io_cfg = p_cfg->p_iocfg;

	/* Defog & LCE I/O control */
	if 	(p_lce_info->enable) {

		{
			//lca & defog using same subimg
			UINT64 addr = p_cfg->p_iocfg->sub_in_addr.pa;
			ipe_eng_set_dma_in_defog_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
		}
	}

	if (p_cfg->p_iocfg->sub_out_en) {
		{
			UINT64 addr = p_cfg->p_iocfg->sub_out_addr.pa;
			ipe_eng_set_dma_out_defog_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
		}
		ipe_eng_set_defog_subout_enable_buf_reg(p_hdl->p_eng, ENABLE);
		//p_single_out->lce_en = ENABLE; //538 remove
		p_single_out->defog_en = ENABLE;
	} else {
		ipe_eng_set_defog_subout_enable_buf_reg(p_hdl->p_eng, DISABLE);
	}
	if (p_cfg->p_iqcfg->update & KDRV_IPE_UPDATE_LCE) {
		if (p_lce_info->enable) {
			ipe_eng_set_lce_lum_lut_buf_reg(p_hdl->p_eng, &p_lce_info->lum_wt_lut[0]);
			ipe_eng_set_lce_diff_param_buf_reg(p_hdl->p_eng, p_lce_info->diff_wt_avg, p_lce_info->diff_wt_pos, p_lce_info->diff_wt_neg);
			ipe_eng_set_lce_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_lce_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	if (p_cfg->p_iqcfg->update & KDRV_IPE_UPDATE_SUBIMG) {

		IPE_IMG_SIZE defog_size;
		UINT32 blk_size_h, blk_size_v, blk_cent_fact_h, blk_cent_fact_v;
		UINT32 scaling_h, scaling_v;

		defog_size.h_size = p_sub_img_info->subimg_size.h_size;
		defog_size.v_size = p_sub_img_info->subimg_size.v_size;
		//p_io_cfg->subimg_lofs_out = p_sub_img_info->subimg_size.h_size << 2; //538 added
		//p_io_cfg->subimg_lofs_in = p_sub_img_info->subimg_size.h_size << 2;


		ipe_eng_set_dma_out_defog_offset_buf_reg(p_hdl->p_eng, p_io_cfg->subimg_lofs_out, p_io_cfg->defog_subout_addr_ofs);    //p_sub_img_info->subimg_lofs_out
		ipe_eng_set_dma_in_defog_offset_buf_reg(p_hdl->p_eng, p_io_cfg->subimg_lofs_in,  p_io_cfg->defog_subin_addr_ofs);//p_sub_img_info->subimg_lofs_in
		ipe_eng_set_dfg_subimg_size_buf_reg(p_hdl->p_eng, defog_size);
		ipe_eng_set_dfg_scal_filtcoeff_buf_reg(p_hdl->p_eng, &p_sub_img_info->subimg_ftrcoef[0]); //tmp early porting need to check ftcoef
		/* auto calculate */
		blk_size_h = (( (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1) / (p_sub_img_info->subimg_size.h_size)));
		blk_size_v = (((p_cfg->p_iocfg->in_frm.size.h - 1) / (p_sub_img_info->subimg_size.v_size)));
		blk_cent_fact_h = ((((p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1) << 12) / (p_sub_img_info->subimg_size.h_size - 1)));
		blk_cent_fact_v = ((((p_cfg->p_iocfg->in_frm.size.h - 1) << 12) / (p_sub_img_info->subimg_size.v_size - 1)));

		ipe_eng_set_dfg_subout_param_buf_reg(p_hdl->p_eng, blk_size_h, blk_cent_fact_h, blk_size_v, blk_cent_fact_v);

		scaling_h = ((((p_sub_img_info->subimg_size.h_size - 1) << 16) /  (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1)));
		scaling_v = ((((p_sub_img_info->subimg_size.v_size - 1) << 16) / (p_cfg->p_iocfg->in_frm.size.h - 1)));

		ipe_eng_set_dfg_scal_factor_buf_reg(p_hdl->p_eng, scaling_h, scaling_v);
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_defog(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg, IPE_ENG_DMA_OUT_INFO* p_single_out)
{
	KDRV_IPE_IQ_DEFOG *p_defog_info;
	KDRV_IPE_IQ_SUBIMG *p_sub_img_info;
	KDRV_IPE_IO_CFG* p_io_cfg = NULL;

	if(p_hdl == NULL){
		DBG_ERR("hdl == NULL\n");
		return E_PAR;
	}

	if(p_hdl->p_eng == NULL ){
		DBG_ERR("eng == NULL\n");
		return E_PAR;
	}

	p_defog_info = &p_cfg->p_iqcfg->defog;
	p_sub_img_info = &p_cfg->p_iqcfg->subimg;
	p_io_cfg = p_cfg->p_iocfg;

	/* Defog I/O control */
	if 	(p_defog_info->enable) {
		if(p_cfg->p_iocfg->sub_in_addr.pa != 0 && p_cfg->p_iocfg->sub_in_addr.pa != (ULONG)-1){

			{
				UINT64 addr = p_cfg->p_iocfg->sub_in_addr.pa;
				ipe_eng_set_dma_in_defog_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
			}
		}
		else{
			DBG_ERR("dma_in_defog_addr:0x%lx invalid\n", p_cfg->p_iocfg->sub_in_addr.pa);
			return E_PAR;
		}
	}
	if (p_cfg->p_iocfg->sub_out_en) {
		if(p_cfg->p_iocfg->sub_out_addr.pa != 0 && p_cfg->p_iocfg->sub_out_addr.pa != (ULONG)-1){

			{
				UINT64 addr = p_cfg->p_iocfg->sub_out_addr.pa;
				ipe_eng_set_dma_out_defog_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
			}
		}
		else{
			DBG_ERR("dma_out_defog_addr:0x%lx invalid\n", p_cfg->p_iocfg->sub_out_addr.pa);
			return E_PAR;
		}
		ipe_eng_set_defog_subout_enable_buf_reg(p_hdl->p_eng, ENABLE);
		p_single_out->defog_en = ENABLE;
	} else {
		ipe_eng_set_defog_subout_enable_buf_reg(p_hdl->p_eng, DISABLE);
	}

	if (p_cfg->p_iqcfg->update & KDRV_IPE_UPDATE_DEFOG) {
		//539A add
		ipe_eng_set_defog_airlight_enable_buf_reg(p_hdl->p_eng, p_defog_info->air_enable);

		if (p_defog_info->enable) {

			UINT16 air_nfactor = 0; // NORMFACTOR
			IPE_DEFOGROUND dfg_round = {0};

			UINT32 airlight_pxl_cnt = 0;

			airlight_pxl_cnt = (UINT32)((p_cfg->p_iocfg->in_frm.size.w * p_cfg->p_iocfg->in_frm.size.h * p_cfg->p_iqcfg->defog.dfg_stcs.airlight_stcs_ratio) >> 12);

			ipe_eng_set_defog_stcs_pcnt_buf_reg(p_hdl->p_eng, airlight_pxl_cnt);

			ipe_eng_set_defog_strength_buf_reg(p_hdl->p_eng, p_defog_info->dfg_strength.str_mode_sel,p_defog_info->dfg_strength.fog_ratio,
												p_defog_info->dfg_strength.dgain_ratio,p_defog_info->dfg_strength.gain_th);
			ipe_eng_set_defog_fog_target_lut_buf_reg(p_hdl->p_eng, &p_defog_info->dfg_strength.target_lut[0]);

			ipe_eng_set_defog_fog_mod_buf_reg(p_hdl->p_eng, &p_defog_info->env_estimation.fog_mod_lut[0]);
			ipe_eng_set_defog_fog_protect_buf_reg(p_hdl->p_eng, p_defog_info->env_estimation.dfg_self_comp_en,p_defog_info->env_estimation.dfg_min_diff);
			ipe_eng_set_defog_atmospherelight_buf_reg(p_hdl->p_eng, p_defog_info->env_estimation.dfg_airlight[0],
														p_defog_info->env_estimation.dfg_airlight[1],
														p_defog_info->env_estimation.dfg_airlight[2]);

			ipe_eng_set_defog_scal_edgeinterp_buf_reg(p_hdl->p_eng, p_defog_info->scalup_param.interp_wdist,p_defog_info->scalup_param.interp_wout,
														p_defog_info->scalup_param.interp_wcenter,
														p_defog_info->scalup_param.interp_wsrc);
			ipe_eng_set_dfg_scalg_edgeinterplut_buf_reg(p_hdl->p_eng, &p_defog_info->scalup_param.interp_diff_lut[0]);


			air_nfactor = ipe_eng_cal_defog_airlight_nfactor(p_cfg->p_iocfg->defog_info.hw_airlight);

			ipe_eng_set_defog_outbld_lum_lut_buf_reg(p_hdl->p_eng, p_defog_info->dfg_outbld.outbld_ref_sel, &p_defog_info->dfg_outbld.outbld_lum_wt[0]);
			ipe_eng_set_defog_outbld_diff_lut_buf_reg(p_hdl->p_eng, &p_defog_info->dfg_outbld.outbld_diff_wt[0]);
			ipe_eng_set_defog_outbld_local_buf_reg(p_hdl->p_eng, p_defog_info->dfg_outbld.outbld_local_en,air_nfactor);

			dfg_round.rand_opt = p_defog_info->dfg_round.rand_opt;
			dfg_round.rand_rst = p_defog_info->dfg_round.rand_rst;
			ipe_eng_set_defog_rand_buf_reg(p_hdl->p_eng, &dfg_round);

			ipe_eng_set_dfg_input_bld_buf_reg(p_hdl->p_eng, &p_defog_info->input_bld.in_blend_wt[0]);
			ipe_eng_set_defog_enable_buf_reg(p_hdl->p_eng, ENABLE);

			//539A add
			ipe_eng_set_defog_airlight_enable_buf_reg(p_hdl->p_eng, ENABLE);
			ipe_eng_set_sram_sd_defog_buf_reg(p_hdl->p_eng, DISABLE);
		} else {
			if(!p_cfg->p_iqcfg->lce.enable && !p_cfg->p_iocfg->sub_out_en) {
				if (kdrv_ipe_typecast_opmode(p_io_cfg->mode) != IPE_ENG_OPMODE_ALL_DIRECT || (kdrv_ipe_typecast_opmode(p_io_cfg->mode) == IPE_ENG_OPMODE_ALL_DIRECT && p_io_cfg->in_job_num == 1)) { //direct only start job can shutdown
					ipe_eng_set_sram_sd_defog_buf_reg(p_hdl->p_eng, ENABLE);
				}
			}
			ipe_eng_set_defog_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}
	if (p_cfg->p_iqcfg->update & KDRV_IPE_UPDATE_SUBIMG) {
		IPE_IMG_SIZE defog_size;
		UINT32 blk_size_h, blk_size_v, blk_cent_fact_h, blk_cent_fact_v;
		UINT32 scaling_h, scaling_v;

		defog_size.h_size = p_sub_img_info->subimg_size.h_size;
		defog_size.v_size = p_sub_img_info->subimg_size.v_size;
		//p_io_cfg->subimg_lofs_in = p_sub_img_info->subimg_size.h_size << 2; //538 added
		//p_io_cfg->subimg_lofs_out = p_sub_img_info->subimg_size.h_size << 2; //538 added

		ipe_eng_set_dma_out_defog_offset_buf_reg(p_hdl->p_eng, p_io_cfg->subimg_lofs_out, p_io_cfg->defog_subout_addr_ofs);
		ipe_eng_set_dma_in_defog_offset_buf_reg(p_hdl->p_eng, p_io_cfg->subimg_lofs_in,  p_io_cfg->defog_subin_addr_ofs);
		ipe_eng_set_dfg_subimg_size_buf_reg(p_hdl->p_eng, defog_size);
		ipe_eng_set_dfg_scal_filtcoeff_buf_reg(p_hdl->p_eng, &p_sub_img_info->subimg_ftrcoef[0]); //tmp early porting need to check ftcoef

		/* auto calculate */
		blk_size_h = ((  (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1) / (p_sub_img_info->subimg_size.h_size)));
		blk_size_v = (((p_cfg->p_iocfg->in_frm.size.h - 1) / (p_sub_img_info->subimg_size.v_size)));
		blk_cent_fact_h = (((   (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1) << 12) / (p_sub_img_info->subimg_size.h_size - 1)));
		blk_cent_fact_v = ((((p_cfg->p_iocfg->in_frm.size.h - 1) << 12) / (p_sub_img_info->subimg_size.v_size - 1)));
		ipe_eng_set_dfg_subout_param_buf_reg(p_hdl->p_eng, blk_size_h, blk_cent_fact_h, blk_size_v, blk_cent_fact_v);

		scaling_h = ((((p_sub_img_info->subimg_size.h_size - 1) << 16) /  (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1)));
		scaling_v = ((((p_sub_img_info->subimg_size.v_size - 1) << 16) / (p_cfg->p_iocfg->in_frm.size.h - 1)));
		ipe_eng_set_dfg_scal_factor_buf_reg(p_hdl->p_eng, scaling_h, scaling_v);
	}
	return E_OK;
}

#else
static INT32 kdrv_ipe_int_cfg_lce(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg, IPE_ENG_DMA_OUT_INFO* p_single_out) //dual ver
{
	KDRV_IPE_LCE_PARAM *p_lce_info = NULL;
	KDRV_IPE_SUBIMG_PARAM *p_sub_img_info = NULL;
	KDRV_IPE_IO_CFG* p_io_cfg = NULL;

	p_lce_info = &p_cfg->p_iqcfg->lce_param;
	p_sub_img_info = &p_cfg->p_iqcfg->lce_subimg_param;
	p_io_cfg = p_cfg->p_iocfg;

	/* Defog & LCE I/O control */
	if 	(p_lce_info->enable) {

		{
			UINT64 addr = p_cfg->p_iocfg->lce_sub_in_addr.pa;
			ipe_eng_set_dma_in_lce_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
		}
	}

	if (p_cfg->p_iocfg->lce_sub_out_en) {

		{
			UINT64 addr = p_cfg->p_iocfg->lce_sub_out_addr.pa;
			ipe_eng_set_dma_out_lce_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
		}
		ipe_eng_set_lce_subout_enable_buf_reg(p_hdl->p_eng, ENABLE);
		p_single_out->lce_en = ENABLE;
	} else {
	}

	if (p_cfg->p_iqcfg->update & KDRV_IPE_UPDATE_LCE) {
		if (p_lce_info->enable) {
			ipe_eng_set_lce_lum_lut_buf_reg(p_hdl->p_eng, &p_lce_info->lum_wt_lut[0]);
			ipe_eng_set_lce_diff_param_buf_reg(p_hdl->p_eng, p_lce_info->diff_wt_avg, p_lce_info->diff_wt_pos, p_lce_info->diff_wt_neg);

			ipe_eng_set_lce_scalg_edgeinterplut_buf_reg(p_hdl->p_eng, &p_lce_info->scalup_param.interp_diff_lut[0]);
			ipe_eng_set_lce_scal_edgeinterp_buf_reg(p_hdl->p_eng, p_lce_info->scalup_param.interp_wout, p_lce_info->scalup_param.interp_wcenter);

			ipe_eng_set_lce_input_bld_buf_reg(p_hdl->p_eng, &p_lce_info->input_bld.in_blend_wt[0]);

			ipe_eng_set_lce_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_lce_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	if (p_cfg->p_iqcfg->update & KDRV_IPE_UPDATE_LCE_SUBIMG) {
		IPE_IMG_SIZE lce_size;
		UINT32 blk_size_h, blk_size_v, blk_cent_fact_h, blk_cent_fact_v;
		UINT32 scaling_h, scaling_v;

		lce_size.h_size = p_sub_img_info->h_size;
		lce_size.v_size = p_sub_img_info->v_size;
		ipe_eng_set_dma_out_lce_offset_buf_reg(p_hdl->p_eng, p_sub_img_info->subimg_lofs_out, p_io_cfg->lce_subout_addr_ofs);
		ipe_eng_set_dma_in_lce_offset_buf_reg(p_hdl->p_eng, p_sub_img_info->subimg_lofs_in, p_io_cfg->lce_subin_addr_ofs);
		ipe_eng_set_lce_subimg_size_buf_reg(p_hdl->p_eng, lce_size);
		ipe_eng_set_lce_scal_filtcoeff_buf_reg(p_hdl->p_eng, &p_sub_img_info->subimg_ftrcoef[0]);

		/* auto calculate */
		blk_size_h = (( (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1) / (p_sub_img_info->h_size)));
		blk_size_v = (((p_cfg->p_iocfg->in_frm.size.h - 1) / (p_sub_img_info->v_size)));
		blk_cent_fact_h = ((((p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1) << 12) / (p_sub_img_info->h_size - 1)));
		blk_cent_fact_v = ((((p_cfg->p_iocfg->in_frm.size.h - 1) << 12) / (p_sub_img_info->v_size - 1)));
		ipe_eng_set_lce_subout_param_buf_reg(p_hdl->p_eng, blk_size_h, blk_cent_fact_h, blk_size_v, blk_cent_fact_v);

		scaling_h = ((((p_sub_img_info->h_size - 1) << 16) /  (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1)));
		scaling_v = ((((p_sub_img_info->v_size - 1) << 16) / (p_cfg->p_iocfg->in_frm.size.h - 1)));
		ipe_eng_set_lce_scal_factor_buf_reg(p_hdl->p_eng, scaling_h, scaling_v);
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_defog(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg, IPE_ENG_DMA_OUT_INFO* p_single_out) //dual ver
{
	KDRV_IPE_DEFOG_PARAM *p_defog_info;
	KDRV_IPE_SUBIMG_PARAM *p_sub_img_info;
	KDRV_IPE_IO_CFG* p_io_cfg = NULL;

	if(p_hdl == NULL){
		DBG_ERR("hdl == NULL\n");
		return E_PAR;
	}

	if(p_hdl->p_eng == NULL ){
		DBG_ERR("eng == NULL\n");
		return E_PAR;
	}

	p_defog_info = &p_cfg->p_iqcfg->defog_param;
	p_sub_img_info = &p_cfg->p_iqcfg->defog_subimg_param;
	p_io_cfg = p_cfg->p_iocfg;

	/* Defog I/O control */
	if 	(p_defog_info->enable) {
		if(p_cfg->p_iocfg->defog_sub_in_addr.pa != 0 && p_cfg->p_iocfg->defog_sub_in_addr.pa != (ULONG)-1){

			{
				UINT64 addr = p_cfg->p_iocfg->defog_sub_in_addr.pa;
				ipe_eng_set_dma_in_defog_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
			}
		}
		else{
			DBG_ERR("dma_in_defog_addr:0x%lx invalid\n", p_cfg->p_iocfg->defog_sub_in_addr.pa);
			return E_PAR;
		}
	}

	if (p_cfg->p_iocfg->defog_sub_out_en) {
		if(p_cfg->p_iocfg->defog_sub_out_addr.pa != 0 && p_cfg->p_iocfg->defog_sub_out_addr.pa != (ULONG)-1){

			{
				UINT64 addr = p_cfg->p_iocfg->defog_sub_out_addr.pa;
				ipe_eng_set_dma_out_defog_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
			}
		}
		else{
			DBG_ERR("dma_out_defog_addr:0x%lx invalid\n", p_cfg->p_iocfg->defog_sub_out_addr.pa);
			return E_PAR;
		}

		ipe_eng_set_defog_subout_enable_buf_reg(p_hdl->p_eng, ENABLE);
		p_single_out->defog_en = ENABLE;
	} else {
		ipe_eng_set_defog_subout_enable_buf_reg(p_hdl->p_eng, DISABLE);
	}

	if (p_cfg->p_iqcfg->update & KDRV_IPE_UPDATE_DEFOG) {
		if (p_defog_info->enable) {

			UINT16 air_nfactor = 0; // NORMFACTOR
			IPE_DEFOGROUND dfg_round = {0};

			ipe_eng_set_defog_strength_buf_reg(p_hdl->p_eng, p_defog_info->dfg_strength.str_mode_sel,p_defog_info->dfg_strength.fog_ratio,
												p_defog_info->dfg_strength.dgain_ratio,p_defog_info->dfg_strength.gain_th);
			ipe_eng_set_defog_fog_target_lut_buf_reg(p_hdl->p_eng, &p_defog_info->dfg_strength.target_lut[0]);

			ipe_eng_set_defog_fog_mod_buf_reg(p_hdl->p_eng, &p_defog_info->env_estimation.fog_mod_lut[0]);
			ipe_eng_set_defog_fog_protect_buf_reg(p_hdl->p_eng, p_defog_info->env_estimation.dfg_self_comp_en,p_defog_info->env_estimation.dfg_min_diff);
			ipe_eng_set_defog_atmospherelight_buf_reg(p_hdl->p_eng, p_defog_info->env_estimation.dfg_airlight[0],
														p_defog_info->env_estimation.dfg_airlight[1],
														p_defog_info->env_estimation.dfg_airlight[2]);

			ipe_eng_set_defog_scal_edgeinterp_buf_reg(p_hdl->p_eng, p_defog_info->scalup_param.interp_wdist,p_defog_info->scalup_param.interp_wout,
														p_defog_info->scalup_param.interp_wcenter,
														p_defog_info->scalup_param.interp_wsrc);
			ipe_eng_set_dfg_scalg_edgeinterplut_buf_reg(p_hdl->p_eng, &p_defog_info->scalup_param.interp_diff_lut[0]);


			air_nfactor = ipe_eng_cal_defog_airlight_nfactor(p_cfg->p_iocfg->defog_info.hw_airlight);

			ipe_eng_set_defog_outbld_lum_lut_buf_reg(p_hdl->p_eng, p_defog_info->dfg_outbld.outbld_ref_sel, &p_defog_info->dfg_outbld.outbld_lum_wt[0]);
			ipe_eng_set_defog_outbld_diff_lut_buf_reg(p_hdl->p_eng, &p_defog_info->dfg_outbld.outbld_diff_wt[0]);
			ipe_eng_set_defog_outbld_local_buf_reg(p_hdl->p_eng, p_defog_info->dfg_outbld.outbld_local_en,air_nfactor);

			dfg_round.rand_opt = p_defog_info->dfg_round.rand_opt;
			dfg_round.rand_rst = p_defog_info->dfg_round.rand_rst;
			ipe_eng_set_defog_rand_buf_reg(p_hdl->p_eng, &dfg_round);

			ipe_eng_set_dfg_input_bld_buf_reg(p_hdl->p_eng, &p_defog_info->input_bld.in_blend_wt[0]);

			ipe_eng_set_defog_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_defog_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	if (p_cfg->p_iqcfg->update2 & KDRV_IPE_UPDATE2_DEFOG_SUBIMG) {
		IPE_IMG_SIZE defog_size;
		UINT32 blk_size_h, blk_size_v, blk_cent_fact_h, blk_cent_fact_v;
		UINT32 scaling_h, scaling_v;

		defog_size.h_size = p_sub_img_info->h_size;
		defog_size.v_size = p_sub_img_info->v_size;
		ipe_eng_set_dma_out_defog_offset_buf_reg(p_hdl->p_eng, p_sub_img_info->subimg_lofs_out, p_io_cfg->defog_subout_addr_ofs);
		ipe_eng_set_dma_in_defog_offset_buf_reg(p_hdl->p_eng, p_sub_img_info->subimg_lofs_in,  p_io_cfg->defog_subin_addr_ofs);
		ipe_eng_set_dfg_subimg_size_buf_reg(p_hdl->p_eng, defog_size);
		ipe_eng_set_dfg_scal_filtcoeff_buf_reg(p_hdl->p_eng, &p_sub_img_info->subimg_ftrcoef[0]); //tmp early porting need to check ftcoef

		/* auto calculate */
		blk_size_h = ((  (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1) / (p_sub_img_info->h_size)));
		blk_size_v = (((p_cfg->p_iocfg->in_frm.size.h - 1) / (p_sub_img_info->v_size)));
		blk_cent_fact_h = (((   (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1) << 12) / (p_sub_img_info->h_size - 1)));
		blk_cent_fact_v = ((((p_cfg->p_iocfg->in_frm.size.h - 1) << 12) / (p_sub_img_info->v_size - 1)));
		ipe_eng_set_dfg_subout_param_buf_reg(p_hdl->p_eng, blk_size_h, blk_cent_fact_h, blk_size_v, blk_cent_fact_v);

		scaling_h = ((((p_sub_img_info->h_size - 1) << 16) /  (p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width - 1)));
		scaling_v = ((((p_sub_img_info->v_size - 1) << 16) / (p_cfg->p_iocfg->in_frm.size.h - 1)));
		ipe_eng_set_dfg_scal_factor_buf_reg(p_hdl->p_eng, scaling_h, scaling_v);
	}
	return E_OK;
}
#endif

static INT32 kdrv_ipe_int_cfg_eext(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_EEXT) {
		ipe_eng_set_edge_extract_kernel_b_buf_reg(p_hdl->p_eng, &p_iq_cfg->eext.edge_ker[0], p_iq_cfg->eext.edge_ker[10], p_iq_cfg->eext.edge_ker[11]);

		{
			IPE_EEXT_KER_STRENGTH eext_kerstrength;
			eext_kerstrength.ker_a.eext_div = p_iq_cfg->eext.eext_kerstrength.ker_freq0.eext_div;
			eext_kerstrength.ker_a.eext_enh = p_iq_cfg->eext.eext_kerstrength.ker_freq0.eext_enh;
			eext_kerstrength.ker_c.eext_div = p_iq_cfg->eext.eext_kerstrength.ker_freq1.eext_div;
			eext_kerstrength.ker_c.eext_enh = p_iq_cfg->eext.eext_kerstrength.ker_freq1.eext_enh;
			eext_kerstrength.ker_d.eext_div = p_iq_cfg->eext.eext_kerstrength.ker_freq2.eext_div;
			eext_kerstrength.ker_d.eext_enh = p_iq_cfg->eext.eext_kerstrength.ker_freq2.eext_enh;
			ipe_eng_set_eext_kerstrength_buf_reg(p_hdl->p_eng, &eext_kerstrength);
		}

		{
			IPE_EEXT_ENG_CON eext_engcon;
			eext_engcon.eext_div_con = p_iq_cfg->eext.eext_engcon.eext_div_con;
			eext_engcon.eext_div_eng = p_iq_cfg->eext.eext_engcon.eext_div_eng;
			eext_engcon.wt_con_eng = p_iq_cfg->eext.eext_engcon.wt_con_eng;
			ipe_eng_set_eext_engcon_buf_reg(p_hdl->p_eng, &eext_engcon);
		}

		{
			IPE_KER_THICKNESS kdr_thickness;
			kdr_thickness.iso_ker_robust = p_iq_cfg->eext.ker_thickness.iso_ker_robust;
			kdr_thickness.iso_ker_thin = p_iq_cfg->eext.ker_thickness.iso_ker_thin;
			kdr_thickness.wt_ker_robust = p_iq_cfg->eext.ker_thickness.wt_ker_robust;
			kdr_thickness.wt_ker_thin = p_iq_cfg->eext.ker_thickness.wt_ker_thin;
			ipe_eng_set_ker_thickness_buf_reg(p_hdl->p_eng, &kdr_thickness);
		}

		{
			IPE_KER_THICKNESS kdr_thickness_hld;
			kdr_thickness_hld.iso_ker_robust = p_iq_cfg->eext.ker_thickness_hld.iso_ker_robust;
			kdr_thickness_hld.iso_ker_thin = p_iq_cfg->eext.ker_thickness_hld.iso_ker_thin;
			kdr_thickness_hld.wt_ker_robust = p_iq_cfg->eext.ker_thickness_hld.wt_ker_robust;
			kdr_thickness_hld.wt_ker_thin = p_iq_cfg->eext.ker_thickness_hld.wt_ker_thin;
			ipe_eng_set_ker_thickness_hld_buf_reg(p_hdl->p_eng, &kdr_thickness_hld);
		}

		{
			IPE_REGION_PARAM region = {0};
			region.wt_low = p_iq_cfg->eext.eext_region.reg_wt.wt_low;
			region.wt_high = p_iq_cfg->eext.eext_region.reg_wt.wt_high;
			region.wt_high_hld = p_iq_cfg->eext.eext_region.reg_wt.wt_high_hld;
			region.wt_low_hld = p_iq_cfg->eext.eext_region.reg_wt.wt_low_hld;
			region.th_edge = p_iq_cfg->eext.eext_region.reg_th.th_edge;
			region.th_edge_hld = p_iq_cfg->eext.eext_region.reg_th.th_edge_hld;
			region.th_flat = p_iq_cfg->eext.eext_region.reg_th.th_flat;
			region.th_flat_hld = p_iq_cfg->eext.eext_region.reg_th.th_flat_hld;
			region.th_lum_hld = p_iq_cfg->eext.eext_region.reg_th.th_lum_hld;
			ipe_eng_set_region_buf_reg(p_hdl->p_eng, &region);
		}

		{
			INT16 slope_con_eng, slope_con_eng_hld, wlow, whigh, th_edge, th_flat;

			wlow = p_iq_cfg->eext.eext_region.reg_wt.wt_low;
			whigh = p_iq_cfg->eext.eext_region.reg_wt.wt_high;
			th_edge = p_iq_cfg->eext.eext_region.reg_th.th_edge;
			th_flat = p_iq_cfg->eext.eext_region.reg_th.th_flat;
			slope_con_eng = (th_edge == th_flat) ? 65535 : (((whigh -  wlow) * 1024) / IPE_ABS(th_edge - th_flat));

			wlow = p_iq_cfg->eext.eext_region.reg_wt.wt_low_hld;
			whigh = p_iq_cfg->eext.eext_region.reg_wt.wt_high_hld;
			th_edge = p_iq_cfg->eext.eext_region.reg_th.th_edge_hld;
			th_flat = p_iq_cfg->eext.eext_region.reg_th.th_flat_hld;
			slope_con_eng_hld = (th_edge == th_flat) ? 65535 : (((whigh -  wlow) * 1024) / IPE_ABS(th_edge - th_flat));
			ipe_eng_set_region_slope_buf_reg(p_hdl->p_eng, slope_con_eng,slope_con_eng_hld);
		}

		{
			IPE_EDGE_KER_BLENDING eext_blending = {0};

			eext_blending.eext_blending_slope = p_iq_cfg->eext.eext_blending.eext_blending_slope;
			eext_blending.eext_blending_th = p_iq_cfg->eext.eext_blending.eext_blending_th;
			eext_blending.eext_blending_w1 = p_iq_cfg->eext.eext_blending.eext_blending_w1;
			eext_blending.eext_blending_w2 = p_iq_cfg->eext.eext_blending.eext_blending_w2;

			ipe_eng_set_eext_blending_buf_reg(p_hdl->p_eng, &eext_blending);
		}

		//538 add
		{
			IPE_EEXT_DIR_KER_STRENGTH eext_dir_kerstrength = {0};

			eext_dir_kerstrength.ker_h.eext_enh = p_iq_cfg->eext.dir_ker_strength.ker_h.eext_enh;
			eext_dir_kerstrength.ker_v.eext_enh = p_iq_cfg->eext.dir_ker_strength.ker_v.eext_enh;
			eext_dir_kerstrength.ker_d1.eext_enh = p_iq_cfg->eext.dir_ker_strength.ker_d1.eext_enh;
			eext_dir_kerstrength.ker_d2.eext_enh = p_iq_cfg->eext.dir_ker_strength.ker_d2.eext_enh;
			eext_dir_kerstrength.ker_h.eext_div = p_iq_cfg->eext.dir_ker_strength.ker_h.eext_div;
			eext_dir_kerstrength.ker_v.eext_div = p_iq_cfg->eext.dir_ker_strength.ker_v.eext_div;
			eext_dir_kerstrength.ker_d1.eext_div= p_iq_cfg->eext.dir_ker_strength.ker_d1.eext_div;
			eext_dir_kerstrength.ker_d2.eext_div= p_iq_cfg->eext.dir_ker_strength.ker_d2.eext_div;

			ipe_eng_set_eext_dir_kerstrength_buf_reg(p_hdl->p_eng, &eext_dir_kerstrength);
		}

		{
			IPE_EEXT_DIR_KER_PARAM eext_dir_ker_para = {0};

			eext_dir_ker_para.eext_dir_min_s_low_bound= p_iq_cfg->eext.dir_ker_para.eext_dir_min_s_low_bound;
			eext_dir_ker_para.eext_dir_s_th_mul= p_iq_cfg->eext.dir_ker_para.eext_dir_s_th_mul;
			eext_dir_ker_para.eext_dir_s_th_shift= p_iq_cfg->eext.dir_ker_para.eext_dir_s_th_shift;
			eext_dir_ker_para.dir_eng_blend_w1 = p_iq_cfg->eext.dir_ker_para.dir_eng_blend_w1;
			eext_dir_ker_para.dir_eng_blend_w2= p_iq_cfg->eext.dir_ker_para.dir_eng_blend_w2;
			eext_dir_ker_para.dir_eng_blend_w3 = p_iq_cfg->eext.dir_ker_para.dir_eng_blend_w3;
			eext_dir_ker_para.dir_eng_blend_w4= p_iq_cfg->eext.dir_ker_para.dir_eng_blend_w4;
			eext_dir_ker_para.s_count_th= p_iq_cfg->eext.dir_ker_para.s_count_th;

			ipe_eng_set_eext_dir_param_buf_reg(p_hdl->p_eng, &eext_dir_ker_para);
		}
		 //539A add
		{
			IPE_ENG_MOTION_SYMBOL_PARAM motion_symbol_para = {0};
			IPE_ENG_MOTION_STR_PARAM motion_str_para = {0};

			motion_symbol_para.ipe_motion_type = p_cfg->p_iocfg->motion_type;

			motion_symbol_para.ipe_static_symbol = p_iq_cfg->eext.motion_symbol.static_symbol;
			motion_symbol_para.ipe_transition_symbol = p_iq_cfg->eext.motion_symbol.transition_symbol;
			motion_symbol_para.ipe_motion_symbol = p_iq_cfg->eext.motion_symbol.motion_symbol;

			motion_str_para.ipe_static_str = p_iq_cfg->eext.motion_str.static_str;
			motion_str_para.ipe_transition_str = p_iq_cfg->eext.motion_str.transition_str;
			motion_str_para.ipe_motion_str = p_iq_cfg->eext.motion_str.motion_str;

			ipe_eng_set_motion_enable_buf_reg(p_hdl->p_eng, p_iq_cfg->eext.motion_enable);
			ipe_eng_set_motion_symbol_buf_reg(p_hdl->p_eng, &motion_symbol_para);
			ipe_eng_set_motion_str_buf_reg(p_hdl->p_eng, &motion_str_para);


			/* flush address in ctrl ipp isp */
			if(p_iq_cfg->eext.motion_enable) {
				if(p_cfg->p_iocfg->edge_motion_addr.pa != 0 && p_cfg->p_iocfg->edge_motion_addr.pa != (ULONG)-1){

					{
						UINT64 addr = p_cfg->p_iocfg->edge_motion_addr.pa;

						ipe_eng_set_dma_in_motion_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF)); // check addr %4
					}

					ipe_eng_set_dma_in_motion_offset_buf_reg(p_hdl->p_eng, p_cfg->p_iocfg->edge_motion_in_lofs, p_cfg->p_iocfg->edge_motion_in_addr_lofs);
				}
				else{
					DBG_ERR("edge_motion_addr:0x%lx invalid\n",p_cfg->p_iocfg->edge_motion_addr.pa);
					return E_PAR;
				}
			}

			ipe_eng_set_power_save_edge_buf_reg(p_hdl->p_eng, p_iq_cfg->eext.edge_ker_mode,  p_iq_cfg->eext.edge_mode);

		}


	}

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_EEXT_TONEMAP) {
		if(p_cfg->p_iocfg->mode == KDRV_IPP_OPMODE_D2D) {
			ipe_eng_set_edge_extract_sel_buf_reg(p_hdl->p_eng, p_iq_cfg->eext_tonemap.gamma_sel, KDRV_IPE_EEXT_G_CHANNEL);
		} else {
			ipe_eng_set_edge_extract_sel_buf_reg(p_hdl->p_eng, p_iq_cfg->eext_tonemap.gamma_sel, KDRV_IPE_EEXT_Y_CHANNEL);
		}
		ipe_eng_set_tone_remap_buf_reg(p_hdl->p_eng, &p_iq_cfg->eext_tonemap.tone_map_lut[0]);
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_overshoot(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_OVERSHOOT) {
		/* spilit api*/
		IPE_OVERSHOOT_PARAM overshoot;
		overshoot.overshoot_en = p_iq_cfg->edge_overshoot.overshoot_en;
		overshoot.clamp_wt_mod_eng = p_iq_cfg->edge_overshoot.clamp_wt_mod_eng;
		overshoot.clamp_wt_mod_lum = p_iq_cfg->edge_overshoot.clamp_wt_mod_lum;
		overshoot.norm_lum_eng = p_iq_cfg->edge_overshoot.norm_lum_eng;
		overshoot.slope_overshoot = p_iq_cfg->edge_overshoot.slope_overshoot;
		overshoot.slope_undershoot = p_iq_cfg->edge_overshoot.slope_undershoot;
		overshoot.slope_undershoot_eng = p_iq_cfg->edge_overshoot.slope_undershoot_eng;
		overshoot.slope_undershoot_lum = p_iq_cfg->edge_overshoot.slope_undershoot_lum;
		overshoot.strength_lum_eng = p_iq_cfg->edge_overshoot.strength_lum_eng;
		overshoot.th_overshoot = p_iq_cfg->edge_overshoot.th_overshoot;
		overshoot.th_undershoot = p_iq_cfg->edge_overshoot.th_undershoot;
		overshoot.th_undershoot_eng = p_iq_cfg->edge_overshoot.th_undershoot_eng;
		overshoot.th_undershoot_lum = p_iq_cfg->edge_overshoot.th_undershoot_lum;
		overshoot.wt_overshoot = p_iq_cfg->edge_overshoot.wt_overshoot;
		overshoot.wt_undershoot = p_iq_cfg->edge_overshoot.wt_undershoot;
		ipe_eng_set_overshoot_buf_reg(p_hdl->p_eng, &overshoot);
	}

	return E_OK;
}
static INT32 kdrv_ipe_int_cfg_eproc(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_EPROC) {
		{
			IPE_ESMAP_INFOR esmap;
			esmap.etab_high = p_iq_cfg->eproc.es_map_th.etab_high;
			esmap.etab_low = p_iq_cfg->eproc.es_map_th.etab_low;
			esmap.ethr_high = p_iq_cfg->eproc.es_map_th.ethr_high;
			esmap.ethr_low = p_iq_cfg->eproc.es_map_th.ethr_low;
			ipe_eng_set_esmap_thresholds_buf_reg(p_hdl->p_eng, &esmap);
		}

		{
			IPE_EDGEMAP_INFOR edmap;
			edmap.ein_sel = p_iq_cfg->eproc.edge_map_th.map_sel;
			edmap.etab_high = p_iq_cfg->eproc.edge_map_th.etab_high;
			edmap.etab_low = p_iq_cfg->eproc.edge_map_th.etab_low;
			edmap.ethr_high = p_iq_cfg->eproc.edge_map_th.ethr_high;
			edmap.ethr_low = p_iq_cfg->eproc.edge_map_th.ethr_low;
			ipe_eng_set_edgemap_thresholds_buf_reg(p_hdl->p_eng, &edmap);
		}
		ipe_eng_set_estab_buf_reg(p_hdl->p_eng, &p_iq_cfg->eproc.es_map_lut[0]);
		ipe_eng_set_edgemap_tab_buf_reg(p_hdl->p_eng, &p_iq_cfg->eproc.edge_map_lut[0]);
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_cctrl(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CCTRL) {
		if (p_iq_cfg->cctrl.enable) {
			/*api name do not relate to funtion */
			ipe_eng_set_int_sat_offset_buf_reg(p_hdl->p_eng, p_iq_cfg->cctrl.int_ofs, p_iq_cfg->cctrl.sat_ofs);
			ipe_eng_set_hue_rotate_enable_buf_reg(p_hdl->p_eng, p_iq_cfg->cctrl.hue_rotate_en);
			ipe_eng_set_color_suppress_buf_reg(p_hdl->p_eng, p_iq_cfg->cctrl.cctrl_sel, p_iq_cfg->cctrl.vdet_div);
			ipe_eng_set_hue_c2g_enable_buf_reg(p_hdl->p_eng, p_iq_cfg->cctrl.hue_c2g);
			ipe_eng_set_cctrl_hue_buf_reg(p_hdl->p_eng, &p_iq_cfg->cctrl.hue_tab[0]);
			ipe_eng_set_cctrl_int_buf_reg(p_hdl->p_eng, &p_iq_cfg->cctrl.int_tab[0]);
			ipe_eng_set_cctrl_sat_buf_reg(p_hdl->p_eng, &p_iq_cfg->cctrl.sat_tab[0]);
			ipe_eng_set_cctrl_edge_buf_reg(p_hdl->p_eng, &p_iq_cfg->cctrl.edge_tab[0]);
			ipe_eng_set_cctrl_dds_tab_buf_reg(p_hdl->p_eng, &p_iq_cfg->cctrl.dds_tab[0]);
			ipe_eng_set_color_ctrl_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_color_ctrl_enable_buf_reg(p_hdl->p_eng, DISABLE);
			ipe_eng_set_hue_rotate_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_cadj(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CADJ_EE) {
		if (p_iq_cfg->cadj_ee.enable) {
			ipe_eng_set_edge_enhance_buf_reg(p_hdl->p_eng, p_iq_cfg->cadj_ee.edge_enh_p, p_iq_cfg->cadj_ee.edge_enh_n);
			ipe_eng_set_edge_invert_buf_reg(p_hdl->p_eng, p_iq_cfg->cadj_ee.edge_inv_p_en, p_iq_cfg->cadj_ee.edge_inv_n_en);
			ipe_eng_set_cadj_y_edge_enh_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_cadj_y_edge_enh_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CADJ_YCCON) {
		if (p_iq_cfg->cadj_yccon.enable) {
			ipe_eng_set_y_contrast_buf_reg(p_hdl->p_eng, p_iq_cfg->cadj_yccon.y_con);
			ipe_eng_set_cbcr_contab_buf_reg(p_hdl->p_eng, &p_iq_cfg->cadj_yccon.cconlut[0]);
			ipe_eng_set_cbcr_contrast_buf_reg(p_hdl->p_eng, p_iq_cfg->cadj_yccon.c_con);
			ipe_eng_set_cbcr_contab_sel_buf_reg(p_hdl->p_eng, p_iq_cfg->cadj_yccon.ccontab_sel);
			ipe_eng_set_cadj_y_cb_ctrl_enable_buf_reg(p_hdl->p_eng, ENABLE);
			ipe_eng_set_cadj_uv_cb_ctrl_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_cadj_y_cb_ctrl_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CADJ_COFS) {
		if (p_iq_cfg->cadj_cofs.enable) {
			ipe_eng_set_cbcr_offset_buf_reg(p_hdl->p_eng, p_iq_cfg->cadj_cofs.cb_ofs, p_iq_cfg->cadj_cofs.cr_ofs);
		} else {
			ipe_eng_set_cbcr_offset_buf_reg(p_hdl->p_eng, 0x80, 0x80);
		}
	}

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CADJ_RAND) {
		IPE_CADJ_RAND_PARAM cadj_rand;
		if (p_iq_cfg->cadj_rand.enable) {
			cadj_rand.crand_en = p_iq_cfg->cadj_rand.rand_en_c;
			cadj_rand.yrand_en = p_iq_cfg->cadj_rand.rand_en_y;
			cadj_rand.crand_level = p_iq_cfg->cadj_rand.rand_level_c;
			cadj_rand.yrand_level = p_iq_cfg->cadj_rand.rand_level_y;
			cadj_rand.yc_rand_rst = p_iq_cfg->cadj_rand.rand_reset;
		} else {
			cadj_rand.crand_en = 0;
			cadj_rand.yrand_en = 0;
			cadj_rand.crand_level = 0;
			cadj_rand.yrand_level = 0;
			cadj_rand.yc_rand_rst = 0;
		}
		ipe_eng_set_yc_rand_buf_reg(p_hdl->p_eng, &cadj_rand);
	}

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CADJ_HUE) {
		if(p_iq_cfg->cadj_hue.enable) {
			ipe_eng_set_hue_adj_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_hue_adj_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	/*separate api ??????*/
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CADJ_FIXTH) {
		if (p_iq_cfg->cadj_fixth.enable) {
			{
				STR_YTH1_INFOR yth1;
				STR_YTH2_INFOR yth2;

				yth1.y_th = p_iq_cfg->cadj_fixth.yth1.y_th;
				yth1.hit_sel = p_iq_cfg->cadj_fixth.yth1.ycth_sel_hit;
				yth1.hit_value = p_iq_cfg->cadj_fixth.yth1.value_hit;
				yth1.nhit_sel = p_iq_cfg->cadj_fixth.yth1.ycth_sel_nonhit;
				yth1.nhit_value = p_iq_cfg->cadj_fixth.yth1.nonvalue_hit;
				yth1.edgeth = p_iq_cfg->cadj_fixth.yth1.edge_th;
				yth2.hit_sel = p_iq_cfg->cadj_fixth.yth2.ycth_sel_hit;
				yth2.hit_value = p_iq_cfg->cadj_fixth.yth2.value_hit;
				yth2.nhit_sel = p_iq_cfg->cadj_fixth.yth2.ycth_sel_nonhit;
				yth2.nhit_value = p_iq_cfg->cadj_fixth.yth2.nonvalue_hit;
				yth2.y_th = p_iq_cfg->cadj_fixth.yth2.y_th;
				ipe_eng_set_yfix_th_buf_reg(p_hdl->p_eng, &yth1, &yth2);
			}

			{
				STR_CTH_INFOR cth;

				cth.cbth_high = p_iq_cfg->cadj_fixth.cth.cb_th_high;
				cth.cbth_low = p_iq_cfg->cadj_fixth.cth.cb_th_low;
				cth.cb_hit_value = p_iq_cfg->cadj_fixth.cth.cb_value_hit;
				cth.cb_nhit_value = p_iq_cfg->cadj_fixth.cth.cb_value_nonhit;
				cth.crth_high = p_iq_cfg->cadj_fixth.cth.cr_th_high;
				cth.crth_low = p_iq_cfg->cadj_fixth.cth.cr_th_low;
				cth.cr_nhit_value = p_iq_cfg->cadj_fixth.cth.cr_value_nonhit;
				cth.cr_hit_value = p_iq_cfg->cadj_fixth.cth.cr_value_hit;
				cth.edgeth = p_iq_cfg->cadj_fixth.cth.edge_th;
				cth.hit_sel = p_iq_cfg->cadj_fixth.cth.ycth_sel_hit;
				cth.nhit_sel = p_iq_cfg->cadj_fixth.cth.ycth_sel_nonhit;
				cth.yth_high = p_iq_cfg->cadj_fixth.cth.y_th_high;
				cth.yth_low = p_iq_cfg->cadj_fixth.cth.y_th_low;
				ipe_eng_set_cbcr_fixth_buf_reg(p_hdl->p_eng, &cth);
			}
			ipe_eng_set_cadj_y_uv_th_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_cadj_y_uv_th_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CADJ_MASK) {
		if (p_iq_cfg->cadj_mask.enable) {
			ipe_eng_set_yc_mask_buf_reg(p_hdl->p_eng, p_iq_cfg->cadj_mask.y_mask, p_iq_cfg->cadj_mask.cb_mask, p_iq_cfg->cadj_mask.cr_mask);
		} else {
			ipe_eng_set_yc_mask_buf_reg(p_hdl->p_eng, 0xff, 0xff, 0xff);
		}
	}

	if (p_iq_cfg->update & (KDRV_IPE_UPDATE_CADJ_EE | KDRV_IPE_UPDATE_CADJ_YCCON | KDRV_IPE_UPDATE_CADJ_COFS | KDRV_IPE_UPDATE_CADJ_RAND | KDRV_IPE_UPDATE_CADJ_FIXTH | KDRV_IPE_UPDATE_CADJ_MASK)) {
		if (p_iq_cfg->cadj_ee.enable || p_iq_cfg->cadj_yccon.enable || p_iq_cfg->cadj_cofs.enable
			|| p_iq_cfg->cadj_rand.enable || p_iq_cfg->cadj_fixth.enable || p_iq_cfg->cadj_mask.enable) {
			ipe_eng_set_cadj_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_cadj_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	return E_OK;
}


static INT32 kdrv_ipe_int_cfg_gamyrand(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_GAMYRAND) {
		if (p_iq_cfg->gamy_rand.enable) {
			if (p_iq_cfg->gamma.enable || p_iq_cfg->y_curve.enable) {
				IPE_GAMYRAND gamy_rand;
				gamy_rand.gam_y_rand_en = p_iq_cfg->gamy_rand.rand_en;
				gamy_rand.gam_y_rand_rst = p_iq_cfg->gamy_rand.rst_en;
				gamy_rand.gam_y_rand_shft = p_iq_cfg->gamy_rand.rand_shift;
				ipe_eng_set_gam_y_rand_buf_reg(p_hdl->p_eng, &gamy_rand);
			} else {
				DBG_WRN("PARAM_GAMYRAND only support when PARAM_GAMMA or PARAM_YCURVE enable\r\n");
			}
		}
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_gamma(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_GAMMA) {
		if(p_iq_cfg->gamma.enable) {
			/* flush address in ctrl ipp isp */
			if(p_cfg->p_iocfg->gamma_addr.pa != 0 && p_cfg->p_iocfg->gamma_addr.pa != (ULONG)-1){

				{
					UINT64 addr = p_cfg->p_iocfg->gamma_addr.pa;

					ipe_eng_set_dma_in_gamma_lut_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF)); // check addr %4
				}
			}
			else{
				DBG_ERR("gamma_addr:0x%lx invalid\n",p_cfg->p_iocfg->gamma_addr.pa);
				return E_PAR;
			}

			//DBG_DUMP("-<<<<<<<<<<gamma<<<<<<<<< [%lx,%lx] >>>>>>>>>>>>>>>>>>>>>>-\n", p_cfg->p_iocfg->gamma_addr.pa, p_cfg->p_iocfg->gamma_addr.va  );
			vos_cpu_dcache_sync((ULONG)p_cfg->p_iocfg->gamma_addr.va , ALIGN_CEIL_32(sizeof(ULONG) * KDRV_IPE_IQ_GAMMA_TRANS_LEN) , VOS_DMA_TO_DEVICE);
			ipe_eng_set_gamma_enable_buf_reg(p_hdl->p_eng, ENABLE);
			//539A add
			ipe_eng_set_sram_sd_gamma_buf_reg(p_hdl->p_eng, DISABLE);

		} else {
			ipe_eng_set_gamma_enable_buf_reg(p_hdl->p_eng, DISABLE);
			//539A add
			if (kdrv_ipe_typecast_opmode(p_cfg->p_iocfg->mode) != IPE_ENG_OPMODE_ALL_DIRECT ||(kdrv_ipe_typecast_opmode(p_cfg->p_iocfg->mode) == IPE_ENG_OPMODE_ALL_DIRECT && p_cfg->p_iocfg->in_job_num == 1)) { //direct only start job can shutdown
				ipe_eng_set_sram_sd_gamma_buf_reg(p_hdl->p_eng, ENABLE);
			}
		}
	}
	return E_OK;
}

//538 add
static INT32 kdrv_ipe_int_cfg_3dcc(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;

	if (p_iq_cfg->update2 & KDRV_IPE_UPDATE2_3DCC) {
		if(p_iq_cfg->_3dcc.enable) {

			/* flush address in ctrl ipp isp */
			if(p_cfg->p_iocfg->_3dcc_addr.pa != 0 && p_cfg->p_iocfg->_3dcc_addr.pa != (ULONG)-1){

				{
					UINT64 addr = p_cfg->p_iocfg->_3dcc_addr.pa;

					ipe_eng_set_dma_in_3dcc_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF)); // check addr %4
				}
			}
			else{
				DBG_ERR("3dcc_addr:0x%lx invalid\n",p_cfg->p_iocfg->_3dcc_addr.pa);
				return E_PAR;
			}

			//DBG_DUMP("-<<<<<<<<<<gamma<<<<<<<<< [%lx,%lx] >>>>>>>>>>>>>>>>>>>>>>-\n", p_cfg->p_iocfg->gamma_addr.pa, p_cfg->p_iocfg->gamma_addr.va  );
			vos_cpu_dcache_sync((ULONG)p_cfg->p_iocfg->_3dcc_addr.va , ALIGN_CEIL_32(sizeof(ULONG) * KDRV_IPE_IQ_3DCC_TRANS_LEN) , VOS_DMA_TO_DEVICE);
			ipe_eng_set_3dcc_enable_buf_reg(p_hdl->p_eng, ENABLE);
			//539A add
			ipe_eng_set_3dcc_mode_buf_reg(p_hdl->p_eng, p_iq_cfg->_3dcc.mode);
			ipe_eng_set_sram_sd_cc3d_buf_reg(p_hdl->p_eng, DISABLE);
		} else {
			ipe_eng_set_3dcc_enable_buf_reg(p_hdl->p_eng, DISABLE);
			//539A add
			if (kdrv_ipe_typecast_opmode(p_cfg->p_iocfg->mode) != IPE_ENG_OPMODE_ALL_DIRECT ||(kdrv_ipe_typecast_opmode(p_cfg->p_iocfg->mode) == IPE_ENG_OPMODE_ALL_DIRECT && p_cfg->p_iocfg->in_job_num == 1)) { //direct only start job can shutdown
				ipe_eng_set_sram_sd_cc3d_buf_reg(p_hdl->p_eng, ENABLE);
			}
		}
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_ycurve(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_YCURVE) {

		p_hdl->ycurve_en = p_iq_cfg->y_curve.enable; // 690 patch. save ycurve enable

		if (p_iq_cfg->y_curve.enable) {
			/* flush address in ctrl ipp isp */
			ipe_eng_set_ycurve_sel_buf_reg(p_hdl->p_eng, p_iq_cfg->y_curve.ycurve_sel);
			if(p_cfg->p_iocfg->ycurve_addr.pa != 0 && p_cfg->p_iocfg->ycurve_addr.pa != (ULONG)-1){

				{
					UINT64 addr = p_cfg->p_iocfg->ycurve_addr.pa;
					ipe_eng_set_dma_in_ycurve_lut_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF)); // check addr %4

					p_hdl->ycurve_msb = (UINT32)((addr>> 32) & 0xFFFFFFFF); // 690 patch. save ycurve lut addr msb
				}
			}
			else{
				DBG_ERR("ycurve_addr:0x%lx invalid\n",p_cfg->p_iocfg->ycurve_addr.pa);
				return E_PAR;
			}

			//DBG_DUMP("-<<<<<<<<ycurve<<<<<<<<<<< [%lx,%lx] >>>>>>>>>>>>>>>>>>>>>>-\n", p_cfg->p_iocfg->ycurve_addr.pa, p_cfg->p_iocfg->ycurve_addr.va  );
			vos_cpu_dcache_sync((ULONG)p_cfg->p_iocfg->ycurve_addr.va, ALIGN_CEIL_32(sizeof(ULONG) * KDRV_IPE_IQ_YCURVE_TRANS_LEN) , VOS_DMA_TO_DEVICE);
			ipe_eng_set_ycurve_enable_buf_reg(p_hdl->p_eng, ENABLE);
			//539A add
			ipe_eng_set_sram_sd_ycurve_buf_reg(p_hdl->p_eng, DISABLE);

		} else {
			ipe_eng_set_ycurve_enable_buf_reg(p_hdl->p_eng, DISABLE);
			//539A add
			if (kdrv_ipe_typecast_opmode(p_cfg->p_iocfg->mode) != IPE_ENG_OPMODE_ALL_DIRECT ||(kdrv_ipe_typecast_opmode(p_cfg->p_iocfg->mode) == IPE_ENG_OPMODE_ALL_DIRECT && p_cfg->p_iocfg->in_job_num == 1)) { //direct only start job can shutdown
				ipe_eng_set_sram_sd_ycurve_buf_reg(p_hdl->p_eng, ENABLE);
			}
		}
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_cst(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CST) {
		if (p_iq_cfg->cst.enable){
			IPE_CST_PARAM cst_param = {0};

			cst_param.p_cst_coeff = p_iq_cfg->cst.cst_coef;
			if (p_iq_cfg->cst.cst_off_sel == 0) {
				cst_param.cstoff_sel = 0;
			} else if (p_iq_cfg->cst.cst_off_sel == 1) {
				cst_param.cstoff_sel = 1;
			} else {
				DBG_ERR("unsupport cst offset %d\r\n", p_iq_cfg->cst.cst_off_sel);
			}
			ipe_eng_set_cst_buf_reg(p_hdl->p_eng, &cst_param);
			ipe_eng_set_cst_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_cst_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CSTP) {
		if (p_iq_cfg->cstp.enable) {
			ipe_eng_set_cstp_buf_reg(p_hdl->p_eng, p_iq_cfg->cstp.cstp_ratio);
			ipe_eng_set_cst_prot_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_cst_prot_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}

	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_edge_dbg(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_EDGEDBG) {
		if (p_iq_cfg->edgedbg.enable) {
		ipe_eng_set_edge_dbg_sel(p_hdl->p_eng, p_iq_cfg->edgedbg.mode_sel);
		ipe_eng_set_edge_dbg_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_edge_dbg_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_cc(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg, KDRV_IPP_FRAME in_frm)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_CC || p_iq_cfg->update & KDRV_IPE_UPDATE_CCM) {
		IPE_CC_PARAM cc_para;
		/* TO DO d2d mode*/
		if (p_iq_cfg->cc.enable) {
			cc_para.cc2_sel = p_iq_cfg->cc.cc2_sel;
			cc_para.cc_gam_sel = p_iq_cfg->ccm.cc_gamma_sel;
			cc_para.cc_ofs_sel = kdrv_ipe_typecast_cc_ofssel(in_frm); // use input format with ycc bits
			cc_para.cc_range = p_iq_cfg->ccm.cc_range;
			cc_para.cc_stab_sel = p_iq_cfg->cc.cc_stab_sel;
			cc_para.p_cc_coeff = &p_iq_cfg->ccm.coef[0];
			cc_para.p_fdtab = &p_iq_cfg->cc.fdtab[0];
			cc_para.p_fstab = &p_iq_cfg->cc.fstab[0];
			ipe_eng_set_color_correct_buf_reg(p_hdl->p_eng, &cc_para);
			ipe_eng_set_fstab_buf_reg(p_hdl->p_eng, cc_para.p_fstab);
			ipe_eng_set_fdtab_buf_reg(p_hdl->p_eng, cc_para.p_fdtab);
			ipe_eng_set_color_correction_enable_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ipe_eng_set_color_correction_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	return E_OK;
}

#if 0
static INT32 kdrv_ipe_int_cfg_va(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg, IPE_ENG_DMA_OUT_INFO* p_single_out)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg = NULL;
	KDRV_IPE_IO_CFG* p_io_cfg = NULL;
	INT32 rt = E_OK;
	UINT32 i = 0;

	p_iq_cfg = p_cfg->p_iqcfg;
	p_io_cfg = p_cfg->p_iocfg;

	if (p_iq_cfg->va.enable) {

		if(p_cfg->p_iocfg->va_out_addr.pa != 0 && p_cfg->p_iocfg->va_out_addr.pa != (ULONG)-1){

			{
				UINT64 addr = p_cfg->p_iocfg->va_out_addr.pa;
				ipe_eng_set_dma_out_va_addr_buf_reg(p_hdl->p_eng,  (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
			}
		}
		else{
			DBG_ERR("va_out_addr:0x%lx invalid\n", p_cfg->p_iocfg->va_out_addr.pa);
			return E_PAR;
		}
		p_single_out->va_en = ENABLE;
	}

	if (p_iq_cfg->update & KDRV_IPE_UPDATE_VA) {
		{
			if (p_iq_cfg->va.enable) {
				IPE_VA_WIN_PARAM va_win = {0};
				IPE_OUTYCINFO va_out_info = {0};
				USIZE blk_size = {0};
				USIZE win_size = {0};
				//IPE_IMG_SIZE img_size = {0};

				blk_size.w = /*(p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width)*/p_cfg->p_iocfg->in_frm.size.w / p_iq_cfg->va.win_num.w;
				blk_size.h = p_cfg->p_iocfg->in_frm.size.h / p_iq_cfg->va.win_num.h;
				win_size.w = ALIGN_FLOOR_4(((p_iq_cfg->va_window.winsz_ratio.w * blk_size.w) / p_iq_cfg->va_window.ratio_base));
				if (win_size.w > KDRV_IPE_VA_INDEP_WIN_MAX) {
                    win_size.w = KDRV_IPE_VA_INDEP_WIN_MAX;
				}

				win_size.h = ALIGN_FLOOR_4(((p_iq_cfg->va_window.winsz_ratio.h * blk_size.h) / p_iq_cfg->va_window.ratio_base));
				if (win_size.h > KDRV_IPE_VA_INDEP_WIN_MAX) {
                    win_size.h = KDRV_IPE_VA_INDEP_WIN_MAX;
				}

				va_win.win_numx = p_iq_cfg->va.win_num.w;
				va_win.win_numy = p_iq_cfg->va.win_num.h;
				va_win.win_szx = win_size.w;
				va_win.win_szy = win_size.h;
				va_win.win_stx = (blk_size.w - win_size.w) >> 1;
				va_win.win_sty = (blk_size.h - win_size.h) >> 1;
				va_win.win_spx = (blk_size.w - win_size.w);
				va_win.win_spy = (blk_size.h - win_size.h);

				p_io_cfg->va_win_stx = va_win.win_stx;
				p_io_cfg->va_win_size_x = va_win.win_szx;
				p_io_cfg->va_win_skip_x = va_win.win_spx;

				if ((win_size.w > KDRV_IPE_VA_INDEP_WIN_MAX) || (win_size.h > KDRV_IPE_VA_INDEP_WIN_MAX) ||
					(va_win.win_spx >= 64) || (va_win.win_spy >= 64)) {
					//win_size.w = (win_size.w > KDRV_IPE_VA_INDEP_WIN_MAX) ? KDRV_IPE_VA_INDEP_WIN_MAX : win_size.w;
					//win_size.h = (win_size.h > KDRV_IPE_VA_INDEP_WIN_MAX) ? KDRV_IPE_VA_INDEP_WIN_MAX : win_size.h;
					va_win.win_spx = (va_win.win_spx >= 64) ? 63 : va_win.win_spx;
					va_win.win_spy = (va_win.win_spy >= 64) ? 63 : va_win.win_spy;
				}

				//img_size.h_size = p_cfg->p_iocfg->in_frm.size.w;
				//img_size.v_size = p_cfg->p_iocfg->in_frm.size.h;
				//rt= ipe_eng_check_va_win_info(p_hdl->p_eng, &va_win, img_size);
				if (rt != E_OK) {
					DBG_ERR("va limit fail\r\n");
					return rt;
				}

				p_cfg->p_iocfg->va_lofs = p_iq_cfg->va.win_num.w * 8 * (p_iq_cfg->va.va_out_grp1_2 + 1); // 538 added max: 0x40

				va_out_info.va_outsel = p_iq_cfg->va.va_out_grp1_2;
				va_out_info.lofs_va = p_cfg->p_iocfg->va_lofs; //p_iq_cfg->va.va_lofs;
				va_out_info.output_offset_va = p_io_cfg->va_out_addr_ofs; // 538 not used

				rt = ipe_eng_check_va_lofs(p_hdl->p_eng, &va_win, &va_out_info);
				if (rt != E_OK) {
					DBG_ERR("va chk lofs fail\r\n");
					return rt;
				}

				//ipe_eng_set_edge_extract_sel_buf_reg(p_hdl->p_eng, 0, KDRV_IPE_EEXT_G_CHANNEL); // test hard code
				ipe_eng_set_dma_out_va_offset_buf_reg(p_hdl->p_eng, p_cfg->p_iocfg->va_lofs,  p_io_cfg->va_out_addr_ofs); //p_iq_cfg->va.va_lofs
				ipe_eng_set_va_win_info_buf_reg(p_hdl->p_eng, &va_win);

				ipe_eng_set_va_out_sel_buf_reg(p_hdl->p_eng, p_iq_cfg->va.va_out_grp1_2);

				ipe_eng_set_vacc_enable_buf_reg(p_hdl->p_eng, ENABLE);
			} else {
				ipe_eng_set_vacc_enable_buf_reg(p_hdl->p_eng, DISABLE);
			}
		}

		{
			if (p_iq_cfg->va.indep_va_enable) {
				IPE_INDEP_VA_PARAM indep_va_win_info;
				BOOL va_indep_en[KDRV_IPE_IQ_VA_INDEP_NUM];

				for (i = 0; i < KDRV_IPE_IQ_VA_INDEP_NUM; i++) {
					va_indep_en[i] = p_iq_cfg->va.indep_win[i].enable;
					//indep_va_win_info.linemax_g1_en = p_iq_cfg->va.indep_win[i].linemax_g1;  //530 removed
					//indep_va_win_info.linemax_g2_en = p_iq_cfg->va.indep_win[i].linemax_g2;  //530 removed

					indep_va_win_info.win_stx = ALIGN_FLOOR_4(((p_iq_cfg->va_window.indep_roi_ratio[i].x * /*(p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width)*/p_cfg->p_iocfg->in_frm.size.w) / p_iq_cfg->va_window.ratio_base));
					indep_va_win_info.win_sty = ALIGN_FLOOR_4(((p_iq_cfg->va_window.indep_roi_ratio[i].y * p_cfg->p_iocfg->in_frm.size.h) / p_iq_cfg->va_window.ratio_base));
					indep_va_win_info.win_szx = ALIGN_FLOOR_4(((p_iq_cfg->va_window.indep_roi_ratio[i].w * /*(p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width)*/p_cfg->p_iocfg->in_frm.size.w) / p_iq_cfg->va_window.ratio_base));
					indep_va_win_info.win_szy = ALIGN_FLOOR_4(((p_iq_cfg->va_window.indep_roi_ratio[i].h * p_cfg->p_iocfg->in_frm.size.h) / p_iq_cfg->va_window.ratio_base));


					if (indep_va_win_info.win_szx > KDRV_IPE_VA_INDEP_WIN_MAX) {
						indep_va_win_info.win_stx = indep_va_win_info.win_stx +  ALIGN_FLOOR_4((indep_va_win_info.win_szx - KDRV_IPE_VA_INDEP_WIN_MAX) / 2);
						indep_va_win_info.win_szx = KDRV_IPE_VA_INDEP_WIN_MAX;
					}
					if (indep_va_win_info.win_szy > KDRV_IPE_VA_INDEP_WIN_MAX) {
						indep_va_win_info.win_sty = indep_va_win_info.win_sty +  ALIGN_FLOOR_4((indep_va_win_info.win_szy - KDRV_IPE_VA_INDEP_WIN_MAX) / 2);
						indep_va_win_info.win_szy = KDRV_IPE_VA_INDEP_WIN_MAX;
					}
					ipe_eng_set_va_indep_win_buf_reg(p_hdl->p_eng, &indep_va_win_info, i);
				}
				ipe_eng_set_vacc_win_enable_buf_reg(p_hdl->p_eng, va_indep_en[0], va_indep_en[1], va_indep_en[2], va_indep_en[3], va_indep_en[4]);
			}else
				ipe_eng_set_vacc_win_enable_buf_reg(p_hdl->p_eng, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE);

			if (p_iq_cfg->va.enable || p_iq_cfg->va.indep_va_enable) {

					IPE_VA_FLTR_GROUP_PARAM va_fltr_g1;
					IPE_VA_FLTR_GROUP_PARAM va_fltr_g2;
					IPE_VA_COMMON_PARAM va_common_param;

					va_fltr_g1.cnt_en = p_iq_cfg->va.group_1.count_enable;
					//va_fltr_g1.linemax_en = p_iq_cfg->va.group_1.linemax_mode; //530 removed
					va_fltr_g1.filt_h.fltr_size = p_iq_cfg->va.group_1.h_filt.filter_size;
					va_fltr_g1.filt_h.filt_symm = p_iq_cfg->va.group_1.h_filt.symmetry;
					va_fltr_g1.filt_h.tap_a = p_iq_cfg->va.group_1.h_filt.tap_a;
					va_fltr_g1.filt_h.tap_b = p_iq_cfg->va.group_1.h_filt.tap_b;
					va_fltr_g1.filt_h.tap_c = p_iq_cfg->va.group_1.h_filt.tap_c;
					va_fltr_g1.filt_h.tap_d = p_iq_cfg->va.group_1.h_filt.tap_d;
					va_fltr_g1.filt_h.div = p_iq_cfg->va.group_1.h_filt.div;
					va_fltr_g1.filt_h.th_low = p_iq_cfg->va.group_1.h_filt.th_l;
					va_fltr_g1.filt_h.th_high = p_iq_cfg->va.group_1.h_filt.th_u;
					va_fltr_g1.filt_v.fltr_size = p_iq_cfg->va.group_1.v_filt.filter_size;
					va_fltr_g1.filt_v.filt_symm = p_iq_cfg->va.group_1.v_filt.symmetry;
					va_fltr_g1.filt_v.tap_a =p_iq_cfg->va.group_1.v_filt.tap_a;
					va_fltr_g1.filt_v.tap_b = p_iq_cfg->va.group_1.v_filt.tap_b;
					va_fltr_g1.filt_v.tap_c = p_iq_cfg->va.group_1.v_filt.tap_c;
					va_fltr_g1.filt_v.tap_d = p_iq_cfg->va.group_1.v_filt.tap_d;
					va_fltr_g1.filt_v.div = p_iq_cfg->va.group_1.v_filt.div;
					va_fltr_g1.filt_v.th_low = p_iq_cfg->va.group_1.v_filt.th_l;
					va_fltr_g1.filt_v.th_high = p_iq_cfg->va.group_1.v_filt.th_u;

					va_fltr_g2.cnt_en = p_iq_cfg->va.group_2.count_enable;
					//va_fltr_g2.linemax_en = p_iq_cfg->va.group_2.linemax_mode; //530 removed
					va_fltr_g2.filt_h.fltr_size = p_iq_cfg->va.group_2.h_filt.filter_size;
					va_fltr_g2.filt_h.filt_symm = p_iq_cfg->va.group_2.h_filt.symmetry;
					va_fltr_g2.filt_h.tap_a = p_iq_cfg->va.group_2.h_filt.tap_a;
					va_fltr_g2.filt_h.tap_b = p_iq_cfg->va.group_2.h_filt.tap_b;
					va_fltr_g2.filt_h.tap_c = p_iq_cfg->va.group_2.h_filt.tap_c;
					va_fltr_g2.filt_h.tap_d = p_iq_cfg->va.group_2.h_filt.tap_d;
					va_fltr_g2.filt_h.div = p_iq_cfg->va.group_2.h_filt.div;
					va_fltr_g2.filt_h.th_low = p_iq_cfg->va.group_2.h_filt.th_l;
					va_fltr_g2.filt_h.th_high = p_iq_cfg->va.group_2.h_filt.th_u;
					va_fltr_g2.filt_v.fltr_size = p_iq_cfg->va.group_2.v_filt.filter_size;
					va_fltr_g2.filt_v.filt_symm = p_iq_cfg->va.group_2.v_filt.symmetry;
					va_fltr_g2.filt_v.tap_a =p_iq_cfg->va.group_2.v_filt.tap_a;
					va_fltr_g2.filt_v.tap_b = p_iq_cfg->va.group_2.v_filt.tap_b;
					va_fltr_g2.filt_v.tap_c = p_iq_cfg->va.group_2.v_filt.tap_c;
					va_fltr_g2.filt_v.tap_d = p_iq_cfg->va.group_2.v_filt.tap_d;
					va_fltr_g2.filt_v.div = p_iq_cfg->va.group_2.v_filt.div;
					va_fltr_g2.filt_v.th_low = p_iq_cfg->va.group_2.v_filt.th_l;
					va_fltr_g2.filt_v.th_high = p_iq_cfg->va.group_2.v_filt.th_u;

					ipe_eng_set_va_filter_g1_buf_reg(p_hdl->p_eng, &va_fltr_g1);
					ipe_eng_set_va_filter_g2_buf_reg(p_hdl->p_eng, &va_fltr_g2);
					ipe_eng_set_va_mode_enable_buf_reg(p_hdl->p_eng, &va_fltr_g1, &va_fltr_g2);

					va_common_param.pre_filter_mode = p_iq_cfg->va.pre_filter_mode;

					va_common_param.blending_w = p_iq_cfg->va.energy_w;
					va_common_param.high_luma_th = p_iq_cfg->va.high_luma_th;
					va_common_param.win_cnt_out_sel = p_iq_cfg->va.win_cnt_out_sel;
					ipe_eng_set_va_common_info_buf_reg(p_hdl->p_eng, &va_common_param);

					if(p_iq_cfg->va.ldg_enable){

						IPE_VA_LDG_PARAM ldg_param = {0};

						ldg_param.ldg_high_gain = p_iq_cfg->va.ldg_para.ldg_high_gain;
						ldg_param.ldg_low_gain = p_iq_cfg->va.ldg_para.ldg_low_gain;
						ldg_param.ldg_high_slope = p_iq_cfg->va.ldg_para.ldg_high_slope;
						ldg_param.ldg_low_slope = p_iq_cfg->va.ldg_para.ldg_low_slope;
						ldg_param.ldg_high_th = p_iq_cfg->va.ldg_para.ldg_high_th;
						ldg_param.ldg_low_th = p_iq_cfg->va.ldg_para.ldg_low_th;

						ipe_eng_set_va_ldg_buf_reg(p_hdl->p_eng, &ldg_param);
						ipe_eng_set_va_ldg_enable_buf_reg(p_hdl->p_eng, ENABLE);
					}else{
						ipe_eng_set_va_ldg_enable_buf_reg(p_hdl->p_eng, DISABLE);
					}
			}
		}
	}
	return E_OK;
}
#endif

static INT32 kdrv_ipe_int_cfg_edge_region_str(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	IPE_REGION_STRENGTH_PARAM region_str;
	KDRV_IPE_IQ_CFG* p_iq_cfg;

	p_iq_cfg = p_cfg->p_iqcfg;
	if (p_iq_cfg->update & KDRV_IPE_UPDATE_EDGE_REGION_STR) {
		region_str.region_str_en = p_iq_cfg->edge_region_str.enable;
		region_str.enh_thin = p_iq_cfg->edge_region_str.enh_thin;
		region_str.enh_robust = p_iq_cfg->edge_region_str.enh_robust;
		region_str.slope_flat = p_iq_cfg->edge_region_str.slope_flat;
		region_str.slope_edge = p_iq_cfg->edge_region_str.slope_edge;
		region_str.str_flat = p_iq_cfg->edge_region_str.str_flat;
		region_str.str_edge = p_iq_cfg->edge_region_str.str_edge;
		ipe_eng_set_region_strength_ctrl_buf_reg(p_hdl->p_eng, &region_str);
	}
	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_pfr(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg;
	UINT32 i=0;

	p_iq_cfg = p_cfg->p_iqcfg;
	if(p_iq_cfg->update & KDRV_IPE_UPDATE_PFR){

		if(p_iq_cfg->pfr.enable){

			IPE_PFR_PARAM pfr_param;


			pfr_param.uv_filt_en = p_iq_cfg->pfr.uv_filt_en;
			pfr_param.luma_level_en = p_iq_cfg->pfr.luma_level_en;
			pfr_param.wet_out = p_iq_cfg->pfr.out_wet;
			pfr_param.edge_strength = p_iq_cfg->pfr.edge_str;
			pfr_param.edge_th = p_iq_cfg->pfr.edge_th;
			pfr_param.color_wet_g = p_iq_cfg->pfr.color_wet_g;

			pfr_param.luma_th = p_iq_cfg->pfr.luma_th;

			ipe_eng_set_pfr_general_buf_reg(p_hdl->p_eng, &pfr_param);

			for(i=0; i<KDRV_IPE_IQ_PFR_SET_NUM; i++){
				ipe_eng_set_pfr_color_buf_reg(p_hdl->p_eng, i, (IPE_PFR_COLOR_WET *)&p_iq_cfg->pfr.color_wet_set[i]); //tmp for early porting watch out bit
			}

			ipe_eng_set_pfr_luma_buf_reg(p_hdl->p_eng, pfr_param.luma_th, &p_iq_cfg->pfr.luma_lut[0]);

			ipe_eng_set_pfr_enable_buf_reg(p_hdl->p_eng, ENABLE);
		}else{
			ipe_eng_set_pfr_enable_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	return E_OK;
}

static INT32 kdrv_ipe_int_cfg_cfa(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg, IPE_ENG_DMA_OUT_INFO* p_single_out)
{
	KDRV_IPE_IQ_CFG* p_iq_cfg = NULL;
	KDRV_IPE_IO_CFG* p_io_cfg = NULL;
	UINT32 i=0;
	IPE_ENG_CFA_PARAM cfa_param = {0};

	p_iq_cfg = p_cfg->p_iqcfg;
	p_io_cfg = p_cfg->p_iocfg;

	if(p_iq_cfg->update2 & KDRV_IPE_UPDATE2_CFA){

		if(p_iq_cfg->cfa.cfa_enable){
			ipe_eng_set_cfa_enable_buf_reg(p_hdl->p_eng, TRUE);

			//539A add
			ipe_eng_set_sram_sd_cfa_buf_reg(p_hdl->p_eng, DISABLE);

			cfa_param.raw_fmt = kdrv_ipe_typecast_bayer(p_io_cfg->in_frm.fmt);
			cfa_param.cfa_pat = kdrv_ipe_typecast_cfa(p_io_cfg->in_frm.fmt);
            ipe_eng_set_cfa_buf_reg(p_hdl->p_eng, &cfa_param);
            {
                IPE_ENG_CFA_INTERP color_interp;
                UINT8 freq_lut[KDRV_CFA_FREQ_NUM];
                UINT8 luma_wt[KDRV_IPE_IQ_CFA_LUMA_NUM];

                color_interp.edge_dth = p_iq_cfg->cfa.cfa_interp.edge_dth;
                color_interp.edge_dth2 = p_iq_cfg->cfa.cfa_interp.edge_dth2;
                color_interp.freq_th = p_iq_cfg->cfa.cfa_interp.freq_th;
                for (i = 0; i< KDRV_IPE_IQ_CFA_FREQ_NUM; i++)
                    freq_lut[i] = p_iq_cfg->cfa.cfa_interp.freq_lut[i];
                color_interp.p_freq_blend_lut = freq_lut;
                for (i = 0; i< KDRV_IPE_IQ_CFA_LUMA_NUM; i++)
                    luma_wt[i] = p_iq_cfg->cfa.cfa_interp.luma_wt[i];
                color_interp.p_luma_wt = luma_wt;
                ipe_eng_set_cfa_interp_buf_reg(p_hdl->p_eng, &color_interp);
            }
            {
                IPE_ENG_CFA_RB_CORR rb_correct;
                rb_correct.rb_corr_en = p_iq_cfg->cfa.cfa_correction.rb_corr_enable;
                rb_correct.rb_corr_th1 = p_iq_cfg->cfa.cfa_correction.rb_corr_th1;
                rb_correct.rb_corr_th2 = p_iq_cfg->cfa.cfa_correction.rb_corr_th2;
                ipe_eng_set_cfa_rb_correct_buf_reg(p_hdl->p_eng, &rb_correct);
            }
            {
                IPE_ENG_CFA_FCS false_color_supp;
                UINT8 fcs_str[KDRV_IPE_IQ_CFA_FCS_NUM];

                false_color_supp.fcs_dirsel = p_iq_cfg->cfa.cfa_fcs.fcs_dirsel;
                false_color_supp.fcs_coring = p_iq_cfg->cfa.cfa_fcs.fcs_coring;
                false_color_supp.fcs_weight = p_iq_cfg->cfa.cfa_fcs.fcs_weight;
                for (i = 0; i< KDRV_IPE_IQ_CFA_FCS_NUM; i++)
                    fcs_str[i] = p_iq_cfg->cfa.cfa_fcs.fcs_strength[i];
                false_color_supp.p_fcs_strength = fcs_str;
                ipe_eng_set_cfa_false_color_supp_buf_reg(p_hdl->p_eng, &false_color_supp);
            }

		}else{
			ipe_eng_set_cfa_enable_buf_reg(p_hdl->p_eng, FALSE);
		}

		//539A add
		ipe_eng_set_cfa_fcs_enable_buf_reg(p_hdl->p_eng, p_iq_cfg->cfa.cfa_fcs.fcs_enable);
		if(p_iq_cfg->cfa.cfa_enable == FALSE && p_io_cfg->cfa_sub.cfa_subout_enable == FALSE)
			if (kdrv_ipe_typecast_opmode(p_cfg->p_iocfg->mode) != IPE_ENG_OPMODE_ALL_DIRECT ||(kdrv_ipe_typecast_opmode(p_cfg->p_iocfg->mode) == IPE_ENG_OPMODE_ALL_DIRECT && p_cfg->p_iocfg->in_job_num == 1)) { //direct only start job can shutdown
				ipe_eng_set_sram_sd_cfa_buf_reg(p_hdl->p_eng, ENABLE);
		}
	}

    if (p_io_cfg->cfa_sub.cfa_subout_enable == TRUE) {
        IPE_ENG_CFA_SUBOUT cfa_sub_out;
        if (p_io_cfg->cfa_sub.cfa_addr.pa == 0 || p_io_cfg->cfa_sub.cfa_addr.pa == (ULONG)-1) {
            DBG_ERR("cfa sub addr:0x%lx invalid, force to disable!\r\n", p_io_cfg->cfa_sub.cfa_addr.pa);
            p_io_cfg->cfa_sub.cfa_subout_enable = DISABLE;
			return E_PAR;
        } else if (p_io_cfg->cfa_sub.cfa_lofs == 0) {
            DBG_WRN("cfa sub offset=0!\r\n");
            p_io_cfg->cfa_sub.cfa_subout_enable = DISABLE;
        }
        //cfa_sub_out.cfa_subout_addr = p_io_cfg->cfa_sub.cfa_addr.pa;
		//cfa_sub_out.cfa_subout_ofst = p_io_cfg->cfa_sub.cfa_lofs;
        cfa_sub_out.flip = p_io_cfg->cfa_sub.cfa_subout_flip_enable;
        cfa_sub_out.r_shift_bit = p_io_cfg->cfa_sub.subout_shiftbit;
        cfa_sub_out.ch_sel = kdrv_ipe_typecast_cfa_sub_ch(p_io_cfg->cfa_sub.subout_ch_sel);
        cfa_sub_out.byte = kdrv_ipe_typecast_cfa_sub_byte(p_io_cfg->cfa_sub.subout_byte);
        ipe_eng_set_cfa_subout_buf_reg(p_hdl->p_eng, &cfa_sub_out);

        if(p_io_cfg->cfa_sub.cfa_addr.pa != 0 && p_io_cfg->cfa_sub.cfa_addr.pa != (ULONG)-1){

			{
				UINT64 addr = p_io_cfg->cfa_sub.cfa_addr.pa;
				ipe_eng_set_dma_out_ir_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
			}

			ipe_eng_set_dma_out_ir_lineoffset_buf_reg(p_hdl->p_eng, p_io_cfg->cfa_sub.cfa_lofs, p_io_cfg->cfa_sub.cfa_addr_ofs);

			p_single_out->ir_en = ENABLE;
        }
    }

	ipe_eng_set_cfa_subout_enable_buf_reg(p_hdl->p_eng, p_io_cfg->cfa_sub.cfa_subout_enable);



	if (p_iq_cfg->update2 & KDRV_IPE_UPDATE2_IR_PARAM) {

        IPE_ENG_CFA_RGBIR rgbir = {0};

        rgbir.high_freq.ir_cl_check_en = p_iq_cfg->cfa.cfa_ir_hfc.cl_check_enable;
        rgbir.high_freq.ir_hf_check_en = p_iq_cfg->cfa.cfa_ir_hfc.hf_check_enable;
        rgbir.high_freq.ir_average_mode = p_iq_cfg->cfa.cfa_ir_hfc.average_mode;
        rgbir.high_freq.ir_cl_sel = p_iq_cfg->cfa.cfa_ir_hfc.cl_sel;
        rgbir.high_freq.ir_cl_th = p_iq_cfg->cfa.cfa_ir_hfc.cl_th;
        rgbir.high_freq.ir_hf_gth = p_iq_cfg->cfa.cfa_ir_hfc.hf_gth;
        rgbir.high_freq.ir_hf_diff = p_iq_cfg->cfa.cfa_ir_hfc.hf_diff;
        rgbir.high_freq.ir_hf_eth = p_iq_cfg->cfa.cfa_ir_hfc.hf_eth;
        rgbir.high_freq.ir_g_edge_th = p_iq_cfg->cfa.cfa_ir_hfc.ir_g_edge_th;
        rgbir.high_freq.ir_rb_cstrength = p_iq_cfg->cfa.cfa_ir_hfc.ir_rb_cstrength;

        rgbir.ir_sub.ir_sub_r = p_iq_cfg->cfa.cfa_ir_sub.ir_sub_r;
        rgbir.ir_sub.ir_sub_g = p_iq_cfg->cfa.cfa_ir_sub.ir_sub_g;
        rgbir.ir_sub.ir_sub_b = p_iq_cfg->cfa.cfa_ir_sub.ir_sub_b;
        rgbir.ir_sub.ir_sub_wt_lb = p_iq_cfg->cfa.cfa_ir_sub.ir_sub_wt_lb;
        rgbir.ir_sub.ir_sub_th = p_iq_cfg->cfa.cfa_ir_sub.ir_sub_th;
        rgbir.ir_sub.ir_sub_range = p_iq_cfg->cfa.cfa_ir_sub.ir_sub_range;
        rgbir.ir_sub.ir_sat_gain = p_iq_cfg->cfa.cfa_ir_sub.ir_sat_gain;

        ipe_eng_set_cfa_rgbir_buf_reg(p_hdl->p_eng, &rgbir);

	}

    if (p_iq_cfg->update2 & KDRV_IPE_UPDATE2_COLOR_GAIN) {
        IPE_ENG_CFA_CGAIN color_gain = {0};

        color_gain.gain_range = p_iq_cfg->cfa.cfa_cgain.gain_range;
        color_gain.gain_r = p_iq_cfg->cfa.cfa_cgain.r_gain;
        color_gain.gain_g = p_iq_cfg->cfa.cfa_cgain.g_gain;
        color_gain.gain_b = p_iq_cfg->cfa.cfa_cgain.b_gain;
        ipe_eng_set_cfa_color_gain_buf_reg(p_hdl->p_eng, &color_gain);
    }

    if (p_iq_cfg->update2  & KDRV_IPE_UPDATE2_PINK_REDUC) {
        IPE_ENG_CFA_PINKR pink_reduc = {0};
        pink_reduc.pink_reduc_enable = p_iq_cfg->cfa.cfa_pink_reduc.pink_rd_en;
        pink_reduc.pink_rd_mode = p_iq_cfg->cfa.cfa_pink_reduc.pink_rd_mode;
        pink_reduc.pink_rd_th1 = p_iq_cfg->cfa.cfa_pink_reduc.pink_rd_th1;
        pink_reduc.pink_rd_th2 = p_iq_cfg->cfa.cfa_pink_reduc.pink_rd_th2;
        pink_reduc.pink_rd_th3 = p_iq_cfg->cfa.cfa_pink_reduc.pink_rd_th3;
        pink_reduc.pink_rd_th4 = p_iq_cfg->cfa.cfa_pink_reduc.pink_rd_th4;
        ipe_eng_set_cfa_pink_reduc_buf_reg(p_hdl->p_eng, &pink_reduc);
    }

	return E_OK;
}

#endif

#if IPE_SSDRV_SUPPORT
static INT32 kdrv_ipe_int_cfg_dual_pipe(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_info) //tmp remove for 538
{
	KDRV_IPE_IO_CFG* p_io_cfg = p_info->p_iocfg;
	KDRV_IPE_IQ_CFG* p_iq_cfg = p_info->p_iqcfg;

	if(p_io_cfg->dual_en == TRUE){

		if(p_io_cfg->dual_idx == KDRV_IPE_LEFT){
			//ipe_eng_set_dual_ipp_info_buf_reg(p_hdl->p_eng, (p_io_cfg->dual_left_width + p_io_cfg->dual_right_width), p_io_cfg->dual_left_ovlp); // left & right ovlp is same value
		}else if(p_io_cfg->dual_idx == KDRV_IPE_RIGHT){
			//ipe_eng_set_dual_ipp_info_buf_reg(p_hdl->p_eng, (p_io_cfg->dual_left_width + p_io_cfg->dual_right_width), p_io_cfg->dual_right_ovlp); // left & right ovlp is same value
		}else{
			DBG_ERR("unknown dual idx:%d\n", p_io_cfg->dual_idx);
			return E_PAR;
		}

		//ipe_eng_set_dual_ipp_idx_buf_reg(p_hdl->p_eng, p_io_cfg->dual_idx);
		//ipe_eng_set_dual_ipp_enable_buf_reg(p_hdl->p_eng, ENABLE);


		if(p_iq_cfg->va.enable){

			if(p_io_cfg->dual_idx == KDRV_IPE_LEFT){

				p_io_cfg->va_out_addr_ofs = 0;

			}else if(p_io_cfg->dual_idx == KDRV_IPE_RIGHT){

				p_io_cfg->va_out_addr_ofs = 0;
				p_io_cfg->va_out_addr.pa += KDRV_IPE_VA_BUF_SIZE;
				p_io_cfg->va_out_addr.va += KDRV_IPE_VA_BUF_SIZE;
			}
		}
#if (IPE_538_KDRV_FEATURE == 0)
		if(p_io_cfg->defog_sub_out_en){

			if(p_io_cfg->dual_idx == KDRV_IPE_LEFT){

				p_iq_cfg->defog_subimg_param.subimg_lofs_out = ALIGN_CEIL_4(p_iq_cfg->defog_subimg_param.h_size << 2) << 1;
				p_iq_cfg->defog_subimg_param.subimg_lofs_in =  ALIGN_CEIL_4(p_iq_cfg->defog_subimg_param.h_size << 2);
				p_io_cfg->defog_subin_addr_ofs = 0;
				p_io_cfg->defog_subout_addr_ofs = 0;

			}else if(p_io_cfg->dual_idx == KDRV_IPE_RIGHT){

				p_iq_cfg->defog_subimg_param.subimg_lofs_out = ALIGN_CEIL_4(p_iq_cfg->defog_subimg_param.h_size << 2) << 1;
				p_iq_cfg->defog_subimg_param.subimg_lofs_in = ALIGN_CEIL_4(p_iq_cfg->defog_subimg_param.h_size << 2);
				p_io_cfg->defog_subin_addr_ofs = 0;
				p_io_cfg->defog_subout_addr_ofs = p_iq_cfg->defog_subimg_param.h_size << 2;
			}
		}

		//DBG_DUMP("-------defog width[%d]:%d  in_lofs:%d, out_lofs:%d addr_ofs:%d en:%d----------\n",p_io_cfg->dual_idx, p_iq_cfg->defog_subimg_param.h_size, p_iq_cfg->defog_subimg_param.subimg_lofs_in, p_iq_cfg->defog_subimg_param.subimg_lofs_out, p_io_cfg->defog_subout_addr_ofs, p_iq_cfg->defog_param.enable);

		if(p_io_cfg->lce_sub_out_en){

			if(p_io_cfg->dual_idx == KDRV_IPE_LEFT){

				p_iq_cfg->lce_subimg_param.subimg_lofs_out = ALIGN_CEIL_4(p_iq_cfg->lce_subimg_param.h_size << 1) << 1;
				p_iq_cfg->lce_subimg_param.subimg_lofs_in = ALIGN_CEIL_4(p_iq_cfg->lce_subimg_param.h_size << 1);
				p_io_cfg->lce_subin_addr_ofs = 0;
				p_io_cfg->lce_subout_addr_ofs = 0;

			}else if(p_io_cfg->dual_idx == KDRV_IPE_RIGHT){

				p_iq_cfg->lce_subimg_param.subimg_lofs_out = ALIGN_CEIL_4(p_iq_cfg->lce_subimg_param.h_size << 1) << 1;
				p_iq_cfg->lce_subimg_param.subimg_lofs_in = ALIGN_CEIL_4(p_iq_cfg->lce_subimg_param.h_size << 1);
				p_io_cfg->lce_subin_addr_ofs = 0;
				p_io_cfg->lce_subout_addr_ofs = p_iq_cfg->lce_subimg_param.h_size << 1;

			}

		}

		//DBG_DUMP("-------lce width[%d]:%d  in_lofs:%d, out_lofs:%d addr_ofs:%d en:%d----------\n",p_io_cfg->dual_idx, p_iq_cfg->lce_subimg_param.h_size, p_iq_cfg->lce_subimg_param.subimg_lofs_in, p_iq_cfg->lce_subimg_param.subimg_lofs_out, p_io_cfg->lce_subout_addr_ofs, p_iq_cfg->lce_param.enable);
#endif

	}else{
		p_io_cfg->subimg_lofs_in = p_iq_cfg->subimg.subimg_size.h_size << 2;
		p_io_cfg->subimg_lofs_out = p_iq_cfg->subimg.subimg_size.h_size << 2;

		if(p_io_cfg->sub_out_en){ //defog
			p_io_cfg->subimg_lofs_out = ALIGN_CEIL_4(p_io_cfg->subimg_lofs_out);
			p_io_cfg->subimg_lofs_in  = ALIGN_CEIL_4(p_io_cfg->subimg_lofs_in);
		}

		if(p_io_cfg->sub_out_en){ //lce
			p_io_cfg->subimg_lofs_out = ALIGN_CEIL_4(p_io_cfg->subimg_lofs_out);
			p_io_cfg->subimg_lofs_in = ALIGN_CEIL_4(p_io_cfg->subimg_lofs_in);
		}

		//ipe_eng_set_dual_ipp_enable_buf_reg(p_hdl->p_eng, DISABLE);
	}
	return E_OK;
}
#endif

static INT32 kdrv_ipe_int_cfg_all(KDRV_IPE_HANDLE *p_hdl, KDRV_IPE_JOB_CFG *p_cfg)
{

#if (IPE_SSDRV_SUPPORT== 0)
	INT32 rt = E_OK;
	KDRV_IPP_FMT fmt_with_ycc;

	fmt_with_ycc = p_cfg->p_iocfg->in_frm.fmt;
	if (kdrv_ipe_int_check_limit(p_cfg, fmt_with_ycc) != E_OK) {
		return E_PAR;
	}

#else
	KDRV_IPE_IO_CFG* p_io_cfg = NULL;

	INT32 rt = E_OK;
	KDRV_IPE_IO_INFO io_base_info = {0};
	IPE_ENG_DMA_OUT_INFO single_out_info = {0};
	IPE_ENG_STRIPE_CAL_INFO stripe_calinfo = {0};
	IPE_ENG_STRIPE_HV_INFO stripe_info = {0};
	IPE_ENG_OPMODE eng_mode;
	UINT32 lofs_y, lofs_c;
	UINT32 addr_ofs_y = 0;
	UINT32 addr_ofs_c = 0;
	if (kdrv_ipe_int_check_limit(p_cfg, p_cfg->p_iocfg->in_frm.pxl_fmt) != E_OK) {
		return E_PAR;
	}

	p_io_cfg = p_cfg->p_iocfg;

	eng_mode = kdrv_ipe_typecast_opmode(p_io_cfg->mode);
	if ((INT32)eng_mode == -1) {
		return E_PAR;
	}

	ipe_eng_set_op_mode_buf_reg(p_hdl->p_eng, eng_mode);

	kdrv_ipe_parsing_io_info(p_io_cfg, &io_base_info);
	ipe_eng_set_output_switch_buf_reg(p_hdl->p_eng, io_base_info.to_ime, io_base_info.to_dma);
	ipe_eng_set_dma_out_sel_buf_reg(p_hdl->p_eng, io_base_info.out_sel);
	ipe_eng_set_in_format_buf_reg(p_hdl->p_eng, io_base_info.in_fmt);
	ipe_eng_set_out_format_buf_reg(p_hdl->p_eng, io_base_info.out_fmt, IPE_ENG_DROP_RIGHT);
	ipe_eng_set_hovlp_opt_buf_reg(p_hdl->p_eng, p_io_cfg->mst_h_ovlp);

	/* YCC-lite*/
	ipe_eng_set_ycc_lite_enable_buf_reg(p_hdl->p_eng, p_io_cfg->ycc_en, p_io_cfg->ycc_shift_mode_en);


	/* dual pipe config*/
	rt |= kdrv_ipe_int_cfg_dual_pipe(p_hdl,p_cfg);

	/* stipe information */
	stripe_calinfo.dram_o_en = p_io_cfg->dma_out_en;
	stripe_calinfo.dram_o_sel = io_base_info.out_sel;
	stripe_calinfo.height = p_io_cfg->in_frm.size.h;
	stripe_calinfo.width = p_io_cfg->in_frm.size.w;

	ipe_eng_cal_hv_stripe(&stripe_calinfo, eng_mode, &stripe_info);
	ipe_eng_set_horizontal_stripe_buf_reg(p_hdl->p_eng, p_io_cfg->ipe_strp_num, p_io_cfg->ipe_strp[0],
																				p_io_cfg->ipe_strp[1],
																				p_io_cfg->ipe_strp[2],
																				p_io_cfg->ipe_strp[3],
																				p_io_cfg->ipe_strp[4],
																				p_io_cfg->ipe_strp[5],
																				p_io_cfg->ipe_strp[6],
																				p_io_cfg->ipe_strp[7]);

	ipe_eng_set_vertical_stripe_buf_reg(p_hdl->p_eng, stripe_info.v_l);

	ipe_eng_set_ime_overlap_buf_reg(p_hdl->p_eng, (p_io_cfg->ipe_ovlp >> 3));

	if (p_io_cfg->mode == KDRV_IPP_OPMODE_D2D) {
		/* dma in */

		{
			UINT64 addr_y = p_io_cfg->in_frm.phyaddr[0];
			UINT64 addr_c = p_io_cfg->in_frm.phyaddr[1];

			ipe_eng_set_dma_in_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr_y & 0xFFFFFFFF), (UINT32)((addr_y >> 32) & 0xFFFFFFFF),
															(UINT32)(addr_c & 0xFFFFFFFF), (UINT32)((addr_c >> 32) & 0xFFFFFFFF));
		}

		ipe_eng_set_dma_in_offset_buf_reg(p_hdl->p_eng, p_io_cfg->in_frm.lofs[0], p_io_cfg->in_frm.lofs[1]);
		ipe_eng_set_dma_in_rand_buf_reg(p_hdl->p_eng, DISABLE, DISABLE);
	}

	if (p_io_cfg->mode == KDRV_IPP_OPMODE_D2D || p_io_cfg->mode == KDRV_IPP_OPMODE_IFE2IPE || p_io_cfg->mode == KDRV_IPP_OPMODE_PRE2IPE) {
		if (p_io_cfg->dma_out_en) {
			/* dma out */
			single_out_info.y_en= ENABLE;
			single_out_info.c_en= ENABLE;

			{
				UINT64 addr = p_io_cfg->out_frm.phyaddr[0];

				ipe_eng_set_dma_out_addr_y_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));

				addr = p_io_cfg->out_frm.phyaddr[1];

				ipe_eng_set_dma_out_addr_c_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
			}

			lofs_y = p_io_cfg->in_frm.size.w;
			lofs_c = p_io_cfg->in_frm.size.w;

#if(IPE_538_KDRV_FEATURE == 0)
			addr_ofs_y = p_io_cfg->out_frm.addr_ofs[0];
			addr_ofs_c = p_io_cfg->out_frm.addr_ofs[1];
#endif

			ipe_eng_set_dma_out_offset_y_buf_reg(p_hdl->p_eng, lofs_y, addr_ofs_y);
			ipe_eng_set_dma_out_offset_c_buf_reg(p_hdl->p_eng, lofs_c, addr_ofs_c);
		}
	}

	/* enable interrupt*/
	ipe_eng_set_interrupt_enable_buf_reg(p_hdl->p_eng, p_io_cfg->inte_en);

	/* IQ */
	rt |= kdrv_ipe_int_cfg_rgblpf(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_lce(p_hdl, p_cfg, &single_out_info);
	rt |= kdrv_ipe_int_cfg_defog(p_hdl, p_cfg, &single_out_info);
	rt |= kdrv_ipe_int_cfg_eext(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_overshoot(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_eproc(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_cctrl(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_cadj(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_gamyrand(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_gamma(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_ycurve(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_edge_dbg(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_cst(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_cc(p_hdl, p_cfg, p_cfg->p_iocfg->in_frm);
	//rt |= kdrv_ipe_int_cfg_va(p_hdl, p_cfg, &single_out_info); //539a remove
	rt |= kdrv_ipe_int_cfg_edge_region_str(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_pfr(p_hdl, p_cfg);
	rt |= kdrv_ipe_int_cfg_cfa(p_hdl, p_cfg, &single_out_info);
	rt |= kdrv_ipe_int_cfg_3dcc(p_hdl, p_cfg);	//538 add

	single_out_info.enable = TRUE;
	ipe_eng_set_dram_out_mode_buf_reg(p_hdl->p_eng, single_out_info.enable);
	ipe_eng_set_dram_single_channel_enable_buf_reg(p_hdl->p_eng, single_out_info.y_en,
			single_out_info.c_en, single_out_info.va_en, single_out_info.defog_en,
			0, single_out_info.ir_en);  // removed lce_single_out_en

	/* Load Gamma */
	{
		KDRV_IPE_GAMMA_LOAD_INFO load_info = {0};
		BOOL b_gamma_load = FALSE, b_ycurve_load = FALSE;

		if (p_cfg->p_iqcfg->gamma.enable && (p_cfg->p_iqcfg->update & KDRV_IPE_UPDATE_GAMMA)) {
			b_gamma_load = TRUE;
		}

		if (p_cfg->p_iqcfg->y_curve.enable && (p_cfg->p_iqcfg->update & KDRV_IPE_UPDATE_YCURVE)) {
			b_ycurve_load = TRUE;
		}

		if (b_gamma_load || b_ycurve_load) {
			if (b_gamma_load && b_ycurve_load) {
				if (p_cfg->p_iqcfg->gamma.option == KDRV_IPE_IQ_GAMMA_RGB_SEPERATE) {
					DBG_WRN("seperate do not surpport\r\n");
				} else {
					load_info.type = KDRV_IPE_GAMMA_R_Y;
				}
			} else if (b_gamma_load){
				if (p_cfg->p_iqcfg->gamma.option == KDRV_IPE_IQ_GAMMA_RGB_SEPERATE) {
					load_info.type = KDRV_IPE_GAMMA_RGB;
				} else {
					load_info.type = KDRV_IPE_GAMMA_R;
				}
			} else {
				load_info.type = KDRV_IPE_GAMMA_Y;
			}

			ipe_eng_set_gamma_rw_enable_hw_reg(p_hdl->p_eng, DMA_WRITE_LUT, load_info.type );
		}

	}
	/* Load 3dcc */
	{
		BOOL cc3d_load = FALSE;
		if (p_cfg->p_iqcfg->_3dcc.enable && (p_cfg->p_iqcfg->update2 & KDRV_IPE_UPDATE2_3DCC)) {
			cc3d_load = TRUE;
		}
		if (cc3d_load) {
			ipe_eng_set_3dcc_rw_enable_hw_reg(p_hdl->p_eng, DMA_WRITE_LUT);
		}

	}
#endif

	//ipe_eng_write_hw_reg(p_hdl->p_eng, 0x58, 0x00080000);//for 539a checksum

	return rt;
}

#if 0
#endif

static INT32 kdrv_ipe_cfg_process_cpu(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	INT32 rt;
	ULONG eng_reg_buf;
	ULONG eng_flg_buf;
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	/* directly set register address to engine driver, prevent write twice */
	eng_reg_buf = p_hdl->p_eng->reg_io_base;
	eng_flg_buf = kdrv_ipe_int_get_eng_flg_buf_addr(p_hdl);
	if (eng_reg_buf == 0 || eng_flg_buf == 0) {
		DBG_ERR("no eng buffer, reg_buf 0x%.8x, flg_buf 0x%.8x\r\n", (unsigned int)eng_reg_buf, (unsigned int)eng_flg_buf);
		return E_NOMEM;
	}
#if IPE_SSDRV_SUPPORT
	ipe_eng_set_reg_buf(p_hdl->p_eng, eng_reg_buf, eng_flg_buf);
#endif

	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
	rt = kdrv_ipe_int_cfg_all(p_hdl, (KDRV_IPE_JOB_CFG *)p_data);
#if KDRV_IPP_ENG_CHECK_ENABLE
	if (rt == E_OK) {
		/* eng check limitation */
		//rt = ipe_eng_chk_limitation(eng_reg_buf, eng_flg_buf);
	}
#endif

	return rt;
}


static INT32 kdrv_ipe_cfg_process_ll(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{

	INT32 rt = E_OK;
	UINT32 i;
	ULONG eng_reg_buf;
	ULONG eng_flg_buf;
	UINT32 *p_reg;
	UINT8 *p_flg;
	KDRV_IPE_JOB_CFG *p_cfg;
	KDRV_IPP_LL_SET_INFO *p_ll_blk;
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	/* reset engine buffer to 0, and set to engine driver */
	eng_reg_buf = kdrv_ipe_int_get_eng_reg_buf_addr(p_hdl);
	eng_flg_buf = kdrv_ipe_int_get_eng_flg_buf_addr(p_hdl);
	if (eng_reg_buf == 0 || eng_flg_buf == 0) {
		DBG_ERR("no eng buffer, reg_buf 0x%.8x, flg_buf 0x%.8x\r\n", (unsigned int)eng_reg_buf, (unsigned int)eng_flg_buf);
		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
		return E_NOMEM;
	}
#if IPE_SSDRV_SUPPORT
	memset((void *)eng_reg_buf, 0, ipe_eng_get_reg_base_buf_size());
	memset((void *)eng_flg_buf, 0, ipe_eng_get_reg_flag_buf_size());

	ipe_eng_set_reg_buf(p_hdl->p_eng, eng_reg_buf, eng_flg_buf);
#endif
	p_cfg = (KDRV_IPE_JOB_CFG *)p_data;
	p_ll_blk = (KDRV_IPP_LL_SET_INFO *)p_cfg->p_ll_blk;
	if (p_ll_blk == NULL) {
		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
		return E_NOMEM;
	}
	if (p_ll_blk->stage == KDRV_IPP_LL_SET_STAGE1) {
		rt = kdrv_ipe_int_cfg_all(p_hdl, p_cfg);
#if KDRV_IPP_ENG_CHECK_ENABLE
		if (rt == E_OK) {
			/* eng check limitation */
			//rt = ipe_eng_chk_limitation(eng_reg_buf, eng_flg_buf);
		}
#endif

		/* write register to linklist */
		p_reg = (UINT32 *)eng_reg_buf;
		p_flg = (UINT8 *)eng_flg_buf;
		for (i = 1; i < IPE_ENG_REG_NUM; i++) { // bypass p_flag[0]
			if (p_flg[i]) {
				kdrv_ipp_util_write_ll_upd_cmd(p_ll_blk, (i << 2), p_reg[i]);
			}
		}

		if(p_flg[0] == TRUE){ // add p_flag[0]'s cmd if need (load gamma)
			kdrv_ipp_util_write_ll_upd_cmd(p_ll_blk, 0, p_reg[0]);
		}

		kdrv_ipp_util_write_ll_null_cmd(p_ll_blk, p_ll_blk->blk_idx);

		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);

		vos_cpu_dcache_sync( (ULONG)p_ll_blk->cmd_buf_addr.va, ALIGN_CEIL_32(p_ll_blk->cur_cmd_idx * KDRV_IPP_LL_CMD_SIZE) , VOS_DMA_TO_DEVICE);
	} else {
		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
		/* reserve for 2 stage configuration */
	}

	return rt;
}

static INT32 kdrv_ipe_trig_single(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT //538 modified
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	ipe_eng_load_hw_reg(p_hdl->p_eng, IPE_START_LOAD);
	ipe_eng_trig_single_hw_reg(p_hdl->p_eng);
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
#endif
	return 0;
}

static INT32 kdrv_ipe_trig_ll(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT
	ULONG ll_addr;
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	ll_addr = *(ULONG*)p_data;

	{
		UINT64 addr = ll_addr;

		// 690 patch. ipe ll blk address's msb use ycurve lut address's msb
		if (p_hdl->ycurve_en) {
			// if ycurve enable, check if ycurve's msb is same as ll blk's msb
			if (p_hdl->ycurve_msb != (UINT32)((addr >> 32) & 0xFFFFFFFF)) {
				DBG_ERR("690 ycurve msb (0x%x) must as same as ll blk msb (0x%x)\r\n", p_hdl->ycurve_msb, (UINT32)((addr >> 32) & 0xFFFFFFFF));
			}
		} else {
			// if ycurve disable, force set ycurve's msb to ll blk's msb
			ipe_eng_set_dma_in_ycurve_lut_msb_addr_buf_reg(p_hdl->p_eng, (UINT32)((addr >> 32) & 0xFFFFFFFF));
		}

		ipe_eng_trig_ll_hw_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)( (addr>> 32) & 0xFFFFFFFF));
	}

	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
#endif
	return 0;
}

static INT32 kdrv_ipe_trig_direct(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT
	UINT32* is_dir_start = (UINT32*) p_data;
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	#if DIRECT_MODE_GLOBAL_LOAD_EN
	if (*is_dir_start) {
		ipe_eng_global_load_hw_reg(p_hdl->p_eng, ENABLE);
		ipe_eng_trig_single_hw_reg(p_hdl->p_eng);
	}
	#else
	ipe_eng_load_hw_reg(p_hdl->p_eng, IPE_DIRECT_START_LOAD);
	if (*is_dir_start) {
		ipe_eng_trig_single_hw_reg(p_hdl->p_eng);
	}
	#endif

	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
#endif
	return 0;
}

static INT32 kdrv_ipe_set_cb(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	p_hdl->cb = (KDRV_IPP_ISR_CB) p_data;
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);

	return 0;
}

static INT32 kdrv_ipe_stop_single(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	ipe_eng_stop_single_hw_reg(p_hdl->p_eng);
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
#endif
	return 0;
}

static INT32 kdrv_ipe_load_gamma(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if 0
#if IPE_SSDRV_SUPPORT
	KDRV_IPE_GAMMA_LOAD_INFO *p_load_info = (KDRV_IPE_GAMMA_LOAD_INFO *)p_data;
	ULONG eng_reg_buf;
	ULONG eng_flg_buf;
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	/* ll mode must write sai to reg before load */
	if (p_load_info->gamma_addr || p_load_info->ycurve_addr) {
		eng_reg_buf = p_hdl->p_eng->reg_io_base;
		eng_flg_buf = kdrv_ipe_int_get_eng_flg_buf_addr(p_hdl);
		if (eng_reg_buf == 0 || eng_flg_buf == 0) {
			DBG_ERR("no eng buffer, reg_buf 0x%.8lx, flg_buf 0x%.8lx\r\n", (ULONG)eng_reg_buf, (ULONG)eng_flg_buf);
			vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
			return E_NOMEM;
		}
		ipe_eng_set_reg_buf(p_hdl->p_eng, eng_reg_buf, eng_flg_buf);

		if (p_load_info->gamma_addr) {

			{
				UINT64 addr = p_load_info->gamma_phyaddr;

				ipe_eng_set_dma_in_gamma_lut_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
				//DBG_DUMP("-!!!!!!!!!!!!!!! gamma [%lx] !!!!!!!!!!!!!!!-\n",p_load_info->gamma_phyaddr);
			}
		}

		if (p_load_info->ycurve_addr) {

			{
				UINT64 addr = p_load_info->ycurve_phyaddr;

				ipe_eng_set_dma_in_ycurve_lut_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF));
				//DBG_DUMP("-!!!!!!!!!!!!!!! ycurve [%lx] !!!!!!!!!!!!!!!-\n",p_load_info->ycurve_phyaddr);
			}
		}
	}

	ipe_eng_set_gamma_rw_enable_hw_reg(p_hdl->p_eng, DMA_WRITE_LUT, p_load_info->type);

	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
#endif
#endif
	return 0;
}

static INT32 kdrv_ipe_gamma_trans(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT
	KDRV_IPE_GAMMA_TRANS_INFO* p_gamma_info = (KDRV_IPE_GAMMA_TRANS_INFO*) p_data;

	ipe_eng_set_gamma_curve_to_dram(p_gamma_info->p_gamma, p_gamma_info->p_r_lut, p_gamma_info->p_g_lut, p_gamma_info->p_b_lut);

#endif
	return 0;
}

static INT32 kdrv_ipe_ycurve_trans(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	INT32 i=0;
	KDRV_IPE_YCURVE_TRANS_INFO* p_ycurve_info = (KDRV_IPE_YCURVE_TRANS_INFO*) p_data;

	if(p_ycurve_info->ycurv_sel == 2 ){

		for(i = KDRV_IPE_YCURVE_PRE_LEN-1; i >= 0; i--){

			if(p_ycurve_info->p_y_lut[i] > 255){
				DBG_ERR("ycurve_sel is 2 but lut > 255\n");
				break;
			}
		}
	}
#if IPE_SSDRV_SUPPORT
	ipe_eng_set_y_curve_to_dram(p_ycurve_info->p_ycurve, p_ycurve_info->p_y_lut);
#endif
	return 0;
}
/*538 add*/
static INT32 kdrv_ipe_3dcc_trans(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT
	KDRV_IPE_3DCC_TRANS_INFO* p_3dcc_info = (KDRV_IPE_3DCC_TRANS_INFO*) p_data;
	ipe_eng_set_3dcc_to_dram(p_3dcc_info->p_3dcc, p_3dcc_info->p_3d_lut);
#endif
	return 0;
}

static INT32 kdrv_ipe_hard_reset(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	ipe_eng_hard_reset_hw_reg(p_hdl->p_eng);
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
#endif
	return 0;
}

static INT32 kdrv_ipe_defog_stcs(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	KDRV_IPE_DEFOG_STCS_RESULT *p_result;
	DEFOG_STCS_RSLT eng_result = {0};
#if IPE_SSDRV_SUPPORT
	ipe_eng_get_defog_stcs_hw_reg(p_hdl->p_eng, &eng_result);
#endif
	p_result = (KDRV_IPE_DEFOG_STCS_RESULT *)p_data;
	p_result->airlight[0] = eng_result.airlight[0];
	p_result->airlight[1] = eng_result.airlight[1];
	p_result->airlight[2] = eng_result.airlight[2];

	return 0;
}

static INT32 kdrv_ipe_edge_stcs(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	KDRV_IPE_EDGE_STCS_RESULT *p_result = NULL;
	IPE_EDGE_STCS_RSLT eng_result = {0};
#if IPE_SSDRV_SUPPORT
	ipe_get_edge_stcs_hw_reg(p_hdl->p_eng, &eng_result);
#endif
	p_result = (KDRV_IPE_EDGE_STCS_RESULT *)p_data;
	p_result->localmax_max = eng_result.localmax_max;
	p_result->coneng_max = eng_result.coneng_max;
	p_result->coneng_avg = eng_result.coneng_avg;

	return 0;
}

static INT32 kdrv_ipe_va_rst(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT
	KDRV_IPE_VA_RST *p_result;
	IPE_VA_RSLT rslt = {0};
	IPE_VA_SETTING setting = {0};

	p_result = (KDRV_IPE_VA_RST *)p_data;
	rslt.p_g1_h = p_result->g1_h;
	rslt.p_g1_v = p_result->g1_v;
	rslt.p_g1_h_cnt = p_result->g1_h_cnt;
	rslt.p_g1_v_cnt = p_result->g1_v_cnt;
	rslt.p_g2_h = p_result->g2_h;
	rslt.p_g2_v = p_result->g2_v;
	rslt.p_g2_h_cnt = p_result->g2_h_cnt;
	rslt.p_g2_v_cnt = p_result->g2_v_cnt;

	setting.va_en = p_result->va_setting.va_en;
	setting.lineoffset = p_result->va_setting.lineoffset;
	setting.outsel = p_result->va_setting.outsel;
	setting.address = p_result->va_setting.address.va;
	setting.win_num_x = p_result->va_setting.win_num_x;
	setting.win_num_y = p_result->va_setting.win_num_y;

	vos_cpu_dcache_sync(p_result->va_setting.address.va, KDRV_IPE_VA_BUF_SIZE, VOS_DMA_FROM_DEVICE);

	ipe_eng_get_va_result_buf_reg(&setting, &rslt);
#endif

	return 0;
}

static INT32 kdrv_ipe_va_rst_dual(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT
#if(IPE_538_KDRV_FEATURE == 0)
	KDRV_IPE_VA_RST_DUAL *p_result;
	IPE_VA_RSLT rslt = {0};
	//IPE_DUAL_VA_SETTING setting = {0};

	p_result = (KDRV_IPE_VA_RST_DUAL *)p_data;
	rslt.p_g1_h = p_result->g1_h;
	rslt.p_g1_v = p_result->g1_v;
	rslt.p_g1_h_cnt = p_result->g1_h_cnt;
	rslt.p_g1_v_cnt = p_result->g1_v_cnt;
	rslt.p_g2_h = p_result->g2_h;
	rslt.p_g2_v = p_result->g2_v;
	rslt.p_g2_h_cnt = p_result->g2_h_cnt;
	rslt.p_g2_v_cnt = p_result->g2_v_cnt;

	setting.va_en = p_result->va_setting.va_en;
	setting.lineoffset = p_result->va_setting.lineoffset;
	setting.outsel = p_result->va_setting.outsel;
	setting.address_pipe1 = p_result->va_setting.address_pipe1;
	setting.address_pipe2 = p_result->va_setting.address_pipe2;
	setting.win_num_x = p_result->va_setting.win_num_x;
	setting.win_num_y = p_result->va_setting.win_num_y;

	setting.win_start_x = p_result->va_setting.win_start_x;
	setting.win_size_x = p_result->va_setting.win_size_x;
	setting.win_skip_x = p_result->va_setting.win_skip_x;
	setting.dual_cut_pos = p_result->va_setting.dual_cut_pos;
	setting.address_out = p_result->va_setting.address_out;
	vos_cpu_dcache_sync(p_result->va_setting.address_pipe1, KDRV_IPE_VA_BUF_SIZE, VOS_DMA_FROM_DEVICE);
	vos_cpu_dcache_sync(p_result->va_setting.address_pipe2, KDRV_IPE_VA_BUF_SIZE, VOS_DMA_FROM_DEVICE);
	ipe_eng_get_dual_va_result_buf_reg(&setting, &rslt);
#endif
#endif

	return 0;
}

static INT32 kdrv_ipe_inde_va_rst(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	UINT32 i;
	KDRV_IPE_VA_INDEP_RSLT *p_va_rst;
	IPE_INDEP_VA_WIN_RSLT rst = {0};

	p_va_rst = (KDRV_IPE_VA_INDEP_RSLT *)p_data;


	for (i = 0; i < KDRV_IPE_IQ_VA_INDEP_NUM; i++) {
#if IPE_SSDRV_SUPPORT
		ipe_eng_get_indep_va_win_rslt_hw_reg(p_hdl->p_eng, &rst, i);
#endif
		p_va_rst->g1_h[i] = rst.va_g1_h;
		p_va_rst->g1_v[i] = rst.va_g1_v;
		p_va_rst->g2_h[i] = rst.va_g2_h;
		p_va_rst->g2_v[i] = rst.va_g2_v;
		p_va_rst->g1_h_cnt[i] = rst.va_cnt_g1_h;
		p_va_rst->g1_v_cnt[i] = rst.va_cnt_g1_v;
		p_va_rst->g2_h_cnt[i] = rst.va_cnt_g2_h;
		p_va_rst->g2_v_cnt[i] = rst.va_cnt_g2_v;
	}
	return E_OK;
}

static INT32 kdrv_ipe_dma_ch_abort(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	ULONG eng_reg_buf;
	ULONG eng_flg_buf;
#if IPE_SSDRV_SUPPORT
	UINT32 *dma_abort_en;
#endif
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	/* disable all interrupt before disable */
	eng_reg_buf = p_hdl->p_eng->reg_io_base;
	eng_flg_buf = kdrv_ipe_int_get_eng_flg_buf_addr(p_hdl);
	if (eng_reg_buf == 0 || eng_flg_buf == 0) {
		DBG_ERR("no eng buffer, reg_buf 0x%.8lx, flg_buf 0x%.8lx\r\n", (ULONG)eng_reg_buf, (ULONG)eng_flg_buf);

		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
		return E_NOMEM;
	}
#if IPE_SSDRV_SUPPORT
	ipe_eng_set_reg_buf(p_hdl->p_eng, eng_reg_buf, eng_flg_buf);
	ipe_eng_set_interrupt_enable_buf_reg(p_hdl->p_eng, 0);

	dma_abort_en = (UINT32*) p_data;
	ipe_eng_set_dma_channel_enable_hw_reg(p_hdl->p_eng, *dma_abort_en);
#endif
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
	return E_OK;
}

static INT32 kdrv_ipe_dma_ch_sts(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT
	UINT32 *dma_ch_sts;

	dma_ch_sts = (UINT32*) p_data;

	*dma_ch_sts  = ipe_eng_get_dma_channel_status_hw_reg(p_hdl->p_eng);
#endif
	return E_OK;
}

static INT32 kdrv_ipe_reg_base_addr(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	*(ULONG *) p_data = p_hdl->p_eng->reg_io_base;
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);

	return E_OK;
}

static INT32 kdrv_ipe_reg_base_addr_ll(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	*(ULONG *) p_data = kdrv_ipe_int_get_eng_reg_buf_addr(p_hdl);
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);

	return E_OK;
}

static INT32 kdrv_ipe_get_dbg_info(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
	KDRV_IPE_DBG_INFO* info =(KDRV_IPE_DBG_INFO*) p_data;

	info->ipe_frm_ed_cnt = p_hdl->p_ipe_dbg_info->ipe_frm_ed_cnt;
	info->ipe_frm_st_cnt = p_hdl->p_ipe_dbg_info->ipe_frm_st_cnt;
	info->ipe_ll_ed_cnt = p_hdl->p_ipe_dbg_info->ipe_ll_ed_cnt;
	info->ipe_strp_done_cnt = p_hdl->p_ipe_dbg_info->ipe_strp_done_cnt;
	info->ipe_load_gamma_ed_cnt = p_hdl->p_ipe_dbg_info->ipe_load_gamma_ed_cnt;
	info->ipe_defog_out_ed_cnt = p_hdl->p_ipe_dbg_info->ipe_defog_out_ed_cnt;
	info->ipe_va_out_ed_cnt = p_hdl->p_ipe_dbg_info->ipe_va_out_ed_cnt;
	info->ipe_ycbcr_out_ed_cnt = p_hdl->p_ipe_dbg_info->ipe_ycbcr_out_ed_cnt;
	info->ipe_ll_error_cnt = p_hdl->p_ipe_dbg_info->ipe_ll_error_cnt;
	info->ipe_ll_error2_cnt = p_hdl->p_ipe_dbg_info->ipe_ll_error2_cnt;
	info->ipe_frame_err_cnt = p_hdl->p_ipe_dbg_info->ipe_frame_err_cnt;

	return E_OK;
}

static INT32 kdrv_ipe_get_hw_airlight(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT //538 modified
	UINT16* result_airlight = (UINT16 *)p_data;

	ipe_eng_get_defog_airlight_setting_hw_reg(p_hdl->p_eng, result_airlight);
#endif
	return E_OK;
}

static INT32 kdrv_ipe_get_merged_airlight(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT //538 modified
#if(IPE_538_KDRV_FEATURE == 0)
	KDRV_IPP_CB_STCS_OUTPUT* stcs_output = (KDRV_IPP_CB_STCS_OUTPUT* )p_data;
	DEFOG_STCS_MERGE left_stcs = {0};
	DEFOG_STCS_MERGE right_stcs = {0};

	left_stcs.p_airlight = stcs_output->defog_stcs_rslt.airlight;
	right_stcs.p_airlight = stcs_output->defog_stcs_rslt_tmp.airlight;

	ipe_eng_get_merged_defog_stcs_hw_reg(&left_stcs, &right_stcs);
#endif
#endif
	return E_OK;
}

static INT32 kdrv_ipe_get_merged_indp_va(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT //538 modified
#if(IPE_538_KDRV_FEATURE == 0)
	KDRV_IPP_CB_STCS_OUTPUT* stcs_output = (KDRV_IPP_CB_STCS_OUTPUT* )p_data;
	IPE_INDEP_VA_WIN_MERGE left_indp_va = {0};
	IPE_INDEP_VA_WIN_MERGE right_indp_va = {0};

	left_indp_va.p_va_g1_h = stcs_output->ipe_inde_va_rslt.g1_h;
	left_indp_va.p_va_g1_v = stcs_output->ipe_inde_va_rslt.g1_v;
	left_indp_va.p_va_g2_h = stcs_output->ipe_inde_va_rslt.g2_h;
	left_indp_va.p_va_g2_v = stcs_output->ipe_inde_va_rslt.g2_v;
	left_indp_va.p_va_cnt_g1_h = stcs_output->ipe_inde_va_rslt.g1_h_cnt;
	left_indp_va.p_va_cnt_g1_v = stcs_output->ipe_inde_va_rslt.g1_v_cnt;
	left_indp_va.p_va_cnt_g2_h = stcs_output->ipe_inde_va_rslt.g2_h_cnt;
	left_indp_va.p_va_cnt_g2_v = stcs_output->ipe_inde_va_rslt.g2_v_cnt;

	right_indp_va.p_va_g1_h = stcs_output->ipe_inde_va_rslt_tmp.g1_h;
	right_indp_va.p_va_g1_v = stcs_output->ipe_inde_va_rslt_tmp.g1_v;
	right_indp_va.p_va_g2_h = stcs_output->ipe_inde_va_rslt_tmp.g2_h;
	right_indp_va.p_va_g2_v = stcs_output->ipe_inde_va_rslt_tmp.g2_v;
	right_indp_va.p_va_cnt_g1_h = stcs_output->ipe_inde_va_rslt_tmp.g1_h_cnt;
	right_indp_va.p_va_cnt_g1_v = stcs_output->ipe_inde_va_rslt_tmp.g1_v_cnt;
	right_indp_va.p_va_cnt_g2_h = stcs_output->ipe_inde_va_rslt_tmp.g2_h_cnt;
	right_indp_va.p_va_cnt_g2_v = stcs_output->ipe_inde_va_rslt_tmp.g2_v_cnt;
	ipe_eng_get_merged_indep_va_win_rslt_hw_reg(&left_indp_va, &right_indp_va);
#endif
#endif
	return E_OK;
}

static INT32 kdrv_ipe_get_merged_edge_stsc(KDRV_IPE_HANDLE *p_hdl, void *p_data)
{
#if IPE_SSDRV_SUPPORT //538 modified
#if(IPE_538_KDRV_FEATURE == 0)
	KDRV_IPP_CB_STCS_OUTPUT* stcs_output = (KDRV_IPP_CB_STCS_OUTPUT* )p_data;
	IPE_EDGE_STCS_MERGE left_edge_stcs = {0};
	IPE_EDGE_STCS_MERGE right_edge_stcs = {0};

	left_edge_stcs.coneng_avg = &stcs_output->edge_stcs_rslt.coneng_avg;
	left_edge_stcs.coneng_max = &stcs_output->edge_stcs_rslt.coneng_max;
	left_edge_stcs.localmax_max = &stcs_output->edge_stcs_rslt.localmax_max;

	right_edge_stcs.coneng_avg = &stcs_output->edge_stcs_rslt_tmp.coneng_avg;
	right_edge_stcs.coneng_max = &stcs_output->edge_stcs_rslt_tmp.coneng_max;
	right_edge_stcs.localmax_max = &stcs_output->edge_stcs_rslt_tmp.localmax_max;

	ipe_get_merged_edge_stcs_hw_reg(&left_edge_stcs, &right_edge_stcs);
#endif
#endif
	return E_OK;
}


static KDRV_IPE_FUNC_ITEM g_kdrv_ipe_func_tab[KDRV_IPE_PARAM_ID_MAX] = {
	{kdrv_ipe_cfg_process_cpu,	1, NULL, 						0, "cfg_process_cpu"},
	{kdrv_ipe_cfg_process_ll,	1, NULL, 						0, "cfg_process_ll"},
	{kdrv_ipe_trig_single,		0, NULL, 						0, "trig_single"},
	{kdrv_ipe_trig_ll,			1, NULL, 						0, "trig_ll"},
	{kdrv_ipe_trig_direct,		1, NULL, 						0, "trig_dir"},
	{kdrv_ipe_set_cb,			1, NULL,				 		0, "set_cb"},
	{kdrv_ipe_stop_single,		0, NULL, 						0, "stop_single"},
	{kdrv_ipe_load_gamma,		1, NULL, 						0, "load gamma"},
	{NULL,						0, kdrv_ipe_gamma_trans, 		1, "gamma trans"},
	{NULL,						0, kdrv_ipe_ycurve_trans, 		1, "curve trans"},
	{NULL,						0, kdrv_ipe_3dcc_trans, 		1, "3dcc trans"},
	{kdrv_ipe_hard_reset,		0, NULL, 						0, "hard reset"},
	{NULL,						0, kdrv_ipe_defog_stcs, 		1, "defog stcs"},
	{NULL,						0, kdrv_ipe_edge_stcs,	 		1, "edge stcs"},
	{NULL,						0, kdrv_ipe_va_rst,	 			1, "va rst"},
	{NULL,						0, kdrv_ipe_va_rst_dual,		1, "va rst_dual"},
	{NULL,						0, kdrv_ipe_inde_va_rst, 		1, "va inde rst"},
	{kdrv_ipe_dma_ch_abort,		1, NULL,  						0, "dma abort en"},
	{NULL,						0, kdrv_ipe_dma_ch_sts, 		1, "dma ch status"},
	{NULL,						0, kdrv_ipe_reg_base_addr,		1, "reg base addr"},
	{NULL,						0, kdrv_ipe_reg_base_addr_ll,	1, "reg base addr ll mode"},
	{NULL,						0, kdrv_ipe_get_dbg_info,		1, "dbg_info"},
	{NULL,						0, kdrv_ipe_get_hw_airlight,	1, "hw air light"},
	{NULL,						0, kdrv_ipe_get_merged_airlight,1, "merged air light"},
	{NULL,						0, kdrv_ipe_get_merged_indp_va ,1, "merged indp_va"},
	{NULL,						0, kdrv_ipe_get_merged_edge_stsc,1, "merged edge stsc"},
};

INT32 kdrv_ipe_set(UINT32 id, KDRV_IPE_PARAM_ID param_id, VOID *p_data)
{
	KDRV_IPE_HANDLE *p_hdl;
	//unsigned long loc_flg;
	INT32 rt;

	if (param_id >= KDRV_IPE_PARAM_ID_MAX) {
		DBG_ERR("param_id %d overflow\r\n", (int)param_id);
		return E_PAR;
	}

	p_hdl = kdrv_ipe_int_get_handle(KDRV_DEV_ID_CHIP(id), KDRV_DEV_ID_ENGINE(id));
	if (p_hdl == NULL) {
		DBG_ERR("null handle of id 0x%.8x\r\n", (unsigned int)id);
		return E_PAR;
	}

	//vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	rt = E_SYS;
	if (g_kdrv_ipe_func_tab[param_id].set_chk_data) {
		if (p_data == NULL) {
			DBG_ERR("p_data is null pointer\r\n");
			rt = E_PAR;
			goto err;
		}
	}

	if (g_kdrv_ipe_func_tab[param_id].set_fp) {
		rt = g_kdrv_ipe_func_tab[param_id].set_fp(p_hdl, p_data);
		if (rt < 0) {
			DBG_ERR("%s err, rt %d\r\n", g_kdrv_ipe_func_tab[param_id].msg, rt);
		}
	}

err:
	//vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
	return rt;
}

INT32 kdrv_ipe_get(UINT32 id, KDRV_IPE_PARAM_ID param_id, VOID *p_data)
{
	KDRV_IPE_HANDLE *p_hdl;
	//unsigned long loc_flg;
	INT32 rt;

	if (param_id >= KDRV_IPE_PARAM_ID_MAX) {
		DBG_ERR("param_id %d overflow\r\n", (int)param_id);
		return E_PAR;
	}

	p_hdl = kdrv_ipe_int_get_handle(KDRV_DEV_ID_CHIP(id), KDRV_DEV_ID_ENGINE(id));
	if (p_hdl == NULL) {
		DBG_ERR("null handle of id 0x%.8x\r\n", (unsigned int)id);
		return E_PAR;
	}

	//vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	rt = E_SYS;
	if (g_kdrv_ipe_func_tab[param_id].get_chk_data) {
		if (p_data == NULL) {
			DBG_ERR("p_data is null pointer\r\n");
			rt = E_PAR;
			goto err;
		}
	}

	if (g_kdrv_ipe_func_tab[param_id].get_fp) {
		rt = g_kdrv_ipe_func_tab[param_id].get_fp(p_hdl, p_data);
		if (rt < 0) {
			DBG_ERR("%s err, rt %d\r\n", g_kdrv_ipe_func_tab[param_id].msg, rt);
		}
	}

err:
	//vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
	return rt;
}

#if 0
#endif

INT32 kdrv_ipe_query_reg_num(UINT32 id, void *p_data)
{
	*(UINT32 *)p_data = IPE_ENG_REG_NUM;

	return E_OK;
}

static INT32 kdrv_ipe_query_strp_info(UINT32 id, void *p_data)
{
#if IPE_SSDRV_SUPPORT
	IPE_STP_CAL_LIMT info;
	KDRV_IPE_STRP_INFO *p_info;

	ipe_eng_get_stripe_cal_limt(&info);

	p_info = (KDRV_IPE_STRP_INFO *)p_data;
	p_info->alignment = info.stp_size_unit;
	p_info->overlap = info.stp_overlap_size;
#endif
	return E_OK;
}


static KDRV_IPE_QUERY_ITEM g_kdrv_ipe_query_tab[KDRV_IPE_QUERY_ID_MAX] =
{
	{kdrv_ipe_query_reg_num,	"reg number"},
	{kdrv_ipe_query_strp_info,	"stripe info"}
};

INT32 kdrv_ipe_query(UINT32 id, KDRV_IPE_QUERY_ID qid, void *p_param)
{
	INT32 rt;

	if (qid >= KDRV_IPE_QUERY_ID_MAX) {
		DBG_ERR("query_id %d overflow\r\n", (int)qid);
		return E_PAR;
	}


	rt = E_SYS;
	if (g_kdrv_ipe_query_tab[qid].query_fp) {
		rt = g_kdrv_ipe_query_tab[qid].query_fp(id, p_param);
		if (rt < 0) {
			DBG_ERR("query %s err, rt %d\r\n", g_kdrv_ipe_query_tab[qid].msg, rt);
		}
	}

	return rt;
}

#if 0
#endif

void kdrv_ipe_dump(void)
{
	/* todo: dump kdrv ctl/handle info */
	DBG_DUMP("---- kdrv_ipe dump info ----\r\n");
}

void kdrv_ipe_dump_cfg(KDRV_IPE_JOB_CFG *p_cfg, int (*KDRV_IPE_DUMP)(const char *fmt, ...))
{
	UINT32 i;
	CHAR *mode_str[KDRV_IPP_OPMODE_MAX] = {
		"NONE",
		"D2D",
		"IFE2IME",
		"DCE2IME",
		"SIE2IME",
		"IFE2IPE",
        "SIE2PRE",
        "PRE2IME",
        "PRE2IPE"
	};

	CHAR *mst_h_ovlp[] = {
		"auto",
		"16"
	};

	KDRV_IPE_IO_CFG *p_iocfg = p_cfg->p_iocfg;
	KDRV_IPE_IQ_CFG *p_iqcfg = p_cfg->p_iqcfg;


	KDRV_IPE_DUMP("---- kdrv_ipe dump cfg ----\r\n");

	KDRV_IPE_DUMP("---------------- iocfg ----------------\r\n");
	KDRV_IPE_DUMP("%8s %8s %8s %8s %8s %10s %10s%10s %10s %12s\r\n",
				"mode", "in_w", "in_h", "in_lof_y", "in_lof_uv", "in_addr_y", "in_addr_uv", "in_fmt", "inte", "mst_h_ovlp");
	KDRV_IPE_DUMP("%8s %8d %8d %8d %8d 0x%.8lx 0x%.8lx 0x%.8x 0x%.8x %12s\r\n",
				mode_str[p_iocfg->mode],
				(int)p_iocfg->in_frm.size.w,
				(int)p_iocfg->in_frm.size.h,
				(int)p_iocfg->in_frm.lofs[0],
				(int)p_iocfg->in_frm.lofs[1],
				(ULONG)p_iocfg->in_frm.addr[0],
				(ULONG)p_iocfg->in_frm.addr[1],
				(unsigned int)p_iocfg->in_frm.fmt,
				(unsigned int)p_iocfg->inte_en,
				mst_h_ovlp[p_iocfg->mst_h_ovlp]);
	KDRV_IPE_DUMP("%8s %8s %8s %8s %8s 0x%.8lx 0x%.8lx %8s %8s %12s\r\n\n",
					" ",
					" ",
					" ",
					" ",
					" ",
					(p_iocfg->in_frm.phyaddr[0] == 0) ? 0 : (ULONG)p_iocfg->in_frm.phyaddr[0],
					(p_iocfg->in_frm.phyaddr[1] == 0) ? 0 : (ULONG)p_iocfg->in_frm.phyaddr[1],
					" ",
					" ",
					" ");

	KDRV_IPE_DUMP("%8s %8s %8s\r\n", "func", "dam_out", "dfg_out");
	KDRV_IPE_DUMP("%16d %8d\r\n\n", (int)p_iocfg->dma_out_en, (int)p_iocfg->sub_out_en);

	if (p_iocfg->dma_out_en) {
		KDRV_IPE_DUMP("%8s %8s %8s %8s %10s %10s%10s\r\n",
					"out_w", "out_h", "out_lof_y", "out_lof_uv", "out_addr_y", "out_addr_uv", "out_fmt");
		KDRV_IPE_DUMP("%8d %8d %8d %8d 0x%.8lx 0x%.8lx 0x%.8x\r\n",
				(int)p_iocfg->out_frm.size.w,
				(int)p_iocfg->out_frm.size.h,
				(int)p_iocfg->out_frm.lofs[0],
				(int)p_iocfg->out_frm.lofs[1],
				(ULONG)p_iocfg->out_frm.addr[0],
				(ULONG)p_iocfg->out_frm.addr[1],
				(unsigned int)p_iocfg->out_frm.fmt);
		KDRV_IPE_DUMP("%8s %8s %8s %8s 0x%.8lx 0x%.8lx %8s\r\n\n",
				" ",
				" ",
				" ",
				" ",
				(p_iocfg->out_frm.phyaddr[0] == 0) ? 0 : (ULONG)p_iocfg->out_frm.phyaddr[0],
				(p_iocfg->out_frm.phyaddr[1] == 0) ? 0 : (ULONG)p_iocfg->out_frm.phyaddr[1],
				" ");
	}

#if(IPE_538_KDRV_FEATURE == 1) //defog/lce shared subimg
	KDRV_IPE_DUMP("%10s %10s %10s \r\n", "dfg_in", "dfg_out", "va_addr");
	KDRV_IPE_DUMP("0x%.8lx 0x%.8lx 0x%.8lx\r\n", (ULONG)p_iocfg->sub_in_addr.va, (ULONG)p_iocfg->sub_out_addr.va, (ULONG)p_iocfg->va_out_addr.va);
	KDRV_IPE_DUMP("0x%.8lx 0x%.8lx 0x%.8lx\r\n\n",
			(p_iocfg->sub_in_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->sub_in_addr.pa,
			(p_iocfg->sub_out_addr.pa == 0) ? 0 : (ULONG)p_iocfg->sub_out_addr.pa,
			(p_iocfg->va_out_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->va_out_addr.pa);

	KDRV_IPE_DUMP("%10s %8s %8s %8s %8s %8s %8s %8s\r\n", "dfg_subimg", "h_size", "v_size", "lofs_in", "lofs_out", "coef0", "coef1", "coef2");
	KDRV_IPE_DUMP("%10s %8d %8d %8d %8d %8d %8d %8d\r\n\n",
			"",
			(int)p_iqcfg->subimg.subimg_size.h_size,
			(int)p_iqcfg->subimg.subimg_size.v_size,
			(int)p_iocfg->subimg_lofs_in,
			(int)p_iocfg->subimg_lofs_out,
			(int)p_iqcfg->subimg.subimg_ftrcoef[0],
			(int)p_iqcfg->subimg.subimg_ftrcoef[1],
			(int)p_iqcfg->subimg.subimg_ftrcoef[2]);
#else
	if (p_iocfg->eth_info.enable) {
		KDRV_IPE_DUMP("%8s %10s %8s %8s %8s %8s %8s\r\n", "eth_fmt", "eth_sel", "h_out_sel", "v_out_sel", "th_low", "th_mid", "h_high");
		KDRV_IPE_DUMP("%8s %10s %8d %8d %8d %8d %8d\r\n\n",
				eth_fmt[p_iocfg->eth_info.out_bit_sel],
				eth_sel[p_iocfg->eth_info.out_sel],
				(int)p_iocfg->eth_info.h_out_sel,
				(int)p_iocfg->eth_info.v_out_sel,
				(int)p_iocfg->eth_info.th_low,
				(int)p_iocfg->eth_info.th_mid,
				(int)p_iocfg->eth_info.th_mid);
	}

	KDRV_IPE_DUMP("%10s %10s %10s \r\n", "dfg_in", "dfg_out", "va_addr");
	KDRV_IPE_DUMP("0x%.8lx 0x%.8lx 0x%.8lx\r\n", (ULONG)p_iocfg->defog_sub_in_addr.va, (ULONG)p_iocfg->defog_sub_out_addr.va, (ULONG)p_iocfg->va_out_addr.va);
	KDRV_IPE_DUMP("0x%.8lx 0x%.8lx 0x%.8lx\r\n\n",
			(p_iocfg->defog_sub_in_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->defog_sub_in_addr.pa,
			(p_iocfg->defog_sub_out_addr.pa == 0) ? 0 : (ULONG)p_iocfg->defog_sub_out_addr.pa,
			(p_iocfg->va_out_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->va_out_addr.pa);

	KDRV_IPE_DUMP("%10s %10s \r\n", "lce_in", "lce_out");
	KDRV_IPE_DUMP("0x%.8lx 0x%.8lx\r\n", (ULONG)p_iocfg->lce_sub_in_addr.va, (ULONG)p_iocfg->lce_sub_out_addr.va);
	KDRV_IPE_DUMP("0x%.8lx 0x%.8lx\r\n\n",
			(p_iocfg->lce_sub_in_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->lce_sub_in_addr.pa,
			(p_iocfg->lce_sub_out_addr.pa == 0) ? 0 : (ULONG)p_iocfg->lce_sub_out_addr.pa);

	KDRV_IPE_DUMP("%10s %8s %8s %8s %8s %8s %8s %8s\r\n", "dfg_subimg", "h_size", "v_size", "lofs_in", "lofs_out", "coef0", "coef1", "coef2");
	KDRV_IPE_DUMP("%10s %8d %8d %8d %8d %8d %8d %8d\r\n\n",
			"",
			(int)p_iqcfg->defog_subimg_param.h_size,
			(int)p_iqcfg->defog_subimg_param.v_size,
			(int)p_iqcfg->defog_subimg_param.subimg_lofs_in,
			(int)p_iqcfg->defog_subimg_param.subimg_lofs_out,
			(int)p_iqcfg->defog_subimg_param.subimg_ftrcoef[0],
			(int)p_iqcfg->defog_subimg_param.subimg_ftrcoef[1],
			(int)p_iqcfg->defog_subimg_param.subimg_ftrcoef[2]);

	KDRV_IPE_DUMP("%10s %8s %8s %8s %8s %8s %8s %8s\r\n", "lce_subimg", "h_size", "v_size", "lofs_in", "lofs_out", "coef0", "coef1", "coef2");
	KDRV_IPE_DUMP("%10s %8d %8d %8d %8d %8d %8d %8d\r\n\n",
			"",
			(int)p_iqcfg->lce_subimg_param.h_size,
			(int)p_iqcfg->lce_subimg_param.v_size,
			(int)p_iqcfg->lce_subimg_param.subimg_lofs_in,
			(int)p_iqcfg->lce_subimg_param.subimg_lofs_out,
			(int)p_iqcfg->lce_subimg_param.subimg_ftrcoef[0],
			(int)p_iqcfg->lce_subimg_param.subimg_ftrcoef[1],
			(int)p_iqcfg->lce_subimg_param.subimg_ftrcoef[2]);
#endif

	KDRV_IPE_DUMP("%6s %12s %8s %12s\r\n",
				"va_win", "ratio_base", "h_ratio", "v_ratio");
	KDRV_IPE_DUMP("%6s %12d %8d %12d\r\n", "", (int)p_iqcfg->va_window.ratio_base, (int)p_iqcfg->va_window.winsz_ratio.w, (int)p_iqcfg->va_window.winsz_ratio.h);
	KDRV_IPE_DUMP("%3s %12s %12s %12s %12s\r\n", "idx", "ind_ratio_x", "ind_ratio_y", "ind_ratio_h", "ind_ratio_v");
	for(i = 0; i < KDRV_IPE_IQ_VA_INDEP_NUM; i++) {
		DBG_DUMP("%3d %12d %12d %12d %12d\r\n",
			(int)i,
			(int)p_iqcfg->va_window.indep_roi_ratio[i].x,
			(int)p_iqcfg->va_window.indep_roi_ratio[i].y,
			(int)p_iqcfg->va_window.indep_roi_ratio[i].w,
			(int)p_iqcfg->va_window.indep_roi_ratio[i].h);
	}

	#if 0
	KDRV_IPE_DUMP("gamma table\r\n");
	for (i = 0; i < KDRV_IPE_GAMMA_LEN ; i++) {
		if ((i+1) % 8 == 0)
			DBG_DUMP(KERN_CONT "0x%.8x\r\n", p_iqcfg->gamma.gamma_lut[i]);
		else
			DBG_DUMP(KERN_CONT "0x%.8x ", p_iqcfg->gamma.gamma_lut[i]);
	}

	KDRV_IPE_DUMP("y curve table\r\n");
	for (i = 0; i < KDRV_IPE_YCURVE_LEN ; i++) {
		if ((i+1) % 8 == 0)
			DBG_DUMP(KERN_CONT "0x%.8x\r\n", p_iqcfg->y_curve.y_curve_lut[i]);
		else
			DBG_DUMP(KERN_CONT "0x%.8x ", p_iqcfg->y_curve.y_curve_lut[i]);
	}
	#endif
}

#if 0
void kdrv_ipe_dump_cfg_log(KDRV_IPE_JOB_CFG *p_cfg)
{
	UINT32 i;
	CHAR *mode_str[KDRV_IPP_OPMODE_MAX] = {
		"NONE",
		"D2D",
		"IFE2IME",
		"DCE2IME",
		"SIE2IME"
	};

	CHAR *mst_h_ovlp[] = {
		"auto",
		"16"
	};

	KDRV_IPE_IO_CFG *p_iocfg = p_cfg->p_iocfg;
	KDRV_IPE_IQ_CFG *p_iqcfg = p_cfg->p_iqcfg;


	printm2("---- kdrv_ipe dump cfg ----\r\n");

	printm2("---------------- iocfg ----------------\r\n");
	printm2("%8s %8s %8s %8s %8s %10s %10s%10s %10s %12s\r\n",
				"mode", "in_w", "in_h", "in_lof_y", "in_lof_uv", "in_addr_y", "in_addr_uv", "in_fmt", "inte", "mst_h_ovlp");
	printm2("%8s %8d %8d %8d %8d 0x%.8lx 0x%.8lx 0x%.8x 0x%.8x %12s\r\n",
				mode_str[p_iocfg->mode],
				(int)p_iocfg->in_frm.size.w,
				(int)p_iocfg->in_frm.size.h,
				(int)p_iocfg->in_frm.lofs[0],
				(int)p_iocfg->in_frm.lofs[1],
				(ULONG)p_iocfg->in_frm.addr[0],
				(ULONG)p_iocfg->in_frm.addr[1],
				(unsigned int)p_iocfg->in_frm.fmt,
				(unsigned int)p_iocfg->inte_en,
				mst_h_ovlp[p_iocfg->mst_h_ovlp]);
	printm2("%8s %8s %8s %8s %8s 0x%.8lx 0x%.8lx %8s %8s %12s\r\n\n",
					" ",
					" ",
					" ",
					" ",
					" ",
					(p_iocfg->in_frm.phyaddr[0] == 0) ? 0 : (ULONG)p_iocfg->in_frm.phyaddr[0],
					(p_iocfg->in_frm.phyaddr[1] == 0) ? 0 : (ULONG)p_iocfg->in_frm.phyaddr[1],
					" ",
					" ",
					" ");

	printm2("%8s %8s %8s\r\n", "func", "dam_out", "dfg_out");
	printm2("%16d %8d\r\n\n", (int)p_iocfg->dma_out_en, (int)p_iocfg->sub_out_en);

	if (p_iocfg->dma_out_en) {
		printm2("%8s %8s %8s %8s %10s %10s%10s\r\n",
					"out_w", "out_h", "out_lof_y", "out_lof_uv", "out_addr_y", "out_addr_uv", "out_fmt");
		printm2("%8d %8d %8d %8d 0x%.8lx 0x%.8lx 0x%.8x\r\n",
				(int)p_iocfg->out_frm.size.w,
				(int)p_iocfg->out_frm.size.h,
				(int)p_iocfg->out_frm.lofs[0],
				(int)p_iocfg->out_frm.lofs[1],
				(ULONG)p_iocfg->out_frm.addr[0],
				(ULONG)p_iocfg->out_frm.addr[1],
				(unsigned int)p_iocfg->out_frm.fmt);
		printm2("%8s %8s %8s %8s 0x%.8lx 0x%.8lx %8s\r\n\n",
				" ",
				" ",
				" ",
				" ",
				(p_iocfg->out_frm.phyaddr[0] == 0) ? 0 : (ULONG)p_iocfg->out_frm.phyaddr[0],
				(p_iocfg->out_frm.phyaddr[1] == 0) ? 0 : (ULONG)p_iocfg->out_frm.phyaddr[1],
				" ");
	}

#if(IPE_538_KDRV_FEATURE == 1) //defog/lce shared subimg
	printm2("%10s %10s %10s \r\n", "dfg_in", "dfg_out", "va_addr");
	printm2("0x%.8lx 0x%.8lx 0x%.8lx\r\n", (ULONG)p_iocfg->sub_in_addr.va, (ULONG)p_iocfg->sub_out_addr.va, (ULONG)p_iocfg->va_out_addr.va);
	printm2("0x%.8lx 0x%.8lx 0x%.8lx\r\n\n",
			(p_iocfg->sub_in_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->sub_in_addr.pa,
			(p_iocfg->sub_out_addr.pa == 0) ? 0 : (ULONG)p_iocfg->sub_out_addr.pa,
			(p_iocfg->va_out_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->va_out_addr.pa);

	printm2("%10s %8s %8s %8s %8s %8s %8s %8s\r\n", "dfg_subimg", "h_size", "v_size", "lofs_in", "lofs_out", "coef0", "coef1", "coef2");
	printm2("%10s %8d %8d %8d %8d %8d %8d %8d\r\n\n",
			"",
			(int)p_iqcfg->subimg.subimg_size.h_size,
			(int)p_iqcfg->subimg.subimg_size.v_size,
			(int)p_iocfg->subimg_lofs_in,
			(int)p_iocfg->subimg_lofs_out,
			(int)p_iqcfg->subimg.subimg_ftrcoef[0],
			(int)p_iqcfg->subimg.subimg_ftrcoef[1],
			(int)p_iqcfg->subimg.subimg_ftrcoef[2]);

#else
	if (p_iocfg->eth_info.enable) {
		printm2("%8s %10s %8s %8s %8s %8s %8s\r\n", "eth_fmt", "eth_sel", "h_out_sel", "v_out_sel", "th_low", "th_mid", "h_high");
		printm2("%8s %10s %8d %8d %8d %8d %8d\r\n\n",
				eth_fmt[p_iocfg->eth_info.out_bit_sel],
				eth_sel[p_iocfg->eth_info.out_sel],
				(int)p_iocfg->eth_info.h_out_sel,
				(int)p_iocfg->eth_info.v_out_sel,
				(int)p_iocfg->eth_info.th_low,
				(int)p_iocfg->eth_info.th_mid,
				(int)p_iocfg->eth_info.th_mid);
	}

	printm2("%10s %10s %10s \r\n", "dfg_in", "dfg_out", "va_addr");
	printm2("0x%.8lx 0x%.8lx 0x%.8lx\r\n", (ULONG)p_iocfg->defog_sub_in_addr.va, (ULONG)p_iocfg->defog_sub_out_addr.va, (ULONG)p_iocfg->va_out_addr.va);
	printm2("0x%.8lx 0x%.8lx 0x%.8lx\r\n\n",
			(p_iocfg->defog_sub_in_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->defog_sub_in_addr.pa,
			(p_iocfg->defog_sub_out_addr.pa == 0) ? 0 : (ULONG)p_iocfg->defog_sub_out_addr.pa,
			(p_iocfg->va_out_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->va_out_addr.pa);

	printm2("%10s %10s \r\n", "lce_in", "lce_out");
	printm2("0x%.8lx 0x%.8lx\r\n", (ULONG)p_iocfg->lce_sub_in_addr.va, (ULONG)p_iocfg->lce_sub_out_addr.va);
	printm2("0x%.8lx 0x%.8lx\r\n\n",
			(p_iocfg->lce_sub_in_addr.pa == 0)  ? 0 : (ULONG)p_iocfg->lce_sub_in_addr.pa,
			(p_iocfg->lce_sub_out_addr.pa == 0) ? 0 : (ULONG)p_iocfg->lce_sub_out_addr.pa);

	printm2("%10s %8s %8s %8s %8s %8s %8s %8s\r\n", "dfg_subimg", "h_size", "v_size", "lofs_in", "lofs_out", "coef0", "coef1", "coef2");
	printm2("%10s %8d %8d %8d %8d %8d %8d %8d\r\n\n",
			"",
			(int)p_iqcfg->defog_subimg_param.h_size,
			(int)p_iqcfg->defog_subimg_param.v_size,
			(int)p_iqcfg->defog_subimg_param.subimg_lofs_in,
			(int)p_iqcfg->defog_subimg_param.subimg_lofs_out,
			(int)p_iqcfg->defog_subimg_param.subimg_ftrcoef[0],
			(int)p_iqcfg->defog_subimg_param.subimg_ftrcoef[1],
			(int)p_iqcfg->defog_subimg_param.subimg_ftrcoef[2]);

	printm2("%10s %8s %8s %8s %8s %8s %8s %8s\r\n", "lce_subimg", "h_size", "v_size", "lofs_in", "lofs_out", "coef0", "coef1", "coef2");
	printm2("%10s %8d %8d %8d %8d %8d %8d %8d\r\n\n",
			"",
			(int)p_iqcfg->lce_subimg_param.h_size,
			(int)p_iqcfg->lce_subimg_param.v_size,
			(int)p_iqcfg->lce_subimg_param.subimg_lofs_in,
			(int)p_iqcfg->lce_subimg_param.subimg_lofs_out,
			(int)p_iqcfg->lce_subimg_param.subimg_ftrcoef[0],
			(int)p_iqcfg->lce_subimg_param.subimg_ftrcoef[1],
			(int)p_iqcfg->lce_subimg_param.subimg_ftrcoef[2]);
#endif


	printm2("%6s %12s %8s %12s\r\n",
				"va_win", "ratio_base", "h_ratio", "v_ratio");
	printm2("%6s %12d %8d %12d\r\n", "", (int)p_iqcfg->va_window.ratio_base, (int)p_iqcfg->va_window.winsz_ratio.w, (int)p_iqcfg->va_window.winsz_ratio.h);
	printm2("%3s %12s %12s %12s %12s\r\n", "idx", "ind_ratio_x", "ind_ratio_y", "ind_ratio_h", "ind_ratio_v");
	for(i = 0; i < KDRV_IPE_IQ_VA_INDEP_NUM; i++) {
		printm2("%3d %12d %12d %12d %12d\r\n",
			(int)i,
			(int)p_iqcfg->va_window.indep_roi_ratio[i].x,
			(int)p_iqcfg->va_window.indep_roi_ratio[i].y,
			(int)p_iqcfg->va_window.indep_roi_ratio[i].w,
			(int)p_iqcfg->va_window.indep_roi_ratio[i].h);
	}

	#if 0
	DBG_DUMP("gamma table\r\n");
	for (i = 0; i < KDRV_IPE_GAMMA_LEN ; i++) {
		if ((i+1) % 8 == 0)
			printm2(KERN_CONT "0x%.8x\r\n", p_iqcfg->gamma.gamma_lut[i]);
		else
			printm2(KERN_CONT "0x%.8x ", p_iqcfg->gamma.gamma_lut[i]);
	}

	DBG_DUMP("y curve table\r\n");
	for (i = 0; i < KDRV_IPE_YCURVE_LEN ; i++) {
		if ((i+1) % 8 == 0)
			printm2(KERN_CONT "0x%.8x\r\n", p_iqcfg->y_curve.y_curve_lut[i]);
		else
			printm2(KERN_CONT "0x%.8x ", p_iqcfg->y_curve.y_curve_lut[i]);
	}
	#endif
}
#endif

void kdrv_ipe_dump_register(void)
{
	KDRV_IPE_HANDLE *p_hdl;
	UINT32 i;

	DBG_DUMP("---- kdrv_ipe dump register ----\r\n");
	for (i = 0; i < g_kdrv_ipe_ctl.total_ch; i++) {
		p_hdl = &g_kdrv_ipe_ctl.p_hdl[i];
		if (p_hdl->p_eng) {
			DBG_DUMP("chip_id = 0x%.8x, eng_id = 0x%.8x\r\n",
				(unsigned int)p_hdl->chip_id, (unsigned int)p_hdl->eng_id);
#if IPE_SSDRV_SUPPORT
#if defined(__FREERTOS)
			debug_dumpmem(p_hdl->p_eng->reg_io_base, ipe_eng_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
#else
			debug_dumpmem(vos_cpu_get_phy_addr(p_hdl->p_eng->reg_io_base), ipe_eng_get_reg_base_buf_size());
#endif
#endif
		}
	}
}

void kdrv_ipe_dump_register_log(void)
{
	KDRV_IPE_HANDLE *p_hdl;
	UINT32 i;

	printm2("---- kdrv_ipe dump register ----\r\n");
	for (i = 0; i < g_kdrv_ipe_ctl.total_ch; i++) {
		p_hdl = &g_kdrv_ipe_ctl.p_hdl[i];
		if (p_hdl->p_eng) {
			printm2("chip_id = 0x%.8x, eng_id = 0x%.8x\r\n",
				(unsigned int)p_hdl->chip_id, (unsigned int)p_hdl->eng_id);
#if IPE_SSDRV_SUPPORT
#if defined(__FREERTOS)
			debug_dumpmem(p_hdl->p_eng->reg_io_base, ipe_eng_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
#else
			printm2("virtual_addr = 0x%lx\r\n", p_hdl->p_eng->reg_io_base);
			kdrv_ipp_debug_dumpmem_log(p_hdl->p_eng->reg_io_base, ipe_eng_get_reg_base_buf_size());
#endif
#endif
		}
	}
}

INT32 kdrv_ipe_dbg_mode(void *p_hdl, UINT32 param_id, void *data)
{
	INT32 rt=0;

	switch(param_id){

	case KDRV_IPE_DBG_SET_SSDRV_DBG_LV:
	{
#if IPE_SSDRV_SUPPORT
		UINT32 lv = *(UINT32 *)data;
		ipe_eng_set_dbg_level(lv);
#endif
	}
	break;

	default:
		DBG_ERR("Unknown dbg mode id\n");
	break;

	}

	return rt;
}
