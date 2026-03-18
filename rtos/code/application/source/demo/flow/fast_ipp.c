#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/type.h>
#include <kwrap/perf.h>
#include <kwrap/cpu.h>
#include <kwrap/util.h>
#include <kwrap/flag.h>
#include <kwrap/task.h>
#include <kwrap/cmdsys.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include "prjcfg.h"
#include "flow_boot_linux.h"
#include <kwrap/debug.h>
#include <dispdevctrl.h>
#include <plat/display.h>
#include <plat/gpio.h>
#include <comm/hwclock.h>
#include "kdrv_builtin/sie_init.h"
#include "kdrv_builtin/kdrv_ipp_builtin.h"
#include "kdrv_builtin/vdoenc_builtin.h"
#include "kdrv_builtin/nvtmpp_init.h"
#include "isp_builtin.h" // NOTE: Include ISP builtin
#include <kdrv_gfx2d/kdrv_ise_ctl.h>
#include <kdrv_videoprocess/kdrv_ime.h>
#include "sys_fastboot.h"
#include "bridge_fourcc.h"
#include "bridge_mem.h"
#include "fast_ipp.h"

#define CFG_DISPLAY_GREEN DISABLE // only for FASTBOOT_LCD_DEMO

static UINT32 ipp_rtos_id = 0;
static UINT32 ipp_rtos_proc_cnt = 0;
static UINT32 ipp_rtos_err_mode = 0;
static UINT32 ipp_rtos_last_frm_idx = 0;
static UINT32 ipp_ll_start_time = 0;

#define TURN_ON_SENSOR1 0x1
#define TURN_ON_SENSOR2 0x2
extern int m_error_on_sensor;

#if defined(FASTBOOT_LCD_DEMO)

static ID ime_out_id = 0;
static uintptr_t last_ime_out_y = 0;
static uintptr_t last_ime_out_uv = 0;
static uintptr_t last_ime_lineofs = 0;
static uintptr_t last_ime_width = 0;
static uintptr_t last_ime_height = 0;
static uintptr_t mp_y = 0;
static uintptr_t mp_uv = 0;
static uintptr_t mp_y2 = 0;
static uintptr_t mp_uv2 = 0;

