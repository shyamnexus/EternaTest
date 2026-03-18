#include "kwrap/util.h"
#include "comm/hwclock.h"
#include "kdrv_videoprocess/kdrv_ipp_539a.h"
#include "kdrv_ipp_int_tsk.h"
#include "kdrv_ipp_int.h"

#if defined (__KERNEL__)
#include <linux/slab.h>
#else
#include <malloc.h>
#endif


#define CAL_CHKSUM(arg) \
do{ \
	ptr = (UINT8 *)(&arg); \
	for(i=0; i < sizeof(arg); i++) \
		chk_sum -= *(ptr++); \
}while(0)

typedef INT32(*KDRV_IPP_JOB_PROC_FUNC)(KDRV_IPP_HANDLE*, KDRV_IPP_JOB_CFG *, KDRV_IPP_WRITE_REG_MODE, KDRV_IPP_LL_SET_INFO *);


void *kdrv_ipp_int_plat_malloc(UINT32 size)
{
#if defined (__KERNEL__)
	return kzalloc((size), GFP_KERNEL);
#elif defined(__FREERTOS)
	return malloc(size);
#endif
}

void kdrv_ipp_int_plat_free(void *ptr)
{
#if defined (__KERNEL__)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

extern KDRV_IPP_CTL g_kdrv_ipp_ctl;

INT32 kdrv_ipp_push_pre_hdl_queue(KDRV_IPP_HANDLE* p_hdl)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&g_kdrv_ipp_ctl.pre_hdl_queue_lock, loc_flg);
	g_kdrv_ipp_ctl.pre_hw_info[p_hdl->chip_id][p_hdl->eng_id].triggerd_hdl = p_hdl;
	vk_spin_unlock_irqrestore(&g_kdrv_ipp_ctl.pre_hdl_queue_lock, loc_flg);

	//DBG_DUMP("-push[%d][%d] pre:0x%lx-\n", p_hdl->chip_id, p_hdl->eng_id, (ULONG)p_hdl);

	return E_OK;
}

INT32 kdrv_ipp_push_ife_hdl_queue(KDRV_IPP_HANDLE* p_hdl)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&g_kdrv_ipp_ctl.ife_hdl_queue_lock, loc_flg);
	g_kdrv_ipp_ctl.ife_hw_info[p_hdl->chip_id][p_hdl->eng_id].triggerd_hdl = p_hdl;
	vk_spin_unlock_irqrestore(&g_kdrv_ipp_ctl.ife_hdl_queue_lock, loc_flg);

	//DBG_DUMP("-push[%d][%d] ife:0x%lx-\n", p_hdl->chip_id, p_hdl->eng_id, (ULONG)p_hdl);

	return E_OK;
}

INT32 kdrv_ipp_push_ipe_hdl_queue(KDRV_IPP_HANDLE* p_hdl)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&g_kdrv_ipp_ctl.ipe_hdl_queue_lock, loc_flg);
	g_kdrv_ipp_ctl.ipe_hw_info[p_hdl->chip_id][p_hdl->eng_id].triggerd_hdl = p_hdl;
	vk_spin_unlock_irqrestore(&g_kdrv_ipp_ctl.ipe_hdl_queue_lock, loc_flg);

	//DBG_DUMP("-push[%d][%d] ipe:0x%lx-\n", p_hdl->chip_id, p_hdl->eng_id, (ULONG)p_hdl);

	return E_OK;
}

INT32 kdrv_ipp_push_ime_hdl_queue(KDRV_IPP_HANDLE* p_hdl)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&g_kdrv_ipp_ctl.ime_hdl_queue_lock, loc_flg);
	g_kdrv_ipp_ctl.ime_hw_info[p_hdl->chip_id][p_hdl->eng_id].triggerd_hdl = p_hdl;
	vk_spin_unlock_irqrestore(&g_kdrv_ipp_ctl.ime_hdl_queue_lock, loc_flg);

	//DBG_DUMP("-push[%d][%d] ime:0x%lx-\n", p_hdl->chip_id, p_hdl->eng_id, (ULONG)p_hdl);

	return E_OK;
}

static INT32 kdrv_ipp_int_job_proc_pre(KDRV_IPP_HANDLE* p_hdl, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_WRITE_REG_MODE mode, KDRV_IPP_LL_SET_INFO *p_ll_blk)
{
	KDRV_PRE_JOB_CFG pre_cfg;
	UINT32 eng_id = 0;
	UINT32 dev_id = 0;

	eng_id = kdrv_ipp_get_pre_eng_id(p_hdl->eng_id);
	dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

	pre_cfg.p_iocfg = &p_cfg->pre_io_cfg;
	pre_cfg.p_iqcfg = &p_cfg->pre_iq_cfg;
	pre_cfg.p_ll_blk = (void *) p_ll_blk;

	pre_cfg.p_iocfg->in_job_num = p_hdl->job_in_cnt;
	pre_cfg.trig_op = p_hdl->trig_op;

	if(p_cfg->ipp_trig_status == KDRV_IPP_TRIG_STOP)
		pre_cfg.p_iocfg->ipp_status = KDRV_IPP_STOP;
	else
		pre_cfg.p_iocfg->ipp_status = KDRV_IPP_RUNNING;


	/* keep inte for callback, add default inte to each engine by mode
		pre only enable fmd/llend for d2d mode
		(690) d2d link-list mode patch. add ll_err2 isr to do sw reset & re-trigger ll when sie frame start comes during ll cmd transfer to hw reg, which can cause ipp eng hang
	*/
	p_cfg->cb_inte[KDRV_IPP_ENG_PRE] = pre_cfg.p_iocfg->inte_en;
	pre_cfg.p_iocfg->inte_en &= ~(KDRV_PRE_INTERRUPT_LLEND | KDRV_PRE_INTERRUPT_FMD | KDRV_PRE_INTERRUPT_SIE_FRAME_START);
	pre_cfg.p_iocfg->inte_en |= (KDRV_PRE_INTERRUPT_DEC_ERR1 | KDRV_PRE_INTERRUPT_DEC_ERR2 | KDRV_PRE_INTERRUPT_DEC_ERR3
								| KDRV_PRE_INTERRUPT_FRAME_ERR | KDRV_PRE_INTERRUPT_RING_BUF_ERR1 | KDRV_PRE_INTERRUPT_RING_BUF_ERR2
								| KDRV_PRE_INTERRUPT_BUFOVFL | KDRV_PRE_INTERRUPT_LLERR2);
								//| KDRV_PRE_INTERRUPT_PRE_BNR_ENC_OVFL | KDRV_PRE_INTERRUPT_PRE_BNR_DEC_ERR); need check

	//if(pre_cfg.p_iocfg->nn_isp_param.nn_isp_p0.fw_handshake_en || pre_cfg.p_iocfg->nn_isp_param.nn_isp_p1.fw_handshake_en){
		//pre_cfg.p_iocfg->inte_en |= KDRV_PRE_INTERRUPT_PRE_P0_SET_SLICE_RDY | KDRV_PRE_INTERRUPT_PRE_P0_GET_SLICE_CLR |
		//							KDRV_PRE_INTERRUPT_PRE_P1_SET_SLICE_RDY | KDRV_PRE_INTERRUPT_PRE_P1_GET_SLICE_CLR;
	//}
	if (mode == KDRV_IPP_WRITE_REG_CPU) {
		//if (pre_cfg.p_iocfg->mode == KDRV_IPP_OPMODE_D2D) {
			pre_cfg.p_iocfg->inte_en |= KDRV_PRE_INTERRUPT_FMD;
		//}
		if (kdrv_pre_set(dev_id, KDRV_PRE_PARAM_CFG_PROCESS_CPU, (void *)&pre_cfg) != E_OK) {
			DBG_ERR("pre cfg process by cpu fail\r\n");
			return E_PAR;  //to do : remove for dual eng test
		}
	} else if (mode == KDRV_IPP_WRITE_REG_LL) {
			pre_cfg.p_iocfg->inte_en |= KDRV_PRE_INTERRUPT_LLEND;

		if (kdrv_pre_set(dev_id, KDRV_PRE_PARAM_CFG_PROCESS_LL, (void *)&pre_cfg) != E_OK) {
			DBG_ERR("pre cfg process by ll fail\r\n");
			return E_PAR;
		}
	} else {
		DBG_ERR("unsupport write reg mode %d\r\n", mode);
		return E_PAR;
	}

	if (kdrv_pre_set(dev_id, KDRV_PRE_PARAM_SET_CB, (void *)kdrv_ipp_pre_isr) != E_OK) {
		DBG_ERR("pre hook cb fail\r\n");
		return E_PAR;
	}

	return E_OK;
}

