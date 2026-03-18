#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/clk-provider.h> //__clk_is_enabled
//#include <linux/slab.h>
#else
#include <string.h>
#endif

#include "kwrap/cpu.h"
#include "kwrap/spinlock.h"
#include <kwrap/perf.h> //VOS_TICK
#include "jpeg_kdrv.h"
#include "jpeg_dbg.h"
#include "jpeg_main.h"
#include "jpg_header.h"
#include "jpeg_platform.h"
#include "jpeg_api.h"
#include "jpeg_reg.h" //JPEG_GETREG(JPG_VERSION_OFS)
#include <plat/top.h> // for get chip id
#if JPEG_SW_DECODE
#include "jpgdec.h"
#endif

#define ALIGN_16(a)		((((a)+15)>>4)<<4)
#define ALIGN_64(a)		((((a)+63)>>6)<<6)

//rtos fastboot
#define JPEG_ENC_ENTITY_CHN		64
#define JPEG_DEC_ENTITY_CHN		32

/*********************************
*  module parameter
**********************************/
unsigned int jpeg_max_chip = 1;
unsigned int jpeg_max_engine = 1;
unsigned int jpeg_enc_max_chn = JPEG_ENC_ENTITY_CHN;
unsigned int jpeg_dec_max_chn = JPEG_DEC_ENTITY_CHN;
unsigned int timeout_check_framend = 1;
unsigned int timeout_check_framend_ok = 0;
unsigned int retrigger_cnt = 0;
unsigned int g_HeaderChecksumEn = 0;
unsigned int g_ChooseEngineUse = 0; //0: all(default), 1: engine1, 2: engine2

JPEG_SW_DEC_FUNC *jpeg_swdec_func = NULL;


/*********************************
*  const parameter
**********************************/
#if SUP_NON_BLK_MODE
static const JPEG_OSG_RGB_CFG JPEG_ENC_OSG_RGB = {
	{{0x26, 0x4b, 0x0f}, {0xea, 0xd5, 0x41}, {0x41, 0xc9, 0xf5}}
};
static const JPEG_OSG_PAL_CFG JPEG_ENC_OSG_PAL[16] = {
	{0x00, 0xFF, 0x00, 0x00}, {0x00, 0x00, 0xFF, 0x00}, {0x00, 0x00, 0x00, 0xFF}, {0x00, 0xFF, 0xFF, 0x00},
	{0x00, 0xFF, 0x00, 0xFF}, {0x00, 0x00, 0xFF, 0xFF}, {0x00, 0x80, 0x00, 0x00}, {0x00, 0x00, 0x80, 0x00},
	{0x00, 0x00, 0x00, 0x80}, {0x00, 0x80, 0x80, 0x00}, {0x00, 0x80, 0x00, 0x80}, {0x00, 0x00, 0x80, 0x80},
	{0x00, 0xFF, 0xFF, 0xFF}, {0x00, 0x80, 0x80, 0x80}, {0x00, 0x40, 0x40, 0x40}, {0x00, 0x00, 0x00, 0x00}
};

#if SUPPORT_JE_MASK
static const UINT8 MASK_PALETTE[3][16] = {
	{76,  149,  29, 255, 178, 105, 126,  78, 155, 176, 128, 205,  49,  99, 255,   0},
	{85,  43,  255,	   0, 171, 212,  56, 227, 184,  28, 199, 156,  99,  71, 128, 128},
	{255, 21,  107, 148,   0, 234, 219,  71, 199, 184,  36, 163,  92,  56, 128, 128},
};
#endif
#endif //!SUP_NON_BLK_MODE


/*********************************
*  local parameter
**********************************/
#if SUP_NON_BLK_MODE
static KDRV_CALLBACK_FUNC *g_jpeg_enc_cb = NULL;
static KDRV_CALLBACK_FUNC *g_jpeg_dec_cb = NULL;
static vk_spinlock_t    jpeg_job_lock;		/* for link-list and job_item content protection */
static vk_spinlock_t    jpeg_jobitem_lock;	/* job_item_t database protection */
static struct tasklet_struct jpeg_job_tasklet[JPEG_MAX_CHIP][JPEG_MAX_ENG];
static struct tasklet_struct jpeg_cb_tasklet;

static struct job_item_pool_t job_item_pool;

//static struct vos_list_head     *jpeg_eng_job_head = NULL;		/* per chip+engine, thus index is like eng_data_idx */
static struct vos_list_head     *jpeg_job_head = NULL;	        /* per chip, thus index is like chip_idx */
static struct vos_list_head     *jpeg_enc_minor_head = NULL;	/* how many channels per chip, thus it is chip level */
static struct vos_list_head     *jpeg_dec_minor_head = NULL;	/* how many channels per chip, thus it is chip level */

static struct jpeg_enc_data_t *jpeg_enc_data = NULL;
static struct jpeg_dec_data_t *jpeg_dec_data = NULL;
#else
static struct jpeg_enc_data_t jpeg_enc_entity[JPEG_ENC_ENTITY_CHN];
static struct jpeg_dec_data_t jpeg_dec_entity[JPEG_DEC_ENTITY_CHN];
static struct jpeg_enc_data_t *jpeg_enc_data = jpeg_enc_entity;
static struct jpeg_dec_data_t *jpeg_dec_data = jpeg_dec_entity;
#endif

//both block/non-block mode use
static vk_spinlock_t	jpeg_id_lock;		/* jpeg_enc_data_t/jpeg_dec_data_t channel database protection */

#define JPEG_INT_TIMEOUT	0xFFFF0000


/*********************************
*  local function
**********************************/
#if SUP_NON_BLK_MODE
static int jpeg_sync_job(struct job_item_t *job_item, int status);
#endif


/*********************************
*  extern parameter
**********************************/
extern JPEG_ENGINE_DATA jpeg_eng_data[JPEG_MAX_ENTITYS];
extern unsigned int jpeg_sw_timeout_period;	/* ms */
extern UINT32  jpegtest_getbs_checksum(uintptr_t io_addr);


/*********************************
*  extern function
**********************************/
extern void *jpeg_platform_malloc(UINT32 size);
extern void jpeg_platform_free(void *ptr);
extern void jpeg_set_codec_type(JPEG_ENGINE_DATA *p_eng_data, int codec_type);
extern ER jpeg_enc_trigger(void *p_param, JPEG_PROCESS_CFG *p_enc_cfg, uintptr_t io_addr);
extern ER jpeg_dec_trigger(void *p_param, KDRV_CALLBACK_FUNC *p_cb_func, uintptr_t io_addr);
extern int jpeg_brc_update(KDRV_VDOJPGE_PARAM *p_enc_param, JPEG_RC_PARAM *p_rc, UINT32 bs_size, UINT32 eng_idx, uintptr_t io_addr);
extern void jpeg_platform_setup_timer(unsigned int chip_idx, unsigned int eng_idx, platform_timer_list_t *timerlist, void (*timeout_handle)(unsigned int chip_idx, unsigned int eng_idx), unsigned int period_ms);
extern void jpeg_hw_reset(uintptr_t io_addr);
extern uintptr_t jpeg_va2pa(uintptr_t addr);
extern UINT32 heavyload_checksum(uintptr_t addr, unsigned int);
extern UINT32 jpeg_platform_atomic_context(void);


//rtos init
void jpeg_rtos_spin_lock_init(void)
{
	vk_spin_lock_init(&jpeg_id_lock);
}

////////// kdrv id ///////////
int jpeg_enc_get_unused_channel(UINT32 chip, UINT32 engine)
{
	int chn, kdrv_id = -1;
	unsigned long flags;
	struct jpeg_enc_data_t *p_enc_data;

	vk_spin_lock_irqsave(&jpeg_id_lock, flags);
	for (chn = 0; chn < jpeg_enc_max_chn; chn++) {
		p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);

		if (0 == p_enc_data->active) {
			p_enc_data->active = 1;
			kdrv_id = KDRV_DEV_ID(chip, engine, chn);
			break;
		}
	}
	vk_spin_unlock_irqrestore(&jpeg_id_lock, flags);

	if (kdrv_id < 0) {
		DBG_ERR("chip=%d, engine=%d, jpeg get encode channel fail, please increase jpeg_enc_max_chn !! \n", chip, engine);
	}

	return kdrv_id;
}

int jpeg_dec_get_unused_channel(UINT32 chip, UINT32 engine)
{
	int chn, kdrv_id = -1;
	unsigned long flags;
	struct jpeg_dec_data_t *p_dec_data;
	vk_spin_lock_irqsave(&jpeg_id_lock, flags);
	for (chn = 0; chn < jpeg_dec_max_chn; chn++) {
		p_dec_data = jpeg_dec_data + JPEGENC_IDX(chip, chn);

		if (0 == p_dec_data->active) {
			p_dec_data->active = 1;
			kdrv_id = KDRV_DEV_ID(chip, engine, chn);
			break;
		}
	}
	vk_spin_unlock_irqrestore(&jpeg_id_lock, flags);

	if (kdrv_id < 0) {
		DBG_ERR("chip=%d, engine=%d, jpeg get encode channel fail, please increase jpeg_enc_max_chn !! \n", chip, engine);
	}

	return kdrv_id;
}

int jpeg_enc_release_channel(unsigned int kdrv_id)
{
	unsigned int chip, chn;
	unsigned long flags;
	struct jpeg_enc_data_t *p_enc_data;

	chip = KDRV_DEV_ID_CHIP(kdrv_id);
	chn = KDRV_DEV_ID_CHANNEL(kdrv_id);

	// p_enc_data = &jpeg_enc_data[JPEGENC_IDX(chip, chn)];
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);

	vk_spin_lock_irqsave(&jpeg_id_lock, flags);
	p_enc_data->active = 0;
	vk_spin_unlock_irqrestore(&jpeg_id_lock, flags);

	// memcpy(p_enc_data->param_cfg.osg_rgb_cfg, &JPEG_ENC_OSG_RGB, sizeof(JPEG_OSG_RGB_CFG));
	// memcpy(p_enc_data->param_cfg.osg_pal_cfg, JPEG_ENC_OSG_PAL, sizeof(p_enc_data->param_cfg.osg_pal_cfg));
	// memset(p_enc_data->param_cfg.osg_win_cfg, 0, sizeof(p_enc_data->param_cfg.osg_win_cfg));
	
	return 0;
}

int jpeg_dec_release_channel(unsigned int kdrv_id)
{
	unsigned int chip, chn;
	unsigned long flags;
	struct jpeg_dec_data_t *p_dec_data;

	chip = KDRV_DEV_ID_CHIP(kdrv_id);
	chn = KDRV_DEV_ID_CHANNEL(kdrv_id);
	// p_dec_data = &jpeg_dec_data[JPEGDEC_IDX(chip, chn)];
	p_dec_data = jpeg_dec_data + JPEGDEC_IDX(chip, chn);

	vk_spin_lock_irqsave(&jpeg_id_lock, flags);
	p_dec_data->active = 0;
	vk_spin_unlock_irqrestore(&jpeg_id_lock, flags);

	return 0;
}

////////// get decode data //////////
struct jpeg_dec_data_t *jpeg_get_dec_data(UINT32 chip, UINT32 chn)
{
	struct jpeg_dec_data_t *p_dec_data;

	if (chn >= jpeg_dec_max_chn) {
		DBG_ERR("jpeg decode channel(%d) out of range (%d)\n", chn, jpeg_dec_max_chn);
		return NULL;
	}
	p_dec_data = jpeg_dec_data + JPEGENC_IDX(chip, chn);

	return p_dec_data;
}

////////// get encode data //////////
struct jpeg_enc_data_t *jpeg_get_enc_data(UINT32 chip, UINT32 chn)
{
	struct jpeg_enc_data_t *p_enc_data;

	if (chn >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode channel(%d) out of range (%d)\n", chn, jpeg_enc_max_chn);
		return NULL;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);

	return p_enc_data;
}

#if SUP_NON_BLK_MODE
#ifdef __KERNEL__
static int jpeg_enc_clear_channel(unsigned int chip, unsigned int chn)
{
	struct jpeg_enc_data_t *p_enc_data;
	unsigned long flags;

	p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);
	vk_spin_lock_irqsave(&jpeg_job_lock, flags);
	if (vos_list_empty(&jpeg_enc_minor_head[JPEGENC_IDX(chip, chn)])) {
		#if 0
		memset(&p_enc_data->enc_cfg, 0, sizeof(JPEG_PROCESS_CFG));
		#else
		memset(&p_enc_data->enc_cfg.stOsg.stOsgWin, 0, sizeof(p_enc_data->enc_cfg.stOsg.stOsgWin));
		memset(&p_enc_data->enc_cfg.stMask, 0, sizeof(p_enc_data->enc_cfg.stMask));
		p_enc_data->enc_cfg.stRC.enable = 0;
		p_enc_data->enc_cfg.stGray.enable = 0;
		p_enc_data->enc_cfg.uiRotation = 0;
		memset(&p_enc_data->enc_cfg.stUserData, 0, sizeof(p_enc_data->enc_cfg.stUserData));	
		#endif
		p_enc_data->new_feature = 0;
	}
	vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
	return 0;
}
#endif //__KERNEL__

////////// job item //////////
static struct job_item_t *jpeg_job_alloc(void)
{
	unsigned int i;
	struct job_item_t *job_item = NULL, *unused_job = NULL;
	unsigned long flags;

	vk_spin_lock_irqsave(&jpeg_jobitem_lock, flags);
	for (i = 0; i < job_item_pool.total_num; i++) {
		job_item = &job_item_pool.job_item_array[job_item_pool.cur_idx];
		if (0 == job_item->is_used) {
			memset(job_item, 0x0, sizeof(struct job_item_t));	//Harry: clear to zero for safty
			unused_job = job_item;
			job_item->is_used = 1;
			job_item_pool.cur_idx = (job_item_pool.cur_idx + 1) % job_item_pool.total_num;
			break;
		}
		job_item_pool.cur_idx = (job_item_pool.cur_idx + 1) % job_item_pool.total_num;
	}
	vk_spin_unlock_irqrestore(&jpeg_jobitem_lock, flags);
	DBG_IND("get job[%d] item 0x%lx\n", i, (unsigned long)unused_job);
	return unused_job;
}

static void jpeg_job_free(void *job)
{
	struct job_item_t *job_item = (struct job_item_t *)job;
	unsigned long flags;

	/* if job_item is freed to job_item_t database, thus use jpeg_jobitem_lock instead of jpeg_job_lock */
	vk_spin_lock_irqsave(&jpeg_jobitem_lock, flags);
	job_item->is_used = 0;
	vk_spin_unlock_irqrestore(&jpeg_jobitem_lock, flags);
}