static int fast_display_one(uintptr_t y, uintptr_t uv, unsigned int lofs_y, unsigned int w, unsigned h)
{
	int er;
	DISPDEV_PARAM disp_param;
	DISPLAYER_PARAM lyr_param;
	DISPCTRL_PARAM ctrl_param;

	PDISP_OBJ pdisp_obj = disp_get_display_object(DISP_1);

	memset(&disp_param, 0, sizeof(DISPDEV_PARAM));
	disp_param.SEL.GET_PREDISPSIZE.dev_id = DISPDEV_ID_PANEL;
	if ((er = pdisp_obj->dev_ctrl(DISPDEV_GET_PREDISPSIZE, &disp_param)) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
		return er;
	}
	int lcd_w = disp_param.SEL.GET_PREDISPSIZE.ui_buf_width;
	int lcd_h = disp_param.SEL.GET_PREDISPSIZE.ui_buf_height;

	if (mp_y == 0 && mp_uv == 0) {
		uintptr_t addr = (uintptr_t)malloc(lcd_w * lcd_h * 2 + lcd_w * 540 * 2);
		vos_cpu_dcache_sync((VOS_ADDR)addr, lcd_w * lcd_h * 2 + lcd_w * 540 * 2, VOS_DMA_BIDIRECTIONAL);
		mp_y = (uintptr_t)addr;
		mp_uv = (uintptr_t)mp_y + (lcd_w * lcd_h);
		mp_y2 = (uintptr_t)mp_uv + (lcd_w * lcd_h);
		mp_uv2 = (uintptr_t)mp_y2 + (lcd_w * 540);
	}

#if (CFG_DISPLAY_GREEN)
	memset((void *)mp_y, 0x00, lcd_w * lcd_h);
	memset((void *)mp_uv, 0x00, (lcd_w * lcd_h) >> 1);
#else
#if 1
	ise_scale(mp_y, lcd_w, lcd_w, lcd_h, y, lofs_y, w, h, ISE_DRV_Y8_ONLY, ISE_DRV_SCALE_METHOD_AUTO);
	ise_scale(mp_uv, lcd_w, lcd_w >> 1, lcd_h >> 1, uv, lofs_y, w >> 1, h >> 1, ISE_DRV_UVP, ISE_DRV_SCALE_METHOD_AUTO);
#else // 2 pass scale
	ise_scale(mp_y2, lcd_w, lcd_w, 540, y, lofs_y, w, h, ISE_DRV_Y8_ONLY, ISE_DRV_SCALE_METHOD_AUTO);
	ise_scale(mp_uv2, lcd_w, lcd_w >> 1, 540 >> 1, uv, lofs_y, w >> 1, h >> 1, ISE_DRV_UVP, ISE_DRV_SCALE_METHOD_AUTO);
	ise_scale(mp_y, lcd_w, lcd_w, lcd_h, mp_y2, lcd_w, lcd_w, 540, ISE_DRV_Y8_ONLY, ISE_DRV_SCALE_METHOD_AUTO);
	ise_scale(mp_uv, lcd_w, lcd_w >> 1, lcd_h >> 1, mp_uv2, lcd_w, lcd_w >> 1, 540 >> 1, ISE_DRV_UVP, ISE_DRV_SCALE_METHOD_AUTO);
#endif
	vos_cpu_dcache_sync((VOS_ADDR)mp_uv, lcd_w * lcd_h * 2, VOS_DMA_BIDIRECTIONAL);
#if (CFG_LOAD_AI_DETECT)
	if (operation_ai_get_result() == 0) {
		memset((void *)mp_uv, 0x80, (lcd_w * lcd_h) >> 1);
	}
#endif
#endif

	memset(&lyr_param, 0, sizeof(DISPLAYER_PARAM));
	lyr_param.SEL.SET_VDOBUFADDR.buf_sel    = DISPBUFADR_0;
	lyr_param.SEL.SET_VDOBUFADDR.ui_addr_y0  = (unsigned int)mp_y;
	lyr_param.SEL.SET_VDOBUFADDR.ui_addr_cb0 = (unsigned int)mp_uv;
	if ((er = pdisp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_VDOBUFADDR, &lyr_param)) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
		return er;
	}

	memset(&lyr_param, 0, sizeof(DISPLAYER_PARAM));
	lyr_param.SEL.SET_MODE.buf_format   = DISPBUFFORMAT_YUV420PACK;
	lyr_param.SEL.SET_MODE.buf_mode     = DISPBUFMODE_BUFFER_REPEAT;
	lyr_param.SEL.SET_MODE.buf_number   = DISPBUFNUM_1;
	if ((er = pdisp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_MODE, &lyr_param)) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
		return er;
	}

	memset(&lyr_param, 0, sizeof(DISPLAYER_PARAM));
	lyr_param.SEL.SET_BUFWINSIZE.ui_buf_width = lcd_w;
	lyr_param.SEL.SET_BUFWINSIZE.ui_buf_height = lcd_h;
	lyr_param.SEL.SET_BUFWINSIZE.ui_buf_line_ofs = lcd_w;
	lyr_param.SEL.SET_BUFWINSIZE.ui_win_width = lcd_w;
	lyr_param.SEL.SET_BUFWINSIZE.ui_win_height = lcd_h;
	lyr_param.SEL.SET_BUFWINSIZE.i_win_ofs_x = 0;
	lyr_param.SEL.SET_BUFWINSIZE.i_win_ofs_y = 0;
	if ((er = pdisp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_BUFWINSIZE, &lyr_param)) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
		return er;
	}

#if 0
	memset(&lyr_param, 0, sizeof(DISPLAYER_PARAM));
	lyr_param.SEL.SET_OUTDIR.buf_out_dir = DISPOUTDIR_HRZ_FLIP;
	if ((er = pdisp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_OUTDIR, &lyr_param)) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
		return er;
	}
#endif
	memset(&ctrl_param, 0, sizeof(DISPCTRL_PARAM));
	ctrl_param.SEL.SET_ALL_LYR_EN.b_en      = TRUE;
	ctrl_param.SEL.SET_ALL_LYR_EN.disp_lyr  = DISPLAYER_VDO1;
	if ((er = pdisp_obj->disp_ctrl(DISPCTRL_SET_ALL_LYR_EN, &ctrl_param)) != 0) {
		DBG_ERR("%s: %d, er=%d\n",  __func__,  __LINE__, er);
		return er;
	}
	pdisp_obj->load(TRUE);

	return 0;
}

