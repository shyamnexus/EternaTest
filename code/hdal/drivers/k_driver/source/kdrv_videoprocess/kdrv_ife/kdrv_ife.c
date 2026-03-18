#include "kwrap/util.h"
#include "comm/hwclock.h"
#include "kdrv_ife_int.h"
#include "kdrv_ipp_builtin.h"
// For Damnit debug
#include "kdrv_ipp_dbg_log.h"

#ifdef CONFIG_PM
#if defined(__LINUX)
#include <linux/soc/nvt/nvt-info.h>
#include <linux/of.h>
#include <linux/clk.h>
#else
#include <comm/hwclock.h>
#include <pll.h>
#endif

#endif

#define KDRV_IFE_ALLOC_ENG_REG_BUFFER (1)

extern KDRV_IFE_HANDLE* (*kdrv_ife_int_get_handle)(UINT32 chip_id, UINT32 eng_id);

KDRV_IFE_CTL g_kdrv_ife_ctl;

#define KDRV_IFE_ENG_BASE KDRV_VIDEOPROCS_IFE_ENGINE0
#define KDRV_IFE_FMT_PIX(pxlfmt) (KDRV_IPP_FMT_PIX(pxlfmt) & (~KDRV_IPP_FMT_PACK_LSB))

#if defined(__FREERTOS)
#define scnprintf snprintf
#endif

#if 0
#endif



static ULONG kdrv_ife_int_get_eng_reg_buf_addr(KDRV_IFE_HANDLE *p_hdl)
{
	return (ULONG)p_hdl->p_eng_reg_buf;
}

static ULONG kdrv_ife_int_get_eng_flg_buf_addr(KDRV_IFE_HANDLE *p_hdl)
{
	return (ULONG)p_hdl->p_eng_flg_buf;
}

#if IFE_538_OLD_HANDLER_SUPPORT
UINT32 kdrv_ife_buf_overflow_handler(KDRV_IFE_HANDLE *p_hdl){

	UINT32 rt= 0;
	UINT32 *overflow_status_reg =(UINT32 *) (p_hdl->p_eng->reg_io_base + 0x68);

	p_hdl->p_ife_dbg_info->ife_overflow_cnt++;
	p_hdl->p_ife_dbg_info->ife_buf_overflow_status= *overflow_status_reg;

	DBG_ERR("IFE ovflow status=0x%x\n", p_hdl->p_ife_dbg_info->ife_buf_overflow_status  );

	return rt;
}

UINT32 kdrv_ife_frm_end_handler(KDRV_IFE_HANDLE *p_hdl){

	UINT32 rt= 0;
	UINT32 *overflow_status_reg =(UINT32 *) (p_hdl->p_eng->reg_io_base + 0x68);


	if(p_hdl->ife_dbg_ctl.ife_dbg_fun & KDRV_IFE_DBG_STATUS_REC){

		p_hdl->p_ife_dbg_info->ife_buf_current_status = *overflow_status_reg;
		p_hdl->p_ife_dbg_info->ife_buf_rec_status[p_hdl->p_ife_dbg_info->ife_record_idx] = *overflow_status_reg;
		p_hdl->p_ife_dbg_info->ife_record_idx++;

		if(p_hdl->p_ife_dbg_info->ife_record_idx == p_hdl->p_ife_dbg_info->ife_record_num)
			p_hdl->p_ife_dbg_info->ife_record_idx = 0;
	}else{

		p_hdl->p_ife_dbg_info->ife_buf_current_status = *overflow_status_reg;
	}


	return rt;
}
#endif

UINT32 kdrv_ife_isr_state_machine(KDRV_IFE_HANDLE *p_hdl, UINT32 status)
{
	/* return interrupt status that should be process first base on cur state */

	switch (p_hdl->isr_state) {
	case KDRV_IFE_ISR_STATE_UNKNOWN:
		if (status & KDRV_IFE_INTERRUPT_IFE_FRAME_START) {
			p_hdl->isr_state = KDRV_IFE_ISR_STATE_START;
			return KDRV_IFE_INTERRUPT_IFE_FRAME_START;
		}
		break;

	case KDRV_IFE_ISR_STATE_START:
		if (status & KDRV_IFE_INTERRUPT_FMD) {
			p_hdl->isr_state = KDRV_IFE_ISR_STATE_END;
			return KDRV_IFE_INTERRUPT_FMD;
		}

		if (status & KDRV_IFE_INTERRUPT_LLEND) {
			p_hdl->isr_state = KDRV_IFE_ISR_STATE_END;
			return KDRV_IFE_INTERRUPT_LLEND;
		}
		break;
	case KDRV_IFE_ISR_STATE_END:
		if (status & KDRV_IFE_INTERRUPT_IFE_FRAME_START) {
			p_hdl->isr_state = KDRV_IFE_ISR_STATE_START;
			return KDRV_IFE_INTERRUPT_IFE_FRAME_START;
		}
		break;
	default:
		DBG_ERR("unknown state %d\r\n", (int)p_hdl->isr_state);
		break;
	}

	return 0;
}

void kdrv_ife_isr(void *eng, UINT32 status, void *reserve)
{
	IFE_ENG_HANDLE *p_eng;
	KDRV_IFE_HANDLE *p_hdl;
	//UINT32 proc_sts;

	if (eng == NULL) {
		return ;
	}

	p_eng = (IFE_ENG_HANDLE *)eng;

	p_hdl = kdrv_ife_int_get_handle(p_eng->chip_id, p_eng->eng_id);

	if (p_hdl == NULL) {
		return ;
	}

	if (p_hdl->cb == NULL) {
		return ;
	}

	if(status & KDRV_IFE_INTERRUPT_BUFOVFL ){

		KDRV_IFE_ISR_CB_DATA data = {0};
#if IFE_538_OLD_HANDLER_SUPPORT
		kdrv_ife_buf_overflow_handler(p_hdl);
#endif
		data.ife_clk = p_hdl->p_eng->clock_rate;

		p_hdl->cb(p_hdl->chip_id, p_hdl->eng_id, status, (void *)&data, NULL);
		status &= (~KDRV_IFE_INTERRUPT_BUFOVFL); // prevent cb buf overflow to kflow below this function, which bring NULL input data

		p_hdl->p_ife_dbg_info->ife_bufovfl_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_FMD ){
		p_hdl->p_ife_dbg_info->ife_frm_ed_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_DEC_ERR1 ){
		p_hdl->p_ife_dbg_info->ife_r_dec_err1_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_DEC_ERR2 ){
		p_hdl->p_ife_dbg_info->ife_r_dec_err2_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_DEC_ERR3 ){
		p_hdl->p_ife_dbg_info->ife_r_dec_err3_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_LLEND ){
		p_hdl->p_ife_dbg_info->ife_ll_ed_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_LLERR ){
		p_hdl->p_ife_dbg_info->ife_ll_error_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_LLERR2 ){

		// (690) d2d link-list mode patch. ll_err2 occur when sie frame start comes during ll cmd transfer to hw reg. do sw reset & re-trigger to recover eng process
#if IFE_SSDRV_SUPPORT
		ife_eng_int_soft_reset_hw_reg(p_hdl->p_eng);
		ife_eng_ll_fire_hw_reg(p_hdl->p_eng);
#endif

		p_hdl->p_ife_dbg_info->ife_ll_error2_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_RING_BUF_ERR1 ){
		p_hdl->p_ife_dbg_info->ife_ring_buf_err1_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_RING_BUF_ERR2 ){
		p_hdl->p_ife_dbg_info->ife_ring_buf_err2_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_FRAME_ERR ){
		p_hdl->p_ife_dbg_info->ife_frame_err_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_IFE_FRAME_START ){
		p_hdl->p_ife_dbg_info->ife_frm_st_cnt++;
	}

	if(status & KDRV_IFE_INTERRUPT_IFE_P0_SET_SLICE_RDY ){
		//DBG_DUMP("-P0_SET_SLICE_RDY-\n");
		p_hdl->p_ife_dbg_info->ife_p0_rdy++;
	}

	if(status & KDRV_IFE_INTERRUPT_IFE_P0_GET_SLICE_CLR ){
		//DBG_DUMP("-P0_GET_SLICE_CLR-\n");
		p_hdl->p_ife_dbg_info->ife_p0_clr++;
	}

	if(status & KDRV_IFE_INTERRUPT_IFE_P1_SET_SLICE_RDY ){
		//DBG_DUMP("-P1_SET_SLICE_RDY-\n");
		p_hdl->p_ife_dbg_info->ife_p1_rdy++;
	}

	if(status & KDRV_IFE_INTERRUPT_IFE_P1_GET_SLICE_CLR ){
		//DBG_DUMP("-P1_GET_SLICE_CLR-\n");
		p_hdl->p_ife_dbg_info->ife_p1_clr++;
	}


	if(p_hdl->ife_dbg_ctl.ife_dbg_fun & KDRV_IFE_DBG_STATUS_REC){  //ife dbg mode : force frm end intr & LL end intr enable

#if IFE_538_OLD_HANDLER_SUPPORT
		if( (status & KDRV_IFE_INTERRUPT_LLEND) || (status & KDRV_IFE_INTERRUPT_FMD) )
			kdrv_ife_frm_end_handler(p_hdl);
#endif

		if(p_hdl->ife_dbg_ctl.ife_bypass_isr_cb == 0){

			#if 0
			while (status != 0) {
				proc_sts = kdrv_ife_isr_state_machine(p_hdl, status);
				if (proc_sts) {
					status &= ~(proc_sts);
					p_hdl->cb(p_hdl->chip_id, p_hdl->eng_id, proc_sts, NULL, NULL);
				} else {

					break;
				}
			}
			#else
			if(p_hdl->cb)
				p_hdl->cb(p_hdl->chip_id, p_hdl->eng_id, status, NULL, NULL);
			#endif

		}

	}else{ // ife normal mode
		#if 0
		while (status != 0) {
			proc_sts = kdrv_ife_isr_state_machine(p_hdl, status);
			if (proc_sts) {
				status &= ~(proc_sts);
				p_hdl->cb(p_hdl->chip_id, p_hdl->eng_id, proc_sts, NULL, NULL);
			} else {

				break;
			}
		}
		#else
		if(p_hdl->cb)
			p_hdl->cb(p_hdl->chip_id, p_hdl->eng_id, status, NULL, NULL);
		#endif
	}
}

INT32 kdrv_ife_sys_init(UINT32 chip_num, UINT32 eng_num)
{
	KDRV_IFE_HANDLE *p_hdl;
	UINT32 i;
	UINT32 want_size = 0;
	UINT32 total_ch;

	if (g_kdrv_ife_ctl.p_hdl != NULL) {
		DBG_ERR("reinit\r\n");
		return -1;
	}

	total_ch = chip_num * eng_num;
	g_kdrv_ife_ctl.p_hdl = kdrv_ife_os_malloc_wrap(sizeof(KDRV_IFE_HANDLE) * total_ch);
	if (g_kdrv_ife_ctl.p_hdl == NULL) {
		DBG_ERR("alloc buf(%ld) failed\r\n", (ULONG)(sizeof(KDRV_IFE_HANDLE) * total_ch));
		return -1;
	}

	/* init kdrv ctl */
	g_kdrv_ife_ctl.chip_num = chip_num;
	g_kdrv_ife_ctl.eng_num = eng_num;
	g_kdrv_ife_ctl.total_ch = total_ch;

	/* init kdrv handle */
	for (i = 0; i < g_kdrv_ife_ctl.total_ch; i++) {
		// coverity[overwrite_var]
		p_hdl = &g_kdrv_ife_ctl.p_hdl[i];
		memset((void *)p_hdl, 0, sizeof(KDRV_IFE_HANDLE));
		p_hdl->chip_id = KDRV_CHIP0;
		p_hdl->eng_id = KDRV_VIDEOPROCS_IFE_ENGINE0 + i;
		vk_spin_lock_init(&p_hdl->lock);
		SEM_CREATE(p_hdl->sem, 1);


		/* init debug information structure*/
		p_hdl->p_ife_dbg_info = kdrv_ife_os_malloc_wrap(sizeof(KDRV_IFE_DBG_INFO));
		if (p_hdl->p_ife_dbg_info == NULL) {
			DBG_ERR("alloc p_ife_dbg_info failed\r\n");
			kdrv_ife_sys_uninit();
			return -1;
		}
		memset((void *)p_hdl->p_ife_dbg_info, 0, sizeof(KDRV_IFE_DBG_INFO));


		//suspend buff
		p_hdl->reg_data = kdrv_ife_os_malloc_wrap(ife_eng_get_reg_base_buf_size());
		if (p_hdl->reg_data == NULL) {
			DBG_ERR("kmalloc reg_data buf failed\r\n");
			kdrv_ife_sys_uninit();
			// coverity[leaked_storage]
			return -1;
		}
		/* init working buffer for ife_eng */
		#if IFE_SSDRV_SUPPORT
		want_size = ife_eng_get_reg_flag_buf_size();
		#endif
		p_hdl->p_eng_flg_buf = kdrv_ife_os_malloc_wrap(want_size);
		if (p_hdl->p_eng_flg_buf == NULL) {
			#if IFE_SSDRV_SUPPORT
			DBG_ERR("alloc eng flg working buf %d failed\r\n", ife_eng_get_reg_flag_buf_size());
			#endif
			kdrv_ife_sys_uninit();
			// coverity[leaked_storage]
			return -1;
		}

		#if KDRV_IFE_ALLOC_ENG_REG_BUFFER
		#if IFE_SSDRV_SUPPORT
		want_size = ife_eng_get_reg_base_buf_size();
		#endif
		p_hdl->p_eng_reg_buf = kdrv_ife_os_malloc_wrap(want_size);
		if (p_hdl->p_eng_reg_buf == NULL) {
			#if IFE_SSDRV_SUPPORT
			DBG_ERR("alloc eng reg working buf %d failed\r\n", ife_eng_get_reg_base_buf_size());
			#endif
			kdrv_ife_sys_uninit();
			// coverity[leaked_storage]
			return -1;
		}
		#else
		g_kdrv_ife_ctl.p_eng_reg_buf = NULL;
		#endif

	}

	// coverity[leaked_storage]
	return 0;
}

INT32 kdrv_ife_sys_uninit(void)
{
	KDRV_IFE_HANDLE *p_hdl;
	UINT32 i;

	if (g_kdrv_ife_ctl.p_hdl) {
		for (i = 0; i < g_kdrv_ife_ctl.total_ch; i++) {
			p_hdl = &g_kdrv_ife_ctl.p_hdl[i];
			SEM_DESTROY(p_hdl->sem);

			if(p_hdl->p_ife_dbg_info){
				kdrv_ife_os_mfree_wrap(p_hdl->p_ife_dbg_info);
				p_hdl->p_ife_dbg_info = NULL;
			}

			//suspend buff
			if(p_hdl->reg_data) {
				kdrv_ife_os_mfree_wrap(p_hdl->reg_data);
				p_hdl->reg_data = NULL;
			}

			if (p_hdl->p_eng_reg_buf) {
				kdrv_ife_os_mfree_wrap(p_hdl->p_eng_reg_buf);
				p_hdl->p_eng_reg_buf = NULL;
			}

			if (p_hdl->p_eng_flg_buf) {
				kdrv_ife_os_mfree_wrap(p_hdl->p_eng_flg_buf);
				p_hdl->p_eng_flg_buf = NULL;
			}

		}
		kdrv_ife_os_mfree_wrap(g_kdrv_ife_ctl.p_hdl);
		g_kdrv_ife_ctl.p_hdl = NULL;
	}


	memset((void *)&g_kdrv_ife_ctl, 0, sizeof(KDRV_IFE_CTL));

	return 0;
}

#if 0
#endif