static INT32 kdrv_ipp_int_job_proc_ife(KDRV_IPP_HANDLE* p_hdl, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_WRITE_REG_MODE mode, KDRV_IPP_LL_SET_INFO *p_ll_blk)
{
	KDRV_IFE_JOB_CFG ife_cfg;
	UINT32 eng_id = 0;
	UINT32 dev_id = 0;

	eng_id = kdrv_ipp_get_ife_eng_id(p_hdl->eng_id);
	dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

	ife_cfg.p_iocfg = &p_cfg->ife_io_cfg;
	ife_cfg.p_iqcfg = &p_cfg->ife_iq_cfg;
	ife_cfg.p_ll_blk = (void *) p_ll_blk;

	ife_cfg.p_iocfg->in_job_num = p_hdl->job_in_cnt;

	if(p_cfg->ipp_trig_status == KDRV_IPP_TRIG_STOP)
		ife_cfg.p_iocfg->ipp_status = KDRV_IPP_STOP;
	else
		ife_cfg.p_iocfg->ipp_status = KDRV_IPP_RUNNING;

	/* keep inte for callback, add default inte to each engine by mode
		ife only enable fmd/llend for d2d mode
		(690) d2d link-list mode patch. add ll_err2 isr to do sw reset & re-trigger ll when sie frame start comes during ll cmd transfer to hw reg, which can cause ipp eng hang
	*/
	p_cfg->cb_inte[KDRV_IPP_ENG_IFE] = ife_cfg.p_iocfg->inte_en;
	ife_cfg.p_iocfg->inte_en &= ~(KDRV_IFE_INTERRUPT_LLEND | KDRV_IFE_INTERRUPT_FMD | KDRV_IFE_INTERRUPT_SIE_FRAME_START);
	ife_cfg.p_iocfg->inte_en |= (KDRV_IFE_INTERRUPT_DEC_ERR1 | KDRV_IFE_INTERRUPT_DEC_ERR2 | KDRV_IFE_INTERRUPT_DEC_ERR3
								| KDRV_IFE_INTERRUPT_FRAME_ERR | KDRV_IFE_INTERRUPT_RING_BUF_ERR1 | KDRV_IFE_INTERRUPT_RING_BUF_ERR2
								| KDRV_IFE_INTERRUPT_BUFOVFL | KDRV_IFE_INTERRUPT_LLERR2);

	if(ife_cfg.p_iocfg->nn_isp_param.nn_isp_p0.fw_handshake_en || ife_cfg.p_iocfg->nn_isp_param.nn_isp_p1.fw_handshake_en){
		ife_cfg.p_iocfg->inte_en |= KDRV_IFE_INTERRUPT_IFE_P0_SET_SLICE_RDY | KDRV_IFE_INTERRUPT_IFE_P0_GET_SLICE_CLR |
									KDRV_IFE_INTERRUPT_IFE_P1_SET_SLICE_RDY | KDRV_IFE_INTERRUPT_IFE_P1_GET_SLICE_CLR;
	}
	if (mode == KDRV_IPP_WRITE_REG_CPU) {
		//if (ife_cfg.p_iocfg->mode == KDRV_IPP_OPMODE_D2D) {
			ife_cfg.p_iocfg->inte_en |= KDRV_IFE_INTERRUPT_FMD;
		//}
		if (kdrv_ife_set(dev_id, KDRV_IFE_PARAM_CFG_PROCESS_CPU, (void *)&ife_cfg) != E_OK) {
			DBG_ERR("ife cfg process by cpu fail\r\n");
			return E_PAR;  //to do : remove for dual eng test
		}
	} else if (mode == KDRV_IPP_WRITE_REG_LL) {
			ife_cfg.p_iocfg->inte_en |= KDRV_IFE_INTERRUPT_LLEND;

		if (kdrv_ife_set(dev_id, KDRV_IFE_PARAM_CFG_PROCESS_LL, (void *)&ife_cfg) != E_OK) {
			DBG_ERR("ife cfg process by ll fail\r\n");
			return E_PAR;
		}
	} else {
		DBG_ERR("unsupport write reg mode %d\r\n", mode);
		return E_PAR;
	}

	if (kdrv_ife_set(dev_id, KDRV_IFE_PARAM_SET_CB, (void *)kdrv_ipp_ife_isr) != E_OK) {
		DBG_ERR("ife hook cb fail\r\n");
		return E_PAR;
	}

	return E_OK;
}
#if 0 // 690 removed
static INT32 kdrv_ipp_int_job_proc_dce(KDRV_IPP_HANDLE* p_hdl, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_WRITE_REG_MODE mode, KDRV_IPP_LL_SET_INFO *p_ll_blk)
{
	KDRV_DCE_JOB_CFG dce_cfg;
	UINT32 eng_id = 0;
	UINT32 dev_id = 0;

	eng_id = kdrv_ipp_get_dce_eng_id(p_hdl->eng_id);
	dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

	dce_cfg.p_iocfg = &p_cfg->dce_io_cfg;
	dce_cfg.p_iqcfg = &p_cfg->dce_iq_cfg;
	dce_cfg.p_ll_blk = (void *) p_ll_blk;

	if(p_cfg->ipp_trig_status == KDRV_IPP_TRIG_STOP)
		dce_cfg.p_iocfg->ipp_status = KDRV_IPP_STOP;
	else
		dce_cfg.p_iocfg->ipp_status = KDRV_IPP_RUNNING;

	/* keep inte for callback, add default inte to each engine by mode */
	p_cfg->cb_inte[KDRV_IPP_ENG_DCE] = dce_cfg.p_iocfg->inte_en;
	dce_cfg.p_iocfg->inte_en |= (KDRV_DCE_INTE_FST | KDRV_DCE_INTE_FRAMEERR | KDRV_DCE_INTE_STPERR | KDRV_DCE_INTE_LBOVF);
	if (mode == KDRV_IPP_WRITE_REG_CPU) {
		dce_cfg.p_iocfg->inte_en |= KDRV_DCE_INTE_FMD;
		dce_cfg.p_iocfg->inte_en &= ~(KDRV_DCE_INTE_LL_END);
		if (kdrv_dce_set(dev_id, KDRV_DCE_PARAM_CFG_PROCESS_CPU, (void *)&dce_cfg) != E_OK) {
			DBG_ERR("dce cfg process by cpu fail\r\n");
			return E_PAR;
		}
	} else if (mode == KDRV_IPP_WRITE_REG_LL) {
		dce_cfg.p_iocfg->inte_en |= KDRV_DCE_INTE_LL_END;
		dce_cfg.p_iocfg->inte_en &= ~(KDRV_DCE_INTE_FMD);
		if (kdrv_dce_set(dev_id, KDRV_DCE_PARAM_CFG_PROCESS_LL, (void *)&dce_cfg) != E_OK) {
			DBG_ERR("dce cfg process by ll fail\r\n");
			return E_PAR;
		}
	} else {
		DBG_ERR("unsupport write reg mode %d\r\n", mode);
		return E_PAR;
	}

	if (kdrv_dce_set(dev_id, KDRV_DCE_PARAM_SET_CB, (void *)kdrv_ipp_dce_isr) != E_OK) {
		DBG_ERR("dce hook cb fail\r\n");
		return E_PAR;
	}

	return E_OK;
}
#endif
static INT32 kdrv_ipp_int_job_proc_ipe(KDRV_IPP_HANDLE* p_hdl, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_WRITE_REG_MODE mode, KDRV_IPP_LL_SET_INFO *p_ll_blk)
{
	KDRV_IPE_JOB_CFG ipe_cfg;
	UINT32 eng_id = 0;
	UINT32 dev_id = 0;

	eng_id = kdrv_ipp_get_ipe_eng_id(p_hdl->eng_id);
	dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

	ipe_cfg.p_iocfg = &p_cfg->ipe_io_cfg;
	ipe_cfg.p_iqcfg = &p_cfg->ipe_iq_cfg;
	ipe_cfg.p_ll_blk = (void *) p_ll_blk;

	ipe_cfg.p_iocfg->in_job_num = p_hdl->job_in_cnt;

	if(p_cfg->ipp_trig_status == KDRV_IPP_TRIG_STOP)
		ipe_cfg.p_iocfg->ipp_status = KDRV_IPP_STOP;
	else
		ipe_cfg.p_iocfg->ipp_status = KDRV_IPP_RUNNING;

	/* keep inte for callback, add default inte to each engine by mode */
	p_cfg->cb_inte[KDRV_IPP_ENG_IPE] = ipe_cfg.p_iocfg->inte_en;
	ipe_cfg.p_iocfg->inte_en |= (KDRV_IPE_INTERRUPT_FMS | KDRV_IPE_INTERRUPT_FRAMEERR);

	//538 add
	ipe_cfg.p_iocfg->inte_en |= KDRV_IPE_INTE_ENC_OUTOVFL;
	//539a add
	ipe_cfg.p_iocfg->inte_en |= KDRV_IPE_INTE_MOTION_END;

	if (mode == KDRV_IPP_WRITE_REG_CPU) {
		ipe_cfg.p_iocfg->inte_en |= KDRV_IPE_INTERRUPT_FMD;
		ipe_cfg.p_iocfg->inte_en &= ~(KDRV_IPE_INTERRUPT_LL_DONE);
		if (kdrv_ipe_set(dev_id, KDRV_IPE_PARAM_CFG_PROCESS_CPU, (void *)&ipe_cfg) != E_OK) {
			DBG_ERR("ipe cfg process by cpu fail\r\n");
			return E_PAR;
		}
	} else if (mode == KDRV_IPP_WRITE_REG_LL) {
		ipe_cfg.p_iocfg->inte_en |= KDRV_IPE_INTERRUPT_LL_DONE;
		ipe_cfg.p_iocfg->inte_en &= ~(KDRV_IPE_INTERRUPT_FMD);
		if (kdrv_ipe_set(dev_id, KDRV_IPE_PARAM_CFG_PROCESS_LL, (void *)&ipe_cfg) != E_OK) {
			DBG_ERR("ipe cfg process by ll fail\r\n");
			return E_PAR;
		}
	} else {
		DBG_ERR("unsupport write reg mode %d\r\n", mode);
		return E_PAR;
	}

	if (kdrv_ipe_set(dev_id, KDRV_IPE_PARAM_SET_CB, (void *)kdrv_ipp_ipe_isr) != E_OK) {
		DBG_ERR("ipe hook cb fail\r\n");
		return E_PAR;
	}

	return E_OK;
}