void fast_builtin_fmd_cb(KDRV_IPP_BUILTIN_FMD_CB_INFO *p_info, UINT32 reserved)
{
	if (last_ime_lineofs == 0) {
		last_ime_lineofs = p_info->out_img[0].loff[0];
		last_ime_width = p_info->out_img[0].size.w;
		last_ime_height = p_info->out_img[0].size.h;
	}
#if (CFG_LOAD_AI_DETECT)
	operation_ai_set_img((uintptr_t)(p_info->out_img[0].addr[0]), (uintptr_t)(p_info->out_img[0].addr[0] + last_ime_lineofs * last_ime_height), last_ime_lineofs, last_ime_width, last_ime_height, p_info->out_img[0].fmt);
#endif
#if (FASTBOOT_2A || SENSOR_EMBEDDED_2A || PREROLL_2A) && (CFG_RTOS_VENC == ENABLE)
	VdoEnc_BuiltIn_trig(p_info, reserved);
#endif
	last_ime_out_y = p_info->out_img[0].addr[0];
	last_ime_out_uv = p_info->out_img[0].addr[1];
	if (ime_out_id != 0) {
		vos_flag_iset(ime_out_id, (FLGPTN)1);
	}
}

void display_tsk(void *p_param)
{
	FLGPTN flgptn;
	vos_task_enter();
	while (1) {
		vos_flag_wait(&flgptn, ime_out_id, (FLGPTN)(1), TWF_ANDW | TWF_CLR);
		uintptr_t ime_y =  last_ime_out_y;
		uintptr_t ime_uv = last_ime_out_uv;
		fast_display_one(ime_y, ime_uv, last_ime_lineofs, last_ime_width, last_ime_height);
	}
	THREAD_RETURN(0);
}

MAINFUNC_ENTRY(go, argc, argv)
{

	vos_flag_create(&ime_out_id, NULL, "ime_out_id");
	vos_flag_clr(ime_out_id, (FLGPTN) - 1);
	VK_TASK_HANDLE vkt_display_tsk  = vos_task_create(display_tsk, NULL,  "thread_display",  10, 4096);
	vos_task_resume(vkt_display_tsk);

	return 0;
}

#else

void fast_builtin_fmd_cb(KDRV_IPP_BUILTIN_FMD_CB_INFO *p_info, UINT32 reserved)
{
	static int is_1st_frame = 1;

	if (is_1st_frame) {
		vos_perf_list_mark("ipp", __LINE__, 2);
		is_1st_frame = 0;
	}
#if (CFG_LOAD_AI_DETECT)
	operation_ai_set_img((uintptr_t)(p_info->out_img[0].addr[0]),
				(uintptr_t)(p_info->out_img[0].addr[0] + p_info->out_img[0].loff[0] * p_info->out_img[0].size.h),
				p_info->out_img[0].loff[0],
				p_info->out_img[0].size.w,
				p_info->out_img[0].size.h,
				p_info->out_img[0].fmt);
#endif
#if 0
	int pid;
	for (pid = 0; pid < KDRV_IPP_BUILTIN_PATH_ID_MAX; pid++) {
		printf("[trig] name=%s reserved=%d\r\n", p_info->name, (int)reserved);
		printf("[trig] loff=%d w=%d h=%d\r\n", (int)p_info->out_img[pid].loff[0], (int)p_info->out_img[pid].size.w, (int)p_info->out_img[pid].size.h);
		printf("[trig] y=0x%lx uv=0x%lx fmt=0x%lx\r\n", (unsigned long)p_info->out_img[pid].addr[0], (unsigned long)p_info->out_img[pid].addr[1], (unsigned long)p_info->out_img[pid].fmt);
	}
#endif
	ipp_rtos_proc_cnt++;
#if (FASTBOOT_2A || SENSOR_EMBEDDED_2A || PREROLL_2A) && (CFG_RTOS_VENC == ENABLE)
	VdoEnc_BuiltIn_trig(p_info, reserved);
#endif
}

#endif

static void ise_done_cb(struct ise_drv_job_head *head)
{
}