INT32 kdrv_ife_open(UINT32 chip, UINT32 engine)
{
	INT32 rt;
	KDRV_IFE_HANDLE *p_hdl;

	p_hdl = kdrv_ife_int_get_handle(chip, engine);
	if (p_hdl == NULL) {
		DBG_ERR("open failed, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_PAR;
	}

	if (0 /*SEM_WAIT_TIMEOUT(p_hdl->sem, vos_util_msec_to_tick(KDRV_IFE_TIMEOUT_MS))*/) {
		DBG_ERR("chip 0x%.8x, engine 0x%.8x, open timeout(%d ms)\r\n", chip, engine, KDRV_IFE_TIMEOUT_MS);
		rt = E_SYS;
	} else {
		#if IFE_SSDRV_SUPPORT
		p_hdl->p_eng = ife_eng_get_handle(chip, engine);

		if (ife_eng_open(p_hdl->p_eng) != E_OK) {
			DBG_ERR("ife eng open fail\r\n");
			rt = E_SYS;
		} else {
			p_hdl->isr_state = KDRV_IFE_ISR_STATE_UNKNOWN;
			ife_eng_reg_isr_callback(p_hdl->p_eng, kdrv_ife_isr);
			if (p_hdl->p_eng == NULL) {
				rt = E_SYS;
				SEM_SIGNAL(p_hdl->sem);
			} else {
				rt = E_OK;
			}
		}
		#endif
	}

	return rt;
}

INT32 kdrv_ife_close(UINT32 chip, UINT32 engine)
{
	KDRV_IFE_HANDLE *p_hdl;

	p_hdl = kdrv_ife_int_get_handle(chip, engine);
	if (p_hdl == NULL) {
		DBG_ERR("close failed, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_PAR;
	}

	if (p_hdl->p_eng == NULL) {
		DBG_ERR("must open before close\r\n");
		return E_SYS;
	}
#if IFE_SSDRV_SUPPORT
	if (ife_eng_close(p_hdl->p_eng) != E_OK) {
		DBG_ERR("ife eng close fail\r\n");
	}
#endif
	p_hdl->p_eng = NULL;
	SEM_SIGNAL(p_hdl->sem);

	return 0;
}

#ifdef CONFIG_PM
#if defined(__LINUX)
INT32 kdrv_ife_suspend(UINT32 chip, UINT32 engine)
{
	KDRV_IFE_HANDLE *p_hdl;
	UINT32 i;
	UINT32 reg_ofs;
	uintptr_t ife_reg_io_base = 0;

	if (!g_kdrv_ife_ctl.total_ch)
		return E_OK;

	p_hdl = kdrv_ife_int_get_handle(chip, engine);

#if 0
	if (g_kdrv_ife_ctl.p_hdl) {
		for (i = 0; i < g_kdrv_ife_ctl.total_ch; i++) {
			p_hdl = &g_kdrv_ife_ctl.p_hdl[i];
		}
	}
#endif

	if (p_hdl == NULL) {
		DBG_IND("ife driver is not init, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_OK;
	}

	if (p_hdl->p_eng == NULL) {
		DBG_IND("ife is init but not open\r\n");
		return E_OK;
	}

	if (p_hdl->reg_data == NULL) {
		DBG_ERR("HDL tempory register buffer is not init\r\n");
		return E_SYS;
	}


	ife_reg_io_base = p_hdl->p_eng->reg_io_base;
	DBG_IND("ife_reg_io_base_pa %lx for suspend\r\n", vos_cpu_get_phy_addr(ife_reg_io_base));

	if (!ife_reg_io_base) {
		DBG_ERR("base addr get fail!\r\n");
		return E_SYS;
	}

	/* flush output buffer */
	vos_cpu_dcache_sync(ife_reg_io_base, ife_eng_get_reg_base_buf_size(), VOS_DMA_FROM_DEVICE);

	for (i = 0; i < IFE_ENG_REG_NUM; i++) {
		reg_ofs = i << 2;

		p_hdl->reg_data[i] = kdrv_ife_eng_getreg(ife_reg_io_base + reg_ofs);
	}

	if (!IS_ERR(p_hdl->p_eng->pclk)) {
		//ife_eng_release();
		ife_eng_platform_disable_clk(p_hdl->p_eng);
		ife_eng_platform_unprepare_clk(p_hdl->p_eng);
	}

	if (clk_get_phase(p_hdl->p_eng->pclk)) {
		p_hdl->auto_gating = 1;
		clk_set_phase(p_hdl->p_eng->pclk, 0);
	} else
		p_hdl->auto_gating = 0;

#if 0
	#if defined(__FREERTOS)
		debug_dumpmem(p_hdl->p_eng->reg_io_base, ife_eng_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
	#else
		debug_dumpmem(vos_cpu_get_phy_addr(p_hdl->p_eng->reg_io_base), ife_eng_get_reg_base_buf_size());
	#endif
#endif
	return 0;
}

INT32 kdrv_ife_resume(UINT32 chip, UINT32 engine)
{
	KDRV_IFE_HANDLE *p_hdl;
	UINT32 i;
	UINT32 reg_ofs;
	uintptr_t ife_reg_io_base = 0;

	if (!g_kdrv_ife_ctl.total_ch)
		return E_OK;

	p_hdl = kdrv_ife_int_get_handle(chip, engine);

#if 0
	if (g_kdrv_ife_ctl.p_hdl) {
		for (i = 0; i < g_kdrv_ife_ctl.total_ch; i++) {
			p_hdl = &g_kdrv_ife_ctl.p_hdl[i];
		}
	}
#endif

	if (p_hdl == NULL) {
		DBG_IND("ife driver is not init, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_OK;
	}

	if (p_hdl->p_eng == NULL) {
		DBG_IND("ife is init but not open\r\n");
		return E_OK;
	}

	if (p_hdl->reg_data == NULL) {
		DBG_ERR("HDL tempory register buffer is not init\r\n");
		return E_SYS;
	}

	//set clock
	if (!IS_ERR(p_hdl->p_eng->pclk)) {
		ife_eng_platform_set_clk_rate(p_hdl->p_eng); //including set parent
		ife_eng_platform_prepare_clk(p_hdl->p_eng);
		ife_eng_platform_enable_clk(p_hdl->p_eng);
	}
	if (p_hdl->auto_gating) {
		clk_set_phase(p_hdl->p_eng->pclk, 1);
	}


	ife_reg_io_base = p_hdl->p_eng->reg_io_base;

	DBG_IND("ife_reg_io_base_pa %lx for resume\r\n", vos_cpu_get_phy_addr(ife_reg_io_base));

	if (!ife_reg_io_base) {
		DBG_ERR("base addr get fail!\r\n");
		return E_SYS;
	}

	for (i = 0; i < IFE_ENG_REG_NUM; i++) {
		reg_ofs = i << 2;

		kdrv_ife_eng_setreg(ife_reg_io_base + reg_ofs, p_hdl->reg_data[i]);
	}

	/* flush input buffer */
	vos_cpu_dcache_sync(ife_reg_io_base, ife_eng_get_reg_base_buf_size(), VOS_DMA_TO_DEVICE);

#if 0
	#if defined(__FREERTOS)
			debug_dumpmem(p_hdl->p_eng->reg_io_base, ife_eng_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
	#else
			debug_dumpmem(vos_cpu_get_phy_addr(p_hdl->p_eng->reg_io_base), ife_eng_get_reg_base_buf_size());
	#endif
#endif
	return 0;
}
#endif
#endif


#if 0
#endif

static IFE_OPMODE kdrv_ife_typecast_opmode(KDRV_IPP_OPMODE mode)
{
	switch (mode) {
	case KDRV_IPP_OPMODE_D2D:
		return IFE_OPMODE_D2D;

	case KDRV_IPP_OPMODE_IFE2IME:
	case KDRV_IPP_OPMODE_DCE2IME:
	case KDRV_IPP_OPMODE_IFE2IPE:
		return IFE_OPMODE_IPP;

	case KDRV_IPP_OPMODE_SIE2IME:
		return IFE_OPMODE_ALL_DIRECT;

	default:
		DBG_ERR("unknown mode %d \r\n", mode);
		return -1;
	}
}

static IFE_BAYERFMTSEL kdrv_ife_typecast_bayer_fmt(KDRV_IPP_FMT fmt)
{
	UINT32 stpx = KDRV_IFE_FMT_PIX(fmt);

	switch (stpx) {
	case KDRV_IPP_FMT_RGGB_R:
	case KDRV_IPP_FMT_RGGB_GR:
	case KDRV_IPP_FMT_RGGB_GB:
	case KDRV_IPP_FMT_RGGB_B:
		return IFE_BAYER_RGGB;

	case KDRV_IPP_FMT_RGBIR44_RGBG_GIGI:
	case KDRV_IPP_FMT_RGBIR44_GBGR_IGIG:
	case KDRV_IPP_FMT_RGBIR44_GIGI_BGRG:
	case KDRV_IPP_FMT_RGBIR44_IGIG_GRGB:
	case KDRV_IPP_FMT_RGBIR44_BGRG_GIGI:
	case KDRV_IPP_FMT_RGBIR44_GRGB_IGIG:
	case KDRV_IPP_FMT_RGBIR44_GIGI_RGBG:
	case KDRV_IPP_FMT_RGBIR44_IGIG_GBGR:
		return IFE_BAYER_RGBIR;

	default:
		DBG_ERR("Unknown pix 0x%.8x\r\n", (unsigned int)stpx);
		return -1;
	}
}
#if IFE_SSDRV_SUPPORT
static IFE_YUV_IN_FMT kdrv_ife_typecast_yuv_fmt(KDRV_IPP_FMT fmt)
{
	switch (fmt) {
	case KDRV_IPP_FMT_YUV420:
		return IFE_ENG_FMT_Y_PACK_UV420;
	case KDRV_IPP_FMT_YUV422:
		return IFE_ENG_FMT_Y_PACK_UV422;
	case KDRV_IPP_FMT_Y8:
		return IFE_ENG_FMT_Y_8BIT;
	default:
		DBG_ERR("Unknown fmt 0x%x\r\n", (UINT32)fmt);
		return IFE_ENG_FMT_Y_UNKNOWN;
	}
}

static IFE_YUV_IN_FMT kdrv_ife_typecast_yuv_out_fmt(KDRV_IPP_FMT fmt)
{
	switch (fmt) {
	case KDRV_IPP_FMT_YUV420:
		return IFE_OUT_YUV420;
	case KDRV_IPP_FMT_YUV422:
		return IFE_OUT_YUV422;
	default:
		DBG_ERR("Unknown out fmt 0x%x\r\n", (UINT32)fmt);
		return -1;
	}
}

static IFE_ENG_YUV2RGB_FMT kdrv_ife_typecast_yuv2rgb(KDRV_IPP_PXLFMT pxl_fmt)
{
	switch ((pxl_fmt & KDRV_IPP_FMT_YCC_MASK)) {
	case KDRV_IPP_FMT_YCC_FULL:
		return IFE_ENG_FULL_YUV;

	case KDRV_IPP_FMT_YCC_BT601:
		return IFE_ENG_BT601_YUV;

	case KDRV_IPP_FMT_YCC_BT709:
		return IFE_ENG_BT709_YUV;

	default:
		DBG_ERR("unknown pxl fotmat 0x%.8x\r\n", (unsigned int)pxl_fmt);
		return IFE_ENG_UNKNOWN_YUV;
	}
}

static IFE_RDE_ADJMODE_SEL kdrv_ife_typecast_adj_mode(KDRV_IFE_ENCODE_ADJMODE adj_mode)
{
	switch (adj_mode) {
	case KDRV_IFE_ENCODE_ADJMODE_NORMAL:
		return RDE_PARMS_NORMAL_MODE;

	case KDRV_IFE_ENCODE_ADJMODE_AGGRESIVE:
		return RDE_PARMS_AGGRESIVE_MODE;

	default:
		DBG_ERR("Unknown adj mode %d\r\n", adj_mode);
		return RDE_PARMS_NORMAL_MODE;
	}
}

static IFE_CFASEL kdrv_ife_typecast_pix(KDRV_IPP_FMT fmt)
{
	UINT32 stpx = KDRV_IFE_FMT_PIX(fmt);

	switch (stpx) {
	case KDRV_IPP_FMT_RGGB_R:
		return IFE_PAT0;

	case KDRV_IPP_FMT_RGGB_GR:
		return IFE_PAT1;

	case KDRV_IPP_FMT_RGGB_GB:
		return IFE_PAT2;

	case KDRV_IPP_FMT_RGGB_B:
		return IFE_PAT3;

	case KDRV_IPP_FMT_RGBIR44_RGBG_GIGI:
		return IFE_PAT0;

	case KDRV_IPP_FMT_RGBIR44_GBGR_IGIG:
		return IFE_PAT1;

	case KDRV_IPP_FMT_RGBIR44_GIGI_BGRG:
		return IFE_PAT2;

	case KDRV_IPP_FMT_RGBIR44_IGIG_GRGB:
		return IFE_PAT3;

	case KDRV_IPP_FMT_RGBIR44_BGRG_GIGI:
		return IFE_PAT4;

	case KDRV_IPP_FMT_RGBIR44_GRGB_IGIG:
		return IFE_PAT5;

	case KDRV_IPP_FMT_RGBIR44_GIGI_RGBG:
		return IFE_PAT6;

	case KDRV_IPP_FMT_RGBIR44_IGIG_GBGR:
		return IFE_PAT7;

	default:
		DBG_ERR("Unknown pix 0x%.8x\r\n", (unsigned int)stpx);
		return -1;
	}
}

static IFE_BITDEPTH kdrv_ife_typecast_rawbit(KDRV_IPP_FMT fmt)
{
	UINT32 bit = KDRV_IPP_FMT_BPP(fmt);

	switch (bit) {
	case 8:
		return IFE_8BIT;

	case 10:
		return IFE_10BIT;

	case 12:
		return IFE_12BIT;

	case 16:
		return IFE_16BIT;

	default:
		DBG_ERR("unsupport rawbit %d\r\n", (int)(bit));
		return -1;
	}
}
#endif
static INT8 kdrv_ife_typecast_fnum(KDRV_IPP_FMT fmt)
{
	UINT32 fnum = KDRV_IPP_FMT_PLANE(fmt);

	switch (fnum) {
	case 1:
		return 0;
	case 2:
		return 1;
	case 3:
		return 2;
	default:
		DBG_ERR("Unknown fnum %u\r\n", fnum);
		return -1;
	}
}
#if IFE_SSDRV_SUPPORT
static IFE_CGRANGE kdrv_ife_typecast_cgrange(KDRV_IFE_IQ_GAIN_FIELD field)
{
	switch (field) {
	case KDRV_IFE_IQ__2_8:
		return IFE_CGRANGE_2_8;

	case KDRV_IFE_IQ__3_7:
		return IFE_CGRANGE_3_7;

	default:
		DBG_ERR("Unknown field %u\r\n", field);
		return -1;
	}
}

static UINT32 kdrv_ife_cal_vig_square(UINT32 x)
{
	UINT64 min = 0, max = 0, mid, i = 0;

	max = x << 8;
	mid = (max + min) >> 1;

	for (i = 0; i < 20; i++) {
		if (((mid >> 8) * mid) < (x << 8)) {
			min = mid;
		} else if (((mid >> 8) * mid) > (x << 8)) {
			max = mid;
		} else {
			return ((UINT32)mid);
		}
		mid = (max + min) >> 1;
	}
	return ((UINT32)mid);
}

static int kdrv_ife_cal_vig_setting(IFE_VIG_PARAM *p_vig_param, UINT32 in_w, UINT32 in_h)
{
	UINT32 ui_abs_dif_max_x, ui_abs_dif_max_y;
	UINT32 ui_ratio_x, ui_ratio_y;
	UINT32 ui_n, ui_m, ui_k, ui_l, ui_j, ui_i, ui_h, ui_g;
	UINT32 rate_x, rate_y, center_x, center_y;

	UINT64 numerator, denominator;

	center_x = p_vig_param->p_vig_x[0];
	center_y = p_vig_param->p_vig_y[0];


	rate_x = 1000;
	rate_y = rate_x * in_h / in_w;

	// step-1, LUT coverage
	ui_n = 16 << 6; // cover 0~16 LUT

	// step-2, before sqrt
	ui_m = ui_n * ui_n;

	// step-3, before addition
	if (center_x >= (in_w / 2)) {
		ui_abs_dif_max_x = center_x;
	} else {
		ui_abs_dif_max_x = in_w - center_x;
	}
	if (ui_abs_dif_max_x > 0x1fff) {
		//DBG_WRN("x dist >0x1fff\r\n");
		ui_abs_dif_max_x = 0x1fff;
	}

	ui_ratio_x = ui_abs_dif_max_x * rate_x / in_w;
	ui_ratio_x = ui_ratio_x * ui_ratio_x;

	if (center_y >= (in_h / 2)) {
		ui_abs_dif_max_y = center_y;
	} else {
		ui_abs_dif_max_y = in_h - center_y;
	}
	if (ui_abs_dif_max_y > 0x1fff) {
		//DBG_WRN("y dist >0x1fff\r\n");
		ui_abs_dif_max_y = 0x1fff;
	}
	ui_ratio_y = ui_abs_dif_max_y * rate_y / in_h;
	ui_ratio_y = ui_ratio_y * ui_ratio_y;

	numerator = (UINT64)ui_m;
	numerator *= ((UINT64)(ui_ratio_x));
	denominator = (UINT64)(ui_ratio_x + ui_ratio_y);

	ui_k = kdrv_ife_do_div(numerator, denominator);
	//ui_k = (UINT32)numerator;

	numerator = ui_m;
	numerator *= ui_ratio_y;
	denominator = (UINT64)(ui_ratio_x + ui_ratio_y);

	ui_l = kdrv_ife_do_div(numerator, denominator);
	//ui_l = (UINT32)(numerator);

	// step-4, before 2th-power
	ui_i = (UINT32)(kdrv_ife_cal_vig_square(ui_k));
	ui_j = (UINT32)(kdrv_ife_cal_vig_square(ui_l));

	// step-5 DIV-parameters
	if (ui_abs_dif_max_x == 0 || ui_abs_dif_max_y == 0) {
		DBG_ERR("Vig center or input size error!\r\n");
		return -1;
	}


	ui_g = ((ui_i << 10) / ui_abs_dif_max_x) >> 8;

	if (ui_g > 0xfff)    {
		//DBG_WRN("XDIV %x clamped to 0xfff\r\n", (unsigned int)ui_g);
		ui_g = 0xfff;
	} /*else if (ui_g < 100) {
        //DBG_WRN("XDIV accuracy is 1/%d\r\n", (unsigned int)ui_g);
    }*/

	ui_h = ((ui_j << 10) / ui_abs_dif_max_y) >> 8;

	if (ui_h > 0xfff)    {
		//DBG_WRN("YDIV %x clamped to 0xfff\r\n", (unsigned int)ui_h);
		ui_h = 0xfff;
	} /*else if (ui_h < 100) {
        //DBG_WRN("YDIV accuracy is 1/%d\r\n", (unsigned int)ui_h);
    }*/

	// step-6 output
	p_vig_param->vig_xdiv = ui_g;
	p_vig_param->vig_ydiv = ui_h;

	return 0;
}

static void kdrv_ife_cal_vig_tab_gain(IFE_VIG_PARAM *p_vig_param, KDRV_IFE_IQ_VIG *vig_para, IFE_BAYERFMTSEL fmt)
{
	UINT32 real_gain, max_gain = 0, i;

	for (i = 0; i < KDRV_IFE_IQ_VIG_CH0_LUT_SIZE; i++) {
		if (max_gain < vig_para->ch_r_lut[i]) {
			max_gain = vig_para->ch_r_lut[i];
		}
		if (max_gain < vig_para->ch_gr_lut[i]) {
			max_gain = vig_para->ch_gr_lut[i];
		}
		if (fmt == IFE_BAYER_RGBIR) {
			if (max_gain < vig_para->ch_ir_lut[i]) {
				max_gain = vig_para->ch_ir_lut[i];
			}
		} else {
			if (max_gain < vig_para->ch_gb_lut[i]) {
				max_gain = vig_para->ch_gb_lut[i];
			}
		}
		if (max_gain < vig_para->ch_b_lut[i]) {
			max_gain = vig_para->ch_b_lut[i];
		}
	}

	real_gain = 1 + (max_gain >> 10);
	if (real_gain < 2) {
		p_vig_param->vig_tab_gain = 0;
	} else if (real_gain < 3) {
		p_vig_param->vig_tab_gain = 1;
	} else if (real_gain < 5) {
		p_vig_param->vig_tab_gain = 2;
	} else if (real_gain < 9) {
		p_vig_param->vig_tab_gain = 3;
	} else {
		DBG_ERR("Wrong gain value !!");
	}

	for (i = 0; i < KDRV_IFE_IQ_VIG_CH0_LUT_SIZE; i++) {
		vig_para->ch_r_lut[i] = vig_para->ch_r_lut[i] >> p_vig_param->vig_tab_gain;
		vig_para->ch_gr_lut[i] = vig_para->ch_gr_lut[i] >> p_vig_param->vig_tab_gain;
		vig_para->ch_gb_lut[i] = vig_para->ch_gb_lut[i] >> p_vig_param->vig_tab_gain;
		vig_para->ch_ir_lut[i] = vig_para->ch_ir_lut[i] >> p_vig_param->vig_tab_gain;
		vig_para->ch_b_lut[i] = vig_para->ch_b_lut[i] >> p_vig_param->vig_tab_gain;
	}
	p_vig_param->p_vig_lut_c0 = &vig_para->ch_r_lut[0];
	p_vig_param->p_vig_lut_c1 = &vig_para->ch_gr_lut[0];
	if (fmt == IFE_BAYER_RGBIR) {
		p_vig_param->p_vig_lut_c2 = &vig_para->ch_ir_lut[0];
	} else {
		p_vig_param->p_vig_lut_c2 = &vig_para->ch_gb_lut[0];
	}
	p_vig_param->p_vig_lut_c3 = &vig_para->ch_b_lut[0];
}
#endif

static BOOL kdrv_ife_chk_align(UINT32 val, UINT32 align)
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

static INT32 kdrv_ife_int_get_limitation(KDRV_IPP_FMT fmt, KDRV_IFE_LMT *p_lmt)
{

	switch (fmt) {

		default:

			p_lmt->in_w_min = IFE_SRCBUF_W_MIN;
			p_lmt->in_w_max = IFE_SRCBUF_W_MAX;
			p_lmt->in_w_align = IFE_SRCBUF_W_ALIGN;
			p_lmt->in_h_min = IFE_SRCBUF_H_MIN;
			p_lmt->in_h_max = IFE_SRCBUF_H_MAX;
			p_lmt->in_h_align = IFE_SRCBUF_H_ALIGN;
			p_lmt->in_lofs_align = IFE_SRCBUF_LOFS_ALIGN;
			p_lmt->in_addr_align = IFE_SRCBUF_ADDR_ALIGN;

			p_lmt->out_w_min = IFE_DSTBUF_W_MIN;
			p_lmt->out_w_max = IFE_DSTBUF_W_MAX;
			p_lmt->out_w_align = IFE_DSTBUF_W_ALIGN;
			p_lmt->out_h_min = IFE_DSTBUF_H_MIN;
			p_lmt->out_h_max = IFE_DSTBUF_H_MAX;
			p_lmt->out_h_align = IFE_DSTBUF_H_ALIGN;
			p_lmt->out_lofs_align = IFE_DSTBUF_LOFS_ALIGN;
			p_lmt->out_addr_align = IFE_DSTBUF_ADDR_ALIGN;

		break;
	}

	return E_OK;
}

static INT32 kdrv_ife_int_check_limit(KDRV_IFE_JOB_CFG *p_cfg)
{
	KDRV_IFE_IO_CFG *io_cfg;
	KDRV_IFE_IQ_CFG *iq_cfg;
	IFE_OPMODE op_mode;
	UINT32 i, stpx, bit, fnum;
	KDRV_IFE_LMT lmt = {0};

	io_cfg = p_cfg->p_iocfg;
	iq_cfg = p_cfg->p_iqcfg;

	if (io_cfg->mode >= KDRV_IPP_OPMODE_MAX) {
		DBG_ERR("unsupport mode %d\r\n", (int)io_cfg->mode);
		return E_PAR;
	}

	if (kdrv_ife_int_get_limitation(p_cfg->p_iocfg->in_frm.fmt, &lmt) != E_OK) {
		DBG_ERR("get ife limit fail\n");
		return E_PAR;
	}

	op_mode = kdrv_ife_typecast_opmode(io_cfg->mode);

	if(KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX){
		stpx = KDRV_IFE_FMT_PIX(io_cfg->in_frm.fmt);
			if (stpx != KDRV_IPP_FMT_RGGB_R &&
				stpx != KDRV_IPP_FMT_RGGB_GR &&
				stpx != KDRV_IPP_FMT_RGGB_GB &&
				stpx != KDRV_IPP_FMT_RGGB_B &&
				stpx != KDRV_IPP_FMT_RGBIR44_RGBG_GIGI &&
				stpx != KDRV_IPP_FMT_RGBIR44_GBGR_IGIG &&
				stpx != KDRV_IPP_FMT_RGBIR44_GIGI_BGRG &&
				stpx != KDRV_IPP_FMT_RGBIR44_IGIG_GRGB &&
				stpx != KDRV_IPP_FMT_RGBIR44_BGRG_GIGI &&
				stpx != KDRV_IPP_FMT_RGBIR44_GRGB_IGIG &&
				stpx != KDRV_IPP_FMT_RGBIR44_GIGI_RGBG &&
				stpx != KDRV_IPP_FMT_RGBIR44_IGIG_GBGR) {
				DBG_ERR("unsupport stpx %d (fmt 0x%.8x)\r\n", (int)stpx, (unsigned int)io_cfg->in_frm.fmt);
				return E_PAR;
			}
	}

	if(KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX){

		bit = KDRV_IPP_FMT_BPP(io_cfg->in_frm.fmt);
		if (bit != 8 &&
			bit != 10 &&
			bit != 12 &&
			bit != 16) {
			DBG_ERR("unsupport input bit %d (fmt 0x%.8x)\r\n", (int)bit, (unsigned int)io_cfg->in_frm.fmt);
			return E_PAR;
		}
	}

	if( (op_mode == IFE_OPMODE_D2D) && ( KDRV_IPP_FMT_CLASS(io_cfg->out_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW  || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX)  ){
		bit = KDRV_IPP_FMT_BPP(io_cfg->out_frm.fmt);
		if (bit != 8 &&
			bit != 10 &&
			bit != 12 &&
			bit != 16) {
			DBG_ERR("unsupport output bit %d (fmt 0x%.8x)\r\n", (int)bit, (unsigned int)io_cfg->out_frm.fmt);
			return E_PAR;
		}
	}

	fnum = KDRV_IPP_FMT_PLANE(io_cfg->in_frm.fmt);
	if (fnum != 1 &&
		fnum != 2) {
		DBG_ERR("unsupport fnum %d (fmt 0x%.8x)\r\n", (int)fnum, (unsigned int)io_cfg->in_frm.fmt);
		return E_PAR;
	}

	/*-----------input frm limitation-----------*/
	if (kdrv_ife_chk_align(p_cfg->p_iocfg->in_frm.size.w, lmt.in_w_align) != TRUE) {
		DBG_ERR("in_width %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->in_frm.size.w, (UINT32)lmt.in_w_align);
		return E_PAR;
	}

	if (kdrv_ife_chk_align(p_cfg->p_iocfg->in_frm.size.h, lmt.in_h_align) != TRUE) {
		DBG_ERR("in_height %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->in_frm.size.h, (UINT32)lmt.in_h_align);
		return E_PAR;
	}

	/*-----------crop limitation-----------*/
	if ((io_cfg->crp_window.w & 0x3) != 0) {
		DBG_ERR("Input crop width should be 4x!!\r\n");
		return E_PAR;
	}

	if ((io_cfg->crp_window.h & 0x1) != 0) {
		DBG_ERR("Input crop height should be 2x!!\r\n");
		return E_PAR;
	}

	/*-----------input buf limitation-----------*/
	if (op_mode != IFE_OPMODE_ALL_DIRECT) {
		if (io_cfg->in_frm.addr[0] == 0) {
			DBG_ERR("Input1 Address 0\r\n");
			return E_PAR;
		}
		if (kdrv_ife_chk_align(p_cfg->p_iocfg->in_frm.lofs[0], lmt.in_lofs_align) != TRUE) {
			DBG_ERR("in_lofs %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->in_frm.lofs[0], (UINT32)lmt.in_lofs_align);
			return E_PAR;
		}
		if (kdrv_ife_chk_align(p_cfg->p_iocfg->in_frm.phyaddr[0], lmt.in_addr_align) != TRUE) {
			DBG_ERR("in_addr 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->in_frm.phyaddr[0], (UINT32)lmt.in_addr_align);
			return E_PAR;
		}
	}

	if (kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt) == 1 || kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt) == 2) { // fnum >  0: hdr, fnum == 0: fusion disable
		if (io_cfg->in_frm.addr[1] == 0) {
			DBG_ERR("Input2 Address 0\r\n");
			return E_PAR;
		}
		if (kdrv_ife_chk_align(p_cfg->p_iocfg->in_frm.lofs[1], lmt.in_lofs_align) != TRUE) {
			DBG_ERR("in_lofs2 %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->in_frm.lofs[1], (UINT32)lmt.in_lofs_align);
			return E_PAR;
		}
		if (kdrv_ife_chk_align(p_cfg->p_iocfg->in_frm.phyaddr[1], lmt.in_addr_align) != TRUE) {
			DBG_ERR("in_addr2 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->in_frm.phyaddr[1], (UINT32)lmt.in_addr_align);
			return E_PAR;
		}
	}

	if(kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt) == 2 ){
		if (io_cfg->in_frm.addr[2] == 0) {
			DBG_ERR("Input2 Address 0\r\n");
			return E_PAR;
		}
		if (kdrv_ife_chk_align(p_cfg->p_iocfg->in_frm.lofs[2], lmt.in_lofs_align) != TRUE) {
			DBG_ERR("in_lofs3 %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->in_frm.lofs[2], (UINT32)lmt.in_lofs_align);
			return E_PAR;
		}
		if (kdrv_ife_chk_align(p_cfg->p_iocfg->in_frm.phyaddr[2], lmt.in_addr_align) != TRUE) {
			DBG_ERR("in_addr3 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->in_frm.phyaddr[2], (UINT32)lmt.in_addr_align);
			return E_PAR;
		}
	}

	/*-----------output buf limitation-----------*/
	if (op_mode == IFE_OPMODE_D2D) {
		if (io_cfg->out_frm.addr[0] == 0) {
			DBG_ERR("Output Address 0\r\n");
			return E_PAR;
		}
		if (kdrv_ife_chk_align(p_cfg->p_iocfg->out_frm.lofs[0], lmt.out_lofs_align) != TRUE) {
			DBG_ERR("out_lofs %d not align to %d\r\n", (UINT32)p_cfg->p_iocfg->out_frm.lofs[0], (UINT32)lmt.out_lofs_align);
			return E_PAR;
		}
		if (kdrv_ife_chk_align(p_cfg->p_iocfg->out_frm.phyaddr[0], lmt.out_addr_align) != TRUE) {
			DBG_ERR("out_addr 0x%lx not align to %d\r\n", (ULONG)p_cfg->p_iocfg->out_frm.phyaddr[0], (UINT32)lmt.out_addr_align);
			return E_PAR;
		}

		if (KDRV_IPP_FMT_CLASS(io_cfg->out_frm.fmt) != KDRV_IPP_FMT_CLASS_RAW && KDRV_IPP_FMT_CLASS(io_cfg->out_frm.fmt) != KDRV_IPP_FMT_CLASS_NRX) {
			DBG_ERR("unsupport output fmt 0x%.8x\r\n", (unsigned int)io_cfg->out_frm.fmt);
			return E_PAR;
		}
	}

	if (KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) != KDRV_IPP_FMT_CLASS_RAW && KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) != KDRV_IPP_FMT_CLASS_NRX
																			&& KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) != KDRV_IPP_FMT_CLASS_YUV) {
		DBG_ERR("unsupport input fmt 0x%.8x\r\n", (unsigned int)io_cfg->in_frm.fmt);
		return E_PAR;
	}

	if (kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt)) { // fnum > 0: fusion enable, fnum == 0: fusion disable
		if (iq_cfg->fusion.fu_cgain.enable == FALSE) {
			DBG_WRN("Fusion enable but FCgain not enable!!\r\n");
		}

		if (iq_cfg->fcurve.enable == FALSE) {
			DBG_WRN("Fusion enable but Fcurve not enable!!\r\n");
		}
	} else {
		/*if (iq_cfg->fusion.fu_cgain.en == TRUE) {
			DBG_WRN("Fusion not enable but FCgain enable!!\r\n");
		}*/
		//98530 remove
		/*if (iq_cfg->fusion.s_comp.en == TRUE) {
			DBG_WRN("Fusion not enable but S_compression enable!!\r\n");
		}*/
	}

	if (KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX){

		for (i = 0; i < KDRV_IFE_IQ_RANGE_A_TH_NUM; i++) {
			if (iq_cfg->filt.rng_filt_r.a_th[i] > KDRV_IFE_RNG_TH_MAX) {
				DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
			}
			if (iq_cfg->filt.rng_filt_gr.a_th[i] > KDRV_IFE_RNG_TH_MAX) {
				DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
			}


			if (kdrv_ife_typecast_bayer_fmt(io_cfg->in_frm.fmt) == IFE_BAYER_RGBIR) {
				if (iq_cfg->filt.rng_filt_ir.a_th[i] > KDRV_IFE_RNG_TH_MAX) {
					DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
				}
			} else {
				if (iq_cfg->filt.rng_filt_gb.a_th[i] > KDRV_IFE_RNG_TH_MAX) {
					DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
				}
			}


			if (iq_cfg->filt.rng_filt_b.a_th[i] > KDRV_IFE_RNG_TH_MAX) {
				DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
			}
		}

		for (i = 0; i < KDRV_IFE_IQ_RANGE_A_LUT_SIZE; i++) {
			if (iq_cfg->filt.rng_filt_r.a_lut[i] > KDRV_IFE_RNG_TH_MAX) {
				DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
			}
			if (iq_cfg->filt.rng_filt_gr.a_lut[i] > KDRV_IFE_RNG_TH_MAX) {
				DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
			}
			//if (kdrv_ife_typecast_bayer_fmt(io_cfg->in_frm.fmt) == IFE_BAYER_RGBIR) {
				if (iq_cfg->filt.rng_filt_ir.a_lut[i] > KDRV_IFE_RNG_TH_MAX) {
					DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
				}
			//} else {
				//if (iq_cfg->filt.rng_filt_gb.a_lut[i] > KDRV_IFE_RNG_TH_MAX) {
					//DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
				//}
			//}
			if (iq_cfg->filt.rng_filt_b.a_lut[i] > KDRV_IFE_RNG_TH_MAX) {
				DBG_WRN("Range threshold should <= %u\r\n", KDRV_IFE_RNG_TH_MAX);
			}
		}
	}

	if (iq_cfg->update & KDRV_IFE_UPDATE_CGAIN) {
		if (iq_cfg->cgain.enable) {
			if (iq_cfg->cgain.bit_field != KDRV_IFE_IQ__2_8 &&
				iq_cfg->cgain.bit_field != KDRV_IFE_IQ__3_7) {
				DBG_ERR("unsupport bit_field %d\r\n", (int)iq_cfg->cgain.bit_field);
				return E_PAR;
			}
		}
	}
	/*-----------FPN limation ---------------------*/
	if(iq_cfg->fpn.fpn_en){
		if(kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt) != 0){
			DBG_ERR("if IFE_FPN_EN ==1, IFE_F_FUSION_FNUM available range only 0, IFE_F_FUSION_EN = 0\r\n");
			return E_PAR;
		}
	}

	/*-----------thermal_mode limitation-----------*/
	if (io_cfg->thermal_mode_en) {
		if (op_mode == IFE_OPMODE_ALL_DIRECT ||iq_cfg->fcurve.enable == 0 || kdrv_ife_typecast_rawbit(io_cfg->in_frm.fmt) != IFE_16BIT) {
			DBG_ERR("D2D Input bit depth need 16 bits and IFE_F_FC_EN need enable\r\n");
			return E_PAR;
		}
	}
#if 0
	/*-----------mirror limation ---------------------*/
	if((op_mode == IFE_OPMODE_ALL_DIRECT) && io_cfg->mirror.en == 1){
		DBG_ERR("DIRECT mode not support ipp mirror");
		return E_PAR;
	}
#endif

	return E_OK;
}


#if IFE_SSDRV_SUPPORT
static INT32 kdrv_ife_int_cfg_va(KDRV_IFE_HANDLE *p_hdl, KDRV_IFE_JOB_CFG *p_cfg, IFE_ENG_DMA_OUT_INFO* p_single_out)
{
#if 1
	KDRV_IFE_IQ_CFG* p_iq_cfg;
	//INT32 rt = E_OK;
	UINT32 i;

	p_iq_cfg = p_cfg->p_iqcfg;

	if (p_iq_cfg->va.enable) {

		if(p_cfg->p_iocfg->va_out_addr.pa != 0 && p_cfg->p_iocfg->va_out_addr.pa != (ULONG)-1){
			UINT64 addr = p_cfg->p_iocfg->va_out_addr.pa;

			p_cfg->p_iocfg->va_lofs = p_iq_cfg->va.win_num.w * 16 * (p_iq_cfg->va.va_out_grp1_2 + 1);//538 added VA lofs = win_num.h(8) * 16(4word) * group(2) max:0x80;

			ife_eng_set_va_output_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF), p_cfg->p_iocfg->va_lofs); //p_iq_cfg->va.va_lofs
			//ife_eng_set_va_out_addr_ofs_buf_reg(p_hdl->p_eng, p_cfg->p_iocfg->va_out_addr_ofs);
		} else{
			DBG_ERR("va_out_addr:0x%lx invalid\n", p_cfg->p_iocfg->va_out_addr.pa);
			return E_PAR;
		}
		p_single_out->va_en = ENABLE;
	}

	if (p_iq_cfg->update & KDRV_IFE_UPDATE_VA) {
		{
			if (p_iq_cfg->va.enable) {
				IFE_VA_WIN_PARAM va_win;
				USIZE blk_size = {0};
				USIZE win_size = {0};
				//IFE_IMG_SIZE img_size;

				blk_size.w = /*(p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width)*/p_cfg->p_iocfg->in_frm.size.w / p_iq_cfg->va.win_num.w;
				blk_size.h = p_cfg->p_iocfg->in_frm.size.h / p_iq_cfg->va.win_num.h;
				win_size.w = ALIGN_FLOOR_4(((p_iq_cfg->va_window.winsz_ratio.w * blk_size.w) / p_iq_cfg->va_window.ratio_base));
				if (win_size.w > KDRV_IFE_VA_INDEP_WIN_MAX) {
                    win_size.w = KDRV_IFE_VA_INDEP_WIN_MAX;
				}

				win_size.h = ALIGN_FLOOR_4(((p_iq_cfg->va_window.winsz_ratio.h * blk_size.h) / p_iq_cfg->va_window.ratio_base));
				if (win_size.h > KDRV_IFE_VA_INDEP_WIN_MAX) {
                    win_size.h = KDRV_IFE_VA_INDEP_WIN_MAX;
				}

				va_win.win_numx = p_iq_cfg->va.win_num.w;
				va_win.win_numy = p_iq_cfg->va.win_num.h;
				va_win.win_szx = win_size.w;
				va_win.win_szy = win_size.h;
				va_win.win_stx = (blk_size.w - win_size.w) >> 1;
				va_win.win_sty = (blk_size.h - win_size.h) >> 1;
				va_win.win_spx = (blk_size.w - win_size.w);
				va_win.win_spy = (blk_size.h - win_size.h);

				p_cfg->p_iocfg->va_win_stx = va_win.win_stx;
				p_cfg->p_iocfg->va_win_size_x = va_win.win_szx;
				p_cfg->p_iocfg->va_win_skip_x = va_win.win_spx;

				if ((win_size.w > KDRV_IFE_VA_INDEP_WIN_MAX) || (win_size.h > KDRV_IFE_VA_INDEP_WIN_MAX) ||
					(va_win.win_spx >= 64) || (va_win.win_spy >= 64)) {
					//win_size.w = (win_size.w > KDRV_IFE_VA_INDEP_WIN_MAX) ? KDRV_IFE_VA_INDEP_WIN_MAX : win_size.w;
					//win_size.h = (win_size.h > KDRV_IFE_VA_INDEP_WIN_MAX) ? KDRV_IFE_VA_INDEP_WIN_MAX : win_size.h;
					va_win.win_spx = (va_win.win_spx >= 64) ? 63 : va_win.win_spx;
					va_win.win_spy = (va_win.win_spy >= 64) ? 63 : va_win.win_spy;
				}
				//img_size.h_size = p_cfg->p_iocfg->in_frm.size.w;
				//img_size.v_size = p_cfg->p_iocfg->in_frm.size.h;
				//rt= ipe_eng_check_va_win_info(p_hdl->p_eng, &va_win, img_size);
				//if (rt != E_OK) {
				//	DBG_ERR("va limit fail\r\n");
				//	return rt;
				//}

				ife_eng_set_va_win_info_buf_reg(p_hdl->p_eng, &va_win);
				ife_eng_set_va_out_sel_buf_reg(p_hdl->p_eng, p_iq_cfg->va.va_out_grp1_2);
				ife_eng_set_vacc_enable_buf_reg(p_hdl->p_eng, ENABLE);
			} else {
				ife_eng_set_vacc_enable_buf_reg(p_hdl->p_eng, DISABLE);
			}
		}

		{
			if (p_iq_cfg->va.indep_va_enable) {
				IFE_INDEP_VA_PARAM indep_va_win_info;
				BOOL va_indep_en[KDRV_IFE_IQ_VA_INDEP_NUM];

				for (i = 0; i < KDRV_IFE_IQ_VA_INDEP_NUM; i++) {
					va_indep_en[i] = p_iq_cfg->va.indep_win[i].enable;
					//indep_va_win_info.linemax_g1_en = p_iq_cfg->va.indep_win[i].linemax_g1;  //530 removed
					//indep_va_win_info.linemax_g2_en = p_iq_cfg->va.indep_win[i].linemax_g2;  //530 removed

					indep_va_win_info.win_stx = ALIGN_FLOOR_4(((p_iq_cfg->va_window.indep_roi_ratio[i].x * /*(p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width)*/p_cfg->p_iocfg->in_frm.size.w) / p_iq_cfg->va_window.ratio_base));
					indep_va_win_info.win_sty = ALIGN_FLOOR_4(((p_iq_cfg->va_window.indep_roi_ratio[i].y * p_cfg->p_iocfg->in_frm.size.h) / p_iq_cfg->va_window.ratio_base));
					indep_va_win_info.win_szx = ALIGN_FLOOR_4(((p_iq_cfg->va_window.indep_roi_ratio[i].w * /*(p_cfg->p_iocfg->dual_left_width + p_cfg->p_iocfg->dual_right_width)*/p_cfg->p_iocfg->in_frm.size.w) / p_iq_cfg->va_window.ratio_base));
					indep_va_win_info.win_szy = ALIGN_FLOOR_4(((p_iq_cfg->va_window.indep_roi_ratio[i].h * p_cfg->p_iocfg->in_frm.size.h) / p_iq_cfg->va_window.ratio_base));
					if (indep_va_win_info.win_szx > KDRV_IFE_VA_INDEP_WIN_MAX) {
						indep_va_win_info.win_stx = indep_va_win_info.win_stx +  ALIGN_FLOOR_4((indep_va_win_info.win_szx - KDRV_IFE_VA_INDEP_WIN_MAX) / 2);
						indep_va_win_info.win_szx = KDRV_IFE_VA_INDEP_WIN_MAX;
					}
					if (indep_va_win_info.win_szy > KDRV_IFE_VA_INDEP_WIN_MAX) {
						indep_va_win_info.win_sty = indep_va_win_info.win_sty +  ALIGN_FLOOR_4((indep_va_win_info.win_szy - KDRV_IFE_VA_INDEP_WIN_MAX) / 2);
						indep_va_win_info.win_szy = KDRV_IFE_VA_INDEP_WIN_MAX;
					}
					ife_eng_set_va_indep_win_buf_reg(p_hdl->p_eng, (IFE_INDEP_VA_PARAM *)&indep_va_win_info, i);
				}
				ife_eng_set_vacc_win_enable_buf_reg(p_hdl->p_eng, va_indep_en[0], va_indep_en[1], va_indep_en[2], va_indep_en[3], va_indep_en[4]);
			}else
				ife_eng_set_vacc_win_enable_buf_reg(p_hdl->p_eng, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE);

			if (p_iq_cfg->va.enable || p_iq_cfg->va.indep_va_enable) {

					IFE_VA_FLTR_GROUP_PARAM va_fltr_g1 = {0};
					IFE_VA_FLTR_GROUP_PARAM va_fltr_g2 = {0};
					IFE_VA_COMMON_PARAM va_common_param = {0};
					IFE_VA_FILTER_SEL va_fltr_sel = {0};
					IFE_VA_SHIFT_BIT  va_shift_bit = {0};


					va_fltr_g1.cnt_en = p_iq_cfg->va.group_1.count_enable;
					//va_fltr_g1.linemax_en = p_iq_cfg->va.group_1.linemax_mode; //530 removed
					va_fltr_g1.filt_h.fltr_size = p_iq_cfg->va.group_1.h_filt.filter_size;
					va_fltr_g1.filt_h.filt_symm = p_iq_cfg->va.group_1.h_filt.symmetry;
					va_fltr_g1.filt_h.filt_symm_iir2 = p_iq_cfg->va.group_1.iir_filt.symmetry_iir2;
					va_fltr_g1.filt_h.filt_symm_iir3 = p_iq_cfg->va.group_1.iir_filt.symmetry_iir3;
					va_fltr_g1.filt_h.tap_a = p_iq_cfg->va.group_1.h_filt.tap_a;
					va_fltr_g1.filt_h.tap_b = p_iq_cfg->va.group_1.h_filt.tap_b;
					va_fltr_g1.filt_h.tap_c = p_iq_cfg->va.group_1.h_filt.tap_c;
					va_fltr_g1.filt_h.tap_d = p_iq_cfg->va.group_1.h_filt.tap_d;
					va_fltr_g1.filt_h.tap_iir1_e = p_iq_cfg->va.group_1.iir_filt.tap_iir1_e;
					va_fltr_g1.filt_h.tap_iir1_f = p_iq_cfg->va.group_1.iir_filt.tap_iir1_f;
					va_fltr_g1.filt_h.tap_iir2_a = p_iq_cfg->va.group_1.iir_filt.tap_iir2_a;
					va_fltr_g1.filt_h.tap_iir2_b = p_iq_cfg->va.group_1.iir_filt.tap_iir2_b;
					va_fltr_g1.filt_h.tap_iir2_e = p_iq_cfg->va.group_1.iir_filt.tap_iir2_e;
					va_fltr_g1.filt_h.tap_iir2_f = p_iq_cfg->va.group_1.iir_filt.tap_iir2_f;
					va_fltr_g1.filt_h.tap_iir3_a = p_iq_cfg->va.group_1.iir_filt.tap_iir3_a;
					va_fltr_g1.filt_h.tap_iir3_b = p_iq_cfg->va.group_1.iir_filt.tap_iir3_b;
					va_fltr_g1.filt_h.tap_iir3_e = p_iq_cfg->va.group_1.iir_filt.tap_iir3_e;
					va_fltr_g1.filt_h.tap_iir3_f = p_iq_cfg->va.group_1.iir_filt.tap_iir3_f;
					va_fltr_g1.filt_h.div = p_iq_cfg->va.group_1.h_filt.div;
					va_fltr_g1.filt_h.th_low = p_iq_cfg->va.group_1.h_filt.th_l;
					va_fltr_g1.filt_h.th_high = p_iq_cfg->va.group_1.h_filt.th_u;
					va_fltr_g1.h_iir2_en = p_iq_cfg->va.group_1.iir_filt.iir2_enable;
					va_fltr_g1.h_iir3_en = p_iq_cfg->va.group_1.iir_filt.iir3_enable;

					va_fltr_g1.filt_v.fltr_size = p_iq_cfg->va.group_1.v_filt.filter_size;
					va_fltr_g1.filt_v.filt_symm = p_iq_cfg->va.group_1.v_filt.symmetry;
					va_fltr_g1.filt_v.filt_symm_iir2 = p_iq_cfg->va.group_1.iir_filt.symmetry_iir2;
					va_fltr_g1.filt_v.filt_symm_iir3 = p_iq_cfg->va.group_1.iir_filt.symmetry_iir3;
					va_fltr_g1.filt_v.tap_a =p_iq_cfg->va.group_1.v_filt.tap_a;
					va_fltr_g1.filt_v.tap_b = p_iq_cfg->va.group_1.v_filt.tap_b;
					va_fltr_g1.filt_v.tap_c = p_iq_cfg->va.group_1.v_filt.tap_c;
					va_fltr_g1.filt_v.tap_d = p_iq_cfg->va.group_1.v_filt.tap_d;
					va_fltr_g1.filt_v.tap_iir1_e = p_iq_cfg->va.group_1.iir_filt.tap_iir1_e;
					va_fltr_g1.filt_v.tap_iir1_f = p_iq_cfg->va.group_1.iir_filt.tap_iir1_f;
					va_fltr_g1.filt_v.tap_iir2_a = p_iq_cfg->va.group_1.iir_filt.tap_iir2_a;
					va_fltr_g1.filt_v.tap_iir2_b = p_iq_cfg->va.group_1.iir_filt.tap_iir2_b;
					va_fltr_g1.filt_v.tap_iir2_e = p_iq_cfg->va.group_1.iir_filt.tap_iir2_e;
					va_fltr_g1.filt_v.tap_iir2_f = p_iq_cfg->va.group_1.iir_filt.tap_iir2_f;
					va_fltr_g1.filt_v.tap_iir3_a = p_iq_cfg->va.group_1.iir_filt.tap_iir3_a;
					va_fltr_g1.filt_v.tap_iir3_b = p_iq_cfg->va.group_1.iir_filt.tap_iir3_b;
					va_fltr_g1.filt_v.tap_iir3_e = p_iq_cfg->va.group_1.iir_filt.tap_iir3_e;
					va_fltr_g1.filt_v.tap_iir3_f = p_iq_cfg->va.group_1.iir_filt.tap_iir3_f;
					va_fltr_g1.filt_v.div = p_iq_cfg->va.group_1.v_filt.div;
					va_fltr_g1.filt_v.th_low = p_iq_cfg->va.group_1.v_filt.th_l;
					va_fltr_g1.filt_v.th_high = p_iq_cfg->va.group_1.v_filt.th_u;

					va_fltr_g2.cnt_en = p_iq_cfg->va.group_2.count_enable;
					//va_fltr_g2.linemax_en = p_iq_cfg->va.group_2.linemax_mode; //530 removed
					va_fltr_g2.filt_h.fltr_size = p_iq_cfg->va.group_2.h_filt.filter_size;
					va_fltr_g2.filt_h.filt_symm = p_iq_cfg->va.group_2.h_filt.symmetry;
					va_fltr_g2.filt_h.filt_symm_iir2 = p_iq_cfg->va.group_2.iir_filt.symmetry_iir2;
					va_fltr_g2.filt_h.filt_symm_iir3 = p_iq_cfg->va.group_2.iir_filt.symmetry_iir3;
					va_fltr_g2.filt_h.tap_a = p_iq_cfg->va.group_2.h_filt.tap_a;
					va_fltr_g2.filt_h.tap_b = p_iq_cfg->va.group_2.h_filt.tap_b;
					va_fltr_g2.filt_h.tap_c = p_iq_cfg->va.group_2.h_filt.tap_c;
					va_fltr_g2.filt_h.tap_d = p_iq_cfg->va.group_2.h_filt.tap_d;
					va_fltr_g2.filt_h.tap_iir1_e = p_iq_cfg->va.group_2.iir_filt.tap_iir1_e;
					va_fltr_g2.filt_h.tap_iir1_f = p_iq_cfg->va.group_2.iir_filt.tap_iir1_f;
					va_fltr_g2.filt_h.tap_iir2_a = p_iq_cfg->va.group_2.iir_filt.tap_iir2_a;
					va_fltr_g2.filt_h.tap_iir2_b = p_iq_cfg->va.group_2.iir_filt.tap_iir2_b;
					va_fltr_g2.filt_h.tap_iir2_e = p_iq_cfg->va.group_2.iir_filt.tap_iir2_e;
					va_fltr_g2.filt_h.tap_iir2_f = p_iq_cfg->va.group_2.iir_filt.tap_iir2_f;
					va_fltr_g2.filt_h.tap_iir3_a = p_iq_cfg->va.group_2.iir_filt.tap_iir3_a;
					va_fltr_g2.filt_h.tap_iir3_b = p_iq_cfg->va.group_2.iir_filt.tap_iir3_b;
					va_fltr_g2.filt_h.tap_iir3_e = p_iq_cfg->va.group_2.iir_filt.tap_iir3_e;
					va_fltr_g2.filt_h.tap_iir3_f = p_iq_cfg->va.group_2.iir_filt.tap_iir3_f;
					va_fltr_g2.filt_h.div = p_iq_cfg->va.group_2.h_filt.div;
					va_fltr_g2.filt_h.th_low = p_iq_cfg->va.group_2.h_filt.th_l;
					va_fltr_g2.filt_h.th_high = p_iq_cfg->va.group_2.h_filt.th_u;
					va_fltr_g2.h_iir2_en = p_iq_cfg->va.group_2.iir_filt.iir2_enable;
					va_fltr_g2.h_iir3_en = p_iq_cfg->va.group_2.iir_filt.iir3_enable;

					va_fltr_g2.filt_v.fltr_size = p_iq_cfg->va.group_2.v_filt.filter_size;
					va_fltr_g2.filt_v.filt_symm = p_iq_cfg->va.group_2.v_filt.symmetry;
					va_fltr_g2.filt_v.filt_symm_iir2 = p_iq_cfg->va.group_2.iir_filt.symmetry_iir2;
					va_fltr_g2.filt_v.filt_symm_iir3 = p_iq_cfg->va.group_2.iir_filt.symmetry_iir3;
					va_fltr_g2.filt_v.tap_a =p_iq_cfg->va.group_2.v_filt.tap_a;
					va_fltr_g2.filt_v.tap_b = p_iq_cfg->va.group_2.v_filt.tap_b;
					va_fltr_g2.filt_v.tap_c = p_iq_cfg->va.group_2.v_filt.tap_c;
					va_fltr_g2.filt_v.tap_d = p_iq_cfg->va.group_2.v_filt.tap_d;
					va_fltr_g2.filt_v.tap_iir1_e = p_iq_cfg->va.group_2.iir_filt.tap_iir1_e;
					va_fltr_g2.filt_v.tap_iir1_f = p_iq_cfg->va.group_2.iir_filt.tap_iir1_f;
					va_fltr_g2.filt_v.tap_iir2_a = p_iq_cfg->va.group_2.iir_filt.tap_iir2_a;
					va_fltr_g2.filt_v.tap_iir2_b = p_iq_cfg->va.group_2.iir_filt.tap_iir2_b;
					va_fltr_g2.filt_v.tap_iir2_e = p_iq_cfg->va.group_2.iir_filt.tap_iir2_e;
					va_fltr_g2.filt_v.tap_iir2_f = p_iq_cfg->va.group_2.iir_filt.tap_iir2_f;
					va_fltr_g2.filt_v.tap_iir3_a = p_iq_cfg->va.group_2.iir_filt.tap_iir3_a;
					va_fltr_g2.filt_v.tap_iir3_b = p_iq_cfg->va.group_2.iir_filt.tap_iir3_b;
					va_fltr_g2.filt_v.tap_iir3_e = p_iq_cfg->va.group_2.iir_filt.tap_iir3_e;
					va_fltr_g2.filt_v.tap_iir3_f = p_iq_cfg->va.group_2.iir_filt.tap_iir3_f;
					va_fltr_g2.filt_v.div = p_iq_cfg->va.group_2.v_filt.div;
					va_fltr_g2.filt_v.th_low = p_iq_cfg->va.group_2.v_filt.th_l;
					va_fltr_g2.filt_v.th_high = p_iq_cfg->va.group_2.v_filt.th_u;


					ife_eng_set_va_filter_g1_buf_reg(p_hdl->p_eng, &va_fltr_g1);
					ife_eng_set_va_filter_g2_buf_reg(p_hdl->p_eng, &va_fltr_g2);
					ife_eng_set_va_mode_enable_buf_reg(p_hdl->p_eng, &va_fltr_g1, &va_fltr_g2);

					va_fltr_sel.vdetgh1_filter_sel = p_iq_cfg->va.fltr_sel.vdetgh1_filter_sel;
					va_fltr_sel.vdetgh2_filter_sel = p_iq_cfg->va.fltr_sel.vdetgh2_filter_sel;
					va_fltr_sel.vdetgh1_iir_input_sel = p_iq_cfg->va.fltr_sel.vdetgh1_iir_input_sel;
					va_fltr_sel.vdetgh2_iir_input_sel =  p_iq_cfg->va.fltr_sel.vdetgh2_iir_input_sel;

					ife_eng_set_va_filter_sel_buf_reg(p_hdl->p_eng, &va_fltr_sel);

					va_shift_bit.vdetgh1_iir_shift_bit = p_iq_cfg->va.group_1.iir_filt.iir1_shift_bit;
					va_shift_bit.vdetgh1_iir2_shift_bit = p_iq_cfg->va.group_1.iir_filt.iir2_shift_bit;
					va_shift_bit.vdetgh1_iir3_shift_bit = p_iq_cfg->va.group_1.iir_filt.iir3_shift_bit;

					va_shift_bit.vdetgh2_iir_shift_bit = p_iq_cfg->va.group_2.iir_filt.iir1_shift_bit;
					va_shift_bit.vdetgh2_iir2_shift_bit = p_iq_cfg->va.group_2.iir_filt.iir2_shift_bit;
					va_shift_bit.vdetgh2_iir3_shift_bit = p_iq_cfg->va.group_2.iir_filt.iir3_shift_bit;

					ife_eng_set_va_shift_bit_buf_reg(p_hdl->p_eng, &va_shift_bit);

					va_common_param.pre_filter_mode = p_iq_cfg->va.pre_filter_mode;

					va_common_param.blending_w = p_iq_cfg->va.energy_w;
					va_common_param.high_luma_th = p_iq_cfg->va.high_luma_th;
					va_common_param.win_cnt_out_sel = p_iq_cfg->va.win_cnt_out_sel;
					ife_eng_set_va_common_info_buf_reg(p_hdl->p_eng, &va_common_param);

					//va ldg
					if(p_iq_cfg->va.ldg_enable){

						IFE_VA_LDG_PARAM ldg_param = {0};

						ldg_param.ldg_high_gain = p_iq_cfg->va.ldg_para.ldg_high_gain;
						ldg_param.ldg_low_gain = p_iq_cfg->va.ldg_para.ldg_low_gain;
						ldg_param.ldg_high_slope = p_iq_cfg->va.ldg_para.ldg_high_slope;
						ldg_param.ldg_low_slope = p_iq_cfg->va.ldg_para.ldg_low_slope;
						ldg_param.ldg_high_th = p_iq_cfg->va.ldg_para.ldg_high_th;
						ldg_param.ldg_low_th = p_iq_cfg->va.ldg_para.ldg_low_th;

						ife_eng_set_va_ldg_buf_reg(p_hdl->p_eng, &ldg_param);
						ife_eng_set_va_ldg_enable_buf_reg(p_hdl->p_eng, ENABLE);
					}else{
						ife_eng_set_va_ldg_enable_buf_reg(p_hdl->p_eng, DISABLE);
					}


					//va gamma
					{
						IFE_VA_GAMMA gamma_param = {0};
						gamma_param.en = p_iq_cfg->va.gamma_enable;
						gamma_param.p_lut = &p_iq_cfg->va.va_gamma_lut[0];

						ife_eng_set_va_gamma_buf_reg(p_hdl->p_eng, &gamma_param);
					}

			}
		}
	}
#endif

	return E_OK;
}



static INT32 kdrv_ife_int_cfg_nn_isp(KDRV_IFE_HANDLE *p_hdl, KDRV_IFE_JOB_CFG *p_info)
{
	KDRV_IFE_IO_CFG *p_iocfg = p_info->p_iocfg;
	//KDRV_IFE_IQ_CFG *p_iqcfg = p_info->p_iqcfg;
	UINT32 ring_buf_height = 0;

	if(p_iocfg->nn_isp_param.nn_isp_p0.path_en){

		ife_eng_set_nn_isp_enable_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH0, ENABLE);

		/*---------- ring buf ------------*/
		if(p_iocfg->nn_isp_param.nn_isp_p0.ringbuf_addr.pa != 0 && p_iocfg->nn_isp_param.nn_isp_p0.ringbuf_addr.pa != (ULONG)-1){
			UINT64 addr = p_iocfg->nn_isp_param.nn_isp_p0.ringbuf_addr.pa;

			ife_eng_set_nn_isp_ring_buf_addr_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH0, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF));
		}else{
			DBG_ERR("nn_isp_p0.ringbuf_addr err:%lx\n", p_iocfg->nn_isp_param.nn_isp_p0.ringbuf_addr.pa);
			return E_PAR;
		}
		/*---------- out buf ------------*/
		if(p_iocfg->nn_isp_param.nn_isp_p0.outbuf_addr.pa != 0 && p_iocfg->nn_isp_param.nn_isp_p0.outbuf_addr.pa != (ULONG)-1){
			UINT64 addr = p_iocfg->nn_isp_param.nn_isp_p0.outbuf_addr.pa;
			ife_eng_set_nn_isp_out_buf_addr_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH0, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF));
		}else{
			DBG_ERR("nn_isp_p0.outbuf_addr err:%lx\n", p_iocfg->nn_isp_param.nn_isp_p0.outbuf_addr.pa);
			return E_PAR;
		}
		/*----------line offset ------------*/
		ife_eng_set_nn_isp_lineoffset_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH0, p_iocfg->nn_isp_param.nn_isp_p0.lofs);

		ring_buf_height =  p_iocfg->nn_isp_param.nn_isp_p0.ringbuf_height;//(p_iocfg->nn_isp_param.nn_isp_p0.slice_height << 1) -  p_iocfg->nn_isp_param.nn_isp_p0.slice_ovlp;
		ife_eng_set_nn_isp_slice_info_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH0, ring_buf_height, p_iocfg->nn_isp_param.nn_isp_p0.slice_height, p_iocfg->nn_isp_param.nn_isp_p0.slice_ovlp);
		ife_eng_set_nn_isp_output_height_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH0, p_iocfg->nn_isp_param.nn_isp_p0.outbuf_height);

		ife_eng_set_nn_isp_fw_handshake_enable_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH0, p_iocfg->nn_isp_param.nn_isp_p0.fw_handshake_en);

	}else{
		ife_eng_set_nn_isp_enable_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH0, DISABLE);
	}

	if(p_iocfg->nn_isp_param.nn_isp_p1.path_en){

		ife_eng_set_nn_isp_enable_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH1, ENABLE);

		/*---------- ring buf ------------*/
		if(p_iocfg->nn_isp_param.nn_isp_p1.ringbuf_addr.pa != 0 && p_iocfg->nn_isp_param.nn_isp_p1.ringbuf_addr.pa != (ULONG) -1){
			UINT64 addr = p_iocfg->nn_isp_param.nn_isp_p1.ringbuf_addr.pa;
			ife_eng_set_nn_isp_ring_buf_addr_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH1, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF));
		}else{
			DBG_ERR("nn_isp_p1.ringbuf_addr err:%lx\n", p_iocfg->nn_isp_param.nn_isp_p1.ringbuf_addr.pa);
			return E_PAR;
		}
		/*---------- out buf ------------*/
		if(p_iocfg->nn_isp_param.nn_isp_p1.outbuf_addr.pa != 0 && p_iocfg->nn_isp_param.nn_isp_p1.outbuf_addr.pa != (ULONG)-1){
			UINT64 addr = p_iocfg->nn_isp_param.nn_isp_p1.outbuf_addr.pa;
			ife_eng_set_nn_isp_out_buf_addr_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH1, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF));
		}else{
			DBG_ERR("nn_isp_p1.outbuf_y_addr err:%lx\n", p_iocfg->nn_isp_param.nn_isp_p1.outbuf_addr.pa);
			return E_PAR;
		}
		/*----------line offset ------------*/
		ife_eng_set_nn_isp_lineoffset_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH1, p_iocfg->nn_isp_param.nn_isp_p1.lofs);

		ring_buf_height =  p_iocfg->nn_isp_param.nn_isp_p1.ringbuf_height;//(p_iocfg->nn_isp_param.nn_isp_p1.slice_height << 1) -  p_iocfg->nn_isp_param.nn_isp_p1.slice_ovlp;
		ife_eng_set_nn_isp_slice_info_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH1, ring_buf_height, p_iocfg->nn_isp_param.nn_isp_p1.slice_height, p_iocfg->nn_isp_param.nn_isp_p1.slice_ovlp);
		ife_eng_set_nn_isp_output_height_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH1, p_iocfg->nn_isp_param.nn_isp_p1.outbuf_height);

		ife_eng_set_nn_isp_fw_handshake_enable_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH1, p_iocfg->nn_isp_param.nn_isp_p1.fw_handshake_en);
	}else{
		ife_eng_set_nn_isp_enable_buf_reg(p_hdl->p_eng, KDRV_IFE_NN_ISP_PATH1, DISABLE);
	}