static INT32 kdrv_ipp_int_job_proc_ime(KDRV_IPP_HANDLE* p_hdl, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_WRITE_REG_MODE mode, KDRV_IPP_LL_SET_INFO *p_ll_blk)
{
	KDRV_IME_JOB_CFG ime_cfg;
	UINT32 eng_id = 0;
	UINT32 dev_id = 0;

	eng_id = kdrv_ipp_get_ime_eng_id(p_hdl->eng_id);
	dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

	ime_cfg.p_iocfg = &p_cfg->ime_io_cfg;
	ime_cfg.p_iqcfg = &p_cfg->ime_iq_cfg;
	ime_cfg.p_ll_blk = (void *)p_ll_blk;

	ime_cfg.p_iocfg->in_job_num = p_hdl->job_in_cnt;

	if(p_cfg->ipp_trig_status == KDRV_IPP_TRIG_STOP)
		ime_cfg.p_iocfg->ipp_status = KDRV_IPP_STOP;
	else
		ime_cfg.p_iocfg->ipp_status = KDRV_IPP_RUNNING;

	/* keep inte for callback, add default inte to each engine by mode */
	p_cfg->cb_inte[KDRV_IPP_ENG_IME] = ime_cfg.p_iocfg->inte_en;
	ime_cfg.p_iocfg->inte_en |= (KDRV_IME_INTERRUPT_FRM_START | KDRV_IME_INTERRUPT_FRM_ERR);

	if(ime_cfg.p_iocfg->nn_isp_param.nn_isp_p2.fw_handshake_en || ime_cfg.p_iocfg->nn_isp_param.nn_isp_p3.fw_handshake_en){
		ime_cfg.p_iocfg->inte_en |= KDRV_IME_INTERRUPT_NN_ISP2CPU_P2_OUT_READY | KDRV_IME_INTERRUPT_NN_ISP2CPU_P2_IN_CLEAR |
									KDRV_IME_INTERRUPT_NN_ISP2CPU_P3_OUT_READY | KDRV_IME_INTERRUPT_NN_ISP2CPU_P3_IN_CLEAR;
	}

	if (mode == KDRV_IPP_WRITE_REG_CPU) {
		ime_cfg.p_iocfg->inte_en |= KDRV_IME_INTERRUPT_FRM_END;
		ime_cfg.p_iocfg->inte_en &= ~(KDRV_IME_INTERRUPT_LL_END);
		if (kdrv_ime_set(dev_id, KDRV_IME_PARAM_CFG_PROCESS_CPU, (void *)&ime_cfg) != E_OK) {
			DBG_ERR("ime cfg process by cpu fail\r\n");
			return E_PAR;
		}
	} else if (mode == KDRV_IPP_WRITE_REG_LL) {
		ime_cfg.p_iocfg->inte_en |= KDRV_IME_INTERRUPT_LL_END;
		ime_cfg.p_iocfg->inte_en &= ~(KDRV_IME_INTERRUPT_FRM_END);
		if (kdrv_ime_set(dev_id, KDRV_IME_PARAM_CFG_PROCESS_LL, (void *)&ime_cfg) != E_OK) {
			DBG_ERR("ime cfg process by ll fail\r\n");
			return E_PAR;
		}
	} else {
		DBG_ERR("unsupport write reg mode %d\r\n", mode);
		return E_PAR;
	}

	if (kdrv_ime_set(dev_id, KDRV_IME_PARAM_SET_CB, (void *)kdrv_ipp_ime_isr) != E_OK) {
		DBG_ERR("ime hook cb fail\r\n");
		return E_PAR;
	}

	return E_OK;
}

static KDRV_IPP_JOB_PROC_FUNC kdrv_ipp_int_proc_func[KDRV_IPP_ENG_MAX] = {
	kdrv_ipp_int_job_proc_pre,
	kdrv_ipp_int_job_proc_ife,
	//kdrv_ipp_int_job_proc_dce, // 690 removed
	kdrv_ipp_int_job_proc_ipe,
	kdrv_ipp_int_job_proc_ime,
};

