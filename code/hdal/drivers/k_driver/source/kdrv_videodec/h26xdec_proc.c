#include <linux/version.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/of.h>
#include "h26xdec_flow.h"
#include "h26xdec_dbg.h"
#include "h26xdec_platform.h"
#include "vpu/hevc_dec_entity.h"
#include <kdrv_videodec.h>
#include <kdrv_videodec_lmt.h>

#include "kwrap/util.h"

#define ENTITY_PROC_NAME "kdrv_h26xd"

static struct proc_dir_entry *h26xd_entry_proc = NULL;
static struct proc_dir_entry *utilproc = NULL;
static struct proc_dir_entry *jobproc = NULL;
static struct proc_dir_entry *dbgmodeproc = NULL;
static struct proc_dir_entry *levelproc = NULL;
static struct proc_dir_entry *posproc = NULL;
static struct proc_dir_entry *paramproc = NULL;
static struct proc_dir_entry *modparamproc = NULL;
static struct proc_dir_entry *localbufparamproc = NULL;
static struct proc_dir_entry *h26xd_wakeup_proc = NULL;
static struct proc_dir_entry *h26xd_counter_proc = NULL;
static struct proc_dir_entry *h26xd_joblog_proc = NULL;
static struct proc_dir_entry *h26xd_perf_proc = NULL;
static struct proc_dir_entry *h26xd_sw_autogate_proc = NULL;

extern struct bindch_info_t *bind_chn_idx[ENTITY_CHIPS];
extern struct h26xd_data_t *private_data[ENTITY_CHIPS];
extern h26xd_countr_t *counter[ENTITY_CHIPS];
extern h26xd_job_log_t *job_log[ENTITY_CHIPS];
extern int h26xd_max_chip;
extern int max_total_cam_ch;
extern unsigned int lose_pic_handle_flags;
extern unsigned int utilization_period;
extern struct utilization_record_t eng_util[ENTITY_CHIPS][ENTITY_ENGINES];
extern int h264_only_support_single_slice_flag;
extern int h265_only_support_single_slice_flag;
extern unsigned int resv_total_out_cnt;
extern unsigned int free_total_out_cnt;
extern unsigned int resv_cur_cnt[ENTITY_CHIPS][ENTITY_MINORS];
extern int h26xd_dump_err_bs_byte;
extern int disable_mv_wo;
extern unsigned int hw_timeout_delay;
extern vk_spinlock_t h26x_dec_lock;
extern unsigned int h26xd_hw_gate;;

static int job_minor = -1; /* value < 0 means all*/

#define PG_ALIGN(n) ((n + 0xFFF) & 0xFFFFF000) //PAGE_ALIGN// align n to a multiple of page size (4K)

static int h26xd_proc_util_show(struct seq_file *sfile, void *v)
{
	int i, j;

	if (utilization_period == 0) {
		seq_printf(sfile, "HW utilization measurement is disabled. set measurment period to non-zero to enable it.\n");
		return 0;
	}
	for (j = 0; j < h26xd_max_chip; j++) {
		for (i = 0; i < ENTITY_ENGINES; i++) {
			if (eng_util[j][i].utilization_record == 0) {
				seq_printf(sfile, "Chip%d Engine%d HW Utilization Period=%d(sec) Utilization=N/A\n", j, i, utilization_period);
			} else {
				unsigned int uti_num = eng_util[j][i].utilization_record / 100;
				unsigned int uti_den = eng_util[j][i].utilization_record % 100;
				seq_printf(sfile, "Chip%d Engine%d HW Utilization Period=%d(sec) Utilization=%d.%d\n", j, i, utilization_period, uti_num, uti_den);
			}
		}
	}
	return 0;
}

static ssize_t h26xd_proc_util_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int val;
	char value_str[16] = { '\0' };
	int len = count;

	if (len > sizeof(value_str) - 1) {
		len = sizeof(value_str) - 1;
	}

	if (copy_from_user(value_str, buffer, len))
		return -EFAULT;

	sscanf(value_str, "%d", &val);
	utilization_period = val;
	printk("\nUtilization Period =%d(sec)\n", utilization_period);

	return count;
}

static int h26xd_proc_job_show(struct seq_file *sfile, void *v)
{
	struct h26xd_job_item_t *job_item, *next;
	int total_job_cnt = 0;
	int ch_job_cnt = 0;
	int chip_idx, chn;
	int i, j;

	seq_printf(sfile, "\nMinor=%d (<0 means all) System ticks=0x%x\n", job_minor, (int)jiffies & 0xffff);
	seq_printf(sfile, "-----------------------------------\n");
	seq_printf(sfile, "total - reserved buffer %u free buffer %u\n", resv_total_out_cnt, free_total_out_cnt);
	seq_printf(sfile, "current - {is_used, is_closed, bind_chn_id, res_buf, max_ref_num}\n");
	if (job_minor >= 0 && job_minor < ENTITY_MINORS) {
		for (j = 0; j < h26xd_max_chip; j++) {
			seq_printf(sfile, "chip %d ch[%d] %u\n", j, job_minor, resv_cur_cnt[j][job_minor]);
		}
	} else {
		for (j = 0; j < h26xd_max_chip; j++) {
			struct h26xd_data_t *dec_data;

			seq_printf(sfile, "chip %d", j);
			for (i = 0; i < max_total_cam_ch; i++) {
				dec_data = private_data[j] + i;

				if (i % 4 == 0)
					seq_printf(sfile, "\n");

				seq_printf(sfile, "bd_ch[%d] {%d , %d, %x, %u, %d}  ", i, atomic_read(&bind_chn_idx[j][i].is_used), atomic_read(&bind_chn_idx[j][i].is_closed), bind_chn_idx[j][i].chn_id, resv_cur_cnt[j][i], dec_data->max_num_ref_frames);
			}
			seq_printf(sfile, "\n");
		}
	}

	seq_printf(sfile, "-----------------------------------\n");
	for (i = 0; i < h26xd_max_chip; i++) {
		for (j = 0; j < ENTITY_ENGINES; j ++) {
			seq_printf(sfile, "chip:%d,engine:%d idle: %d \n", i, j, test_engine_idle(i, j));
		}
	}
	seq_printf(sfile, "\n\n");
	seq_printf(sfile, "Chip Eng Chn Job_ID         Status Puttime Start  End    cb resvd buf_idx standby cur_job codec\n");
	seq_printf(sfile, "===============================================================================================\n");

	for (chip_idx = 0; chip_idx < h26xd_max_chip; chip_idx++) {
		for (chn = 0; chn < max_total_cam_ch; chn ++) {
			struct h26xd_data_t *dec_data;

			if (atomic_read(&bind_chn_idx[chip_idx][chn].is_used) == 0)
				continue;

			dec_data = private_data[chip_idx] + chn;

			vos_list_for_each_entry_safe(job_item, next, &dec_data->chan_list, chan_list) {
				total_job_cnt++;
				if (job_minor >= 0 && job_item->chn != job_minor) {
					continue; /* skip */
				}

				seq_printf(sfile, "%-4d %-3d %-3d %-13u %-6s 0x%-5x 0x%-4x 0x%-4x %-2d %-5d %-7d %-8s %-7d %-5s\n",
					   job_item->chip, job_item->engine, job_item->chn, job_item->job_id,
					   h26xd_job_status_long_str(job_item->status), job_item->puttime & 0xffff,
					   (int)job_item->starttime & 0xffff, (int)job_item->finishtime & 0xffff,
					   job_item->callback_flg, job_item->res_flg, job_item->buf_idx,
					   vos_list_empty(&dec_data->standby_list) ? "empty" : "no_empty", (dec_data->curr_job != NULL),
					   (job_item->codec_type == KDRV_VDODEC_TYPE_H264) ? "h264" : "h265");

				ch_job_cnt++;
			}
		}
	}

	if (job_minor >= 0) {
		seq_printf(sfile, "ch %d job number: %d\n", job_minor, ch_job_cnt);
	}
	seq_printf(sfile, "total job number: %d\n", total_job_cnt);

	seq_printf(sfile, "Usage: echo <chn> > /proc/videograph/h26xd/job\n");
	seq_printf(sfile, "       <chn>: channel index. (<0 means all channel)\n");

	return 0;
}