static BOOL jpeg_engine_available(int engine, struct job_item_t *job_item)
{
	struct jpeg_enc_data_t *p_enc_data;
	BOOL ret = TRUE;

	if (JPEG_ENC_TYPE == job_item->jpeg_type) {
		if (engine == JPEG_ENGINE_LITE_IDX && 0 == job_item->trans422) {
			DBG_IND("%s: encode engine=%d, but not in trans422 mode \n", __FUNCTION__, engine);
			ret = FALSE;
			goto exit_check;
		}
		if (job_item->chn >= jpeg_enc_max_chn) {
			//DBG_ERR("jpeg encode putjob channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
			ret = FALSE;
			goto exit_check;
		}
		p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, job_item->chn);
		if (engine == JPEG_ENGINE_LITE_IDX && p_enc_data->new_feature) {
			ret = FALSE;
			goto exit_check;
		}
	}
	else {	// decode
		if (engine == JPEG_ENGINE_LITE_IDX) {
			ret = FALSE;
			goto exit_check;
		}
	}
	
exit_check:
	return ret;
}

/* get one job from engine list but not removed from the related list_head */
struct job_item_t *jpeg_get_next_job(unsigned int chip, unsigned int engine)
{
	struct job_item_t *job_item;
	struct job_item_t *target_job = NULL;
	unsigned long flags = 0;

	if (chip >= jpeg_max_chip) { // || engine >= jpeg_max_engine) {
		DBG_ERR("chip (%d-%d) > max chip id (%d/%d)\n", chip, engine, jpeg_max_chip, jpeg_max_engine);
		return NULL;
	}

	// select job
	vk_spin_lock_irqsave(&jpeg_job_lock, flags);
	if (vos_list_empty(&jpeg_job_head[chip])) {
		DBG_IND("job list empty\n");
		goto exit_getjob;
	}

	/* not del from engine_list yet */
	vos_list_for_each_entry(job_item, &jpeg_job_head[chip], engine_list) {
		DBG_IND("select job_id = %d, job status = %d, job chn = %d, job engine = %d\n", job_item->job_id, job_item->status, job_item->chn, job_item->engine);
		// check lite and new feature
		if (FALSE == jpeg_engine_available(engine, job_item)) {
			DBG_IND("engine lite not support new feature\n");
			continue;
		}
		if (job_item->status == DRIVER_STATUS_STANDBY) {
			target_job = job_item;
			target_job->status = DRIVER_STATUS_ONGOING;
			break;
		}
	}

exit_getjob:
	vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
	return target_job;
}

///////// set callback function //////////
void jpeg_enc_set_cb(KDRV_CALLBACK_FUNC *p_cb_func)
{
	g_jpeg_enc_cb = p_cb_func;
}
KDRV_CALLBACK_FUNC *jpeg_enc_get_cb(void)
{
	return g_jpeg_enc_cb;
}

void jpeg_dec_set_cb(KDRV_CALLBACK_FUNC *p_cb_func)
{
	g_jpeg_dec_cb = p_cb_func;
}
KDRV_CALLBACK_FUNC *jpeg_dec_get_cb(void)
{
	return g_jpeg_dec_cb;
}

////////// callback. go through all engines //////////
/* Notice: this function is jpeg_job_lock atomic context */
static void jpeg_handle_callback_job(struct job_item_t *job_item)
{
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(job_item->chip, job_item->engine);

	if (eng_data_idx >= JPEG_MAX_CHIP*JPEG_MAX_ENG) {
		DBG_ERR("chip:%d, eng:%d over-boundary error! \n", job_item->chip, job_item->engine);
		return;
	}

	//jpeg_eng_data[eng_data_idx].callback_cnt ++;

	if (JPEG_ENC_TYPE == job_item->jpeg_type) {
		KDRV_VDOJPGE_RESULT enc_result;
		// JPEG_RC_INFO rc_info = {0};

		enc_result.job_id = job_item->job_id;
		enc_result.bs_len = job_item->bs_length;
		enc_result.first_bs_len = job_item->first_bs_length;
		enc_result.bs_offset = job_item->bs_offset;

		// jpeg_get_rc_info(0, job_item->chn, &rc_info);
		enc_result.base_qp = job_item->enc_param.base_qp;

		if (job_item->status == DRIVER_STATUS_FAIL) {
			enc_result.status = KDRV_VDOJPGE_FAIL;
			enc_result.errorcode = job_item->enc_param.errorcode;
			DBG_FUNC(BHRED"(enc_result) job_id/status=0x%x/%d, errorcode=0x%x, bs_len/offset=0x%x/0x%x, base_qp=%d, bs_pa/size[0]=0x%lx/0x%x, bs_pa/size[1]=0x%lx/0x%x "COLOR_END, 
							enc_result.job_id, job_item->status, enc_result.errorcode, enc_result.bs_len, enc_result.bs_offset, enc_result.base_qp, 
							(unsigned long)job_item->enc_param.bs_addr_pa[0], job_item->enc_param.bs_buf_size[0],
							(unsigned long)job_item->enc_param.bs_addr_pa[1], job_item->enc_param.bs_buf_size[1]);
			//jpeg_eng_data[eng_data_idx].callback_err_cnt ++;
		} else {
			enc_result.status = KDRV_VDOJPGE_OK;
			enc_result.errorcode = 0;
			DBG_FUNC(BHGRN"(enc_result) job_id/status=0x%x/%d, errorcode=0x%x, bs_len/offset=0x%x/0x%x, base_qp=%d, bs_pa/size[0]=0x%lx/0x%x, bs_pa/size[1]=0x%lx/0x%x "COLOR_END, 
							enc_result.job_id, job_item->status, enc_result.errorcode, enc_result.bs_len, enc_result.bs_offset, enc_result.base_qp, 
							(unsigned long)job_item->enc_param.bs_addr_pa[0], job_item->enc_param.bs_buf_size[0],
							(unsigned long)job_item->enc_param.bs_addr_pa[1], job_item->enc_param.bs_buf_size[1]);
		}

		//job_item->callbacktime = jpeg_get_timems();

		/* callback to flow */
		if (job_item->p_cb_func) {
			KDRV_CALLBACK_FUNC *p_cb_func = (KDRV_CALLBACK_FUNC *)job_item->p_cb_func;
			p_cb_func->callback(&enc_result, job_item->user_data);
			DBG_FUNC("je job 0x%x callback: status %d\n", job_item->job_id, enc_result.status);
		}
	}
	else if (JPEG_DEC_TYPE == job_item->jpeg_type) {
		KDRV_VDOJPGD_RESULT dec_result;
		dec_result.job_id = job_item->job_id;
		dec_result.img_width = job_item->img_width;
		dec_result.img_height = job_item->img_height;
		dec_result.bg_width = job_item->bg_width;
		dec_result.bg_height = job_item->bg_height;
		dec_result.img_fmt = job_item->dec_param.yuv_fmt;

		if (job_item->status == DRIVER_STATUS_FAIL) {
			dec_result.status = KDRV_VDOJPGD_FAIL;
			dec_result.errorcode = job_item->dec_param.errorcode;
			DBG_FUNC(BHRED"(dec_result) job_id=0x%x, img_w/img_h=%d/%d, bg_w/bg_h=%d/%d, img_fmt=%d(11: 420sp, 1: 422), errorcode=0x%x "COLOR_END, 
							dec_result.job_id, dec_result.img_width, dec_result.img_height, dec_result.bg_width, dec_result.bg_height, dec_result.img_fmt, dec_result.errorcode);
		} else {
			dec_result.status = KDRV_VDOJPGD_OK;
			dec_result.errorcode = 0;
			DBG_FUNC(BHGRN"(dec_result) job_id=0x%x, img_w/img_h=%d/%d, bg_w/bg_h=%d/%d, img_fmt=%d(11: 420sp, 1: 422), errorcode=0x%x "COLOR_END, 
							dec_result.job_id, dec_result.img_width, dec_result.img_height, dec_result.bg_width, dec_result.bg_height, dec_result.img_fmt, dec_result.errorcode);
		}

		/* callback to flow */
		if (job_item->p_cb_func) {
			KDRV_CALLBACK_FUNC *p_cb_func = (KDRV_CALLBACK_FUNC *)job_item->p_cb_func;
			p_cb_func->callback(&dec_result, job_item->user_data);
			DBG_FUNC("jd job 0x%x callback: status %d\n", job_item->job_id, dec_result.status);
		}
	}
	return;
}

void jpeg_callback_scheduler(unsigned long data)
{
	struct job_item_t *job_item, *job_item_next;
	unsigned long flags;
	unsigned int chip, engine, eng_data_idx;

	for (chip = 0; chip < jpeg_max_chip; chip ++) {
		for (engine = 0; engine < jpeg_max_engine; engine ++) {
			eng_data_idx = JPEG_ENTITY_IDX(chip, engine);

			/* lock */
			vk_spin_lock_irqsave(&jpeg_job_lock, flags);

			vos_list_for_each_entry_safe(job_item, job_item_next, &jpeg_job_head[eng_data_idx], engine_list) {
				DBG_IND(BHGRN"job_id = 0x%x, job_status = %d(3: DRIVER_STATUS_FINISH, 4: DRIVER_STATUS_FAIL) "COLOR_END, job_item->job_id, job_item->status);
				if ((DRIVER_STATUS_FINISH == job_item->status) || (DRIVER_STATUS_FAIL == job_item->status)) {
					vos_list_del_init(&job_item->engine_list);
					vos_list_del_init(&job_item->minor_list);

					/* callback to flow */
					// jpeg_handle_callback_job(job_item);
					/* free this job */
					jpeg_job_free(job_item);
				}
			}
			/* unlock */
			vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
		}
	}
	//DBG_UNIT("jpeg, OUT\n");
}

void trigger_callback(struct job_item_t *job_item, int status)
{
	unsigned long flags;
	//int eng_data_idx;

	if (NULL == job_item) {
		DBG_ERR("job item is null, status = %d\n", status);
		return;
	}

	//eng_data_idx = JPEG_ENTITY_IDX(job_item->chip, job_item->engine);
	//DBG_UNIT("eng_data_idx = %d \n", eng_data_idx);
	/*
	if (job_item->jpeg_type == JPEG_DEC_TYPE) {
		jpeg_eng_data[eng_data_idx].job_dec_done_cnt ++;
	} else {
		jpeg_eng_data[eng_data_idx].job_enc_done_cnt ++;
	}
	*/

	vk_spin_lock_irqsave(&jpeg_job_lock, flags);
	job_item->status = (status < 0) ? DRIVER_STATUS_FAIL : DRIVER_STATUS_FINISH;
	vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);

	if (status < 0) {
		if (printk_ratelimit())
			DBG_FUNC("jpeg, job 0x%x callback fail\n", job_item->job_id);
	} else {
		DBG_FUNC("jpeg, job 0x%x callback finish\n", job_item->job_id);
	}

	/* callback to flow */
	jpeg_handle_callback_job(job_item);

	tasklet_schedule(&jpeg_cb_tasklet);
}

////////// start job //////////
int jpeg_start_job(struct job_item_t *job_item, unsigned int engine)
{
	int ret = -1;
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(job_item->chip, engine);
	uintptr_t io_addr = jpeg_eng_data[eng_data_idx].engine_base_va;

	if (JPEG_GETREG(JPG_VERSION_OFS) == 0) {
		DBG_ERR("jpeg hw version(%x) error !! \n", (unsigned int)JPEG_GETREG(JPG_VERSION_OFS));
		return -1;
	}

	if (JPEG_DEC_TYPE == job_item->jpeg_type) {
		struct jpeg_dec_data_t *p_dec_data;

		p_dec_data = jpeg_get_dec_data(job_item->chip, job_item->chn);
		if (NULL == p_dec_data) {
			ret = -1;
			DBG_ERR("p_dec_data == NULL !! \n");
			goto exit_start;
		}

		DBG_UNIT("{ch%d} jd start job 0x%x, engine %d\n", job_item->chn, job_item->job_id, job_item->engine);
		/* start. dec_param: associated with job provided by kflow. param_cfg: provided by kdrv_videoenc_jpeg_set() */
		jpeg_set_codec_type(&jpeg_eng_data[eng_data_idx], JPEG_CODEC_MODE_DEC);
		jpeg_mark_engine_start(job_item->chip, job_item->engine);
		ret = jpeg_dec_trigger((void *)&job_item->dec_param, job_item->p_cb_func, io_addr);
		if (ret != E_OK) {
			jpeg_mark_engine_end(job_item->chip, job_item->engine, 0);
			// callback fail
			if (job_item->dec_param.errorcode == JPG_HEADER_ER_SOF_SFY) {
				if (jpeg_swdec_func == NULL) {
					DBG_UNIT("not support format \n");
					job_item->dec_param.errorcode = KDRV_JPGD_N_SUP_FMT;
					goto exit_start;
				}
				// sw decode
				#if JPEG_SW_DECODE
				DBG_IND("jpeg decode not support(0x%x), force SW decode\r\n", job_item->dec_param.errorcode);
				jpeg_eng_data[eng_data_idx].sw_dec_job_item = job_item;
				jpg_sw_dec_wakeup(job_item->chip, engine);
				ret = 0;
				goto exit_start;
				#else
				DBG_WRN("jpeg decode not support this format\r");
				ret = -1;
				goto exit_start;
				#endif
			}
			else {
				ret = -1;
				goto exit_start;
			}
		}
	}
	else if (JPEG_ENC_TYPE == job_item->jpeg_type) {
		struct jpeg_enc_data_t *p_enc_data;

		p_enc_data = jpeg_get_enc_data(job_item->chip, job_item->chn);
		if (NULL == p_enc_data) {
			ret = -1;
			DBG_ERR("p_enc_data == NULL !! \n");
			goto exit_start;
		}

		//job_item->enc_param.engine_idx = job_item->chip;

		//printk("job_item->chip = %d, eng_pa=0x%lx \n", job_item->chip, (unsigned long)jpeg_eng_data[eng_data_idx].engine_base_pa);

		/* start. enc_param: associated with job provided by kflow. param_cfg: provided by kdrv_videoenc_jpeg_set() */
		DBG_UNIT("{ch%d} je start job 0x%x, engine %d io_addr 0x%lx\n", job_item->chn, job_item->job_id, job_item->engine, (unsigned long)io_addr);
		jpeg_set_codec_type(&jpeg_eng_data[eng_data_idx], JPEG_CODEC_MODE_ENC);
		jpeg_mark_engine_start(job_item->chip, job_item->engine);
		ret = jpeg_enc_trigger((void *)&job_item->enc_param, &p_enc_data->enc_cfg, io_addr);
		if (ret != E_OK) {
			jpeg_mark_engine_end(job_item->chip, job_item->engine, 1);
			ret = -1;
		}
		else
			ret = 0;
	}

exit_start:
	return ret;
}