int ise_scale(uintptr_t out_addr, int out_lofs, int out_w, int out_h, uintptr_t in_addr, int in_lofs, int in_w, int in_h, int io_pack_fmt, int scl_method)
{
	UINT32 blk_mode = 1;
	INT32 ret = 0;
	struct ise_drv_job_cfg job_cfg = {0};

	job_cfg.in.fmt = io_pack_fmt;
	job_cfg.iq.scale_method = scl_method;
	job_cfg.in.flush_buf = 0;
	job_cfg.out.flush_buf = 0;
	job_cfg.in.roi.w = in_w;
	job_cfg.in.roi.h = in_h;
	job_cfg.in.roi.lofs[0] = in_lofs;
	job_cfg.in.addr[0].va = in_addr;
	job_cfg.in.addr[0].pa = in_addr;
	job_cfg.out.roi.w = out_w;
	job_cfg.out.roi.h = out_h;
	job_cfg.out.roi.lofs[0] = out_lofs;
	job_cfg.out.addr[0].va = out_addr;
	job_cfg.out.addr[0].pa = out_addr;

	struct ise_drv_job_head job_head = {0};

	VOS_INIT_LIST_HEAD(&job_head.job_list_root);
	job_cfg.sts = ISE_DRV_STS_IDLE;
	job_cfg.out.alpha_trans_th = 128;
	VOS_INIT_LIST_HEAD(&job_cfg.list);
	vos_list_add_tail(&job_cfg.list, &job_head.job_list_root);
	struct ise_drv_job_cfg *p_job_cfg = vos_list_first_entry_or_null(&job_head.job_list_root, struct ise_drv_job_cfg, list);
	if (p_job_cfg == NULL) {
		DBG_ERR("null job\r\n");
		return -1;
	}

	job_head.chip = KDRV_DEV_ID_CHIP(KDRV_CHIP0);
	job_head.eng = 0;
	job_head.num = 1;
	job_head.id = 1111;
	job_head.callback = ise_done_cb;

	ULONG hdl = kdrv_ise_open(job_head.chip, job_head.eng); //alloc handle
	if (hdl == 0) {
		DBG_ERR("null hdl\r\n");
		return -1;
	}

	ret = kdrv_ise_set(hdl, KDRV_ISE_PARAM_BLK_MODE, (void *)&blk_mode);  //set blocking mode
	if (ret < 0) {
		DBG_ERR("BLK_MODE %d\r\n", ret);
		return ret;
	}

	ret = kdrv_ise_trigger(hdl, &job_head); //trigger ise
	if (ret != ISE_DRV_STS_OK) {
		DBG_ERR("trigger %d\r\n", ret);
		return ret;
	}

	ret = kdrv_ise_close(hdl); //free handle
	if (ret < 0) {
		DBG_ERR("close %d\r\n", ret);
		return ret;
	}

	return 0;
}

int fast_ipp_run_ll(void)
{
	//extern INT32 kdrv_ipp_rtos_ll_free_run_start(void);
	//extern INT32 kdrv_ipp_rtos_wait_ll_done(void);
	//kdrv_ipp_rtos_ll_free_run_start();
	//kdrv_ipp_rtos_wait_ll_done();
	//ipp_ll_start_time = (UINT32)hwclock_get_counter();
	return 0;
}

int fast_ipp(void)
{
	/**
	 * get fastboot memory from dts
	 * nvtmpp_parse_fastboot_mem_dt has called earlier on _fast_open_sie
	 * do not call it again, or will boot linux NG
	 */

	vos_perf_list_mark("ipp", __LINE__, 0);

	NVTMPP_FASTBOOT_MEM_S *p_fastboot_mem;
	KDRV_IPP_BUILTIN_INIT_INFO ipp_init_info = {0};
	p_fastboot_mem = nvtmpp_get_fastboot_mem();

	if (p_fastboot_mem == NULL) {
		DBG_ERR("p_fastboot_mem is NULL.\n");
		return -1;
	}
	memset((void *)&ipp_init_info, 0, sizeof(KDRV_IPP_BUILTIN_INIT_INFO));

	ipp_init_info.hdl_num = 2;
	ipp_init_info.hdl_info[0].name = "vdoprc0";
	ipp_init_info.hdl_info[0].isp_id = 0x0;
	ipp_init_info.hdl_info[0].src_sie_id_bit = 0x1;
	ipp_init_info.hdl_info[0].ctrl_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC_CTRL].addr;
	ipp_init_info.hdl_info[0].ctrl_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC_CTRL].size;
	ipp_init_info.hdl_info[0].shdr_ring_buf_addr[0] = p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].addr;
	ipp_init_info.hdl_info[0].shdr_ring_buf_size[0] = p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].size;

	ipp_init_info.hdl_info[1].name = "vdoprc1";
	ipp_init_info.hdl_info[1].isp_id = 0x1;
	ipp_init_info.hdl_info[1].src_sie_id_bit = 0x2;
	ipp_init_info.hdl_info[1].ctrl_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC1_CTRL].addr;
	ipp_init_info.hdl_info[1].ctrl_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC1_CTRL].size;
	ipp_init_info.hdl_info[1].shdr_ring_buf_addr[0] = 0;
	ipp_init_info.hdl_info[1].shdr_ring_buf_size[0] = 0;