static ssize_t h26xd_proc_job_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int job_minor_val = 0;
	char value_str[16] = { '\0' };
	int len = count;

	if (len > sizeof(value_str) - 1) {
		len = sizeof(value_str) - 1;
	}

	if (copy_from_user(value_str, buffer, len))
		return -EFAULT;

	sscanf(value_str, "%d", &job_minor_val);
	job_minor = job_minor_val;

	printk("\nchannel=%d (<0 means all)\n", job_minor);

	return count;
}

static int h26xd_proc_dbgmode_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "Usage: echo [dbg_cmd] > /proc/videograph/h26xd/dbgmode\n");
	seq_printf(sfile, "Debug mode(printm): %d (0:disabled 1:enabled)\n", h26xd_dbg_mode);

	return 0;
}

static ssize_t h26xd_proc_dbgmode_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int level;
	char value_str[128] = { '\0' };
	int len = count;

	if (len > sizeof(value_str) - 1) {
		len = sizeof(value_str) - 1;
	}

	if (copy_from_user(value_str, buffer, len))
		return -EFAULT;

	sscanf(value_str, "%d", &level);
	h26xd_dbg_mode = level;
	printk("\nDebug mode = %d (0:disabled >0:enabled)\n", h26xd_dbg_mode);

	return count;
}

static int h26xd_proc_level_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\nLog level = %d (0:error, 1:warning, 2:info)\n", h26xd_dbglevel);
	return 0;
}

static ssize_t h26xd_proc_level_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int level;
	char value_str[16] = { '\0' };
	int len = count;

	if (len > sizeof(value_str) - 1) {
		len = sizeof(value_str) - 1;
	}

	if (copy_from_user(value_str, buffer, len))
		return -EFAULT;

	sscanf(value_str, "%d", &level);

	h26xd_dbglevel = level;
	printk("\nLog level =%d (0:error, 1:warning, 2:info)\n", h26xd_dbglevel);

	return count;
}

static int h26xd_proc_pos_show(struct seq_file *sfile, void *v)
{
    seq_printf(sfile, "\nBIT0:H265_VPS\tBIT1:H265_NAL\tBIT2:H265_ENTITY\n");
    seq_printf(sfile, "BIT3:H265_TOOL\tBIT4:H265_RESMGR\tBIT5:H265_HW\n");
    seq_printf(sfile, "BIT6:H264_VPS\tBIT7:H264_NAL\tBIT8:H264_ENTITY\n");
    seq_printf(sfile, "BIT9:H264_PICBUF\tBIT10:H264_DRV\tBIT11:H264_VLD\tBIT12:H264_HW\n");
	seq_printf(sfile, "\nPrint Log position = %d\n", h26xd_dbgpos);
	return 0;
}

static ssize_t h26xd_proc_pos_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int postion;
	char value_str[16] = { '\0' };
	int len = count;

	if (len > sizeof(value_str) - 1) {
		len = sizeof(value_str) - 1;
	}

	if (copy_from_user(value_str, buffer, len))
		return -EFAULT;

	sscanf(value_str, "%d", &postion);

	h26xd_dbgpos = postion;
	printk("\nLog postion =%d\n", h26xd_dbgpos);

	return count;
}

static int h26xd_proc_mod_param_show(struct seq_file *sfile, void *v)
{
	extern unsigned int h265_max_support_vps;
	extern unsigned int h265_max_support_sps;
	extern unsigned int h265_max_support_pps;
	extern unsigned int h265_max_support_st;
	extern unsigned int h264_max_support_sps;
	extern unsigned int h264_max_support_pps;
	extern unsigned int h26xd_max_width;
	extern unsigned int h26xd_max_height;
	extern char h26xd_ver_str[128];
    extern unsigned int h265_fifo_mode;

	seq_printf(sfile, "%s\n", h26xd_ver_str);
	seq_printf(sfile, "\n== module parameter ==\n");
	seq_printf(sfile, "max_total_cam_ch = %d\n", max_total_cam_ch);
	seq_printf(sfile, "h265_max_support_vps = %u\n", h265_max_support_vps);
	seq_printf(sfile, "h265_max_support_sps = %u\n", h265_max_support_sps);
	seq_printf(sfile, "h265_max_support_pps = %u\n", h265_max_support_pps);
	seq_printf(sfile, "h265_max_support_st  = %u\n", h265_max_support_st);
	seq_printf(sfile, "h264_max_support_sps = %u\n", h264_max_support_sps);
	seq_printf(sfile, "h264_max_support_pps = %u\n", h264_max_support_pps);
	seq_printf(sfile, "max_width = %u\n", h26xd_max_width);
	seq_printf(sfile, "max_height = %u\n", h26xd_max_height);
    seq_printf(sfile, "h265_fifo_mode : %s\n", (h265_fifo_mode != 0) ? "Enable" : "Disable");

	return 0;
}

static ssize_t h26xd_proc_mod_param_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	return 0;
}