void jpeg_start_handler(unsigned long data)
{
	struct job_item_t *target_job = NULL;
	int data_eng_idx = (unsigned int)data;
	int chip = JPEG_CHIP_ID((unsigned int)data);
	int engine = JPEG_ENGINE_ID((unsigned int)data);
	unsigned long flags;

	DBG_IND("start %s (chip %d, engine %d)\n", __FUNCTION__, chip, engine);
	if (chip >= jpeg_max_chip || engine > JPEG_MAX_ENG) {
		DBG_ERR("%s: chip id(%d,%d) over max chip id(%d,%d)\n", __FUNCTION__, chip, engine, jpeg_max_chip, JPEG_MAX_ENG);
		goto exit_schedule;
	}
	/* fix the behavior identical to linux test_and_set_bit() */
	if (1 == jpeg_test_and_set_engine_busy(chip, engine)) {
		DBG_FUNC("{chip%d/eng%d} engine busy\r\n", chip, engine);	/* it is normal case due to two callers */
		goto exit_schedule;
	}

find_next_job:
	/* get one job from this engine list but not remove yet. It is removed in callback scheduler. */
	target_job = jpeg_get_next_job(chip, engine);
	if (NULL == target_job) {
		DBG_FUNC("{chip%d/eng%d} can not find job\r\n", chip, engine);

		/* clock close */
		vk_spin_lock_irqsave(&jpeg_eng_data[data_eng_idx].engine_lock, flags);
		if (jpeg_eng_data[data_eng_idx].clock_status == 1) {
			jpeg_eng_data[data_eng_idx].clock_status = 0;
			vk_spin_unlock_irqrestore(&jpeg_eng_data[data_eng_idx].engine_lock, flags);
			jpeg_platform_clk_enable(jpeg_eng_data[data_eng_idx].mclk, 0, &jpeg_eng_data[data_eng_idx]);
		} else {
			vk_spin_unlock_irqrestore(&jpeg_eng_data[data_eng_idx].engine_lock, flags);
		}

		jpeg_set_engine_idle(chip, engine);
		goto exit_schedule;
	} else {
		DBG_IND("{chip%d/eng%d} find job %d\n", target_job->chip, target_job->engine, target_job->job_id);

		/* clock open */
		vk_spin_lock_irqsave(&jpeg_eng_data[data_eng_idx].engine_lock, flags);
		if (jpeg_eng_data[data_eng_idx].clock_status == 0) {
			jpeg_eng_data[data_eng_idx].clock_status = 1;
			vk_spin_unlock_irqrestore(&jpeg_eng_data[data_eng_idx].engine_lock, flags);
			jpeg_platform_clk_enable(jpeg_eng_data[data_eng_idx].mclk, 1, &jpeg_eng_data[data_eng_idx]);
		} else {
			vk_spin_unlock_irqrestore(&jpeg_eng_data[data_eng_idx].engine_lock, flags);
		}

		/* keep the current job processed by this engine */
		jpeg_eng_data[data_eng_idx].fire_job = target_job; /* handled in interupt handler: jpeg_drv_isr() */
		target_job->engine = engine;
		if (jpeg_start_job(target_job, engine) < 0) {
			DBG_ERR("{chip%d/eng%d} start jpeg error\r\n", target_job->chip, target_job->engine);
			//jpeg_eng_data[data_eng_idx].fire_error ++;
			jpeg_sync_job(target_job, -1); /* callback to kflow */
			jpeg_eng_data[data_eng_idx].fire_job = NULL;
			goto find_next_job;
		}
	}
	
exit_schedule:
	return;	
}

////////// job function //////////
static int jpeg_sync_job(struct job_item_t *job_item, int status)
{
	//unsigned int	eng_data_idx;
	int ret = 0;

	if (status < 0) {
		trigger_callback(job_item, -1);
	} else {
		if (JPEG_DEC_TYPE == job_item->jpeg_type) {
			job_item->img_width = job_item->dec_param.uiWidth;
			job_item->img_height = job_item->dec_param.uiHeight;
			job_item->bg_width = ALIGN_64(job_item->dec_param.uiWidth);
			job_item->bg_height = ALIGN_16(job_item->dec_param.uiHeight);
		}
		else if (JPEG_ENC_TYPE == job_item->jpeg_type) {
			struct jpeg_enc_data_t *p_enc_data;

			p_enc_data = jpeg_get_enc_data(job_item->chip, job_item->chn);
			if (NULL == p_enc_data) {
				status = -1;
			}

			if (status >= 0) {
#if SUPPORT_JPEGENC_SLICE
				if (job_item->enc_param.enc_slice_en) {
					if (0x80000000 & job_item->enc_param.slice_cnt)
						job_item->bs_length = job_item->enc_param.bs_hdr_size + jpeg_get_bssize(jpeg_eng_data[job_item->engine].engine_base_va);
					else
						job_item->bs_length = job_item->enc_param.bs_hdr_size + jpeg_get_bssize(jpeg_eng_data[job_item->engine].engine_base_va) - 1;
					// DBG_WRN("end ===== hdr_size(0x%x), [engine=%d]bs_size(0x%x), bs_length(0x%x) \n\n", 
					// 			job_item->enc_param.bs_hdr_size, job_item->engine, jpeg_get_bssize(jpeg_eng_data[job_item->engine].engine_base_va), job_item->bs_length);
				} else {
					job_item->bs_length = job_item->enc_param.bs_hdr_size + jpeg_get_bssize(jpeg_eng_data[job_item->engine].engine_base_va);
					DBG_IND("hdr_size=0x%x, engine=%d, io_addr=0x%lx, bs_size=0x%x, bs_length(0x%x) \n", job_item->enc_param.bs_hdr_size, job_item->engine, 
								(unsigned long)jpeg_eng_data[job_item->engine].engine_base_va, jpeg_get_bssize(jpeg_eng_data[job_item->engine].engine_base_va), job_item->bs_length);
				}
#else
				job_item->bs_length = job_item->enc_param.bs_hdr_size + jpeg_get_bssize(jpeg_eng_data[job_item->engine].engine_base_va);
#endif
				job_item->bs_offset = job_item->enc_param.bs_offset;
				//jpeg_enc_brc_update_quality(job_item->chn, (void *)(&p_enc_data->param_cfg.rc_param), job_item->bs_length, job_item->chip, jpeg_eng_data[eng_data_idx].engine_base_va);
#if SUPPORT_JPEGENC_SLICE
				if (job_item->enc_param.enc_slice_en) {
					if (0 == job_item->enc_param.slice_cnt)
						job_item->bs_slice_size = job_item->bs_length;
					else
						job_item->bs_slice_size += job_item->bs_length;
					if (0x80000000 & job_item->enc_param.slice_cnt)
						jpeg_brc_update(&job_item->enc_param, &p_enc_data->enc_cfg.stRC, job_item->bs_slice_size, job_item->engine, jpeg_eng_data[job_item->engine].engine_base_va);
				}
				else 
#endif
				{
					jpeg_brc_update(&job_item->enc_param, &p_enc_data->enc_cfg.stRC, job_item->bs_length, job_item->engine, jpeg_eng_data[job_item->engine].engine_base_va);
#if JPEG_APP_HEADER
					//app header add checksum
					if (g_HeaderChecksumEn) {
						UINT32 checksum;
						unsigned char *bs_buffer;

						checksum = jpegtest_getbs_checksum(jpeg_eng_data[job_item->engine].engine_base_va);
						bs_buffer = (unsigned char *)job_item->enc_param.bs_addr_va[0];
						bs_buffer[6] = (checksum)	  & 0xFF;
						bs_buffer[7] = (checksum>> 8) & 0xFF;
						bs_buffer[8] = (checksum>>16) & 0xFF;
						bs_buffer[9] = (checksum>>24) & 0xFF;
						ret = jpeg_platform_dma_flush_dev2mem(1, CACHE_ALIGN_UP((unsigned long)&bs_buffer[6]), CACHE_ALIGN_UP(64));
					}
#endif
				}
			}
			// {
			// 	UINT32 tmp_out_i;
			// 	unsigned char *bs_buffer_1;

			// 	bs_buffer_1 = (unsigned char *)job_item->enc_param.bs_addr_va[0];
			// 	for (tmp_out_i = 0x00; tmp_out_i < 0x10; tmp_out_i += 0x10) {
			// 		DBG_WRN("start bs_buffer_1(va=0x%lx/pa=0x%lx) [0x%05x] %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n", 
			// 					(unsigned long)bs_buffer_1, job_item->enc_param.bs_addr_pa[0], tmp_out_i,
			// 					bs_buffer_1[tmp_out_i +   0], bs_buffer_1[tmp_out_i +   1], bs_buffer_1[tmp_out_i +   2], bs_buffer_1[tmp_out_i +   3],
			// 					bs_buffer_1[tmp_out_i +   4], bs_buffer_1[tmp_out_i +   5], bs_buffer_1[tmp_out_i +   6], bs_buffer_1[tmp_out_i +   7],
			// 					bs_buffer_1[tmp_out_i +   8], bs_buffer_1[tmp_out_i +   9], bs_buffer_1[tmp_out_i + 0xA], bs_buffer_1[tmp_out_i + 0xB],
			// 					bs_buffer_1[tmp_out_i + 0xC], bs_buffer_1[tmp_out_i + 0xD], bs_buffer_1[tmp_out_i + 0xE], bs_buffer_1[tmp_out_i + 0xF]);
			// 	}
			// 	for (tmp_out_i = jpeg_get_bssize(jpeg_eng_data[job_item->engine].engine_base_va); tmp_out_i > jpeg_get_bssize(jpeg_eng_data[job_item->engine].engine_base_va) - 0x10; tmp_out_i -= 0x10) {
			// 		DBG_WRN("..end bs_buffer_1(va=0x%lx/pa=0x%lx) [0x%05x] %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n", 
			// 					(unsigned long)bs_buffer_1, job_item->enc_param.bs_addr_pa[0], tmp_out_i,
			// 					bs_buffer_1[tmp_out_i - 0xF], bs_buffer_1[tmp_out_i - 0xE], bs_buffer_1[tmp_out_i - 0xD], bs_buffer_1[tmp_out_i - 0xC],  
			// 					bs_buffer_1[tmp_out_i - 0xB], bs_buffer_1[tmp_out_i - 0xA], bs_buffer_1[tmp_out_i -   9], bs_buffer_1[tmp_out_i -   8],  
			// 					bs_buffer_1[tmp_out_i -   7], bs_buffer_1[tmp_out_i -   6], bs_buffer_1[tmp_out_i -   5], bs_buffer_1[tmp_out_i -   4],  
			// 					bs_buffer_1[tmp_out_i -   3], bs_buffer_1[tmp_out_i -   2], bs_buffer_1[tmp_out_i -   1], bs_buffer_1[tmp_out_i -   0]);
			// 	}
			// 	DBG_WRN("\n");
			// }
		}
		trigger_callback(job_item, 0);
	}

	return ret;
}

int jpeg_process_done(UINT32 engine_idx, UINT32 status)
{
	int ret = 0;
	//struct job_item_t *job_item;

	if (engine_idx >= JPEG_MAX_ENG) {
		ret = -1;
		goto exit_process;
	}

	if (JPEG_INT_FRAMEEND != status) {
		jpeg_sync_job(jpeg_eng_data[JPEG_ENTITY_IDX(0, engine_idx)].fire_job, -1);
	}
	else {
		jpeg_sync_job(jpeg_eng_data[JPEG_ENTITY_IDX(0, engine_idx)].fire_job, 0);
	}
	
exit_process:
	return ret;
}

int jpeg_start_schedule(UINT32 chip, UINT32 engine)
{
	int eng_idx;
	if (JPEG_ALL_ENGINE == engine) {
		for (eng_idx = 0; eng_idx < JPEG_MAX_ENG; eng_idx++) {
		//for (eng_idx = JPEG_MAX_ENG-1; eng_idx >= 0; eng_idx--) {
			DBG_MSG("(multi) start tasklet %d-%d (0x%lx) \n", chip, eng_idx, (unsigned long)&jpeg_job_tasklet[chip][eng_idx]);
			tasklet_schedule(&jpeg_job_tasklet[chip][eng_idx]);
		}
	}
	else {
		DBG_MSG("(single) start tasklet %d-%d (0x%lx) \n", chip, engine, (unsigned long)&jpeg_job_tasklet[chip][engine]);
		tasklet_schedule(&jpeg_job_tasklet[chip][engine]);
	}
	return 0;
}

