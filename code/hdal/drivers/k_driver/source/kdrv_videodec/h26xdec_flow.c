#include "platform_port.h"
#include "h265dec_comm_def.h"
#include "h26xdec_flow.h"
#include "h26xdec_dbg.h"
#include "h26xdec_proc.h"
#include "vpu/hevc_dec_entity.h"
#include "vpu/avc_dec_entity.h"
#include <kdrv_videodec.h>
#include "imvq.h"
#include "debug.h"

#include "kwrap/util.h"
#include <nvt_api_ver.h>

//#define STATIC static
#define STATIC  /* to make functions non-static, so that its name will occur on stack trace when kernel oops */

int h26xd_max_chip = 1; /* default value, don't change. It increases when new chip is probed. */
int max_total_cam_ch = 8;
unsigned int h265_max_support_vps = 2;
unsigned int h265_max_support_sps = 2;
unsigned int h265_max_support_pps = 2;
unsigned int h265_max_support_st = 13;
unsigned int h264_max_support_sps = 1;
unsigned int h264_max_support_pps = 2;
int h26xd_support_ep_bs = 0;
unsigned int h26xd_max_width = 8192;
unsigned int h26xd_max_height = 8192;
unsigned int h26xd_max_buf_num = 8;
unsigned int g_state_machine_err = 0;
unsigned int g_internal_err = 0;
unsigned int g_dec_err = 0;
unsigned int h26xd_hw_gate = 1;	/* hardware auto gating. 0 means software always release clock */
unsigned int h26xd_fail_damnit = 0; /* for debugging job fail */

int h26xd_dbglevel = NVT_DBG_WRN;
int h26xd_dbgpos = POS_ALL;
int h26xd_dbg_mode = 0; /*  0: normal mode, > 0: debug mode */

unsigned int hw_timeout_delay = 3750000; // 25ms
unsigned int real_pclk_rate = 150000000; // 150M

int address_check = 0;
int disable_mv_wo = 1;	//Must be disabled

/* speed up SW parsing */
int h264_only_support_single_slice_flag = 1;
int h265_only_support_single_slice_flag = 1;

unsigned int lose_pic_handle_flags = 1;

/* print bitstream message when error happen
*	0: disable, 1: printm, 2: printk
*   {print_type, fd, dump_byte, only_print_err}
*/
int n_dump_bs_msg = 4;
unsigned int dump_bs_msg[4] = {0, 0, 16, 0};

unsigned int h26xd_err_count_thd = 10;  /* threshold of message to print hw_timeout or dec_isr_err */

#define DEF_DUMP_FILE_PATH "/tmp"
char file_path_buf[64] = DEF_DUMP_FILE_PATH;	/* path for saving file */
char *dump_file_path;
int save_bs_cnt = 0; /* number of input bitstreams to be saved (in job number) */