static int h26xd_proc_local_buf_param_show(struct seq_file *sfile, void *v)
{
	extern struct buffer_info_t h265d_cabac_buffer[ENTITY_CHIPS][2];
	extern struct buffer_info_t h26xd_link_list_buffer[ENTITY_CHIPS][ENTITY_ENGINES][2];
	int chip, eng, ll_idx;

	seq_printf(sfile, "chip      base(pa)          base(va)         size\n");
	seq_printf(sfile, "====  ================  ================  ==========\n");
	seq_printf(sfile, "hevc_cabac_local_buffer\n");
	/* show cabac local buffer */
	for (chip = 0; chip < ENTITY_CHIPS; chip++) {
		seq_printf(sfile, "%3d   %lx  %lx  0x%x\n%3d   %lx  %lx  0x%x\n", chip,
			   (unsigned long)h265d_cabac_buffer[chip][0].addr_pa, (unsigned long)h265d_cabac_buffer[chip][0].addr_va, h265d_cabac_buffer[chip][0].size,
			   chip, (unsigned long)h265d_cabac_buffer[chip][1].addr_pa, (unsigned long)h265d_cabac_buffer[chip][1].addr_va, h265d_cabac_buffer[chip][1].size);

		for (eng = 0; eng < ENTITY_ENGINES; eng ++) {
            seq_printf(sfile, "\nlink_list_buffer (eng:%d)\n", eng);
			for (ll_idx = 0; ll_idx < 2; ll_idx ++) {
				seq_printf(sfile, "%3d   %lx  %lx  0x%x\n", chip,
					   (unsigned long)h26xd_link_list_buffer[chip][eng][ll_idx].addr_pa,
					   (unsigned long)h26xd_link_list_buffer[chip][eng][ll_idx].addr_va,
					   h26xd_link_list_buffer[chip][eng][ll_idx].size);
			}
		}
	}

	seq_printf(sfile, "\n");
	for (chip = 0; chip < ENTITY_CHIPS; chip++) {
		seq_printf(sfile, "private_data(chip:%d): vaddr:%lx, size:0x%zx \n",
			   chip, (unsigned long)private_data[chip], (sizeof(*private_data[chip]) * max_total_cam_ch));
		seq_printf(sfile, "bind_chn_idx(chip:%d): vaddr:%lx, size:0x%zx \n",
			   chip, (unsigned long)bind_chn_idx[chip], (sizeof(*bind_chn_idx[chip]) * max_total_cam_ch));
		seq_printf(sfile, "counter(chip:%d): vaddr:%lx, size:0x%zx \n",
			   chip, (unsigned long)counter[chip], (sizeof(*counter[chip]) * max_total_cam_ch));
	}

	return 0;
}

static ssize_t h26xd_proc_local_buf_param_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	return 0;
}

/* Wakeup proc.
 */
static int h26xd_wakeup_proc_param_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "echo <chip_idx 0~> <0:trigger_work / 1:trigger_callback> > wakeup \n");

	return 0;
}

static int h26xd_wakeup_proc_param_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_wakeup_proc_param_show, PDE_DATA(inode));
}

static ssize_t h26xd_wakeup_proc_param_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	int len = size, chip_idx = 0, tmp = 0;
	unsigned char value[60];
	extern void h26xd_trigger_work(int chip_idx);
	extern void h26xd_trigger_callback(int chip_idx);

	if (copy_from_user(value, buf, len))
		return 0;

	value[len] = '\0';
	sscanf(value, "%d %d\n", &chip_idx, &tmp);

	if ((unsigned int)chip_idx >= h26xd_max_chip) {
		printk("chip_idx:%d is out of range! \n", chip_idx);
		return size;
	}

	if (tmp == 0) {
		h26xd_trigger_work(chip_idx);
		printk("chip_idx: %d, h26xd_trigger_work \n", chip_idx);
	} else {
		h26xd_trigger_callback(chip_idx);
		printk("chip_idx: %d, h26xd_trigger_callback \n", chip_idx);
	}

	return size;
}

/* Counter
 */
static int h26xd_counter_proc_param_show(struct seq_file *sfile, void *v)
{
	int	chip_idx, chn;
	extern unsigned int h26xd_err_count_thd;
	extern unsigned int g_state_machine_err;
	extern unsigned int g_internal_err;
	extern unsigned int g_dec_err;

	seq_printf(sfile, "h26xd_err_count_thd:%u state_machine_err:%u, internal_err:%u, dec_err:%u\n", h26xd_err_count_thd, g_state_machine_err, g_internal_err, g_dec_err);
	seq_printf(sfile, "-----------------------------------------------------\n");
	for (chip_idx = 0; chip_idx < h26xd_max_chip; chip_idx ++) {
		for (chn = 0; chn < max_total_cam_ch; chn ++) {
			h26xd_countr_t *counter_ptr;

			counter_ptr = counter[chip_idx] + chn;

			seq_printf(sfile, "chip:%d, chn_id:0x%x(active:%d), chn:%d \n", chip_idx, bind_chn_idx[chip_idx][chn].chn_id, atomic_read(&bind_chn_idx[chip_idx][chn].is_used), chn);
			seq_printf(sfile, "	fd:0x%x, codec_type: %s, resoultuon:%ux%u, last_hwto_val:0x%x\n", counter_ptr->fd,
				   (counter_ptr->codec_type == BIT1) ? "H264" : ((counter_ptr->codec_type == BIT2) ? "H265" : "NA"),
				   (counter_ptr->resolution >> 16), (counter_ptr->resolution & 0xFFFF), counter_ptr->last_hwto_val);
			seq_printf(sfile, "	trig_fail: %u \n", counter_ptr->trig_fail);
			seq_printf(sfile, "	slice_header_fail: %u \n", counter_ptr->slice_header_fail);
			seq_printf(sfile, "	state_machine_err: %u \n", counter_ptr->state_machine_err);
			seq_printf(sfile, "	internal_err: %u \n", counter_ptr->internal_err);
			seq_printf(sfile, "	set_rec_buf_err: %u \n", counter_ptr->set_rec_buf_err);
			seq_printf(sfile, "	no_bufidx: %u \n", counter_ptr->no_bufidx);
			seq_printf(sfile, "	dec_isr_err: %u \n", counter_ptr->dec_isr_err);
			seq_printf(sfile, "	dec_hwtimeout: %u \n", counter_ptr->dec_hwtimeout);
			seq_printf(sfile, "	putjob_cnt: %u \n", counter_ptr->putjob_cnt);
			seq_printf(sfile, "	callback_cnt: %u \n", counter_ptr->callback_cnt);
			seq_printf(sfile, "	address_error: %u \n", counter_ptr->address_err);
			seq_printf(sfile, "--------------------------------\n");
		}
	}

	return 0;
}

static int h26xd_counter_proc_param_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_counter_proc_param_show, PDE_DATA(inode));
}

/* joglog
 */