////////// put job //////////
int jpeg_dec_putjob(unsigned int kdrv_id, void *param, void *user_data)
{
	struct job_item_t *job_item = NULL;
	KDRV_VDOJPGD_PARAM *dec_param = (KDRV_VDOJPGD_PARAM *)param;
	struct jpeg_dec_data_t *p_dec_data;
	unsigned long flags;
	int engine_cnt;

	DBG_IND("%s: kdrv_id = 0x%x (%d), dec_idx = %d\n", __FUNCTION__, kdrv_id, KDRV_DEV_ID_CHANNEL(kdrv_id), JPEGDEC_IDX(0, KDRV_DEV_ID_CHANNEL(kdrv_id)));
	if (KDRV_DEV_ID_CHANNEL(kdrv_id) >= jpeg_dec_max_chn) {
		DBG_ERR("jpeg decode putjob channel(%d) out of range (%d)\n", KDRV_DEV_ID_CHANNEL(kdrv_id), jpeg_dec_max_chn);
		goto fail_putjob;
	}
	p_dec_data = jpeg_dec_data + JPEGDEC_IDX(0, KDRV_DEV_ID_CHANNEL(kdrv_id));
	if (NULL == p_dec_data) {
		DBG_ERR("p_dec_data == NULL !! \n");
		goto fail_putjob;
	}
	job_item = jpeg_job_alloc();
	if (NULL == job_item) {
		DBG_ERR("can not allocate job item job %u\n", dec_param->job_id);
		goto fail_putjob;
	}

	job_item->job_id = dec_param->job_id;
	job_item->chip = 0;
	job_item->engine = 0;
	job_item->chn = KDRV_DEV_ID_CHANNEL(kdrv_id);
	job_item->jpeg_type = JPEG_DEC_TYPE;
	DBG_MSG("start copy dec param\n");
	memcpy(&job_item->dec_param, dec_param, sizeof(KDRV_VDOJPGD_PARAM));
	DBG_MSG("copy dec param done\n");

#if JPEGSDK_USE_PA
	if ((job_item->dec_param.y_addr_pa == 0) || (job_item->dec_param.bs_addr_pa[0] == 0)) {
		DBG_ERR("y_addr_pa(0x%lx), bs_addr_pa[0](0x%lx), pa error !! \n", (unsigned long)job_item->dec_param.y_addr_pa, (unsigned long)job_item->dec_param.bs_addr_pa[0]);
		goto fail_putjob;
	}
#endif
	if ((job_item->dec_param.y_addr_va == 0) || (job_item->dec_param.bs_addr_va[0] == 0)) {
		DBG_ERR("y_addr_va(0x%lx), bs_addr_va[0](0x%lx), va error !! \n", (unsigned long)job_item->dec_param.y_addr_va, (unsigned long)job_item->dec_param.bs_addr_va[0]);
		goto fail_putjob;
	}
	if (job_item->dec_param.raw_size == 0) {
		DBG_ERR("raw_size(%d) error !! \n", job_item->dec_param.raw_size);
		goto fail_putjob;
	}

	job_item->user_data = user_data;
	job_item->bs_length = 0;
	job_item->bs_offset = 0;

	// job_item->p_cb_func = p_cb_func;
	if (g_jpeg_dec_cb)
		job_item->p_cb_func = g_jpeg_dec_cb;
	else {
		DBG_ERR("callback func NULL !! \n");
		goto fail_putjob;
	}

	job_item->trans422 = 0;
	DBG_MSG("put job 0x%x, chn %d\n", job_item->job_id, KDRV_DEV_ID_CHANNEL(kdrv_id));
	// insert job
	vk_spin_lock_irqsave(&jpeg_job_lock, flags);
	job_item->status = DRIVER_STATUS_STANDBY;
	VOS_INIT_LIST_HEAD(&job_item->engine_list);
	VOS_INIT_LIST_HEAD(&job_item->minor_list);

	/* added to the specific engine list */
	vos_list_add_tail(&job_item->engine_list, &jpeg_job_head[job_item->chip]);
	vos_list_add_tail(&job_item->minor_list, &jpeg_dec_minor_head[JPEGDEC_IDX(job_item->chip, job_item->chn)]);
	vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
	DBG_UNIT("{chn%d} jd insert job %u to minor %d, eng:%d\n", job_item->chn, (unsigned int)job_item->job_id, job_item->chn, job_item->engine);

	// trigger
	if (g_ChooseEngineUse == 0) {
		// multi engine
		engine_cnt = JPEG_ALL_ENGINE;
		jpeg_start_schedule(job_item->chip, engine_cnt);
	}
	else {
		// choose engine
		engine_cnt = g_ChooseEngineUse - 1;
		DBG_UNIT("{choose_engine_mode} job_id(%d), chn(%d), chip(%d), engine_cnt(%d), job_item->status(%d) \n", 
					job_item->job_id, job_item->chn, job_item->chip, engine_cnt, job_item->status);
		tasklet_schedule(&jpeg_job_tasklet[job_item->chip][engine_cnt]);
	}

	return 0;

fail_putjob:
	// callback fail
	DBG_IND("putjob fail\n");
	if (g_jpeg_dec_cb) {
		KDRV_VDOJPGD_RESULT dec_result = {0};
		dec_result.status = KDRV_VDOJPGD_FAIL;
		g_jpeg_dec_cb->callback(&dec_result, user_data);
	}
	/*
	if (job_item) {
		jpeg_job_free(job_item);
	}
	*/

	return -1;
}

////////// put job //////////
int jpeg_enc_putjob(unsigned int kdrv_id, void *param, void *user_data, KDRV_CALLBACK_FUNC *p_cb_func)
{
	struct job_item_t *job_item = NULL;
	KDRV_VDOJPGE_PARAM *enc_param = (KDRV_VDOJPGE_PARAM *)param;
	struct jpeg_enc_data_t *p_enc_data;
	unsigned long flags;
	int engine_cnt;

#if JPEGENC_CHUCKSUM
	uint checksum1, checksum2;
#endif

	DBG_IND("kdrv_id = 0x%x (%d), enc_idx = %d\n", kdrv_id, KDRV_DEV_ID_CHANNEL(kdrv_id), JPEGENC_IDX(0, KDRV_DEV_ID_CHANNEL(kdrv_id)));
	if (KDRV_DEV_ID_CHANNEL(kdrv_id) >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode putjob channel(%d) out of range (%d)\n", KDRV_DEV_ID_CHANNEL(kdrv_id), jpeg_enc_max_chn);
		goto fail_putjob;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, KDRV_DEV_ID_CHANNEL(kdrv_id));
	if (NULL == p_enc_data) {
		DBG_ERR("p_enc_data == NULL !! \n");
		goto fail_putjob;
	}
	/*
	if (unlikely(p_enc_data->active == 0)) {
		DBG_ERR("%s: Error to use channel(%d,%d) due to not active! \n", __func__, chip, chn);
		goto fail_putjob;
	}
	*/
	job_item = jpeg_job_alloc();
	if (NULL == job_item) {
		DBG_ERR("can not allocate job item job %u\n", (unsigned int)enc_param->job_id);
		goto fail_putjob;
	}

	job_item->job_id = enc_param->job_id;
	job_item->chip = 0;
	job_item->engine = 0;
	// job_item->engine = (enc_param->slice_cnt & 0x1) % 2;
	job_item->chn = KDRV_DEV_ID_CHANNEL(kdrv_id);
	job_item->jpeg_type = JPEG_ENC_TYPE;
	DBG_MSG("start copy enc param\n");
	memcpy(&job_item->enc_param, enc_param, sizeof(KDRV_VDOJPGE_PARAM));
	DBG_MSG("copy enc param done\n");
	
#if JPEGENC_CHUCKSUM
	checksum1 = heavyload_checksum(job_item->enc_param.bs_addr_va[0], job_item->enc_param.bs_buf_size[0]);
	jpeg_platform_dma_flush_dev2mem(1, CACHE_ALIGN_UP(job_item->enc_param.bs_addr_va[0]), CACHE_ALIGN_UP(job_item->enc_param.bs_buf_size[0])); //invalidate
	checksum2 = heavyload_checksum(job_item->enc_param.bs_addr_va[0], job_item->enc_param.bs_buf_size[0]);
	if (checksum1 != checksum2)
		DBG_ERR("checksum1(%d) != checksum2(%d) !! \n", checksum1, checksum2);
#endif

#if JPEGSDK_USE_PA
	if ((job_item->enc_param.y_addr_pa == 0) || (job_item->enc_param.uv_addr_pa == 0) || (job_item->enc_param.bs_addr_pa[0] == 0)) {
		DBG_ERR("y_addr_pa(0x%lx), uv_addr_pa(0x%lx), bs_addr_pa[0](0x%lx), pa error !! \n",
				(unsigned long)job_item->enc_param.y_addr_pa, (unsigned long)job_item->enc_param.uv_addr_pa, (unsigned long)job_item->enc_param.bs_addr_pa[0]);
		goto fail_putjob;
	}
#endif
	if (job_item->enc_param.bs_addr_va[0] == 0) {
		DBG_ERR("bs_addr_va[0](0x%lx), va error !! \n", (unsigned long)job_item->enc_param.bs_addr_va[0]);
		goto fail_putjob;
	}

	job_item->user_data = user_data;
	job_item->bs_length = 0;
	job_item->first_bs_length = 0;
	job_item->bs_offset = 0;

#if 1
	if (g_jpeg_enc_cb)
		job_item->p_cb_func = g_jpeg_enc_cb;
	else {
		DBG_ERR("callback func NULL !! \n");
		goto fail_putjob;
	}
#else
	job_item->p_cb_func = p_cb_func;
	if (job_item->p_cb_func == NULL) {
		DBG_ERR("job_item->p_cb_func == NULL !! \n");
		goto fail_putjob;
	}
#endif

	job_item->trans422 = enc_param->fmt_trans_en;
	DBG_MSG("put job 0x%x, chn %d\n", job_item->job_id, KDRV_DEV_ID_CHANNEL(kdrv_id));
	// insert job
	vk_spin_lock_irqsave(&jpeg_job_lock, flags);
	job_item->status = DRIVER_STATUS_STANDBY;
	VOS_INIT_LIST_HEAD(&job_item->engine_list);
	VOS_INIT_LIST_HEAD(&job_item->minor_list);

	/* added to the specific engine list */
	vos_list_add_tail(&job_item->engine_list, &jpeg_job_head[job_item->chip]);
	vos_list_add_tail(&job_item->minor_list, &jpeg_enc_minor_head[JPEGENC_IDX(job_item->chip, job_item->chn)]);
	vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
	DBG_UNIT("{chn%d} je insert job %u to minor %d, eng:%d\n", job_item->chn, (unsigned int)job_item->job_id, job_item->chn, job_item->engine);

	// trigger
	if (g_ChooseEngineUse == 0) {
		// multi engine
		engine_cnt = JPEG_ALL_ENGINE;
		jpeg_start_schedule(job_item->chip, engine_cnt);
	}
	else {
		// choose engine
		engine_cnt = g_ChooseEngineUse - 1;
		DBG_UNIT("{choose_engine_mode} job_id(%d), chn(%d), chip(%d), engine_cnt(%d), slice_cnt(0x%x/0x%x), job_item->status(%d) \n", 
					job_item->job_id, job_item->chn, job_item->chip, engine_cnt, enc_param->slice_cnt, job_item->enc_param.slice_cnt, job_item->status);
		tasklet_schedule(&jpeg_job_tasklet[job_item->chip][engine_cnt]);
	}

	return 0;

fail_putjob:
	// callback fail
	DBG_IND("putjob fail\n");
#if 1
	if (g_jpeg_enc_cb) {
		KDRV_VDOJPGE_RESULT enc_result = {0};
		enc_result.status = KDRV_VDOJPGE_FAIL;
		g_jpeg_enc_cb->callback(&enc_result, user_data);
	}
#else
	if (p_cb_func) {
		KDRV_VDOJPGE_RESULT enc_result = {0};
		enc_result.status = KDRV_VDOJPGE_FAIL;
		p_cb_func->callback(&enc_result, user_data);
	}
#endif
	/*
	if (job_item) {
		jpeg_job_free(job_item);
	}
	*/

	return -1;
}

ER jpeg_putjob(JPEG_CODEC_MODE codec_mode, unsigned int kdrv_id, void *p_param, void *user_data, KDRV_CALLBACK_FUNC *p_cb_func)
{
	ER  ret = E_OK;

    if (codec_mode == JPEG_CODEC_MODE_ENC)
        ret = jpeg_enc_putjob(kdrv_id, p_param, user_data, p_cb_func);
	else if (codec_mode == JPEG_CODEC_MODE_DEC)
        ret = jpeg_dec_putjob(kdrv_id, p_param, user_data);
	else
		ret = -1;

	return ret;
}

////////// stop job //////////
ER jpeg_dec_stopjob(unsigned int kdrv_id)
{
#ifdef __KERNEL__
	struct job_item_t *job_item, *job_item_next;
	unsigned long flags;
	unsigned int chip = 0, chn, delay_cnt = 0, empty = 0;

	DBG_IND("[JD] kdrv_id 0x%x (%d) stop job\n", kdrv_id, KDRV_DEV_ID_CHANNEL(kdrv_id));

	chn = KDRV_DEV_ID_CHANNEL(kdrv_id);
	if (chn >= jpeg_dec_max_chn) {
		DBG_ERR("jpeg decode stopjob channel(%d) out of range (%d)\n", chn, jpeg_dec_max_chn);
		goto fail_stopjob;
	}

	/*
		handle DRIVER_STATUS_STANDBY
	*/
	vk_spin_lock_irqsave(&jpeg_job_lock, flags);
	//by channel extract job_item
	vos_list_for_each_entry_safe(job_item, job_item_next, &jpeg_dec_minor_head[JPEGDEC_IDX(chip, KDRV_DEV_ID_CHANNEL(kdrv_id))], minor_list) {
		if (DRIVER_STATUS_STANDBY == job_item->status)
			job_item->status = DRIVER_STATUS_FAIL;

		DBG_IND(BHGRN"job_id = 0x%x, job_status = %d(1: DRIVER_STATUS_STANDBY, 2: DRIVER_STATUS_ONGOING) "COLOR_END, job_item->job_id, job_item->status);
	}
	vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);

	/*
		handle DRIVER_STATUS_FINISH and DRIVER_STATUS_FAIL
	*/
	// tasklet_schedule(&jpeg_cb_tasklet); //execute after leaving func
	jpeg_callback_scheduler(0);

	/*
		handle DRIVER_STATUS_ONGOING
	*/
	do {
		vk_spin_lock_irqsave(&jpeg_job_lock, flags);
		empty = vos_list_empty(&jpeg_dec_minor_head[JPEGDEC_IDX(chip, KDRV_DEV_ID_CHANNEL(kdrv_id))]);
		vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
		if (empty) {
			DBG_IND("job list empty\n");
			break;
		}
		//timeout
		if (delay_cnt >= 50) {
			while (1) {
				vk_spin_lock_irqsave(&jpeg_job_lock, flags);
				job_item = vos_list_first_entry_or_null(&jpeg_dec_minor_head[JPEGDEC_IDX(chip, KDRV_DEV_ID_CHANNEL(kdrv_id))], struct job_item_t, minor_list);
				if (job_item == NULL) {
					DBG_IND("job_item null \n");
					vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
					break;
				}
				if (DRIVER_STATUS_ONGOING == job_item->status) {
					DBG_WRN(BHYEL"wait list_empty takes too long, (warning)force callback to flow !! channel=%d, job_status=%d, engine=%d "COLOR_END, 
									KDRV_DEV_ID_CHANNEL(kdrv_id), job_item->status, job_item->engine);
					jpeg_mark_engine_end(chip, job_item->engine, 1);
					jpeg_hw_reset(jpeg_eng_data[job_item->engine].engine_base_va);
					jpeg_set_engine_idle(chip, job_item->engine);
					jpeg_start_schedule(chip, JPEG_ALL_ENGINE);
				}
				if (DRIVER_STATUS_STANDBY == job_item->status || DRIVER_STATUS_ONGOING == job_item->status) {
					//add DRIVER_STATUS_STANDBY just in case
					job_item->status = DRIVER_STATUS_FAIL;
				}
				vos_list_del_init(&job_item->engine_list);
				vos_list_del_init(&job_item->minor_list);

				/* callback to flow */
				vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
				jpeg_handle_callback_job(job_item);
				/* free this job */
				jpeg_job_free(job_item);
			}
		}
		if (jpeg_platform_atomic_context())
			mdelay(10); //only delay allowed
		else
			vos_util_delay_ms(10); //msleep
		delay_cnt ++;
	} while (!empty);

	return 0;