INT32 kdrv_ipp_int_stripe_preprocess(KDRV_IPP_HANDLE* p_hdl, KDRV_IPP_JOB_CFG *p_cfg)
{
	UINT32 i = 0;
	INT32 rt = 0;
	KDRV_IPP_STRP_INFO info_cal = {0};


	//if ((p_cfg->wait_eng_mask /*& KDRV_IPP_ENG_DCE_IDLE*/) == 0)
	//	return E_OK;

	////// if nn_isp enabled, force ime scale to use bi-linear for stripe issue //////
	if(p_cfg->ife_io_cfg.nn_isp_param.nn_isp_p0.path_en ||
			p_cfg->ife_io_cfg.nn_isp_param.nn_isp_p1.path_en ||
			p_cfg->ime_io_cfg.nn_isp_param.nn_isp_p2.path_en ||
			p_cfg->ime_io_cfg.nn_isp_param.nn_isp_p3.path_en){


		p_cfg->ime_io_cfg.out_scl_method_sel.method_l = KDRV_IME_SCL_BILINEAR;
		p_cfg->ime_io_cfg.out_scl_method_sel.method_h = KDRV_IME_SCL_BILINEAR;
	}

	info_cal.cal_info.ime_op_mode = p_cfg->ime_io_cfg.mode;
	info_cal.cal_info.ime_in_fmt = p_cfg->ime_io_cfg.in_frm.fmt;
	info_cal.cal_info.ime_in_size.w = p_cfg->ime_io_cfg.in_frm.size.w;
	info_cal.cal_info.ime_in_size.h = p_cfg->ime_io_cfg.in_frm.size.h;
	info_cal.cal_info.lca_width = p_cfg->ime_io_cfg.lca.out_size.w;
	info_cal.cal_info._3dnr_en = p_cfg->ime_iq_cfg.tmnr.enable;
	info_cal.cal_info.lca_en = p_cfg->ime_io_cfg.lca.in_enable;
	info_cal.cal_info.pxl_subout_en = p_cfg->ime_io_cfg.pm_pxlimg.pxl_subout_en;

	for(i = 0; i < KDRV_IME_PATH_NUM_MAX; i++){
		info_cal.cal_info.ime_out_size[i].w = p_cfg->ime_io_cfg.out_path[i].scl_size.w;
		info_cal.cal_info.ime_out_size[i].h = p_cfg->ime_io_cfg.out_path[i].scl_size.h;

		info_cal.cal_info.ime_path_en[i] = p_cfg->ime_io_cfg.out_path[i].enable;
		info_cal.cal_info.ime_out_fmt[i] = p_cfg->ime_io_cfg.out_path[i].frm.fmt;
	}

	info_cal.cal_info.scl_method_sel.scl_th = p_cfg->ime_io_cfg.out_scl_method_sel.scl_th;
	info_cal.cal_info.scl_method_sel.method_l = p_cfg->ime_io_cfg.out_scl_method_sel.method_l;
	info_cal.cal_info.scl_method_sel.method_h = p_cfg->ime_io_cfg.out_scl_method_sel.method_h;

	if(p_cfg->pre_io_cfg.mode) {
		info_cal.cal_info.op_mode = p_cfg->pre_io_cfg.mode;
		info_cal.cal_info.in_width = p_cfg->pre_io_cfg.crp_window.w;
		info_cal.cal_info.in_height= p_cfg->pre_io_cfg.crp_window.h;
		info_cal.cal_info.ife_in_fmt = p_cfg->pre_io_cfg.in_frm.fmt;
	} else {
		info_cal.cal_info.op_mode = p_cfg->ife_io_cfg.mode;
		info_cal.cal_info.in_width = p_cfg->ife_io_cfg.crp_window.w;
		info_cal.cal_info.in_height= p_cfg->ife_io_cfg.crp_window.h;
		info_cal.cal_info.ife_in_fmt = p_cfg->ife_io_cfg.in_frm.fmt;
	}
	//539A add
	info_cal.cal_info.pre_in_fmt = p_cfg->pre_io_cfg.in_frm.fmt;
	info_cal.cal_info.pre_op_mode = p_cfg->pre_io_cfg.mode;
	info_cal.cal_info.ife_op_mode = p_cfg->ife_io_cfg.mode;
	info_cal.cal_info.ipe_op_mode = p_cfg->ipe_io_cfg.mode;
	if(p_cfg->pre_iq_cfg.bnr.bnr_enable)
		info_cal.cal_info.pre_bnr_enable = ENABLE;

	info_cal.cal_info.ife_nn_enable = p_cfg->ife_io_cfg.nn_isp_param.nn_isp_p0.path_en | p_cfg->ife_io_cfg.nn_isp_param.nn_isp_p1.path_en;

	if(p_cfg->ife_io_cfg.nn_isp_param.nn_isp_p0.nn_max_strp > p_cfg->ife_io_cfg.nn_isp_param.nn_isp_p1.nn_max_strp)
		info_cal.cal_info.ife_nn_max_strp_size = p_cfg->ife_io_cfg.nn_isp_param.nn_isp_p0.nn_max_strp;
	else
		info_cal.cal_info.ife_nn_max_strp_size = p_cfg->ife_io_cfg.nn_isp_param.nn_isp_p1.nn_max_strp;

	info_cal.cal_info.ime_nn_enable = p_cfg->ime_io_cfg.nn_isp_param.nn_isp_p2.path_en | p_cfg->ime_io_cfg.nn_isp_param.nn_isp_p3.path_en;

	if(p_cfg->ime_io_cfg.nn_isp_param.nn_isp_p2.nn_max_strp > p_cfg->ime_io_cfg.nn_isp_param.nn_isp_p3.nn_max_strp)
		info_cal.cal_info.ime_nn_max_strp_size = p_cfg->ime_io_cfg.nn_isp_param.nn_isp_p2.nn_max_strp;
	else
		info_cal.cal_info.ime_nn_max_strp_size = p_cfg->ime_io_cfg.nn_isp_param.nn_isp_p3.nn_max_strp;

	//info_cal.cal_info.crop_window = p_cfg->dce_io_cfg.out_img_info.crop_window;
	//info_cal.cal_info.sram_sel = p_cfg->dce_io_cfg.sram_sel;
	//info_cal.cal_info.stripe_type =  p_cfg->dce_iq_cfg.stripe_param.stripe_type;
	//info_cal.cal_info.strp_rule =  p_cfg->dce_iq_cfg.stripe_param.dce_strp_rule;
	info_cal.cal_info.strp_rule = p_cfg->ipp_io_cfg.strp_rule;

	rt |= kdrv_ipp_cal_stripe(p_hdl, &info_cal);

	p_hdl->ipp_dbg_info.dce_strp_rule = p_cfg->dce_iq_cfg.stripe_param.dce_strp_rule;

	/* config stripe information to each engine	*/
	/* dce stripe config */
	//p_cfg->dce_io_cfg.strp.info = stripe.info;

	for(i=0; i < KDRV_IPP_MAX_STP_NUM; i++){
		p_cfg->pre_io_cfg.pre_strp[i] = info_cal.rst.pre_in_strp[i]; //539A add
		p_cfg->ife_io_cfg.ife_strp[i] = info_cal.rst.ife_in_strp[i];
		p_cfg->ipe_io_cfg.ipe_strp[i] = info_cal.rst.ipe_in_strp[i];
		p_cfg->ime_io_cfg.ime_strp[i] = info_cal.rst.ime_in_strp[i];
	}

	/* set stripe size for d2d mode */
	p_cfg->ime_io_cfg.strp_max_width = 0;

	for(i=0; i < KDRV_IPP_MAX_STP_NUM; i++){

		if(info_cal.rst.ime_in_strp[i] >= p_cfg->ime_io_cfg.strp_max_width)
			p_cfg->ime_io_cfg.strp_max_width = info_cal.rst.ime_in_strp[i];
	}

		/* ipe stripe config */
	#if 0
	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IPE_IDLE) {

		switch (stripe.ipe_ovlp) {
			case 0:
				p_cfg->ipe_io_cfg.mst_h_ovlp = KDRV_IPE_OVLP_AUTO;
				break;

			case 1:
				p_cfg->ipe_io_cfg.mst_h_ovlp = KDRV_IPE_OVLP_16;
				break;

			default:
				DBG_ERR("unknown ipe overlap\r\n");
				rt = E_PAR;
				break;
		}
	}
	#else
	p_cfg->ipe_io_cfg.mst_h_ovlp = KDRV_IPE_OVLP_AUTO; // todo need check
	p_cfg->ipe_io_cfg.ime_ovlp = info_cal.rst.ime_ovlp >> 2;
	#endif

	//539A ADD
	p_cfg->pre_io_cfg.pre_strp_num = info_cal.rst.mod_stripe_num - 1;
	p_cfg->pre_io_cfg.pre_ovlp = (info_cal.rst.pre_ovlp + info_cal.rst.ife_ovlp + info_cal.rst.ipe_ovlp + info_cal.rst.ime_ovlp);// Pre need check

	p_cfg->ife_io_cfg.ife_strp_num = info_cal.rst.mod_stripe_num - 1;
	p_cfg->ife_io_cfg.ife_ovlp = (info_cal.rst.ife_ovlp + info_cal.rst.ipe_ovlp + info_cal.rst.ime_ovlp);

	p_cfg->ipe_io_cfg.ipe_strp_num = info_cal.rst.mod_stripe_num - 1;
	p_cfg->ipe_io_cfg.ipe_ovlp = info_cal.rst.ipe_ovlp + info_cal.rst.ime_ovlp;

	p_cfg->ime_io_cfg.ime_strp_num = info_cal.rst.mod_stripe_num - 1;
	p_cfg->ime_io_cfg.ime_ovlp = info_cal.rst.ime_ovlp;

	p_cfg->ime_io_cfg.ime_qry_rst = info_cal.rst.ime_qry_rst;

	/* ime stripe config */
	#if 0
	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IME_IDLE) {
		p_cfg->ime_io_cfg.strp_max_width = 0;
		for (i = 0; i < stripe.info.hstp_num; i++) {
			if (stripe.ui_ime_in_hstp[i] > p_cfg->ime_io_cfg.strp_max_width) {
				p_cfg->ime_io_cfg.strp_max_width = stripe.ui_ime_in_hstp[i];
			}
		}

		switch (stripe.ime_ovlp) {
			case 0:
				p_cfg->ime_io_cfg.strp_ovlp = 16;
				p_cfg->ime_io_cfg.strp_prt = 5;
				break;

			case 1:
				p_cfg->ime_io_cfg.strp_ovlp = 24;
				p_cfg->ime_io_cfg.strp_prt = 5;
				break;

			case 2:
				p_cfg->ime_io_cfg.strp_ovlp = 32;
				p_cfg->ime_io_cfg.strp_prt = 5;
				break;

			case 3:
				/*  ovlp 256 -> prt 128
					ovlp 128 ->prt 64
				*/
				p_cfg->ime_io_cfg.strp_ovlp = stripe.info.hstp_ime_usr_val;
				p_cfg->ime_io_cfg.strp_prt = (p_cfg->ime_io_cfg.strp_ovlp >> 1);
				break;

			default:
				DBG_ERR("unknown ime overlap %d\r\n", (int)stripe.info.hstp_imeolap);
				rt = E_PAR;
				break;
		}

	}
	#endif

	/* check stripe related limitation */
	if (info_cal.rst.mod_stripe_num > 1) {
		/* mst not support yuv compress + scale */
		for (i = 0; i < KDRV_IME_PATH_NUM_MAX; i++) {
			if (p_cfg->ime_io_cfg.out_path[i].enable && KDRV_IPP_FMT_CLASS(p_cfg->ime_io_cfg.out_path[i].frm.fmt) == KDRV_IPP_FMT_CLASS_NVX) {
				if (p_cfg->ime_io_cfg.out_path[i].scl_size.w != p_cfg->ime_io_cfg.in_frm.size.w ||
					p_cfg->ime_io_cfg.out_path[i].scl_size.h != p_cfg->ime_io_cfg.in_frm.size.h ||
					p_cfg->ime_io_cfg.out_path[i].post_crp_window.x != 0 ||
					p_cfg->ime_io_cfg.out_path[i].post_crp_window.y != 0 ||
					p_cfg->ime_io_cfg.out_path[i].post_crp_window.w !=  p_cfg->ime_io_cfg.out_path[i].scl_size.w ||
					p_cfg->ime_io_cfg.out_path[i].post_crp_window.h !=  p_cfg->ime_io_cfg.out_path[i].scl_size.h) {
					DBG_ERR("yuv compress not support scale/crop when multi-stripe\r\n");
					rt = E_PAR;
					break;
				}
			}
		}
	}

	return rt;
}
#if 0
static void kdrv_ipp_int_load_gamma(KDRV_IPP_JOB_CFG *p_cfg, UINT8 is_ll_mode)
{
	KDRV_IPE_GAMMA_LOAD_INFO load_info;
	UINT32 dev_id;
	BOOL b_gamma_load = FALSE, b_ycurve_load = FALSE;

	if (p_cfg->ipe_iq_cfg.gamma.enable && (p_cfg->ipe_iq_cfg.update & KDRV_IPE_UPDATE_GAMMA)) {
		b_gamma_load = TRUE;
	}

	if (p_cfg->ipe_iq_cfg.y_curve.enable && (p_cfg->ipe_iq_cfg.update & KDRV_IPE_UPDATE_YCURVE)) {
		b_ycurve_load = TRUE;
	}

	/* load ipe gamma */
	if (b_gamma_load || b_ycurve_load) {
		if (b_gamma_load && b_ycurve_load) {
			if (p_cfg->ipe_iq_cfg.gamma.option == KDRV_IPE_GAMMA_RGB_SEPERATE) {
				DBG_WRN("seperate do not surpport\r\n");
			} else {
				load_info.type = KDRV_IPE_GAMMA_R_Y;
			}
		} else if (b_gamma_load){
			load_info.type = KDRV_IPE_GAMMA_R;
		} else {
			load_info.type = KDRV_IPE_GAMMA_Y;
		}

		if (is_ll_mode) {
			load_info.gamma_addr = (ULONG)p_cfg->ipe_io_cfg.gamma_addr.va;
			load_info.ycurve_addr = (ULONG)p_cfg->ipe_io_cfg.ycurve_addr.va;
			load_info.gamma_phyaddr = (ULONG)p_cfg->ipe_io_cfg.gamma_addr.pa;
			load_info.ycurve_phyaddr = (ULONG)p_cfg->ipe_io_cfg.ycurve_addr.pa;
		} else {
			load_info.gamma_addr = 0;
			load_info.ycurve_addr = 0;
		}

		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IPE_ENGINE0, 0);
		kdrv_ipe_set(dev_id, KDRV_IPE_PARAM_LOAD_GAMMA, (void *)&load_info);
	}
	return;
}
#endif
INT32 kdrv_ipp_job_trigger_eng_d2d(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_CFG *p_cfg, void *reserve)
{
	UINT32 dev_id;

	//539a reorder from ime to pre
	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IME_IDLE) {
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IME_ENGINE0, 0);
		if (kdrv_ime_set(dev_id, KDRV_IME_PARAM_TRIG_SINGLE, 0) != E_OK) {
			DBG_ERR("ime trig single fail\r\n");
			return E_SYS;
		}
	}

	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IPE_IDLE) {
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IPE_ENGINE0, 0);
		if (kdrv_ipe_set(dev_id, KDRV_IPE_PARAM_TRIG_SINGLE, 0) != E_OK) {
			DBG_ERR("ipe trig single fail\r\n");
			return E_SYS;
		}
	}

	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IFE_IDLE) {
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IFE_ENGINE0, 0);
		if (kdrv_ife_set(dev_id, KDRV_IFE_PARAM_TRIG_SINGLE, 0) != E_OK) {
			DBG_ERR("ife trig single fail\r\n");
			return E_SYS;
		}
		/* special check for ife mask
			remove frame start bit
			remove frame end bit if not d2d mode
		*/
		p_cfg->start_eng_mask &= ~(KDRV_IPP_ENG_IFE_IDLE);
		if (p_cfg->ife_io_cfg.mode != KDRV_IPP_OPMODE_D2D) {
			p_cfg->wait_eng_mask &= ~(KDRV_IPP_ENG_IFE_IDLE);
		}
	}

	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_PRE_IDLE) {
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_PRE_ENGINE0, 0);
		if (kdrv_pre_set(dev_id, KDRV_PRE_PARAM_TRIG_SINGLE, 0) != E_OK) {
			DBG_ERR("pre trig single fail\r\n");
			return E_SYS;
		}

		/* special check for pre mask
			remove frame start bit
			remove frame end bit if not d2d mode
		*/
		p_cfg->start_eng_mask &= ~(KDRV_IPP_ENG_PRE_IDLE);
		if (p_cfg->pre_io_cfg.mode != KDRV_IPP_OPMODE_D2D) {
			p_cfg->wait_eng_mask &= ~(KDRV_IPP_ENG_PRE_IDLE);
		}
	}

	#if 0 // 690 removed
	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_DCE_IDLE) {
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_DCE_ENGINE0, 0);
		if (kdrv_dce_set(dev_id, KDRV_DCE_PARAM_TRIG_SINGLE, 0) != E_OK) {
			DBG_ERR("dce trig single fail\r\n");
			return E_SYS;
		}
	}
	#endif
	return E_OK;
}