int log_chip = 0, log_chan = -1;
static int h26xd_joblog_proc_param_show(struct seq_file *sfile, void *v)
{
	int	chip, chn, i, dist, idx;
	h26xd_job_log_t	*log_item_chan;
	job_log_item_t *log_item;
	u32	head, tail;
	unsigned long flags;
	char *frame_type[] = {"Uknown", "I-frme", "P-frme", "B-frme", "Others"}; /* dec_frame_slice_t */

	seq_printf(sfile, "syntax: echo [chip 0~] [chan 0~, -1:all] > joblog \n");
	seq_printf(sfile, "syntax: echo -1 -1 > joblog to clear log database. (log_chip:%d,log_chan:%d) \n\n", log_chip, log_chan);
	seq_printf(sfile, "bs_len    f_type resolution job_id    status hwto_val   hw(ms) chn\n");

	for (chip = 0; chip < h26xd_max_chip; chip ++) {
		if (chip != log_chip)
			continue;

		log_item_chan = job_log[chip];
		h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
		head = log_item_chan->head;
		tail = log_item_chan->tail;
		h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
		dist = (int)head - (int)tail;
		if (dist == 0)
			continue;

		for (chn = 0; chn < max_total_cam_ch; chn ++) {
			/* only list the specific channel if log_chan != -1 (all) */
			if ((log_chan != -1) && (log_chan != chn))
				continue;

			seq_printf(sfile, "--------- ------ ---------- --------- ------ ---------- ------ ---\n");
			for (i = 0; i < dist; i ++) {
				idx = (tail + i) & (MAX_JOB_LOG_ITEM - 1);
				log_item = &log_item_chan->log_item[idx];

				/* filer the specific channel chn */
				if (log_item->chn != chn)
					continue;

				seq_printf(sfile, "%-9u %-6s %5dx%-4d %-9u %-6d 0x%-8x %-6d %-3d\n",
					   log_item->bs_len, frame_type[log_item->slice_type], log_item->width, log_item->height, log_item->job_id,
					   log_item->err_num, log_item->hw_timeout, log_item->process_time, log_item->chn);
			}
		}
	}

	return 0;
}

static int h26xd_joblog_proc_param_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_joblog_proc_param_show, PDE_DATA(inode));
}

static ssize_t h26xd_joblog_proc_param_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	int len = size, chip;
	h26xd_job_log_t	*log_item_chan;
	unsigned char value[60];
	unsigned long flags;

	if (copy_from_user(value, buf, len))
		return 0;

	value[len] = '\0';
	sscanf(value, "%d %d\n", &log_chip, &log_chan);

	printk("log_chip = %d, log_chan = %d\n", log_chip, log_chan);

	if ((log_chip == -1) && (log_chan == -1)) {
		h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
		for (chip = 0; chip < h26xd_max_chip; chip ++) {
			log_item_chan = job_log[chip];
			log_item_chan->head = log_item_chan->tail = 0;
		}
		h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
		printk("log buffer is cleared! \n");
		log_chip = 0;
		return size;
	}

	if ((unsigned int)log_chip >= h26xd_max_chip) {
		printk("chip_idx:%d is out of range! \n", log_chip);
		log_chip = 0;
		return size;
	}

	return size;
}

/* SW auto clk gating */
static int h26xd_proc_auto_gate_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "%d\n", h26xd_hw_gate);
	return 0;
}

static ssize_t h26xd_proc_auto_gate_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    unsigned int level;
	int chip_idx;
	char value_str[128] = { '\0' };
	int len = count;

	if (len > sizeof(value_str) - 1){
		len = sizeof(value_str) - 1;
	}

	if (copy_from_user(value_str, buffer, len))
		return -EFAULT;

	sscanf(value_str, "%u", &level);

    h26xd_hw_gate = (level > 0) ? 1 : 0;

	if (0 == h26xd_hw_gate) {
		for (chip_idx = 0; chip_idx < h26xd_max_chip; chip_idx++) {
            /* disable hw clk auto gating */
            pf_h26xd_auto_clk_gate_off(chip_idx);
		}
	} else {
		for (chip_idx = 0; chip_idx < h26xd_max_chip; chip_idx++) {
            /* enable hw clk auto gating */
            pf_h26xd_auto_clk_gate_on(chip_idx);
		}
	}

	return count;
}

/* Perf
 */
static int h26xd_proc_perf_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "echo <bsFile> <lenFile> <num_of_rounds> <exc_total_chn> <codec_type> <queue_job_num> > perf \n");
	seq_printf(sfile, "default:exc_total_chn(8) codec_type(0:264) queue_job_num(10)\n");

	return 0;
}

static int h26xd_proc_perf_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_perf_show, PDE_DATA(inode));
}

/* perf related APIs
 */
KDRV_CALLBACK_FUNC	func;
#define TOTAL_CH	16
#define ARRAY_SZ	512
#define DEC_BUF_CNT	3
volatile unsigned int perf_jobs, perf_pidx[TOTAL_CH], perf_cidx[TOTAL_CH];

typedef struct {
	void *bs_start_va;
	uintptr_t bs_base;
	unsigned int *sz_array;	//array
	int frames_per_round;	//how many pictures in a file or like GOP
	int round;				//how many rounds are testing
	uintptr_t dec_bin_out_buf_pa;
	/* decout buff */
	void *dec_out_start_va[TOTAL_CH];
	uintptr_t dec_buf_array[TOTAL_CH][DEC_BUF_CNT];	//paddr
	int dec_buf_cnt;
	int dec_buf_sz;
} perf_data_t;

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
static INT32 h26xd_reserve_buf(uintptr_t phy_addr, VOID *data) {return 0; }
static INT32 h26xd_free_buf(uintptr_t phy_addr, VOID *data) {return 0;}
#else
static INT32 h26xd_reserve_buf(uintptr_t phy_addr, int ddr_id, unsigned int job_id) {return 0;}
static INT32 h26xd_free_buf(uintptr_t phy_addr, int ddr_id, unsigned int job_id) {return 0;}
#endif /* CONFIG_NVT_IVOT_PLAT_NA51102 */


/*-1 means EOF */
static int h26xd_get_bslen(struct file *bslen_fp, unsigned char *str, int str_len, unsigned int *bslen)
{
	unsigned char ch;
	int	index = 0, pos = 0, tmp = 0;
	char integer[20];
	long val;

	while (str_len -- > 0) {
		ch = *str ++;
		if (ch < 0x30 || ch > 0x39) {
			if (pos) {
				tmp += kstrtol(integer, 10, &val);
				bslen[index ++] = val;
				bslen[index] = 0;
			}
			pos = 0;
			continue;
		}
		integer[pos ++] = ch;
		integer[pos] = 0;	/* terminated */
	}
	/* last one */
	if (pos) {
		val = 0; /* init val */
		tmp += kstrtol(integer, 10, &val);
		bslen[index ++] = val;
		bslen[index] = 0;
	}
	return tmp ? 0 : index;
}