#if defined(_SEN1_ENABLED_)
	ipp_init_info.valid_src_id_bit |= 0x1;
#endif
#if defined(_SEN2_ENABLED_)
	ipp_init_info.valid_src_id_bit |= 0x2;
#endif
	ipp_rtos_id = ipp_init_info.valid_src_id_bit;

	if (m_error_on_sensor) {
		//todo: wait ipp_init_info struct ready
		//ipp_init_info.err_mode = 1;
		//ipp_rtos_err_mode = ipp_init_info.err_mode;
	}

	ime_builtin_rtos_init();
	//kdrv_ipp_rtos_pre_roll_init(&ipp_init_info);
	//kdrv_ipp_rtos_exit();

	kdrv_ipp_rtos_init(&ipp_init_info);
#if (POWERON_MODE == POWERON_MODE_PREROLL)
	kdrv_ipp_builtin_reg_fmd_cb_by_isp_id(0, fast_builtin_fmd_cb);
#else
	kdrv_ipp_builtin_reg_fmd_cb(fast_builtin_fmd_cb);
#endif

	vos_perf_list_mark("ipp", __LINE__, 1);

#if defined(FASTBOOT_LCD_DEMO)
#if (CFG_DISPLAY_GREEN)
	fast_display_one(0, 0, 0, 0, 0);
#else
	vos_perf_list_mark("ipp", __LINE__, 1);
#if (!CFG_LOAD_AI_DETECT)
	// for display and ai detect. ise has ininted if CFG_LOAD_AI_DETECT
	kdrv_ise_rtos_init();
#endif
	while (last_ime_out_y == 0) {
		vos_util_delay_ms(1);
	}
	vos_perf_list_mark("ipp", __LINE__, 2);

	uintptr_t ime_y =  last_ime_out_y;
	uintptr_t ime_uv = last_ime_out_uv;
	fast_display_one(ime_y, ime_uv, last_ime_lineofs, last_ime_width, last_ime_height);
	vos_perf_list_mark("ipp", __LINE__, 3);
#endif
#endif
	return 0;
}