INT32 kdrv_ipp_job_trigger_eng_direct(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_TRIG_PARAM *p_trig_param)
{
	UINT32 dev_id;
	UINT32 eng_id;
	UINT32 is_dir_start;

	/* direct mode set trigger start at ipp start*/
	if (p_trig_param->op == KDRV_IPP_TRIG_START) {
		is_dir_start = TRUE;
	} else {
		is_dir_start = FALSE;
	}

	/* skip eng start for fastboot flow
		rewrite wait_eng bit for first frame
		set ime_builtin start switch to hdal
	*/
#if FPGA_BUILTIN_SUPPORT
	if (kdrv_ipp_builtin_get_status() && is_dir_start) {
		p_cfg->wait_eng_mask = KDRV_IPP_ENG_IME_IDLE;
		p_cfg->start_eng_mask = KDRV_IPP_ENG_IME_IDLE;

		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IME_ENGINE0, 0);
		ime_builtin_set_switch_to_hdal(dev_id);
		return E_OK;
	}
#endif

	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IME_IDLE) {

		eng_id = kdrv_ipp_get_ime_eng_id(p_hdl->eng_id);
		dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

		kdrv_ipp_push_ime_hdl_queue(p_hdl);

		if (kdrv_ime_set(dev_id, KDRV_IME_PARAM_TRIG_DIR, &is_dir_start) != E_OK) {
			DBG_ERR("ime trig dir fail\r\n");
			return E_SYS;
		}
	}

	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IPE_IDLE) {

		eng_id = kdrv_ipp_get_ipe_eng_id(p_hdl->eng_id);
		dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

		kdrv_ipp_push_ipe_hdl_queue(p_hdl);

		if (kdrv_ipe_set(dev_id, KDRV_IPE_PARAM_TRIG_DIR, &is_dir_start) != E_OK) {
			DBG_ERR("ipe trig dir fail\r\n");
			return E_SYS;
		}

	}

	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IFE_IDLE) {

		eng_id = kdrv_ipp_get_ife_eng_id(p_hdl->eng_id);
		dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

#if (FUNC_MOVE2PRE == 0)
		/* clear ife ringbuf error when trig start */
		if (is_dir_start) {
			if (kdrv_ife_set(dev_id, KDRV_IFE_PARAM_CLR_RING_BUF_LINE_CNT, NULL) != E_OK){
				DBG_ERR("clr_ring_buf_line fail\r\n");
				return E_SYS;
			}
		}
#endif

		kdrv_ipp_push_ife_hdl_queue(p_hdl);

		if (kdrv_ife_set(dev_id, KDRV_IFE_PARAM_TRIG_DIR, &is_dir_start) != E_OK) {
			DBG_ERR("ife trig dir fail\r\n");
			return E_SYS;
		}
#if 1
		/* special check for ife mask
			remove frame start bit
			remove frame end bit if not d2d mode
		*/
		p_cfg->start_eng_mask &= ~(KDRV_IPP_ENG_IFE_IDLE);
		if (p_cfg->ife_io_cfg.mode != KDRV_IPP_OPMODE_D2D) {
			p_cfg->wait_eng_mask &= ~(KDRV_IPP_ENG_IFE_IDLE);
		}
#endif
	}

	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_PRE_IDLE) {

		eng_id = kdrv_ipp_get_pre_eng_id(p_hdl->eng_id);
		dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

		/* clear pre ringbuf error when trig start */
		if (is_dir_start) {
			if (kdrv_pre_set(dev_id, KDRV_PRE_PARAM_CLR_RING_BUF_LINE_CNT, NULL) != E_OK){
				DBG_ERR("clr_ring_buf_line fail\r\n");
				return E_SYS;
			}
		}

		kdrv_ipp_push_pre_hdl_queue(p_hdl);

		if (kdrv_pre_set(dev_id, KDRV_PRE_PARAM_TRIG_DIR, &is_dir_start) != E_OK) {
			DBG_ERR("pre trig dir fail\r\n");
			return E_SYS;
		}

		/* special check for pre mask
			remove frame start bit
			remove frame end bit if not d2d mode
		*/
		p_cfg->start_eng_mask &= ~(KDRV_IPP_ENG_PRE_IDLE);

		if (p_cfg->pre_io_cfg.mode != KDRV_IPP_OPMODE_D2D) {
			p_cfg->wait_eng_mask &= ~(KDRV_IPP_ENG_PRE_IDLE);
		}
	}

	#if 0 // 690 removed
	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_DCE_IDLE) {
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_DCE_ENGINE0, 0);
		if (kdrv_dce_set(dev_id, KDRV_DCE_PARAM_TRIG_DIRECT, &is_dir_start) != E_OK) {
			DBG_ERR("dce trig dir fail\r\n");
			return E_SYS;
		}
	}
	#endif
	return E_OK;
}

INT32 kdrv_ipp_job_stop_eng_direct(KDRV_IPP_HANDLE *p_hdl, void *reserve)
{
	UINT32 dev_id = 0;
	UINT32 eng_id = 0;

	eng_id = kdrv_ipp_get_pre_eng_id(p_hdl->eng_id);
	dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
	if (kdrv_pre_set(dev_id, KDRV_PRE_PARAM_STOP_SINGLE, 0) != E_OK) {
		DBG_ERR("pre stop fail\r\n");
		return E_SYS;
	}

	eng_id = kdrv_ipp_get_ife_eng_id(p_hdl->eng_id);
	dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
	if (kdrv_ife_set(dev_id, KDRV_IFE_PARAM_STOP_SINGLE, 0) != E_OK) {
		DBG_ERR("ife stop fail\r\n");
		return E_SYS;
	}

	eng_id = kdrv_ipp_get_ipe_eng_id(p_hdl->eng_id);
	dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
	if (kdrv_ipe_set(dev_id, KDRV_IPE_PARAM_STOP_SINGLE, 0) != E_OK) {
		DBG_ERR("ipe stop fail\r\n");
		return E_SYS;
	}

	eng_id = kdrv_ipp_get_ime_eng_id(p_hdl->eng_id);
	dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
	if (kdrv_ime_set(dev_id, KDRV_IME_PARAM_STOP_SINGLE, 0) != E_OK) {
		DBG_ERR("ime stop fail\r\n");
		return E_SYS;
	}
	#if 0 // 690 removed
	dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_DCE_ENGINE0, 0);
	if (kdrv_dce_set(dev_id, KDRV_DCE_PARAM_STOP_SINGLE, 0) != E_OK) {
		DBG_ERR("dce stop fail\r\n");
		return E_SYS;
	}
	#endif

	return E_OK;
}

UINT32 kdrv_ipp_dtsi_checksum_cal(KDRV_IPP_JOB_CFG* p_ipp_cfg)
{
	ULONG i=0;

	UINT32 chk_sum=0;
	UINT8* ptr = NULL;

	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.mode);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.in_frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.in_frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.in_frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.out_frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.out_frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.out_frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.crp_window.x);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.crp_window.y);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.crp_window.w);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.crp_window.h);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.rde.encode_rate);
	//CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.ring_buf.en);
	CAL_CHKSUM(p_ipp_cfg->ife_io_cfg.mirror.en);

	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.mode);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.func_en);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.in_frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.in_frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.in_frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.out_img_info.crop_window.x);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.out_img_info.crop_window.y);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.out_img_info.crop_window.w);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.out_img_info.crop_window.h);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.out_img_info.frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.out_img_info.frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.out_img_info.frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.cfa_sub.cfa_subout_enable);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.cfa_sub.cfa_subout_flip_enable);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.cfa_sub.subout_byte);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.cfa_sub.subout_ch_sel);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.cfa_sub.subout_shiftbit);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.sram_sel);
	CAL_CHKSUM(p_ipp_cfg->dce_io_cfg.sub_out_en);

	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.mode);
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.in_frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.in_frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.in_frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.out_frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.out_frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.out_frm.size.h);
#if(IPE_538_KDRV_FEATURE == 1)
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.sub_out_en);
#else
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.defog_sub_out_en);
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.lce_sub_out_en);
	CAL_CHKSUM(p_ipp_cfg->ipe_io_cfg.eth_info.enable);