static void h26xd_load_bs(struct file *bs_fp, void *bs_start_va, unsigned int *sz_array, int total_frames)
{
	void *bs_end_va = bs_start_va;
	int	i, ret;

	bs_fp->f_pos = 0;

	for (i = 0; i < total_frames; i ++) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
		if (1) {
			mm_segment_t old_fs;

			old_fs = get_fs();
			set_fs(get_ds());
			ret = vfs_read(bs_fp, (void *)bs_end_va, (size_t)sz_array[i], &bs_fp->f_pos);
			set_fs(old_fs);
		}
#else
		ret = kernel_read(bs_fp, (void *)bs_end_va, (size_t)sz_array[i], &bs_fp->f_pos);
#endif

		bs_end_va += PG_ALIGN(sz_array[i]);
	}
}

static INT32 h26xd_callback(VOID *callback_info, VOID *user_data)
{
	KDRV_VDODEC_H26X_CB_DATA *cb_data = (KDRV_VDODEC_H26X_CB_DATA *)callback_info;
	int chan = cb_data->fd;

	perf_jobs --;
	perf_cidx[chan] ++;	/* increase callback index */

	if (cb_data->status != KDRV_OK)
		printk("%s, chan:%d, cb_data->status = %d \n", __func__, chan, cb_data->status);

	return 0;
}

static int h26xd_start_perf(perf_data_t *data, int codec_type, int queue_job_num, int exc_total_chn)
{
	int	ret = 0, total_jobs[TOTAL_CH], frame_idx[TOTAL_CH], frames_per_round, chan;
	INT32 handle[TOTAL_CH];
	KDRV_VDODEC_H26X_DATA job;
	uintptr_t bs_buf_paddr[TOTAL_CH];
	void *bs_start_va[TOTAL_CH] = {NULL};
	int	bContinue = 0;

	/* clear to zero */
	memset(&total_jobs[0], 0, sizeof(total_jobs));
	memset(&frame_idx[0], 0, sizeof(frame_idx));
	memset(&handle[0], 0, sizeof(handle));
	memset(&bs_buf_paddr[0], 0, sizeof(bs_buf_paddr));
	memset(&job, 0, sizeof(job));

	perf_jobs = data->frames_per_round * data->round * exc_total_chn;
	frames_per_round = data->frames_per_round;

	/* can't use local memory because low level function not keeps the function pointer. Please see H26XD_SET_CB_FUNC
	 */
	func.reserve_buf = h26xd_reserve_buf;
	func.free_buf = h26xd_free_buf;
	func.callback = h26xd_callback;

	for (chan = 0; chan < exc_total_chn; chan ++) {
		handle[chan] = (codec_type == 0) ?
            kdrv_videodec_h26x_open(0, KDRV_VIDEODEC_ENGINE_H264) :
            kdrv_videodec_h26x_open(0, KDRV_VIDEODEC_ENGINE_H265);
		perf_pidx[chan] = perf_cidx[chan] = 0;	/* reset put/callback index */
		total_jobs[chan] = 0;
	}

	if (kdrv_videodec_h26x_set(handle[chan], H26XD_SET_CB_FUNC, &func) != KDRV_OK) {
		printk("%s(#%d) call H26XD_SET_CB_FUNC fail! \n", __func__, __LINE__);
		return -1;
	}

	memset(&job, 0, sizeof(KDRV_VDODEC_H26X_DATA));

	while (1) {
		bContinue = 0;
		for (chan = 0; chan < exc_total_chn; chan ++) {
			if (total_jobs[chan] >= (data->frames_per_round * data->round))
				continue;
			if ((total_jobs[chan] % frames_per_round) == 0) {
				/* reset */
				frame_idx[chan] = 0;
				bs_buf_paddr[chan] = data->bs_base;
				bs_start_va[chan] = data->bs_start_va;
			}
			bContinue = 1;
			job.param.codec_type = (codec_type == 0) ?
                KDRV_VDODEC_TYPE_H264 : KDRV_VDODEC_TYPE_H265;
			job.param.bs_size = data->sz_array[frame_idx[chan]];
			job.param.hdal_max_ref_num = 1;
			job.param.fd = chan;	//any value
			job.param.frame_id = frame_idx[chan];
			job.param.out_frame_buffer.ddr_id = 0;
			job.param.out_frame_buffer.addr_pa = data->dec_buf_array[chan][total_jobs[chan] % data->dec_buf_cnt];
			job.param.out_frame_buffer.size = 0x400000;

			job.param.mbinfo_buffer.ddr_id = 0;
			job.param.mbinfo_buffer.addr_pa = job.param.out_frame_buffer.addr_pa + 0x400000;
			job.param.mbinfo_buffer.size = 0x100000;

			job.param.in_bs_buffer.ddr_id = 0;
			job.param.in_bs_buffer.addr_pa = bs_buf_paddr[chan];
			job.param.in_bs_buffer.addr_va = (uintptr_t)bs_start_va[chan];
			job.param.in_bs_buffer.size = data->sz_array[frame_idx[chan]];

			ret = kdrv_videodec_h26x_set(handle[chan], H26XD_SET_INIT, &job);
			if (ret != KDRV_OK) {
				printk("Error in calling kdrv_videodec_h26x_set()! \n");
				goto exit;
			}

			ret = kdrv_videodec_h26x_trigger(handle[chan], &job);
			if (ret != KDRV_OK) {
				printk("Error in calling kdrv_videodec_h26x_trigger()! \n");
				goto exit;
			}

			/* move to next base */
			bs_buf_paddr[chan] += PG_ALIGN(data->sz_array[frame_idx[chan]]);
			bs_start_va[chan] += PG_ALIGN(data->sz_array[frame_idx[chan]]);
			frame_idx[chan] ++;
			perf_pidx[chan] ++;
			/* prevent driver from not having enough job_item to store jobs */
			while (((int)perf_pidx[chan] - (int)perf_cidx[chan]) > queue_job_num) {
				 vos_util_delay_ms(1);
			}
			total_jobs[chan] ++;
		}

		if (bContinue == 0)
			break;
	}

	while (perf_jobs != 0)
		vos_util_delay_ms(1);

exit:
	for (chan = 0; chan < exc_total_chn; chan ++) {
		ret |= kdrv_videodec_h26x_close(handle[chan]);
	}

	return ret;
}

