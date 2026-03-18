#include "platform_port.h"
#include "h26xdec_dbg.h"
#include "h26xdec_flow.h"
#include "imvq.h"

extern struct bindch_info_t *bind_chn_idx[ENTITY_CHIPS];

/* Get job status string */
const char *h26xd_job_status_str(unsigned int status)
{
	const char *str;

	switch (status) {
	case JOB_STATUS_STANDBY:
		str = "1S";
		break;
	case JOB_STATUS_ONGOING:
		str = "2O";
		break;
	case JOB_STATUS_KEEP:
		str = "3K";
		break;
	case JOB_STATUS_DPB_REL:
		str = "4D";
		break;
	case JOB_STATUS_FLUSH:
		str = "5FLH";
		break;
	case JOB_STATUS_FAIL:
		str = "6FAL";
		break;
	default:
		str = "0U";
		break;
	}
	return str;
}

const char *h26xd_job_status_long_str(unsigned int status)
{
	const char *str;

	switch (status) {
	case JOB_STATUS_STANDBY:
		str = "STANDBY";
		break;
	case JOB_STATUS_ONGOING:
		str = "ONGOING";
		break;
	case JOB_STATUS_KEEP:
		str = "   KEEP";
		break;
	case JOB_STATUS_DPB_REL:
		str = "DPB_REL";
		break;
	case JOB_STATUS_FLUSH:
		str = "  FLUSH";
		break;
	case JOB_STATUS_FAIL:
		str = "   FAIL";
		break;
	default:
		str = "UNKNOWN";
		break;
	}
	return str;
}

const char *h26xd_err_num_str(enum h26xd_err_type err_type)
{
	const char *str;

	switch (err_type) {
	case H26XD_SCAN_HEADER_ERR:
		str = "SCAN_HEADER";
		break;
	case H26XD_SET_REC_BUF_ERR:
		str = "SET_REC_BUF";
		break;
	case H26XD_ERR_TRIGGER_ERR:
		str = "TRIGGER";
		break;
	case H26XD_ERR_HW_TIMEOUT:
		str = "HW_TIMEOUT";
		break;
	case H26XD_ERR_AD_ISR_ERR:
		str = "AD_ISR";
		break;
	case H26XD_ERR_BIN_ISR_ERR:
		str = "BIN_ISR";
		break;
	default:
		str = "UNKNOWN";
		break;
	}
	return str;
}

/*
 * handler function for printing message at damnit/dump log
 */