#endif

	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.mode);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.in_frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.in_frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.in_frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].post_crp_window.x);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].post_crp_window.y);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].post_crp_window.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].post_crp_window.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].dma_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].flip_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].scl_size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].scl_size.h);
	//CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[0].sprt_en); // 690 removed

	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].post_crp_window.x);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].post_crp_window.y);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].post_crp_window.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].post_crp_window.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].dma_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].flip_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].scl_size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].scl_size.h);
	//CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[1].sprt_en); // 690 removed

	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].post_crp_window.x);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].post_crp_window.y);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].post_crp_window.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].post_crp_window.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].dma_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].flip_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].scl_size);
	//CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[2].sprt_en); // 690 removed

	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].post_crp_window.x);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].post_crp_window.y);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].post_crp_window.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].post_crp_window.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].dma_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].flip_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].scl_size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].scl_size.h);
	//CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[3].sprt_en); // 690 removed

	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].post_crp_window.x);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].post_crp_window.y);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].post_crp_window.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].post_crp_window.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].dma_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].flip_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].scl_size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].scl_size.h);
	//CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_path[4].sprt_en); // 690 removed

	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.out_scl_method_sel);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg._3dnr.in_ref_flip_enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg._3dnr.in_ref_frm.fmt);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg._3dnr.in_ref_frm.size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg._3dnr.in_ref_frm.size.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg._3dnr.out_ms_roi_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg._3dnr.out_ms_roi_flip_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg._3dnr.out_sta_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.lca.in_enable);
	//CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.lca.out_en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.lca.in_size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.lca.in_size.h);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.lca.out_size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.lca.out_size.h);
	//CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.lca.in_bypass);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.low_delay.en);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.low_delay.pid);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_mask[0].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_mask[1].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_mask[2].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_mask[3].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_mask[4].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_mask[5].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_mask[6].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_mask[7].enable);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_pxlimg.img_size.w);
	CAL_CHKSUM(p_ipp_cfg->ime_io_cfg.pm_pxlimg.img_size.h);

	return chk_sum;
}

static INT32 kdrv_ipp_do_job_start_cb_cpu(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_TRIG_PARAM *p_trig_param)
{
	void *p_handle = NULL;
	UINT32 param_id=0;
	void *data = NULL;

	UINT32 dev_id=0;
	UINT32 eng_id=0;
	UINT32 i=0;
	ULONG pre_reg_base=0,ife_reg_base=0,ipe_reg_base=0,ime_reg_base=0;
	UINT32 pre_reg_num=0,ife_reg_num=0,ipe_reg_num=0,ime_reg_num=0;
	KDRV_IPP_DTSI_ENG_INFO eng_info[KDRV_IPP_ENG_MAX];
	//KDRV_IPP_DTSI_INFO dtsi_info;

	KDRV_IPP_DTSI_INFO *dtsi_info;
	dtsi_info = p_hdl->dtsi_info;

	switch(p_trig_param->job_st_op){

		case KDRV_IPP_JOB_ST_OP_GET_DTSI_INFO:

			dtsi_info->eng_info = eng_info;
			dtsi_info->eng_num = KDRV_IPP_ENG_MAX;

			p_handle = (void *)&p_trig_param->job_owner;
			param_id = KDRV_IPP_JOB_ST_OP_GET_DTSI_INFO;
			data = (void *)dtsi_info;

			eng_id = kdrv_ipp_get_pre_eng_id(p_hdl->eng_id);
			dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
			kdrv_pre_get(dev_id, KDRV_PRE_PARAM_GET_REG_BASE_ADDR, (void *) &pre_reg_base);
			kdrv_pre_query(dev_id, KDRV_PRE_QUERY_REG_NUM, (void *)&pre_reg_num);
			eng_info[KDRV_IPP_ENG_PRE].reg_base_addr = pre_reg_base;
			eng_info[KDRV_IPP_ENG_PRE].reg_num = pre_reg_num;
			eng_info[KDRV_IPP_ENG_PRE].id = KDRV_IPP_ENG_PRE;


			eng_id = kdrv_ipp_get_ife_eng_id(p_hdl->eng_id);
			dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
			kdrv_ife_get(dev_id, KDRV_IFE_PARAM_GET_REG_BASE_ADDR, (void *) &ife_reg_base);
			kdrv_ife_query(dev_id, KDRV_IFE_QUERY_REG_NUM, (void *)&ife_reg_num);
			eng_info[KDRV_IPP_ENG_IFE].reg_base_addr = ife_reg_base;
			eng_info[KDRV_IPP_ENG_IFE].reg_num = ife_reg_num;
			eng_info[KDRV_IPP_ENG_IFE].id = KDRV_IPP_ENG_IFE;

			#if 0 // 690 removed
			dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_DCE_ENGINE0, 0);
			kdrv_dce_get(dev_id, KDRV_DCE_PARAM_GET_REG_BASE_ADDR, (void *) &dce_reg_base);
			kdrv_dce_query(dev_id, KDRV_DCE_QUERY_REG_NUM, (void *)&dce_reg_num);
			eng_info[KDRV_IPP_ENG_DCE].reg_base_addr = dce_reg_base;
			eng_info[KDRV_IPP_ENG_DCE].reg_num = dce_reg_num;
			eng_info[KDRV_IPP_ENG_DCE].id = KDRV_IPP_ENG_DCE;
			#endif
			dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IPE_ENGINE0, 0);
			eng_id = kdrv_ipp_get_ipe_eng_id(p_hdl->eng_id);
			dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
			kdrv_ipe_get(dev_id, KDRV_IPE_PARAM_GET_REG_BASE_ADDR, (void *) &ipe_reg_base);
			kdrv_ipe_query(dev_id, KDRV_IPE_QUERY_REG_NUM, (void *)&ipe_reg_num);
			eng_info[KDRV_IPP_ENG_IPE].reg_base_addr = ipe_reg_base;
			eng_info[KDRV_IPP_ENG_IPE].reg_num = ipe_reg_num;
			eng_info[KDRV_IPP_ENG_IPE].id = KDRV_IPP_ENG_IPE;

			eng_id = kdrv_ipp_get_ime_eng_id(p_hdl->eng_id);
			dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
			kdrv_ime_get(dev_id, KDRV_IME_PARAM_GET_REG_BASE_ADDR, (void *) &ime_reg_base);
			kdrv_ime_query(dev_id, KDRV_IME_QUERY_REG_NUM, (void *)&ime_reg_num);
			eng_info[KDRV_IPP_ENG_IME].reg_base_addr = ime_reg_base;
			eng_info[KDRV_IPP_ENG_IME].reg_num = ime_reg_num;
			eng_info[KDRV_IPP_ENG_IME].id = KDRV_IPP_ENG_IME;

			for(i = 0; i < KDRV_IME_PM_SET_IDX_MAX ; i++){
				dtsi_info->pm_info[i] = p_hdl->p_fired_cfg->ime_io_cfg.pm_mask[i];


				//DBG_DUMP("---mask(%d,%x,%x,%x,%x,%x)---\n",(unsigned int)dtsi_info.pm_info[i].enable,
				//	(unsigned int)dtsi_info.pm_info[i].color[0],
				//	(unsigned int)dtsi_info.pm_info[i].msk_type,
				//	(unsigned int)dtsi_info.pm_info[i].weight,
				//	(unsigned int)dtsi_info.pm_info[i].coord[0].x,
				//	(unsigned int)dtsi_info.pm_info[i].coord2[0].x);
			}

			dtsi_info->dtsi_checksum =  kdrv_ipp_dtsi_checksum_cal(p_hdl->p_fired_cfg);

		break;
		case KDRV_IPP_JOB_ST_OP_CHK_DTSI_CHKSUM:
		{
			UINT32 checksum=0;
			KDRV_IPP_BUILTIN_DBG_DTSI_CHKSUM dtsi_dbg_info = {0,0};

			dtsi_dbg_info.id = p_hdl->p_fired_cfg->isp_id;
#if FPGA_BUILTIN_SUPPORT
			kdrv_ipp_builtin_get(NULL, KDRV_IPP_PARAM_BUILTIN_GET_DTSI_CHKSUM, &dtsi_dbg_info);
#endif
			checksum = kdrv_ipp_dtsi_checksum_cal(p_hdl->p_fired_cfg);

			if(checksum != dtsi_dbg_info.chksum)
				DBG_DUMP("[fastboot Note]  dtsi(0x%x) & HDAL(0x%x)\n", dtsi_dbg_info.chksum, checksum);
			else
				DBG_DUMP("[fastboot Note]  dtsi chksum pass(0x%x,0x%x)\n",dtsi_dbg_info.chksum, checksum);
		}
		break;
		default:
		break;

	}


	if(p_trig_param->job_start_cb)
		p_trig_param->job_start_cb(p_handle, param_id, data);

	return E_OK;
}