/* rec + mbinfo */
#define DEC_BUF_SZ  ((0x400000) + (0x100000))
#define DEC_BIN_OUT_BUF_SZ (0x800000)
static ssize_t h26xd_proc_perf_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    KDRV_VDODEC_H265_CABAC_BUF binout_buf_info;
	unsigned char *value = NULL, *bs_name, *bslen_name;
	unsigned int *array, *sz_array;
	int i, len = size, round = 0;
	struct file *bs_fp = NULL, *bslen_fp = NULL;
	struct device_node *node;
	struct property *prop;
	uintptr_t	hdal_base, bs_end;
	int ret, length, hdal_size, total_sz = 0;
	unsigned int total_frames = 0, test_frames = 0xffff;
	void *bs_start_va = NULL;
	perf_data_t	data;
	ktime_t ktime_start, time_diff;
	int time_ms, chan;
    int codec_type = 0;
    int queue_job_num = 10;
    int exc_total_chn = 8;

	value = (void *)vmalloc(4096);
	array = (void *)value + ARRAY_SZ;
	if (value == NULL) {
		printk("%s, Null memory! \n", __func__);
		return 0;
	}
	sz_array = (void *)array + ARRAY_SZ;
	bs_name = (void *)sz_array + ARRAY_SZ;
	bslen_name = (void *)bs_name + ARRAY_SZ;

	if (copy_from_user(value, buf, len)) {
		vfree(value);
		return 0;
	}
	node = of_find_node_by_name(NULL, "hdal-memory");
	if (node == NULL) {
		printk("Can't find hdal-memory in dtsi! \n");
		goto exit;
	}
	node = of_find_node_by_name(node, "media");
	prop = of_find_property(node, "reg", &length);

	if (!prop || (length % 24) || (ARRAY_SZ < length)) {
		printk("error length = %d \n", length);
		goto exit;
	}
	/* only parsing DDR0 */
	if (of_property_read_u32_array(node, "reg", (u32 *)&array[0], length / 4)) {
		printk("error length_4 = %d \n", length);
		goto exit;
	}

	hdal_base = array[3];
	hdal_size = array[5];

	value[len] = '\0';
	sscanf(value, "%s %s %d %d %d %d %d\n", &bs_name[0], &bslen_name[0], &round, &exc_total_chn, &codec_type, &queue_job_num, &test_frames);
	printk("bs_name:%s, bs_len:%s, round:%d, exc_chn_num:%d, codec_type:%d, queue_job_num:%d\n", bs_name, bslen_name, round, exc_total_chn, codec_type, queue_job_num);

    if (exc_total_chn > TOTAL_CH) {
        printk("exc_total_chn(%d) is over TOTAL_CH(%d)\n", exc_total_chn, TOTAL_CH);
		goto exit;
    }

    bs_fp = filp_open(bs_name, O_RDONLY, 0777);
	if (IS_ERR(bs_fp)) {
		printk("bs:%s not exists! \n", bs_name);
		bs_fp = NULL;
		goto exit;
	}
	bslen_fp = filp_open(bslen_name, O_RDONLY, 0777);
	if (IS_ERR(bslen_fp)) {
		printk("bslen:%s not exists! \n", bslen_name);
		bslen_fp = NULL;
		goto exit;
	}

	if (round == 0)
		goto exit;

	/* calculate frame count of bs */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	if (1) {
		mm_segment_t old_fs;

		old_fs = get_fs();
		set_fs(get_ds());
		bslen_fp->f_pos = 0;
		ret = vfs_read(bslen_fp, (void *)array, (size_t)ARRAY_SZ, &bslen_fp->f_pos);

		set_fs(old_fs);
	}
#else
	ret = kernel_read(bslen_fp, (void *)array, (size_t)ARRAY_SZ, &bslen_fp->f_pos);
#endif
	if (ret == -EFAULT) {
		printk("read bslen fail! \n");
		goto exit;
	}
	total_frames = h26xd_get_bslen(bslen_fp, (unsigned char *)array, ret, sz_array);

	/* ioremap size */
	for (i = 0; i < total_frames; i ++) {
		total_sz += PG_ALIGN(sz_array[i]);
	}
	bs_end = PG_ALIGN(hdal_base + total_sz);

	total_sz += DEC_BUF_CNT * DEC_BUF_SZ * exc_total_chn;

	bs_start_va = (void *)ioremap_cache(hdal_base, total_sz);
	if (bs_start_va == NULL) {
		printk("ioremap_wc return NULL! hdal_base = 0x%lx, size = 0x%x \n", (unsigned long)hdal_base, total_sz);
		goto exit;
	}

	/* load bs */
	h26xd_load_bs(bs_fp, bs_start_va, sz_array, total_frames);

	if ((bs_end + DEC_BUF_SZ * DEC_BUF_CNT * exc_total_chn) > (hdal_base + hdal_size)) {
		printk("bs_end:0x%lx exceeds 0x%lx \n", (unsigned long)(bs_end + DEC_BUF_SZ * DEC_BUF_CNT * exc_total_chn), (unsigned long)(hdal_base + hdal_size));
		goto exit;
	}
	vos_cpu_dcache_sync((VOS_ADDR)bs_start_va, total_sz, VOS_DMA_BIDIRECTIONAL);

	memset(&data, 0, sizeof(data));

    if (codec_type != 0) {
        data.dec_bin_out_buf_pa = PG_ALIGN(bs_end);
        bs_end += DEC_BIN_OUT_BUF_SZ;

        binout_buf_info.cabac_buffer.addr_pa = data.dec_bin_out_buf_pa;
        binout_buf_info.cabac_buffer.size = DEC_BIN_OUT_BUF_SZ;
        binout_buf_info.chip_id = 0;
        binout_buf_info.ddr_no = 0;
		//coverity[check_return]:
        ret = kdrv_videodec_h26x_set(0, H265D_SET_CABAC_BUF, &binout_buf_info);
        if (ret < 0) {
        	printk("%s, fail to give cabac buffer! \n", __func__);
        	goto exit;
        }
    }

	//paddr
	for (chan = 0; chan < exc_total_chn; chan ++) {
		for (i = 0; i < DEC_BUF_CNT; i ++) {
			data.dec_buf_array[chan][i] = bs_end + DEC_BUF_SZ * i + (DEC_BUF_SZ * DEC_BUF_CNT * chan);
		}
	}

	data.bs_start_va = bs_start_va;
	data.bs_base = hdal_base;
	data.sz_array = sz_array;

	data.frames_per_round = (total_frames > test_frames) ? test_frames : total_frames;
	data.round = round;
	data.dec_buf_cnt = DEC_BUF_CNT;
	data.dec_buf_sz = DEC_BUF_SZ;

	ktime_start = ktime_get();
	/* start to test performance */
	if (h26xd_start_perf(&data, codec_type, queue_job_num, exc_total_chn))
		goto exit;
	time_diff = ktime_sub(ktime_get(), ktime_start);
	time_ms = (int)ktime_to_ms(time_diff);
	if (time_ms == 0)
		time_ms = 1;

	printk("Total frames:%d, time=%d ms, fps:%d \n", data.round * data.frames_per_round, time_ms, (int)((exc_total_chn * data.round * data.frames_per_round * 1000) / time_ms));

    if (codec_type != 0) {
        if (kdrv_videodec_h26x_set(0, H265D_RELEASE_CABAC_BUF, &binout_buf_info) != KDRV_OK) {
        	printk("%s(%d) call H265D_RELEASE_CABAC_BUF fail! \n", __func__, __LINE__);
        	goto exit;
        }
    }