fail_stopjob:
#endif
	return -1;
}

ER jpeg_enc_stopjob(unsigned int kdrv_id)
{
#ifdef __KERNEL__
	struct job_item_t *job_item, *job_item_next;
	unsigned long flags;
	unsigned int chip = 0, chn, delay_cnt = 0, empty = 0;

	DBG_IND("[JE] kdrv_id 0x%x (%d) stop job\n", kdrv_id, KDRV_DEV_ID_CHANNEL(kdrv_id));

	chn = KDRV_DEV_ID_CHANNEL(kdrv_id);
	if (chn >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode stopjob channel(%d) out of range (%d)\n", chn, jpeg_enc_max_chn);
		goto fail_stopjob;
	}

	/*
		handle DRIVER_STATUS_STANDBY
	*/
	vk_spin_lock_irqsave(&jpeg_job_lock, flags);
	//by channel extract job_item
	vos_list_for_each_entry_safe(job_item, job_item_next, &jpeg_enc_minor_head[JPEGENC_IDX(chip, KDRV_DEV_ID_CHANNEL(kdrv_id))], minor_list) {
		if (DRIVER_STATUS_STANDBY == job_item->status)
			job_item->status = DRIVER_STATUS_FAIL;

		DBG_IND(BHGRN"job_id = 0x%x, job_status = %d(1: DRIVER_STATUS_STANDBY, 2: DRIVER_STATUS_ONGOING) "COLOR_END, job_item->job_id, job_item->status);
	}
	vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);

	/*
		handle DRIVER_STATUS_FINISH and DRIVER_STATUS_FAIL
	*/
	// tasklet_schedule(&jpeg_cb_tasklet); //execute after leaving func
	jpeg_callback_scheduler(0);

	/*
		handle DRIVER_STATUS_ONGOING
	*/
	do {
		vk_spin_lock_irqsave(&jpeg_job_lock, flags);
		empty = vos_list_empty(&jpeg_enc_minor_head[JPEGENC_IDX(chip, KDRV_DEV_ID_CHANNEL(kdrv_id))]);
		vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
		if (empty) {
			DBG_IND("job list empty\n");
			break;
		}
		//timeout
		if (delay_cnt >= 50) {
			while (1) {
				vk_spin_lock_irqsave(&jpeg_job_lock, flags);
				job_item = vos_list_first_entry_or_null(&jpeg_enc_minor_head[JPEGENC_IDX(chip, KDRV_DEV_ID_CHANNEL(kdrv_id))], struct job_item_t, minor_list);
				if (job_item == NULL) {
					DBG_IND("job_item null \n");
					vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
					break;
				}
				if (DRIVER_STATUS_ONGOING == job_item->status) {
					DBG_WRN(BHYEL"wait list_empty takes too long, (warning)force callback to flow !! channel=%d, job_status=%d, engine=%d "COLOR_END, 
									KDRV_DEV_ID_CHANNEL(kdrv_id), job_item->status, job_item->engine);
					jpeg_mark_engine_end(chip, job_item->engine, 1);
					jpeg_hw_reset(jpeg_eng_data[job_item->engine].engine_base_va);
					jpeg_set_engine_idle(chip, job_item->engine);
					jpeg_start_schedule(chip, JPEG_ALL_ENGINE);
				}
				if (DRIVER_STATUS_STANDBY == job_item->status || DRIVER_STATUS_ONGOING == job_item->status) {
					//add DRIVER_STATUS_STANDBY just in case
					job_item->status = DRIVER_STATUS_FAIL;
				}
				vos_list_del_init(&job_item->engine_list);
				vos_list_del_init(&job_item->minor_list);

				/* callback to flow */
				vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
				jpeg_handle_callback_job(job_item);
				/* free this job */
				jpeg_job_free(job_item);
			}
		}
		if (jpeg_platform_atomic_context())
			mdelay(10); //only delay allowed
		else
			vos_util_delay_ms(10); //msleep
		delay_cnt ++;
	} while (!empty);

	//IVOT_N12084_CO-436
	jpeg_enc_clear_channel(chip, chn);
	
	return 0;

fail_stopjob:
#endif
	return -1;
}

ER jpeg_stopjob(JPEG_CODEC_MODE codec_mode, unsigned int kdrv_id)
{
	ER  ret = E_OK;

	// DBG_DUMP(BHMAG"preempt_count=0x%x, line=%d "COLOR_END, preempt_count(), __LINE__);
	if (codec_mode == JPEG_CODEC_MODE_ENC)
		ret = jpeg_enc_stopjob(kdrv_id);
	else if (codec_mode == JPEG_CODEC_MODE_DEC)
		ret = jpeg_dec_stopjob(kdrv_id);
	else
		return -1;

	return ret;
}

#ifdef __KERNEL__
#if JPEG_SW_TIMEOUT
void jpeg_drv_reset_handler(unsigned int chip_idx, unsigned int eng_idx)
{
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip_idx, eng_idx);
	struct jpg_engine_data_t *p_eng_data = &jpeg_eng_data[eng_data_idx];
	// platform_clk_t *clk = &jpeg_eng_data[eng_data_idx].clock;
	struct job_item_t *job_item = p_eng_data->fire_job;
	unsigned long flags;

	DBG_IND("%s{chip:%d,eng:%d}!!!\n", __func__, chip_idx, eng_idx);

	// jpeg_dump_reg(p_eng_data->engine_base_va);

	jpeg_hw_reset(p_eng_data->engine_base_va);

	job_item->retrigger_cnt ++;
	/* re-trigger once */
	if (job_item->retrigger_cnt <= retrigger_cnt) {
		if (job_item->jpeg_type == JPEG_ENC_TYPE) {
			DBG_ERR("%s{chip:%d,eng:%d} jpeg encode, re-trigger job_id:%d again!!!\n", __func__, chip_idx, eng_idx, job_item->job_id);
			memcpy(&job_item->enc_param, &job_item->enc_param_backup, sizeof(KDRV_VDOJPGE_PARAM));
		} else {
			DBG_ERR("%s{chip:%d,eng:%d} jpeg decode, re-trigger job_id:%d again!!!\n", __func__, chip_idx, eng_idx, job_item->job_id);
			memcpy(&job_item->dec_param, &job_item->dec_param_backup, sizeof(KDRV_VDOJPGD_PARAM));
		}

		vk_spin_lock_irqsave(&jpeg_job_lock, flags);
		job_item->status = DRIVER_STATUS_STANDBY;
		/* re-add the head of the related list and re-trigger again. */
		vk_spin_unlock_irqrestore(&jpeg_job_lock, flags);
	} else {
		// sync job
		jpeg_sync_job(p_eng_data->fire_job, -1);
	}
	// give engine to next job
	jpeg_set_engine_idle(p_eng_data->chip_idx, p_eng_data->engine_idx);

	//tasklet_schedule(&p_eng_data->job_tasklet);
	// jpeg_trigger_job(p_eng_data->chip_idx, p_eng_data->engine_idx);
	// jpeg_start_schedule(p_eng_data->chip_idx, p_eng_data->engine_idx);
	jpeg_start_schedule(p_eng_data->chip_idx, JPEG_ALL_ENGINE/*pmodule_info->engine_idx*/);
}

/* Racing case:(1)ISR service first(normal exit) and timeout is pending (2) start_job tasklet is higher than jpeg_drv_reset_handler worker.
 * It may change engine from IDLE to busy in another core. Thus jpeg_check_engine_and_clear_timer() will be true. In this way, a wrong reset will be trigger.
 */
void jpeg_drv_bottom_half(unsigned int chip_idx, unsigned int eng_idx, int int_status)
{
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip_idx, eng_idx);
	struct jpg_engine_data_t *p_eng_data = &jpeg_eng_data[eng_data_idx];
	// platform_clk_t *clk = &jpeg_eng_data[eng_data_idx].clock;
	struct job_item_t *fire_job = NULL;
	unsigned long flags;
	// unsigned int time_start = 0;

	if (chip_idx >= JPEG_MAX_CHIP || eng_idx >= JPEG_MAX_ENG) {
		DBG_ERR("%s, chip (%d/%d) > max chip id (%d/%d)\n", __func__, chip_idx, eng_idx, jpeg_max_chip, jpeg_max_engine);
		return;
	}

	// check engine whether idle.
	if (jpeg_check_engine_and_clear_timer(p_eng_data) == 0) {
		/* engine is idle */
		DBG_FUNC("{chip:%d,eng:%d}jpeg false alarm encode status = %d, int_sts=0x%x\r\n", chip_idx, eng_idx, p_eng_data->encode_status, int_status);
		goto exit_timeout;
	}
	
	vk_spin_lock_irqsave(&p_eng_data->func_lock, flags);

	/* prevent ISR and timeout race condition. The case that this function was executed in normal flow and trigger next job; meanwhere timeout handler wakes up simultaneously....
	 * It will reset the hardware which it is running. That is wrong.
	 */
	if (int_status == JPEG_INT_TIMEOUT) {
		unsigned int diff, now_time;
		// unsigned int time_end = 0;
		
		jpeg_get_timems(&now_time);
		// jpeg_get_timems(&time_end);

		/* Note: Please be aware that timeout timer may be refresh earlier than isr_timestamp, thus isr_timestamp must be cleared before new trigger.  */
		diff = (int)now_time - (int)p_eng_data->isr_timestamp;
		// diff = vos_perf_duration(time_start, time_end);
		DBG_FUNC(" JPEG_INT_TIMEOUT ~~~~~~~~~~~~~~~~~~~~~ diff = %d \n", diff);

		if (diff < jpeg_sw_timeout_period) {
			DBG_FUNC("{chip:%d,eng:%d}jpeg false alarm timeout status = %d, diff = %d \r\n", chip_idx, eng_idx, p_eng_data->encode_status, diff);
			vk_spin_unlock_irqrestore(&p_eng_data->func_lock, flags);
			goto exit_timeout;
		}
	}
	/* Timestamp to avoid racing condition between normal ISR and timeout handler. The reason is mentioned in the func header.
	 * Note: jpeg_mark_engine_end will refresh timeout timer(timer is kicked off), we must get timer before it to avoid diff < jpeg_sw_timeout_period if JPEG_INT_TIMEOUT happens.
	 */
	jpeg_get_timems(&p_eng_data->isr_timestamp); //p_eng_data->isr_timestamp = jpeg_get_timems();
	// jpeg_get_timems(&time_start);

	vk_spin_unlock_irqrestore(&p_eng_data->func_lock, flags);

	/* clear p_eng_data->encode_status and refresh the timeout timer. */
	jpeg_mark_engine_end(p_eng_data->chip_idx, p_eng_data->engine_idx, 0);
	
	if (int_status == JPEG_INT_TIMEOUT) {
		p_eng_data->timeout_count ++;

		if (p_eng_data->active_type == JPEG_CODEC_MODE_ENC) {
			DBG_ERR("Jpeg {chip:%d,eng:%d} encode timeout\r\n", p_eng_data->chip_idx, p_eng_data->engine_idx);
		} else {
			DBG_ERR("Jpeg {chip:%d,eng:%d} decode timeout\r\n", p_eng_data->chip_idx, p_eng_data->engine_idx);
		}

		jpeg_platform_setup_timer(p_eng_data->chip_idx, p_eng_data->engine_idx, &p_eng_data->reset_timer, jpeg_drv_reset_handler, 0);	/* fire immediately */
		goto exit_timeout;
	}

	fire_job = p_eng_data->fire_job;

	if ((int_status & JPEG_INT_FRAMEEND) != JPEG_INT_FRAMEEND) {
		unsigned int	chan;
		// error handle
		//JPEG_INT_DECERR 	= 0x04, 		///< Decode error. Encounter error in JPEG decode.
		//JPEG_INT_BUFEND 	= 0x08, 		///< Bit-stream buffer end. Bit-stream length exceed specified length.

		if (int_status & JPEG_INT_BUFEND)
			DBG_WRN("interrupt status = 0x%x (JPEG_INT_BUFEND) \r\n", int_status);
		else
			DBG_FUNC("interrupt status = 0x%x \r\n", int_status);

		(p_eng_data->active_type == JPEG_CODEC_MODE_ENC) ? p_eng_data->job_enc_err_cnt ++ : p_eng_data->job_dec_err_cnt ++;
		chan = (fire_job == NULL) ? 0xFF : fire_job->chn;

		if (int_status == JPEG_INT_BUFEND) {
			if (p_eng_data->active_type == JPEG_CODEC_MODE_ENC) {
				// JPEG_RC_PARAM *p_rc_cfg;
				// struct jpeg_enc_data_t *p_enc_data;

				// if (chan != 0xFF) {
				// 	p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip_idx, chan);
				// 	p_rc_cfg = &p_enc_data->param_cfg.rc_param;

				// 	if (p_rc_cfg->enable)
				// 		printk("%s, rc_enable:%d, vbr_mode:%d, base_qp:%d \n", __func__, p_rc_cfg->enable, p_rc_cfg->vbr_mode, p_rc_cfg->base_qp);
				// 	else
				// 		printk("%s, fix quality:%d	\n", __func__, fire_job->enc_param.quality);
				// }
				if (printk_ratelimit())
					DBG_IND("%s, jpeg encode buffer not enough(%d)!!!(chip:%d,eng:%d,chan:%d) \n", __func__, jpeg_get_bssize(p_eng_data->engine_base_va), chip_idx, eng_idx, chan);
				
				p_eng_data->fire_job->enc_param.errorcode = KDRV_JPGE_BUFFER_FULL;
			} else {
				if (printk_ratelimit())
					DBG_IND("%s, jpeg deccode buffer not enough(%d)!!!(chip:%d,eng:%d,chan:%d) \n", __func__, jpeg_get_bssize(p_eng_data->engine_base_va), chip_idx, eng_idx, chan);
			}
		}

		jpeg_platform_setup_timer(p_eng_data->chip_idx, p_eng_data->engine_idx, &p_eng_data->reset_timer, jpeg_drv_reset_handler, 0);	/* fire immediately */

		goto exit_timeout;
	}

	// flush_dev2mem bs
	if (p_eng_data->active_type == JPEG_CODEC_MODE_ENC) {
		uintptr_t bs_addr;
		UINT32 bs_size;

		bs_addr = p_eng_data->fire_job->enc_param.bs_addr_va[0] + p_eng_data->fire_job->enc_param.bs_hdr_size;
		if (bs_addr != CACHE_ALIGN_DOWN(p_eng_data->fire_job->enc_param.bs_addr_va[0] + p_eng_data->fire_job->enc_param.bs_hdr_size))
			bs_size = CACHE_ALIGN_UP(jpeg_get_bssize(p_eng_data->engine_base_va)) + CACHE_LINE_SZ;
		else
			bs_size = CACHE_ALIGN_UP(jpeg_get_bssize(p_eng_data->engine_base_va));

		bs_addr = CACHE_ALIGN_DOWN(p_eng_data->fire_job->enc_param.bs_addr_va[0] + p_eng_data->fire_job->enc_param.bs_hdr_size);
		jpeg_platform_dma_post_flush_dev2mem(bs_addr, bs_size); //because bs_addr ALIGN_DOWN, add CACHE_LINE_SZ to keep cache_size enough
	}

	// sync job
	jpeg_sync_job(p_eng_data->fire_job, 0);

	// give engine to next job
	jpeg_set_engine_idle(p_eng_data->chip_idx, p_eng_data->engine_idx);

	//tasklet_schedule(&p_eng_data->job_tasklet);
	// jpeg_trigger_job(p_eng_data->chip_idx, p_eng_data->engine_idx);
	// jpeg_start_schedule(p_eng_data->chip_idx, p_eng_data->engine_idx);
	jpeg_start_schedule(p_eng_data->chip_idx, JPEG_ALL_ENGINE/*pmodule_info->engine_idx*/);