static INT32 kdrv_ipp_do_job_start_cb_ll(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_TRIG_PARAM *p_trig_param)
{
	void *p_handle = NULL;
	UINT32 param_id=0;
	void *data = NULL;

	UINT32 dev_id=0;
	UINT32 eng_id=0;
	UINT32 i=0;
	ULONG pre_reg_base=0,ife_reg_base=0,ipe_reg_base=0,ime_reg_base=0;
	UINT32 pre_reg_num=0,ife_reg_num=0,ipe_reg_num=0,ime_reg_num=0;
	KDRV_IPP_DTSI_ENG_INFO eng_info[KDRV_IPP_ENG_MAX];
	//KDRV_IPP_DTSI_INFO dtsi_info;

	KDRV_IPP_DTSI_INFO *dtsi_info;
	dtsi_info = p_hdl->dtsi_info;

	switch(p_trig_param->job_st_op){

		case KDRV_IPP_JOB_ST_OP_GET_DTSI_INFO:

			dtsi_info->eng_info = eng_info;
			dtsi_info->eng_num = KDRV_IPP_ENG_MAX;

			p_handle = (void *)&p_trig_param->job_owner;
			param_id = KDRV_IPP_JOB_ST_OP_GET_DTSI_INFO;
			data = (void *)dtsi_info;

			eng_id = kdrv_ipp_get_pre_eng_id(p_hdl->eng_id);
			dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
			kdrv_pre_get(dev_id, KDRV_PRE_PARAM_GET_REG_BASE_ADDR_LL, (void *) &pre_reg_base);
			kdrv_pre_query(dev_id, KDRV_PRE_QUERY_REG_NUM, (void *)&pre_reg_num);
			eng_info[KDRV_IPP_ENG_PRE].reg_base_addr = pre_reg_base;
			eng_info[KDRV_IPP_ENG_PRE].reg_num = pre_reg_num;
			eng_info[KDRV_IPP_ENG_PRE].id = KDRV_IPP_ENG_PRE;

			eng_id = kdrv_ipp_get_ife_eng_id(p_hdl->eng_id);
			dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
			kdrv_ife_get(dev_id, KDRV_IFE_PARAM_GET_REG_BASE_ADDR_LL, (void *) &ife_reg_base);
			kdrv_ife_query(dev_id, KDRV_IFE_QUERY_REG_NUM, (void *)&ife_reg_num);
			eng_info[KDRV_IPP_ENG_IFE].reg_base_addr = ife_reg_base;
			eng_info[KDRV_IPP_ENG_IFE].reg_num = ife_reg_num;
			eng_info[KDRV_IPP_ENG_IFE].id = KDRV_IPP_ENG_IFE;

			#if 0 // 690 removed
			dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_DCE_ENGINE0, 0);
			kdrv_dce_get(dev_id, KDRV_DCE_PARAM_GET_REG_BASE_ADDR_LL, (void *) &dce_reg_base);
			kdrv_dce_query(dev_id, KDRV_DCE_QUERY_REG_NUM, (void *)&dce_reg_num);
			eng_info[KDRV_IPP_ENG_DCE].reg_base_addr = dce_reg_base;
			eng_info[KDRV_IPP_ENG_DCE].reg_num = dce_reg_num;
			eng_info[KDRV_IPP_ENG_DCE].id = KDRV_IPP_ENG_DCE;
			#endif
			eng_id = kdrv_ipp_get_ipe_eng_id(p_hdl->eng_id);
			dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
			kdrv_ipe_get(dev_id, KDRV_IPE_PARAM_GET_REG_BASE_ADDR_LL, (void *) &ipe_reg_base);
			kdrv_ipe_query(dev_id, KDRV_IPE_QUERY_REG_NUM, (void *)&ipe_reg_num);
			eng_info[KDRV_IPP_ENG_IPE].reg_base_addr = ipe_reg_base;
			eng_info[KDRV_IPP_ENG_IPE].reg_num = ipe_reg_num;
			eng_info[KDRV_IPP_ENG_IPE].id = KDRV_IPP_ENG_IPE;

			eng_id = kdrv_ipp_get_ime_eng_id(p_hdl->eng_id);
			dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);
			kdrv_ime_get(dev_id, KDRV_IME_PARAM_GET_REG_BASE_ADDR_LL, (void *) &ime_reg_base);
			kdrv_ime_query(dev_id, KDRV_IME_QUERY_REG_NUM, (void *)&ime_reg_num);
			eng_info[KDRV_IPP_ENG_IME].reg_base_addr = ime_reg_base;
			eng_info[KDRV_IPP_ENG_IME].reg_num = ime_reg_num;
			eng_info[KDRV_IPP_ENG_IME].id = KDRV_IPP_ENG_IME;

			for(i = 0; i < KDRV_IME_PM_SET_IDX_MAX ; i++){
				dtsi_info->pm_info[i] = p_hdl->p_ll_fired_cfg->ime_io_cfg.pm_mask[i];

				//DBG_DUMP("---ll mask(%d,%x,%x,%x,%x,%x)---\n",(unsigned int)dtsi_info.pm_info[i].enable,
				//	(unsigned int)dtsi_info.pm_info[i].color[0],
				//	(unsigned int)dtsi_info.pm_info[i].msk_type,
				//	(unsigned int)dtsi_info.pm_info[i].weight,
				//	(unsigned int)dtsi_info.pm_info[i].coord[0].x,
				//	(unsigned int)dtsi_info.pm_info[i].coord2[0].x);
			}

			dtsi_info->dtsi_checksum = kdrv_ipp_dtsi_checksum_cal(p_hdl->p_ll_fired_cfg);

		break;
		case KDRV_IPP_JOB_ST_OP_CHK_DTSI_CHKSUM:
		{
			UINT32 checksum=0;
			KDRV_IPP_BUILTIN_DBG_DTSI_CHKSUM dtsi_dbg_info = {0,0};

			dtsi_dbg_info.id = p_hdl->p_ll_fired_cfg->isp_id;
#if FPGA_BUILTIN_SUPPORT
			kdrv_ipp_builtin_get(NULL, KDRV_IPP_PARAM_BUILTIN_GET_DTSI_CHKSUM, &dtsi_dbg_info);
#endif
			checksum = kdrv_ipp_dtsi_checksum_cal(p_hdl->p_ll_fired_cfg);

			if(checksum != dtsi_dbg_info.chksum)
				DBG_DUMP("[fastboot Note]  dtsi(0x%x) & HDAL(0x%x)\n", dtsi_dbg_info.chksum, checksum);
			else
				DBG_DUMP("[fastboot Note]  dtsi chksum pass(0x%x,0x%x)\n",dtsi_dbg_info.chksum, checksum);
		}
		break;
		default:
		break;

	}

	if(p_trig_param->job_start_cb)
		p_trig_param->job_start_cb(p_handle, param_id, data);

	return E_OK;
}

INT32 kdrv_ipp_job_process_cpu(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_HEAD* p_job, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_TRIG_PARAM *p_trig_param)
{
	UINT32 i = 0;
	INT32 rt = E_OK;

	if (p_cfg == NULL || p_trig_param == NULL) {
		return E_PAR;
	}

	/* keep trigger engine bit, wait mask will be adjust for flow */
	p_cfg->trig_eng_bit = p_cfg->wait_eng_mask;

	if(p_cfg->nn_isp_en)
		rt |= kdrv_ipp_get_nn_isp_max_strp(p_hdl, p_job);

	/* config kdrv_eng */
	rt |= kdrv_ipp_int_stripe_preprocess(p_hdl, p_cfg);
	if (rt != E_OK) {
		goto EXIT_PROCESS;
	}

	if(p_cfg->nn_isp_en){
		rt |= kdrv_ipp_nn_isp_cal_slice(p_hdl, p_job);
		rt |= kdrv_ipp_set_nn_isp_info(p_hdl, p_job);
	}

#if FPGA_BUILTIN_SUPPORT
	if ((kdrv_ipp_builtin_get_status() == TRUE) &&
		p_hdl->direct_info.is_direct &&
		p_trig_param->op == KDRV_IPP_TRIG_START)
#else
	if ( 0 &&
		p_hdl->direct_info.is_direct &&
		p_trig_param->op == KDRV_IPP_TRIG_START)
#endif

	{
		/* skip write register for fastboot first frame */
		UINT32 dev_id;

		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_PRE_ENGINE0, 0);
		if (kdrv_pre_set(dev_id, KDRV_PRE_PARAM_SET_CB, (void *)kdrv_ipp_pre_isr) != E_OK) {
		        DBG_ERR("pre hook cb fail\r\n");
		        return E_PAR;
		}

		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IFE_ENGINE0, 0);
		if (kdrv_ife_set(dev_id, KDRV_IFE_PARAM_SET_CB, (void *)kdrv_ipp_ife_isr) != E_OK) {
		        DBG_ERR("ife hook cb fail\r\n");
		        return E_PAR;
		}
		#if 0 // 690 removed
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_DCE_ENGINE0, 0);
		if (kdrv_dce_set(dev_id, KDRV_DCE_PARAM_SET_CB, (void *)kdrv_ipp_dce_isr) != E_OK) {
		        DBG_ERR("dce hook cb fail\r\n");
		        return E_PAR;
		}
		#endif
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IPE_ENGINE0, 0);
		if (kdrv_ipe_set(dev_id, KDRV_IPE_PARAM_SET_CB, (void *)kdrv_ipp_ipe_isr) != E_OK) {
		        DBG_ERR("ipe hook cb fail\r\n");
		        return E_PAR;
		}

		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IME_ENGINE0, 0);
		if (kdrv_ime_set(dev_id, KDRV_IME_PARAM_SET_CB, (void *)kdrv_ipp_ime_isr) != E_OK) {
		        DBG_ERR("ime hook cb fail\r\n");
		        return E_PAR;
		}

	} else {
		for (i = 0; i < KDRV_IPP_ENG_MAX; i++) {
			if ((1 << i) & p_cfg->wait_eng_mask) {
				if (kdrv_ipp_int_proc_func[i])
					rt = kdrv_ipp_int_proc_func[i](p_hdl, p_cfg, KDRV_IPP_WRITE_REG_CPU, NULL);

				if (rt != E_OK)
					goto EXIT_PROCESS;
			}
		}

		/* debug dump */
		if (kdrv_ipp_dbg_job(0x0, 0)) {
			kdrv_ipp_dbg_dump_cfg(p_trig_param->job_owner_name, p_cfg, kdrv_ipp_int_printf);
		}

		/* ipe gamma & ycurve*/
		//kdrv_ipp_int_load_gamma(p_cfg, FALSE); // move to kdrv_ipe cfg_all to set gamma
	}

	/* Do job start callback */
	kdrv_ipp_do_job_start_cb_cpu(p_hdl, p_trig_param);

	/* trigger */
	if (p_hdl->direct_info.is_direct) {
		kdrv_ipp_job_trigger_eng_direct(p_hdl, p_cfg, p_trig_param);
	} else {
		kdrv_ipp_job_trigger_eng_d2d(p_hdl, p_cfg, NULL);
	}