exit:
	if (value)
		vfree(value);
	if (bs_fp)
		filp_close(bs_fp, NULL);
	if (bslen_fp)
		filp_close(bslen_fp, NULL);
	if (bs_start_va)
		iounmap(bs_start_va);

	return size;
}
/*
* start of type/structure for param proc node
*/
#define UINT32_PROC  0
#define INT32_PROC   1
#define UINT8_PROC   2
#define INT8_PROC    3
#define HEX8_PROC    4
#define STR_T_PROC   5

typedef struct mapping_st {
	char *tokenName;
	void *value;
	int type;
	/* 0: uint32_t, 1: int32_t, 2: uint8_t, 3: int8_t, 4: hex, 5: char array
	* unimplemented: 5: double, 6: 4 integer, 7: int array
	*/
	int lb;
	int ub;
	char *note;
} MapInfo;

static const MapInfo syntax[] = {
	{ "LosePicFlag",          &lose_pic_handle_flags,      UINT32_PROC, 0, 1, "lose picture handling flags - 0: force to decode. 1: jump probably incorrect frame." },
	{ "HWTimeout", &hw_timeout_delay, UINT32_PROC, 0, 134217728, "hw timeout cycle count. 0: disalbe." },
	{ "H264OnlySingleSlice", &h264_only_support_single_slice_flag, INT32_PROC, 0, 1, "h264 only parsing first slice to speed up sw parsing." },
	{ "H265OnlySingleSlice", &h265_only_support_single_slice_flag, INT32_PROC, 0, 1, "h265 only parsing first slice to speed up sw parsing." },
	{ "DisMvWo", &disable_mv_wo, INT32_PROC, 0, 1, "h264 force to disable mv data write out." },



	{ NULL, NULL, 0, 0, 0, NULL }
};

static int h26xd_proc_param_show(struct seq_file *sfile, void *v)
{
	int i = 0;
	unsigned int *u32Val;
	int *i32Val;
	unsigned char *u8Val;
	signed char *i8Val;
	char *str_ptr;

	seq_printf(sfile, "Usage: echo [parameter name] [value] > /proc/videograph/h26xd/param\n");
	seq_printf(sfile, "\n");
	seq_printf(sfile, "                                range\n");
	seq_printf(sfile, "    parameter name       value   /size             note\n");
	seq_printf(sfile, "======================= ======= ====== ==============================\n");
	while (syntax[i].tokenName) {
		seq_printf(sfile, "%-23s ", syntax[i].tokenName);
		switch (syntax[i].type) {
		case UINT32_PROC:
			u32Val = (unsigned int *)syntax[i].value;
			seq_printf(sfile, "%7u", *u32Val);
			seq_printf(sfile, " %d~%3d", syntax[i].lb, syntax[i].ub);
			break;
		case INT32_PROC:
			i32Val = (int *)syntax[i].value;
			seq_printf(sfile, "%7d", *i32Val);
			seq_printf(sfile, " %d~%3d", syntax[i].lb, syntax[i].ub);
			break;
		case UINT8_PROC:
			u8Val = (unsigned char *)syntax[i].value;
			seq_printf(sfile, "%7u", *u8Val);
			seq_printf(sfile, " %d~%3d", syntax[i].lb, syntax[i].ub);
			break;
		case HEX8_PROC:
			u8Val = (unsigned char *)syntax[i].value;
			seq_printf(sfile, "0x%07X", *u8Val);
			seq_printf(sfile, " %d~%3d", syntax[i].lb, syntax[i].ub);
			break;
		case INT8_PROC:
			i8Val = (char *)syntax[i].value;
			seq_printf(sfile, "%7d", *i8Val);
			seq_printf(sfile, " %d~%3d", syntax[i].lb, syntax[i].ub);
			break;
		case STR_T_PROC:
			str_ptr = (char *)syntax[i].value;
			seq_printf(sfile, "\"%s\"", str_ptr);
			seq_printf(sfile, " %6d", syntax[i].ub);
			break;
		default:
			break;
		}
		if (syntax[i].note)
			seq_printf(sfile, "   %s", syntax[i].note);
		seq_printf(sfile, "\n");
		i++;
	}

	return 0;
}

static ssize_t h26xd_proc_param_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int i;
	int len = count;
	int value;
	char cmd_str[40] = { 0 };
	char str[80];
	int idx;

	unsigned int *u32Val;
	int *i32Val;
	unsigned char *u8Val;
	signed char *i8Val;
	char *str_ptr;
	char *param_str_ptr;
	int param_str_size;
	int str_buf_size;

	int change_variable = 0;

	if (copy_from_user(str, buffer, len))
		return -EFAULT;

	if (len > sizeof(str) - 1) {
		len = sizeof(str) - 1;
	}
	str[len] = '\0';

	sscanf(str, "%s %d\n", cmd_str, &value);

	idx = 0;
	while (syntax[idx].tokenName) {
		if (strcmp(syntax[idx].tokenName, cmd_str) == 0) {
			break;
		}
		idx++;
	}

	printk("idx:%d\n", idx);

	if (syntax[idx].tokenName == NULL) {
		printk("unknown \"%s\"", cmd_str);
		goto err_ret;
	}

	/* check range/size */
	switch (syntax[idx].type) {
	case UINT32_PROC:
	case INT32_PROC:
	case UINT8_PROC:
	case INT8_PROC:
		if (value < syntax[idx].lb || value > syntax[idx].ub) {
			printk("%s(%d) is out of range! (%d ~ %d)\n", syntax[idx].tokenName, value, syntax[idx].lb, syntax[idx].ub);
			goto err_ret;
		}
		break;

	case STR_T_PROC:
		/* check latter */
		break;
	}

	/* get value */
	switch (syntax[idx].type) {
	case UINT32_PROC:
		u32Val = (unsigned int *)syntax[idx].value;
		if (*u32Val != value)
			change_variable = 1;
		*u32Val = value;
		break;
	case INT32_PROC:
		i32Val = (int *)syntax[idx].value;
		if (*i32Val != value)
			change_variable = 1;
		*i32Val = value;
		break;
	case UINT8_PROC:
	case HEX8_PROC:
		u8Val = (unsigned char *)syntax[idx].value;
		if (*u8Val != value)
			change_variable = 1;
		*u8Val = value;
		break;
	case INT8_PROC:
		i8Val = (char *)syntax[idx].value;
		if (*i8Val != value)
			change_variable = 1;
		*i8Val = value;
		break;
	case STR_T_PROC:
		str_ptr = (char *)syntax[idx].value;
		str_buf_size = syntax[idx].ub;

		param_str_ptr = str + strlen(syntax[idx].tokenName) + 1;
		param_str_size = strlen(param_str_ptr);

		if (param_str_size >= str_buf_size) {
			printk("%s(%d) is out of range! (%d)\n", syntax[idx].tokenName, param_str_size, str_buf_size);
			goto err_ret;
		}

		if (strncmp(str_ptr, param_str_ptr, param_str_size) != 0) {
			change_variable = 1;
		}

		strncpy(str_ptr, param_str_ptr, param_str_size);
		str_ptr[param_str_size] = 0;

		/* remove the trailing char: '\n' */
		for (i = 0; i < param_str_size; i++) {
			if (str_ptr[i] == '\n') {
				str_ptr[i] = 0;
			}
			if (str_ptr[i] == 0) {
				break;
			}
		}
		break;
	default:
		break;
	}