exit_timeout:
	return;
}
#endif //SUP_NON_BLK_MODE

////////// timeout //////////
/* WorkQueue context. */
void jpeg_timeout_handle(unsigned int chip_idx, unsigned int eng_idx)
{
	int eng_data_idx = JPEG_ENTITY_IDX(chip_idx, eng_idx);
	struct jpg_engine_data_t *p_eng_data;

	if (chip_idx >= JPEG_MAX_CHIP || eng_idx >= JPEG_MAX_ENG) {
		DBG_ERR("%s, chip (%d/%d) > max chip id (%d/%d)\n", __func__, chip_idx, eng_idx, JPEG_MAX_CHIP, JPEG_MAX_ENG);
		return;
	}

	DBG_FUNC("Jpeg {chip:%d,eng:%d}, %s \n", chip_idx, eng_idx, __func__);

	/* workaround. Process frameEnd miss case */
	if (timeout_check_framend) {
		UINT32 int_status;

		p_eng_data = &jpeg_eng_data[eng_data_idx];
		if (jpeg_check_frame_end_isr(p_eng_data->engine_base_va) == 1) {
			DBG_FUNC("Jpeg {chip:%d,eng:%d}miss frameEnd isr, treat it as normal(type:%d). BEGIN. \n", chip_idx, eng_idx, p_eng_data->active_type);
			timeout_check_framend_ok ++; /* counter */
			int_status = jpeg_isr(p_eng_data->engine_idx);
			jpeg_drv_bottom_half(p_eng_data->chip_idx, p_eng_data->engine_idx, int_status);
			DBG_FUNC("Jpeg {chip:%d,eng:%d}miss frameEnd isr END \n", chip_idx, eng_idx);
			return;
		}
	}

	jpeg_drv_bottom_half(chip_idx, eng_idx, JPEG_INT_TIMEOUT);
	return;
}
#endif //JPEG_SW_TIMEOUT
#endif //__KERNEL__

////////// sw decoder //////////
#if JPEG_SW_DECODE
#ifdef __KERNEL__
#if SUP_NON_BLK_MODE
static int jpg_sw_decode_done(struct job_item_t *job_item, int error_type)
{
	if (error_type < 0) {
		jpeg_sync_job(job_item, -1);
	}
	else {
		jpeg_sync_job(job_item, 0);
	}
	if (job_item->chip >= JPEG_MAX_CHIP || job_item->engine >= JPEG_MAX_ENG) {
		DBG_ERR("%s, chip (%d/%d) > max chip id (%d/%d)\n", __func__, job_item->chip, job_item->engine, JPEG_MAX_CHIP, JPEG_MAX_ENG);
	}
	jpeg_set_engine_idle(job_item->chip, job_item->engine);
	jpeg_start_schedule(job_item->chip, JPEG_ALL_ENGINE/*pmodule_info->engine_idx*/);
	return 0;
}
#endif
#endif
int jpg_sw_decoder(struct job_item_t *job_item)
{
	KDRV_VDOJPGD_PARAM *dec_param = &job_item->dec_param;
	JpgSWDec sw_param;

	if (NULL == jpeg_swdec_func) {
		DBG_ERR("can not support sw decode !! \r\n");
		sw_param.error_type = -1;
		goto exit_sw_dec;
	}
	DBG_IND("job %u sw decode start\n", job_item->job_id);

	sw_param.uv_swap = 0;	// default uv
	sw_param.jpeg_hdr_addr = (unsigned char *)dec_param->jpeg_hdr_addr;
	sw_param.bs_size = dec_param->bs_buf_size[0];
	sw_param.yuv_addr_va = (unsigned char *)dec_param->y_addr_va;
	sw_param.raw_size = dec_param->raw_size;
	// printk("bs_size(%d), raw_size(%d), yuv_addr_va(%lx) \n", sw_param.bs_size, sw_param.raw_size, (unsigned long)sw_param.yuv_addr_va);
	jpeg_swdec_func->jpg_sw_dec(&sw_param);

	job_item->dec_param.uiWidth = sw_param.img_width;
	job_item->dec_param.uiHeight = sw_param.img_height;
	//job_item->bg_width = sw_param.bg_width;
	//job_item->bg_height = sw_param.bg_height;
	DBG_IND("sw decode ret = %d, job resolution = %d x %d\n", sw_param.error_type, job_item->img_width, job_item->img_height);

exit_sw_dec:
#ifdef __KERNEL__
	#if SUP_NON_BLK_MODE
	if (jpeg_eng_data[JPEG_ENTITY_IDX(0, job_item->engine)].fire_job != NULL) {
		DBG_IND("sw decode fire_job != NULL \n");
		jpg_sw_decode_done(job_item, sw_param.error_type);
	}
	#endif
#endif
	if (0 == sw_param.error_type)
		vos_cpu_dcache_sync(dec_param->y_addr_va, dec_param->raw_size, VOS_DMA_BIDIRECTIONAL);
	return sw_param.error_type;
}
#endif


////////////// check new featute //////////////
static int jpeg_update_new_feature(struct jpeg_enc_data_t *p_enc_data)
{
	int layer, id, roi_idx;
	int new_feature = 0;
	
	for (layer = 0; layer < MAX_JPEG_OSG_LAYER; layer++) {
		for (id = 0; id < MAX_JPEG_OSG_NUM; id++) {
			if (p_enc_data->enc_cfg.stOsg.stOsgWin[layer][id].bEnable) {
				new_feature = 1;
				goto exit_update;
			}
		}
	}
	if (p_enc_data->enc_cfg.stMask.bEnable) {
		new_feature = 1;
		goto exit_update;
	}
	if (p_enc_data->enc_cfg.stGray.enable) {
		new_feature = 1;
		goto exit_update;
	}
	if (p_enc_data->enc_cfg.uiRotation != JPG_ROTATE_DISABLE) {
		new_feature = 1;
		goto exit_update;
	}
	for (roi_idx = 0; roi_idx < MAX_JPEG_ROI_NUM; roi_idx++) {
		if (p_enc_data->enc_cfg.stRoi.stRoiParam->roi_enable) {
			new_feature = 1;
			goto exit_update;
		}
	}

exit_update:
	p_enc_data->new_feature = new_feature;
	return 0;
}

////////// get RC parameter ////////
int jpeg_get_rc_info(unsigned int chip, unsigned int chn, JPEG_RC_INFO *p_rc_param)
{
	struct jpeg_enc_data_t *p_enc_data;
	JPEG_RC_PARAM *rc_param;

	if (chn >= jpeg_enc_max_chn) {
		goto exit_get_rc;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);

	rc_param = &p_enc_data->enc_cfg.stRC;

	if (rc_param->enable && rc_param->target_rate) {
		if (rc_param->vbr_mode) {
			p_rc_param->mode = JPG_RC_VBR;
			if (rc_param->vbrState == VBR_NORMAL)
				p_rc_param->quality = rc_param->vbrQuality;
			else
				p_rc_param->quality = rc_param->brcStdQTableQuality;
			p_rc_param->priority = rc_param->vbr_priority;
			p_rc_param->cur_fps = rc_param->cur_framerate;
			p_rc_param->min_fps = rc_param->min_framerate;
		}
		else {
			p_rc_param->mode = JPG_RC_CBR;
			p_rc_param->quality = rc_param->brcStdQTableQuality;
		}
		p_rc_param->bitrate = rc_param->target_rate*8;
		p_rc_param->frame_rate_base = rc_param->frame_rate_base;
		p_rc_param->frame_rate_incr = rc_param->frame_rate_incr;
		p_rc_param->init_quality = rc_param->quality;
		p_rc_param->max_quality = rc_param->max_quality;
		p_rc_param->min_quality = rc_param->min_quality;
	}
	else if (rc_param->quality) {
		p_rc_param->mode = JPG_RC_FIX;
		p_rc_param->quality = rc_param->quality;
	}
	else
		p_rc_param->mode = JPG_RC_NONE;

exit_get_rc:
	return p_rc_param->mode;
}
int jpeg_get_rc_info_by_param(JPEG_RC_PARAM *p_rc_param, JPEG_RC_INFO *p_rc_info)
{
	if (p_rc_param->enable && p_rc_param->target_rate) {
		if (p_rc_param->vbr_mode) {
			p_rc_info->mode = JPG_RC_VBR;
			if (p_rc_param->vbrState == VBR_NORMAL)
				p_rc_info->quality = p_rc_param->vbrQuality;
			else
				p_rc_info->quality = p_rc_param->brcStdQTableQuality;
		}
		else {
			p_rc_info->mode = JPG_RC_CBR;
			p_rc_info->quality = p_rc_param->brcStdQTableQuality;
		}
		p_rc_info->bitrate = p_rc_param->target_rate*8;
		p_rc_info->frame_rate_base = p_rc_param->frame_rate_base;
		p_rc_info->frame_rate_incr = p_rc_param->frame_rate_incr;
		p_rc_info->init_quality = p_rc_param->quality;
		p_rc_info->max_quality = p_rc_param->max_quality;
		p_rc_info->min_quality = p_rc_param->min_quality;
	}
	else if (p_rc_param->quality) {
		p_rc_info->mode = JPG_RC_FIX;
		p_rc_info->quality = p_rc_param->quality;
	}
	else {
		p_rc_info->mode = JPG_RC_NONE;
	}

	return p_rc_info->mode;
}

////////// encode set rc //////////
static int get_gcd(int a, int b)
{
	if (0 == a || 0 == b)
		return 0;
	while (a > 0 && b > 0) {
		if (a > b)
			a = a % b;
		else
			b = b % a;
	}
	if (0 == a)
		return b;
	if (0 == b)
		return a;
	return 0;
}
int jpeg_enc_set_rc(UINT32 chn_id, KDRV_VDOJPGE_RC *p_rc_param)
{
	struct jpeg_enc_data_t *p_enc_data;
	JPEG_RC_PARAM *p_rc_cfg;
	int fps_gcd;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("Error to use enc chn %d, max is %d\n", chn_id, jpeg_enc_max_chn);
		goto fail_set_rc;
	}

	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);
	p_rc_cfg = &p_enc_data->enc_cfg.stRC;
	/* store to per channel structure */
	p_rc_cfg->enable = p_rc_param->enable;
	p_rc_cfg->base_qp = JPEG_CLIP3(1, 100, p_rc_param->base_qp);
	p_rc_cfg->quality = p_rc_cfg->base_qp;
	p_rc_cfg->vbr_mode = p_rc_param->vbr_mode;
	if (p_rc_param->max_quality < p_rc_param->min_quality) {
		DBG_WRN("%s: max quality(%d) less than min quality(%d), force 1~100\r\n", __FUNCTION__, p_rc_param->max_quality, p_rc_param->min_quality);
		p_rc_param->min_quality = 1;
		p_rc_param->max_quality = 100;
	}
	p_rc_cfg->min_quality = JPEG_CLIP3(1, 100, p_rc_param->min_quality);
	p_rc_cfg->max_quality = JPEG_CLIP3(1, 100, p_rc_param->max_quality);
	p_rc_cfg->target_rate = p_rc_param->target_rate / 8;
	fps_gcd = get_gcd(p_rc_param->frame_rate_base, p_rc_param->frame_rate_incr);
	if (fps_gcd > 0) {
		p_rc_cfg->frame_rate_base = p_rc_param->frame_rate_base / fps_gcd;
		p_rc_cfg->frame_rate_incr = p_rc_param->frame_rate_incr / fps_gcd;
	}

	if (p_rc_cfg->vbr_mode) {
		p_rc_cfg->vbr_priority = (UINT32)p_rc_param->vbr_priority;
		if (p_rc_param->min_frame_rate < 1)
			p_rc_cfg->min_framerate = 1;
		else
			p_rc_cfg->min_framerate = p_rc_param->min_frame_rate;
	}
	else {
		p_rc_cfg->vbr_priority = KDRV_JPGE_VBR_PRI_NONE;
		p_rc_cfg->min_framerate = 1;
	}

	p_rc_cfg->update = 1;
	DBG_FUNC("{chn%d} set rc: enable = %u, vbr = %u, base qp = %u (%u ~ %u), bitrate = %u, frame rate = %u/%u priority = %d (min.fps=%u)\r\n",
				chn_id, p_rc_cfg->enable, p_rc_cfg->vbr_mode, p_rc_cfg->base_qp, p_rc_cfg->min_quality, p_rc_cfg->max_quality,
				p_rc_cfg->target_rate, p_rc_cfg->frame_rate_base, p_rc_cfg->frame_rate_incr,  (int)p_rc_cfg->vbr_priority, p_rc_cfg->min_framerate);

	return 0;
fail_set_rc:
	return -1;
}