/*  Module parameters */
module_param_string(dump_file_path, file_path_buf, 64, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dump_file_path, "path of dumping error bs");
module_param(max_total_cam_ch, int, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(max_total_cam_ch, "active channel number");
module_param(h265_max_support_vps, uint, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(h265_max_support_vps, "h265 max support VPS");
module_param(h265_max_support_sps, uint, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(h265_max_support_sps, "h265 max support SPS");
module_param(h265_max_support_pps, uint, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(h265_max_support_pps, "h265 max support PPS");
module_param(h265_max_support_st, uint, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(h265_max_support_st, "h265 max support short term ref set");
module_param(h264_max_support_sps, uint, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(h264_max_support_sps, "h264 max support SPS");
module_param(h264_max_support_pps, uint, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(h264_max_support_pps, "h264 max support PPS");
module_param(h26xd_max_width, uint, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(h26xd_max_width, "Max Width");
module_param(h26xd_max_height, uint, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(h26xd_max_height, "Max Height");
module_param(h26xd_max_buf_num, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(h26xd_max_buf_num, "Max Total Buffer Number of KDRV can be used to decoder");
module_param(h26xd_dbglevel, int, S_IRUGO | S_IWUSR); /* larger level, more message */
MODULE_PARM_DESC(h26xd_dbglevel, "h26xd log message level");
module_param(h26xd_dbgpos, int, S_IRUGO | S_IWUSR); /* print log message where position */
MODULE_PARM_DESC(h26xd_dbgpos, "log message where position");
module_param(h26xd_dbg_mode, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(h26xd_dbg_mode, "h26xd debug mode");
module_param(lose_pic_handle_flags, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(lose_pic_handle_flags, "lose picture handling flags: bit 0: whether to print error message. bit 1: whether to return error");
module_param(save_bs_cnt, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(save_bs_cnt, "count of dumping error bitstream");
module_param_array(dump_bs_msg, uint, &n_dump_bs_msg, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dump_bs_msg, "{print_type, fd, dump_byte, only_print_err}, print_type: 0: disable, 1: printm, 2: printk_ratelimited, 3: printk");
module_param(h26xd_err_count_thd, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(h26xd_err_count_thd, "threshold of message to print hw_timeout or dec_isr_err");
module_param(address_check, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(address_check, "use to check if the videograph pass kernel address for buffer purpose");
/* sw gating */
module_param(h26xd_hw_gate, uint, S_IRUGO); /* set at module loading time only or use proc to set it */
MODULE_PARM_DESC(h26xd_hw_gate, "1 for enable hw auto gating, 0 for always release.");
/* force stopping flow when job fail */
module_param(h26xd_fail_damnit, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(h26xd_fail_damnit, "1 for enable to force stopping flow when job fail.");

module_param(disable_mv_wo, uint, S_IRUGO); /* set at module loading time only */
MODULE_PARM_DESC(disable_mv_wo, "disable_mv_wo");

/* 264 only */
int h264d_print_addr = 0;
module_param(h264d_print_addr, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(h264d_print_addr, "print_addr");

unsigned int h264_gap_drop_counter = 0;
module_param(h264_gap_drop_counter, uint, S_IRUGO);
MODULE_PARM_DESC(h264_gap_drop_counter, "show 264 gap_drop_counter");

unsigned int h265_gap_drop_counter = 0;
module_param(h265_gap_drop_counter, uint, S_IRUGO);
MODULE_PARM_DESC(h265_gap_drop_counter, "show 265 gap_drop_counter");
/* new function of AD/BIN reset automatically */
int h264d_auto_sw_reset = 0;	//must be disabled in 331/530
module_param(h264d_auto_sw_reset, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(h264d_auto_sw_reset, "auto_sw_reset for AD/BIN");

int callback_wq = 0;
module_param(callback_wq, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(callback_wq, "callback use wq");

unsigned int h265_fifo_mode = 0;
module_param(h265_fifo_mode, uint, S_IRUGO); /* set at module loading time only or use proc to set it */
MODULE_PARM_DESC(h265_fifo_mode, "0: disable other: enable");

/*
*  global parameter
*/
static int engine_first_serve_ch[ENTITY_CHIPS][ENTITY_ENGINES];
struct buffer_info_t h26xd_vpu_work_buffer[ENTITY_CHIPS][ENTITY_ENGINES][ENTITY_MINORS];
struct buffer_info_t h26xd_link_list_buffer[ENTITY_CHIPS][ENTITY_ENGINES][2];
struct buffer_info_t h265d_cabac_buffer[ENTITY_CHIPS][2];
unsigned int *h264d_small_bs_buffer[ENTITY_CHIPS];
struct vos_list_head h264d_job_head[ENTITY_CHIPS];
struct vos_list_head h265d_job_head[ENTITY_CHIPS];
bool h265d_hdal_allocate_buffer_flag[ENTITY_CHIPS] = { 0 };

DecoderEngInfo h26xd_engine_info[ENTITY_CHIPS][ENTITY_ENGINES];
/* decode data */
struct h26xd_data_t *private_data[ENTITY_CHIPS] = {NULL};
h26xd_countr_t *counter[ENTITY_CHIPS] = {NULL};
h26xd_job_log_t *job_log[ENTITY_CHIPS] = {NULL};

/* bind channel */
struct bindch_info_t *bind_chn_idx[ENTITY_CHIPS] = {NULL};
DecLLJob	h26xd_link_list[ENTITY_CHIPS][ENTITY_ENGINES][2] = {0};// for link list, ping/pong buffer

static unsigned int max_job_array = 0;
static struct h26xd_job_item_t *job_array = NULL;
static int *job_used = NULL;
static int job_idx = 0;

unsigned int h26xd_vmalloc_cnt = 0;
unsigned int h26xd_vfree_cnt = 0;
struct h26xd_worker_s {
	struct work_struct	worker;
	unsigned int chip_idx;
} h26xd_worker[ENTITY_CHIPS];

KDRV_CALLBACK_FUNC *h26xd_cb_func = NULL;

/* tasklet */
struct tasklet_struct h26xd_cb_tasklet[ENTITY_CHIPS];
struct tasklet_struct h26xd_wk_tasklet[ENTITY_CHIPS];

/* utilization */
/* 5sec calculation (the duration of an engine utilization measurment) */
unsigned int utilization_period = 5;

/* version information */
char *plt_str = H26XD_PLT_STR;
char h26xd_ver_str[128] = { 0 };

/* reserve/free buffer count */
unsigned int resv_total_out_cnt = 0;
unsigned int free_total_out_cnt = 0;
unsigned int resv_cur_cnt[ENTITY_CHIPS][ENTITY_MINORS] = { { 0 } };

/* spinlock */
H26XD_DEFINE_SPINLOCK(h26x_dec_lock);      	// for decode flow lock
H26XD_DEFINE_SPINLOCK(h26x_dec_job_lock);		// job_item database lock/unlock. Notice: Exclude job_item status(use h26x_dec_lock).
H26XD_DEFINE_SPINLOCK(h26x_dec_bd_lock);		// for bind chn lock

/* tasklet */
int h26xd_exit_tasklet(int chip_idx);
int h26xd_init_tasklet(int chip_idx);
void h26xd_trigger_work(int chip_idx);
void h26xd_trigger_callback(int chip_idx);
void h26xd_work_process(int chip_idx);
int h26xd_stop(unsigned int fd);
int h26xd_get_buf_idx(struct h26xd_data_t *dec_data, struct h26xd_job_item_t *job_item);

/* job allocation / release for module */
struct h26xd_job_item_t *__h26xd_alloc_job_item(unsigned int size)
{
	struct h26xd_job_item_t *job_item = NULL;
	unsigned int i;
	unsigned long flags;

	h26xd_spin_lock_irqsave(&h26x_dec_job_lock, flags);
	for (i = 0; i < max_job_array; i++) {
		if (0 == job_used[job_idx]) {
			job_item = &job_array[job_idx];
			job_item->job_item_id = job_idx;
			job_used[job_idx] = 1;
			job_idx = (job_idx + 1) % max_job_array;
			break;
		}
		job_idx = (job_idx + 1) % max_job_array;
	}
	h26xd_spin_unlock_irqrestore(&h26x_dec_job_lock, flags);

	return job_item;
}

void __h26xd_free_job_item(void *ptr)
{
	struct h26xd_job_item_t *job_item = (struct h26xd_job_item_t *)ptr;
	h26xd_countr_t *counter_ptr;
	unsigned long flags;

	h26xd_spin_lock_irqsave(&h26x_dec_job_lock, flags);
	if (!job_used[job_item->job_item_id] || (job_item->job_item_id >= max_job_array)) {
		counter_ptr = counter[job_item->chip] + job_item->chn;

		counter_ptr->internal_err ++;
		g_internal_err ++;
		h26xd_err("job_item with job_item_id:%d, job_id:%u is invalid! \n", job_item->job_item_id, job_item->job_id);
	} else {
		job_used[job_item->job_item_id] = 0;
		//h26xd_info("{chip:%d,chn:%d} job_item with job_item_id:%d, job_id:%u is free! \n", job_item->chip, job_item->chn, job_item->job_item_id, job_item->job_id);
	}
	h26xd_spin_unlock_irqrestore(&h26x_dec_job_lock, flags);
}

struct h26xd_job_item_t *__h26xd_find_job_item(unsigned int idx)
{
	struct h26xd_job_item_t *job_item = NULL;
	unsigned long flags;

	h26xd_spin_lock_irqsave(&h26x_dec_job_lock, flags);
	if (job_used[idx]) {
		job_item = &job_array[idx];
	}
	h26xd_spin_unlock_irqrestore(&h26x_dec_job_lock, flags);

	return job_item;
}

/* log items are never freed
 */
STATIC job_log_item_t *__h26xd_alloc_job_log_item(int chip)
{
	h26xd_job_log_t	*log_item_chan;
	job_log_item_t	*log_item;
	unsigned int  head;
	unsigned long flags;

	if (chip >= h26xd_max_chip)
		return NULL;

	log_item_chan = job_log[chip];

	h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);

	head = log_item_chan->head & (MAX_JOB_LOG_ITEM - 1);
	log_item = &log_item_chan->log_item[head];
	memset(log_item, 0, sizeof(job_log_item_t));
	log_item_chan->head ++; /* move to next */
	if (((int)log_item_chan->head - (int)log_item_chan->tail) >= MAX_JOB_LOG_ITEM)
		log_item_chan->tail ++;

	h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);

	return log_item;
}

/*
* initial job's struct
*/
STATIC int __h26xd_job_init(struct h26xd_job_item_t *job_item)
{
	/* clear flags */
	job_item->callback_flg = 0;
	job_item->buf_idx = -1;
	job_item->res_flg = 0;
	job_item->status = 0;
	job_item->puttime = 0;
	job_item->starttime = 0;
	job_item->finishtime = 0;
	job_item->in_property_parsed_flg = 0;
	job_item->addr_chk_flg = 0;
	job_item->err_num = H26XD_OK;
	job_item->err_pos = ERR_NONE;

	job_item->poc = -1;
	job_item->yuv_range = 0;	/* uncertain */

    /* initial setting of extra write */
    job_item->extw_uv_swap = 1; /* fixed in swap. */
#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
	job_item->extw_uv_swap = 0;
#endif

    job_item->extw_sce_enabled = 1;
    job_item->sub_yuv_en = 0;
    job_item->sub_rec_addr = 0;

	/* initial list */
	VOS_INIT_LIST_HEAD(&job_item->standby_list);
	VOS_INIT_LIST_HEAD(&job_item->chan_list);

	VOS_INIT_LIST_HEAD(&job_item->job_list);

	return 0;
}

/* ====================== K_DRV ================================== */

/* Find an empty slot in bind_chn_idx[] and return the FD to kflow
 * Input: chip/codec_enigne
 * Return: unique FD number
 */
int h26xdec_open(unsigned int chip, unsigned int codec_enigne)
{
	struct h26xd_data_t *dec_data;
	struct bindch_info_t *bindch_info;
	h26xd_countr_t *counter_ptr;
	unsigned long bd_flags;
	int i, bind_chn_id = KDRV_ERR_NG;

	if ((int)chip >= h26xd_max_chip) {
		printk("%s, wrong chip_no:%d \n", __func__, chip);
		goto exit;
	}
	if ((KDRV_VIDEODEC_ENGINE_H264 != codec_enigne) && (KDRV_VIDEODEC_ENGINE_H265 != codec_enigne)) {
		printk("open channel fail, hw engine (0x%x) is not decoder!\n", codec_enigne);
		goto exit;
	}
	bindch_info = &bind_chn_idx[chip][0];

	/* find a new channel for this open. */
	h26xd_spin_lock_irqsave(&h26x_dec_bd_lock, bd_flags); /* lock */

	for (i = 0; i < max_total_cam_ch; i ++) {
		if (atomic_read(&bindch_info[i].is_used) == 1)
			continue;

		atomic_set(&bindch_info[i].is_used, 1);

		/* get an empty slot */
		dec_data = private_data[chip] + i;
		counter_ptr = counter[chip] + i;

		/* init record_err_count */
		counter_ptr->record_isr_err_cnt = 0;
		counter_ptr->record_hw_timeout_cnt = 0;

		/* init list head */
		VOS_INIT_LIST_HEAD(&dec_data->standby_list);
		VOS_INIT_LIST_HEAD(&dec_data->chan_list);

		/* init database */
		atomic_set(&bindch_info[i].is_closed, 0);
		atomic_set(&bindch_info[i].is_ready, 0);
		bind_chn_id = KDRV_DEV_ID(chip, codec_enigne, i);
		bindch_info[i].chn_id = bind_chn_id;

		h26xd_info("{chip:%d,ch:%d} bind_chn_idx is opened, bind_chn_id:0x%x. \n", chip, i, bind_chn_id);
		break;
	}

	h26xd_spin_unlock_irqrestore(&h26x_dec_bd_lock, bd_flags); /* unlock */

	if (bind_chn_id >= 0) {
		if (KDRV_VIDEODEC_ENGINE_H264 == codec_enigne) {
			H264Dec_Create(dec_data->dec_handle);
			H264Dec_SetVpu(dec_data->dec_handle);
			counter_ptr->codec_type = BIT1;
		} else {
			H265Dec_Create(dec_data->dec_handle);
			H265Dec_SetVpu(dec_data->dec_handle);
			H265Dec_SetConfigSramBuf(dec_data->dec_handle);
			counter_ptr->codec_type = BIT2;
		}
		atomic_set(&bindch_info[i].is_ready, 1);
	} else {
		h26xd_warn("open fail Bind chn is full (act_bind %d), please increase max_act_chn_num(or maybe some channels are closing).\n", max_total_cam_ch);
		for (i = 0; i < max_total_cam_ch; i++) {
			h26xd_warn("idx:%d, used:%d, closing:%d, chn_id:0x%x\n", i,
				   atomic_read(&bindch_info[i].is_used),
				   atomic_read(&bindch_info[i].is_closed),
				   bindch_info[i].chn_id);
		}
	}

exit:
	return bind_chn_id;
}

/* Close an existed FD number
 * Input: FD number created in h26xdec_open()
 * Return: 0 for success, < 0 for failure
 * Notice: This function is NOT in flow, thus it can be long latency.
 */
int h26xdec_close(unsigned int id)
{
	struct h26xd_data_t *dec_data;
	unsigned long flags;
	unsigned int chip, bd_chn;
	struct h26xd_job_item_t *job_item, *next;

	chip = KDRV_DEV_ID_CHIP(id);
	bd_chn = KDRV_DEV_ID_CHANNEL(id);
	if (((int)chip >= h26xd_max_chip) || ((int)bd_chn >= max_total_cam_ch)) {
		h26xd_err("pass error FD: 0x%x\n", id);
		return -1;
	}
	if (atomic_read(&bind_chn_idx[chip][bd_chn].is_used) == 0) {
		h26xd_err("FD:0x%x is not used!\n", id);
		return -1;
	}
	if (bind_chn_idx[chip][bd_chn].chn_id != id) {
		h26xd_err("pass error FD: 0x%x which is not exist!\n", id);
		return -1;
	}

	h26xd_info("{chip:%d,chan:%d} bind_chn_idx is closing. \n", chip, bd_chn);

	dec_data = private_data[chip] + bd_chn;

	h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
	/* go through this channel */
	vos_list_for_each_entry_safe(job_item, next, &dec_data->standby_list, standby_list) {
		/* job not processed yet */
		SET_JOB_STATUS(job_item, JOB_STATUS_FLUSH);
        vos_list_del_init(&job_item->standby_list);

        /* remove job in codec_list */
        vos_list_del_init(&job_item->job_list);
		//h26xd_info("chip:%d, chan:%d, job_id:%d is set to JOB_STATUS_FLUSH. \n", chip, bd_chn, job_item->job_id);
	}
	h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags); /* unlock for list */

	/* start to flush jobs. callback schedule will clear xx.is_used flag */
	atomic_set(&bind_chn_idx[chip][bd_chn].is_closed, 1);

	h26xd_trigger_callback(chip);

	return 0;
}

int h26xdec_setCallbackFunc(void *param)
{
	if (NULL == param) {
		h26xd_err("Error to set CB Func (param is NULL)\n");
		return KDRV_ERR_NG;
	} else {
		h26xd_cb_func = (KDRV_CALLBACK_FUNC *)param;
		return KDRV_OK;;
	}
}

/* called from kdrv_videodec_h26x_set()
 */
int h26xdec_setInitial(unsigned int handle, KDRV_VDODEC_H26X_DATA *p_dec_data)
{
	h26xd_countr_t *counter_ptr;
	job_log_item_t *log_item = NULL;
	struct h26xd_job_item_t *job_item = NULL;
	unsigned int chip;
	unsigned int chn;
	unsigned int codec_engine_type = KDRV_DEV_ID_ENGINE(handle);

	if (KDRV_VIDEODEC_ENGINE_H264 != codec_engine_type && KDRV_VIDEODEC_ENGINE_H265 != codec_engine_type) {
		h26xd_err("Error to set job (handle 0x%x is invalid)\n", handle);
		goto fail_ret;
	}

	if ((KDRV_VIDEODEC_ENGINE_H264 != codec_engine_type) && (KDRV_VDODEC_TYPE_H264 == p_dec_data->param.codec_type)) {
		h26xd_err("Error to set job (codec_engine is 265, but dec_job is 264)!\n");
		goto fail_ret;
	}

	if ((KDRV_VIDEODEC_ENGINE_H265 != codec_engine_type) && (KDRV_VDODEC_TYPE_H265 == p_dec_data->param.codec_type)) {
		h26xd_err("Error to set job (codec_engine is 264, but dec_job is 265)!\n");
		goto fail_ret;
	}

	if (NULL == h26xd_cb_func) {
		h26xd_err("Error to set job (CB Func is not setting)\n");
		goto fail_ret;
	}

	chip = KDRV_DEV_ID_CHIP(handle);
	chn = KDRV_DEV_ID_CHANNEL(handle);

	log_item = __h26xd_alloc_job_log_item(chip);
	if (log_item == NULL) {
		printk("[DE] error in alloc log_item!\n");
		damnit("DE");
		goto fail_ret;
	}

	counter_ptr = counter[chip] + chn;
	counter_ptr->fd = p_dec_data->param.fd;

	/* RC address check. The address should not be in kernel range! */
	if ((chip == 0) && (address_check == 1)) {
		if (pfn_valid(p_dec_data->param.out_frame_buffer.addr_pa >> PAGE_SHIFT)) {
			h26xd_err("rec_addr:0x%lx is in kernel space! \n", (unsigned long)p_dec_data->param.out_frame_buffer.addr_pa);
			counter_ptr->address_err ++;
			goto fail_ret;
		}

		if (pfn_valid(p_dec_data->param.mbinfo_buffer.addr_pa >> PAGE_SHIFT)) {
			h26xd_err("mbinfo_buffer:0x%lx is in kernel! \n", (unsigned long)p_dec_data->param.mbinfo_buffer.addr_pa);
			counter_ptr->address_err ++;
			goto fail_ret;
		}

		if (pfn_valid(p_dec_data->param.in_bs_buffer.addr_pa >> PAGE_SHIFT)) {
			h26xd_err("bs_buffer:0x%lx is in kernel! \n", (unsigned long)p_dec_data->param.in_bs_buffer.addr_pa);
			counter_ptr->address_err ++;
			goto fail_ret;
		}
	}

	job_item = __h26xd_alloc_job_item(sizeof(struct h26xd_job_item_t));
	if (NULL == job_item) {
		h26xd_err("Error to set job (handle 0x%x allocate job_item failed)\n", handle);
		goto fail_ret;
	}

	/* initial job_item */
	__h26xd_job_init(job_item);

	job_item->job_id = p_dec_data->param.frame_id;
	job_item->fd = p_dec_data->param.fd;
	job_item->chip = chip;
	job_item->engine = 0;
	job_item->chn = chn;
	job_item->codec_type = p_dec_data->param.codec_type;
	job_item->yuv_width_thrd = p_dec_data->param.yuv_width_thrd;
	job_item->sub_yuv_ratio = p_dec_data->param.sub_yuv_ratio;
	job_item->hdal_max_ref_num = p_dec_data->param.hdal_max_ref_num;

	job_item->bs_addr_va = p_dec_data->param.in_bs_buffer.addr_va;	/* may be replaced by RC local memory due to bs in EP */

	job_item->bs_addr_pa = p_dec_data->param.in_bs_buffer.addr_pa;
	job_item->in_buf_size = p_dec_data->param.in_bs_buffer.size;
	job_item->bs_size = p_dec_data->param.bs_size;
	job_item->bs_ddr_id = p_dec_data->param.in_bs_buffer.ddr_id;
	job_item->dst_bg_dim = p_dec_data->param.hdal_rec_bg_dim;
	job_item->org_bs_addr_pa = job_item->bs_addr_pa;

	/* the case that bs in EP, thus job_item->bs_addr_va is zero! */
	if (!h26xd_support_ep_bs && !job_item->bs_addr_va) {
		printk_ratelimited("%s, bs_ddr_id = %d, h26xd_support_ep_bs = %d (module parameter should be 1 due to bs in EP case)!!! \n", __func__, job_item->bs_ddr_id, h26xd_support_ep_bs);
		goto fail_ret;
	}

	/* reconstruct buffer */
	job_item->org_rec_addr = p_dec_data->param.out_frame_buffer.addr_pa;
	job_item->rec_size = p_dec_data->param.out_frame_buffer.size;
	job_item->rec_ddr_id = p_dec_data->param.out_frame_buffer.ddr_id;
	job_item->rec_addr = job_item->org_rec_addr;

	/* mbinfo buffer */
	job_item->mbinfo_addr = p_dec_data->param.mbinfo_buffer.addr_pa;
	job_item->mbinfo_size = p_dec_data->param.mbinfo_buffer.size;
	job_item->mbinfo_ddr_id = p_dec_data->param.mbinfo_buffer.ddr_id;

	job_item->fw_disable = 0;

	if (KDRV_VDODEC_TYPE_H264 == job_item->codec_type) {
#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
		job_item->fw_uv_swap = 0 ;
#else
		job_item->fw_uv_swap = 1; /* yuv420 - vpe does not uv_swap */
#endif
	} else {
		job_item->fw_uv_swap = 0; /* 16x2 - vpe will uv_swap */
	}
	job_item->sub_rec_addr = p_dec_data->param.sub_out_frame_buffer.addr_pa;
	job_item->sub_rec_size = p_dec_data->param.sub_out_frame_buffer.size;
	job_item->sub_rec_ddr_id = p_dec_data->param.sub_out_frame_buffer.ddr_id;
	if (job_item->sub_yuv_ratio) {
		/* p_dec_data->param.hdal_rec_bg_dim is the max width/height */
		job_item->extw_uv_swap = 1;	/* fixed in swap. */

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
		job_item->extw_uv_swap = 0;
		job_item->extw_sce_enabled = 0;
#else
		job_item->extw_sce_enabled = p_dec_data->param.sce_enable_flag;
#endif
		job_item->sub_yuv_en = 1;
	}

	job_item->callback_data = &p_dec_data->result;
	job_item->callback_data->fd = job_item->fd;
	p_dec_data->job_idx = job_item->job_item_id;

	/* log */
	job_item->log_item = log_item;
	log_item->job_id = job_item->job_id;
	log_item->job_id = job_item->job_id;
	log_item->fd = job_item->fd;
	log_item->chn = (unsigned char)job_item->chn;
	log_item->codec_type = job_item->codec_type;
	log_item->bs_len = job_item->bs_size;

	h26xd_info("chip:%d,chn:%d is xx_set() ok. \n", chip, chn);
	return KDRV_OK;

fail_ret:
	if (job_item != NULL)
		__h26xd_free_job_item(job_item);

	return KDRV_ERR_NG;
}
/* important note: In this stage, necessary resource is ready */
int h26xdec_prepareOnePicture(unsigned int handle, void *p_param)
{
	struct h26xd_job_item_t *job_item;
	h26xd_countr_t *counter_ptr;
	struct h26xd_data_t *dec_data;
	unsigned long flags;
	unsigned int chn = KDRV_DEV_ID_CHANNEL(handle);
	unsigned int chip = KDRV_DEV_ID_CHIP(handle);
	KDRV_VDODEC_H26X_DATA *p_dec_data = (KDRV_VDODEC_H26X_DATA *)p_param;

	dec_data = private_data[chip] + chn;
	counter_ptr = counter[chip] + chn;

	/* job_item is created in in h26xdec_setInitial() */
	job_item = __h26xd_find_job_item(p_dec_data->job_idx);
	if (NULL == job_item) {
		h26xd_err("0x%x does not find job_item! code bug!!!\n", handle);
		counter_ptr->internal_err ++;
		g_internal_err ++;
		goto fail_ret;
	}

	if ((int)chip >= h26xd_max_chip) {
		h26xd_err("0x%x, job_item->chip:%d is invalid!!!\n", handle, chip);
		goto fail_ret;
	}
	job_item->puttime = (unsigned int)jiffies;

	if ((atomic_read(&bind_chn_idx[chip][chn].is_ready)) == 0) {
		h26xd_warn("{chn:%d}Error to putjob (handle 0x%x is not open or ready!) \n", chn, handle);
		goto err_ret;
	}

	/* during closing, the incoming jobs are treated as fail! */
	if (atomic_read(&bind_chn_idx[chip][chn].is_closed) == 1) {
		h26xd_warn("still put job during channel%d closing! (handle: 0x%x)\n", chn, handle);
		goto err_ret;
	}

	job_item->in_property_parsed_flg = 1;
	job_item->callback_data->frame_id = job_item->job_id;

	/* lock list */
	h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);

	h26xd_info("job_item with job_item_id:%d, job_id:%u is allocated. \n", job_item->job_item_id, job_item->job_id);

	SET_JOB_STATUS(job_item, JOB_STATUS_STANDBY);

	//h26xd_info("{chip:%d,chan:%d} job_id:%d is set to JOB_STATUS_STANDBY. \n", chip, chn, job_item->job_id);
	counter_ptr->putjob_cnt ++;

	/* per channel list */
	vos_list_add_tail(&job_item->standby_list, &dec_data->standby_list);
	vos_list_add_tail(&job_item->chan_list, &dec_data->chan_list);

	if (KDRV_VDODEC_TYPE_H264 == job_item->codec_type) {
		vos_list_add_tail(&job_item->job_list, &h264d_job_head[chip]);
		//h26xd_dbg("[%s] 264 job add id:%u\n", __FUNCTION__, job_item->job_id);
	} else {
		vos_list_add_tail(&job_item->job_list, &h265d_job_head[chip]);
		//h26xd_dbg("[%s] 265 job add id:%u\n", __FUNCTION__, job_item->job_id);
	}

	/* unlock list */
	h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
	/* wake up thread */
	h26xd_trigger_work(job_item->chip);

	return KDRV_OK;

err_ret:
	job_item->callback_data->status = KDRV_ERR_NG;
	__h26xd_free_job_item(job_item);

fail_ret:
	return KDRV_ERR_NG;
}

int h26xdec_getChipInfo(void *param)
{
    KDRV_VDODEC_H26X_CHIP_INFO *chip_info = (KDRV_VDODEC_H26X_CHIP_INFO *)param;

    chip_info->chip = h26xd_max_chip;
    chip_info->engine = 1;
    chip_info->chan = max_total_cam_ch;

    return 0;
}

int h26xdec_getCabacNeedBufferSize(void *param)
{
    KDRV_VDODEC_H26X_NEED_CABAC_BUF_INFO *buf_info = (KDRV_VDODEC_H26X_NEED_CABAC_BUF_INFO *)param;
    unsigned int need_bin_bsdma_size;
    unsigned int need_size;
    unsigned int lmt_size;
    unsigned int fifo_flag = (0 == h265_fifo_mode) ? 2 : 1;

    need_size = buf_info->width * buf_info->height;
    lmt_size = h26xd_max_width * h26xd_max_height;

    if (need_size > lmt_size) {
        printk("%s: ERROR set_w x set_h (%dx%d) > kdrv_max_w * kdrv_max_h (%dx%d)\n", __FUNCTION__,
            buf_info->width, buf_info->height, h26xd_max_width, h26xd_max_height);
        buf_info->size = 0;

        return -1;
    } else {
        /* bin_out_size     : max_w * max_h * 1.5 * 0.3 * 4
        *  bin_out_cmd_size : (20 * (max_h / 64) * 2 + 1) * 2
        */
        need_bin_bsdma_size = ALIGN((((MaxTileCols * (h26xd_max_height >> 4)) << 1) + 1) << 2, DCACHE_LINE_SIZE) * fifo_flag;

        buf_info->size = ALIGN((need_size * 9 / 5), 128) + ALIGN(need_bin_bsdma_size, 128);

        return 0;
    }
}

int h26xdec_freeCabacBuffer(void *param)
{
	KDRV_VDODEC_H265_CABAC_BUF *buf_info = (KDRV_VDODEC_H265_CABAC_BUF *)param;
	int chip_id;
	int ret = 0;

	chip_id = buf_info->chip_id;

	if (0 == h265d_hdal_allocate_buffer_flag[chip_id]) {
		printk("%s: buffer chip id(%d) ddr no(%u) does not set this buffer!!\n", __FUNCTION__, chip_id, (unsigned int)buf_info->ddr_no);
		ret = -1;
	} else {
		int i;
		/* ping pong bin out buffer */
		for (i = 0; i < 2; i++) {
			h265d_cabac_buffer[chip_id][i].chip_id = 0;
			h265d_cabac_buffer[chip_id][i].ddr_no = 0;
			h265d_cabac_buffer[chip_id][i].addr_pa = 0;
			h265d_cabac_buffer[chip_id][i].size = 0;
		}
		h265d_hdal_allocate_buffer_flag[chip_id] = 0;
	}

	return ret;
}

int h26xdec_setCabacBuffer(void *param)
{
	KDRV_VDODEC_H265_CABAC_BUF *buf_info = (KDRV_VDODEC_H265_CABAC_BUF *)param;
	int chip_id;
	int ret = 0;

	if (buf_info->chip_id >= ENTITY_CHIPS) {
		printk("%s: buffer chip id(%u) out of range(%d)!!\n", __FUNCTION__, (unsigned int)buf_info->chip_id, ENTITY_CHIPS);
		ret = -1;
		goto exit_set;
	} else {
		chip_id = buf_info->chip_id;
	}

	if (0 == h265d_hdal_allocate_buffer_flag[chip_id]) {
		int i;
		unsigned int cabac_buf_size;
        unsigned int max_bin_bsdma_size;
        unsigned int fifo_flag = (0 == h265_fifo_mode) ? 2: 1;

        max_bin_bsdma_size = fifo_flag * ALIGN((((MaxTileCols * (h26xd_max_height >> 4)) << 1) + 1) << 2, DCACHE_LINE_SIZE);

        if ((unsigned int)buf_info->cabac_buffer.size <= max_bin_bsdma_size) {
            printk("%s: chip id(%d) ddr no(%u) dec_tile_buffer size is not enough, please adjust it!!\n", __FUNCTION__, chip_id, (unsigned int)buf_info->ddr_no);
            ret = -1;
            goto exit_set;
        } else {
            cabac_buf_size = (buf_info->cabac_buffer.size >> 7) << 7; /* FLOOR */
        }

        if (h265_fifo_mode) {
    		for (i = 0; i < 2; i++) {
    			h265d_cabac_buffer[chip_id][i].chip_id = chip_id;
    			h265d_cabac_buffer[chip_id][i].ddr_no = buf_info->ddr_no;
    			h265d_cabac_buffer[chip_id][i].addr_pa = ALIGN(buf_info->cabac_buffer.addr_pa, 128) ;
    			h265d_cabac_buffer[chip_id][i].size = cabac_buf_size;
    		}
        } else {
    		for (i = 0; i < 2; i++) {
    			h265d_cabac_buffer[chip_id][i].chip_id = chip_id;
    			h265d_cabac_buffer[chip_id][i].ddr_no = buf_info->ddr_no;
    			h265d_cabac_buffer[chip_id][i].addr_pa = ALIGN((buf_info->cabac_buffer.addr_pa + (cabac_buf_size / 2) * i), 128) ;
    			h265d_cabac_buffer[chip_id][i].size = cabac_buf_size / 2;
    		}
        }
		h265d_hdal_allocate_buffer_flag[chip_id] = 1;
	} else {
		printk("%s: chip id(%d) ddr no(%u) buffer is allocated, please release it first!!\n", __FUNCTION__, chip_id, (unsigned int)buf_info->ddr_no);
		ret = -1;
		goto exit_set;
	}

exit_set:

	return ret;
}

/* ====================== K_DRV ================================== */

/*
*	record reserve/free buffer counter. They work in atomic context
*/
void decoder_reserve_buffercnt(int chip, int chn)
{
	resv_total_out_cnt++;
	if (resv_total_out_cnt == (1 << 29)) {
		resv_total_out_cnt = 0;
	}
	resv_cur_cnt[chip][chn]++;
}

void decoder_free_buffercnt(int chip, int chn)
{
	free_total_out_cnt++;
	if (free_total_out_cnt == (1 << 29)) {
		free_total_out_cnt = 0;
	}
	resv_cur_cnt[chip][chn]--;
}

/* must work in h26x_dec_lock atomic context
 * release job's reserved output buffer and memory of job_item
 * Note: job_item->chan_list is also removed from dec_data->chan_list
 */
STATIC int __h26xd_job_cleanup(struct h26xd_job_item_t *job_item)
{
	/* release job item when:
	 * 1. job's output buffer is not referenced
	 * 2. job has been callback
	 */
	int	ret = 1;

	if (GET_JOB_STATUS(job_item) & (JOB_STATUS_DPB_REL | JOB_STATUS_FLUSH | JOB_STATUS_FAIL)) {
		if (job_item->res_flg) {
			decoder_free_buffercnt(job_item->chip, job_item->chn);
			job_item->res_flg = 0;

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
			h26xd_cb_func->free_buf(job_item->org_rec_addr, (VOID *)&job_item->fd);
#else
			h26xd_cb_func->free_buf(job_item->org_rec_addr, job_item->rec_ddr_id, job_item->job_id);
#endif
            h26xd_info("{%d,%d,%d} calling video_free_buffer for job_id:%d org_buf:%lx\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, (unsigned long)job_item->org_rec_addr);
		}
		/* remove from chan list */
		vos_list_del_init(&job_item->chan_list);
		/* free job_item un-conditional */
		__h26xd_free_job_item(job_item);
		ret = 0;
	} else {
		/* other status such as JOB_STATUS_KEEP is valid, others are not. */
		if (GET_JOB_STATUS(job_item) != JOB_STATUS_KEEP) {
			h26xd_countr_t *counter_ptr = counter[job_item->chip] + job_item->chn;

			counter_ptr->state_machine_err ++;
			g_state_machine_err ++;
			h26xd_err("Error! job_id:%u, wrong status:%d to free job_item!\n", job_item->job_id, GET_JOB_STATUS(job_item));
			ret = -1;
		} else {
			//h26xd_info("job_id:%u, status:%d, OK, not free job_item!\n", job_item->job_id, GET_JOB_STATUS(job_item));
			ret = 1;
		}
	}

	return ret;
}

/*
*  set information of callback data
*/
STATIC void decoder_set_callback_job_status(struct h26xd_job_item_t *job_item)
{
	unsigned long flags;
	KDRV_VDODEC_H26X_CB_DATA *p_callback_data = job_item->callback_data;
	int fail_flag = 0;

	if ((GET_JOB_STATUS(job_item) & (JOB_STATUS_FLUSH | JOB_STATUS_FAIL)) || (job_item->err_num != H26XD_OK)) {
		h26xd_countr_t *counter_ptr = counter[job_item->chip] + job_item->chn;
		p_callback_data->status = KDRV_ERR_NG;
		h26xd_info("{%d,%d,%d} job_id:%u callback fail (%s) bs_size %u\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, h26xd_job_status_str(job_item->status), job_item->bs_size);
		if (counter_ptr->record_isr_err_cnt > h26xd_err_count_thd) {
			h26xd_err("fd(%8x) dec_chn(%u) DEC_ERR_ISR is over %u times\n", job_item->fd, job_item->chn, h26xd_err_count_thd);
			h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
			counter_ptr->record_isr_err_cnt = 0;
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
		} else if (counter_ptr->record_isr_err_cnt == 1 && job_item->err_pos == ERR_ISR) {
		    h26xd_err("fd(%8x) dec_chn(%u) DEC_ERR_ISR\n", job_item->fd, job_item->chn);
        }
		if (counter_ptr->record_hw_timeout_cnt > h26xd_err_count_thd) {
			h26xd_err("fd(%8x) dec_chn(%u) DEC_HW_TIMEOUT is over %u times\n", job_item->fd, job_item->chn, h26xd_err_count_thd);
			h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
			counter_ptr->record_hw_timeout_cnt = 0;
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
		} else if (counter_ptr->record_hw_timeout_cnt == 1 && job_item->err_pos == ERR_ISR) {
		    h26xd_err("fd(%8x) dec_chn(%u) DEC_HW_TIMEOUT\n", job_item->fd, job_item->chn);
        }

		h26xd_save_bitstream_to_file(job_item);
		fail_flag = 1;
		if (h26xd_fail_damnit) {
			h26xd_err("fd(%8x) dec_chn(%u) job_id:%u callback fail, bs_size %u\n", job_item->fd, job_item->chn, job_item->job_id, job_item->bs_size);
			damnit("DE");
		}
	} else if (GET_JOB_STATUS(job_item) & (JOB_STATUS_KEEP | JOB_STATUS_DPB_REL)) {
		p_callback_data->status = KDRV_OK;
		h26xd_info("{%d,%d,%d} job_id:%u callback finish (%s) bs_size %u\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, h26xd_job_status_str(job_item->status), job_item->bs_size);
	} else {
		h26xd_warn("{%d,%d,%d} job_id:%u callback incorrect (%s) bs_size %u\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, h26xd_job_status_str(job_item->status), job_item->bs_size);
		fail_flag = 1;
		if (h26xd_fail_damnit) {
			h26xd_err("fd(%8x) dec_chn(%u) job_id:%u callback incorrect, bs_size %u\n", job_item->fd, job_item->chn, job_item->job_id, job_item->bs_size);
			damnit("DE");
		}
	}
	if (dump_bs_msg[0] > 0 && (dump_bs_msg[1] == job_item->fd || dump_bs_msg[1] == 0xffffffff)) {
		if (dump_bs_msg[3] && fail_flag == 0) {
			return ;
		} else {
			unsigned char *tmp_bs_val = (unsigned char *)job_item->bs_addr_va;
			if (job_item->bs_size < 16 || tmp_bs_val == NULL) {
				DUMP_BS_MSG("fd(%8x) bs_size(%u) is smaller 16 bytes or dec_in_buffer is NULL type(%s)\n", job_item->fd, job_item->bs_size, (fail_flag == 0 ? "OK" : "FAIL"));
			} else {
				int i;
				int dump_size = (dump_bs_msg[2] > job_item->bs_size) ? 256 : ALIGN(dump_bs_msg[2], 16);
				DUMP_BS_MSG("fd(%8x) bs_size(%u) job_id(%u) type(%s)\n", job_item->fd, job_item->bs_size, job_item->job_id, (fail_flag == 0 ? "OK" : "FAIL"));
				for (i = 0; i < dump_size; i += 16) {
					DUMP_BS_MSG("  0x%08x:  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						    job_item->bs_addr_va + i,
						    tmp_bs_val[i], tmp_bs_val[i + 1], tmp_bs_val[i + 2], tmp_bs_val[i + 3],
						    tmp_bs_val[i + 4], tmp_bs_val[i + 5], tmp_bs_val[i + 6], tmp_bs_val[i + 7],
						    tmp_bs_val[i + 8], tmp_bs_val[i + 9], tmp_bs_val[i + 10], tmp_bs_val[i + 11],
						    tmp_bs_val[i + 12], tmp_bs_val[i + 13], tmp_bs_val[i + 14], tmp_bs_val[i + 15]);
				}
			}
		}
	}
}

/*
* reserve job's output buffer
*/
STATIC void __h26xd_job_reserve_buf(struct h26xd_job_item_t *job_item)
{
	if (job_item->res_flg) { /* error checking. This should never happen */
		h26xd_countr_t *counter_ptr = counter[job_item->chip] + job_item->chn;;

		counter_ptr->state_machine_err ++;
		g_state_machine_err ++;

		h26xd_err("{%d,%d,%d} job_id:%u, %s for a reserved job\n",
			  job_item->chip, job_item->engine, job_item->chn, job_item->job_id, __func__);

		return;
	}

	h26xd_info("{%d,%d,%d} calling video_reserve_buffer for job_id:%d org_buf:%lx\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, (unsigned long)job_item->org_rec_addr);

	decoder_reserve_buffercnt(job_item->chip, job_item->chn);

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
	h26xd_cb_func->reserve_buf(job_item->org_rec_addr, (VOID *)&job_item->fd);
#else
	h26xd_cb_func->reserve_buf(job_item->org_rec_addr, job_item->rec_ddr_id, job_item->job_id);
#endif

	job_item->res_flg = 1;
}

/*
* Set job's output property
*/
void decoder_set_out_property(struct h26xd_data_t *dec_data, struct h26xd_job_item_t *job_item)
{
	DecoderParams *pDec = dec_data->dec_handle;
	KDRV_VDODEC_H26X_CB_DATA *p_callback_data = job_item->callback_data;
	job_log_item_t *log_item = (job_log_item_t *)job_item->log_item;
	int output_type = 0;
	/* fixed job dst_fmt
	*  FORM_FMT_PROP_VALUE(fmt_1st, fmt_2nd, fmt_3rd, fmt_4th)
	*  ((fmt_2nd) << 24 | (fmt_2nd) << 16 | (fmt_2nd) << 8 | (fmt_1st))
	*/
	if (KDRV_VDODEC_TYPE_H264 == dec_data->codec_type)
        output_type |= 0x1;

	if (job_item->extw_sce_enabled)
		output_type |= 0x2;

	switch (output_type) {
	case 0:  /* 265  non-sce */
		job_item->dst_fmt = KDRV_BUFTYPE_YUV420_16x2 | (KDRV_BUFTYPE_YUV420_SP << 8) | (KDRV_BUFTYPE_UNKNOWN << 16) | (KDRV_BUFTYPE_UNKNOWN << 24);
		break;
	case 1:  /* 264  non-sce */
		job_item->dst_fmt = KDRV_BUFTYPE_YUV420_SP | (KDRV_BUFTYPE_YUV420_SP << 8) | (KDRV_BUFTYPE_UNKNOWN << 16) | (KDRV_BUFTYPE_UNKNOWN << 24);
		break;
	case 2:  /* 265  sce */
		job_item->dst_fmt = KDRV_BUFTYPE_YUV420_16x2 | (KDRV_BUFTYPE_YUV420_SCE << 8) | (KDRV_BUFTYPE_UNKNOWN << 16) | (KDRV_BUFTYPE_UNKNOWN << 24);
		break;
	default: /* 264  sce */
		job_item->dst_fmt = KDRV_BUFTYPE_YUV420_SP | (KDRV_BUFTYPE_YUV420_SCE << 8) | (KDRV_BUFTYPE_UNKNOWN << 16) | (KDRV_BUFTYPE_UNKNOWN << 24);
		break;
	}

	dec_data->dst_fmt = job_item->dst_fmt;

	p_callback_data->dst_fmt = dec_data->dst_fmt;

	/* BG_DIM */
	/* non-cropped frame size
	*  8x4 output width need to align 64
	*/
	p_callback_data->out_bg_dim.w = ALIGN(pDec->u16FrameWidth, 64); /* non-cropped frame size */
	p_callback_data->out_bg_dim.h = pDec->u16FrameHeight; /* non-cropped frame size */

	/* FG_DIM */
	p_callback_data->out_dim.w = pDec->u16FrameWidthCropped;
	p_callback_data->out_dim.h = pDec->u16FrameHeightCropped;

	/*
	h26xd_info("{%d,%d,%d}, codec_type:%s, job_id:%u set bg_dim:%d/%d, fb_dim:%d/%d, dst_fmt:0x%x, poc:%d, yuv_ratio:%d \n",
											job_item->chip, job_item->engine, job_item->chn,
											(KDRV_VDODEC_TYPE_H264 == job_item->codec_type) ? "h264" : "h265",
											job_item->job_id,
											pDec->u16FrameWidth, pDec->u16FrameHeight,
											pDec->u16FrameWidthCropped, pDec->u16FrameHeightCropped,
											dec_data->dst_fmt, job_item->poc, job_item->sub_yuv_ratio);
	*/
	if (pDec->bHasScaledFrame) {
		p_callback_data->sub_yuv_ratio = job_item->sub_yuv_ratio;
		p_callback_data->sub_bg_dim.w = pDec->u16SubBgWidth;
		p_callback_data->sub_bg_dim.h = pDec->u16SubBgHeight;
	} else {
		p_callback_data->sub_yuv_ratio = 0;
		p_callback_data->sub_bg_dim.w = 0;
		p_callback_data->sub_bg_dim.h = 0;
	}

	dec_data->frame_interlace = (0 == pDec->u32ProgSeq) ? 1 : 0;
	p_callback_data->frame_interlace = dec_data->frame_interlace;

	job_item->poc = dec_data->dec_frame.i32POC;
	p_callback_data->poc = job_item->poc;

	job_item->yuv_range = dec_data->dec_frame.i32YuvRange;
	p_callback_data->yuv_range = job_item->yuv_range;

	log_item->width = pDec->u16FrameWidth;
	log_item->height = pDec->u16FrameHeight;
}

/*
* free memory for decoder data structure (accessed by CPU only)
*/
void h26xd_vfree(void *ptr)
{
	if (ptr) {
		vfree(ptr);
		h26xd_vfree_cnt++;
	}
}

/*
* allocate memory for decoder data structure (accessed by CPU only)
*/
void *h26xd_vmalloc(unsigned int size)
{
	unsigned int  sz = size;
	void          *ptr;

	ptr = vmalloc(sz);

	if (ptr == 0) {
		printk("Fail to h26xd_vmalloc for addr %lx size 0x%x\n", (unsigned long)ptr, sz);
		return 0;
	}

	h26xd_vmalloc_cnt++;
	return ptr;
}

/*
* free DMA-able buffer via frammap
*/
STATIC int h26xd_free_buffer(int chip_idx, int eng_idx, struct buffer_info_t *buf)
{
	if (buf->addr_va) {
		dma_free_coherent(h26xd_engine_info[chip_idx][eng_idx].dev, buf->size, (void *)buf->addr_va, buf->addr_pa);

		buf->addr_va = 0;
		buf->addr_pa = 0;
	}

	return 0;
}

/*
* allocate DMA-able buffer via frammap
*/
STATIC int h26xd_allocate_buffer(int chip_idx, int eng_idx, struct buffer_info_t *buf, int size)
{
	dma_addr_t	addr_pa = 0;

	/* note, must use the other variable addr_pa to get return value due to different prototype. This prevent memory size inconsistent!!! */
	buf->addr_va = (uintptr_t)dma_alloc_coherent(h26xd_engine_info[chip_idx][eng_idx].dev, size, &addr_pa, GFP_KERNEL);
	if ((uintptr_t)0 == buf->addr_va || 0 == (uintptr_t)addr_pa) {
		h26xd_err("dma_alloc_coherent failed va: %lx pa: %lx\n", (unsigned long)buf->addr_va, (unsigned long)addr_pa);
		return -1;
	}
	buf->addr_pa = (uintptr_t)addr_pa;
	buf->size = size;
	memset_io((void *)buf->addr_va, 0, size);

	if (chip_idx != 0) {
		h26xd_err("%s, chip:%d not supported! Current only supports EP0! \n", __func__, chip_idx);
		return -1;
	}

	return 0;
}

/*
* release DMA-able memory for one engine
*/
STATIC int h26xd_release_per_engine_buffer(int chip_idx, int engine_idx)
{
	/* release engine buffer via frammap */
	if (0 == chip_idx) {
		int i;
		for (i = 0 ; i < max_total_cam_ch; i++) {
			h26xd_free_buffer(chip_idx, engine_idx, &h26xd_vpu_work_buffer[chip_idx][engine_idx][i]);
		}
		for (i = 0 ; i < 2; i++) {
			h26xd_free_buffer(chip_idx, engine_idx, &h26xd_link_list_buffer[chip_idx][engine_idx][i]);
		}
	}

	return 0;
}

/*
* allocate DMA-able memory for one engine
*/
STATIC int h26xd_allocate_per_engine_buffer(int chip_idx, int engine_idx, int max_width, int max_height)
{
	unsigned int h264d_vpu_buffer_size, h265d_vpu_buffer_size;
	unsigned int h26xd_max_vpu_buffer_size;
	unsigned int h26xd_ll_buffer_size;
	int ret = 0;
	unsigned int max_qmat_size = MAX_HEVC_QMAT_SIZE;
	unsigned int max_bsdma_size = (1 + MAX_HEVC_BSDMA_ENTRY * 2) << 2;
	int i;

	/* allocate engine buffer */
    h265d_vpu_buffer_size = ALIGN((ALIGN(max_bsdma_size, DCACHE_LINE_SIZE)
                       + (ALIGN(max_qmat_size, DCACHE_LINE_SIZE) + ALIGN(MAX_HEVC_RPIC_SIZE, DCACHE_LINE_SIZE) + ALIGN(MAX_HEVC_RPIC_MCINFO_SIZE, DCACHE_LINE_SIZE)
                      + ALIGN(MAX_HEVC_RLIST_SIZE, DCACHE_LINE_SIZE)) * MAX_PIC_RESOURCE_SIZE), 128);

	h264d_vpu_buffer_size = 0x1000;

	h26xd_ll_buffer_size = 0x600 * max_total_cam_ch;

	//coverity[dead_error_line]: h265d_vpu_buffer_size depends on platform. Thus we keep this statement.
	h26xd_max_vpu_buffer_size = (h265d_vpu_buffer_size > h264d_vpu_buffer_size) ? h265d_vpu_buffer_size : h264d_vpu_buffer_size;

	/* allocate vpu work buffer */
	for (i = 0; i < max_total_cam_ch; i++) {
		ret = h26xd_allocate_buffer(chip_idx, engine_idx, &h26xd_vpu_work_buffer[chip_idx][engine_idx][i], h26xd_max_vpu_buffer_size);
		if (ret < 0) {
			h26xd_err("{chip:%d,eng:%d} allocate h26x engine vpu buffer [%d] error\n", chip_idx, engine_idx, i);
			goto allocate_fail;
		}
	}

	for (i = 0; i < 2; i++) {
		/* allocate link list buffer */
		ret = h26xd_allocate_buffer(chip_idx, engine_idx, &h26xd_link_list_buffer[chip_idx][engine_idx][i], h26xd_ll_buffer_size);
		if (ret < 0) {
			h26xd_err("{chip: %d,eng:%d} allocate h26x engine link list buffer error\n", chip_idx, engine_idx);
			goto allocate_fail;
		}
	}


	return 0;

allocate_fail:
	h26xd_release_per_engine_buffer(chip_idx, engine_idx);
	return -1;
}

/* must work in h26x_dec_lock critical section */
int h26xd_free_buf_idx(struct h26xd_data_t *dec_data, int buf_idx)
{
	struct h26xd_job_item_t *job_item;

	/* JOB_STATUS_FLUSH case */
	if (buf_idx == -1)
		return 0;	/* do nothing */

	job_item = dec_data->dec_buf[buf_idx].job_item;

	h26xd_info("{%d,%d,%d} free_buf_idx:%d, status:%d, job_id:%u.\n", dec_data->chip, dec_data->engine, dec_data->chn,
		   buf_idx, job_item->status, job_item->job_id);

	dec_data->dec_buf[buf_idx].job_item = NULL;
	dec_data->dec_buf[buf_idx].is_used = 0;
	dec_data->used_buffer_num --;

	return 0;
}

/* must work in h26x_dec_lock critical section
 * allocate buf_idx
 */
int h26xd_get_buf_idx(struct h26xd_data_t *dec_data, struct h26xd_job_item_t *job_item)
{
	BufferAddr *dec_buf_ptr;
	unsigned int i;
    int buf_idx;

	buf_idx = -1;
	for (i = 0; i < h26xd_max_buf_num; i++) {
		if (!dec_data->dec_buf[i].is_used) {
			dec_data->dec_buf[i].is_used = 1;
			buf_idx = i;
			job_item->buf_idx = buf_idx;
			break;
		}
	}

	if (buf_idx < 0) {
		h26xd_countr_t *counter_ptr = counter[job_item->chip] + job_item->chn;

		counter_ptr->no_bufidx ++;
		return -1;
	} else {
		dec_data->used_buffer_num++;
		dec_buf_ptr = &dec_data->dec_buf[buf_idx];

		/* mbinfo buffer */
		dec_buf_ptr->mbinfo_pa = job_item->mbinfo_addr;
		dec_buf_ptr->mbinfo_size = job_item->mbinfo_size;

		/* reconstruct buffer */
		dec_buf_ptr->start_pa = job_item->rec_addr;
		dec_buf_ptr->yuv_buf_size = job_item->rec_size;

		/* scaled buffer */
		if (job_item->sub_yuv_en) {
			dec_buf_ptr->scale_pa = job_item->sub_rec_addr;
			dec_buf_ptr->scale_buf_size = job_item->sub_rec_size;
		} else {
			dec_buf_ptr->scale_pa = 0;
			dec_buf_ptr->scale_buf_size = 0;
		}

		dec_buf_ptr->job_item = job_item; /* link job_item to the buffer */
		dec_buf_ptr->is_released = 0;
		dec_buf_ptr->is_outputed = 0;
	}

	h26xd_info("{chip:%d,chn:%d} allocate buf_idx:%d, job_id:%u \n", job_item->chip, job_item->chn, job_item->buf_idx, job_item->job_id);

	return 0;
}

/* mark Job as DPB Release. Then these jobs can return its construct buffer to upper layer. */
STATIC int __h26xd_mark_job_as_dpb_release(struct h26xd_data_t *dec_data, int buf_idx)
{
	struct h26xd_job_item_t *job_item;

	job_item = dec_data->dec_buf[buf_idx].job_item;
	if (NULL == job_item) {
		h26xd_err("{%d,%d,%d}, Error! no job is recorded for idx: %d\n", dec_data->chip, dec_data->engine, dec_data->chn, buf_idx);
		return -1;
	}

	/* if this job was successful job, then mark it as DPB_REL */
	if (GET_JOB_STATUS(job_item) == JOB_STATUS_KEEP) {
		SET_JOB_STATUS(job_item, JOB_STATUS_DPB_REL);
		h26xd_info("{%d,%d,%d} DPB release: job_id:%u buf_idx:%d, set to JOB_STATUS_DPB_REL\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, buf_idx);
	} else if (GET_JOB_STATUS(job_item) == JOB_STATUS_FAIL) {
		/* the case that: job is on hardware and reference list, but the hardware reports error! Thus it causes this job in dpb list to be flushed */
		h26xd_info("{%d,%d,%d} job_id:%u buf_idx:%d, low level driver has error case!!!\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, buf_idx);
	} else {
		h26xd_countr_t *counter_ptr = counter[job_item->chip] + job_item->chn;

		counter_ptr->state_machine_err ++;
		g_state_machine_err ++;
		/* should not happen */
		h26xd_warn("{%d,%d,%d} Error! job_id:%u buf_idx:%d status: %d should have JOB_STATUS_KEEP!\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, buf_idx, job_item->status);
	}

	return 0;
}

/* mark dec_buf[buf_idx] as xx.is_outputed = 1
 * This function is mainly for debug purpose.
 */
STATIC int __h26xd_mark_bufidx_as_outputed(struct h26xd_data_t *dec_data, int buf_idx)
{
	struct h26xd_job_item_t *job_item;
	h26xd_countr_t *counter_ptr = counter[dec_data->chip] + dec_data->chn;

	job_item = dec_data->dec_buf[buf_idx].job_item;
	if (NULL == job_item) {
		h26xd_err("{%d,%d,%d} error!!! no job is recorded for buf_idx: %d\n", dec_data->chip, dec_data->engine, dec_data->chn, buf_idx);
		g_internal_err ++;
		return -1;
	}

	/* debug purpose */
	if (dec_data->dec_buf[buf_idx].is_outputed == 1)
		return 0;	/* do nothing */

	h26xd_info("{%d,%d,%d} info: job_id:%u buf_idx:%d status:%d(is_released:%d). Set is_outputed=1\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, buf_idx, GET_JOB_STATUS(job_item), dec_data->dec_buf[buf_idx].is_released);

	/* santy check 1: job status */
	switch (job_item->status) {
	case JOB_STATUS_DPB_REL:
	case JOB_STATUS_KEEP:
		break;
	case JOB_STATUS_FAIL:
		//h26xd_info("{%d,%d,%d} job that has error: job_idx:%u buf_idx:%d status:%d\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, buf_idx, job_item->status);
		break;
	default:
		/* NOTE: it should never happen */
		h26xd_warn("{%d,%d,%d} got poc for job that is not waiting poc: job_id:%u buf_idx:%d status:%d\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, buf_idx, job_item->status);
		counter_ptr->state_machine_err ++;
		g_state_machine_err ++;
		return -1;
		break;
	}

	dec_data->dec_buf[buf_idx].is_outputed = 1;

	return 0;
}

/*
* mark DPB buffers in release list as released
*/
STATIC int __h26xd_process_release_list(struct h26xd_data_t *dec_data)
{
	unsigned int i;
	unsigned int buf_idx;
	unsigned char *buf_idx_array = dec_data->dec_frame.u8ReleaseBuffer;
	unsigned char buf_num;

	buf_num = dec_data->dec_frame.u8ReleaseBufferNum;

	for (i = 0; i < buf_num; i ++) {
		buf_idx = buf_idx_array[i];
		if (buf_idx > h26xd_max_buf_num) {
			int k;
			h26xd_err("%s - codec_type(0x%x) buf_idx(%d) is over h26xd_max_buf_num(%d)\n", __FUNCTION__, dec_data->codec_type, buf_idx, h26xd_max_buf_num);
			for (k = 0; k < buf_num; k++) {
				h26xd_err("buf_idx_array[%d] : %u\n", k, buf_idx_array[k]);
			}

			return 0;
		}

		/* The job may be released already before.
		 * The case: job was in u8ReleaseBuffer and released already. During the channel stopping,
		 * __h26xd_output_all_frame() is called immediately. The causes the job in u8ReleaseBuffer to
		 * double release. Please check callback_scheduler() flow.
		 */
		if (!dec_data->dec_buf[buf_idx].is_used || dec_data->dec_buf[buf_idx].is_released)
			continue;

		__h26xd_mark_bufidx_as_outputed(dec_data, buf_idx);
		__h26xd_mark_job_as_dpb_release(dec_data, buf_idx);
		/* set to release of this buffer */
		dec_data->dec_buf[buf_idx].is_released = 1;
	}

	return 0;
}

/*
 * output all frames (kept by low level driver) of the channel and move them to callback list
 * Mark those reference list buffers to 1 (dec_data->dec_buf[buf_idx].is_released=1 & is_outputed = 1)
 * Notice: This function only includes the processed job_items' buffers and excludes those buffers not processed yet.
 */
STATIC int __h26xd_output_all_frame(struct h26xd_data_t *dec_data, int stop_flag)
{
	int ret;
	H26X_DEC_FRAME_IOCTL *dec_frame = &dec_data->dec_frame;

	h26xd_info("{%d,%d,%d}, output_all_frame() set all buf_idx to is_released=1.\n", dec_data->chip, dec_data->engine, dec_data->chn);

	if (KDRV_VDODEC_TYPE_H264 == dec_data->codec_type) {
		ret = H264Dec_CleanAll(dec_data->dec_handle, dec_frame);
	} else if (KDRV_VDODEC_TYPE_H265 == dec_data->codec_type) {
		ret = H265Dec_OutputAllPicture(dec_data->dec_handle, dec_frame, stop_flag);
	} else {
		/* this is directly close chn case (open chn -> close chn)
		*  normal case : open chn -> set_info -> trigger -> close chn
		*  it needed bypass, because dec_data is not setting
		*/
		ret = 0;
	}

	__h26xd_process_release_list(dec_data);

	/* avoid warning */
	if (ret) {}

	return 0;

}

/*
* Copy parsed property from job_item to per-channel data
*/
int h26xd_get_in_property(struct h26xd_data_t *dec_data, struct h26xd_job_item_t *job_item)
{
	dec_data->yuv_width_thrd = job_item->yuv_width_thrd;
	dec_data->sub_yuv_en = job_item->sub_yuv_en;
	dec_data->sub_yuv_ratio = job_item->sub_yuv_ratio;
	dec_data->dst_bg_dim = job_item->dst_bg_dim;

	return 0;
}

/*
* pass the input bitsteam address/size to lowwer level driver, and fill padding bytes
*/
void h26xd_set_bitstream(struct h26xd_data_t *dec_data, unsigned int codec_type)
{
	struct h26xd_job_item_t *job_item = dec_data->curr_job;
	uintptr_t in_addr_va = job_item->bs_addr_va;
	uintptr_t in_addr_pa = job_item->bs_addr_pa;

	/* set bitstream address to lowwer level driver */
	switch (codec_type) {
	case KDRV_VDODEC_TYPE_H264:
		H264Dec_SetBitstreamBuf(dec_data->dec_handle, in_addr_pa, in_addr_va, job_item->bs_size, job_item->in_buf_size, job_item->hdal_max_ref_num);
		break;
	case KDRV_VDODEC_TYPE_H265:
		H265Dec_SetBitstreamBuf(dec_data->dec_handle, in_addr_pa, in_addr_va, job_item->bs_size, job_item->in_buf_size, job_item->hdal_max_ref_num);
		break;
	default:
		break;
	}
}

/*
*  prepare parameters for H265Dec_OneFrame()
*/
int h26xd_set_buffer(struct h26xd_data_t *dec_data, H26X_DEC_BUFFER *pBuf, int buf_idx)
{
	struct h26xd_job_item_t *job_item;

	job_item = dec_data->curr_job;

	if (buf_idx < 0 || buf_idx >= (int)h26xd_max_buf_num) {
		h26xd_err("{%d,%d,%d} set_buffer invalid buf_idx:%d for job_id:%u\n", dec_data->chip, dec_data->engine, dec_data->chn, buf_idx, job_item->job_id);
		return -1;
	}

	if (dec_data->dec_buf[buf_idx].is_used == 0) {
		h26xd_err("{%d,%d,%d} set_buffer to an unused idx: %d\n", job_item->chip, job_item->engine, job_item->chn, buf_idx);
		return -1;
	}

	pBuf->buffer_index = buf_idx;
	pBuf->dec_yuv_buf_phy = (unsigned char *)dec_data->dec_buf[buf_idx].start_pa;
	pBuf->dec_yuv_buf_size = dec_data->dec_buf[buf_idx].yuv_buf_size;
	pBuf->dec_mbinfo_buf_phy = (unsigned char *)dec_data->dec_buf[buf_idx].mbinfo_pa;
	pBuf->dec_mbinfo_size = dec_data->dec_buf[buf_idx].mbinfo_size;
	pBuf->dec_scale_buf_phy = (unsigned char *)dec_data->dec_buf[buf_idx].scale_pa;
	pBuf->dec_scale_buf_size = dec_data->dec_buf[buf_idx].scale_buf_size;

	h26xd_info("{%d,%d,%d} set_buffer rec %lx(0x%x), mbinfo %lx(0x%x), scale %lx(0x%x)\n", dec_data->chip, dec_data->engine, dec_data->chn,
		   (unsigned long)pBuf->dec_yuv_buf_phy, pBuf->dec_yuv_buf_size, (unsigned long)pBuf->dec_mbinfo_buf_phy, pBuf->dec_mbinfo_size,
		   (unsigned long)pBuf->dec_scale_buf_phy, pBuf->dec_scale_buf_size);

	return 0;
}

/*
 * H265. check start job error
 */
void h26xd_check_error_type(struct h26xd_job_item_t *job_item, struct h26xd_data_t *dec_data, int err_type)
{
	h26xd_countr_t *counter_ptr = counter[job_item->chip] + job_item->chn;
	job_log_item_t *log_item = (job_log_item_t *)job_item->log_item;

	/* handling decode start error */
	if (KDRV_VDODEC_TYPE_H264 == job_item->codec_type) {
		H264Dec_ReinitDRV(dec_data->dec_handle);
	} else {
		/* KDRV_VDODEC_TYPE_H265 */
		H265Dec_TriggerHandler(dec_data->dec_handle);
	}

	SET_JOB_STATUS(job_item, JOB_STATUS_FAIL);
	job_item->err_pos = ERR_TRIG;

	if (EN_VPU_STATUS_TRIGGER_FAIL == err_type) {
		h26xd_info("{%d,%d,%d}, job_id:%u, buf_idx:%d, H26XD_ERR_TRIGGER_ERR! set to JOB_STATUS_FAIL\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, job_item->buf_idx);

		job_item->err_num = H26XD_ERR_TRIGGER_ERR;
		dec_data->dec_buf[job_item->buf_idx].is_released = 1;
		__h26xd_mark_bufidx_as_outputed(dec_data, job_item->buf_idx);
		counter_ptr->trig_fail ++;
		g_dec_err ++;
		log_item->err_num = job_item->err_num;

		/* roll out all DPB buffers */
		__h26xd_output_all_frame(dec_data, 0);
	} else {
		h26xd_info("{%d,%d,%d}, job_id:%u, buf_idx:%d, H26XD_SCAN_HEADER_ERR! set to JOB_STATUS_FAIL \n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, job_item->buf_idx);

		job_item->err_num = H26XD_SCAN_HEADER_ERR;
		dec_data->dec_buf[job_item->buf_idx].is_released = 1;
		__h26xd_mark_bufidx_as_outputed(dec_data, job_item->buf_idx);
		counter_ptr->slice_header_fail ++;
		g_dec_err ++;
		log_item->err_num = job_item->err_num;
	}
}

void h26xd_start_error_handle(struct h26xd_job_item_t *job_item, struct h26xd_data_t *dec_data, unsigned int codec_type)
{
	SET_JOB_STATUS(job_item, JOB_STATUS_FAIL);

	h26xd_info("{%d,%d,%d} job_id:%u connect (%s) fail #0, set to JOB_STATUS_FAIL\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, h26xd_job_status_str(job_item->status));

	if (KDRV_VDODEC_TYPE_H264 == codec_type) {
		H264Dec_ReinitDRV(dec_data->dec_handle);
	}

	__h26xd_mark_bufidx_as_outputed(dec_data, job_item->buf_idx);
	dec_data->dec_buf[job_item->buf_idx].is_released = 1;
}

/*
*   decode process function
*/
STATIC int h26xd_start_job(struct h26xd_job_item_t *job_item, unsigned int codec_type, int ll_buf_idx)
{
	unsigned long flags;
	struct h26xd_data_t *dec_data;
	h26xd_countr_t *counter_ptr;
	int chip, engine, chn;
	int ret;
	H26X_DEC_FRAME_IOCTL *ptdec_frame;
	job_log_item_t *log_item = (job_log_item_t *)job_item->log_item;

	chip = job_item->chip;
	engine = job_item->engine;
	chn = job_item->chn;

	h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);

	dec_data = private_data[chip] + chn;
	counter_ptr = counter[chip] + chn;

	dec_data->chip = chip;
	dec_data->engine = engine;
	dec_data->chn = chn;
	ptdec_frame = &dec_data->dec_frame;
	ptdec_frame->u8ReleaseBufferNum = 0;

	h26xd_get_in_property(dec_data, job_item);

	/* set bitstream buffer address and size to HW */
	h26xd_set_bitstream(dec_data, codec_type);

	/* prepare parameters for H265Dec_OneFrame() */
	h26xd_set_buffer(dec_data, &dec_data->recon_buf, job_item->buf_idx);
	ptdec_frame->ptReconstBuf = &dec_data->recon_buf;
	ptdec_frame->i32BufferIdx = job_item->buf_idx;
	ptdec_frame->u32BgDim = dec_data->dst_bg_dim;
	ptdec_frame->fw_disable = dec_data->fw_disable = job_item->fw_disable;
	ptdec_frame->fw_uv_swap = dec_data->fw_uv_swap = job_item->fw_uv_swap;

	ptdec_frame->extw_sub_yuv_en = dec_data->sub_yuv_en;
	ptdec_frame->extw_sub_yuv_ratio = dec_data->sub_yuv_ratio;
	ptdec_frame->chn_idx = chn;
	ptdec_frame->yuv_width_thrd = dec_data->yuv_width_thrd;

	ptdec_frame->extw_uv_swap = dec_data->extw_uv_swap = job_item->extw_uv_swap;
	ptdec_frame->extw_sce_enabled = dec_data->extw_sce_enabled = job_item->extw_sce_enabled;

	job_item->starttime = jiffies;

	h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);

	if (KDRV_VDODEC_TYPE_H264 == codec_type) {
		H264Dec_ReSetVpu(dec_data->dec_handle, h26xd_vpu_work_buffer[chip][engine][chn]);
		ret = H264Dec_ScanFirstHeader(dec_data->dec_handle);
	} else {
		/* KDRV_VDODEC_TYPE_H265 */
        int chk_cabac_buf_flg;
		int common_buf_idx;
		int cabac_buf_idx;

		common_buf_idx = get_ll_connect_cnt(chip, ll_buf_idx) % 2;
		cabac_buf_idx = (0 == h265_fifo_mode) ? common_buf_idx : 0;

		if (common_buf_idx < 0) {
			ret = -1;
			printk("[DE] common_buf_idx = %d < 0 \n", common_buf_idx);
		} else {
    		H265Dec_ReSetVpu(dec_data->dec_handle, h26xd_vpu_work_buffer[chip][engine][chn]);
    		chk_cabac_buf_flg = H265Dec_SetCabacBuffer(dec_data->dec_handle, h265d_cabac_buffer[chip][cabac_buf_idx]);
            if (chk_cabac_buf_flg == 0)
    		    ret = H265Dec_ScanFirstHeader(dec_data->dec_handle);
            else
                ret = -1;
        }
	}

	if (ret) {
		/* lock */
		h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
		job_item->err_num = H26XD_SCAN_HEADER_ERR;
		job_item->err_pos = ERR_TRIG;
		h26xd_start_error_handle(job_item, dec_data, codec_type);
		counter_ptr->slice_header_fail ++;
		g_dec_err ++;
		log_item->err_num = (char)job_item->err_num;
		/* unlock */
		h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
		printk_ratelimited("[DE] {%d,%d,%d} job_id:%u scan first header error, ret %d, please refer to Novatek_HDAL_Playback_Issues_FAQ_en.doc\n"
            , chip, engine, chn, job_item->job_id, ret);
		return -1;
	}

	switch (codec_type) {
	case KDRV_VDODEC_TYPE_H264:
		/* moved to place behind of H264Dec_OneFrameStart() due to not real width/height from bitstream */
        ret = 0;
		break;
	case KDRV_VDODEC_TYPE_H265:
		H265Dec_SetEXWrite(dec_data, chn);
		ret = H265Dec_SetRecBufferAddr(dec_data->dec_handle, ptdec_frame);
		break;
	}

	if (ret) {
		/* lock */
		h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
		job_item->err_num = H26XD_SET_REC_BUF_ERR;
		job_item->err_pos = ERR_TRIG;
		counter_ptr->set_rec_buf_err ++;
		g_dec_err ++;
		h26xd_start_error_handle(job_item, dec_data, codec_type);
		/* unlock */
		h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);

		log_item->err_num = (char)job_item->err_num;
		return -1;
	}

	if (KDRV_VDODEC_TYPE_H264 == codec_type) {
		set_first_ll_header(&dec_data->dec_handle->stVpuCtx_H264.vpu_list_header, &h26xd_link_list[chip][engine][ll_buf_idx], chn, codec_type);

		/* trigger decoder
		 */
		ret = H264Dec_OneFrameStart(dec_data->dec_handle, ptdec_frame);
		/* lock */
		h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
		if (ret) {
			job_item->err_num = H26XD_ERR_TRIGGER_ERR;
			job_item->err_pos = ERR_TRIG;
			h26xd_check_error_type(job_item, dec_data, EN_VPU_STATUS_TRIGGER_FAIL);
			recover_ll_all(&h26xd_link_list[chip][engine][ll_buf_idx]);
			/* unlock */
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
			counter_ptr->trig_fail ++;
			g_dec_err ++;
			log_item->err_num = (char)job_item->err_num;
			printk_ratelimited("[DE] {%d,%d,%d) job_id:%u 264_dec trigger fail, ret %d\n", chip, engine, chn, job_item->job_id, ret);
			return -1;
		} else {
            counter_ptr->last_hwto_val = H264Dec_GetHwTimeout(dec_data->dec_handle);
            update_final_offset(&h26xd_link_list[chip][engine][ll_buf_idx], chn, codec_type);
		}

		/* Harry */
		dec_data->max_num_ref_frames = dec_data->dec_handle->max_num_ref_frames;

		/* unlock */
		h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
	} else { /* KDRV_VDODEC_TYPE_H265 */
		set_first_ll_header(&dec_data->dec_handle->stVpuCtx_H265.vpu_list_header, &h26xd_link_list[chip][engine][ll_buf_idx], chn, codec_type);

		/* trigger decoder */
		ret = H265Dec_OneFrameStart(dec_data->dec_handle, ptdec_frame); /* check header */
		/* Harry */
		dec_data->max_num_ref_frames = job_item->hdal_max_ref_num;

		/* lock */
		h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
		if (ret) {
			h26xd_check_error_type(job_item, dec_data, ret);
            recover_ll_job_id(&h26xd_link_list[chip][engine][ll_buf_idx]);
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
			printk_ratelimited("[DE] {%d,%d,%d) job_id:%u 265_dec check header fail, ret %d\n", chip, engine, chn, job_item->job_id, ret);
			return -1;
		} else {
			ret = H265Dec_TriggerStart(dec_data->dec_handle, ptdec_frame, BIT1); /* trigger AD */

			if (ret) {
				h26xd_check_error_type(job_item, dec_data, ret);
				recover_ll_all(&h26xd_link_list[chip][engine][ll_buf_idx]);
				/* unlock */
				h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
				printk_ratelimited("[DE] {%d,%d,%d) job_id:%u 265_dec trigger fail, ret %d\n", chip, engine, chn, job_item->job_id, ret);
				return -1;
			} else {
                counter_ptr->last_hwto_val = H265Dec_GetHwTimeout(dec_data->dec_handle);
                update_final_offset(&h26xd_link_list[chip][engine][ll_buf_idx], chn, codec_type);
			}
		}
		/* unlock */
		h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
	}
    log_item->slice_type = (unsigned char)ptdec_frame->slice_type;
    log_item->hw_timeout = counter_ptr->last_hwto_val;

	h26xd_info("{%d,%d,%d) type(%d) ll_idx(%d) job_id:%u connect ok ret %d\n", job_item->chip, job_item->engine, job_item->chn, codec_type, ll_buf_idx, job_item->job_id, ret);

	return 0;
}

/* Used to update job_item state with JOB_STATUS_FLUSH of this channel.
 * function must work in h26x_dec_lock atomic context
 */
STATIC void __h26xd_update_flushed_decbuf(struct h26xd_data_t *dec_data)
{
	unsigned int i;
	struct h26xd_job_item_t *job_item;
	h26xd_countr_t *counter_ptr = counter[dec_data->chip] + dec_data->chn;

	for (i = 0; i < h26xd_max_buf_num; i ++) {
		/* the buf_idx is not processed yet and its job status is JOB_STATUS_FLUSH */
		if (dec_data->dec_buf[i].is_used && !dec_data->dec_buf[i].is_outputed) {
			job_item = dec_data->dec_buf[i].job_item;

			/* If the job is JOB_STATUS_FLUSH, it indicates them were not processed yet.
			 * Thus we must manually change related parameters.
			 */
			if (GET_JOB_STATUS(job_item) == JOB_STATUS_FLUSH) {
				/* Mark this buffer as outputed and can be released as well */
				dec_data->dec_buf[i].is_outputed = 1;
				dec_data->dec_buf[i].is_released = 1;
				h26xd_info("{%d,%d,%d}, flushed_decbuf: buf_idx:%d, update job->id:%u to release!\n", dec_data->chip, dec_data->engine, dec_data->chn, i, job_item->job_id);
			}
		}
		/* sanity check only. Only JOB_STATUS_STANDBY / JOB_STATUS_ONGOING are not well processed done. */
		if (dec_data->dec_buf[i].is_used && !dec_data->dec_buf[i].is_outputed) {
			u32 status = GET_JOB_STATUS(job_item);

			/* JOB_STATUS_ONGOING: in hardware */
			if ((status != JOB_STATUS_STANDBY) && (status != JOB_STATUS_ONGOING)) {
				h26xd_err("Error! {%d,%d,%d}, buf_idx:%d, job_id:%u, status:%u \n",
					  dec_data->chip, dec_data->engine, dec_data->chn, i, job_item->job_id, status);
				counter_ptr->state_machine_err ++;
				g_state_machine_err ++;
			}
		}
	}
	return;
}

/* 1. process channel stop.
 * 2. callback job and release it to KDRV flow
 */
void h26xd_callback_scheduler(int chip)
{
	struct h26xd_job_item_t *job_item, *next;
	struct h26xd_data_t *dec_data;
	h26xd_countr_t *counter_ptr;
	unsigned long flags, bd_flags;
	int chn, buf_idx, need_sch = 0, is_released = 0;
	u32 status;

	//h26xd_info("%s(chip:%d) \n", __func__, chip);

	/*  roll out DPB buffers or flush jobs with state JOB_STATUS_FLUSH */
	for (chn = 0; chn < max_total_cam_ch; chn ++) {
		dec_data = private_data[chip] + chn;

		/* lock */
		h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);

		/* if the channels is stopped, then flush all low level reserved buffers */
		if (atomic_read(&bind_chn_idx[chip][chn].is_closed) == 1) {
			/* check if all jobs of this channel were processed done. It may have buf reserved ... */
			if (vos_list_empty(&dec_data->standby_list) && (dec_data->curr_job == NULL)) {
				/* get all already processed buffers to be ouputed. This function excludes those job with status=JOB_STATUS_FLUSH. */
				__h26xd_output_all_frame(dec_data, 1);
			}
		}
		/* flush all JOB_STATUS_FLUSH jobs of this channel if have */
		__h26xd_update_flushed_decbuf(dec_data);

		/* unlock */
		h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
	}

	/* Here we already have all released buffers with xx.dec_buf[i].is_released=1 from now on.
	 * When the dec_buf is released, the job_item state will be corresponding to be changed as well.
	 */

	/* callback jobs to kflow */
	for (chn = 0; chn < max_total_cam_ch; chn ++) {
		dec_data = private_data[chip] + chn;
		counter_ptr = counter[chip] + chn;

		job_item = NULL;

		if (atomic_read(&bind_chn_idx[chip][chn].is_ready)) {
			h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
			job_item = vos_list_first_entry_or_null(&dec_data->chan_list, struct h26xd_job_item_t, chan_list);
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
		}

		while (job_item != NULL) {
			/* job_item->status and dec_data->dec_buf[buf_idx].is_used works in pair.
			 * ISR may change the status during h26x_dec_lock, orginal code take the dec_data->dec_buf[buf_idx].is_released value as condition check, it may produce timing issue. Thus we use
			 * is_released to replace dec_data->dec_buf[buf_idx].is_used.
			 */
			h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
			buf_idx = job_item->buf_idx;
			status = GET_JOB_STATUS(job_item);
			is_released = dec_data->dec_buf[buf_idx].is_used ? dec_data->dec_buf[buf_idx].is_released : 0;
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);

			/* validation check. only JOB_STATUS_STANDBY and JOB_STATUS_FLUSH may have empty buf_idx */
			if (buf_idx < 0) {
				if ((status & (JOB_STATUS_STANDBY | JOB_STATUS_FLUSH)) == 0) {
					h26xd_err("Error! Job_id:%u ST(0x%x) has illegal buf_idx:%d! \n", job_item->job_id, status, buf_idx);
					counter_ptr->internal_err ++;
					g_internal_err ++;
					damnit("DE");
				}
			} else {
				if (!dec_data->dec_buf[buf_idx].is_used) {
					h26xd_err("Error! Job_id:%u has illegal dec_buf buf_idx! \n", job_item->job_id);
					counter_ptr->state_machine_err ++;
					g_state_machine_err ++;
				}
			}

			/*
			 * Notice: Be careful for buf_idx = -1 case while go through the following code.
			 */

			/* if the job is not done */
			if (!(status & JOB_STATUS_DONE)) {
				//h26xd_info("chn:%d, job_id:%u not job done, status:%d. \n", chn, job_item->job_id, GET_JOB_STATUS(job_item));
				break;
			}

			/* From now on, only JOB_STATUS_FLUSH has buf_idx = -1.
			 */

			/* it is not callback to kflow yet */
			if (job_item->callback_flg == 0) {
				/* reserved this buffer */
				if (status == JOB_STATUS_KEEP) {
					if ((buf_idx < 0) || (buf_idx >= (int)h26xd_max_buf_num)) {
						printk("%s, bug 111! buf_idx = %d. status = KEEP! \n", __func__, buf_idx);
						damnit("DE");
					}

					/* The case that job is stored in DPB list. It will have JOB_STATUS_KEEP and xx.is_released = 0 */
					if (is_released != 0) {
						h26xd_err("Error! Job_id:%u has status JOB_STATUS_KEEP, but buf_idx is set released! \n", job_item->job_id);
						counter_ptr->state_machine_err ++;
						g_state_machine_err ++;
					}

					/* reserve construct buffer */
					__h26xd_job_reserve_buf(job_item);
				}
				decoder_set_callback_job_status(job_item);
				/* callback job to kflow */
				h26xd_cb_func->callback(job_item->callback_data, NULL);
				job_item->callback_flg = 1;
				counter_ptr->callback_cnt ++;
				//h26xd_info("{%d,%d,%d} job_id:%u callback finish (%s), ret:%d\n", job_item->chip, job_item->engine, job_item->chn, job_item->job_id, h26xd_job_status_str(job_item->status), job_item->callback_data->status);
			}

			/* validation check */
			if ((buf_idx >= 0) && (is_released == 1)) {
				if (!(GET_JOB_STATUS(job_item) & (JOB_STATUS_DPB_REL | JOB_STATUS_FLUSH | JOB_STATUS_FAIL))) {
					h26xd_err("Error! Job_id:%u has status:%d , but buf_idx is NOT release! \n", job_item->job_id, GET_JOB_STATUS(job_item));
					counter_ptr->state_machine_err ++;
					g_state_machine_err ++;
				}
			}

			h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
			/* get next item first because job_item is going to be released. */
			next = (job_item->chan_list.next == &dec_data->chan_list) ? NULL : vos_list_next_entry(job_item, chan_list);
			/* release job_item & its buffer if they can be released */
			if (!__h26xd_job_cleanup(job_item)) {
				h26xd_free_buf_idx(dec_data, job_item->buf_idx);
				/* CID 297564 Unused value (UNUSED_VALUE) */
				//coverity[assigned_pointer]: give job_item to NULL to prevent from using again.
				job_item = NULL; /* prevent from using job_item again */
			}

			/* Notice:
			 * From now on, job_item can not be used anymore due to be released already.
			 */

			/* sanity check */
			if ((dec_data->used_buffer_num && vos_list_empty(&dec_data->chan_list)) ||
			    (vos_list_empty(&dec_data->chan_list) && dec_data->used_buffer_num)) {
				h26xd_err("chip:%d,chn:%d, used_buffer_num:%d, list_empty:%d \n", dec_data->chip, dec_data->chn, dec_data->used_buffer_num, vos_list_empty(&dec_data->chan_list));
				counter_ptr->state_machine_err ++;
				g_state_machine_err ++;
			}

			job_item = next;
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
		}; /* while */
	} /* chn */

	/* again, check if any channels are stopping, free bind_chn_idx[] */
	for (chn = 0; chn < max_total_cam_ch; chn ++) {
		int	bClose = 0;

		if (atomic_read(&bind_chn_idx[chip][chn].is_closed) == 0)
			continue;

		dec_data = private_data[chip] + chn;

		h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
		bClose = vos_list_empty(&dec_data->chan_list);
		h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);

		if (bClose == 0) {
			need_sch = 1;
			continue;
		}
		h26xd_spin_lock_irqsave(&h26x_dec_bd_lock, bd_flags); /* lock */
		atomic_set(&bind_chn_idx[chip][chn].is_used, 0);
		atomic_set(&bind_chn_idx[chip][chn].is_closed, 0);
		atomic_set(&bind_chn_idx[chip][chn].is_ready, 0);

		bind_chn_idx[chip][chn].chn_id = 0x0;
		dec_data->codec_type = 0x35670889;
		h26xd_spin_unlock_irqrestore(&h26x_dec_bd_lock, bd_flags); /* unlock */

		h26xd_info("chip:%d,chn:%d bind_chn_idx free. \n", chip, chn);
	}

	if (need_sch) {
		h26xd_trigger_callback(chip);
	}
}

/*
*   LL fire
*/
void h26xd_ll_fire(struct h26xd_data_t *dec_data, int chip, int engine, int ll_buf_idx)
{
	h26xd_countr_t *counter_ptr;
	unsigned int w = dec_data->dec_handle->u16FrameWidth;
	unsigned int h = dec_data->dec_handle->u16FrameHeight;

	if (dec_data->curr_job == NULL) {
		/* degug, sdk need to remove it */
		h26xd_err("%s - ll_idx(%d)\n", __FUNCTION__, ll_buf_idx);
		damnit("DE");
		return ;
	}

	set_engine_busy(chip, engine);
	/* bind engine */
	H26XDec_BindEngine(&h26xd_engine_info[chip][engine], dec_data->chn, ll_buf_idx);

	IMVQ_VDEC_SetLLstartAddr(h26xd_link_list[chip][engine][ll_buf_idx].base_pa, chip);

	if (KDRV_VDODEC_TYPE_H264 == dec_data->codec_type) {
		/* link list connect final job */
		IMVQ_VDEC_ConnectLLJob(&h26xd_link_list[chip][engine][ll_buf_idx], FINIAL_JOB);
		IMVQ_VDEC_TriggerLLfire(0, &h26xd_link_list[chip][engine][ll_buf_idx], chip);
		mark_engine_start(chip, engine, utilization_period);
	} else {
	    if (0 == h265_fifo_mode) {
	        H265Dec_TriggerBin(dec_data->dec_handle);
        }
		/* link list connect final job */
		IMVQ_VDEC_ConnectLLJob(&h26xd_link_list[chip][engine][ll_buf_idx], FINIAL_JOB);
		IMVQ_VDEC_TriggerLLfire(1, &h26xd_link_list[chip][engine][ll_buf_idx], chip);
		mark_engine_start(chip, engine, utilization_period);
	}

	counter_ptr = counter[chip] + dec_data->chn;

	counter_ptr->resolution = (w << 16) | h;

    h26xd_info("{chip:%d,engine:%d} type(%d) ll_idx(%d) fire ok @@\n", chip, engine, dec_data->codec_type, ll_buf_idx);
}

struct h26xd_job_item_t *get_standby_job(int chip_idx, int *type_flag, int *need_callback_flg)
{
	struct h26xd_job_item_t *job_item, *next;
	struct h26xd_data_t *dec_data;
	struct vos_list_head *job_head;
	int	bFound = 0;

    /* type_flag
    * 0x0: pre job is 264 , 0x1: pre job is 265
    * 0x2: non 265 job    , 0x4: non 264 job
    * 0x8: stop selecting current type
    * 0x10: stop selecting all types
    */
    if (*type_flag & 0x2) {
        job_head = &h264d_job_head[chip_idx];
    } else if (*type_flag & 0x4) {
        job_head = &h265d_job_head[chip_idx];
        *type_flag |= 0x1;
    } else if (*type_flag & 0x1) {
        job_head = &h264d_job_head[chip_idx];

		/* keep status and set 264 flag */
		*type_flag &= 0x6;
    } else {
		job_head = &h265d_job_head[chip_idx];
		*type_flag |= 0x1;
    }

	/* get job to connect */
	vos_list_for_each_entry_safe(job_item, next, job_head, job_list) {
		dec_data = private_data[chip_idx] + job_item->chn;
		if (vos_list_empty(&dec_data->chan_list))
			continue;
		if (dec_data->curr_job != NULL) {
			/* current chn is busy */
			continue;
		}

		/* get next standby job and it will be removed from standby_list as well */
		if (GET_JOB_STATUS(job_item) == JOB_STATUS_STANDBY) {
			if (h26xd_get_buf_idx(dec_data, job_item) < 0) {
				*need_callback_flg = 1;
				break;
			}
			bFound = 1;
		}

		/* remove from standby_list */
		vos_list_del_init(&job_item->standby_list);

		/* remove from job_list */
		vos_list_del_init(&job_item->job_list);
		if (bFound)
			break;
	}

	if (!bFound) {
		if (*type_flag & 0x8)
			*type_flag = 0x10;
		else
			*type_flag |= 0x8;
	}

	return bFound ? job_item : NULL;
}

/*
* Parse input property and get buffer from per-channel buffer pool for each job
*/
void h26xd_work_process(int chip_idx)
{
	struct h26xd_job_item_t *job_item = NULL;
	struct h26xd_data_t *dec_data;
	unsigned long flags;
	int select_ll_buf_idx, next_fire_ll_buf_idx, next_chn_idx;
	int need_callback_flg = 0, trigger_flag;
	int engine, engine_start = 0, engine_end = ENTITY_ENGINES;
	int lock_ll_buf_flag = 0;
	int select_count = max_total_cam_ch * h26xd_max_buf_num;	/* the case select_count still be decreased while error pasing */
	int type_flag = 0x1; /* select 264 first */
    int next_can_fire_flg = 0;

	if ((unsigned int)chip_idx >= ENTITY_CHIPS) {
		printk("%s, invalid chip_idx:%d \n", __func__, chip_idx);
		return;
	}

	for (engine = engine_start; engine < engine_end; engine ++) {
		if (!test_ll_buf_idle(&h26xd_link_list[chip_idx][engine][0]) && !test_ll_buf_idle(&h26xd_link_list[chip_idx][engine][1])) {
			break;
		}

		/* lock */
		h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
		/* whether link list could be trigger */
		if (test_engine_idle(chip_idx, engine)) {
			next_chn_idx = select_ll_buf_to_fire(&h26xd_link_list[chip_idx][engine][0], &h26xd_link_list[chip_idx][engine][1], &next_fire_ll_buf_idx);
			if (next_chn_idx >= 0) {
				dec_data = private_data[chip_idx] + next_chn_idx;
				h26xd_ll_fire(dec_data, chip_idx, engine, next_fire_ll_buf_idx);
				h26xd_dbg("FIRE - {%d,%d,%d} #0 ll_list:%d id:%u\n", dec_data->curr_job->chip, dec_data->curr_job->engine, dec_data->curr_job->chn,
					  next_fire_ll_buf_idx, dec_data->curr_job->job_id);
                release_ll_buf_fire_flag(&h26xd_link_list[chip_idx][engine][next_fire_ll_buf_idx]);
			}
		}

        lock_ll_buf_flag = 0;

		while (select_count > 0) {
			select_count --;
			/* check idel ll list */
			if (lock_ll_buf_flag==0 && !test_ll_buf_idle(&h26xd_link_list[chip_idx][engine][0]) && !test_ll_buf_idle(&h26xd_link_list[chip_idx][engine][1])) {
				break;
			}

			if (vos_list_empty(&h265d_job_head[chip_idx])) {
				type_flag |= 0x2;
			}
			if (vos_list_empty(&h264d_job_head[chip_idx])) {
				type_flag |= 0x4;
			}

			if ((type_flag & 0x6) == 0x6) {
				break;
			}

			if ((job_item = get_standby_job(chip_idx, &type_flag, &need_callback_flg)) == NULL) {
				if (type_flag == 0x10)
					break;
				continue;
			} else {
				dec_data = private_data[chip_idx] + job_item->chn;
			}

			/* start to process this job item */
			/* the job_item status is JOB_STATUS_STANDBY. Now moving forward to next stage */
			SET_JOB_STATUS(job_item, JOB_STATUS_ONGOING);
			dec_data->curr_job = job_item;
			dec_data->codec_type = job_item->codec_type;
			job_item->engine = engine; /* mark the job_item is scheduled to run on which engine */

			/* select ll_ist for connecting ll_job */
			if (lock_ll_buf_flag == 0) {
				select_ll_buf_idx = select_ll_buf_and_set_busy(&h26xd_link_list[chip_idx][engine][0], &h26xd_link_list[chip_idx][engine][1], job_item->chn);

                if (select_ll_buf_idx < 0) {
                    /* it should never happen */
                    next_can_fire_flg = 0;
                    printk("{%d,%d,%d} error select_ll_buf_idx = -1\n", job_item->chip, job_item->engine, job_item->chn);
                    damnit("DE");
                    break;
                }

                release_ll_buf_fire_flag(&h26xd_link_list[chip_idx][engine][select_ll_buf_idx]);
				lock_ll_buf_flag = 1;
			}

			h26xd_dbg("WK - {%d,%d,%d} ll_list:%d id:%u\n", job_item->chip, job_item->engine, job_item->chn,
				  select_ll_buf_idx, job_item->job_id);

			/* unlock */
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);

			/* start to process this job */
			trigger_flag = h26xd_start_job(job_item, job_item->codec_type, select_ll_buf_idx);

			/* lock */
			h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);

			if (trigger_flag < 0) {
				dec_data = private_data[chip_idx] + job_item->chn;
				/* connect job failed, restore status to find another job */
				dec_data->curr_job = NULL; /* clear the ongoing job */
				job_item->engine = 0;
				need_callback_flg = 1;
                /* ll_list is empty, it can reselect ll_ist to connect ll_job */
				if (get_ll_connect_cnt(chip_idx, select_ll_buf_idx) == 0) {
					set_ll_buf_idle(&h26xd_link_list[chip_idx][engine][select_ll_buf_idx]);
                    lock_ll_buf_flag = 0;
				}
			} else {
				h26xd_info("{%d,%d,%d} type(%d) ll_idx(%d) job_id:%u connect ok\n", job_item->chip, job_item->engine, job_item->chn, job_item->codec_type, select_ll_buf_idx, job_item->job_id);
				if (test_engine_idle(chip_idx, engine)) {
					h26xd_ll_fire(dec_data, chip_idx, engine, select_ll_buf_idx);
					lock_ll_buf_flag = 0;
					h26xd_dbg("FIRE - {%d,%d,%d} #1 ll_list:%d id:%u\n", job_item->chip, job_item->engine, job_item->chn,
						  select_ll_buf_idx, job_item->job_id);
                    next_can_fire_flg = 0;
				} else {
				    next_can_fire_flg = 1;
				}
			}
		}

        /* connect ll_job is done and ll_fire is inactive this time */
        if (next_can_fire_flg) {
            set_ll_buf_fire_flag(&h26xd_link_list[chip_idx][engine][select_ll_buf_idx]);
        }

		/* unlock */
		h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
	}

	if (need_callback_flg) {
		h26xd_trigger_work(chip_idx);
		h26xd_trigger_callback(chip_idx);
		//h26xd_info("%s, chip:%d, trigger callback and work_process\n", __func__, chip_idx);
	}
}

/*
 *  ISR function
 */
STATIC irqreturn_t h26xd_int_process(int irq, unsigned int chip_idx, unsigned int engine_idx)
{
	DecoderEngInfo *eng_info;
	struct h26xd_data_t *dec_data;
	h26xd_countr_t *counter_ptr;
	struct h26xd_job_item_t *job_item;
	job_log_item_t *log_item;
	unsigned long flags;
	int chn_idx;
	int ret = 0;
	int last_chn_flag = 0;
	int h264_first_chn_flag = 1;
	int h265_first_chn_flag = 1;
	int fail_flag = 0;
	int cur_ll_buf_idx;
	int finish_job_count;
	int h265_bin_st = 0;
    int is_h265_ad = 0;
    int is_codec_header = 0;
    int isr_chn_idx;
    int skip_this_chn_flag = 0;

	h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);  // DISABLE_IRQ_IN_ISR

	eng_info = &h26xd_engine_info[chip_idx][engine_idx];

	/* engine use which channel */
	cur_ll_buf_idx = eng_info->ll_buf_idx;

	finish_job_count = get_ll_connect_cnt(chip_idx, cur_ll_buf_idx);

	/* 1. check current ll status
	 * 2. update record ll table
	 * 3. set last job to skip
	 */
	isr_chn_idx = update_ll_finish_table(&h26xd_link_list[chip_idx][engine_idx][cur_ll_buf_idx], &is_codec_header);

	/* disable mclk auto gating first. Because IP reset always needs clock if necessary */
	pf_h26xd_auto_clk_gate_off(chip_idx);

	while (finish_job_count >= 0) {
        fail_flag = 0;
		chn_idx = get_ll_chn_idx(&h26xd_link_list[chip_idx][engine_idx][cur_ll_buf_idx]);

		if (chn_idx >= max_total_cam_ch || chn_idx < 0) {
			/* degug, sdk need to remove it */
            VPU_ClearInterrupt(1, chip_idx);
            VPU_ClearInterrupt(0, chip_idx);
            damnit("DE");
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags); // DISABLE_IRQ_IN_ISR
			goto isr_ret;
		}

		dec_data = private_data[chip_idx] + chn_idx;
		counter_ptr = counter[chip_idx] + chn_idx;

		job_item = dec_data->curr_job;

		if (NULL == job_item) {
			h26xd_err("{%d,%d,%d} curr job is null\n", chip_idx, engine_idx, chn_idx);
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags); // DISABLE_IRQ_IN_ISR
			counter_ptr->internal_err ++;
			goto isr_ret;
		}

		log_item = (job_log_item_t *)job_item->log_item;
		last_chn_flag = chk_ll_finish_job(&h26xd_link_list[chip_idx][engine_idx][cur_ll_buf_idx]);

		if (KDRV_VDODEC_TYPE_H264 == job_item->codec_type) {
			VPU_AcquireHW(0, chip_idx);
			if (h264_first_chn_flag) {
				ret = H264Dec_Isr(dec_data->dec_handle, 0);
				h264_first_chn_flag = 0;
                if (ret)
                    VPU_ResetHW(0, chip_idx, 1); /* fail case */
                else
                    VPU_ResetHW(0, chip_idx, 0);
			} else {
				ret = H264Dec_Isr(dec_data->dec_handle, 1);
			}

			if (ret) {
				/* error handling */
				SET_JOB_STATUS(job_item, JOB_STATUS_FAIL);
				if (ret > 1) {
					job_item->err_num = H26XD_ERR_HW_TIMEOUT;
				} else {
					job_item->err_num = H26XD_ERR_BIN_ISR_ERR;
				}
				job_item->err_pos = ERR_ISR;
			} else {
				H264Dec_GetOutPutInfo(dec_data->dec_handle, &dec_data->dec_frame);
				SET_JOB_STATUS(job_item, JOB_STATUS_KEEP);
			}
			VPU_ReleaseHW(0, chip_idx);
		} else {
            IMVQ_VDEC_CheckH264Enable(chip_idx);
            VPU_AcquireHW(1, chip_idx);
			if (h265_first_chn_flag) {
				ret = H265Dec_ReceiveIRQ(dec_data->dec_handle, 0, &h265_bin_st);
				h265_first_chn_flag = 0;

                if (ret < 0)
                    is_h265_ad = 1;
                else if (h265_bin_st ||
                    (h265_bin_st == 0 && ret == 0 && h264_first_chn_flag == 1 && is_codec_header == 0))
                    skip_this_chn_flag = 1;

                /* only BIN job case */
				if (is_codec_header && h265_bin_st) {
                    skip_this_chn_flag = 0;

					if (h265_bin_st == 0x2)
						ret = -2;
					else if (h265_bin_st == 0x1)
						ret = -1;
				}
                if (ret)
                    VPU_ResetHW(1, chip_idx, 1); /* fail case */
                else
                    VPU_ResetHW(1, chip_idx, 0);
			} else {
				ret = H265Dec_ReceiveIRQ(dec_data->dec_handle, 1, &h265_bin_st);
			}

            if (skip_this_chn_flag == 0) {
    			if (ret < 0) {
    				/* interrupt by someting error */
    				SET_JOB_STATUS(job_item, JOB_STATUS_FAIL);
    				if (ret == -2)
    					job_item->err_num = H26XD_ERR_HW_TIMEOUT;
    				else
    					job_item->err_num = H26XD_ERR_BIN_ISR_ERR;
    				job_item->err_pos = ERR_ISR;
    				H265Dec_TriggerHandler(dec_data->dec_handle);
    			} else {
    				H265Dec_Sync(dec_data->dec_handle, &dec_data->dec_frame);
    				SET_JOB_STATUS(job_item, JOB_STATUS_KEEP);
    			}
            } else {
                recover_ll_job_data(&h26xd_link_list[chip_idx][engine_idx][cur_ll_buf_idx]);
            }
            VPU_ReleaseHW(1, chip_idx);
        }
		/* frame done or decoding error */

        if (skip_this_chn_flag == 0) {
    		job_item->finishtime = jiffies;
    		log_item->process_time = (unsigned short)(((int)job_item->finishtime - (int)job_item->starttime) & 0xFFFF);
    		log_item->err_num = job_item->err_num;

    		if (job_item->err_num != 0) {
    			if (job_item->err_num == H26XD_ERR_HW_TIMEOUT) {
    				counter_ptr->dec_hwtimeout ++;
    				counter_ptr->record_hw_timeout_cnt ++;
    			} else {
    				counter_ptr->dec_isr_err ++;
    				counter_ptr->record_isr_err_cnt ++;
    			}
    			g_dec_err ++;
    			fail_flag = 1;
    		}

    		dec_data->curr_job = NULL;

    		h26xd_dbg("ISR - {%d,%d,%d} #0 ll_list:%d id:%u, err_num:%d\n", job_item->chip, job_item->engine, job_item->chn,
    			  cur_ll_buf_idx, job_item->job_id, job_item->err_num);

    		/* Note: set bufidx buffer to outputed, it may occur again in __h26xd_process_release_list() below.
    		 * Think the case of nal_ref_idc = 0, the buffer is rolled out immediately.
    		 */
    		__h26xd_mark_bufidx_as_outputed(dec_data, job_item->buf_idx);

    		if (job_item->err_num == 0) {
    			decoder_set_out_property(dec_data, job_item);
    			/* process DPB */
    			__h26xd_process_release_list(dec_data);
    		} else {
    			/* flush all DPB if error */
    			__h26xd_output_all_frame(dec_data, 0);
    		}
        }

        h26xd_info("ISR2 - {%d,%d,%d} h26%s - ll_idx:%d job_id:%u, ret %d, set status %d last_chn_flag %d fail_flag %d skip_this_chn_flag %d\n",
            job_item->chip, job_item->engine, job_item->chn,
            (KDRV_VDODEC_TYPE_H264 == job_item->codec_type) ? "4" : "5",
            cur_ll_buf_idx, job_item->job_id, ret, GET_JOB_STATUS(job_item),
            last_chn_flag, fail_flag, skip_this_chn_flag);

		if (last_chn_flag) {
			if (last_chn_flag == 1) {
				/* AD HW timeout or decode error case */
                IMVQ_VDEC_LLReset(chip_idx);
                re_trigger_ll_job(&h26xd_link_list[chip_idx][engine_idx][cur_ll_buf_idx], is_h265_ad, isr_chn_idx);
				IMVQ_VDEC_TriggerLLfire(1, &h26xd_link_list[chip_idx][engine_idx][cur_ll_buf_idx], chip_idx);
			} else {
				mark_engine_finish(chip_idx, engine_idx, utilization_period);
				reinit_ll(&h26xd_link_list[chip_idx][engine_idx][cur_ll_buf_idx]);
        		h26xd_dbg("reinit_ll - {%d,%d,%d} ll_idx:%d\n", job_item->chip, job_item->engine, job_item->chn,
                    cur_ll_buf_idx);
				/* unbind engine */
				H26XDec_UnbindEngine(&h26xd_engine_info[chip_idx][engine_idx]);
				set_engine_idle(job_item->chip, job_item->engine);
			}
			break;
		}
        skip_this_chn_flag = 0;
		finish_job_count--;
	}

	/* enable mclk auto gating */
	pf_h26xd_auto_clk_gate_on(chip_idx);

	h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags); // DISABLE_IRQ_IN_ISR

	h26xd_trigger_work(chip_idx);
	h26xd_trigger_callback(chip_idx);

isr_ret:
	return IRQ_HANDLED;
}

STATIC irqreturn_t h26xd_int_handler(int irq, void *dev)
{
	irqreturn_t ret;
	unsigned int eng_idx;
	unsigned int chip_idx;
	ktime_t ktime_start, time_diff;
	int time_ms;

	eng_idx = ((uintptr_t)dev) % ENTITY_ENGINES;
	chip_idx = ((uintptr_t)dev) / ENTITY_ENGINES;

	ktime_start = ktime_get();

	ret = h26xd_int_process(irq, chip_idx, eng_idx);

	time_diff = ktime_sub(ktime_get(), ktime_start);
	time_ms = (int)ktime_to_ms(time_diff);

	if (time_ms > 2) {
		if (printk_ratelimit())
			printk("%s, isr execution time:%d ms! \n", __func__, time_ms);
	}

	return ret;
}

/*
* stop all channel and wait until all jobs are done
*/
int h26xd_flush_jobs(int chip_idx)
{
	int chn, wait_cnt = 3;
	int ret = 0;

	for (chn = 0; chn < max_total_cam_ch; chn ++) {
		/* close this channel */
		if (atomic_read(&bind_chn_idx[chip_idx][chn].is_ready))
			h26xdec_close(bind_chn_idx[chip_idx][chn].chn_id);
	}

	do {
		for (chn = 0; chn < max_total_cam_ch; chn ++) {
			ret |= atomic_read(&bind_chn_idx[chip_idx][chn].is_ready);
		}
		if (ret == 0)
			break;
		vos_util_delay_ms(100);
		wait_cnt --;
	} while (wait_cnt > 0);

	if (ret) {
		for (chn = 0; chn < max_total_cam_ch; chn ++) {
			if (atomic_read(&bind_chn_idx[chip_idx][chn].is_ready) == 0)
				continue;

			h26xd_err("{chip:%d,chn:%d} fail to close! \n", chip_idx, chn);
		}
	} else {
		h26xd_info("{chip:%d} all channels close done. \n", chip_idx);
	}

	return ret ? -1 : 0;
}

/* get GIC IRQ number and base address */
int h26xd_remove(struct platform_device *pdev)
{
	int engine_idx, chip_idx;
	uintptr_t data;
	struct device_node *node = pdev->dev.of_node;
    int device_id = -1;
	int irq_num = platform_get_irq(pdev, 0);

	if (of_property_read_u32(node, "device-id", (unsigned int *)&device_id)) {
		printk("[DE] remove - device-id!!!\n");
		device_id = 0;	/* fail case */
	}

	switch (device_id) {
	case 0:
		chip_idx = 0;
		engine_idx = 0;
		break;
	default:
		goto exit;
	}

	data = (uintptr_t)ENGINE_IDX(chip_idx, engine_idx);
	free_irq(irq_num, (void *)data);

	h26xd_info("free irq %d\n", irq_num);

	if (h26xd_engine_info[chip_idx][engine_idx].pu32BaseAddr) {
		iounmap((void __iomem *)h26xd_engine_info[chip_idx][engine_idx].pu32BaseAddr);
		h26xd_engine_info[chip_idx][engine_idx].pu32BaseAddr = NULL;
	}

exit:

	return 0;
}

int h26xd_probe(struct platform_device *pdev)
{
	struct resource *dev_res;
	int engine_idx, chip_idx;
	int irq_num = platform_get_irq(pdev, 0);
	uintptr_t data;
    int device_id = -1;
	struct device_node *node = pdev->dev.of_node;
	extern NVT_API_CHK_DECLARE(h26xdec);

	dev_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (NULL == dev_res) {
		printk("[DE] probe - unknown resource\n");
		goto err_ret;
	}

	if (of_property_read_u32(node, "device-id", (unsigned int *)&device_id)) {
		printk("[DE] probe - device-id!!!\n");
		goto err_ret;
	}

	if (device_id >= ENTITY_CHIPS) {
		printk("[DE] Invalid device_id:%d in dts! max is %d! \n", device_id, ENTITY_CHIPS);
	}

	switch (device_id) {
	case 0:
		chip_idx = 0;
		engine_idx = 0;
		break;
	default:
		printk("[DE] unknown chip_idx:%d\n", device_id);
		goto err_ret;
	}

	//coverity[dead_error_begin]: deadcode. Because in another platform we may have one more chips, thus keeps these statements.
	if (chip_idx != 0) {
		h26xd_max_chip ++;
		if (h26xd_max_chip > ENTITY_CHIPS) {
			printk("%s, h26xd_max_chip:%d is over %d! \n", __func__, h26xd_max_chip, ENTITY_CHIPS);
			goto err_ret;
		}
	}

	NVT_API_CHK_CALL(h26xdec);

	/* mmap phy register address to virtual address space */
	h26xd_engine_info[chip_idx][engine_idx].pu32BaseAddr = ioremap_nocache(dev_res->start, dev_res->end - dev_res->start + 1);

	if (unlikely(!h26xd_engine_info[chip_idx][engine_idx].pu32BaseAddr)) {
		printk("[DE] chip %d failed to ioremap for h26xd register\n", chip_idx);
		goto err_ret;
	}

	data = (uintptr_t)ENGINE_IDX(chip_idx, engine_idx);

#ifdef __KERNEL__
	/* register irq handler */
	if (request_irq(irq_num, h26xd_int_handler, 0, irq_name[chip_idx][engine_idx], (void *)data) != 0) {
		printk("[DE] unable request IRQ %d\n", irq_num);
		goto err_ret;
	}
#else
	/* register irq handler */
	if (platform_rtos_request_irq(irq_num, h26xd_int_handler, 0, irq_name[chip_idx][engine_idx], (void *)data) != 0) {
		printk("[DE] unable request IRQ %d\n", irq_num);
		goto err_ret;
	}
#endif

	if (dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(36))) {
		printk("[DE] set coherent_mask to 36bit fail!\n");
        goto err_ret;
    }

	h26xd_engine_info[chip_idx][engine_idx].irq_num = irq_num;
	//h26xd_info("irq[%d,%d] no = %d\n", 0, engine_idx, h26xd_engine_info[0][engine_idx].irq_num);

	h26xd_engine_info[chip_idx][engine_idx].dev = &pdev->dev;

	return 0;

err_ret:
	h26xd_remove(pdev);

	return -1;
}

#ifdef CONFIG_OF
static const struct of_device_id h26xd_dt_match[] = {
	{ .compatible = "nvt,h26xdec" },
	{},
};
MODULE_DEVICE_TABLE(of, h26xd_dt_match);
#endif

static struct platform_driver h26xd_platform_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = H26X_DEV_NAME,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(h26xd_dt_match),
#endif
	},
	.probe = h26xd_probe,
	.remove = h26xd_remove,
};

/* KDRV global initial / cleanup fuction */
static void __h26xd_kdrv_global_cleanup(void)
{
	int chip;

	for (chip = 0; chip < h26xd_max_chip; chip++) {
		if (private_data[chip]) {
			kfree(private_data[chip]);
		}
		if (bind_chn_idx[chip]) {
			kfree(bind_chn_idx[chip]);
		}
		if (counter[chip]) {
			kfree(counter[chip]);
		}
		if (job_log[chip]) {
			kfree(job_log[chip]);
		}
		if (h264d_small_bs_buffer[chip]) {
			kfree(h264d_small_bs_buffer[chip]);
		}
	}

	if (job_array) {
		kfree(job_array);
	}
	if (job_used) {
		kfree(job_used);
	}
#if INCLUDE_HVYLOAD
	h26xdec_debug_uninit();
#endif
}

static int __h26xd_kdrv_global_init(void)
{
	int chip;

#ifndef CONFIG_NVT_IVOT_PLAT_NA51102
    int ret = 0;
#endif

	/* initial list head */
	for (chip = 0; chip < h26xd_max_chip; chip ++) {
		/* initial decoder data */
		private_data[chip] = kzalloc(sizeof(struct h26xd_data_t) * max_total_cam_ch, GFP_KERNEL);
		if (private_data[chip] == NULL) {
			h26xd_err("error, h26xd failed to allocate private_data memory\n");
			goto err_ret;
		}
		/* bind chn */
		bind_chn_idx[chip] = kzalloc(sizeof(struct bindch_info_t) * max_total_cam_ch, GFP_KERNEL);
		if (bind_chn_idx[chip] == NULL) {
			h26xd_err("error, h26xd failed to allocate bind_chn_idx memory\n");
			goto err_ret;
		}
		/* per channel counter */
		counter[chip] = kzalloc(sizeof(h26xd_countr_t) * max_total_cam_ch, GFP_KERNEL);
		if (counter[chip] == NULL) {
			h26xd_err("error, h26xd failed to allocate counter memory\n");
			goto err_ret;
		}
		/* global log */
		job_log[chip] = kzalloc(sizeof(h26xd_job_log_t), GFP_KERNEL);
		if (job_log[chip] == NULL) {
			printk("error: h26xd failed to allocate decoder job_log memory\n");
			goto err_ret;
		}

		/* actually 16 bytes is enough */
		h264d_small_bs_buffer[chip] = kzalloc(sizeof(char) * 32, GFP_KERNEL);
		if (h264d_small_bs_buffer[chip] == NULL) {
			printk("error: h26xd failed to allocate h264d_small_bs_buffer memory\n");
			goto err_ret;
		}

		/* initial global list */
		VOS_INIT_LIST_HEAD(&h264d_job_head[chip]);
		VOS_INIT_LIST_HEAD(&h265d_job_head[chip]);
	}
	/* keep first service channel */
	memset(&engine_first_serve_ch[0][0], 0, sizeof(engine_first_serve_ch));

#ifndef CONFIG_NVT_IVOT_PLAT_NA51102
	ret = register_printout_notifier(h26xd_log_printout_handler);
	if (ret < 0) {
		h26xd_err("error, H26XD register log system printout notifier failed!\n");
		goto err_ret;
	}
#endif
	/* initial module job */
	max_job_array = max_total_cam_ch * h26xd_max_buf_num * h26xd_max_chip;

	job_array = kzalloc(sizeof(struct h26xd_job_item_t) * max_job_array, GFP_KERNEL);
	job_used = kzalloc(sizeof(int) * max_job_array, GFP_KERNEL);
	if (job_array == NULL || job_used == NULL) {
		h26xd_err("error, h26xd failed to allocate job of module memory\n");
		goto err_ret;
	}

	h26xd_spin_lock_init(&h26x_dec_lock);
	h26xd_spin_lock_init(&h26x_dec_job_lock);
	h26xd_spin_lock_init(&h26x_dec_bd_lock);

#if INCLUDE_HVYLOAD
	h26xdec_debug_init();
#endif
	return 0;

err_ret:
	return -1;
}

/* KDRV engine initial / cleanup fuction */
static void __h26xd_kdrv_engine_cleanup(int chip_idx, int engine_idx)
{
	if (h26xd_engine_info[chip_idx][engine_idx].bs_addr_va) {
		kfree(h26xd_engine_info[chip_idx][engine_idx].bs_addr_va);
		h26xd_engine_info[chip_idx][engine_idx].bs_addr_va = NULL;
	}
	/* release common buffer for all channel use it together */
	/* release per-engine buffers */
	h26xd_release_per_engine_buffer(chip_idx, engine_idx);
}

static int __h26xd_kdrv_engine_init(int chip_idx, int engine_idx)
{
	/* allocate common buffer for all channel use it together */
	/* allocate per-engine buffers */
	if (h26xd_allocate_per_engine_buffer(chip_idx, engine_idx, h26xd_max_width, h26xd_max_height)) {
		printk("[DE] chip %d failed to allocate common buffers\n", chip_idx);
		goto err_ret;
	}

	/* init engine info */
	h26xd_engine_info[chip_idx][engine_idx].chip_idx = chip_idx;
	h26xd_engine_info[chip_idx][engine_idx].engine_idx = engine_idx;
	h26xd_engine_info[chip_idx][engine_idx].ll_buf_idx = -1;
	h26xd_engine_info[chip_idx][engine_idx].dev = NULL;

	return 0;

err_ret:
	return -1;
}

/* KDRV engine initial / cleanup fuction */
static void __h26xd_kdrv_channel_cleanup(int chip_idx, int chn_idx)
{
	/* allocate dec_handel each channel */
	if (private_data[chip_idx][chn_idx].dec_handle) {
		H26XDec_Handler_Release(private_data[chip_idx][chn_idx].dec_handle);
		private_data[chip_idx][chn_idx].dec_handle = NULL;
	}
}

static int __h26xd_kdrv_channel_init(int chip_idx, int chn_idx)
{
	H26XD_DEC_INIT_PARAM stParam;
	unsigned int h264_chn_struct_size;
	unsigned int h265_chn_struct_size;
	unsigned int max_chn_struct_size;

	stParam.pfnMalloc = h26xd_vmalloc;
	stParam.pfnFree = h26xd_vfree;
	stParam.u16MaxWidth = h26xd_max_width;
	stParam.u16MaxHeight = h26xd_max_height;

	private_data[chip_idx][chn_idx].chip = chip_idx;
	private_data[chip_idx][chn_idx].engine = -1;
	private_data[chip_idx][chn_idx].chn = chn_idx;

	h265_chn_struct_size = H265Dec_CheckMemSize();
	h264_chn_struct_size = H264Dec_CheckMemSize();
	max_chn_struct_size = (h265_chn_struct_size < h264_chn_struct_size) ? h264_chn_struct_size : h265_chn_struct_size;

	/* allocate dec_handel each channel */
	private_data[chip_idx][chn_idx].dec_handle = H26XDec_Handler_Create(&stParam, max_chn_struct_size, chip_idx, chn_idx);
	if (private_data[chip_idx][chn_idx].dec_handle == NULL) {
		printk("error: h26xd failed to allocate memory for dec_handle\n");
		goto err_ret;
	}

	return 0;

err_ret:
	return -1;

}

/*
* KDRV driver module cleanup
*/
void h26xd_kdrv_cleanup(int fail_flag)
{
	int chip;
	int engine;
	int chn;
	int ret;

	for (chip = 0; chip < h26xd_max_chip; chip++) {
		ret = h26xd_flush_jobs(chip);
		if (ret) {
			h26xd_err("chip:%d wait unfinished job timeout \n", chip);
		}

		if (0 == fail_flag) {
			/* closed HW engine */
			IMVQ_VDEC_CloseHW(chip);
		}
		/* clock off and module reset ON */
		pf_h26xd_clk_off(chip);
	}

	/* closed proc */
	h26xd_proc_close();

	/* unregister driver */
	platform_driver_unregister(&h26xd_platform_driver);

	for (chip = 0; chip < h26xd_max_chip; chip++) {
		for (engine = 0; engine < ENTITY_ENGINES; engine++) {
			__h26xd_kdrv_engine_cleanup(chip, engine);
		}
	}

	for (chip = 0; chip < h26xd_max_chip; chip++) {
		if (private_data[chip]) {
			for (chn = 0; chn < max_total_cam_ch; chn++) {
				__h26xd_kdrv_channel_cleanup(chip, chn);
			}
		}
	}

	if (h26xd_vmalloc_cnt != h26xd_vfree_cnt) {
		printk("[DE] vmalloc/vfree error detected: %d / %d\n", h26xd_vmalloc_cnt, h26xd_vfree_cnt);
	}

	for (chip = 0; chip < h26xd_max_chip; chip++) {
		/* cleanup tasklet */
		h26xd_exit_tasklet(chip);
	}

	__h26xd_kdrv_global_cleanup();

	/* recover to default value, don't change. It increases when new chip is probed. */
	h26xd_max_chip = 1;

}

/*
* check module parameter
*/
static void h26xd_chk_module_param(void)
{
	if (h26xd_max_chip > ENTITY_CHIPS) {
		h26xd_err("h26xd_max_chip: %d is over %d, force to %d \n", h26xd_max_chip, ENTITY_CHIPS, ENTITY_CHIPS);
		h26xd_max_chip = ENTITY_CHIPS;
	}

	if (max_total_cam_ch <= 0 || max_total_cam_ch > ENTITY_MINORS) {
		h26xd_err("max_total_cam_ch(%d) is out of range [%d , %d], force to %d\n", max_total_cam_ch, 1, ENTITY_MINORS, ENTITY_MINORS);
		max_total_cam_ch = ENTITY_MINORS;
	} else {
		int tmp_total_ch = max_total_cam_ch + 2;
		max_total_cam_ch = (tmp_total_ch > ENTITY_MINORS) ? ENTITY_MINORS : tmp_total_ch;
	}

	if (h265_max_support_vps <= 0 || h265_max_support_vps > MAX_VPS_NUM) {
		h26xd_err("h265_max_support_vps(%u) is out of range [%d , %d], force to 1\n", h265_max_support_vps, 1, MAX_VPS_NUM);
		h265_max_support_vps = 1;
	}

	if (h265_max_support_sps <= 0 || h265_max_support_sps > MAX_SPS_NUM) {
		h26xd_err("h265_max_support_sps(%u) is out of range [%d , %d], force to 1\n", h265_max_support_sps, 1, MAX_SPS_NUM);
		h265_max_support_sps = 1;
	}

	if (h265_max_support_pps <= 0 || h265_max_support_pps > MAX_PPS_NUM) {
		h26xd_err("h265_max_support_pps(%u) is out of range [%d , %d], force to 1\n", h265_max_support_pps, 1, MAX_PPS_NUM);
		h265_max_support_pps = 1;
	}

	if (h265_max_support_st <= 0 || h265_max_support_st > MAX_ST_RPS_SIZE) {
		h26xd_err("h265_max_support_st(%u) is out of range [%d , %d], force to 4\n", h265_max_support_st, 1, MAX_ST_RPS_SIZE);
		h265_max_support_st = 4;
	}

	if (h264_max_support_sps <= 0 || h264_max_support_sps > H264DEC_MAXSPS) {
		h26xd_err("h264_max_support_sps(%u) is out of range [%d , %d], force to 1\n", h264_max_support_sps, 1, H264DEC_MAXSPS);
		h264_max_support_sps = 1;
	}

	if (h264_max_support_pps <= 0 || h264_max_support_pps > H264DEC_MAXPPS) {
		h26xd_err("h264_max_support_pps(%u) is out of range [%d , %d], force to 1\n", h264_max_support_pps, 1, H264DEC_MAXPPS);
		h264_max_support_pps = 1;
	}

	if (h26xd_max_width < 64 || h26xd_max_width > MAX_HEVC_WIDTH) {
		h26xd_err("h26xd_max_width(%d) is out of range [%d , %d], force to 64\n", h26xd_max_width, 64, MAX_HEVC_WIDTH);
		h26xd_max_width = 64;
	}

	if (h26xd_max_height < 64 || h26xd_max_height > MAX_HEVC_HEIGHT) {
		h26xd_err("h26xd_max_height(%d) is out of range [%d , %d], force to 64\n", h26xd_max_height, 64, MAX_HEVC_HEIGHT);
		h26xd_max_height = 64;
	}

	/* check h26xd_max_buf_num */
	if (h26xd_max_buf_num > MAX_FRAME_BUFFER_SIZE || h26xd_max_buf_num < 3) {
		h26xd_err("h26xd_max_buf_num(%d) is out of range [3, %d], force to %d\n", h26xd_max_buf_num, MAX_FRAME_BUFFER_SIZE, MAX_FRAME_BUFFER_SIZE);
		h26xd_max_buf_num = MAX_FRAME_BUFFER_SIZE;
	}
}

/*
* KDRV driver module init
*/
int h26xd_kdrv_init(void)
{
	int chip;
	int engine;
	int chn;
	int ret;
	int ll_list;

	h26xd_chk_module_param();
	memset(h26xd_engine_info, 0, sizeof(h26xd_engine_info));

	ret = __h26xd_kdrv_global_init();
	if (ret < 0) {
		goto err_ret;
	}

	/* init proc */
	if (h26xd_proc_init() < 0)
		goto err_ret;

	/* register driver */
	if (platform_driver_register(&h26xd_platform_driver)) {
		printk("[DE] Failed to register platform driver '%s'\n", h26xd_platform_driver.driver.name);
		goto err_ret;
	}

	for (chip = 0; chip < h26xd_max_chip; chip++) {
		/* stop reset and enable clock */
		ret = pf_h26xd_clk_on(chip);
		if (ret) {
			printk("[DE] error: failed to enable h26xd clock and stop reset\n");
			goto err_ret;
		}

		for (chn = 0; chn < max_total_cam_ch; chn++) {
			ret = __h26xd_kdrv_channel_init(chip, chn);
			if (ret < 0) {
				goto err_ret;
			}
		}
	}

	/* calculate 5ms as the base */
    real_pclk_rate = pf_h26xd_get_plck(0);
	hw_timeout_delay = real_pclk_rate / 40;		//25ms
	printk("[DE] hw_timeout_delay:%d ticks, plck:%d \n", hw_timeout_delay, real_pclk_rate);

	for (chip = 0; chip < h26xd_max_chip; chip++) {
		for (engine = 0; engine < ENTITY_ENGINES; engine++) {
			ret = __h26xd_kdrv_engine_init(chip, engine);
			if (ret < 0) {
				goto err_ret;
			}
		}
	}

	/* FIXME : it should adjust hevc_hw.c to support multi-engine in future */
	for (chip = 0; chip < h26xd_max_chip; chip++) {
		for (engine = 0; engine < ENTITY_ENGINES; engine++) {
			IMVQ_VDEC_OpenHW((uintptr_t)h26xd_engine_info[chip][engine].pu32BaseAddr, chip);
			H265Dec_SetBaseAddr((uintptr_t)h26xd_engine_info[chip][engine].pu32BaseAddr, chip);
			H264Dec_SetBaseAddr((uintptr_t)h26xd_engine_info[chip][engine].pu32BaseAddr, chip);
		}
	}

	/*
	 *  setup buffer of vpu struct
	 *  Note : only one common buffer each engine
	 */
	for (chip = 0; chip < h26xd_max_chip; chip++) {
		for (engine = 0; engine < ENTITY_ENGINES; engine++) {
			for (ll_list = 0; ll_list < 2; ll_list++) {
				init_ll_job(&h26xd_link_list[chip][engine][ll_list],
					    h26xd_link_list_buffer[chip][engine][ll_list].addr_va,
					    h26xd_link_list_buffer[chip][engine][ll_list].addr_pa,
					    ll_list, chip);
			}
            /* for first time */
			pf_h26xd_xclk_off(chip);
		}
		/* initial tasklet */
		h26xd_init_tasklet(chip);
	}

	return 0;

err_ret:
	h26xd_kdrv_cleanup(1);

	return -EFAULT;
}

void h26xd_trigger_callback(int chip_idx)
{
	if (callback_wq) {
		schedule_work(&h26xd_worker[chip_idx].worker);
	} else {
		tasklet_schedule(&h26xd_cb_tasklet[chip_idx]);
	}
}

void h26xd_trigger_work(int chip_idx)
{
	tasklet_schedule(&h26xd_wk_tasklet[chip_idx]);
}

void h26xd_callback_job(unsigned long data)
{
	h26xd_callback_scheduler((int)data);
}

void h26xd_work_job(unsigned long data)
{
	h26xd_work_process((int)data);
}

static void h26xd_callback_worker(struct work_struct *work)
{
	struct h26xd_worker_s *worker = vos_container_of(work, struct h26xd_worker_s, worker);

	h26xd_callback_job((unsigned long)worker->chip_idx);
}

int h26xd_exit_tasklet(int chip_idx)
{
	tasklet_kill(&h26xd_cb_tasklet[chip_idx]);
	tasklet_kill(&h26xd_wk_tasklet[chip_idx]);
	return 0;
}

int h26xd_init_tasklet(int chip_idx)
{
	tasklet_init(&h26xd_cb_tasklet[chip_idx], h26xd_callback_job, (unsigned long)chip_idx);
	tasklet_init(&h26xd_wk_tasklet[chip_idx], h26xd_work_job, (unsigned long)chip_idx);

	/* init worker for callback debug due to save stream. */
	INIT_WORK(&h26xd_worker[chip_idx].worker, h26xd_callback_worker);
	h26xd_worker[chip_idx].chip_idx = chip_idx;

	return 0;
}