err_ret:
	return count;
}

static int h26xd_proc_util_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_util_show, PDE_DATA(inode));
}

static int h26xd_proc_job_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_job_show, PDE_DATA(inode));
}

static int h26xd_proc_dbgmode_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_dbgmode_show, PDE_DATA(inode));
}

static int h26xd_proc_level_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_level_show, PDE_DATA(inode));
}

static int h26xd_proc_pos_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_pos_show, PDE_DATA(inode));
}

static int h26xd_proc_param_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_param_show, PDE_DATA(inode));
}

static int h26xd_proc_mod_param_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_mod_param_show, PDE_DATA(inode));
}

static int h26xd_proc_local_buf_param_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_local_buf_param_show, PDE_DATA(inode));
}

static int h26xd_proc_auto_gate_open(struct inode *inode, struct file *file)
{
	return single_open(file, h26xd_proc_auto_gate_show, PDE_DATA(inode));
}

static struct file_operations h26xd_proc_util_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_util_open,
	.write = h26xd_proc_util_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_proc_job_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_job_open,
	.write = h26xd_proc_job_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_proc_dbgmode_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_dbgmode_open,
	.write = h26xd_proc_dbgmode_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_proc_level_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_level_open,
	.write = h26xd_proc_level_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_proc_pos_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_pos_open,
	.write = h26xd_proc_pos_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_proc_param_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_param_open,
	.write = h26xd_proc_param_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_proc_mod_param_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_mod_param_open,
	.write = h26xd_proc_mod_param_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_proc_local_buf_param_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_local_buf_param_open,
	.write = h26xd_proc_local_buf_param_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_wakeup_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_wakeup_proc_param_open,
	.write = h26xd_wakeup_proc_param_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_counter_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_counter_proc_param_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_joblog_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_joblog_proc_param_open,
	.write = h26xd_joblog_proc_param_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_proc_perf_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_perf_open,
	.write = h26xd_proc_perf_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations h26xd_proc_auto_gate_ops = {
	.owner = THIS_MODULE,
	.open = h26xd_proc_auto_gate_open,
	.write = h26xd_proc_auto_gate_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

void h26xd_proc_close(void)
{
	if (utilproc != 0)
		proc_remove(utilproc);
	if (jobproc != 0)
		proc_remove(jobproc);
	if (dbgmodeproc)
		proc_remove(dbgmodeproc);
	if (levelproc != 0)
		proc_remove(levelproc);
	if (posproc != 0)
		proc_remove(posproc);
	if (paramproc)
		proc_remove(paramproc);
	if (modparamproc)
		proc_remove(modparamproc);
	if (localbufparamproc)
		proc_remove(localbufparamproc);
	if (h26xd_wakeup_proc)
		proc_remove(h26xd_wakeup_proc);
	if (h26xd_counter_proc)
		proc_remove(h26xd_counter_proc);
	if (h26xd_joblog_proc)
		proc_remove(h26xd_joblog_proc);
	if (h26xd_perf_proc)
		proc_remove(h26xd_perf_proc);
	if (h26xd_sw_autogate_proc)
		proc_remove(h26xd_sw_autogate_proc);
	if (h26xd_entry_proc != 0)
		proc_remove(h26xd_entry_proc); /* NOTE: can not use h26xd_entry_proc->name */
}

int h26xd_proc_init(void)
{
	h26xd_entry_proc = proc_mkdir(ENTITY_PROC_NAME, NULL);
	if (NULL == h26xd_entry_proc) {
		printk("Error to create driver proc, please insert log.ko first.\n");
		goto fail_init_proc;
	}

	utilproc = proc_create("utilization", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_util_ops);
	if (NULL == utilproc) {
		printk("error to create %s/utilization proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}

	jobproc = proc_create("job", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_job_ops);
	if (jobproc == NULL) {
		printk("error to create %s/job proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}

	dbgmodeproc = proc_create("h26xd_dbg_mode", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_dbgmode_ops);
	if (dbgmodeproc == NULL) {
		printk("error to create %s/h26xd_dbg_mode proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}

	levelproc = proc_create("h26xd_dbglevel", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_level_ops);
	if (levelproc == NULL) {
		printk("error to create %s/dbglevel proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}

	posproc = proc_create("h26xd_dbgpos", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_pos_ops);
	if (posproc == NULL) {
		printk("error to create %s/dbgpos proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}

	paramproc = proc_create("param", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_param_ops);
	if (paramproc == NULL) {
		printk("error to create %s/param proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}

	modparamproc = proc_create("mod_param", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_mod_param_ops);
	if (modparamproc == NULL) {
		printk("error to create %s/mod_param proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}

	localbufparamproc = proc_create("local_buf_param", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_local_buf_param_ops);
	if (localbufparamproc == NULL) {
		printk("error to create %s/local_buf_param proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}

	h26xd_wakeup_proc = proc_create("wakeup", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_wakeup_ops);
	if (h26xd_wakeup_proc == NULL) {
		printk("error to create %s/wakeup proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}
	h26xd_counter_proc = proc_create("counter", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_counter_ops);
	if (h26xd_counter_proc == NULL) {
		printk("error to create %s/counter proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}
	h26xd_joblog_proc = proc_create("joblog", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_joblog_ops);
	if (h26xd_joblog_proc == NULL) {
		printk("error to create %s/joblog proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}
	h26xd_perf_proc = proc_create("perf", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_perf_ops);
	if (h26xd_perf_proc == NULL) {
		printk("error to create %s/perf proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}
	h26xd_sw_autogate_proc = proc_create("gating_en", S_IRUGO | S_IXUGO, h26xd_entry_proc, &h26xd_proc_auto_gate_ops);
	if (h26xd_sw_autogate_proc == NULL) {
		printk("error to create %s/param proc\n", ENTITY_PROC_NAME);
		goto fail_init_proc;
	}

	return 0;

fail_init_proc:

	h26xd_proc_close();
	return -EFAULT;
}