int jpeg_enc_get_rc(UINT32 chn_id, KDRV_VDOJPGE_RC *p_rc_param)
{
	struct jpeg_enc_data_t *p_enc_data;
	JPEG_RC_PARAM *p_rc_cfg;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("Error to use enc chn %d, max is %d\n", chn_id, jpeg_enc_max_chn);
		goto fail_get_rc;
	}

	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);
	p_rc_cfg = &p_enc_data->enc_cfg.stRC;

	p_rc_param->enable = p_rc_cfg->enable;
	p_rc_param->base_qp = p_rc_cfg->base_qp;
	p_rc_param->vbr_mode = p_rc_cfg->vbr_mode;
	p_rc_param->min_quality = p_rc_cfg->min_quality;
	p_rc_param->max_quality = p_rc_cfg->max_quality;
	p_rc_param->target_rate = p_rc_cfg->target_rate * 8;
	p_rc_param->frame_rate_base = p_rc_cfg->frame_rate_base;
	p_rc_param->frame_rate_incr = p_rc_cfg->frame_rate_incr;

	return 0;
fail_get_rc:
	return -1;
}


////////// kdrvier set parameter: OSG //////////
int jpeg_set_osg_win_cfg(UINT32 chn_id, UINT8 ucLayerIdx, UINT8 ucWinIdx, JPEG_OSG_WIN_CFG *pOsgWin)
{
	JPEG_OSG_WIN_CFG *p_osg_win;
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode osg win channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_osg;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	if (ucWinIdx >= MAX_JPEG_OSG_NUM) {
		DBG_ERR("JPEG Encode OSG win(%d) over max win id(%d)!\r\n", ucWinIdx, MAX_JPEG_OSG_NUM);
		ret = -1;
		goto exit_set_osg;
	}
	if (ucLayerIdx >= MAX_JPEG_OSG_LAYER) {
		DBG_ERR("JPEG Encode OSG layer(%d) over max layer id(%d)!\r\n", ucLayerIdx, MAX_JPEG_OSG_LAYER);
		ret = -1;
		goto exit_set_osg;
	}
	p_osg_win = &p_enc_data->enc_cfg.stOsg.stOsgWin[ucLayerIdx][ucWinIdx];
	memcpy(p_osg_win, pOsgWin, sizeof(JPEG_OSG_WIN_CFG));
	// if ((p_osg_win->stGrap.uiAddr != 0) && (p_osg_win->stGrap.uiAddr_pa == 0)) {
	// 	p_osg_win->stGrap.uiAddr_pa = jpeg_va2pa(p_osg_win->stGrap.uiAddr);
	// 	DBG_WRN("jpeg_set_osg_win_cfg uiAddr(%lx), uiAddr_pa(%lx) \n", (unsigned long)p_osg_win->stGrap.uiAddr, (unsigned long)p_osg_win->stGrap.uiAddr_pa);
	// }
	DBG_MSG("%s[%d] set osg win%d-%d, enable %d, xy = %dx%d, wh = %dx%d (lofs %d)\r\n",
		__FUNCTION__, chn_id, ucLayerIdx, ucWinIdx,
		p_osg_win->bEnable, p_osg_win->stDisp.usXStr, p_osg_win->stDisp.usYStr,
		p_osg_win->stGrap.usWidth, p_osg_win->stGrap.usHeight, p_osg_win->stGrap.usLofs);
	jpeg_update_new_feature(p_enc_data);
exit_set_osg:
	return ret;
}

int jpeg_set_osg_rgb_cfg(UINT32 chn_id, JPEG_OSG_RGB_CFG *pOsgRgb)
{
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set osg rgb channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_osg;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[0][0] = pOsgRgb->ucRgb2Yuv[0][0];
	p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[0][1] = pOsgRgb->ucRgb2Yuv[0][1];
	p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[0][2] = pOsgRgb->ucRgb2Yuv[0][2];
	p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[1][0] = pOsgRgb->ucRgb2Yuv[1][0];
	p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[1][1] = pOsgRgb->ucRgb2Yuv[1][1];
	p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[1][2] = pOsgRgb->ucRgb2Yuv[1][2];
	p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[2][0] = pOsgRgb->ucRgb2Yuv[2][0];
	p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[2][1] = pOsgRgb->ucRgb2Yuv[2][1];
	p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[2][2] = pOsgRgb->ucRgb2Yuv[2][2];
exit_set_osg:
	return ret;
}

int jpeg_set_osg_pal_cfg(UINT32 chn_id, UINT8 ucPalIdx, JPEG_OSG_PAL_CFG *pOsgPal)
{
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set osg palette channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_osg;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	if (ucPalIdx >= MAX_JPEG_OSG_PAL_NUM) {
		DBG_ERR("JPEG Encode OSG palette(%d) over max palette id(%d)!\r\n", ucPalIdx, MAX_JPEG_OSG_PAL_NUM);
		ret = -1;
		goto exit_set_osg;
	}

	memcpy(&p_enc_data->enc_cfg.stOsg.stOsgPal[ucPalIdx], pOsgPal, sizeof(JPEG_OSG_PAL_CFG));
exit_set_osg:
	return ret;
}

int jpeg_set_osg_global_cfg(UINT32 chn_id, JPEG_OSG_GLOBAL_CFG *pOsgGlobal)
{
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set osg rgb channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_osg;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	p_enc_data->enc_cfg.stOsg.stOsgGlobal.overlap_type = pOsgGlobal->overlap_type;
	// memcpy(&p_enc_data->enc_cfg.stOsg.stOsgGlobal, pOsgGlobal, sizeof(JPEG_OSG_GLOBAL_CFG));
	jpeg_update_new_feature(p_enc_data);
exit_set_osg:
	return ret;
}

// =========== kdrvier get parameter: OSG =========== //
int jpeg_get_osg_win_cfg(unsigned int kdrv_id, void *param)
{
	unsigned int chip, chn;
	struct jpeg_enc_data_t *p_enc_data;
	KDRV_VDOJPGE_OSG_WIN *osg_param = (KDRV_VDOJPGE_OSG_WIN *)param;
	JPEG_OSG_WIN_CFG *p_osg_win;

	// check chip active
	chip = KDRV_DEV_ID_CHIP(kdrv_id);
	chn = KDRV_DEV_ID_CHANNEL(kdrv_id);

	if (chip >= jpeg_max_chip) {
		DBG_ERR("Error to use enc chip %d, max is %d\n", chip, jpeg_max_chip);
		goto fail_get_osg;
	}

	if (chn >= jpeg_enc_max_chn) {
		DBG_ERR("Error to use enc chn %d, max is %d\n", chn, jpeg_enc_max_chn);
		goto fail_get_osg;
	}

	if (osg_param->win_idx >= MAX_JPEG_OSG_NUM) {
		DBG_ERR("Error to use osg win id %d, max is %d\n", osg_param->win_idx, MAX_JPEG_OSG_NUM);
		goto fail_get_osg;
	}

	p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);
	p_osg_win = &p_enc_data->enc_cfg.stOsg.stOsgWin[osg_param->layer_idx][osg_param->win_idx];

	osg_param->enable = p_osg_win->bEnable;
	if (nvt_get_chip_id() == CHIP_NS02402)
		osg_param->roi_invalid = p_osg_win->bRoiInvalid;
	osg_param->st_grap.type = p_osg_win->stGrap.ucType;
	osg_param->st_grap.width = p_osg_win->stGrap.usWidth;
	osg_param->st_grap.height = p_osg_win->stGrap.usHeight;
	osg_param->st_grap.line_offset = p_osg_win->stGrap.usLofs;
	osg_param->st_grap.addr_pa = p_osg_win->stGrap.uiAddr_pa;

	osg_param->st_disp.mode = p_osg_win->stDisp.ucMode;
	osg_param->st_disp.x_str = p_osg_win->stDisp.usXStr;
	osg_param->st_disp.y_str = p_osg_win->stDisp.usYStr;
	osg_param->st_disp.bg_alpha = p_osg_win->stDisp.ucBgAlpha;
	osg_param->st_disp.fg_alpha = p_osg_win->stDisp.ucFgAlpha;
	osg_param->st_disp.mask_type = p_osg_win->stDisp.ucMaskType;
	osg_param->st_disp.mask_bd_size = p_osg_win->stDisp.ucMaskBdSize;
	osg_param->st_disp.mask_y[0] = p_osg_win->stDisp.ucMaskY[0];
	osg_param->st_disp.mask_y[1] = p_osg_win->stDisp.ucMaskY[1];
	osg_param->st_disp.mask_cb = p_osg_win->stDisp.ucMaskCb;
	osg_param->st_disp.mask_cr = p_osg_win->stDisp.ucMaskCr;

	osg_param->st_key.enable = p_osg_win->stKey.bEnable;
	osg_param->st_key.alpha_en = p_osg_win->stKey.bAlphaEn;
	osg_param->st_key.alpha = p_osg_win->stKey.ucAlpha;
	osg_param->st_key.red = p_osg_win->stKey.ucRed;
	osg_param->st_key.green = p_osg_win->stKey.ucGreen;
	osg_param->st_key.blue = p_osg_win->stKey.ucBlue;

	return 0;
fail_get_osg:
	return -1;
}

int jpeg_get_osg_rgb_cfg(unsigned int kdrv_id, void *param)
{
	unsigned int chip, chn;
	struct jpeg_enc_data_t *p_enc_data;
	KDRV_VDOJPGE_OSG_RGB *osg_rgb = (KDRV_VDOJPGE_OSG_RGB *)param;

	// check chip active
	chip = KDRV_DEV_ID_CHIP(kdrv_id);
	chn = KDRV_DEV_ID_CHANNEL(kdrv_id);

	if (chip >= jpeg_max_chip) {
		DBG_ERR("Error to use enc chip %d, max is %d\n", chip, jpeg_max_chip);
		goto fail_get_osg;
	}

	if (chn >= jpeg_enc_max_chn) {
		DBG_ERR("Error to use enc chn %d, max is %d\n", chn, jpeg_enc_max_chn);
		goto fail_get_osg;
	}

	p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);

	osg_rgb->rgb2yuv[0][0] = p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[0][0];
	osg_rgb->rgb2yuv[0][1] = p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[0][1];
	osg_rgb->rgb2yuv[0][2] = p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[0][2];
	osg_rgb->rgb2yuv[1][0] = p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[1][0];
	osg_rgb->rgb2yuv[1][1] = p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[1][1];
	osg_rgb->rgb2yuv[1][2] = p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[1][2];
	osg_rgb->rgb2yuv[2][0] = p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[2][0];
	osg_rgb->rgb2yuv[2][1] = p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[2][1];
	osg_rgb->rgb2yuv[2][2] = p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv[2][2];

	return 0;
fail_get_osg:
	return -1;
}

int jpeg_get_osg_pal_cfg(unsigned int kdrv_id, void *param)
{
	unsigned int chip, chn;
	struct jpeg_enc_data_t *p_enc_data;
	KDRV_VDOJPGE_OSG_PAL *osg_pal = (KDRV_VDOJPGE_OSG_PAL *)param;

	// check chip active
	chip = KDRV_DEV_ID_CHIP(kdrv_id);
	chn = KDRV_DEV_ID_CHANNEL(kdrv_id);

	if (chip >= jpeg_max_chip) {
		DBG_ERR("Error to use enc chip %d, max is %d\n", chip, jpeg_max_chip);
		goto fail_get_osg;
	}

	if (chn >= jpeg_enc_max_chn) {
		DBG_ERR("Error to use enc chn %d, max is %d\n", chn, jpeg_enc_max_chn);
		goto fail_get_osg;
	}

	if (osg_pal->pal_idx >= MAX_JPEG_OSG_PAL_NUM) {
		DBG_ERR("%s: Error to use palette id %d, max is %d\n", __FUNCTION__, (int)osg_pal->pal_idx, (int)MAX_JPEG_OSG_PAL_NUM);
		goto fail_get_osg;
	}

	p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);

	osg_pal->alpha = p_enc_data->enc_cfg.stOsg.stOsgPal[osg_pal->pal_idx].ucAlpha;
	osg_pal->red = p_enc_data->enc_cfg.stOsg.stOsgPal[osg_pal->pal_idx].ucRed;
	osg_pal->green = p_enc_data->enc_cfg.stOsg.stOsgPal[osg_pal->pal_idx].ucGreen;
	osg_pal->blue = p_enc_data->enc_cfg.stOsg.stOsgPal[osg_pal->pal_idx].ucBlue;

	return 0;
fail_get_osg:
	return -1;
}
// =========== kdrvier get parameter: OSG =========== //

////////// kdrvier set parameter: MASK //////////
int jpeg_set_mask_init_cfg(UINT32 chn_id, JPEG_MASK_INIT_CFG *pMaskInitCfg)
{
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set mask init cfg channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_mask;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	memcpy(&p_enc_data->enc_cfg.stMask.stInitCfg, pMaskInitCfg, sizeof(JPEG_MASK_INIT_CFG));
exit_set_mask:
	return ret;
}

int jpeg_set_mask_win_cfg(UINT32 chn_id, UINT8 ucMaskId, JPEG_MASK_WIN_CFG *pMaskWinCfg)
{
	struct jpeg_enc_data_t *p_enc_data;
	JPEG_MASK_WIN_CFG *pMaskWin;
	BOOL enable = FALSE;
	int i;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set mask win channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_mask;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	if (ucMaskId >= MAX_JPEG_MASK_NUM) {
		DBG_ERR("JPEG Encode MASK ID(%d) over max id(%d)!\r\n", ucMaskId, MAX_JPEG_MASK_NUM);
		ret = -1;
		goto exit_set_mask;
	}

	// set parameter
	pMaskWin = &p_enc_data->enc_cfg.stMask.stWinCfg[ucMaskId];
	memcpy(pMaskWin, pMaskWinCfg, sizeof(JPEG_MASK_WIN_CFG));
	DBG_MSG("set win[%d] (%d,%d), (%d,%d), (%d,%d), (%d,%d)\n", ucMaskId,
				pMaskWin->stPos[0].usPosX, pMaskWin->stPos[0].usPosY, pMaskWin->stPos[1].usPosX, pMaskWin->stPos[1].usPosY,
				pMaskWin->stPos[2].usPosX, pMaskWin->stPos[2].usPosY, pMaskWin->stPos[3].usPosX, pMaskWin->stPos[3].usPosY);
	for (i = 0; i < MAX_JPEG_MASK_NUM; i++) {
		if (pMaskWin->bEnable) {
			enable = TRUE;
			break;
		}
	}
	p_enc_data->enc_cfg.stMask.bEnable = enable;

	jpeg_set_mask_line_cfg(pMaskWin);

	jpeg_update_new_feature(p_enc_data);
	
exit_set_mask:
	return ret;
}