#if KDRV_IPP_AI_DROP
	//////// drop frm work around for aiisp 3dnr 1st abnormal frm /////////////
	if(p_iocfg->nn_isp_param.nn_isp_p0.update){

		if(p_iocfg->nn_isp_param.nn_isp_p0.path_en){
			//DBG_DUMP("-------p0 update enable------\n");
			p_iocfg->nn_isp_drop_frm_flg = 1;
		}

	}

	if(p_iocfg->nn_isp_param.nn_isp_p1.update){

		if(p_iocfg->nn_isp_param.nn_isp_p1.path_en){
			//DBG_DUMP("-------p1 update enable------\n");
			p_iocfg->nn_isp_drop_frm_flg = 1;
		}

	}
	/////////////////////////////////////////////////////////////////////////////
#endif

	return E_OK;
}

static INT32 kdrv_ife_int_cfg_dual_pipe(KDRV_IFE_HANDLE *p_hdl, KDRV_IFE_JOB_CFG *p_info, IFE_SIE_HANDSHAKE* sie_handshake)
{
	KDRV_IFE_IO_CFG* p_io_cfg = p_info->p_iocfg;
	KDRV_IFE_IQ_CFG* p_iq_cfg = p_info->p_iqcfg;

	if(p_io_cfg->dual_en){
		//IFE_ENG_DUAL_PARAM param = {0};

		if(p_io_cfg->dual_idx == KDRV_IFE_LEFT){

			//param.start_offset = 0;
			//param.left_overlap = 0;
			//param.right_overlap = p_io_cfg->dual_right_ovlp;

			p_io_cfg->dual_start_ofs = 0;

			if( KDRV_IPP_FMT_CLASS(p_info->p_iocfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX ){
				if(p_info->p_iocfg->rde.encode_rate == KDRV_IFE_ENCODE_RATE_50){
					p_info->p_iocfg->in_frm.lofs[0] = ((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width + 63) >> 6)*12*4;
					p_info->p_iocfg->in_frm.addr_ofs[0] = 0;
				}else if(p_info->p_iocfg->rde.encode_rate == KDRV_IFE_ENCODE_RATE_58){
					p_info->p_iocfg->in_frm.lofs[0] =  ((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width + 63) >> 6)*14*4;
					p_info->p_iocfg->in_frm.addr_ofs[0] = 0;
				}/*else if(p_info->p_iocfg->rde.encode_rate == KDRV_IFE_ENCODE_RATE_66){
					p_info->p_iocfg->in_frm.lofs[0] =  ((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width + 63) >> 6)*16*4;
					p_info->p_iocfg->in_frm.addr_ofs[0] = 0;
				}else if(p_info->p_iocfg->rde.encode_rate == KDRV_IFE_ENCODE_RATE_41){
					p_info->p_iocfg->in_frm.lofs[0] =  ((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width + 63) >> 6)*10*4;
					p_info->p_iocfg->in_frm.addr_ofs[0] = 0;
				}*/
				else{
					DBG_ERR("unknown bayer compress rate\n");
					return E_PAR;
				}
			}else if( KDRV_IPP_FMT_CLASS(p_info->p_iocfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW ){

				p_info->p_iocfg->in_frm.lofs[0] = ALIGN_CEIL_4(((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width)*3)/2);
				p_info->p_iocfg->in_frm.addr_ofs[0] = 0;

			}else if( KDRV_IPP_FMT_CLASS(p_info->p_iocfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_YUV ){

				p_info->p_iocfg->in_frm.lofs[0] = ALIGN_CEIL_4(((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width)*3)/2);
				p_info->p_iocfg->in_frm.addr_ofs[0] = 0;

			}else{
				DBG_ERR("ife unsupport dual in_fmt:0x%x\n", p_info->p_iocfg->in_frm.fmt);
				return E_PAR;
			}

			sie_handshake->input_combine_en = 0;
			sie_handshake->src_clk_sel_fifo0 = 0;
			sie_handshake->src_clk_sel_fifo1 = 0;
			sie_handshake->in_buf_write_width0 = p_info->p_iocfg->dual_right_width + p_info->p_iocfg->dual_left_width;
			sie_handshake->in_buf_write_width1 = p_info->p_iocfg->dual_right_width + p_info->p_iocfg->dual_left_width;
			sie_handshake->in_buf_crop_st0 = 0;
			sie_handshake->in_buf_crop_st1 = 0;
			sie_handshake->in_buf_crop_width0 = p_info->p_iocfg->dual_left_width + p_info->p_iocfg->dual_right_ovlp;
			sie_handshake->in_buf_crop_width1 = p_info->p_iocfg->dual_left_width + p_info->p_iocfg->dual_right_ovlp;

		}else if(p_io_cfg->dual_idx == KDRV_IFE_RIGHT){

			//param.start_offset = p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp;  //unit : pixel
			//param.left_overlap = p_io_cfg->dual_left_ovlp;
			//param.right_overlap = 0;

			p_io_cfg->dual_start_ofs = p_info->p_iocfg->dual_left_width - p_info->p_iocfg->dual_left_ovlp;

			if( KDRV_IPP_FMT_CLASS(p_info->p_iocfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX ){
				if(p_info->p_iocfg->rde.encode_rate == KDRV_IFE_ENCODE_RATE_50){
					p_info->p_iocfg->in_frm.lofs[0] = ((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width + 63) >> 6)*12*4;
					p_info->p_iocfg->in_frm.addr_ofs[0] = ((p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp + 63) >> 6)*12*4;
					p_info->p_iocfg->in_frm.addr_ofs[1] = ((p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp) >> 6)*12*4;
				}else if(p_info->p_iocfg->rde.encode_rate == KDRV_IFE_ENCODE_RATE_58){
					p_info->p_iocfg->in_frm.lofs[0] =  ((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width + 63) >> 6)*14*4;
					p_info->p_iocfg->in_frm.addr_ofs[0] = ((p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp + 63) >> 6)*14*4;
					p_info->p_iocfg->in_frm.addr_ofs[1] = ((p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp + 63) >> 6)*14*4;
				}/*else if(p_info->p_iocfg->rde.encode_rate == KDRV_IFE_ENCODE_RATE_66){
					p_info->p_iocfg->in_frm.lofs[0] =  ((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width + 63) >> 6)*16*4;
					p_info->p_iocfg->in_frm.addr_ofs[0] = ((p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp + 63) >> 6)*16*4;
					p_info->p_iocfg->in_frm.addr_ofs[1] = ((p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp + 63) >> 6)*16*4;
				}else if(p_info->p_iocfg->rde.encode_rate == KDRV_IFE_ENCODE_RATE_41){
					p_info->p_iocfg->in_frm.lofs[0] =  ((p_io_cfg->dual_left_width + p_io_cfg->dual_right_width + 63) >> 6)*10*4;
					p_info->p_iocfg->in_frm.addr_ofs[0] = ((p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp + 63) >> 6)*10*4;
					p_info->p_iocfg->in_frm.addr_ofs[1] = ((p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp + 63) >> 6)*10*4;
				}*/
				else{
					DBG_ERR("unknown bayer compress rate\n");
					return E_PAR;
				}
			}else if( KDRV_IPP_FMT_CLASS(p_info->p_iocfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW ){

				p_info->p_iocfg->in_frm.lofs[0] = ALIGN_CEIL_4(((p_io_cfg->dual_right_width + p_io_cfg->dual_left_width)*3)/2);
				p_info->p_iocfg->in_frm.addr_ofs[0] = ALIGN_CEIL_4(((p_io_cfg->dual_left_width  - p_io_cfg->dual_right_ovlp)*3)/2);
				p_info->p_iocfg->in_frm.addr_ofs[1] = ALIGN_CEIL_4(((p_io_cfg->dual_left_width  - p_io_cfg->dual_right_ovlp)*3)/2);

			}else if( KDRV_IPP_FMT_CLASS(p_info->p_iocfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_YUV ){

				p_info->p_iocfg->in_frm.lofs[0] = ALIGN_CEIL_4(((p_io_cfg->dual_right_width + p_io_cfg->dual_left_width)*3)/2);
				p_info->p_iocfg->in_frm.addr_ofs[0] = ALIGN_CEIL_4(((p_io_cfg->dual_left_width  - p_io_cfg->dual_right_ovlp)*3)/2);
				p_info->p_iocfg->in_frm.addr_ofs[1] = ALIGN_CEIL_4(((p_io_cfg->dual_left_width  - p_io_cfg->dual_right_ovlp)*3)/2);


			}else{
				DBG_ERR("ife unsupport dual in_fmt:0x%x\n", p_info->p_iocfg->in_frm.fmt);
				return E_PAR;
			}

			sie_handshake->input_combine_en = 0;
			sie_handshake->src_clk_sel_fifo0 = 0;
			sie_handshake->src_clk_sel_fifo1 = 0;
			sie_handshake->in_buf_write_width0 = p_io_cfg->dual_right_width + p_io_cfg->dual_left_width;
			sie_handshake->in_buf_write_width1 = p_io_cfg->dual_right_width + p_io_cfg->dual_left_width;
			sie_handshake->in_buf_crop_st0 = p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp;
			sie_handshake->in_buf_crop_st1 = p_io_cfg->dual_left_width - p_io_cfg->dual_right_ovlp;
			sie_handshake->in_buf_crop_width0 = p_io_cfg->dual_right_width + p_io_cfg->dual_left_ovlp;
			sie_handshake->in_buf_crop_width1 = p_io_cfg->dual_right_width + p_io_cfg->dual_left_ovlp;

		}else{
			DBG_ERR("unknow dual idx:%d\n", p_io_cfg->dual_idx);
			return E_PAR;
		}


		if(p_iq_cfg->va.enable){

			if(p_io_cfg->dual_idx == KDRV_IFE_LEFT){

				p_io_cfg->va_out_addr_ofs = 0;

			}else if(p_io_cfg->dual_idx == KDRV_IFE_RIGHT){

				p_io_cfg->va_out_addr_ofs = 0;
				p_io_cfg->va_out_addr.pa += KDRV_IFE_VA_BUF_SIZE;
				p_io_cfg->va_out_addr.va += KDRV_IFE_VA_BUF_SIZE;
			}
			//ife_eng_set_force_enable_buf_reg(p_hdl->p_eng, ENABLE);

		}
#if 0
		if(p_io_cfg->subisp_param.subisp_out_en){

			if(p_io_cfg->dual_idx == KDRV_IFE_LEFT){

				IFE_ENG_DUAL_PARAM dual_param = {0};
				IFE_ENG_SUBISP_PARAM param = {0};
				IFE_ENG_SUBISP_PARAM gbl_set = {0};

				dual_param.left_overlap = p_io_cfg->dual_left_ovlp;
				dual_param.right_overlap = p_io_cfg->dual_right_ovlp;
				dual_param.start_offset = 0;
				param.subisp_scale.sample_rate_x = p_io_cfg->subisp_param.subsample_rate_x;
				param.subisp_scale.sample_rate_y = p_io_cfg->subisp_param.subsample_rate_y;
				gbl_set.subisp_crop.crop_height = p_io_cfg->subisp_param.crop_height;
				gbl_set.subisp_crop.crop_width = p_io_cfg->subisp_param.crop_width;

				ife_eng_cal_dual_sub_isp_setting(&dual_param, &param, &gbl_set, p_io_cfg->dual_right_width + p_io_cfg->dual_left_width, p_io_cfg->dual_left_width,  IFE_PIPE_LEFT);
				ife_eng_set_subisp_dual_buf_reg(p_hdl->p_eng, param.subisp_dual);

				p_io_cfg->subisp_param.subisp_frm.addr_ofs[0] = 0;
				p_io_cfg->subisp_param.subisp_frm.addr_ofs[1] = 0;
				p_io_cfg->subisp_param.subisp_frm.lofs[0] = p_io_cfg->subisp_param.crop_width;
				p_io_cfg->subisp_param.subisp_frm.lofs[1] = p_io_cfg->subisp_param.crop_width;


				p_io_cfg->subisp_param.crop_width = param.subisp_crop.crop_width;


				/*DBG_DUMP("-~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~-\n");
				DBG_DUMP("-[%d]left_width:%d, right_width:%d-\n", p_io_cfg->dual_idx ,p_io_cfg->dual_left_width, p_io_cfg->dual_right_width);
				DBG_DUMP("-[%d]crop_height:%d, crop_width:%d-\n", p_io_cfg->dual_idx ,p_io_cfg->subisp_param.crop_height, p_io_cfg->subisp_param.crop_width);
				DBG_DUMP("-[%d]addr_ofs[0]:%ld, addr_ofs[1]:%ld-\n",p_io_cfg->dual_idx, p_io_cfg->subisp_param.subisp_frm.addr_ofs[0], p_io_cfg->subisp_param.subisp_frm.addr_ofs[1]);
				DBG_DUMP("-[%d]lofs[0]:%d, lofs[1]:%d-\n",p_io_cfg->dual_idx, p_io_cfg->subisp_param.subisp_frm.lofs[0], p_io_cfg->subisp_param.subisp_frm.lofs[1]);
				DBG_DUMP("-[%d]sub_ipp_para1:%d, sub_ipp_para2:%d, sub_ipp_smpl_hit_init:%d-\n",p_io_cfg->dual_idx, param.subisp_dual.sub_ipp_para1, param.subisp_dual.sub_ipp_para2, param.subisp_dual.sub_ipp_smpl_hit_init);
				*/

			}else if(p_io_cfg->dual_idx == KDRV_IFE_RIGHT){

				IFE_ENG_DUAL_PARAM dual_param = {0};
				IFE_ENG_SUBISP_PARAM param = {0};
				IFE_ENG_SUBISP_PARAM gbl_set = {0};

				dual_param.left_overlap = p_io_cfg->dual_left_ovlp;
				dual_param.right_overlap = p_io_cfg->dual_right_ovlp;
				dual_param.start_offset =  p_io_cfg->dual_left_width - p_io_cfg->dual_left_ovlp;

				param.subisp_scale.sample_rate_x = p_io_cfg->subisp_param.subsample_rate_x;
				param.subisp_scale.sample_rate_y = p_io_cfg->subisp_param.subsample_rate_y;
				gbl_set.subisp_crop.crop_height = p_io_cfg->subisp_param.crop_height;
				gbl_set.subisp_crop.crop_width = p_io_cfg->subisp_param.crop_width;

				ife_eng_cal_dual_sub_isp_setting(&dual_param, &param, &gbl_set, p_io_cfg->dual_right_width + p_io_cfg->dual_left_width, p_io_cfg->dual_right_width,  IFE_PIPE_RIGHT);
				ife_eng_set_subisp_dual_buf_reg(p_hdl->p_eng, param.subisp_dual);

				p_io_cfg->subisp_param.subisp_frm.addr_ofs[0] = (p_io_cfg->dual_left_width / (param.subisp_scale.sample_rate_x + 1));
				p_io_cfg->subisp_param.subisp_frm.addr_ofs[1] = (p_io_cfg->dual_left_width / (param.subisp_scale.sample_rate_x + 1));
				p_io_cfg->subisp_param.subisp_frm.lofs[0] = p_io_cfg->subisp_param.crop_width;
				p_io_cfg->subisp_param.subisp_frm.lofs[1] = p_io_cfg->subisp_param.crop_width;

				p_io_cfg->subisp_param.crop_width = param.subisp_crop.crop_width;

				/*DBG_DUMP("-~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~-\n");
				DBG_DUMP("-[%d]left_width:%d, right_width:%d-\n", p_io_cfg->dual_idx ,p_io_cfg->dual_left_width, p_io_cfg->dual_right_width);
				DBG_DUMP("-[%d]crop_height:%d, crop_width:%d-\n", p_io_cfg->dual_idx ,p_io_cfg->subisp_param.crop_height, p_io_cfg->subisp_param.crop_width);
				DBG_DUMP("-[%d]addr_ofs[0]:%ld, addr_ofs[1]:%ld-\n",p_io_cfg->dual_idx, p_io_cfg->subisp_param.subisp_frm.addr_ofs[0], p_io_cfg->subisp_param.subisp_frm.addr_ofs[1]);
				DBG_DUMP("-[%d]lofs[0]:%d, lofs[1]:%d-\n",p_io_cfg->dual_idx, p_io_cfg->subisp_param.subisp_frm.lofs[0], p_io_cfg->subisp_param.subisp_frm.lofs[1]);
				DBG_DUMP("-[%d]sub_ipp_para1:%d, sub_ipp_para2:%d, sub_ipp_smpl_hit_init:%d-\n", p_io_cfg->dual_idx, param.subisp_dual.sub_ipp_para1, param.subisp_dual.sub_ipp_para2, param.subisp_dual.sub_ipp_smpl_hit_init);
				*/
			}

		}
#endif
		//ife_eng_set_dual_para_buf_reg(p_hdl->p_eng, param);
	} //else{
		// 538 unuse
#if 0
		sie_handshake->src_clk_sel_fifo0 = p_io_cfg->combine_param.src_sel[0];
		sie_handshake->src_clk_sel_fifo1 = p_io_cfg->combine_param.src_sel[1];;
		sie_handshake->in_buf_write_width0 = p_io_cfg->combine_param.strp_size[0];
		sie_handshake->in_buf_write_width1 = p_io_cfg->combine_param.strp_size[1];
		sie_handshake->in_buf_crop_st0 = 0;
		sie_handshake->in_buf_crop_st1 = 0;
		sie_handshake->in_buf_crop_width0 = p_io_cfg->combine_param.strp_size[0];
		sie_handshake->in_buf_crop_width1 = p_io_cfg->combine_param.strp_size[1];
	}
#endif

	return E_OK;
}

#endif


static INT32 kdrv_ife_int_cfg_all(KDRV_IFE_HANDLE *p_hdl, KDRV_IFE_JOB_CFG *p_cfg)
{
#if (IFE_SSDRV_SUPPORT == 0)
	INT32 rt = E_OK;

	if (kdrv_ife_int_check_limit(p_cfg) != E_OK) {
		return E_PAR;
	}
#else
	KDRV_IFE_IO_CFG *io_cfg;
	KDRV_IFE_IQ_CFG *iq_cfg;
	IFE_OPMODE op_mode;
	IFE_STRIPE stripe_info = {0};
	IFE_RING_BUF ring_info = {0};
	IFE_VIG_PARAM vig_set = {0};
	IFE_SIE_HANDSHAKE sie_handshake = {0};
	IFE_BAYERFMTSEL bayer_fmt_sel = {0};
	INT8 fnum = 0;
	IFE_ENG_DMA_OUT_INFO single_out_info = {0};
	IFE_ENG_WDR_HIST hist_param   = {0};
	//IFE_ENG_DUAL_PARAM dual_param = {0};

	INT32 vig_x[KDRV_IFE_VIG_CH_MAX] = {0}, vig_y[KDRV_IFE_VIG_CH_MAX] = {0};
	UINT32 i;
	INT32 rt = E_OK;

	if (kdrv_ife_int_check_limit(p_cfg) != E_OK) {
		return E_PAR;
	}

	io_cfg = p_cfg->p_iocfg;
	iq_cfg = p_cfg->p_iqcfg;

	// set opmode
	op_mode = kdrv_ife_typecast_opmode(io_cfg->mode);
	ife_eng_set_op_mode_buf_reg(p_hdl->p_eng, op_mode);

	// set dual ipp config
	rt = kdrv_ife_int_cfg_dual_pipe(p_hdl, p_cfg, &sie_handshake);

	// set in image size
	ife_eng_set_in_size_buf_reg(p_hdl->p_eng, io_cfg->in_frm.size.w, io_cfg->in_frm.size.h);
	ife_eng_set_crop_buf_reg(p_hdl->p_eng, io_cfg->crp_window.w, io_cfg->crp_window.h, io_cfg->crp_window.x, io_cfg->crp_window.y);

	// set stripe info
	//ife_eng_cal_hv_stripe(io_cfg->in_frm.size.w, io_cfg->in_frm.size.h, &stripe_info);

	stripe_info.ife_stripe_num = io_cfg->ife_strp_num;
	stripe_info.ife_stripe_overlap = io_cfg->ife_ovlp;

	for(i=0; i < KDRV_IFE_STRP_NUM_MAX; i++){
		stripe_info.ife_stripe_w[i] = io_cfg->ife_strp[i];
	}

	ife_eng_set_stripe_buf_reg(p_hdl->p_eng, stripe_info);

	// set in/out buffer address
	if (op_mode != IFE_OPMODE_ALL_DIRECT) {

		UINT64 addr = io_cfg->in_frm.phyaddr[0];

		ife_eng_set_dma_in_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF), io_cfg->in_frm.lofs[0]);
		//ife_eng_set_dma_in_addr_ofs_buf_reg(p_hdl->p_eng, io_cfg->in_frm.addr_ofs[0]);
		//ife_eng_set_hshift_buf_reg(p_hdl->p_eng, 0); //690 removed
	}else{

		if(io_cfg->combine_param.combine_mode_en){

			sie_handshake.input_combine_en = 1;

		}else{
			sie_handshake.input_combine_en = 0;
		}

		//ife_eng_set_sie_handshake_buf_reg(p_hdl->p_eng, &sie_handshake);
	}

	if ((KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX)
			&& kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt) == 1) { // fnum == 1: 2 frame fusion enable, fnum == 0: fusion disable

		UINT64 addr = io_cfg->in_frm.phyaddr[1];
		//UINT64 addr2 = io_cfg->in_frm.phyaddr[2];

		ife_eng_set_dma_in_addr_2_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF), io_cfg->in_frm.lofs[1]);
		//ife_eng_set_dma_in_addr_2_ofs_buf_reg(p_hdl->p_eng, io_cfg->in_frm.addr_ofs[1]);

		// fpn input address
		//ife_eng_set_dma_in_addr_3_buf_reg(p_hdl->p_eng, (UINT32)(addr2 & 0xFFFFFFFF), (UINT32)((addr2 >> 32) & 0xFFFFFFFF), io_cfg->in_frm.lofs[2]);
		//ife_eng_set_dma_in_addr_3_ofs_buf_reg(p_hdl->p_eng, io_cfg->in_frm.addr_ofs[2]);
		//DBG_ERR("shdr2 fpn addr 0x%llx lofs %u\r\n", addr2, io_cfg->in_frm.lofs[2]);

		if(io_cfg->in_frm.addr[1])
			ife_eng_set_fusion_en_buf_reg(p_hdl->p_eng, ENABLE);
		else
			DBG_ERR("fusion input addr not alloc\n");

	}
	// 538 not support 3 frame
	else if((KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX)
			&& kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt) == 2) { // fnum == 2: 3 frame fusion enable, fnum == 0: fusion disable

		UINT64 addr = io_cfg->in_frm.phyaddr[1];
		//UINT64 addr2 = io_cfg->in_frm.phyaddr[2];

		ife_eng_set_dma_in_addr_2_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF), io_cfg->in_frm.lofs[1]);
		//ife_eng_set_dma_in_addr_2_ofs_buf_reg(p_hdl->p_eng, io_cfg->in_frm.addr_ofs[1]);

		//ife_eng_set_dma_in_addr_3_buf_reg(p_hdl->p_eng, (UINT32)(addr2 & 0xFFFFFFFF), (UINT32)((addr2 >> 32) & 0xFFFFFFFF), io_cfg->in_frm.lofs[2]);
		//ife_eng_set_dma_in_addr_3_ofs_buf_reg(p_hdl->p_eng, io_cfg->in_frm.addr_ofs[2]);

		if(io_cfg->in_frm.addr[1] && io_cfg->in_frm.addr[2])
			ife_eng_set_fusion_en_buf_reg(p_hdl->p_eng, ENABLE);
		else
			DBG_ERR("fusion input addr not alloc\n");

	}
	else if( (KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX)
			&& kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt) == 0){

		UINT64 addr2 = io_cfg->in_frm.phyaddr[2];

		// fpn input address
		ife_eng_set_dma_in_addr_2_buf_reg(p_hdl->p_eng, (UINT32)(addr2 & 0xFFFFFFFF), (UINT32)((addr2 >> 32) & 0xFFFFFFFF), io_cfg->in_frm.lofs[2]);
		//ife_eng_set_dma_in_addr_3_ofs_buf_reg(p_hdl->p_eng, io_cfg->in_frm.addr_ofs[2]);
		//DBG_ERR("fpn addr 0x%llx lofs %u\r\n", addr2, io_cfg->in_frm.lofs[2]);

		ife_eng_set_fusion_en_buf_reg(p_hdl->p_eng, DISABLE);

	}else if(KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_YUV){

		UINT64 addr = io_cfg->in_frm.phyaddr[1];

		ife_eng_set_dma_in_addr_2_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr>> 32) & 0xFFFFFFFF), io_cfg->in_frm.lofs[1]);
		//ife_eng_set_dma_in_addr_2_ofs_buf_reg(p_hdl->p_eng, io_cfg->in_frm.addr_ofs[1]);

	}else{
		DBG_DUMP("not raw hdr fmt:%x\n",io_cfg->in_frm.fmt);
	}


	if (op_mode == IFE_OPMODE_D2D) {

		UINT64 addr = io_cfg->out_frm.phyaddr[0];

		ife_eng_set_out_sel_buf_reg(p_hdl->p_eng,KDRV_IFE_WDR_OUT);

		ife_eng_set_dma_out_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF), io_cfg->out_frm.lofs[0]);
		//ife_eng_set_dma_out_addr_ofs_buf_reg(p_hdl->p_eng, io_cfg->out_frm.addr_ofs[0]);
		ife_eng_set_dma_out_en_buf_reg(p_hdl->p_eng, ENABLE);

		if(KDRV_IPP_FMT_CLASS(io_cfg->out_frm.fmt) == KDRV_IPP_FMT_CLASS_YUV){

			UINT64 addr = io_cfg->out_frm.phyaddr[1];

			ife_eng_set_dma_out_uv_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF), io_cfg->out_frm.lofs[1]);
			//ife_eng_set_dma_out_uv_addr_ofs_buf_reg(p_hdl->p_eng, io_cfg->out_frm.addr_ofs[1]);
		}
	}else{

		ife_eng_set_out_sel_buf_reg(p_hdl->p_eng, KDRV_IFE_WDR_OUT);
		ife_eng_set_dma_out_en_buf_reg(p_hdl->p_eng, DISABLE);
	}

	if(io_cfg->subisp_param.subisp_out_en){
		IFE_ENG_SUBISP_CROP subisp_crop = {0};
		IFE_ENG_SUBISP_SUBSAMPLE subisp_subsample = {0};

		ife_eng_set_yuv_out_fmt_buf_reg(p_hdl->p_eng, kdrv_ife_typecast_yuv_out_fmt(io_cfg->subisp_param.subisp_frm.fmt));

		{
			UINT64 addr = io_cfg->subisp_param.subisp_frm.phyaddr[0];

			ife_eng_set_dma_out_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF), io_cfg->subisp_param.subisp_frm.lofs[0]);

			addr = io_cfg->subisp_param.subisp_frm.phyaddr[1];
			ife_eng_set_dma_out_uv_addr_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF), io_cfg->subisp_param.subisp_frm.lofs[1]);

			//ife_eng_set_dma_out_addr_ofs_buf_reg(p_hdl->p_eng, io_cfg->subisp_param.subisp_frm.addr_ofs[0]);
			//ife_eng_set_dma_out_uv_addr_ofs_buf_reg(p_hdl->p_eng, io_cfg->subisp_param.subisp_frm.addr_ofs[1]);
		}

		ife_eng_set_out_sel_buf_reg(p_hdl->p_eng, KDRV_IFE_SUBISP_OUT);
		ife_eng_set_dma_out_en_buf_reg(p_hdl->p_eng, ENABLE);

		subisp_crop.crop_height = io_cfg->subisp_param.crop_height;
		subisp_crop.crop_width = io_cfg->subisp_param.crop_width;;

		ife_eng_set_subisp_crop_buf_reg(p_hdl->p_eng, subisp_crop);

		subisp_subsample.sample_rate_x = io_cfg->subisp_param.subsample_rate_x;
		subisp_subsample.sample_rate_y = io_cfg->subisp_param.subsample_rate_y;

		ife_eng_set_subisp_subsample_buf_reg(p_hdl->p_eng, subisp_subsample);

	}

	// set in/out image format

	if(KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt ) == KDRV_IPP_FMT_CLASS_YUV){

		IFE_ENG_YUV_IN_PARAM yuv_in_param = {0};

		ife_eng_set_in_fmt_buf_reg(p_hdl->p_eng, IFE_YUV_IN);

		yuv_in_param.yuv_in_fmt = kdrv_ife_typecast_yuv_fmt(io_cfg->in_frm.fmt);

		if(yuv_in_param.yuv_in_fmt == IFE_ENG_FMT_Y_UNKNOWN){
			return E_PAR;;
		}

		yuv_in_param.yuv2rgb_enable = ENABLE;

		yuv_in_param.yuv2rgb_fmt = kdrv_ife_typecast_yuv2rgb(io_cfg->in_frm.pxl_fmt);

		if(yuv_in_param.yuv2rgb_fmt == IFE_ENG_UNKNOWN_YUV){
			return E_PAR;;
		}

		yuv_in_param.yuv_in_random_lsb_enable = io_cfg->yuv_in_info.random_lsb_enable;
		yuv_in_param.yuv_in_random_seed_rst = io_cfg->yuv_in_info.random_reset;
		yuv_in_param.yuv_in_uv_lpf_enable = io_cfg->yuv_in_info.uv_lpf_enable;
		yuv_in_param.yuv_in_io_stop = 0;

		ife_eng_set_yuv_d2d_in_buf_reg(p_hdl->p_eng, &yuv_in_param);

	}else if(KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt ) == KDRV_IPP_FMT_CLASS_RAW ||
			KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt ) == KDRV_IPP_FMT_CLASS_NRX ){

		ife_eng_set_in_fmt_buf_reg(p_hdl->p_eng, IFE_RAW_IN);

		bayer_fmt_sel = kdrv_ife_typecast_bayer_fmt(io_cfg->in_frm.fmt);
		if( (bayer_fmt_sel == IFE_BAYER_RGGB) || (bayer_fmt_sel == IFE_BAYER_RGBIR))
			ife_eng_set_bayer_fmt_buf_reg(p_hdl->p_eng, bayer_fmt_sel);
		else
			ife_eng_set_bayer_fmt_buf_reg(p_hdl->p_eng, IFE_BAYER_RGGB);

		ife_eng_set_cfa_pat_buf_reg(p_hdl->p_eng, kdrv_ife_typecast_pix(io_cfg->in_frm.fmt));
		ife_eng_set_cfa_pat_2_buf_reg(p_hdl->p_eng, kdrv_ife_typecast_pix(io_cfg->in_frm.pxl_fmt));
		ife_eng_set_bitsel_buf_reg(p_hdl->p_eng, kdrv_ife_typecast_rawbit(io_cfg->in_frm.fmt), kdrv_ife_typecast_rawbit(io_cfg->out_frm.fmt));
		if (KDRV_IPP_FMT_BPP(io_cfg->in_frm.fmt) == 16 && (io_cfg->in_frm.fmt & KDRV_IPP_FMT_PACK_LSB)) { // select LSB-bit when 16-bits input format
			ife_eng_set_16bit_fmt_sel_reg(p_hdl->p_eng, 1);
		} else { // select MSB-bit
			ife_eng_set_16bit_fmt_sel_reg(p_hdl->p_eng, 0);
		}

	}else{
		DBG_ERR("unknown input fmt:0x%x\n", io_cfg->in_frm.fmt);
	}


	if(p_hdl->ife_dbg_ctl.ife_dbg_fun & KDRV_IFE_DBG_STATUS_REC)  // enable ife frm end and ll end for ife debug mode (to record ife 0x68 status)
		io_cfg->inte_en |= (KDRV_IFE_INTERRUPT_FMD | KDRV_IFE_INTERRUPT_LLEND);

	// set interrupt enable
	//ife_eng_set_enable_int_buf_reg(p_hdl->p_eng, io_cfg->inte_en);
	ife_eng_set_enable_int_hw_reg(p_hdl->p_eng, io_cfg->inte_en); // (690) d2d link-list mode patch. directly set inte_en reg by cpu, to prevent sie frame start interrupts ll cmd transfer to hw reg

	// set fusion number. yuv format should be 0
	if (KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_RAW || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX) {
		fnum = kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt);
	} else {
		fnum = 0;
	}

	if(fnum != -1)
		ife_eng_set_fnum_buf_reg(p_hdl->p_eng, fnum);
	else
		ife_eng_set_fnum_buf_reg(p_hdl->p_eng, 0);

	// set mirror
	ife_eng_set_mirror_en_buf_reg(p_hdl->p_eng, io_cfg->mirror.en);

	// set thermal_mode
	{
		if(io_cfg->thermal_mode_en){
			IFE_FUSIONSET fusion = {0};
			fusion.ev_ratio = iq_cfg->fusion.fu_ctrl.ev_ratio;
			ife_eng_set_thermal_mode_buf_reg(p_hdl->p_eng, io_cfg->thermal_mode_en);
			ife_eng_set_fusion_ctrl_buf_reg(p_hdl->p_eng, &fusion);
		}
	}

	// set rde
	if (KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX) { // class == nrx: r_encode enable
		IFE_RDESET rde = {0};

		rde.encode_rate = io_cfg->rde.encode_rate;
		ife_eng_set_rde_default(p_hdl->p_eng, &rde, kdrv_ife_typecast_adj_mode(io_cfg->rde.encode_adjmode));

#if 0
		if(io_cfg->rde.encode_rate == KDRV_IFE_ENCODE_RATE_41){
			IFE_RDESET rde_ctrl = {0};

			rde_ctrl.b_degamma_en = 1;
			rde_ctrl.b_dithering_en = 1;
			rde_ctrl.b_index_en = 1;
			rde_ctrl.dither_reset = 0;

			ife_eng_set_rde_ctrl_buf_reg(p_hdl->p_eng, &rde_ctrl);
		}
		else{
			IFE_RDESET rde_ctrl = {0};

			rde_ctrl.b_degamma_en = 1;
			rde_ctrl.b_dithering_en = 1;
			rde_ctrl.b_index_en = 0;
			rde_ctrl.dither_reset = 0;

			ife_eng_set_rde_ctrl_buf_reg(p_hdl->p_eng, &rde_ctrl);
		}

		ife_eng_set_rde_dpcmtable_inx_buf_reg(p_hdl->p_eng,);
#endif
		if (kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt) == 0){
			ife_eng_set_decode_en_buf_reg(p_hdl->p_eng, ENABLE);
			ife_eng_set_decode2_en_buf_reg(p_hdl->p_eng, DISABLE);
		} else if (kdrv_ife_typecast_fnum(io_cfg->in_frm.fmt) == 1){
			ife_eng_set_decode_en_buf_reg(p_hdl->p_eng, ENABLE);
			ife_eng_set_decode2_en_buf_reg(p_hdl->p_eng, ENABLE);
		}
	} else {
		ife_eng_set_decode_en_buf_reg(p_hdl->p_eng, DISABLE);
		ife_eng_set_decode2_en_buf_reg(p_hdl->p_eng, DISABLE);
	}

	if (op_mode == IFE_OPMODE_ALL_DIRECT){	//direct mode must enable decode
		IFE_RDESET rde = {0};

		rde.encode_rate = io_cfg->rde.encode_rate;
		ife_eng_set_rde_default(p_hdl->p_eng, &rde, kdrv_ife_typecast_adj_mode(io_cfg->rde.encode_adjmode));
		ife_eng_set_decode_en_buf_reg(p_hdl->p_eng, ENABLE);

	}

	// set ring buffer
	ring_info.dmaloop_en = io_cfg->ring_buf.ring[0].en;
	ring_info.dmaloop_line = io_cfg->ring_buf.ring[0].line_num;
	ife_eng_set_ring_buf_chk_buf_reg(p_hdl->p_eng, &ring_info);

	ring_info.dmaloop_ctrl = io_cfg->ring_buf.imm_start;
	ife_eng_set_ring_buf_ctrl_buf_reg(p_hdl->p_eng, &ring_info);

	// set vig center
	for (i = 0; i < KDRV_IFE_VIG_CH_MAX; i++) {
		vig_x[i] = io_cfg->vig_pos.center[i].x;
		vig_y[i] = io_cfg->vig_pos.center[i].y;
	}
	vig_set.p_vig_x = &vig_x[0];
	vig_set.p_vig_y = &vig_y[0];
	ife_eng_set_vignette_center_buf_reg(p_hdl->p_eng, &vig_set);

	// set ife hdr reference channel
	{
		IFE_DICH_SYNC_BUF dich= {0};

		dich.dich_line_ctrl = 0;

		if(p_hdl->ife_dbg_ctl.ife_dbg_fun & KDRV_IFE_DBG_SET_FUSION_REF){

			if(p_hdl->ife_dbg_ctl.hdr_ref_chk & KDRV_IFE_HDR_REF_SIE1)
				dich.dma_sync_dich_line_dis = DISABLE;
			else
				dich.dma_sync_dich_line_dis = ENABLE;

			if(p_hdl->ife_dbg_ctl.hdr_ref_chk & KDRV_IFE_HDR_REF_SIE2)
				dich.dma1_wait_sie2_start_dis = DISABLE;
			else
				dich.dma1_wait_sie2_start_dis = ENABLE;

			ife_eng_set_dich_line_buf_reg(p_hdl->p_eng, &dich);
			ife_eng_set_sync_dich_disable_buf_reg(p_hdl->p_eng, &dich);
			ife_eng_set_dma_wait_sie2_start_buf_reg(p_hdl->p_eng, &dich);

		}else{

			dich.dich_line_ctrl = 0;

			if(io_cfg->hdr_ref_chk & KDRV_IFE_HDR_REF_SIE1)
				dich.dma_sync_dich_line_dis = DISABLE;
			else
				dich.dma_sync_dich_line_dis = ENABLE;

			if(io_cfg->hdr_ref_chk & KDRV_IFE_HDR_REF_SIE2)
				dich.dma1_wait_sie2_start_dis = DISABLE;
			else
				dich.dma1_wait_sie2_start_dis = ENABLE;

			ife_eng_set_dich_line_buf_reg(p_hdl->p_eng, &dich);
			ife_eng_set_sync_dich_disable_buf_reg(p_hdl->p_eng, &dich);
			ife_eng_set_dma_wait_sie2_start_buf_reg(p_hdl->p_eng, &dich);
		}
	}

	// set NN ISP settings


	// set IQ settings
    /************************* SubISP param *************************/
    if (iq_cfg->update & KDRV_IFE_UPDATE_SUBISP) {

		IFE_ENG_SUBISP_GAMMA subisp_gamma = {0};
		IFE_ENG_SUBISP_CST subisp_cst = {0};
		IFE_ENG_SUBISP_CCM subisp_ccm = {0};

		subisp_gamma.gamma_en = iq_cfg->subisp_iq.subisp_gamma_en;
		subisp_gamma.p_gamma_lut = &iq_cfg->subisp_iq.subisp_gamma_lut[0];
		ife_eng_set_subisp_gamma_buf_reg(p_hdl->p_eng, subisp_gamma);


		subisp_cst.cst_en = iq_cfg->subisp_iq.subisp_cst_en;
		//subisp_cst.p_cst_coef = (INT16 *)&iq_cfg->subisp_iq.coef[0];  // not need to set coef
		subisp_ccm.p_ccm_coef = (INT16 *)&iq_cfg->subisp_iq.coef[0];
		ife_eng_set_subisp_cst_buf_reg(p_hdl->p_eng, subisp_cst);
		ife_eng_set_subisp_ccm_buf_reg(p_hdl->p_eng, subisp_ccm); // color correction matri

		ife_eng_set_subisp_cfa_buf_reg(p_hdl->p_eng, iq_cfg->subisp_iq.subisp_cfa_en);
    }

    /************************* WDR param *************************/
	/* no need to check KDRV_IFE_UPDATE_WDR_SUB for subout */

    if (iq_cfg->update & KDRV_IFE_UPDATE_WDR) {

        IFE_ENG_WDR_PARAM wdr_param = {0};
        UINT8 wdr_lpf[WDR_SUBIMG_FILT_NUM] = {0};
        UINT8 wdr_bld_lut[WDR_INPUT_BLD_NUM] = {0};

        ife_eng_set_wdr_tone_curve_enable_buf_reg(p_hdl->p_eng, iq_cfg->wdr.tonecurve_enable);
        ife_eng_set_wdr_output_blend_enable_buf_reg(p_hdl->p_eng, iq_cfg->wdr.outbld.outbld_en);

        for (i = 0; i < WDR_SUBIMG_FILT_NUM; i++)
            wdr_lpf[i] = iq_cfg->wdr.ftrcoef[i];
        //wdr_param.wdr_subimg.p_lpf_coef = wdr_lpf;
        ife_eng_set_wdr_subimg_lpf_coeff_buf_reg(p_hdl->p_eng, wdr_lpf);

		wdr_param.dither.wdr_dithering_en = iq_cfg->wdr.dither.wdr_dithering_en; // 538 fixed
        wdr_param.dither.wdr_rand_rst = iq_cfg->wdr.dither.wdr_rand_rst;
        wdr_param.dither.wdr_rand_sel = iq_cfg->wdr.dither.wdr_rand_sel;

        for (i = 0 ; i <  WDR_INPUT_BLD_NUM; i++)
            wdr_bld_lut[i] = iq_cfg->wdr.input_bld.blend_lut[i];

        for (i = 0 ; i <  WDR_INPUT_YV_BLD_NUM; i++)
            wdr_param.wdr_input_bld.wdr_input_yv_bld_lut[i] = iq_cfg->wdr.input_bld.in_yv_blend_lut[i];

        wdr_param.wdr_input_bld.p_inblend_lut = wdr_bld_lut;
        wdr_param.wdr_input_bld.wdr_bld_sel = iq_cfg->wdr.input_bld.bld_sel;
        //wdr_param.wdr_input_bld.wdr_bld_wt = iq_param->wdr_param.input_bld.bld_wt; //removed from 530

        for (i = 0; i< WDR_COEF_NUM; i++)
            wdr_param.wdr_strength.wdr_coeff[i] = iq_cfg->wdr.wdr_str.wdr_coeff[i];

        wdr_param.wdr_strength.strength = iq_cfg->wdr.wdr_str.strength;
        wdr_param.wdr_strength.wdr_mode  =iq_cfg->wdr.wdr_str.wdr_mode;
		wdr_param.wdr_strength.wdr_anti_halo_opt=iq_cfg->wdr.wdr_str.wdr_anti_halo_opt;
		wdr_param.wdr_strength.wdr_halo_ratio=iq_cfg->wdr.wdr_str.wdr_halo_ratio;
		wdr_param.wdr_strength.wdr_halo_slope=iq_cfg->wdr.wdr_str.wdr_halo_slope;
		wdr_param.wdr_strength.wdr_b2p_var=iq_cfg->wdr.wdr_str.wdr_b2p_var;

		//// WDR FBC
		wdr_param.wdr_fbc.fbc_en = iq_cfg->wdr.fbc.fbc_en;
		wdr_param.wdr_fbc.fbc_ratio = iq_cfg->wdr.fbc.fbc_ratio;
		for (i=0; i < WDR_FBC_TH_NUM; i++){
			wdr_param.wdr_fbc.fbc_th[i] = iq_cfg->wdr.fbc.fbc_th[i];
		}

        wdr_param.wdr_gainctrl.gainctrl_en = iq_cfg->wdr.gainctrl.gainctrl_en;
        wdr_param.wdr_gainctrl.max_gain = iq_cfg->wdr.gainctrl.max_gain;
        wdr_param.wdr_gainctrl.min_gain = iq_cfg->wdr.gainctrl.min_gain;

        wdr_param.wdr_sat_reduct.sat_th = iq_cfg->wdr.sat_reduct.sat_th;
        wdr_param.wdr_sat_reduct.sat_wt_low = iq_cfg->wdr.sat_reduct.sat_wt_low;
        wdr_param.wdr_sat_reduct.sat_delta = iq_cfg->wdr.sat_reduct.sat_delta;
        wdr_param.wdr_gain_port_str = iq_cfg->wdr.wdr_gain_prot_str;

        ife_eng_set_wdr_buf_reg(p_hdl->p_eng, &wdr_param);
        ife_eng_set_wdr_in_blend_ratio_buf_reg(p_hdl->p_eng, &wdr_param);


		if (io_cfg->wdr_sub_in_addr.pa != 0 && io_cfg->wdr_sub_in_addr.pa != (ULONG)-1) {

			UINT64 addr = io_cfg->wdr_sub_in_addr.pa;

			//538 added
			io_cfg->subimg_lofs_in = iq_cfg->wdr_subimg.subimg_size_h << 3;
		    ife_eng_set_wdr_sub_in_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF),  (UINT32)( (addr>> 32) & 0xFFFFFFFF), io_cfg->subimg_lofs_in);//iq_cfg->wdr_subimg.subimg_lofs_in
		}else{
			static UINT32 cnt = 0;
			DBG_WRN("wdr_subin_addr:0x%lx invalid(cnt=%d)\n",io_cfg->wdr_sub_in_addr.pa,cnt);
			cnt++;
		}

        if(iq_cfg->wdr.wdr_enable == TRUE){

            ife_eng_set_wdr_enable_buf_reg(p_hdl->p_eng, TRUE);

         }else{
             ife_eng_set_wdr_enable_buf_reg(p_hdl->p_eng, FALSE);
         }

        if(iq_cfg->hist.hist_enable == TRUE){

			hist_param.histogram_enable = TRUE;
            hist_param.hist_sel = iq_cfg->hist.hist_sel;
            hist_param.hist_step_h = iq_cfg->hist.step_h;
            hist_param.hist_step_v = iq_cfg->hist.step_v;
            //hist_param.hist_shift  = iq_cfg->hist_param.hist_shift;

            ife_eng_set_wdr_histo_buf_reg(p_hdl->p_eng, &hist_param);

        }else{
			IFE_ENG_WDR_HIST hist_param = {0};

			hist_param.histogram_enable = FALSE;
            hist_param.hist_sel = iq_cfg->hist.hist_sel;
            hist_param.hist_step_h = iq_cfg->hist.step_h;
            hist_param.hist_step_v = iq_cfg->hist.step_v;

            ife_eng_set_wdr_histo_buf_reg(p_hdl->p_eng, &hist_param);
        }

    }

	/*------------------- Set dual hist statistic out--------------------*/
    if(io_cfg->dual_en){
#if(IFE_538_KDRV_FEATURE == 0)
        dual_param.left_overlap = io_cfg->dual_left_ovlp;
        dual_param.right_overlap = io_cfg->dual_right_ovlp;
        dual_param.start_offset = io_cfg->dual_start_ofs;
        ife_eng_cal_dual_wdr_hist_next_hit(&dual_param, &hist_param, io_cfg->dual_idx);
        ife_eng_set_hist_dual_buf_reg(p_hdl->p_eng, &hist_param);
#endif
    }

	/*------------------- Set WDR Subimg Out --------------------*/
	if (iq_cfg->update & KDRV_IFE_UPDATE_WDR_SUB || io_cfg->wdr_sub_out_en) {
		UINT32 subout_sizeh, subout_sizev;
        IFE_ENG_WDR_SUBIMG wdr_subimg = {0};

		subout_sizeh = iq_cfg->wdr_subimg.subimg_size_h;
		subout_sizev = iq_cfg->wdr_subimg.subimg_size_v;

		if (subout_sizeh <  8) {
			subout_sizeh = 8;
			DBG_ERR("subimg size h cannot less than 8, modify to 8\r\n");
		}
		if (subout_sizev < 8) {
			subout_sizev = 8;
			DBG_ERR("subimg size v cannot less than 8, modify to 8\r\n");
		}
	    wdr_subimg.wdr_subimg_width = subout_sizeh;
	    wdr_subimg.wdr_subimg_height = subout_sizev;

		wdr_subimg.subin_scal_fact.h_fact = (/*(io_cfg->dual_left_width + io_cfg->dual_right_width)*/ io_cfg->in_frm.size.w <<6) / wdr_subimg.wdr_subimg_width;
		wdr_subimg.subin_scal_fact.v_fact = (io_cfg->crp_window.h <<6) / wdr_subimg.wdr_subimg_height;

		wdr_subimg.wdr_subimg_div.wdr_subimg_hori_div = (wdr_subimg.wdr_subimg_width<<16) / /*(io_cfg->dual_left_width + io_cfg->dual_right_width)*/io_cfg->in_frm.size.w;
		wdr_subimg.wdr_subimg_div.wdr_subimg_vert_div = (wdr_subimg.wdr_subimg_height<<16) / io_cfg->crp_window.h;

		ife_eng_set_wdr_subimg_param_reg(p_hdl->p_eng, &wdr_subimg);

	}

    if(io_cfg->wdr_sub_out_en)
    {
        //IFE_ENG_DUAL_PARAM dual_param = {0};

		if (io_cfg->wdr_sub_out_addr.pa != 0 && io_cfg->wdr_sub_out_addr.pa != (ULONG)-1) {

			UINT64 addr = io_cfg->wdr_sub_out_addr.pa;

			//538 added
			io_cfg->subimg_lofs_out = iq_cfg->wdr_subimg.subimg_size_h << 3;
		    ife_eng_set_wdr_sub_out_buf_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)((addr >> 32) & 0xFFFFFFFF), io_cfg->subimg_lofs_out);//iq_cfg->wdr_subimg.subimg_lofs_out
		}else{
			DBG_ERR("wdr_subout_addr:0x%lx invalid\n",io_cfg->wdr_sub_out_addr.pa);
			return E_PAR;
		}

		if(io_cfg->dual_en){
#if(IFE_538_KDRV_FEATURE == 0)
			dual_param.right_overlap = io_cfg->dual_right_ovlp;
			dual_param.left_overlap = io_cfg->dual_left_ovlp;
			dual_param.start_offset = io_cfg->dual_start_ofs;
			ife_eng_cal_dual_wdr_subout_block_st_end(&dual_param, &wdr_subimg, io_cfg->in_frm.size.w, io_cfg->wdr_sub_out_en, io_cfg->dual_idx);
			ife_eng_set_wdr_dual_buf_reg(p_hdl->p_eng, &wdr_subimg);
		}
		else{
			wdr_subimg.dual_set.subimg_end_blk = 0;
			wdr_subimg.dual_set.subimg_limit_en = 0;
			wdr_subimg.dual_set.subimg_st_blk = 0;
			ife_eng_set_wdr_dual_buf_reg(p_hdl->p_eng, &wdr_subimg);
#endif
		}
    }
	ife_eng_set_wdr_subout_enable_buf_reg(p_hdl->p_eng, io_cfg->wdr_sub_out_en);

    if (iq_cfg->update  & KDRV_IFE_UPDATE_WDR_TONE_CRV) {

		if (iq_cfg->wdr.tonecurve_enable == TRUE) {

			ife_eng_set_wdr_tone_curve_buf_l_reg(p_hdl->p_eng, &iq_cfg->wdr.tonecurve.left_table[0]);
			ife_eng_set_wdr_tone_curve_buf_r_reg(p_hdl->p_eng, &iq_cfg->wdr.tonecurve.right_table[0]);
        }
    }
    if (iq_cfg->update  & KDRV_IFE_UPDATE_WDR_BLD_LUT) {
        if (iq_cfg->wdr.outbld.outbld_en == TRUE) {

			ife_eng_set_wdr_output_blend_buf_l_reg(p_hdl->p_eng, &iq_cfg->wdr.outbld.outbld_lut.left_table[0]);
            ife_eng_set_wdr_output_blend_buf_r_reg(p_hdl->p_eng, &iq_cfg->wdr.outbld.outbld_lut.right_table[0]);
        }
    }

	// - nrs
	if (iq_cfg->update & KDRV_IFE_UPDATE_NRS0) {
		if (iq_cfg->nrs0.enable) {
			IFE_NRSSET nrs = {0};
			nrs.ife_f_nrs0_str = &iq_cfg->nrs0.str[0];
			ife_eng_set_nrs0_str_buf_reg(p_hdl->p_eng, &nrs);
			ife_eng_set_nrs0_en_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ife_eng_set_nrs0_en_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	// - fcurve
	if (iq_cfg->update & KDRV_IFE_UPDATE_FCURVE) {
		if (iq_cfg->fcurve.enable) {
			IFE_FCURVESET fcurve = {0};
			fcurve.y_mean_select = iq_cfg->fcurve.fcur_ctrl.y_mean_sel;
			fcurve.yv_weight     = iq_cfg->fcurve.fcur_ctrl.yv_w;
			fcurve.ev_format     = iq_cfg->fcurve.fcur_ctrl.ev_fmt;
			fcurve.p_y_weight_lut = &iq_cfg->fcurve.y_weight.y_w_lut[0];
			fcurve.p_left_lut   = &iq_cfg->fcurve.fcurve_l.fcur_l_lut[0];
			fcurve.p_right_lut   = &iq_cfg->fcurve.fcurve_r.fcur_r_lut[0];
			//fcurve.p_end_lut = &iq_cfg->fcurve.fcurve_end.fcur_end_lut[0];

			ife_eng_set_fcurve_ctrl_buf_reg(p_hdl->p_eng, &fcurve);
			ife_eng_set_fcurve_yweight_buf_reg(p_hdl->p_eng, &fcurve);

			ife_eng_set_fcurve_l_buf_reg(p_hdl->p_eng, &fcurve);
			ife_eng_set_fcurve_r_buf_reg(p_hdl->p_eng, &fcurve);
			//ife_eng_set_fcurve_end_buf_reg(p_hdl->p_eng, &fcurve);

			/* NT98530 removed
			fcurve.p_index_lut = &iq_cfg->fcurve.index.idx_lut[0];
			fcurve.p_split_lut = &iq_cfg->fcurve.split.split_lut[0];
			fcurve.p_value_lut = &iq_cfg->fcurve.value.val_lut[0];
			ife_eng_set_fcurve_index_buf_reg(&fcurve);
			ife_eng_set_fcurve_split_buf_reg(&fcurve);
			ife_eng_set_fcurve_value_buf_reg(&fcurve);
			*/
			ife_eng_set_fcurve_en_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ife_eng_set_fcurve_en_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	// - fusion
	if (iq_cfg->update & KDRV_IFE_UPDATE_FUSION) {

			IFE_FUSIONSET fusion = {0};

			fusion.y_mean_sel = iq_cfg->fusion.fu_ctrl.y_mean_sel;
			fusion.nor_blendcur_sel = iq_cfg->fusion.bld_cur.nor_sel;
			fusion.diff_blendcur_sel = iq_cfg->fusion.bld_cur.dif_sel;

			if(p_hdl->ife_dbg_ctl.ife_dbg_fun & KDRV_IFE_DBG_SET_HDR_OUT_MODE){ // debug mode for ife output only short or long exposure frame when HDR turn on
				fusion.mode = (UINT8)p_hdl->ife_dbg_ctl.ife_fusion_out_mode;
			}
			else{
				fusion.mode = iq_cfg->fusion.fu_ctrl.mode;
			}

			fusion.ev_ratio = iq_cfg->fusion.fu_ctrl.ev_ratio;
			//538 remove
			//fusion.ev_ratio_1 = iq_cfg->fusion.fu_ctrl.ev_ratio_1;
			fusion.mc_lum_th = iq_cfg->fusion.mc_para.lum_th;
			fusion.mc_diff_ratio = iq_cfg->fusion.mc_para.diff_ratio;
			fusion.p_mc_diff_w = &iq_cfg->fusion.mc_para.diff_w[0];
			//98530 remove
			//fusion.p_mc_pos_diff_w = &iq_cfg->fusion.mc_para.pos_diff_w[0];
			//fusion.p_mc_neg_diff_w = &iq_cfg->fusion.mc_para.neg_diff_w[0];
			fusion.mc_diff_lumth_diff_w = iq_cfg->fusion.mc_para.dwd;
			fusion.long_nor_blendcur_knee = iq_cfg->fusion.bld_cur.l_nor_knee[0];
			fusion.long_nor_blendcur_range = iq_cfg->fusion.bld_cur.l_nor_range;
			fusion.long_nor_blendcur_slope = iq_cfg->fusion.bld_cur.l_nor_slope;

			fusion.short_nor_blendcur_knee = iq_cfg->fusion.bld_cur.s_nor_knee[0];
			fusion.short_nor_blendcur_range = iq_cfg->fusion.bld_cur.s_nor_range;
			fusion.short_nor_blendcur_slope = iq_cfg->fusion.bld_cur.s_nor_slope;
			//fusion.very_short_nor_blendcur_knee = iq_cfg->fusion.bld_cur.vs_nor_knee[0];
			//fusion.very_short_nor_blendcur_range = iq_cfg->fusion.bld_cur.vs_nor_range;
			//fusion.very_short_nor_blendcur_slope = iq_cfg->fusion.bld_cur.vs_nor_slope;

			fusion.long_diff_blendcur_knee = iq_cfg->fusion.bld_cur.l_dif_knee[0];
			fusion.long_diff_blendcur_range = iq_cfg->fusion.bld_cur.l_dif_range;
			fusion.long_diff_blendcur_slope = iq_cfg->fusion.bld_cur.l_dif_slope;

			fusion.short_diff_blendcur_knee = iq_cfg->fusion.bld_cur.s_dif_knee[0];
			fusion.short_diff_blendcur_range = iq_cfg->fusion.bld_cur.s_dif_range;
			fusion.short_diff_blendcur_slope = iq_cfg->fusion.bld_cur.s_dif_slope;
			//fusion.very_short_diff_blendcur_knee = iq_cfg->fusion.bld_cur.vs_dif_knee[0];
			//fusion.very_short_diff_blendcur_range = iq_cfg->fusion.bld_cur.vs_dif_range;
			//fusion.very_short_diff_blendcur_slope = iq_cfg->fusion.bld_cur.vs_dif_slope;

			fusion.p_dark_sat_reduce_th = &iq_cfg->fusion.dk_sat.th[0];
			fusion.p_dark_sat_reduce_step = &iq_cfg->fusion.dk_sat.step[0];
			fusion.p_dark_sat_reduce_lowbound = &iq_cfg->fusion.dk_sat.low_bound[0];
			//ife_eng_set_fusion_debug_buf_reg(&fusion);
			ife_eng_set_fusion_ctrl_buf_reg(p_hdl->p_eng, &fusion);
			ife_eng_set_fusion_blend_curve(p_hdl->p_eng, &fusion);
			ife_eng_set_fusion_diff_weight_buf_reg(p_hdl->p_eng, &fusion);
			ife_eng_set_fusion_dark_saturation_reduction_buf_reg(p_hdl->p_eng, &fusion);

			// NT98530 remove
			/*
			if (iq_cfg->fusion.s_comp.en) {
				fusion.p_short_comp_knee_point = &iq_cfg->fusion.s_comp.knee[0];
				fusion.p_short_comp_sub_point = &iq_cfg->fusion.s_comp.sub_point[0];
				fusion.p_short_comp_shift_bit = &iq_cfg->fusion.s_comp.shift[0];
				ife_eng_set_fusion_short_exposure_compress_buf_reg(&fusion);

				ife_eng_set_scompression_en_buf_reg(ENABLE);
			} else {
				ife_eng_set_scompression_en_buf_reg(DISABLE);
			}*/


			if (iq_cfg->fusion.fu_cgain.enable) {
				IFE_FCGAINSET fcgain = {0};
				fcgain.fcgain_range = iq_cfg->fusion.fu_cgain.bit_field;
				for (i = 0; i < KDRV_IFE_IQ_CGAIN_CH_MAX; i++) {

					fcgain.p_fusion_cgain_path0[i] = iq_cfg->fusion.fu_cgain.fcgain_s[i];
					fcgain.p_fusion_cgain_path1[i] = iq_cfg->fusion.fu_cgain.fcgain_l[i];
					//fcgain.p_fusion_cgain_path2[i] = iq_cfg->fusion.fu_cgain.fcgain_vs[i];
					fcgain.p_fusion_cofs_path0[i] = iq_cfg->fusion.fu_cgain.fcofs_s[i];
					fcgain.p_fusion_cofs_path1[i] = iq_cfg->fusion.fu_cgain.fcofs_l[i];
					//fcgain.p_fusion_cofs_path2[i] = iq_cfg->fusion.fu_cgain.fcofs_vs[i];

				}

				ife_eng_set_fusion_color_gain_buf_reg(p_hdl->p_eng, &fcgain);
				ife_eng_set_fcgain_en_buf_reg(p_hdl->p_eng, ENABLE);
			} else {
				ife_eng_set_fcgain_en_buf_reg(p_hdl->p_eng, DISABLE);
			}

			if(iq_cfg->fusion.dbg.enable){

				fusion.debug_mode = (UINT16) iq_cfg->fusion.dbg.mode;
				ife_eng_set_fusion_debug_buf_reg(p_hdl->p_eng, &fusion);

			}else{
				fusion.debug_mode = KDRV_IFE_IQ_DISABLE_DBG_MODE;
				ife_eng_set_fusion_debug_buf_reg(p_hdl->p_eng, &fusion);
			}

	}

	// - outlier
	if (iq_cfg->update & KDRV_IFE_UPDATE_OUTL) {
		if (iq_cfg->outl.enable) {
			IFE_OUTLSET outlier = {0};
			UINT32 bright_th[KDRV_IFE_IQ_OUTL_BRI_TH_NUM] = {0};
			UINT32 dark_th[KDRV_IFE_IQ_OUTL_DARK_TH_NUM] = {0};
			UINT32 outl_cnt[KDRV_IFE_IQ_OUTL_CNT_NUM] = {0};

			for (i = 0; i < KDRV_IFE_IQ_OUTL_BRI_TH_NUM; i++) {
				bright_th[i] = iq_cfg->outl.bright_th[i];
			}
			for (i = 0; i < KDRV_IFE_IQ_OUTL_DARK_TH_NUM; i++) {
				dark_th[i] = iq_cfg->outl.dark_th[i];
			}
			for (i = 0; i < KDRV_IFE_IQ_OUTL_CNT_NUM; i++) {
				outl_cnt[i] = iq_cfg->outl.outl_cnt[i];
			}

			outlier.p_bri_th = &bright_th[0];
			outlier.p_dark_th = &dark_th[0];
			outlier.p_outl_cnt = &outl_cnt[0];
			outlier.outl_w = iq_cfg->outl.outl_weight;
			outlier.dark_ofs = iq_cfg->outl.dark_ofs;
			outlier.bright_ofs = iq_cfg->outl.bright_ofs;
			outlier.ord_range_bri = iq_cfg->outl.ord_rng_bri;
			outlier.ord_range_dark = iq_cfg->outl.ord_rng_dark;
			outlier.ord_protect_th = iq_cfg->outl.ord_protect_th;
			outlier.ord_blend_w = iq_cfg->outl.ord_blend_w;
			//outlier.outl_edge_smooth_en = iq_cfg->outl.outl_edge_smooth_en;
			outlier.outl_comp_mode = iq_cfg->outl.outl_comp_mode;
			//outlier.outl_extre_idx = iq_cfg->outl.outl_extre_idx;
			outlier.p_ord_bri_w = &iq_cfg->outl.ord_bri_w[0];
			outlier.p_ord_dark_w = &iq_cfg->outl.ord_dark_w[0];
			outlier.outl_rgbir_rb_w = iq_cfg->outl.outl_rgbir_rb_w;
			outlier.ord_rgbir_rb_w = iq_cfg->outl.ord_rgbir_rb_w;

			ife_eng_set_outlier_buf_reg(p_hdl->p_eng, &outlier);

			ife_eng_set_outlier_en_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ife_eng_set_outlier_en_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	// - filter
	if (iq_cfg->update & KDRV_IFE_UPDATE_FILT) {
		if (iq_cfg->filt.enable) {

			if(KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt ) == KDRV_IPP_FMT_CLASS_RAW || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt ) == KDRV_IPP_FMT_CLASS_NRX){

				IFE_CLAMPSET clamp_weight = {0};
				{
					UINT32 weight[KDRV_IFE_IQ_SPATIAL_W_LEN] = {0};

					for (i = 0; i < KDRV_IFE_IQ_SPATIAL_W_LEN; i++) {
						weight[i] = iq_cfg->filt.spatial.weight[i];
					}
					ife_eng_set_2dnr_spatial_w_buf_reg(p_hdl->p_eng, &weight[0]);
					//ife_eng_set_2dnr_filter_mode_buf_reg(p_hdl->p_eng, iq_cfg->filt.spatial.ife_filt_mode);
				}

				{
					IFE_RANGESETA range_set_a = {0};

					range_set_a.p_rngth_c0 = &iq_cfg->filt.rng_filt_r.a_th[0];
					range_set_a.p_rnglut_c0 = &iq_cfg->filt.rng_filt_r.a_lut[0];
					range_set_a.p_rngth_c1 = &iq_cfg->filt.rng_filt_gr.a_th[0];
					range_set_a.p_rnglut_c1 = &iq_cfg->filt.rng_filt_gr.a_lut[0];
					if (kdrv_ife_typecast_bayer_fmt(io_cfg->in_frm.fmt) == IFE_BAYER_RGBIR) {
						range_set_a.p_rngth_c2 = &iq_cfg->filt.rng_filt_ir.a_th[0];
						range_set_a.p_rnglut_c2 = &iq_cfg->filt.rng_filt_ir.a_lut[0];
					} else {
						range_set_a.p_rngth_c2 = &iq_cfg->filt.rng_filt_gb.a_th[0];
						range_set_a.p_rnglut_c2 = &iq_cfg->filt.rng_filt_gb.a_lut[0];
					}
					range_set_a.p_rngth_c3 = &iq_cfg->filt.rng_filt_b.a_th[0];
					range_set_a.p_rnglut_c3 = &iq_cfg->filt.rng_filt_b.a_lut[0];
					ife_eng_set_2dnr_nlm_parameter_buf_reg(p_hdl->p_eng, &range_set_a);
				}

				{
					IFE_RANGESETB range_set_b = {0};

					range_set_b.p_rngth_c0 = &iq_cfg->filt.rng_filt_r.b_th[0];
					range_set_b.p_rnglut_c0 = &iq_cfg->filt.rng_filt_r.b_lut[0];
					range_set_b.p_rngth_c1 = &iq_cfg->filt.rng_filt_gr.b_th[0];
					range_set_b.p_rnglut_c1 = &iq_cfg->filt.rng_filt_gr.b_lut[0];
					if (kdrv_ife_typecast_bayer_fmt(io_cfg->in_frm.fmt) == IFE_BAYER_RGBIR) {
						range_set_b.p_rngth_c2 = &iq_cfg->filt.rng_filt_ir.b_th[0];
						range_set_b.p_rnglut_c2 = &iq_cfg->filt.rng_filt_ir.b_lut[0];
					} else {
						range_set_b.p_rngth_c2 = &iq_cfg->filt.rng_filt_gb.b_th[0];
						range_set_b.p_rnglut_c2 = &iq_cfg->filt.rng_filt_gb.b_lut[0];
					}
					range_set_b.p_rngth_c3 = &iq_cfg->filt.rng_filt_b.b_th[0];
					range_set_b.p_rnglut_c3 = &iq_cfg->filt.rng_filt_b.b_lut[0];
					ife_eng_set_2dnr_bilat_parameter_buf_reg(p_hdl->p_eng, &range_set_b);
				}

				if (iq_cfg->filt.center_mod.enable) {
					IFE_CENMODSET center_mod_set = {0};

					center_mod_set.bilat_th1 = iq_cfg->filt.center_mod.th1;
					center_mod_set.bilat_th2 = iq_cfg->filt.center_mod.th2;
					center_mod_set.bilat_cen_sel = iq_cfg->filt.center_mod.cen_sel;
					ife_eng_set_2dnr_center_modify_buf_reg(p_hdl->p_eng, &center_mod_set);

					ife_eng_set_cen_mod_en_buf_reg(p_hdl->p_eng, ENABLE);
				} else {
					ife_eng_set_cen_mod_en_buf_reg(p_hdl->p_eng, DISABLE);
				}

				ife_eng_set_2dnr_blend_weight_buf_reg(p_hdl->p_eng, iq_cfg->filt.blend_w);

				ife_eng_set_2dnr_range_weight_buf_reg(p_hdl->p_eng, iq_cfg->filt.rng_th_w);

				if (iq_cfg->filt.rbfill.enable) {
					IFE_RBFill_PARAM rbfill_set = {0};

					rbfill_set.p_rbfill_rbluma = &iq_cfg->filt.rbfill.luma[0];
					rbfill_set.p_rbfill_rbratio = &iq_cfg->filt.rbfill.ratio[0];
					rbfill_set.rbfill_ratio_mode = iq_cfg->filt.rbfill.ratio_mode;
					ife_eng_set_2dnr_rbfill_buf_reg(p_hdl->p_eng, &rbfill_set);
					ife_eng_set_2dnr_rbfill_ratio_mode_buf_reg(p_hdl->p_eng, &rbfill_set);

					ife_eng_set_rb_fill_en_buf_reg(p_hdl->p_eng, ENABLE);
				} else {
					ife_eng_set_rb_fill_en_buf_reg(p_hdl->p_eng, DISABLE);
				}

				clamp_weight.clamp_th = iq_cfg->filt.clamp.th;
				clamp_weight.clamp_mul = iq_cfg->filt.clamp.mul;
				clamp_weight.clamp_dlt = iq_cfg->filt.clamp.dlt;
				ife_eng_set_clamp_buf_reg(p_hdl->p_eng, &clamp_weight);

				ife_eng_set_binning_buf_reg(p_hdl->p_eng, iq_cfg->filt.bin);

				ife_eng_set_filter_en_buf_reg(p_hdl->p_eng, ENABLE);

			}else{
				DBG_ERR("filt enable but format not bayer\n");
			}


		} else {
			ife_eng_set_filter_en_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	// - dgain
	if (iq_cfg->update & KDRV_IFE_UPDATE_DGAIN) {
		if (iq_cfg->dgain.enable) {
			IFE_DGAINSET dgain_para = {0};

			dgain_para.dgain = iq_cfg->dgain.dgain;
			ife_eng_set_digital_gain_buf_reg(p_hdl->p_eng, &dgain_para);

			ife_eng_set_dgain_en_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ife_eng_set_dgain_en_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	// - cgain
	if (iq_cfg->update & KDRV_IFE_UPDATE_CGAIN) {
		if (iq_cfg->cgain.enable) {
			IFE_CGAINSET cgain = {0};

			cgain.b_cgain_inv = iq_cfg->cgain.inv;
			cgain.b_cgain_hinv = iq_cfg->cgain.hinv;
			cgain.cgain_range = kdrv_ife_typecast_cgrange(iq_cfg->cgain.bit_field);
			cgain.cgain_mask = iq_cfg->cgain.mask;
			cgain.p_cgain[0] = iq_cfg->cgain.cgain_r;
			cgain.p_cgain[1] = iq_cfg->cgain.cgain_gr;
			cgain.p_cgain[2] = iq_cfg->cgain.cgain_gb;
			cgain.p_cgain[3] = iq_cfg->cgain.cgain_b;
			cgain.p_cgain[4] = iq_cfg->cgain.cgain_ir;
			cgain.p_cofs[0] = iq_cfg->cgain.cofs_r;
			cgain.p_cofs[1] = iq_cfg->cgain.cofs_gr;
			cgain.p_cofs[2] = iq_cfg->cgain.cofs_gb;
			cgain.p_cofs[3] = iq_cfg->cgain.cofs_b;
			cgain.p_cofs[4] = iq_cfg->cgain.cofs_ir;
			ife_eng_set_color_gain_buf_reg(p_hdl->p_eng, &cgain, kdrv_ife_typecast_pix(io_cfg->in_frm.fmt));

			ife_eng_set_cgain_en_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ife_eng_set_cgain_en_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	// - vignette
	if (iq_cfg->update & KDRV_IFE_UPDATE_VIG) {
		if (iq_cfg->vig.enable) {

			if(KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt ) == KDRV_IPP_FMT_CLASS_RAW || KDRV_IPP_FMT_CLASS(io_cfg->in_frm.fmt ) == KDRV_IPP_FMT_CLASS_NRX){

				KDRV_IFE_IQ_VIG vig_set_lut_para = {0};
				int i = 0;
				vig_set_lut_para.enable             = iq_cfg->vig.enable;
				vig_set_lut_para.dist_th            = iq_cfg->vig.dist_th;
				vig_set_lut_para.dither_enable      = iq_cfg->vig.dither_enable;
				vig_set_lut_para.dither_rst_enable  = iq_cfg->vig.dither_rst_enable;

				for(i = 0; i< KDRV_IFE_IQ_VIG_CH0_LUT_SIZE; i++){
					vig_set_lut_para.ch_r_lut[i]  = iq_cfg->vig.ch_r_lut[i];
				}
				for(i = 0; i< KDRV_IFE_IQ_VIG_CH1_LUT_SIZE; i++){
					vig_set_lut_para.ch_gr_lut[i] = iq_cfg->vig.ch_gr_lut[i];
				}
				for(i = 0; i< KDRV_IFE_IQ_VIG_CH2_LUT_SIZE; i++){
					vig_set_lut_para.ch_gb_lut[i] = iq_cfg->vig.ch_gb_lut[i];
					vig_set_lut_para.ch_ir_lut[i] = iq_cfg->vig.ch_ir_lut[i];
				}
				for(i = 0; i< KDRV_IFE_IQ_VIG_CH3_LUT_SIZE; i++){
					vig_set_lut_para.ch_b_lut[i]  = iq_cfg->vig.ch_b_lut[i];
				}

				vig_set.vig_dist_th = iq_cfg->vig.dist_th;
				vig_set.b_vig_dither_en = iq_cfg->vig.dither_enable;
				vig_set.b_vig_dither_rst = iq_cfg->vig.dither_rst_enable;
				kdrv_ife_cal_vig_setting(&vig_set, io_cfg->in_frm.size.w, io_cfg->in_frm.size.h);
				kdrv_ife_cal_vig_tab_gain(&vig_set, &vig_set_lut_para, kdrv_ife_typecast_bayer_fmt(io_cfg->in_frm.fmt));


				ife_eng_set_vignette_param_buf_reg(p_hdl->p_eng, &vig_set);

				ife_eng_set_vig_en_buf_reg(p_hdl->p_eng, ENABLE);
			}else{
				DBG_ERR("vig enable but format is not bayer\n");
			}

		} else {
			ife_eng_set_vig_en_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	// - gbal
	if (iq_cfg->update & KDRV_IFE_UPDATE_GBAL) {
		if (iq_cfg->gbal.enable) {
			IFE_GBAL_PARAM gbal_set = {0};

			gbal_set.b_protect_en = iq_cfg->gbal.protect_enable;
			gbal_set.diff_th_str = iq_cfg->gbal.diff_th_str;
			gbal_set.diff_w_max = iq_cfg->gbal.diff_w_max;
			gbal_set.edge_protect_th1 = iq_cfg->gbal.edge_protect_th1;
			gbal_set.edge_protect_th0 = iq_cfg->gbal.edge_protect_th0;
			gbal_set.edge_w_max = iq_cfg->gbal.edge_w_max;
			gbal_set.edge_w_min = iq_cfg->gbal.edge_w_min;
			gbal_set.p_gbal_ofs = &iq_cfg->gbal.gbal_ofs[0];
			gbal_set.str_luma_low_bnd  = iq_cfg->gbal.str_luma_low_bnd;
			gbal_set.edge_luma_low_bnd = iq_cfg->gbal.edge_luma_low_bnd;

			ife_eng_set_gbal_buf_reg(p_hdl->p_eng, &gbal_set);

			ife_eng_set_gbal_en_buf_reg(p_hdl->p_eng, ENABLE);
		} else {
			ife_eng_set_gbal_en_buf_reg(p_hdl->p_eng, DISABLE);
		}
	}

	if (iq_cfg->update & KDRV_IFE_UPDATE_FPN) {
		IFE_ENG_FPN_PARAM fpn_param = {0};

		fpn_param.fpn_en = iq_cfg->fpn.fpn_en;
		fpn_param.fpn_cgain_range = iq_cfg->fpn.fpn_cgain_range;

		for(i=0; i < 1; i++){
			fpn_param.fpn_cgain_r[i] = iq_cfg->fpn.fpn_cgain_r[i];
			fpn_param.fpn_cgain_gr[i] = iq_cfg->fpn.fpn_cgain_gr[i];
			fpn_param.fpn_cgain_gb[i] = iq_cfg->fpn.fpn_cgain_gb[i];
			fpn_param.fpn_cgain_b[i] = iq_cfg->fpn.fpn_cgain_b[i];
			fpn_param.fpn_cgain_ir[i] = iq_cfg->fpn.fpn_cgain_ir[i];

			fpn_param.fpn_cofs_r[i] = iq_cfg->fpn.fpn_cofs_r[i];
			fpn_param.fpn_cofs_gr[i] = iq_cfg->fpn.fpn_cofs_gr[i];
			fpn_param.fpn_cofs_gb[i] = iq_cfg->fpn.fpn_cofs_gb[i];
			fpn_param.fpn_cofs_b[i] = iq_cfg->fpn.fpn_cofs_b[i];
			fpn_param.fpn_cofs_ir[i] = iq_cfg->fpn.fpn_cofs_ir[i];
		}

		ife_eng_set_fpn_para_buf_reg(p_hdl->p_eng, &fpn_param);
	}


	// - VA
	rt |= kdrv_ife_int_cfg_va(p_hdl, p_cfg, &single_out_info);
	/* nn-isp config */
	rt |= kdrv_ife_int_cfg_nn_isp(p_hdl, p_cfg);


	// - Single Out
	{
		IFE_ENG_SIGLE_OUT_PARAM single_out_param = {0};

		single_out_param.out_mode = 1; // 0: orginal mode 1: single out mode

		if ((op_mode == IFE_OPMODE_D2D) || (io_cfg->subisp_param.subisp_out_en)){
			single_out_param.single_out_0_en = 1;
		}

		if(io_cfg->wdr_sub_out_en) {
			single_out_param.single_out_1_en = 1;
		}

		if(single_out_info.va_en) {
			single_out_param.single_out_2_en = 1;//single_out_info.va_en;
			single_out_param.single_out_indep_va_en = 1;
		}

		ife_eng_set_single_out_buf_reg(p_hdl->p_eng, single_out_param);
	}

#endif
	return rt;
}

#if 0
#endif

static INT32 kdrv_ife_cfg_process_cpu(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	INT32 rt = E_OK;
	ULONG eng_reg_buf;
	ULONG eng_flg_buf;
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	/* directly set register address to engine driver, prevent write twice */
	eng_reg_buf = p_hdl->p_eng->reg_io_base;
	eng_flg_buf = kdrv_ife_int_get_eng_flg_buf_addr(p_hdl);

	if (eng_reg_buf == 0 || eng_flg_buf == 0) {
		DBG_ERR("no eng buffer, reg_buf 0x%.8x, flg_buf 0x%.8x\r\n", (unsigned int)eng_reg_buf, (unsigned int)eng_flg_buf);
		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
		return E_NOMEM;
	}
#if IFE_SSDRV_SUPPORT
	ife_eng_set_reg_buf(p_hdl->p_eng, eng_reg_buf, eng_flg_buf);
#endif
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
	rt = kdrv_ife_int_cfg_all(p_hdl, (KDRV_IFE_JOB_CFG *)p_data);
#if KDRV_IPP_ENG_CHECK_ENABLE
	if (rt == E_OK) {
		/* eng check limitation */
		#if IFE_SSDRV_SUPPORT  //538 modified
		rt = ife_eng_chk_limitation(eng_reg_buf, eng_flg_buf);
		#endif
	}
#endif

	return rt;
}


static INT32 kdrv_ife_cfg_process_ll(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	INT32 rt = E_OK;
	UINT32 i;
	ULONG eng_reg_buf;
	ULONG eng_flg_buf;
	UINT32 *p_reg;
	UINT8 *p_flg;
	KDRV_IFE_JOB_CFG *p_cfg;
	KDRV_IPP_LL_SET_INFO *p_ll_blk;
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	/* reset engine buffer to 0, and set to engine driver */
	eng_reg_buf = kdrv_ife_int_get_eng_reg_buf_addr(p_hdl);
	eng_flg_buf = kdrv_ife_int_get_eng_flg_buf_addr(p_hdl);
	if (eng_reg_buf == 0 || eng_flg_buf == 0) {
		DBG_ERR("no eng buffer, reg_buf 0x%.8x, flg_buf 0x%.8x\r\n", (unsigned int)eng_reg_buf, (unsigned int)eng_flg_buf);
		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
		return E_NOMEM;
	}
#if IFE_SSDRV_SUPPORT
	memset((void *)eng_reg_buf, 0, ife_eng_get_reg_base_buf_size());
	memset((void *)eng_flg_buf, 0, ife_eng_get_reg_flag_buf_size());

	ife_eng_set_reg_buf(p_hdl->p_eng, eng_reg_buf, eng_flg_buf);
#endif
	p_cfg = (KDRV_IFE_JOB_CFG *)p_data;
	p_ll_blk = (KDRV_IPP_LL_SET_INFO *)p_cfg->p_ll_blk;
	if (p_ll_blk == NULL) {
		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
		return E_NOMEM;
	}

	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);

	if (p_ll_blk->stage == KDRV_IPP_LL_SET_STAGE1) {
		rt = kdrv_ife_int_cfg_all(p_hdl, p_cfg);
#if KDRV_IPP_ENG_CHECK_ENABLE
		if (rt == E_OK) {
			/* eng check limitation */
			#if IFE_SSDRV_SUPPORT  //538 modified
			rt = ife_eng_chk_limitation(eng_reg_buf, eng_flg_buf);
			#endif
		}
#endif

		vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

		/* write register to linklist */
		p_reg = (UINT32 *)eng_reg_buf;
		p_flg = (UINT8 *)eng_flg_buf;
		for (i = 0; i < IFE_ENG_REG_NUM; i++) {
			if (p_flg[i]) {
				kdrv_ipp_util_write_ll_upd_cmd(p_ll_blk, (i << 2), p_reg[i]);
			}
		}
		kdrv_ipp_util_write_ll_null_cmd(p_ll_blk, p_ll_blk->blk_idx);

		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);

		vos_cpu_dcache_sync(p_ll_blk->cmd_buf_addr.va, ALIGN_CEIL_32(p_ll_blk->cur_cmd_idx * KDRV_IPP_LL_CMD_SIZE) , VOS_DMA_TO_DEVICE);
	} else {
		/* reserve for 2 stage configuration */
	}

	return rt;
}

static INT32 kdrv_ife_trig_single(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	ife_eng_set_load_hw_reg(p_hdl->p_eng, IFE_LOADTYPE_IFE_START_LOAD);
	ife_eng_trig_single_hw_reg(p_hdl->p_eng);
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
#endif
	return E_OK;
}

static INT32 kdrv_ife_trig_ll(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT
	ULONG ll_addr;
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	ll_addr = *(ULONG*)p_data;

	// (690) d2d link-list mode patch. sie frame start comes during ipp frame end to ll cmd transfer to hw reg done will cause ll_err2 occur after ll fire.
	// do sw reset first before trig ll to reduce the change of ll_err2 isr occur to improve system preformance.
	ife_eng_int_soft_reset_hw_reg(p_hdl->p_eng);

	{
		UINT64 addr = ll_addr;
		ife_eng_trig_ll_hw_reg(p_hdl->p_eng, (UINT32)(addr & 0xFFFFFFFF), (UINT32)( (addr>> 32) & 0xFFFFFFFF));
	}

		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
#endif
	return E_OK;
}

static INT32 kdrv_ife_trig_direct(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT
	UINT32* is_dir_start = (UINT32*) p_data;
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	#if DIRECT_MODE_GLOBAL_LOAD_EN
	if (*is_dir_start) {
		ife_eng_set_load_hw_reg(p_hdl->p_eng, IFE_LOADTYPE_GLOBAL_LOAD);
		ife_eng_trig_single_hw_reg(p_hdl->p_eng);
	}
	#else
	ife_eng_set_load_hw_reg(p_hdl->p_eng, IFE_LOADTYPE_FRAME_START_LOAD);
	if (*is_dir_start) {
		ife_eng_trig_single_hw_reg(p_hdl->p_eng);
	}
	#endif

	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
#endif
	return 0;
}

static INT32 kdrv_ife_set_cb(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	p_hdl->cb = (KDRV_IPP_ISR_CB) p_data;
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);

	return E_OK;
}

static INT32 kdrv_ife_stop_single(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT
	ife_eng_stop_single_hw_reg(p_hdl->p_eng);
#endif
	return E_OK;
}

static INT32 kdrv_ife_hard_reset(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT
	ife_eng_hard_reset_hw_reg(p_hdl->p_eng);
#endif
	return E_OK;
}

static INT32 kdrv_ife_clr_ring_buf_line_cnt(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT
	ife_eng_set_dmaloop_line_count_clear_buf_reg(p_hdl->p_eng);
#endif
	return E_OK;
}

static INT32 kdrv_ife_dma_ch_abort(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	ULONG eng_reg_buf;
	ULONG eng_flg_buf;
#if IFE_SSDRV_SUPPORT
	UINT32 *dma_abort_en;
#endif
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);

	/* disable all interrupt before disable */
	eng_reg_buf = p_hdl->p_eng->reg_io_base;
	eng_flg_buf = kdrv_ife_int_get_eng_flg_buf_addr(p_hdl);
	if (eng_reg_buf == 0 || eng_flg_buf == 0) {
		DBG_ERR("no eng buffer, reg_buf 0x%.8lx, flg_buf 0x%.8lx\r\n", (ULONG)eng_reg_buf, (ULONG)eng_flg_buf);
		vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
		return E_NOMEM;
	}
#if IFE_SSDRV_SUPPORT
	ife_eng_set_reg_buf(p_hdl->p_eng, eng_reg_buf, eng_flg_buf);
	ife_eng_set_enable_int_buf_reg(p_hdl->p_eng, 0);

	dma_abort_en = (UINT32*) p_data;

	ife_eng_dma_channel_enable_hw_reg(p_hdl->p_eng, *dma_abort_en);
#endif
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
	return E_OK;
}

static INT32 kdrv_ife_dma_ch_sts(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT
	UINT32 *dma_ch_sts;

	dma_ch_sts = (UINT32*) p_data;
	*dma_ch_sts  = ife_eng_get_dma_channel_status_hw_reg(p_hdl->p_eng);
#endif
	return E_OK;
}

static INT32 kdrv_ife_reg_base_addr(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	*(ULONG *) p_data = p_hdl->p_eng->reg_io_base;
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
	return E_OK;
}

static INT32 kdrv_ife_reg_base_addr_ll(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	*(ULONG *) p_data = kdrv_ife_int_get_eng_reg_buf_addr(p_hdl);
	vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);

	return E_OK;
}

static INT32 kdrv_ife_get_dbg_info(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	KDRV_IFE_DBG_INFO* info =(KDRV_IFE_DBG_INFO*) p_data;

	info->ife_frm_ed_cnt = p_hdl->p_ife_dbg_info->ife_frm_ed_cnt;
	info->ife_frm_st_cnt = p_hdl->p_ife_dbg_info->ife_frm_st_cnt;
	info->ife_ll_ed_cnt = p_hdl->p_ife_dbg_info->ife_ll_ed_cnt;
	info->ife_bufovfl_cnt = p_hdl->p_ife_dbg_info->ife_bufovfl_cnt;
	info->ife_ring_buf_err1_cnt = p_hdl->p_ife_dbg_info->ife_ring_buf_err1_cnt;
	info->ife_ring_buf_err2_cnt = p_hdl->p_ife_dbg_info->ife_ring_buf_err2_cnt;
	info->ife_r_dec_err1_cnt = p_hdl->p_ife_dbg_info->ife_r_dec_err1_cnt;
	info->ife_r_dec_err2_cnt = p_hdl->p_ife_dbg_info->ife_r_dec_err2_cnt;
	info->ife_r_dec_err3_cnt = p_hdl->p_ife_dbg_info->ife_r_dec_err3_cnt;
	info->ife_ll_error_cnt = p_hdl->p_ife_dbg_info->ife_ll_error_cnt;
	info->ife_ll_error2_cnt = p_hdl->p_ife_dbg_info->ife_ll_error2_cnt;
	info->ife_frame_err_cnt = p_hdl->p_ife_dbg_info->ife_frame_err_cnt;

	return E_OK;
}

static INT32 kdrv_ife_get_hist_result(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT // 538 modified
	KDRV_IFE_HIST_RSLT *p_kdrv_rst;
	IFE_ENG_WDR_HIST_RESULT eng_rst;

	p_kdrv_rst = (KDRV_IFE_HIST_RSLT *)p_data;

	eng_rst.p_hist_stcs = &p_kdrv_rst->hist_stcs[0];

	ife_eng_get_wdr_hist_hw_reg(p_hdl->p_eng, &eng_rst);
#endif

	return E_OK;
}


static INT32 kdrv_ife_va_rst(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT
	KDRV_IFE_VA_RST *p_result;
	IFE_VA_RSLT rslt = {0};
	IFE_VA_SETTING setting = {0};
	IFE_VA_WIN_PARAM win = {0};

	p_result = (KDRV_IFE_VA_RST *)p_data;
	rslt.p_luma = p_result->p_luma;
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
	setting.address_pipe1 = p_result->va_setting.address.va;
	win.win_numx = p_result->va_setting.win_num_x;
	win.win_numy = p_result->va_setting.win_num_y;

	vos_cpu_dcache_sync(p_result->va_setting.address.va, KDRV_IFE_VA_BUF_SIZE, VOS_DMA_FROM_DEVICE);

	ife_eng_get_va_result_buf_reg(&setting, &win, &rslt);
#endif

	return 0;
}

static INT32 kdrv_ife_va_rst_dual(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT
#if(IFE_538_KDRV_FEATURE == 0)
	KDRV_IFE_VA_RST_DUAL *p_result;
	IFE_VA_RSLT rslt = {0};
	IFE_VA_SETTING setting = {0};
	IFE_VA_WIN_PARAM win = {0};

	p_result = (KDRV_IFE_VA_RST_DUAL *)p_data;
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
	win.win_numx = p_result->va_setting.win_num_x;
	win.win_numy = p_result->va_setting.win_num_y;

	win.win_stx = p_result->va_setting.win_start_x;
	win.win_szx = p_result->va_setting.win_size_x;
	win.win_spx = p_result->va_setting.win_skip_x;
	setting.dual_cut_pos = p_result->va_setting.dual_cut_pos;
	//setting.address_out = p_result->va_setting.address_out;

	vos_cpu_dcache_sync(p_result->va_setting.address_pipe1, KDRV_IFE_VA_BUF_SIZE, VOS_DMA_FROM_DEVICE);
	vos_cpu_dcache_sync(p_result->va_setting.address_pipe2, KDRV_IFE_VA_BUF_SIZE, VOS_DMA_FROM_DEVICE);

	ife_eng_get_dual_va_result_buf_reg(&setting, &win, &rslt);
#endif
#endif

	return 0;
}

static INT32 kdrv_ife_inde_va_rst(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT // 538 modified
	UINT32 i;
	KDRV_IFE_VA_INDEP_RSLT *p_va_rst;
	IFE_INDEP_VA_WIN_RSLT rst = {0};

	p_va_rst = (KDRV_IFE_VA_INDEP_RSLT *)p_data;

	for (i = 0; i < KDRV_IFE_IQ_VA_INDEP_NUM; i++) {

		ife_eng_get_indep_va_win_rslt_hw_reg(p_hdl->p_eng, &rst, i);
		p_va_rst->va_luma[i] = rst.va_luma;
		p_va_rst->g1_h[i] = rst.va_g1_h;
		p_va_rst->g1_v[i] = rst.va_g1_v;
		p_va_rst->g2_h[i] = rst.va_g2_h;
		p_va_rst->g2_v[i] = rst.va_g2_v;
		p_va_rst->g1_h_cnt[i] = rst.va_cnt_g1_h;
		p_va_rst->g1_v_cnt[i] = rst.va_cnt_g1_v;
		p_va_rst->g2_h_cnt[i] = rst.va_cnt_g2_h;
		p_va_rst->g2_v_cnt[i] = rst.va_cnt_g2_v;
	}
#endif
	return E_OK;
}

static INT32 kdrv_ife_get_merged_indp_va(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT // 538 modified
#if(IFE_538_KDRV_FEATURE == 0)
	KDRV_IPP_CB_STCS_OUTPUT* stcs_output = (KDRV_IPP_CB_STCS_OUTPUT* )p_data;
	IFE_INDEP_VA_WIN_MERGE left_indp_va = {0};
	IFE_INDEP_VA_WIN_MERGE right_indp_va = {0};


	left_indp_va.p_va_g1_h = stcs_output->ife_inde_va_rslt.g1_h;
	left_indp_va.p_va_g1_v = stcs_output->ife_inde_va_rslt.g1_v;
	left_indp_va.p_va_g2_h = stcs_output->ife_inde_va_rslt.g2_h;
	left_indp_va.p_va_g2_v = stcs_output->ife_inde_va_rslt.g2_v;
	left_indp_va.p_va_cnt_g1_h = stcs_output->ife_inde_va_rslt.g1_h_cnt;
	left_indp_va.p_va_cnt_g1_v = stcs_output->ife_inde_va_rslt.g1_v_cnt;
	left_indp_va.p_va_cnt_g2_h = stcs_output->ife_inde_va_rslt.g2_h_cnt;
	left_indp_va.p_va_cnt_g2_v = stcs_output->ife_inde_va_rslt.g2_v_cnt;

	right_indp_va.p_va_g1_h = stcs_output->ife_inde_va_rslt_tmp.g1_h;
	right_indp_va.p_va_g1_v = stcs_output->ife_inde_va_rslt_tmp.g1_v;
	right_indp_va.p_va_g2_h = stcs_output->ife_inde_va_rslt_tmp.g2_h;
	right_indp_va.p_va_g2_v = stcs_output->ife_inde_va_rslt_tmp.g2_v;
	right_indp_va.p_va_cnt_g1_h = stcs_output->ife_inde_va_rslt_tmp.g1_h_cnt;
	right_indp_va.p_va_cnt_g1_v = stcs_output->ife_inde_va_rslt_tmp.g1_v_cnt;
	right_indp_va.p_va_cnt_g2_h = stcs_output->ife_inde_va_rslt_tmp.g2_h_cnt;
	right_indp_va.p_va_cnt_g2_v = stcs_output->ife_inde_va_rslt_tmp.g2_v_cnt;

	ife_eng_get_merged_indep_va_win_rslt_hw_reg(&left_indp_va, &right_indp_va);
#endif

#endif
	return E_OK;
}

static INT32 kdrv_ife_get_merged_hist_sts(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
#if IFE_SSDRV_SUPPORT // 538 modified
#if(IFE_538_KDRV_FEATURE == 0)
	KDRV_IPP_CB_STCS_OUTPUT* stcs_output = (KDRV_IPP_CB_STCS_OUTPUT* )p_data;
	IFE_WDR_HIST_MERGE left_histo = {0};
	IFE_WDR_HIST_MERGE right_histo = {0};

	left_histo.p_wdr_hist = stcs_output->ife_hist_rslt.hist_stcs;
	right_histo.p_wdr_hist = stcs_output->ife_hist_rslt_tmp.hist_stcs;
	ife_eng_get_merged_wdr_hist_hw_reg(&left_histo, &right_histo);
#endif

#endif

	return E_OK;
}

static INT32 kdrv_ife_nn_isp_do_next_slice(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	UINT32* path_id = (UINT32*) p_data;

	//DBG_DUMP("-do_next_slice-\n");

	if(*path_id == 0){

		ife_eng_set_nn_isp_p0_ring_buf_slice_clear_hw_reg(p_hdl->p_eng, 1);

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[ife_nn_isp_p0_ringbuf_slice_clear SET_TO_1]\n");
		DBG_DUMP("[polling ife_nn_isp_p0_set_slice_ready_status to 0 START]\n");
		#endif

		while(ife_eng_set_nn_isp_p0_ring_buf_read_set_slice_ready_status(p_hdl->p_eng) != 0){
		}

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[polling ife_nn_isp_p0_set_slice_ready_status to 0 OK]\n");
		#endif

		ife_eng_set_nn_isp_p0_ring_buf_slice_clear_hw_reg(p_hdl->p_eng, 0);
		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[ife_nn_isp_p0_ringbuf_slice_clear SET_TO_0]\n");
		#endif

	}else if(*path_id == 1){

		ife_eng_set_nn_isp_p1_ring_buf_slice_clear_hw_reg(p_hdl->p_eng, 1);
		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[ife_nn_isp_p1_ringbuf_slice_clear SET_TO_1]\n");
		DBG_DUMP("[polling ife_nn_isp_p1_set_slice_ready_status to 0 START]\n");
		#endif

		while(ife_eng_set_nn_isp_p1_ring_buf_read_set_slice_ready_status(p_hdl->p_eng) != 0){
		}

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[polling ife_nn_isp_p1_set_slice_ready_status to 0 OK]\n");
		#endif

		ife_eng_set_nn_isp_p1_ring_buf_slice_clear_hw_reg(p_hdl->p_eng, 0); //wait Sam corrrect the api

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[ife_nn_isp_p1_ringbuf_slice_clear SET_TO_0]\n");
		#endif
	}else
		DBG_DUMP("unknown ife do next slice path_id:%d\n", *path_id);

	return E_OK;
}

static INT32 kdrv_ife_nn_isp_slice_done(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	UINT32* path_id = (UINT32*) p_data;

	#if KDRV_IPP_AI_LOG_EN
	DBG_DUMP("-ife slice_done:%d-\n", *path_id);
	#endif

	if(*path_id == 0){

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[polling ife_nn_isp_p0_get_slice_clear_status to 0 START]\n");
		#endif
		while(ife_eng_set_nn_isp_p0_ring_buf_read_set_slice_clear_status(p_hdl->p_eng) != 0){
		}

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[polling ife_nn_isp_p0_get_slice_clear_status to 0 OK]\n");

		DBG_DUMP("[ife_nn_isp_p0_outbuf_slice_ready SET_TO_1]\n");
		#endif
		ife_eng_set_nn_isp_p0_out_buf_slice_ready_hw_reg(p_hdl->p_eng, 1);

	}else if(*path_id == 1){

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[polling ife_nn_isp_p1_get_slice_clear_status to 0 START]\n");
		#endif
		while(ife_eng_set_nn_isp_p1_ring_buf_read_set_slice_clear_status(p_hdl->p_eng) != 0){
		}

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[polling ife_nn_isp_p1_get_slice_clear_status to 0 OK]\n");

		DBG_DUMP("[ife_nn_isp_p1_outbuf_slice_ready SET_TO_1]\n");
		#endif
		ife_eng_set_nn_isp_p1_out_buf_slice_ready_hw_reg(p_hdl->p_eng, 1);
	}else
		DBG_DUMP("unknown ife slice don clear path_id:%d\n", *path_id);

	return E_OK;
}

static INT32 kdrv_ife_nn_isp_slice_clr(KDRV_IFE_HANDLE *p_hdl, void *p_data)
{
	UINT32* intr_status = (UINT32*) p_data;

	if(*intr_status == KDRV_IFE_INTERRUPT_IFE_P0_GET_SLICE_CLR){

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[ife_nn_isp_p0_outbuf_slice_ready SET_TO_0]\n");
		#endif

		ife_eng_set_nn_isp_p0_out_buf_slice_ready_hw_reg(p_hdl->p_eng, 0);
	}
	else if(*intr_status == KDRV_IFE_INTERRUPT_IFE_P1_GET_SLICE_CLR){

		#if KDRV_IPP_AI_LOG_EN
		DBG_DUMP("[ife_nn_isp_p1_outbuf_slice_ready SET_TO_0]\n");
		#endif

		ife_eng_set_nn_isp_p1_out_buf_slice_ready_hw_reg(p_hdl->p_eng, 0);
	}
	else{
		DBG_DUMP("unknown slice_clr:0x%x\n", *intr_status );
	}


	return E_OK;
}

static KDRV_IFE_FUNC_ITEM g_kdrv_ife_func_tab[KDRV_IFE_PARAM_ID_MAX] = {
	{kdrv_ife_cfg_process_cpu,			1, NULL, 						0, "cfg_process_cpu"},
	{kdrv_ife_cfg_process_ll,			1, NULL, 						0, "cfg_process_ll"},
	{kdrv_ife_trig_single,				0, NULL, 						0, "trig_single"},
	{kdrv_ife_trig_ll,					1, NULL, 						0, "trig_ll"},
	{kdrv_ife_trig_direct,				1, NULL, 						0, "trig_direct mode"},
	{kdrv_ife_set_cb,					1, NULL, 						0, "set_cb"},
	{kdrv_ife_stop_single,				0, NULL, 						0, "stop_single"},
	{kdrv_ife_hard_reset,				0, NULL, 						0, "hard reset"},
	{kdrv_ife_clr_ring_buf_line_cnt,	0, NULL, 						0, "clr_ring_buf_line_cnt"},
	{kdrv_ife_dma_ch_abort,				1, NULL, 						0, "dma abort en"},
	{kdrv_ife_nn_isp_do_next_slice,		1, NULL, 						0, "nn isp do next slice"},
	{kdrv_ife_nn_isp_slice_done,		1, NULL, 						0, "nn isp slice done"},
	{kdrv_ife_nn_isp_slice_clr,			1, NULL, 						0, "nn isp slice clr"},
	{NULL,								0, kdrv_ife_dma_ch_sts,			1, "dma ch status"},
	{NULL,								0, kdrv_ife_reg_base_addr,		1, "reg base addr"},
	{NULL,								0, kdrv_ife_reg_base_addr_ll,	1, "reg base addr ll mode"},
	{NULL,								0, kdrv_ife_get_dbg_info,		1, "dbg info"},
	{NULL,                              0, kdrv_ife_get_hist_result, 	1, "histogram"},
	{NULL,				                0, kdrv_ife_va_rst,	 			1, "ife va rst"},
	{NULL,								0, kdrv_ife_va_rst_dual,		1, "va rst_dual"},
	{NULL,                              0, kdrv_ife_inde_va_rst, 		1, "ife va inde rst"},
	{NULL,                              0, kdrv_ife_get_merged_indp_va,	1, "ife merged indp va"},
	{NULL,                              0, kdrv_ife_get_merged_hist_sts,1, "ife merged histo stsc"},
};

INT32 kdrv_ife_set(UINT32 id, KDRV_IFE_PARAM_ID param_id, VOID *p_data)
{
	KDRV_IFE_HANDLE *p_hdl;
	//unsigned long loc_flg;
	INT32 rt;

	if (param_id >= KDRV_IFE_PARAM_ID_MAX) {
		DBG_ERR("param_id %d overflow\r\n", (int)param_id);
		return E_PAR;
	}

	p_hdl = kdrv_ife_int_get_handle(KDRV_DEV_ID_CHIP(id), KDRV_DEV_ID_ENGINE(id));
	if (p_hdl == NULL) {
		DBG_ERR("null handle of id 0x%.8x\r\n", (unsigned int)id);
		return E_PAR;
	}

	//vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	rt = E_SYS;
	if (g_kdrv_ife_func_tab[param_id].set_chk_data) {
		if (p_data == NULL) {
			DBG_ERR("p_data is null pointer\r\n");
			rt = E_PAR;
			goto err;
		}
	}

	if (g_kdrv_ife_func_tab[param_id].set_fp) {
		rt = g_kdrv_ife_func_tab[param_id].set_fp(p_hdl, p_data);
		if (rt < 0) {
			DBG_ERR("%s err, rt %d\r\n", g_kdrv_ife_func_tab[param_id].msg, rt);
		}
	}

err:
	//vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
	return rt;
}

INT32 kdrv_ife_get(UINT32 id, KDRV_IFE_PARAM_ID param_id, VOID *p_data)
{
	KDRV_IFE_HANDLE *p_hdl;
	//unsigned long loc_flg;
	INT32 rt;

	if (param_id >= KDRV_IFE_PARAM_ID_MAX) {
		DBG_ERR("param_id %d overflow\r\n", (int)param_id);
		return E_PAR;
	}

	p_hdl = kdrv_ife_int_get_handle(KDRV_DEV_ID_CHIP(id), KDRV_DEV_ID_ENGINE(id));
	if (p_hdl == NULL) {
		DBG_ERR("null handle of id 0x%.8x\r\n", (unsigned int)id);
		return E_PAR;
	}

	//vk_spin_lock_irqsave(&p_hdl->lock, loc_flg);
	rt = E_SYS;
	if (g_kdrv_ife_func_tab[param_id].get_chk_data) {
		if (p_data == NULL) {
			DBG_ERR("p_data is null pointer\r\n");
			rt = E_PAR;
			goto err;
		}
	}

	if (g_kdrv_ife_func_tab[param_id].get_fp) {
		rt = g_kdrv_ife_func_tab[param_id].get_fp(p_hdl, p_data);
		if (rt < 0) {
			DBG_ERR("%s err, rt %d\r\n", g_kdrv_ife_func_tab[param_id].msg, rt);
		}
	}

err:
	//vk_spin_unlock_irqrestore(&p_hdl->lock, loc_flg);
	return rt;
}

#if 0
#endif

INT32 kdrv_ife_query_reg_num(UINT32 id, void *p_data)
{
	*(UINT32 *)p_data = IFE_ENG_REG_NUM;

	return E_OK;
}

static INT32 kdrv_ife_query_strp_info(UINT32 id, void *p_data)
{
#if IFE_SSDRV_SUPPORT
	IFE_STRIPE_LIMIT info;
	KDRV_IFE_STRP_INFO *p_info;

	p_info = (KDRV_IFE_STRP_INFO *)p_data;

	if(KDRV_IPP_FMT_CLASS(p_info->in_format) == KDRV_IPP_FMT_CLASS_RAW ||
			KDRV_IPP_FMT_CLASS(p_info->in_format) == KDRV_IPP_FMT_CLASS_NRX ){
		ife_eng_get_stripe_limit(0, &info);
	}
	else if(KDRV_IPP_FMT_CLASS(p_info->in_format) == KDRV_IPP_FMT_CLASS_YUV){
		ife_eng_get_stripe_limit(1, &info);
	}else{
		DBG_ERR("unknown ife ovlap in format\n");
		return E_PAR;
	}

	p_info->alignment = info.stripe_align;
	p_info->overlap = info.stripe_overlap;
#endif
	return E_OK;
}


static KDRV_IFE_QUERY_ITEM g_kdrv_ife_query_tab[KDRV_IFE_QUERY_ID_MAX] =
{
	{kdrv_ife_query_reg_num,	"reg number"},
	{kdrv_ife_query_strp_info,	"stripe info"}
};

INT32 kdrv_ife_query(UINT32 id, KDRV_IFE_QUERY_ID qid, void *p_param)
{
	INT32 rt;

	if (qid >= KDRV_IFE_QUERY_ID_MAX) {
		DBG_ERR("query_id %d overflow\r\n", (int)qid);
		return E_PAR;
	}


	rt = E_SYS;
	if (g_kdrv_ife_query_tab[qid].query_fp) {
		rt = g_kdrv_ife_query_tab[qid].query_fp(id, p_param);
		if (rt < 0) {
			DBG_ERR("query %s err, rt %d\r\n", g_kdrv_ife_query_tab[qid].msg, rt);
		}
	}

	return rt;
}

#if 0
#endif

void kdrv_ife_dump(void)
{
	UINT32 i=0,j=0;
	KDRV_IFE_HANDLE *handle;

	DBG_DUMP("---- kdrv_ife dump info ----\r\n");

	if ((g_kdrv_ife_ctl.p_hdl == NULL) || (g_kdrv_ife_ctl.total_ch == 0)) {
		DBG_ERR("error g_kdrv_ife_ctl.p_hdl(0x%.8lx) total ch = %d\r\n", (ULONG)g_kdrv_ife_ctl.p_hdl, g_kdrv_ife_ctl.total_ch);
		return;
	}

	DBG_DUMP("===================================================================================\n");
	DBG_DUMP("eng chip_num = %d\n", g_kdrv_ife_ctl.chip_num);
	DBG_DUMP("eng eng_num = %d\n", g_kdrv_ife_ctl.eng_num);
	DBG_DUMP("eng total ch = %d\n", g_kdrv_ife_ctl.total_ch);
	DBG_DUMP("===================================================================================\n");
	DBG_DUMP("chip        eng eng_handle   callback\n");

	for (i = 0; i < g_kdrv_ife_ctl.total_ch; i ++) {
		handle = &g_kdrv_ife_ctl.p_hdl[i];

		if (handle) {

			if(handle->p_eng == NULL){
				DBG_ERR("null eng handle:0x%lx\n",(ULONG) handle->p_eng);
				return;
			}

			DBG_DUMP("%4d 0x%.8x 0x%.8lx 0x%.8lx\r\n", handle->chip_id, handle->eng_id, (ULONG)handle->p_eng, (ULONG)handle->cb);

			handle->p_ife_dbg_info->ife_buf_current_status = *((UINT32 *) (handle->p_eng->reg_io_base + 0x68));

			DBG_DUMP("\n------------------------------------- Debug Info %d ----------------------------------\n",i);
			DBG_DUMP("%19s%1s  %19s%1s  %16s%4s %12s%8s\n","ife_line_buf_status"," ","ife_overflow_status"," ", "ife_overflow_cnt"," ","ife_clk_rate"," ");
			DBG_DUMP("0x%.8x%10s  0x%.8x%10s  %10d%10s  %5d%3s%12s\n",
					handle->p_ife_dbg_info->ife_buf_current_status,
					" ",
					handle->p_ife_dbg_info->ife_buf_overflow_status,
					" ",
					handle->p_ife_dbg_info->ife_overflow_cnt,
					" ",
					(UINT32) handle->p_eng->clock_rate,
					"Mhz",
					" "
					);
			DBG_DUMP("%17s%3s %18s%2s\n","ife_cur_proc_time"," ","ife_ovfl_proc_time"," ");
			DBG_DUMP("%8d%2s%10s %8d%2s%10s\n",
					(handle->p_ife_dbg_info->ife_buf_current_status & 0xFFFF) * ( 1000/handle->p_eng->clock_rate),
					"ns",
					" ",
					(handle->p_ife_dbg_info->ife_buf_overflow_status & 0xFFFF) * ( 1000/handle->p_eng->clock_rate),
					"ns",
					" "
					);

			if(handle->ife_dbg_ctl.ife_dbg_fun & KDRV_IFE_DBG_STATUS_REC){

				DBG_DUMP("\n record:%d\n",handle->p_ife_dbg_info->ife_record_idx);

				for(j = 0 ; j < handle->p_ife_dbg_info->ife_record_num; j++){

					if(j == handle->p_ife_dbg_info->ife_record_idx){
						DBG_DUMP("* 0x%.8x  %dns\n",handle->p_ife_dbg_info->ife_buf_rec_status[j],
								(handle->p_ife_dbg_info->ife_buf_rec_status[j] & 0xFFFF) * ( 1000/handle->p_eng->clock_rate) );
					}
					else{
						DBG_DUMP("0x%.8x  %dns\n",handle->p_ife_dbg_info->ife_buf_rec_status[j],
								(handle->p_ife_dbg_info->ife_buf_rec_status[j] & 0xFFFF) * ( 1000/handle->p_eng->clock_rate) );
					}
				}
			}
		}
	}




	DBG_DUMP("\r\n");
}

static CHAR* kdrv_ife_dump_mode_str(KDRV_IPP_OPMODE mode)
{
	CHAR *mode_str[KDRV_IPP_OPMODE_MAX] = {
		"NONE",
		"D2D",
		"IFE2IME",
		"DCE2IME",
		"SIE2IME"
	};

	if (mode >= KDRV_IPP_OPMODE_MAX) {
		return "ERR";
	}

	return mode_str[mode];
}

static CHAR* kdrv_ife_dump_fmt_str(KDRV_IPP_FMT fmt)
{
	switch (KDRV_IFE_FMT_PIX(fmt)) {
	case KDRV_IPP_FMT_RGGB_R:
	case KDRV_IPP_FMT_RGGB_GR:
	case KDRV_IPP_FMT_RGGB_GB:
	case KDRV_IPP_FMT_RGGB_B:
		return "RGGB";
	case KDRV_IPP_FMT_RGBIR44_RGBG_GIGI:
	case KDRV_IPP_FMT_RGBIR44_GBGR_IGIG:
	case KDRV_IPP_FMT_RGBIR44_GIGI_BGRG:
	case KDRV_IPP_FMT_RGBIR44_IGIG_GRGB:
	case KDRV_IPP_FMT_RGBIR44_BGRG_GIGI:
	case KDRV_IPP_FMT_RGBIR44_GRGB_IGIG:
	case KDRV_IPP_FMT_RGBIR44_GIGI_RGBG:
	case KDRV_IPP_FMT_RGBIR44_IGIG_GBGR:
		return "RGBIR";
	default:
		return "ERR";
	}
}

typedef enum {
	KDRV_IFE_DUMP_ARY_UINT32,
	KDRV_IFE_DUMP_ARY_INT32,
	KDRV_IFE_DUMP_ARY_UINT8,
	KDRV_IFE_DUMP_ARY_UINT16,
} KDRV_IFE_DUMP_ARY_TYPE;

#define KDRV_IFE_DUMP_ARY_ENABLE	(0)
#define KDRV_IFE_DUMP_BUF_SIZE 1024

static void kdrv_ife_dump_ary(char *title, UINT32 num, KDRV_IFE_DUMP_ARY_TYPE type, void *tbl, int (*KDRV_IFE_DUMP)(const char *fmt, ...))
{
#if KDRV_IFE_DUMP_ARY_ENABLE
	/* work around, check stack usage */
	static CHAR _str_buf[KDRV_IFE_DUMP_BUF_SIZE] = {0};
	CHAR *str_buf = _str_buf;
	UINT32 i, st;

	KDRV_IFE_DUMP("    [%s]\r\n", title);

	st = 0;
	st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%12s: ", "index");
	for (i = 0; i < num; i++) {
		st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4u ", (unsigned int)i);
	}
	KDRV_IFE_DUMP("%s\r\n", str_buf);

	st = 0;
	st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%12s: ", "value");
	if (type == KDRV_IFE_DUMP_ARY_UINT32) {
		for (i = 0; i < num; i++) {
			st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4u ", (unsigned int)((UINT32 *)tbl)[i]);
		}
	} else if (type == KDRV_IFE_DUMP_ARY_INT32) {
		for (i = 0; i < num; i++) {
			st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4d ", (unsigned int)((INT32 *)tbl)[i]);
		}
	} else if (type == KDRV_IFE_DUMP_ARY_UINT8) {
		for (i = 0; i < num; i++) {
			st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4u ", (unsigned int)((UINT8 *)tbl)[i]);
		}
	} else if (type == KDRV_IFE_DUMP_ARY_UINT16) {
		for (i = 0; i < num; i++) {
			st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4u ", (unsigned int)((UINT16 *)tbl)[i]);
		}
	}
	KDRV_IFE_DUMP("%s\r\n", str_buf);

	KDRV_IFE_DUMP("\r\n");
#endif
}
#if 0
static void kdrv_ife_dump_ary_log(char *title, UINT32 num, KDRV_IFE_DUMP_ARY_TYPE type, void *tbl)
{
#if KDRV_IFE_DUMP_ARY_ENABLE
	/* work around, check stack usage */
	static CHAR _str_buf[KDRV_IFE_DUMP_BUF_SIZE] = {0};
	CHAR *str_buf = _str_buf;
	UINT32 i, st;

	printm2("    [%s]\r\n", title);

	st = 0;
	st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%12s: ", "index");
	for (i = 0; i < num; i++) {
		st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4u ", (unsigned int)i);
	}
	printm2("%s\r\n", str_buf);

	st = 0;
	st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%12s: ", "value");
	if (type == KDRV_IFE_DUMP_ARY_UINT32) {
		for (i = 0; i < num; i++) {
			st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4u ", (unsigned int)((UINT32 *)tbl)[i]);
		}
	} else if (type == KDRV_IFE_DUMP_ARY_INT32) {
		for (i = 0; i < num; i++) {
			st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4d ", (unsigned int)((INT32 *)tbl)[i]);
		}
	} else if (type == KDRV_IFE_DUMP_ARY_UINT8) {
		for (i = 0; i < num; i++) {
			st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4u ", (unsigned int)((UINT8 *)tbl)[i]);
		}
	} else if (type == KDRV_IFE_DUMP_ARY_UINT16) {
		for (i = 0; i < num; i++) {
			st += scnprintf(str_buf+st, KDRV_IFE_DUMP_BUF_SIZE-st, "%4u ", (unsigned int)((UINT16 *)tbl)[i]);
		}
	}
	printm2("%s\r\n", str_buf);

	printm2("\r\n");
#endif
}
#endif

void kdrv_ife_dump_cfg(KDRV_IFE_JOB_CFG *p_cfg, int (*KDRV_IFE_DUMP)(const char *fmt, ...))
{
	KDRV_IFE_IO_CFG *p_iocfg = p_cfg->p_iocfg;
	KDRV_IFE_IQ_CFG *p_iqcfg = p_cfg->p_iqcfg;

	KDRV_IFE_DUMP("---- kdrv_ife dump cfg ----\r\n");

	KDRV_IFE_DUMP("---------------- IOCFG ----------------\r\n");
	KDRV_IFE_DUMP("%8s(%3s)  %8s(%10s)  %4s  %4s  %6s%4s%2s%4s%2s%4s%2s%4s%1s  %10s  %8s  %10s  %8s  %8s(%10s)  %10s  %8s  %10s\r\n",
		"mode", "RAW", "i_fmt", "RAW",
		"i_w", "i_h",
		"i_crp(", "x", ", ", "y", ", ", "w", ", ", "h", ")",
		"i_addr0", "i_lofs0", "i_addr1", "i_lofs1",
		"o_fmt", "RAW", "o_addr", "o_lofs",
		"inte_en");
	KDRV_IFE_DUMP("%8s(%3u)  %8s(0x%08x)  %4u  %4u  %6s%4u%2s%4u%2s%4u%2s%4u%1s  0x%08lx  %8u  0x%08lx  %8u  %8s(0x%08x)  0x%08lx  %8u  0x%08x\r\n",
		kdrv_ife_dump_mode_str(p_iocfg->mode), p_iocfg->mode, kdrv_ife_dump_fmt_str(p_iocfg->in_frm.fmt), p_iocfg->in_frm.fmt,
		p_iocfg->in_frm.size.w, p_iocfg->in_frm.size.h,
		"", p_iocfg->crp_window.x, "", p_iocfg->crp_window.y, "", p_iocfg->crp_window.w, "", p_iocfg->crp_window.h, "",
		p_iocfg->in_frm.addr[0], p_iocfg->in_frm.lofs[0], p_iocfg->in_frm.addr[1], p_iocfg->in_frm.lofs[1],
		kdrv_ife_dump_fmt_str(p_iocfg->out_frm.fmt), p_iocfg->out_frm.fmt, p_iocfg->out_frm.addr[0], p_iocfg->out_frm.lofs[0],
		p_iocfg->inte_en);
	KDRV_IFE_DUMP("%8s %3s   %8s   %8s   %4s  %4s  %6s%4s%2s%4s%2s%4s%2s%4s%1s  0x%08lx  %8s  0x%08lx  %8s  %8s    %8s  0x%08lx  %8s  %8s\r\n",
		" ",
		" ",
		" ",
		" ",
		" ",
		" ",
		"",
		" ",
		"",
		" ",
		"",
		" ",
		"",
		" ",
		"",
		(p_iocfg->in_frm.phyaddr[0] == 0) ? 0 : (ULONG)p_iocfg->in_frm.phyaddr[0],
		" ",
		(p_iocfg->in_frm.phyaddr[1] == 0) ? 0 : (ULONG)p_iocfg->in_frm.phyaddr[1],
		" ",
		" ",
		" ",
		(p_iocfg->out_frm.phyaddr[0] == 0) ? 0 : (ULONG)p_iocfg->out_frm.phyaddr[0],
		" ",
		" ");
	KDRV_IFE_DUMP("\r\n");

	KDRV_IFE_DUMP("%10s  %6s  %12s  %12s  %14s  %16s  %12s %12s\r\n", "mirror_en", "rde_en", "rde_adj_mode", "rde_enc_rate", "ring_buf[0]_en", "ring_buf[0]_line", "ring_buf_imm", "thermal_mode_en");
	KDRV_IFE_DUMP("%10u  %6u  %12u  %12u  %14u  %16u  %12u %12u\r\n", p_iocfg->mirror.en, (KDRV_IPP_FMT_CLASS(p_iocfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX),
		p_iocfg->rde.encode_adjmode,  p_iocfg->rde.encode_rate, p_iocfg->ring_buf.ring[0].en, p_iocfg->ring_buf.ring[0].line_num, p_iocfg->ring_buf.imm_start, p_iocfg->thermal_mode_en);
	KDRV_IFE_DUMP("\r\n");

	KDRV_IFE_DUMP("%12s (%4s, %4s), (%4s, %4s), (%4s, %4s), (%4s, %4s)\r\n",
				"vig_center", "c1_x", "c1_y", "c2_x", "c2_y", "c3_x", "c3_y", "c4_x", "c4_y");
	KDRV_IFE_DUMP("%12s (%4d, %4d), (%4d, %4d), (%4d, %4d), (%4d, %4d)\r\n",
				" ",
				(int)p_iocfg->vig_pos.center[0].x, (int)p_iocfg->vig_pos.center[0].y,
				(int)p_iocfg->vig_pos.center[1].x, (int)p_iocfg->vig_pos.center[1].y,
				(int)p_iocfg->vig_pos.center[2].x, (int)p_iocfg->vig_pos.center[2].y,
				(int)p_iocfg->vig_pos.center[3].x, (int)p_iocfg->vig_pos.center[3].y);
	KDRV_IFE_DUMP("\r\n");

	KDRV_IFE_DUMP("---------------- IQCFG (update: 0x%08x) ----------------\r\n", p_iqcfg->update);

	KDRV_IFE_DUMP("---- nrs0 (en: %u, update: %u) ----\r\n", p_iqcfg->nrs0.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_NRS0) != 0);
	//KDRV_IFE_DUMP("---- nrs1 (en: %u, update: %u) ----\r\n", p_iqcfg->nrs1.en, (p_iqcfg->update & KDRV_IFE_UPDATE_NRS1) != 0);


	KDRV_IFE_DUMP("%9s\r\n", "[nrs0 str]");
	kdrv_ife_dump_ary("nrs0 str", KDRV_IFE_IQ_NRS_STR_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->nrs0.str[0], KDRV_IFE_DUMP);
	//KDRV_IFE_DUMP("%9s\r\n", "[nrs1 str]");
	//kdrv_ife_dump_ary("nrs1 str", KDRV_IFE_NRS1_STR_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->nrs1.str[0], KDRV_IFE_DUMP);


	////////////////////////////////////////////////////////////////////////////

	KDRV_IFE_DUMP("---- fcurve (en: %u, update: %u) ----\r\n", p_iqcfg->fcurve.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_FCURVE) != 0);
	KDRV_IFE_DUMP("%11s  %10s  %5s\r\n", "[fcur_ctrl]", "y_mean_sel", "yv_w");
	KDRV_IFE_DUMP("%11s  %10u  %5u\r\n", "", p_iqcfg->fcurve.fcur_ctrl.y_mean_sel, p_iqcfg->fcurve.fcur_ctrl.yv_w);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("y_w_lut", KDRV_IFE_IQ_FCURVE_Y_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fcurve.y_weight.y_w_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("cur_l_lut", KDRV_IFE_IQ_FCURVE_L_NUM, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->fcurve.fcurve_l.fcur_l_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("cur_r_lut", KDRV_IFE_IQ_FCURVE_R_NUM, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->fcurve.fcurve_r.fcur_r_lut[0], KDRV_IFE_DUMP);
	//kdrv_ife_dump_ary("cur_end_lut", KDRV_IFE_FCURVE_END_NUM, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->fcurve.fcurve_end.fcur_end_lut[0], KDRV_IFE_DUMP);

	////////////////////////////////////////////////////////////////////////////

	KDRV_IFE_DUMP("---- fusion ([en: %u,] update: %u) ----\r\n", (kdrv_ife_typecast_fnum(p_iocfg->in_frm.fmt) > 0), (p_iqcfg->update & KDRV_IFE_UPDATE_FUSION) != 0);

	KDRV_IFE_DUMP("%9s  %10s  %4s  %8s\r\n", "[fu_ctrl]", "y_mean_sel", "mode", "ev_ratio");
	KDRV_IFE_DUMP("%9s  %10u  %4u  %8u\r\n", "", p_iqcfg->fusion.fu_ctrl.y_mean_sel, p_iqcfg->fusion.fu_ctrl.mode, p_iqcfg->fusion.fu_ctrl.ev_ratio);
	KDRV_IFE_DUMP("\r\n");
	KDRV_IFE_DUMP("%9s  %7s  %7s  %11s  %12s  %11s  %12s  %11s  %12s  %11s  %12s\r\n",
		"[bld_cur]", "nor_sel", "dif_sel",
		"l_nor_range", "l_nor_slope",
		"s_nor_range", "s_nor_slope",
		"l_dif_range", "l_dif_slope",
		"s_dif_range", "s_dif_slope");
	KDRV_IFE_DUMP("%9s  %7u  %7u  %11u  %12u  %11u  %12u  %11u  %12u  %11u  %12u\r\n", "", p_iqcfg->fusion.bld_cur.nor_sel, p_iqcfg->fusion.bld_cur.dif_sel,
		p_iqcfg->fusion.bld_cur.l_nor_range, p_iqcfg->fusion.bld_cur.l_nor_slope,
		p_iqcfg->fusion.bld_cur.s_nor_range, p_iqcfg->fusion.bld_cur.s_nor_slope,
		p_iqcfg->fusion.bld_cur.l_dif_range, p_iqcfg->fusion.bld_cur.l_dif_slope,
		p_iqcfg->fusion.bld_cur.s_dif_range, p_iqcfg->fusion.bld_cur.s_dif_slope);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("l_nor_knee", KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.bld_cur.l_nor_knee[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("s_nor_knee", KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.bld_cur.s_nor_knee[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("l_dif_knee", KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.bld_cur.l_dif_knee[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("s_dif_knee", KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.bld_cur.s_dif_knee[0], KDRV_IFE_DUMP);
	KDRV_IFE_DUMP("%9s  %6s  %10s  %5s\r\n", "[mc_para]", "lum_th", "diff_ratio", "dwd");
	KDRV_IFE_DUMP("%9s  %6u  %10u  %5u\r\n", "", p_iqcfg->fusion.mc_para.lum_th, p_iqcfg->fusion.mc_para.diff_ratio, p_iqcfg->fusion.mc_para.dwd);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("diff_w", KDRV_IFE_IQ_FUSION_MC_DIFF_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.mc_para.diff_w[0], KDRV_IFE_DUMP);
	//98530 remove//kdrv_ife_dump_ary("neg_diff_w", KDRV_IFE_FUSION_MC_DIFF_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.mc_para.neg_diff_w[0]);
	KDRV_IFE_DUMP("%8s\r\n", "[dk_sat]");
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("th", KDRV_IFE_IQ_FUSION_DARK_SAT_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.dk_sat.th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("step", KDRV_IFE_IQ_FUSION_DARK_SAT_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.dk_sat.step[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("low_bound", KDRV_IFE_IQ_FUSION_DARK_SAT_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.dk_sat.low_bound[0], KDRV_IFE_DUMP);
	//530 removed
	/*KDRV_IFE_DUMP("%8s  %2s\r\n", "[s_comp]", "en");
	KDRV_IFE_DUMP("%8s  %2u\r\n", "", p_iqcfg->fusion.s_comp.en);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("knee", KDRV_IFE_FUSION_SHORT_COMP_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.s_comp.knee[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("sub_point", KDRV_IFE_FUSION_SHORT_COMP_SUB_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.s_comp.sub_point[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("shift", KDRV_IFE_FUSION_SHORT_COMP_SHIFT_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.s_comp.shift[0], KDRV_IFE_DUMP);*/
	KDRV_IFE_DUMP("%10s  %2s  %9s\r\n", "[fu_cgain]", "en", "bit_field");
	KDRV_IFE_DUMP("%10s  %2u  %9u\r\n", "", p_iqcfg->fusion.fu_cgain.enable, p_iqcfg->fusion.fu_cgain.bit_field);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("fcgain_s", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.fu_cgain.fcgain_s[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("fcofs_s", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.fu_cgain.fcofs_s[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("fcgain_l", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.fu_cgain.fcgain_l[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("fcofs_l", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.fu_cgain.fcofs_l[0], KDRV_IFE_DUMP);

	KDRV_IFE_DUMP("%10s\n", "[hdr_ref_chk]");
	KDRV_IFE_DUMP("%10x\n", p_iocfg->hdr_ref_chk);

	////////////////////////////////////////////////////////////////////////////

	KDRV_IFE_DUMP("---- outlier (en: %u, update: %u) ----\r\n", p_iqcfg->outl.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_OUTL) != 0);

	KDRV_IFE_DUMP("%11s  %8s  %10s  %11s  %12s  %14s  %11s\r\n", "outl_weight", "dark_ofs", "bright_ofs", "ord_rng_bri", "ord_rng_dark", "ord_protect_th", "ord_blend_w");
	KDRV_IFE_DUMP("%11u  %8u  %10u  %11u  %12u  %14u  %11u\r\n", p_iqcfg->outl.outl_weight, p_iqcfg->outl.dark_ofs, p_iqcfg->outl.bright_ofs,
		p_iqcfg->outl.ord_rng_bri, p_iqcfg->outl.ord_rng_dark, p_iqcfg->outl.ord_protect_th, p_iqcfg->outl.ord_blend_w);
	//KDRV_IFE_DUMP("%20s  %20s  %20s\r\n", "outl_comp_mode", "outl_edge_smooth_en", "outl_extre_idx");
	//KDRV_IFE_DUMP("%20u  %20u  %20u\r\n", p_iqcfg->outl.outl_comp_mode, p_iqcfg->outl.outl_edge_smooth_en, p_iqcfg->outl.outl_extre_idx);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("bright_th", KDRV_IFE_IQ_OUTL_BRI_TH_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.bright_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("dark_th", KDRV_IFE_IQ_OUTL_DARK_TH_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.dark_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("outl_cnt", KDRV_IFE_IQ_OUTL_CNT_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.outl_cnt[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("ord_bri_w", KDRV_IFE_IQ_OUTL_ORD_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.ord_bri_w[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("ord_dark_w", KDRV_IFE_IQ_OUTL_ORD_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.ord_dark_w[0], KDRV_IFE_DUMP);

	////////////////////////////////////////////////////////////////////////////

	KDRV_IFE_DUMP("---- filter (en: %u, update: %u) ----\r\n", p_iqcfg->filt.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_FILT) != 0);

	KDRV_IFE_DUMP("%7s  %8s  %3s\r\n", "blend_w", "rng_th_w", "bin");
	KDRV_IFE_DUMP("%7u  %8u  %3u\r\n", p_iqcfg->filt.blend_w, p_iqcfg->filt.rng_th_w, p_iqcfg->filt.bin);
	KDRV_IFE_DUMP("\r\n");
	KDRV_IFE_DUMP("%12s  %2s  %5s  %5s  %7s\r\n", "[center_mod]", "en", "th1", "th2", "cen_sel");
	KDRV_IFE_DUMP("%12s  %2u  %5u  %5u  %7s\r\n", "", p_iqcfg->filt.center_mod.enable, p_iqcfg->filt.center_mod.th1, p_iqcfg->filt.center_mod.th2, p_iqcfg->filt.center_mod.cen_sel == 0 ? "DPC" : "Bilat");
	KDRV_IFE_DUMP("\r\n");
	KDRV_IFE_DUMP("%7s  %5s  %5s  %5s\r\n", "[clamp]", "th", "mul", "dlt");
	KDRV_IFE_DUMP("%7s  %5u  %5u  %5u\r\n", "", p_iqcfg->filt.clamp.th, p_iqcfg->filt.clamp.mul, p_iqcfg->filt.clamp.dlt);
	KDRV_IFE_DUMP("\r\n");
	KDRV_IFE_DUMP("%8s  %2s  %10s\r\n", "[rbfill]", "en", "ratio_mode");
	KDRV_IFE_DUMP("%8s  %2u  %10u\r\n", "", p_iqcfg->filt.rbfill.enable, p_iqcfg->filt.rbfill.ratio_mode);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("spatial weight", KDRV_IFE_IQ_SPATIAL_W_LEN, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->filt.spatial.weight[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch0 a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_r.a_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch0 a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_r.a_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch0 b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_r.b_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch0 b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_r.b_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch1 a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gr.a_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch1 a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gr.a_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch1 b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gr.b_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch1 b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gr.b_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch2(gb) a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gb.a_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch2(gb) a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gb.a_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch2(gb) b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gb.b_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch2(gb) b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gb.b_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch2(ir) a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_ir.a_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch2(ir) a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_ir.a_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch2(ir) b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_ir.b_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch2(ir) b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_ir.b_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch3 a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_b.a_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch3 a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_b.a_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch3 b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_b.b_th[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rng_filt_ch3 b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_b.b_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rbfill luma", KDRV_IFE_IQ_RBFILL_LUMA_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->filt.rbfill.luma[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("rbfill ratio", KDRV_IFE_IQ_RBFILL_RATIO_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->filt.rbfill.ratio[0], KDRV_IFE_DUMP);

	////////////////////////////////////////////////////////////////////////////

	KDRV_IFE_DUMP("---- cgain (en: %u, update: %u) ----\r\n", p_iqcfg->cgain.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_CGAIN) != 0);

	KDRV_IFE_DUMP("%3s  %4s  %9s  %5s\r\n", "inv", "hinv", "bit_field", "mask");
	KDRV_IFE_DUMP("%3u  %4u  %9u  %5u\r\n", p_iqcfg->cgain.inv, p_iqcfg->cgain.hinv, p_iqcfg->cgain.bit_field, p_iqcfg->cgain.mask);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("cgain", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->cgain.cgain_r, KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("cofs", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->cgain.cofs_r, KDRV_IFE_DUMP);

	////////////////////////////////////////////////////////////////////////////

	KDRV_IFE_DUMP("---- vignette (en: %u, update: %u) ----\r\n", p_iqcfg->vig.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_VIG) != 0);

	KDRV_IFE_DUMP("%7s  %7s  %11s\r\n", "dist_th", "dith_en", "dith_rst_en");
	KDRV_IFE_DUMP("%7u  %7u  %11u\r\n", p_iqcfg->vig.dist_th, p_iqcfg->vig.dither_enable, p_iqcfg->vig.dither_rst_enable);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("lut_ch0", KDRV_IFE_IQ_VIG_CH0_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_r_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("lut_ch1", KDRV_IFE_IQ_VIG_CH1_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_gr_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("lut_ch2(gb)", KDRV_IFE_IQ_VIG_CH2_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_gb_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("lut_ch2(ir)", KDRV_IFE_IQ_VIG_CH2_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_ir_lut[0], KDRV_IFE_DUMP);
	kdrv_ife_dump_ary("lut_ch3", KDRV_IFE_IQ_VIG_CH3_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_b_lut[0], KDRV_IFE_DUMP);

	////////////////////////////////////////////////////////////////////////////

	KDRV_IFE_DUMP("---- gbal (en: %u, update: %u) ----\r\n", p_iqcfg->gbal.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_GBAL) != 0);

	KDRV_IFE_DUMP("%7s  %10s  %9s  %12s  %12s  %9s  %9s\r\n", "prot_en", "dif_th_str", "dif_w_max", "edg_prot_th0", "edg_prot_th1", "edg_w_min", "edg_w_max");
	KDRV_IFE_DUMP("%7u  %10u  %9u  %12u  %12u  %9u  %9u\r\n", p_iqcfg->gbal.protect_enable, p_iqcfg->gbal.diff_th_str, p_iqcfg->gbal.diff_w_max,
		p_iqcfg->gbal.edge_protect_th0, p_iqcfg->gbal.edge_protect_th1, p_iqcfg->gbal.edge_w_min, p_iqcfg->gbal.edge_w_max);
	KDRV_IFE_DUMP("\r\n");
	kdrv_ife_dump_ary("gbal_ofs", KDRV_IFE_IQ_GBAL_OFS_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->gbal.gbal_ofs[0], KDRV_IFE_DUMP);
}
#if 0
void kdrv_ife_dump_cfg_log(KDRV_IFE_JOB_CFG *p_cfg)
{
	KDRV_IFE_IO_CFG *p_iocfg = p_cfg->p_iocfg;
	KDRV_IFE_IQ_CFG *p_iqcfg = p_cfg->p_iqcfg;

	printm2("---- kdrv_ife dump cfg ----\r\n");

	printm2("---------------- IOCFG ----------------\r\n");
	printm2("%8s(%3s)  %8s(%10s)  %4s  %4s  %6s%4s%2s%4s%2s%4s%2s%4s%1s  %10s  %8s  %10s  %8s  %8s(%10s)  %10s  %8s  %10s\r\n",
		"mode", "RAW", "i_fmt", "RAW",
		"i_w", "i_h",
		"i_crp(", "x", ", ", "y", ", ", "w", ", ", "h", ")",
		"i_addr0", "i_lofs0", "i_addr1", "i_lofs1",
		"o_fmt", "RAW", "o_addr", "o_lofs",
		"inte_en");
	printm2("%8s(%3u)  %8s(0x%08x)  %4u  %4u  %6s%4u%2s%4u%2s%4u%2s%4u%1s  0x%08lx  %8u  0x%08lx  %8u  %8s(0x%08x)  0x%08lx  %8u  0x%08x\r\n",
		kdrv_ife_dump_mode_str(p_iocfg->mode), p_iocfg->mode, kdrv_ife_dump_fmt_str(p_iocfg->in_frm.fmt), p_iocfg->in_frm.fmt,
		p_iocfg->in_frm.size.w, p_iocfg->in_frm.size.h,
		"", p_iocfg->crp_window.x, "", p_iocfg->crp_window.y, "", p_iocfg->crp_window.w, "", p_iocfg->crp_window.h, "",
		p_iocfg->in_frm.addr[0], p_iocfg->in_frm.lofs[0], p_iocfg->in_frm.addr[1], p_iocfg->in_frm.lofs[1],
		kdrv_ife_dump_fmt_str(p_iocfg->out_frm.fmt), p_iocfg->out_frm.fmt, p_iocfg->out_frm.addr[0], p_iocfg->out_frm.lofs[0],
		p_iocfg->inte_en);
	printm2("%8s %3s   %8s   %8s   %4s  %4s  %6s%4s%2s%4s%2s%4s%2s%4s%1s  0x%08lx  %8s  0x%08lx  %8s  %8s    %8s  0x%08lx  %8s  %8s\r\n",
		" ",
		" ",
		" ",
		" ",
		" ",
		" ",
		"",
		" ",
		"",
		" ",
		"",
		" ",
		"",
		" ",
		"",
		(p_iocfg->in_frm.phyaddr[0] == 0) ? 0 : (ULONG)p_iocfg->in_frm.phyaddr[0],
		" ",
		(p_iocfg->in_frm.phyaddr[1] == 0) ? 0 : (ULONG)p_iocfg->in_frm.phyaddr[1],
		" ",
		" ",
		" ",
		(p_iocfg->out_frm.phyaddr[0] == 0) ? 0 : (ULONG)p_iocfg->out_frm.phyaddr[0],
		" ",
		" ");
	printm2("\r\n");

	printm2("%10s  %6s  %12s  %12s  %14s  %16s  %12s %12s\r\n", "mirror_en", "rde_en", "rde_adj_mode", "rde_enc_rate", "ring_buf[0]_en", "ring_buf[0]_line", "ring_buf_imm", "thermal_mode_en");
	printm2("%10u  %6u  %12u  %12u  %14u  %16u  %12u %12u\r\n", p_iocfg->mirror.en, (KDRV_IPP_FMT_CLASS(p_iocfg->in_frm.fmt) == KDRV_IPP_FMT_CLASS_NRX),
		p_iocfg->rde.encode_adjmode,  p_iocfg->rde.encode_rate, p_iocfg->ring_buf.ring[0].en, p_iocfg->ring_buf.ring[0].line_num, p_iocfg->ring_buf.imm_start, p_iocfg->thermal_mode_en);
	printm2("\r\n");

	printm2("%12s (%4s, %4s), (%4s, %4s), (%4s, %4s), (%4s, %4s)\r\n",
				"vig_center", "c1_x", "c1_y", "c2_x", "c2_y", "c3_x", "c3_y", "c4_x", "c4_y");
	printm2("%12s (%4d, %4d), (%4d, %4d), (%4d, %4d), (%4d, %4d)\r\n",
				" ",
				(int)p_iocfg->vig_pos.center[0].x, (int)p_iocfg->vig_pos.center[0].y,
				(int)p_iocfg->vig_pos.center[1].x, (int)p_iocfg->vig_pos.center[1].y,
				(int)p_iocfg->vig_pos.center[2].x, (int)p_iocfg->vig_pos.center[2].y,
				(int)p_iocfg->vig_pos.center[3].x, (int)p_iocfg->vig_pos.center[3].y);
	printm2("\r\n");

	printm2("---------------- IQCFG (update: 0x%08x) ----------------\r\n", p_iqcfg->update);

	printm2("---- nrs0 (en: %u, update: %u) ----\r\n", p_iqcfg->nrs0.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_NRS0) != 0);
	//printm2("---- nrs1 (en: %u, update: %u) ----\r\n", p_iqcfg->nrs1.en, (p_iqcfg->update & KDRV_IFE_UPDATE_NRS1) != 0);


	printm2("%9s\r\n", "[nrs0 str]");
	kdrv_ife_dump_ary_log("nrs0 str", KDRV_IFE_IQ_NRS_STR_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->nrs0.str[0]);
	//printm2("%9s\r\n", "[nrs1 str]");
	//kdrv_ife_dump_ary_log("nrs1 str", KDRV_IFE_NRS1_STR_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->nrs1.str[0]);


	////////////////////////////////////////////////////////////////////////////

	printm2("---- fcurve (en: %u, update: %u) ----\r\n", p_iqcfg->fcurve.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_FCURVE) != 0);
	printm2("%11s  %10s  %5s\r\n", "[fcur_ctrl]", "y_mean_sel", "yv_w");
	printm2("%11s  %10u  %5u\r\n", "", p_iqcfg->fcurve.fcur_ctrl.y_mean_sel, p_iqcfg->fcurve.fcur_ctrl.yv_w);
	printm2("\r\n");
	kdrv_ife_dump_ary_log("y_w_lut", KDRV_IFE_IQ_FCURVE_Y_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fcurve.y_weight.y_w_lut[0]);
	kdrv_ife_dump_ary_log("cur_l_lut", KDRV_IFE_IQ_FCURVE_L_NUM, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->fcurve.fcurve_l.fcur_l_lut[0]);
	kdrv_ife_dump_ary_log("cur_r_lut", KDRV_IFE_IQ_FCURVE_R_NUM, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->fcurve.fcurve_r.fcur_r_lut[0]);
	//kdrv_ife_dump_ary_log("cur_end_lut", KDRV_IFE_FCURVE_END_NUM, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->fcurve.fcurve_end.fcur_end_lut[0]);

	////////////////////////////////////////////////////////////////////////////

	printm2("---- fusion ([en: %u,] update: %u) ----\r\n", (kdrv_ife_typecast_fnum(p_iocfg->in_frm.fmt) > 0), (p_iqcfg->update & KDRV_IFE_UPDATE_FUSION) != 0);

	printm2("%9s  %10s  %4s  %8s\r\n", "[fu_ctrl]", "y_mean_sel", "mode", "ev_ratio");
	printm2("%9s  %10u  %4u  %8u\r\n", "", p_iqcfg->fusion.fu_ctrl.y_mean_sel, p_iqcfg->fusion.fu_ctrl.mode, p_iqcfg->fusion.fu_ctrl.ev_ratio);
	printm2("\r\n");
	printm2("%9s  %7s  %7s  %11s  %12s  %11s  %12s  %11s  %12s  %11s  %12s\r\n",
		"[bld_cur]", "nor_sel", "dif_sel",
		"l_nor_range", "l_nor_slope",
		"s_nor_range", "s_nor_slope",
		"l_dif_range", "l_dif_slope",
		"s_dif_range", "s_dif_slope");
	printm2("%9s  %7u  %7u  %11u  %12u  %11u  %12u  %11u  %12u  %11u  %12u\r\n", "", p_iqcfg->fusion.bld_cur.nor_sel, p_iqcfg->fusion.bld_cur.dif_sel,
		p_iqcfg->fusion.bld_cur.l_nor_range, p_iqcfg->fusion.bld_cur.l_nor_slope,
		p_iqcfg->fusion.bld_cur.s_nor_range, p_iqcfg->fusion.bld_cur.s_nor_slope,
		p_iqcfg->fusion.bld_cur.l_dif_range, p_iqcfg->fusion.bld_cur.l_dif_slope,
		p_iqcfg->fusion.bld_cur.s_dif_range, p_iqcfg->fusion.bld_cur.s_dif_slope);
	printm2("\r\n");
	kdrv_ife_dump_ary_log("l_nor_knee", KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.bld_cur.l_nor_knee[0]);
	kdrv_ife_dump_ary_log("s_nor_knee", KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.bld_cur.s_nor_knee[0]);
	kdrv_ife_dump_ary_log("l_dif_knee", KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.bld_cur.l_dif_knee[0]);
	kdrv_ife_dump_ary_log("s_dif_knee", KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.bld_cur.s_dif_knee[0]);
	printm2("%9s  %6s  %10s  %5s\r\n", "[mc_para]", "lum_th", "diff_ratio", "dwd");
	printm2("%9s  %6u  %10u  %5u\r\n", "", p_iqcfg->fusion.mc_para.lum_th, p_iqcfg->fusion.mc_para.diff_ratio, p_iqcfg->fusion.mc_para.dwd);
	printm2("\r\n");
	kdrv_ife_dump_ary_log("diff_w", KDRV_IFE_IQ_FUSION_MC_DIFF_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.mc_para.diff_w[0]);
	//98530 remove//kdrv_ife_dump_ary_log("neg_diff_w", KDRV_IFE_FUSION_MC_DIFF_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.mc_para.neg_diff_w[0]);
	printm2("%8s\r\n", "[dk_sat]");
	printm2("\r\n");
	kdrv_ife_dump_ary_log("th", KDRV_IFE_IQ_FUSION_DARK_SAT_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.dk_sat.th[0]);
	kdrv_ife_dump_ary_log("step", KDRV_IFE_IQ_FUSION_DARK_SAT_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.dk_sat.step[0]);
	kdrv_ife_dump_ary_log("low_bound", KDRV_IFE_IQ_FUSION_DARK_SAT_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.dk_sat.low_bound[0]);
	//530 removed
	/*DBG_DUMP("%8s  %2s\r\n", "[s_comp]", "en");
	DBG_DUMP("%8s  %2u\r\n", "", p_iqcfg->fusion.s_comp.en);
	DBG_DUMP("\r\n");
	kdrv_ife_dump_ary_log("knee", KDRV_IFE_FUSION_SHORT_COMP_KNEE_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.s_comp.knee[0]);
	kdrv_ife_dump_ary_log("sub_point", KDRV_IFE_FUSION_SHORT_COMP_SUB_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.s_comp.sub_point[0]);
	kdrv_ife_dump_ary_log("shift", KDRV_IFE_FUSION_SHORT_COMP_SHIFT_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->fusion.s_comp.shift[0]);*/
	printm2("%10s  %2s  %9s\r\n", "[fu_cgain]", "en", "bit_field");
	printm2("%10s  %2u  %9u\r\n", "", p_iqcfg->fusion.fu_cgain.enable, p_iqcfg->fusion.fu_cgain.bit_field);
	printm2("\r\n");
	kdrv_ife_dump_ary_log("fcgain_s", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.fu_cgain.fcgain_s[0]);
	kdrv_ife_dump_ary_log("fcofs_s", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.fu_cgain.fcofs_s[0]);
	kdrv_ife_dump_ary_log("fcgain_l", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.fu_cgain.fcgain_l[0]);
	kdrv_ife_dump_ary_log("fcofs_l", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->fusion.fu_cgain.fcofs_l[0]);

	printm2("%10s\n", "[hdr_ref_chk]");
	printm2("%10x\n", p_iocfg->hdr_ref_chk);

	////////////////////////////////////////////////////////////////////////////

	printm2("---- outlier (en: %u, update: %u) ----\r\n", p_iqcfg->outl.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_OUTL) != 0);

	printm2("%11s  %8s  %10s  %11s  %12s  %14s  %11s\r\n", "outl_weight", "dark_ofs", "bright_ofs", "ord_rng_bri", "ord_rng_dark", "ord_protect_th", "ord_blend_w");
	printm2("%11u  %8u  %10u  %11u  %12u  %14u  %11u\r\n", p_iqcfg->outl.outl_weight, p_iqcfg->outl.dark_ofs, p_iqcfg->outl.bright_ofs,
		p_iqcfg->outl.ord_rng_bri, p_iqcfg->outl.ord_rng_dark, p_iqcfg->outl.ord_protect_th, p_iqcfg->outl.ord_blend_w);
	//printm2("%20s  %20s  %20s\r\n", "outl_comp_mode", "outl_edge_smooth_en", "outl_extre_idx");
	//printm2("%20u  %20u  %20u\r\n", p_iqcfg->outl.outl_comp_mode, p_iqcfg->outl.outl_edge_smooth_en, p_iqcfg->outl.outl_extre_idx);
	printm2("\r\n");
	kdrv_ife_dump_ary_log("bright_th", KDRV_IFE_IQ_OUTL_BRI_TH_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.bright_th[0]);
	kdrv_ife_dump_ary_log("dark_th", KDRV_IFE_IQ_OUTL_DARK_TH_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.dark_th[0]);
	kdrv_ife_dump_ary_log("outl_cnt", KDRV_IFE_IQ_OUTL_CNT_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.outl_cnt[0]);
	kdrv_ife_dump_ary_log("ord_bri_w", KDRV_IFE_IQ_OUTL_ORD_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.ord_bri_w[0]);
	kdrv_ife_dump_ary_log("ord_dark_w", KDRV_IFE_IQ_OUTL_ORD_W_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->outl.ord_dark_w[0]);

	////////////////////////////////////////////////////////////////////////////

	printm2("---- filter (en: %u, update: %u) ----\r\n", p_iqcfg->filt.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_FILT) != 0);

	printm2("%7s  %8s  %3s\r\n", "blend_w", "rng_th_w", "bin");
	printm2("%7u  %8u  %3u\r\n", p_iqcfg->filt.blend_w, p_iqcfg->filt.rng_th_w, p_iqcfg->filt.bin);
	printm2("\r\n");
	printm2("%12s  %2s  %5s  %5s  %7s\r\n", "[center_mod]", "en", "th1", "th2", "cen_sel");
	printm2("%12s  %2u  %5u  %5u  %7s\r\n", "", p_iqcfg->filt.center_mod.enable, p_iqcfg->filt.center_mod.th1, p_iqcfg->filt.center_mod.th2, p_iqcfg->filt.center_mod.cen_sel == 0 ? "DPC" : "Bilat");
	printm2("\r\n");
	printm2("%7s  %5s  %5s  %5s\r\n", "[clamp]", "th", "mul", "dlt");
	printm2("%7s  %5u  %5u  %5u\r\n", "", p_iqcfg->filt.clamp.th, p_iqcfg->filt.clamp.mul, p_iqcfg->filt.clamp.dlt);
	printm2("\r\n");
	printm2("%8s  %2s  %10s\r\n", "[rbfill]", "en", "ratio_mode");
	printm2("%8s  %2u  %10u\r\n", "", p_iqcfg->filt.rbfill.enable, p_iqcfg->filt.rbfill.ratio_mode);
	printm2("\r\n");
	kdrv_ife_dump_ary_log("spatial weight", KDRV_IFE_IQ_SPATIAL_W_LEN, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->filt.spatial.weight[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch0 a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_r.a_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch0 a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_r.a_lut[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch0 b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_r.b_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch0 b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_r.b_lut[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch1 a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gr.a_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch1 a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gr.a_lut[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch1 b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gr.b_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch1 b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gr.b_lut[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch2(gb) a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gb.a_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch2(gb) a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gb.a_lut[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch2(gb) b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gb.b_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch2(gb) b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_gb.b_lut[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch2(ir) a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_ir.a_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch2(ir) a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_ir.a_lut[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch2(ir) b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_ir.b_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch2(ir) b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_ir.b_lut[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch3 a_th", KDRV_IFE_IQ_RANGE_A_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_b.a_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch3 a_lut", KDRV_IFE_IQ_RANGE_A_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_b.a_lut[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch3 b_th", KDRV_IFE_IQ_RANGE_B_TH_NUM, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_b.b_th[0]);
	kdrv_ife_dump_ary_log("rng_filt_ch3 b_lut", KDRV_IFE_IQ_RANGE_B_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->filt.rng_filt_b.b_lut[0]);
	kdrv_ife_dump_ary_log("rbfill luma", KDRV_IFE_IQ_RBFILL_LUMA_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->filt.rbfill.luma[0]);
	kdrv_ife_dump_ary_log("rbfill ratio", KDRV_IFE_IQ_RBFILL_RATIO_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->filt.rbfill.ratio[0]);

	////////////////////////////////////////////////////////////////////////////

	printm2("---- cgain (en: %u, update: %u) ----\r\n", p_iqcfg->cgain.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_CGAIN) != 0);

	printm2("%3s  %4s  %9s  %5s\r\n", "inv", "hinv", "bit_field", "mask");
	printm2("%3u  %4u  %9u  %5u\r\n", p_iqcfg->cgain.inv, p_iqcfg->cgain.hinv, p_iqcfg->cgain.bit_field, p_iqcfg->cgain.mask);
	printm2("\r\n");
	kdrv_ife_dump_ary_log("cgain", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->cgain.cgain_r);
	kdrv_ife_dump_ary_log("cofs", KDRV_IFE_IQ_CGAIN_CH_MAX, KDRV_IFE_DUMP_ARY_UINT16, &p_iqcfg->cgain.cofs_r);

	////////////////////////////////////////////////////////////////////////////

	printm2("---- vignette (en: %u, update: %u) ----\r\n", p_iqcfg->vig.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_VIG) != 0);

	printm2("%7s  %7s  %11s\r\n", "dist_th", "dith_en", "dith_rst_en");
	printm2("%7u  %7u  %11u\r\n", p_iqcfg->vig.dist_th, p_iqcfg->vig.dither_enable, p_iqcfg->vig.dither_rst_enable);
	printm2("\r\n");
	kdrv_ife_dump_ary_log("lut_ch0", KDRV_IFE_IQ_VIG_CH0_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_r_lut[0]);
	kdrv_ife_dump_ary_log("lut_ch1", KDRV_IFE_IQ_VIG_CH1_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_gr_lut[0]);
	kdrv_ife_dump_ary_log("lut_ch2(gb)", KDRV_IFE_IQ_VIG_CH2_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_gb_lut[0]);
	kdrv_ife_dump_ary_log("lut_ch2(ir)", KDRV_IFE_IQ_VIG_CH2_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_ir_lut[0]);
	kdrv_ife_dump_ary_log("lut_ch3", KDRV_IFE_IQ_VIG_CH3_LUT_SIZE, KDRV_IFE_DUMP_ARY_UINT32, &p_iqcfg->vig.ch_b_lut[0]);

	////////////////////////////////////////////////////////////////////////////

	printm2("---- gbal (en: %u, update: %u) ----\r\n", p_iqcfg->gbal.enable, (p_iqcfg->update & KDRV_IFE_UPDATE_GBAL) != 0);

	printm2("%7s  %10s  %9s  %12s  %12s  %9s  %9s\r\n", "prot_en", "dif_th_str", "dif_w_max", "edg_prot_th0", "edg_prot_th1", "edg_w_min", "edg_w_max");
	printm2("%7u  %10u  %9u  %12u  %12u  %9u  %9u\r\n", p_iqcfg->gbal.protect_enable, p_iqcfg->gbal.diff_th_str, p_iqcfg->gbal.diff_w_max,
		p_iqcfg->gbal.edge_protect_th0, p_iqcfg->gbal.edge_protect_th1, p_iqcfg->gbal.edge_w_min, p_iqcfg->gbal.edge_w_max);
	printm2("\r\n");
	kdrv_ife_dump_ary_log("gbal_ofs", KDRV_IFE_IQ_GBAL_OFS_NUM, KDRV_IFE_DUMP_ARY_UINT8, &p_iqcfg->gbal.gbal_ofs[0]);
}
#endif

void kdrv_ife_dump_register(void)
{
	KDRV_IFE_HANDLE *p_hdl;
	UINT32 i;

	DBG_DUMP("---- kdrv_ife dump register ----\r\n");
	for (i = 0; i < g_kdrv_ife_ctl.total_ch; i++) {
		p_hdl = &g_kdrv_ife_ctl.p_hdl[i];
		if (p_hdl->p_eng) {
			DBG_DUMP("chip_id = 0x%.8x, eng_id = 0x%.8x\r\n",
				(unsigned int)p_hdl->chip_id, (unsigned int)p_hdl->eng_id);
#if IFE_SSDRV_SUPPORT
#if defined(__FREERTOS)
			debug_dumpmem(p_hdl->p_eng->reg_io_base, ife_eng_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
#else
			debug_dumpmem(vos_cpu_get_phy_addr(p_hdl->p_eng->reg_io_base), ife_eng_get_reg_base_buf_size());
#endif
#endif
		}
	}
}
// For Damnit debug
void kdrv_ife_dump_register_log(void)
{
	KDRV_IFE_HANDLE *p_hdl;
	UINT32 i;

	printm2("---- kdrv_ife dump register ----\r\n");

	for (i = 0; i < g_kdrv_ife_ctl.total_ch; i++) {
		p_hdl = &g_kdrv_ife_ctl.p_hdl[i];
		if (p_hdl->p_eng) {
			printm2("chip_id = 0x%.8x, eng_id = 0x%.8x\r\n",
				(unsigned int)p_hdl->chip_id, (unsigned int)p_hdl->eng_id);
#if IFE_SSDRV_SUPPORT
#if defined(__FREERTOS)
			debug_dumpmem(p_hdl->p_eng->reg_io_base, ife_eng_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
#else
			printm2("virtual_addr = 0x%lx\r\n", p_hdl->p_eng->reg_io_base);
			kdrv_ipp_debug_dumpmem_log(p_hdl->p_eng->reg_io_base, ife_eng_get_reg_base_buf_size());
#endif
#endif
		}
	}
}

UINT32 kdrv_ife_dbg_mode(void *p_hdl, UINT32 param_id, void *data)
{
	KDRV_IFE_DBG_CMD_HDL *p_dbg_hdl = (KDRV_IFE_DBG_CMD_HDL *) p_hdl;
	KDRV_IFE_HANDLE *p_handle;
	UINT32 ret=0;

	switch(param_id){

	case KDRV_IFE_DBG_IFE_LINEBUF_STATUS_REC_EN:
	{
		KDRV_IFE_DBG_REC_CTL_PARAM *param = (KDRV_IFE_DBG_REC_CTL_PARAM *)data;

		if(p_dbg_hdl ==NULL){
			DBG_ERR("p_dbg_hdl is NULL\n");
			return E_PAR;
		}

		p_handle = kdrv_ife_int_get_handle(p_dbg_hdl->chip_id, p_dbg_hdl->eng_id);

		if(p_handle == NULL){
			DBG_ERR("handle is NULL, chip id=0x%x and eng id=0x%x\n",p_dbg_hdl->chip_id, p_dbg_hdl->eng_id);
			return E_PAR;
		}

		if(param->rec_en == 1){

			if( (p_handle->ife_dbg_ctl.ife_dbg_fun & KDRV_IFE_DBG_STATUS_REC) == 0){

				if(param->rec_num < 1)
					p_handle->p_ife_dbg_info->ife_record_num = 1;
				else
					p_handle->p_ife_dbg_info->ife_record_num = param->rec_num;

				p_handle->ife_dbg_ctl.ife_bypass_isr_cb = param->bypass_isr_cb;
				p_handle->p_ife_dbg_info->ife_buf_rec_status =  kdrv_ife_os_malloc_wrap(sizeof(UINT32) * p_handle->p_ife_dbg_info->ife_record_num);

				if (p_handle->p_ife_dbg_info->ife_buf_rec_status == NULL){
					DBG_ERR("alloc buf for ife record failed\r\n");
					ret = -1;
					return ret;
				}

				memset((void *)p_handle->p_ife_dbg_info->ife_buf_rec_status, 0, sizeof(UINT32) * p_handle->p_ife_dbg_info->ife_record_num);
				p_handle->p_ife_dbg_info->ife_record_idx = 0;
				p_handle->ife_dbg_ctl.ife_dbg_fun |= KDRV_IFE_DBG_STATUS_REC;

				DBG_DUMP("---- ife line buf status rec en=%d bypass_cb=%d num=%d----\n", param->rec_en, param->bypass_isr_cb, param->rec_num);
			}else{
				DBG_DUMP("---- ife line buf status rec enable twice ----\n");
			}

		}else if(param->rec_en == 0){

			if((p_handle->ife_dbg_ctl.ife_dbg_fun & KDRV_IFE_DBG_STATUS_REC) == 0){
				DBG_DUMP("---- ife line buf status rec disable twice ----\n");
			}else{

				kdrv_ife_os_mfree_wrap(p_handle->p_ife_dbg_info->ife_buf_rec_status);
				p_handle->p_ife_dbg_info->ife_buf_rec_status =  NULL;
				p_handle->ife_dbg_ctl.ife_dbg_fun &= ~KDRV_IFE_DBG_STATUS_REC;

				DBG_DUMP("---- ife line buf status rec en = 0 ----\n");
			}
		}else{
			DBG_DUMP("---- unknown ife dbg mode ----\n");
		}

	}
	break;
	case KDRV_IFE_DBG_IFE_FUSION_DBG_EN:
		{
			KDRV_IFE_DBG_FUSION_OUT_CTL_PARAM *param = (KDRV_IFE_DBG_FUSION_OUT_CTL_PARAM *)data;

			if(p_dbg_hdl ==NULL){
				DBG_ERR("p_dbg_hdl is NULL\n");
				return E_PAR;
			}

			p_handle = kdrv_ife_int_get_handle(p_dbg_hdl->chip_id, p_dbg_hdl->eng_id);

			if(p_handle == NULL){
				DBG_ERR("handle is NULL, chip id=0x%x and eng id=0x%x\n",p_dbg_hdl->chip_id, p_dbg_hdl->eng_id);
				return E_PAR;
			}

			DBG_DUMP("---- ife fusion dbg en:%d mode:%d----\n",param->fusion_dbg_en,param->out_mode);

			if(param->fusion_dbg_en == 1){

				p_handle->ife_dbg_ctl.ife_dbg_fun |= KDRV_IFE_DBG_SET_HDR_OUT_MODE;
				p_handle->ife_dbg_ctl.ife_fusion_out_mode = param->out_mode;

			}else if(param->fusion_dbg_en == 0){

				p_handle->ife_dbg_ctl.ife_dbg_fun &= ~KDRV_IFE_DBG_SET_HDR_OUT_MODE;

			}else{

				DBG_DUMP("---- unknown ife fusion dbg mode ----\n");
			}
		}
		break;
	case KDRVO_IFE_DBB_SET_HDR_REF_CH:
		{
			KDRV_IFE_DBG_SET_FU_REF_PARAM *param = (KDRV_IFE_DBG_SET_FU_REF_PARAM *)data;

			if(p_dbg_hdl ==NULL){
				DBG_ERR("p_dbg_hdl is NULL\n");
				return E_PAR;
			}

			p_handle = kdrv_ife_int_get_handle(p_dbg_hdl->chip_id, p_dbg_hdl->eng_id);

			if(p_handle == NULL){
				DBG_ERR("handle is NULL, chip id=0x%x and eng id=0x%x\n",p_dbg_hdl->chip_id, p_dbg_hdl->eng_id);
				return E_PAR;
			}

			DBG_DUMP("---- ife set fusion ref channel en:%d check:%d----\n",param->fusion_set_fu_ref_en ,param->hdr_ref_chk);

			if(param->fusion_set_fu_ref_en == 1){

				p_handle->ife_dbg_ctl.ife_dbg_fun |= KDRV_IFE_DBG_SET_FUSION_REF;
				p_handle->ife_dbg_ctl.hdr_ref_chk = param->hdr_ref_chk;

			}else if(param->fusion_set_fu_ref_en == 0){

				p_handle->ife_dbg_ctl.ife_dbg_fun &= ~KDRV_IFE_DBG_SET_FUSION_REF;

			}else{

				DBG_DUMP("---- unknown ife fusion dbg mode ----\n");
			}
		}
		break;
	case KDRV_IFE_DBG_SET_SSDRV_DBG_LV:
		{
#if IFE_SSDRV_SUPPORT
			UINT32 lv = *(UINT32 *)data;

			ife_eng_set_dbg_level(lv);
#endif
		}
		break;
	default:
		DBG_ERR("Unknown dbg mode id\n");
	break;

	}

	return ret;
}