EXIT_PROCESS:
	/* debug dump */
	if (kdrv_ipp_dbg_job(0x0, 0)) {
		kdrv_ipp_dbg_dump_cfg(p_trig_param->job_owner_name, p_cfg, kdrv_ipp_int_printf);
	}

	return rt;
}

INT32 kdrv_ipp_job_process_ll(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_JOB_HEAD* p_job,  KDRV_IPP_TRIG_PARAM *p_trig_param, KDRV_IPP_JOB_CFG *p_cfg, KDRV_IPP_LL_BLK *p_ll_blk, UINT32 *eng_cmd_num, UINT32 stage)
{
	KDRV_IPP_LL_SET_INFO tmp_ll_info = {0};
	UINT32 i = 0;
	INT32 rt = 0;

	if (p_cfg == NULL || p_ll_blk == NULL || eng_cmd_num == NULL) {
		return E_PAR;
	}

	/* keep trigger engine bit, wait mask will be adjust for flow */
	p_cfg->trig_eng_bit = p_cfg->wait_eng_mask;

	if(p_cfg->nn_isp_en)
		rt |= kdrv_ipp_get_nn_isp_max_strp(p_hdl, p_job);

	/* config kdrv_eng */
	rt |= kdrv_ipp_int_stripe_preprocess(p_hdl, p_cfg);
	if (rt != E_OK) {
		goto EXIT_PROCESS;
	}

	if(p_cfg->nn_isp_en){
		rt |= kdrv_ipp_nn_isp_cal_slice(p_hdl, p_job);
		rt |= kdrv_ipp_set_nn_isp_info(p_hdl, p_job);
	}

	/* config kdrv_eng */
	for (i = 0; i < KDRV_IPP_ENG_MAX; i++) {
		if ((1 << i) & p_cfg->wait_eng_mask) {
			tmp_ll_info.max_cmd_num = eng_cmd_num[i];
			tmp_ll_info.cur_cmd_idx = p_ll_blk->cmd_idx[i];
			tmp_ll_info.cmd_buf_addr.va = p_ll_blk->cmd_buf_addr[i].va;
			tmp_ll_info.cmd_buf_addr.pa = p_ll_blk->cmd_buf_addr[i].pa;
			tmp_ll_info.blk_idx = p_ll_blk->blk_idx;
			tmp_ll_info.stage = stage;
			if (kdrv_ipp_int_proc_func[i])
				rt = kdrv_ipp_int_proc_func[i](p_hdl, p_cfg, KDRV_IPP_WRITE_REG_LL, &tmp_ll_info);

			if (rt != E_OK)
				goto EXIT_PROCESS;
		}
		p_ll_blk->cmd_idx[i] = tmp_ll_info.cur_cmd_idx;
	}

	p_hdl->p_ll_fired_cfg = p_cfg;
	/* Do job start callback */
	kdrv_ipp_do_job_start_cb_ll(p_hdl, p_trig_param);

EXIT_PROCESS:
	/* debug dump */
	if (kdrv_ipp_dbg_job(0x0, 0)) {
		kdrv_ipp_dbg_dump_cfg(p_trig_param->job_owner_name, p_cfg, kdrv_ipp_int_printf);
	}
	if (kdrv_ipp_dbg_job(0x1, 0)) {
		kdrv_ipp_dbg_dump_ll(p_ll_blk);
	}

	return rt;
}

INT32 kdrv_ipp_job_trig_ll(KDRV_IPP_HANDLE *p_hdl, KDRV_IPP_LL_BLK *p_ll_blk)
{
	UINT32 dev_id = 0;
	UINT32 eng_id = 0;
	KDRV_IPP_JOB_CFG *p_cfg = NULL;
	KDRV_IPP_BUF_ADDR *p_ll_addr = NULL;

	p_cfg = (KDRV_IPP_JOB_CFG *)p_ll_blk->p_parent_job_cfg;

	p_ll_addr = p_ll_blk->cmd_buf_addr;

	if (p_cfg == NULL) {
		return E_PAR;
	}

	/* ipe gamma & ycurve*/
	//kdrv_ipp_int_load_gamma(p_cfg, TRUE); // move to kdrv_ipe cfg_all to set gamma

	//539a reorder from ime to pre
#if 1
	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IFE_IDLE) {


		eng_id = kdrv_ipp_get_ife_eng_id(p_hdl->eng_id);
		dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

		kdrv_ipp_push_ife_hdl_queue(p_hdl);

		if (kdrv_ife_set(dev_id, KDRV_IFE_PARAM_TRIG_LL, (void *)&p_ll_addr[KDRV_IPP_ENG_IFE].pa) != E_OK) {

			DBG_ERR("ife trig ll fail\r\n");
			return E_SYS;
		}
#if 0
		/* special check for ife mask
			remove frame start bit
			remove frame end bit if not d2d mode
		*/
		p_cfg->start_eng_mask &= ~(KDRV_IPP_ENG_IFE_IDLE);
		if (p_cfg->ife_io_cfg.mode != KDRV_IPP_OPMODE_D2D) {
			p_cfg->wait_eng_mask &= ~(KDRV_IPP_ENG_IFE_IDLE);
		}
#endif
		/* special check for ife mask
			remove frame start bit if not raw flow
			remove frame end bit if not d2d mode and raw flow
		*/
		if (p_cfg->ife_io_cfg.mode != KDRV_IPP_OPMODE_IFE2IME && p_cfg->ife_io_cfg.mode != KDRV_IPP_OPMODE_PRE2IME)
			p_cfg->start_eng_mask &= ~(KDRV_IPP_ENG_IFE_IDLE);
		if (p_cfg->ife_io_cfg.mode != KDRV_IPP_OPMODE_D2D && p_cfg->ife_io_cfg.mode != KDRV_IPP_OPMODE_IFE2IME && p_cfg->ife_io_cfg.mode != KDRV_IPP_OPMODE_PRE2IME) {
			p_cfg->wait_eng_mask &= ~(KDRV_IPP_ENG_IFE_IDLE);
		}
	}

	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IME_IDLE) {
		eng_id = kdrv_ipp_get_ime_eng_id(p_hdl->eng_id);
		dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

		kdrv_ipp_push_ime_hdl_queue(p_hdl);

		if (kdrv_ime_set(dev_id, KDRV_IME_PARAM_TRIG_LL, (void *)&p_ll_addr[KDRV_IPP_ENG_IME].pa) != E_OK) {

			DBG_ERR("ime trig ll fail\r\n");
			return E_SYS;
		}
	}


	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_IPE_IDLE) {
		eng_id = kdrv_ipp_get_ipe_eng_id(p_hdl->eng_id);
		dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

		kdrv_ipp_push_ipe_hdl_queue(p_hdl);

		if (kdrv_ipe_set(dev_id, KDRV_IPE_PARAM_TRIG_LL, (void *)&p_ll_addr[KDRV_IPP_ENG_IPE].pa) != E_OK) {

			DBG_ERR("ipe trig ll fail\r\n");
			return E_SYS;
		}
	}

	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_PRE_IDLE) {


		eng_id = kdrv_ipp_get_pre_eng_id(p_hdl->eng_id);
		dev_id = KDRV_DEV_ID(p_hdl->chip_id, eng_id, 0);

		kdrv_ipp_push_pre_hdl_queue(p_hdl);

		if (kdrv_pre_set(dev_id, KDRV_PRE_PARAM_TRIG_LL, (void *)&p_ll_addr[KDRV_IPP_ENG_PRE].pa) != E_OK) {

			DBG_ERR("pre trig ll fail\r\n");
			return E_SYS;
		}
#if 0
		/* special check for pre mask
			remove frame start bit
			remove frame end bit if not d2d mode
		*/
		p_cfg->start_eng_mask &= ~(KDRV_IPP_ENG_PRE_IDLE);
		if (p_cfg->pre_io_cfg.mode != KDRV_IPP_OPMODE_D2D) {
			p_cfg->wait_eng_mask &= ~(KDRV_IPP_ENG_PRE_IDLE);
		}
#endif
		/* special check for ife mask
			remove frame start bit if not raw flow
			remove frame end bit if not d2d mode and raw flow
		*/
		if (p_cfg->pre_io_cfg.mode != KDRV_IPP_OPMODE_IFE2IME && p_cfg->pre_io_cfg.mode != KDRV_IPP_OPMODE_PRE2IME)
			p_cfg->start_eng_mask &= ~(KDRV_IPP_ENG_PRE_IDLE);
		if (p_cfg->pre_io_cfg.mode != KDRV_IPP_OPMODE_D2D && p_cfg->pre_io_cfg.mode != KDRV_IPP_OPMODE_PRE2IME) {
			p_cfg->wait_eng_mask &= ~(KDRV_IPP_ENG_PRE_IDLE);
		}

	}
#endif

	#if 0 // 690 removed
	if (p_cfg->wait_eng_mask & KDRV_IPP_ENG_DCE_IDLE) {
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_DCE_ENGINE0, 0);

		if (kdrv_dce_set(dev_id, KDRV_DCE_PARAM_TRIG_LL, (void *)&p_ll_addr[KDRV_IPP_ENG_DCE].pa) != E_OK) {

			DBG_ERR("dce trig ll fail\r\n");
			return E_SYS;
		}
	}
	#endif


	p_hdl->dbg_perf_info.trig_cnt++;

	return E_OK;
}