int h26xd_log_printout_handler(uintptr_t data)
{
	struct h26xd_job_item_t *job_item, *next;
	int chip_idx, chn;
	unsigned long flags;
	extern vk_spinlock_t h26x_dec_lock;
	extern struct h26xd_data_t *private_data[ENTITY_CHIPS];
	extern char h26xd_ver_str[128];
	extern unsigned int resv_total_out_cnt;
	extern unsigned int free_total_out_cnt;
	extern int h26xd_max_chip, max_total_cam_ch;
	extern unsigned int g_state_machine_err;
	extern unsigned int g_internal_err;
	extern unsigned int g_dec_err;

	printm(MODULE_NAME, "<<< H26XD PrintOut Start >>>\n");

	printm(MODULE_NAME, "%s\n", h26xd_ver_str);
	printm(MODULE_NAME, "state_machine_err:%u, internal_err:%u, dec_err:%u\n", g_state_machine_err, g_internal_err, g_dec_err);
	printm(MODULE_NAME, "total - reserved buffer %u free buffer %u, jiffies:%d\n", resv_total_out_cnt, free_total_out_cnt, (int)jiffies & 0xffff);
	printm(MODULE_NAME, "=========================================================================\n");
	printm(MODULE_NAME, "Chip Eng Chn Job_ID         Status Puttime Start  End    cb resvd buf_idx standby cur_job max_ref_num closing h264\n");

	printk("[DE] <<< H26XD PrintOut Start >>>\n");
	printk("[DE] %s\n", h26xd_ver_str);
	printk("[DE] state_machine_err:%u, internal_err:%u, dec_err:%u\n", g_state_machine_err, g_internal_err, g_dec_err);
	printk("[DE] total - reserved buffer %u free buffer %u, jiffies:%d\n", resv_total_out_cnt, free_total_out_cnt, (int)jiffies & 0xffff);

	for (chip_idx = 0; chip_idx < h26xd_max_chip; chip_idx++) {
		for (chn = 0; chn < max_total_cam_ch; chn ++) {
			struct h26xd_data_t *dec_data;
			int	 close, h264;

			if (atomic_read(&bind_chn_idx[chip_idx][chn].is_used) == 0)
				continue;

			h26xd_spin_lock_irqsave(&h26x_dec_lock, flags);
			dec_data = private_data[chip_idx] + chn;
			close = atomic_read(&bind_chn_idx[chip_idx][chn].is_closed);

			vos_list_for_each_entry_safe(job_item, next, &dec_data->chan_list, chan_list) {
				h264 = (job_item->codec_type == KDRV_VDODEC_TYPE_H264) ? 1 : 0;
				printm(MODULE_NAME, "%-4d %-3d %-3d %-13u %-6s 0x%-5x 0x%-4x 0x%-4x %-2d %-5d %-7d %-8s %-7d %-11d %-7d %-4d \n",
				       job_item->chip, job_item->engine, job_item->chn, job_item->job_id,
				       h26xd_job_status_long_str(job_item->status), job_item->puttime & 0xffff,
				       (int)job_item->starttime & 0xffff, (int)job_item->finishtime & 0xffff,
				       job_item->callback_flg, job_item->res_flg, job_item->buf_idx,
				       vos_list_empty(&dec_data->standby_list) ? "empty" : "no_empty", (dec_data->curr_job != NULL), dec_data->max_num_ref_frames, close, h264);
			}
			h26xd_spin_unlock_irqrestore(&h26x_dec_lock, flags);
		}
	}
	printm(MODULE_NAME, "<<< H26XD PrintOut End >>>\n");
	printk("[DE] <<< H26XD PrintOut End >>>\n");
	return 0;
}

int h26xd_write_file(char *path, unsigned char *buf, int size)
{
	struct file *filp;

	filp = filp_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);

	if (IS_ERR(filp)) {
		printk("Error to open %s\n", path);
		goto returnit;
	}
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0))
	kernel_write(filp, (void *)buf, (size_t)size, &filp->f_pos);
#else
	kernel_write(filp, (void *)buf, (size_t)size, filp->f_pos);
#endif

	//printk("ret: %d size:%d offset:%llu\n", ret, size, offset);

	filp_close(filp, NULL);

	return 0;
returnit:
	return -1;
}

/*
 *  save input bitstream to file
 */
void h26xd_save_bitstream_to_file(struct h26xd_job_item_t *job_item)
{
	static int save_bs_idx = 0; /* index of the saving bitstream */
	extern int save_bs_cnt;
	extern char file_path_buf[64];

	if (save_bs_cnt != 0) {
		char path[128];
		if (save_bs_idx != save_bs_cnt) {
			snprintf(path, sizeof(path), "%s/idx-%d_bs_size-%d_job_id-%d_fd-0x%08x.%s",
				 file_path_buf, save_bs_idx, job_item->bs_size, job_item->job_id, job_item->fd,
				 (KDRV_VDODEC_TYPE_H264 == job_item->codec_type) ? "264" : "265");
			h26xd_write_file(path, (unsigned char *)job_item->bs_addr_va, job_item->bs_size);
			printk("write bs: %s\n", path);
			save_bs_idx++;
		} else {
			save_bs_idx = save_bs_cnt = 0;
		}
	}
}

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
/* only for 530. Because ipcam sdk doesn't have such symbol */
int damnit(char *module)
{
	printk("---------- module: %s ----------\n", module);
	dump_stack();

	return 0;
}