int bridge_mem_plan_sie(void)
{
	UINT32 sie_skip_id_bit = 0;
	SIE_FB_BRIDGE_INFO sie_bridge_info;
#if defined(_SEN1_ENABLED_)
	// for sie0 run on rtos
	sie_skip_id_bit |= 0x1;
#endif
#if defined(_SEN2_ENABLED_)
	// for sie1 run on rtos
	sie_skip_id_bit |= 0x2;
#endif
	bridge_mem_add_tag(SIE_SKIP_ID_BIT, sie_skip_id_bit);

	sie_fb_get_bridge_info(SIE_FB_ID_1, &sie_bridge_info);
	bridge_mem_add_tag(SIE_FC_ID_1, sie_bridge_info.frame_cnt);
	bridge_mem_add_tag(SIE_TS_ID_1, sie_bridge_info.timestamp);
	bridge_mem_add_tag(SIE_ID_1_ADDR_0, (UINT32)(sie_bridge_info.buf_addr_0 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_1_ADDR_1, (UINT32)(sie_bridge_info.buf_addr_1 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_1_ADDR_2, (UINT32)(sie_bridge_info.buf_addr_0>>32 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_1_ADDR_3, (UINT32)(sie_bridge_info.buf_addr_1>>32 & 0xffffffff));

	sie_fb_get_bridge_info(SIE_FB_ID_2, &sie_bridge_info);
	bridge_mem_add_tag(SIE_FC_ID_2, sie_bridge_info.frame_cnt);
	bridge_mem_add_tag(SIE_TS_ID_2, sie_bridge_info.timestamp);
	bridge_mem_add_tag(SIE_ID_2_ADDR_0, (UINT32)(sie_bridge_info.buf_addr_0 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_2_ADDR_1, (UINT32)(sie_bridge_info.buf_addr_1 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_2_ADDR_2, (UINT32)(sie_bridge_info.buf_addr_0>>32 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_2_ADDR_3, (UINT32)(sie_bridge_info.buf_addr_1>>32 & 0xffffffff));

	sie_fb_get_bridge_info(SIE_FB_ID_3, &sie_bridge_info);
	bridge_mem_add_tag(SIE_FC_ID_3, sie_bridge_info.frame_cnt);
	bridge_mem_add_tag(SIE_TS_ID_3, sie_bridge_info.timestamp);
	bridge_mem_add_tag(SIE_ID_3_ADDR_0, (UINT32)(sie_bridge_info.buf_addr_0 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_3_ADDR_1, (UINT32)(sie_bridge_info.buf_addr_1 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_3_ADDR_2, (UINT32)(sie_bridge_info.buf_addr_0>>32 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_3_ADDR_3, (UINT32)(sie_bridge_info.buf_addr_1>>32 & 0xffffffff));

	sie_fb_get_bridge_info(SIE_FB_ID_4, &sie_bridge_info);
	bridge_mem_add_tag(SIE_FC_ID_4, sie_bridge_info.frame_cnt);
	bridge_mem_add_tag(SIE_TS_ID_4, sie_bridge_info.timestamp);
	bridge_mem_add_tag(SIE_ID_4_ADDR_0, (UINT32)(sie_bridge_info.buf_addr_0 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_4_ADDR_1, (UINT32)(sie_bridge_info.buf_addr_1 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_4_ADDR_2, (UINT32)(sie_bridge_info.buf_addr_0>>32 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_4_ADDR_3, (UINT32)(sie_bridge_info.buf_addr_1>>32 & 0xffffffff));

	sie_fb_get_bridge_info(SIE_FB_ID_5, &sie_bridge_info);
	bridge_mem_add_tag(SIE_FC_ID_5, sie_bridge_info.frame_cnt);
	bridge_mem_add_tag(SIE_TS_ID_5, sie_bridge_info.timestamp);
	bridge_mem_add_tag(SIE_ID_5_ADDR_0, (UINT32)(sie_bridge_info.buf_addr_0 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_5_ADDR_1, (UINT32)(sie_bridge_info.buf_addr_1 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_5_ADDR_2, (UINT32)(sie_bridge_info.buf_addr_0>>32 & 0xffffffff));
	bridge_mem_add_tag(SIE_ID_5_ADDR_3, (UINT32)(sie_bridge_info.buf_addr_1>>32 & 0xffffffff));

	return 0;
}

int bridge_mem_plan_ipp(void)
{
	UINT32 ipp_rtos_bit = 0;
	UINT32 ipp_rtos_cnt = 0;
	UINT32 ipp_rtos_err = 0;
	UINT32 ipp_rtos_frm = 0;

#if (CFG_RTOS_IPP == ENABLE) || (PREROLL_2A)

	extern UINT32 kdrv_ipp_rtos_switch_to_builtin(UINT32 src_sie_id_bit);
	ipp_rtos_last_frm_idx = kdrv_ipp_rtos_switch_to_builtin(0x1);
	//printf("= ipp_rtos_last_frm_idx =%d\r\n", (int)ipp_rtos_last_frm_idx);

#endif

	ipp_rtos_bit = ipp_rtos_id;
	ipp_rtos_cnt = ipp_rtos_proc_cnt;
	ipp_rtos_err = ipp_rtos_err_mode;
	ipp_rtos_frm = ipp_rtos_last_frm_idx;

	// temp code for vprc0 run on rtos
	ipp_rtos_bit |= 0x1;

	bridge_mem_add_tag(IPP_RTOS_BIT, ipp_rtos_bit);
	bridge_mem_add_tag(IPP_RTOS_CNT, ipp_rtos_cnt);
	bridge_mem_add_tag(IPP_RTOS_ERR, ipp_rtos_err);
	bridge_mem_add_tag(IPP_RTOS_FRM, ipp_rtos_frm);
	bridge_mem_add_tag(IPP_LL_START_TIME, ipp_ll_start_time);

	return 0;
}