////////// kdrvier set parameter: rotation //////////
int jpeg_set_rotate_cfg(UINT32 chn_id, UINT32 uiRotate)
{
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set rotation channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_rot;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	if (JPG_ROTATE_DISABLE != uiRotate && JPG_ROTATE_CCW != uiRotate && JPG_ROTATE_CW != uiRotate && JPG_ROTATE_180 != uiRotate) {
		DBG_WRN("jpeg rotation type out of range %u\n", uiRotate);
		uiRotate = JPG_ROTATE_DISABLE;
	}
	p_enc_data->enc_cfg.uiRotation = uiRotate;
	jpeg_update_new_feature(p_enc_data);
exit_set_rot:
	return ret;
}

////////// kdrvier set parameter: gray //////////
int jpeg_set_gray_cfg(UINT32 chn_id, JPEG_GRAY_CFG *pGray)
{
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set gray channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_gray;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	memcpy(&p_enc_data->enc_cfg.stGray, pGray, sizeof(JPEG_GRAY_CFG));
	DBG_IND("%s[%d]: set gray en %d, gray %d, src.gray %d\n", __FUNCTION__, chn_id, pGray->enable, pGray->color_to_gray, pGray->src_color_to_gray);
	jpeg_update_new_feature(p_enc_data);
exit_set_gray:
	return ret;
}

////////// kdrvier set parameter: user data //////////
int jpeg_set_user_data_cfg(UINT32 chn_id, JPEG_USER_DATA_CFG *pData)
{
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set user data channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_data;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	memcpy(&p_enc_data->enc_cfg.stUserData, pData, sizeof(JPEG_USER_DATA_CFG));
	DBG_IND("%s[%d]: set user data en %d, add 0x%lx, len %d\n", __FUNCTION__, chn_id, pData->enable, (unsigned long)pData->data_addr, pData->data_length);
exit_set_data:
	return ret;
}

////////// kdrvier set parameter: roi //////////
int jpeg_set_roi_cfg(UINT32 chn_id, UINT8 roi_idx, JPEG_ROI_PARAM *p_roi_param)
{
	struct jpeg_enc_data_t *p_enc_data;
	JPEG_ROI_PARAM *tmp_roi_param;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set roi channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_roi;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	if (roi_idx >= MAX_JPEG_ROI_NUM) {
		DBG_ERR("JPEG Encode ROI index(%d) over max define(%d) !! \n", roi_idx, MAX_JPEG_ROI_NUM);
		ret = -1;
		goto exit_set_roi;
	}

	// set parameter
	tmp_roi_param = &p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx];
	memcpy(tmp_roi_param, p_roi_param, sizeof(JPEG_ROI_PARAM));
	DBG_MSG("2 set roi_idx=%d, x=%d, y=%d, w=%d, h=%d, en=%d, th=%d \n", 
				roi_idx, tmp_roi_param->roi_pos_x, tmp_roi_param->roi_pos_y, tmp_roi_param->roi_width, tmp_roi_param->roi_height, tmp_roi_param->roi_enable, tmp_roi_param->roi_threshold);

	jpeg_update_new_feature(p_enc_data);

exit_set_roi:
	return ret;
}

////////// kdriver set parameter: pad //////////
int jpeg_set_pad_cfg(UINT32 chn_id, JPEG_PAD_CFG *p_pad)
{
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;

	if (chn_id >= jpeg_enc_max_chn) {
		DBG_ERR("jpeg encode set roi channel(%d) out of range (%d)\n", chn_id, jpeg_enc_max_chn);
		ret = -1;
		goto exit_set_pad;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(0, chn_id);

	memcpy(&p_enc_data->enc_cfg.stPad, p_pad, sizeof(JPEG_PAD_CFG));

exit_set_pad:
	return ret;
}

////////// kdrvier set parameter: dec_y_out //////////
int jpeg_set_dec_y_out_cfg(UINT32 chn_id, BOOL bDecYOutOnly)
{
    struct jpeg_dec_data_t *p_dec_data;
    int ret = 0;

    if (chn_id >= jpeg_dec_max_chn) {
    	DBG_ERR("jpeg decode set y out only channel(%d) out of range (%d)\n", chn_id, jpeg_dec_max_chn);
    	ret = -1;
        goto exit_set_y_out;
    }
    p_dec_data = jpeg_dec_data + JPEGENC_IDX(0, chn_id);
	//printk("2 set y_out_cfg chn[%d]=[%d] \n",chn_id,bDecYOutOnly);
    p_dec_data->dec_cfg.bDecYOutOnly = bDecYOutOnly;

exit_set_y_out:
       return ret;
}
////////// get parameter: roi //////////
int jpeg_get_roi_cfg(unsigned int chip, unsigned int chn)
{
	struct jpeg_enc_data_t *p_enc_data;
	int ret = 0;
	UINT8 roi_idx, roi_enable_or = 0;

	if (chn >= jpeg_enc_max_chn) {
		ret = -1;
		goto exit_get_roi;
	}
	p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);

	// get parameter
	for (roi_idx = 0; roi_idx < MAX_JPEG_ROI_NUM; roi_idx++)
		roi_enable_or |= p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx].roi_enable;
		
	if (roi_enable_or != 0) {
		for (roi_idx = 0; roi_idx < MAX_JPEG_ROI_NUM; roi_idx++) {
			DBG_DUMP("chn=%02d, idx=%02d, en=%d, th=(%02d %02d %d), x=%d, y=%d, w=%d, h=%d \n", chn, roi_idx,
						p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx].roi_enable,
						p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx].roi_threshold,
						p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx].roi_threshold_ac1_ac2,
						p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx].roi_sub_ac1_en,
						p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx].roi_pos_x << 4,
						p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx].roi_pos_y << 4,
						p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx].roi_width << 4,
						p_enc_data->enc_cfg.stRoi.stRoiParam[roi_idx].roi_height << 4);
		}
	}

exit_get_roi:
	return ret;
}

//// register sw decoder ////
int jpeg_register_func(JPEG_SW_DEC_FUNC *func)
{
	jpeg_swdec_func = func;
	return 0;
}

void jpeg_deregister_func(void)
{
	jpeg_swdec_func = NULL;
}

#if SUP_NON_BLK_MODE
////////// malloc ///////////
static void *jpeg_kmalloc(unsigned int size, char *buf_name)
{
	if (buf_name) {}

	return jpeg_platform_malloc(size);
}

static void jpeg_kfree(void *ptr)
{
	if (ptr)
		jpeg_platform_free(ptr);
}
#endif

void jpeg_kdrv_close(void)
{
	// jpeg_engine_exit();
#if SUP_NON_BLK_MODE
	jpeg_kfree(jpeg_enc_data);
	jpeg_enc_data = NULL;
	jpeg_kfree(jpeg_dec_data);
	jpeg_dec_data = NULL;
	jpeg_kfree(jpeg_job_head);
	jpeg_job_head = NULL;
	jpeg_kfree(jpeg_enc_minor_head);
	jpeg_enc_minor_head = NULL;
	jpeg_kfree(jpeg_dec_minor_head);
	jpeg_dec_minor_head = NULL;
	jpeg_kfree(job_item_pool.job_item_array);
	job_item_pool.job_item_array = NULL;
#endif
}

int jpeg_kdrv_init(void)
{
#if !SUP_NON_BLK_MODE
	//both block/non-block mode use
	vk_spin_lock_init(&jpeg_id_lock);
	return 0;
#else
	unsigned int chip, chn;
	int i, eng_data_idx;
	struct jpeg_enc_data_t *p_enc_data;
	struct jpeg_dec_data_t *p_dec_data;
	
	// jpeg_engine_init();

	// encode data
	jpeg_enc_data = (struct jpeg_enc_data_t *)jpeg_kmalloc(sizeof(struct jpeg_enc_data_t) * jpeg_max_chip * jpeg_enc_max_chn, "enc data");
	if (NULL == jpeg_enc_data) {
		DBG_ERR("Fail to allocate jpeg_enc_data!\n");
		goto init_fail;
	}
	for (chip = 0; chip < jpeg_max_chip; chip ++) {
		for (chn = 0; chn < jpeg_enc_max_chn; chn ++) {
			p_enc_data = jpeg_enc_data + JPEGENC_IDX(chip, chn);

			memset(p_enc_data, 0, sizeof(struct jpeg_enc_data_t));
			p_enc_data->chip = chip;
			p_enc_data->chn = chn;
			p_enc_data->active = 0;
			p_enc_data->new_feature = 0;
			memcpy(p_enc_data->enc_cfg.stOsg.stOsgRgb.ucRgb2Yuv, &JPEG_ENC_OSG_RGB, sizeof(JPEG_OSG_RGB_CFG));
			memcpy(p_enc_data->enc_cfg.stOsg.stOsgPal, JPEG_ENC_OSG_PAL, sizeof(p_enc_data->enc_cfg.stOsg.stOsgPal));
			memcpy(p_enc_data->enc_cfg.stMask.stInitCfg.ucPalY, MASK_PALETTE[0], sizeof(UINT8)*MAX_JPEG_MASK_PAL_NUM);
			memcpy(p_enc_data->enc_cfg.stMask.stInitCfg.ucPalCb, MASK_PALETTE[1], sizeof(UINT8)*MAX_JPEG_MASK_PAL_NUM);
			memcpy(p_enc_data->enc_cfg.stMask.stInitCfg.ucPalCr, MASK_PALETTE[2], sizeof(UINT8)*MAX_JPEG_MASK_PAL_NUM);
		}
	}

	// decode data
	jpeg_dec_data = (struct jpeg_dec_data_t *)jpeg_kmalloc(sizeof(struct jpeg_dec_data_t) * jpeg_max_chip * jpeg_dec_max_chn, "dec data");
	if (NULL == jpeg_dec_data) {
		DBG_ERR("Fail to allocate jpeg_dec_data!\n");
		goto init_fail;
	}
	for (chip = 0; chip < jpeg_max_chip; chip++) {
		for (chn = 0; chn < jpeg_dec_max_chn; chn++) {
			p_dec_data = jpeg_dec_data + JPEGDEC_IDX(chip, chn);

			memset(p_dec_data, 0, sizeof(struct jpeg_dec_data_t));
			p_dec_data->chip = chip;
			p_dec_data->chn = chn;
			p_dec_data->active = 0;
		}
	}

	//both block/non-block mode use
	vk_spin_lock_init(&jpeg_id_lock);

	// tasklet
	for (chip = 0; chip < jpeg_max_chip; chip ++) {
		for (eng_data_idx = 0; eng_data_idx < JPEG_MAX_ENG; eng_data_idx++) {
			tasklet_init(&jpeg_job_tasklet[chip][eng_data_idx], jpeg_start_handler, (unsigned long)JPEG_ENTITY_IDX(chip, eng_data_idx));
			DBG_MSG("start job tasklet[%d-%d] init done (0x%lx)\n", chip, eng_data_idx, (unsigned long)&jpeg_job_tasklet[chip][eng_data_idx]);
		}
	}
	tasklet_init(&jpeg_cb_tasklet, jpeg_callback_scheduler, 0);
	
	vk_spin_lock_init(&jpeg_jobitem_lock);
	vk_spin_lock_init(&jpeg_job_lock);
	
	// job list
	jpeg_job_head = (struct vos_list_head *)jpeg_kmalloc(sizeof(struct vos_list_head) * jpeg_max_chip, "engine job head");
	if (NULL == jpeg_job_head) {
		DBG_ERR("Fail to allocate job_list!\n");
		goto init_fail;
	}
	/* job linked to engine list */
	for (eng_data_idx = 0; eng_data_idx < jpeg_max_chip; eng_data_idx ++)
		VOS_INIT_LIST_HEAD(&jpeg_job_head[eng_data_idx]);

	/* The channels is per chip view instead of engine. Thus we just init for chip + chan */
	jpeg_enc_minor_head = (struct vos_list_head *)jpeg_kmalloc(sizeof(struct vos_list_head) * jpeg_enc_max_chn * jpeg_max_chip, "enc minor head");
	if (NULL == jpeg_enc_minor_head) {
		DBG_ERR("Fail to allocate enc minor_list!\n");
		goto init_fail;
	}
	for (chn = 0; chn < jpeg_enc_max_chn * jpeg_max_chip; chn++)
		VOS_INIT_LIST_HEAD(&jpeg_enc_minor_head[chn]);

	/* The channels is per chip view instead of engine. Thus we just init for chip + chan */
	jpeg_dec_minor_head = (struct vos_list_head *)jpeg_kmalloc(sizeof(struct vos_list_head) * jpeg_dec_max_chn * jpeg_max_chip, "dec minor head");
	if (NULL == jpeg_dec_minor_head) {
		DBG_ERR("Fail to allocate dec minor_list!\n");
		goto init_fail;
	}
	for (chn = 0; chn < jpeg_dec_max_chn * jpeg_max_chip; chn++)
		VOS_INIT_LIST_HEAD(&jpeg_dec_minor_head[chn]);
	
	// job item
	job_item_pool.cur_idx = 0;
	job_item_pool.total_num = (jpeg_enc_max_chn + jpeg_dec_max_chn) * 8;
	job_item_pool.job_item_array = (struct job_item_t *)jpeg_kmalloc(sizeof(struct job_item_t) * job_item_pool.total_num, "job item");
	if (NULL == job_item_pool.job_item_array) {
		DBG_ERR("Fail to allocate job item!\n");
		goto init_fail;
	}
	for (i = 0; i < job_item_pool.total_num; i++)
		memset(&job_item_pool.job_item_array[i], 0, sizeof(struct job_item_t));

	return 0;
init_fail:
	jpeg_kdrv_close();
	return -1;

#endif //SUP_NON_BLK_MODE
}

#ifdef __KERNEL__
#if SUP_NON_BLK_MODE
EXPORT_SYMBOL(jpeg_putjob);
EXPORT_SYMBOL(jpeg_stopjob);
EXPORT_SYMBOL(jpeg_enc_set_cb);
EXPORT_SYMBOL(jpeg_enc_get_cb);
EXPORT_SYMBOL(jpeg_dec_set_cb);
EXPORT_SYMBOL(jpeg_dec_get_cb);
#endif
EXPORT_SYMBOL(jpeg_register_func);
EXPORT_SYMBOL(jpeg_deregister_func);
EXPORT_SYMBOL(jpeg_enc_get_unused_channel);
EXPORT_SYMBOL(jpeg_dec_get_unused_channel);
EXPORT_SYMBOL(jpeg_enc_release_channel);
EXPORT_SYMBOL(jpeg_dec_release_channel);
EXPORT_SYMBOL(jpeg_kdrv_init);
EXPORT_SYMBOL(jpeg_kdrv_close);
EXPORT_SYMBOL(jpeg_enc_max_chn);
EXPORT_SYMBOL(jpeg_dec_max_chn);
#endif