/* not active yet */
static char log[256];
void printm(char *module, const char *fmt, ...)
{
	int	len = 0;
    int prelen = strlen(module);
	va_list args;

	if (prelen > 4)	//prefix must be short.
		return;

	log[0] = '[';
	prelen ++;
	//coverity[fixed_size_dest]: module length never exceeds 256
	strcpy(&log[1], module);
	log[prelen] = ']';
	prelen ++;
	va_start(args, fmt);
	len = vsnprintf((char *)log + prelen, (sizeof(log) - prelen), fmt, args) + prelen;
	va_end(args);

	if (len) {
        printk(log);
    }
}
#endif /* CONFIG_NVT_IVOT_PLAT_NA51102 */

#if INCLUDE_HVYLOAD
#define HVY_LOAD_SKIP_COMPARE       (0x01 << 2)
#define HVY_LOAD_READ_CHKSUM        (0x02 << 4)
#define HVY_LOAD_BURST_64          	(0x3F << 8)
#define HVY_LOAD_TEST_TIMES			(0x1 << 16)

#define DMA_CHANNEL0_HEAVY_LOAD_CTRL_OFS    		0x60
#define DMA_CHANNEL0_HEAVY_LOAD_START_ADDR_OFS  	0x64
#define DMA_CHANNEL0_HEAVY_LOAD_DMA_SIZE_OFS		0x68
#define DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE_OFS		0x6C
#define DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS    		0x5C

#define HEAVY_LOAD_CTRL_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_CTRL_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_ADDR_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_START_ADDR_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_SIZE_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_DMA_SIZE_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_WAIT_CYCLE_OFS(ch)   (DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE_OFS + ((ch) * 0x10))

#define HEAVYLOAD_BASE	0x2F00A8000
static void __iomem *v_heavyload_addr = NULL;

u16 h26xdec_heavyload_checksum(uintptr_t phy_addr, unsigned int length)
{
	u32 checksum = 0, value, msb = 0;
	int ch = 0;

	if (phy_addr & 0x03) {
		printk("addr need word align\n");
		return -1;
	}
	if (length & 0x03) {
		printk("length need word align\n");
		return -1;
	}

	value = readl(v_heavyload_addr + DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS);
	if (value & (0x1 << ch)) {
		printk("Error! ch%d is enabled already! \n", ch);
		return 0x0;
	}

	// set heavy load region
	writel(HVY_LOAD_TEST_TIMES | HVY_LOAD_SKIP_COMPARE | HVY_LOAD_READ_CHKSUM | HVY_LOAD_BURST_64, v_heavyload_addr + HEAVY_LOAD_CTRL_OFS(ch));
	writel(phy_addr, v_heavyload_addr + HEAVY_LOAD_ADDR_OFS(ch));
	writel(length, v_heavyload_addr + HEAVY_LOAD_SIZE_OFS(ch));

#ifdef __aarch64__
	msb = (phy_addr >> 32) & 0xF;
	value = readl(v_heavyload_addr + 0x54);
	value &= ~0xF;
	value |= msb;
	writel(value, v_heavyload_addr + 0x54);
#endif

	// trigger heavy load
	value = readl(v_heavyload_addr + DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS);
	value |= (0x1 << ch);
	writel(value, v_heavyload_addr + DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS);

	while (1) {
		//cond_resched();

		value = readl(v_heavyload_addr + DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS);
		if (!(value & (0x1 << ch)))
			break;
	}

	//checksum = ((readl(v_heavyload_addr + 0x06C) & 0xFFFF0000) >> 16) + (((1 + length) * length) >> 1);
	checksum = (readl(v_heavyload_addr + HEAVY_LOAD_WAIT_CYCLE_OFS(ch)) & 0xFFFF0000) >> 16;

	//printk("HW checksum = 0x%x\n", checksum);

	return (u16)checksum;
}

void h26xdec_debug_init(void)
{
	v_heavyload_addr = ioremap_nocache(HEAVYLOAD_BASE, (size_t) 0x1000);
}

void h26xdec_debug_uninit(void)
{
	if (v_heavyload_addr != NULL)
		iounmap((void *)v_heavyload_addr);
}
#endif /* INCLUDE_HVYLOAD */
