#include <linux/random.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "ddr_arb_proc.h"
#include "ddr_arb_main.h"
#include "ddr_arb_api.h"
#include "ddr_arb_int.h"
#include "comm/timer.h"
#include "kwrap/flag.h"
#include <kwrap/task.h>
#include "ddr_arb_platform.h"
#include <linux/soc/nvt/fmem.h>
#include "kwrap/util.h"
#include <plat/nvt_jiffies.h>



#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#ifdef CONFIG_HAVE_HW_BREAKPOINT
#define CWP_FUNC   1
#else
#define CWP_FUNC   0
#endif

#ifdef CONFIG_NVT_SMALL_HDAL
#define HEAVYLOAD_EN 0
#else
#define HEAVYLOAD_EN 1
#endif

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 256
#define MAX_ARG_NUM     16

#define DBG_TEST_EN	(0)
#define DM_DBG_EN	(1)
#define DM_EFF      (0)

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PMODULE_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
//PXXX_DRV_INFO pdrv_info_data;

//============================================================================
// Function define
//============================================================================


//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================

#if (DBG_TEST_EN == 1)
static PROC_CMD cmd_write_list[] = {
        // keyword          function name
        { "auto",           nvt_ddr_arb_api_auto_test          },
};

#define NUM_OF_WRITE_CMD (sizeof(cmd_write_list) / sizeof(PROC_CMD))


static int nvt_ddr_arb_proc_cmd_show(struct seq_file *sfile, void *v)
{
        nvt_dbg(IND, "\n");
        return 0;
}

static int nvt_ddr_arb_proc_cmd_open(struct inode *inode, struct file *file)
{
        nvt_dbg(IND, "\n");
        return single_open(file, nvt_ddr_arb_proc_cmd_show, &pdrv_info_data->module_info);
}

static ssize_t nvt_ddr_arb_proc_cmd_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
        int len = size;
        int ret = -EINVAL;
        char cmd_line[MAX_CMD_LENGTH];
        char *cmdstr = cmd_line;
        const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
        char *argv[MAX_ARG_NUM] = {0};
        unsigned char ucargc = 0;
        unsigned char loop;

        // check command length
        if (len > (MAX_CMD_LENGTH - 1)) {
                nvt_dbg(ERR, "Command length is too long!\n");
                goto ERR_OUT;
        }

        // copy command string from user space
        if (copy_from_user(cmd_line, buf, len))
                goto ERR_OUT;

	if (len == 0)
                cmd_line[0] = '\0';
        else
                cmd_line[len - 1] = '\0';

        nvt_dbg(IND, "CMD:%s\n", cmd_line);

        // parse command string
        for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
                argv[ucargc] = strsep(&cmdstr, delimiters);

                if (argv[ucargc] == NULL)
                    break;
        }

	// dispatch command handler
        if (strncmp(argv[0], "r", 2) == 0) {
        } else if (strncmp(argv[0], "w", 2) == 0)  {
		for (loop = 0 ; loop < NUM_OF_WRITE_CMD ; loop++) {
                        if (strncmp(argv[1], cmd_write_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
                                ret = cmd_write_list[loop].execute(&pdrv_info_data->module_info, ucargc - 2, &argv[2]);
                                break;
                        }
                }

                if (loop >= NUM_OF_WRITE_CMD)
                goto ERR_INVALID_CMD;

        } else
                goto ERR_INVALID_CMD;

        return size;

ERR_INVALID_CMD:
        nvt_dbg(ERR, "Invalid CMD \"%s\"\n", cmd_line);

ERR_OUT:
        return -1;
}


static struct proc_ops proc_cmd_fops = {
        .proc_open    = nvt_ddr_arb_proc_cmd_open,
        .proc_read    = seq_read,
        .proc_lseek  = seq_lseek,
        .proc_release = single_release,
        .proc_write   = nvt_ddr_arb_proc_cmd_write
};
#endif

//=============================================================================
// proc "help" file operation functions
//=============================================================================
#define DBGUT_DMA_USAGE_HW_PERIOD_MS 25
#define FLGDBGUT_UNKNOWN    FLGPTN_BIT(0)
#define FLGDBGUT_QUIT       FLGPTN_BIT(1)
#define FLGDBGUT_DMA_USAGE  FLGPTN_BIT(2)   //notify task there's command coming
#define FLGDBGUT_DMA_MON    FLGPTN_BIT(3)   //notify task there's command coming
#define FLGDBGUT_DMA_HVYLOAD  FLGPTN_BIT(4)   //notify task there's command coming
#define FLGDBGUT_DMA_HVYLOAD_DONE FLGPTN_BIT(5)  //notify task there's command coming
#define FLGDBGUT_QUIT_HVY     FLGPTN_BIT(6)
#define FLGDBGUT_QUIT_DONE  FLGPTN_BIT(7)



/**
     Timer Type.
*/
typedef enum _DBGUT_TM_TYPE {
	DBGUT_TM_TYPE_DMA,
	DBGUT_TM_TYPE_DMA2,
	ENUM_DUMMY4WORD(DBGUT_TM_TYPE)
} DBGUT_TM_TYPE;




//static VOID dbgut_dma_timer_cb(ULONG uiEvent);


//static DBGUT_CTRL_DMA_USAGE nvt_ddr_proc_cfg;
#if HEAVYLOAD_EN
static DRAM_CONSUME_ATTR nvt_heavyload_cfg;
static FLGPTN nvt_dram1_heavyload_flag_id;
static THREAD_HANDLE nvt_dram1_heavyload_tsk_id;
static void *handle = NULL;
#endif
//static FLGPTN nvt_ddr_proc_flag_id;
//static THREAD_HANDLE nvt_ddr_proc_tsk_id;

//static UINT64 ddr_cnt[2] = {0, 0};



static int run_dma_usage(int id, MODULE_INFO *pmodule_info)
{
	DBGUT_CTRL_DMA_USAGE *p_dma = &pmodule_info->nvt_ddr_proc_cfg[id];
	UINT64 cal_usage = 0;
	UINT64 cal_efficiency = 0;
	UINT64 cal_util = 0;
	//UINT64 cal_mon_period = 0;
	UINT64 cal_bandwidth = 0;
	int i = 0;
	UINT64 band[4];
	UINT64 dma_banwidth = dma_get_ddr_bandwidth((DDR_ARB)id, pmodule_info);
	UINT64 cnt = p_dma->reset_cnt;
	//printk("dma_banwidth = %lld \r\n", dma_banwidth);
	do_div(dma_banwidth, 1000*1000);
	//printk("dma_banwidth do_div= %lld \r\n", dma_banwidth);
	band[0] = band[1] = band[2] = band[3] = 0;

	p_dma->remain_cnt--;

	p_dma->acc_usage += dma_get_utilization((DDR_ARB)id);
	p_dma->acc_efficiency += dma_get_efficiency((DDR_ARB)id);

	for (i = 0; i < 4; i++) {
		if (p_dma->monitor_is_start[i]) {
			//p_dma->acc_monitor[i] += dma_get_monitor_bandwidth((DMA_MON_CH)i);
			p_dma->acc_monitor[i] += dma_get_monitor_count((DDR_ARB)id, (DMA_MON_CH)i);
			pmodule_info->data_cnt[id][i] += dma_get_monitor_count((DDR_ARB)id, (DMA_MON_CH)i);
		}
	}

	if (p_dma->remain_cnt <= 0) {
		cal_usage = p_dma->acc_usage*100;
		cal_efficiency = p_dma->acc_efficiency*100;
		//pr_info("cal usage %lld\r\n",cal_usage);
		//cal_mon_period = cnt * dma_get_monitor_period(DDR_ARB_1);

		do_div(cal_efficiency, cnt);
		do_div(cal_usage, cnt);
		do_div(p_dma->acc_usage, cnt);
		cal_util = cal_efficiency;
		do_div(cal_efficiency, p_dma->acc_usage);
		do_div(cal_usage, dma_get_monitor_period(DDR_ARB_1));
		//do_div(cal_util, dma_get_monitor_period(DDR_ARB_1));
		//pr_info("usage %lld\r\n",cal_usage);
		//pr_info("cal mom %lld\r\n",cal_mon_period);
		//pr_info("reset cnt %d\r\n",p_dma->reset_cnt);
		//pr_info("mon %d\r\n",dma_get_monitor_period(DDR_ARB_1));
		p_dma->usage = (UINT32)cal_usage;
		p_dma->efficiency = (UINT32)cal_efficiency;
		//pr_info("dma_banwidth %lld\r\n", dma_banwidth);
		for (i = 0; i < 4; i++) {
			//do_div(dma_banwidth, p_dma->reset_cnt);
			band[i] = p_dma->acc_monitor[i]*4;
			do_div(band[i], (p_dma->interval_ms / 1000));
			do_div(band[i], 1024*1024);
			p_dma->acc_monitor[i] = band[i] * 100;
			//pr_info("acc_monitor %lld\r\n", p_dma->acc_monitor[i]);
			do_div(p_dma->acc_monitor[i], dma_banwidth);
			//pr_info("acc_monitor %lld\r\n", p_dma->acc_monitor[i]);
			p_dma->monitor[i] = p_dma->acc_monitor[i];
			//pr_info("monitor %d\r\n", p_dma->monitor[i]);
			p_dma->acc_monitor[i] = 0;
		}
		//cal_bandwidth = dma_banwidth * p_dma->usage * p_dma->efficiency;
		cal_bandwidth = dma_banwidth * cal_util;
		//do_div(cal_bandwidth, 10000);
		do_div(cal_bandwidth, 100 * dma_get_monitor_period(DDR_ARB_1));
		do_div(cal_util, dma_get_monitor_period(DDR_ARB_1));
		p_dma->acc_usage = 0;
		p_dma->acc_efficiency = 0;
		p_dma->remain_cnt = (INT32)p_dma->reset_cnt;
		//pr_info("dram1: %d, %d, %lld MB/s\r\n", p_dma->usage, p_dma->efficiency, cal_bandwidth);
		p_dma->interval_cnt++;
		if (p_dma->is_start) {
			//pr_info("dram%d BUSY %d, EFF %d, UTI %d, %lld MB/s\r\n", id+1, p_dma->usage, p_dma->efficiency, (p_dma->usage*p_dma->efficiency)/100, cal_bandwidth);
			pr_info("dram%d BUSY %d, EFF %d, UTI %d, %lld MB/s\r\n", id+1, p_dma->usage, p_dma->efficiency, (int)cal_util, cal_bandwidth);
		}
		for (i = 0; i < 4; i++) {
			if (p_dma->monitor_is_start[i]) {
				pr_info("dram%d monitor %d, channel %d, bandwidth %d, %lld MB/s\r\n", id+1, i, p_dma->mon_ch[i], p_dma->monitor[i], band[i]);
			}
		}
/*
		p_dma->usage = p_dma->acc_usage / p_dma->reset_cnt;
		p_dma->efficiency = p_dma->acc_efficiency / p_dma->reset_cnt;
		//p_dma->usage = p_dma->usage;
		for (i = 0; i < 4; i++) {
			p_dma->monitor[i] = p_dma->acc_monitor[i] / p_dma->reset_cnt;
			p_dma->acc_monitor[i] = 0;
		}
		p_dma->acc_usage = 0;
		p_dma->acc_efficiency = 0;
		p_dma->remain_cnt = (INT32)p_dma->reset_cnt;
		p_dma->interval_cnt++;
		if (p_dma->is_start) {
			pr_info("dram utiliztion %d, efficiency %d, bandwidth %d\r\n", p_dma->usage, p_dma->efficiency, (p_dma->usage*p_dma->efficiency)/100);
		}
		for (i = 0; i < 4; i++) {
			if (p_dma->monitor_is_start[i])
				pr_info("monitor %d, channel %d, bandwidth %d\r\n", i, p_dma->mon_ch[i], p_dma->monitor[i]);
		}
*/
	}

	return 0;
}

static THREAD_DECLARE(dbgut_tsk, arglist)
{
	FLGPTN flg_ptn = 0;
	MODULE_INFO *pmodule_info = arglist;
	ID flag_id = pmodule_info->nvt_ddr_proc_flag_id[0];
	FLGPTN mask = FLGDBGUT_DMA_USAGE | FLGDBGUT_DMA_MON | FLGDBGUT_QUIT;

	DBGUT_CTRL_DMA_USAGE *p_dma = &pmodule_info->nvt_ddr_proc_cfg[0];
	unsigned long time = 0;

	while (1) {
		//wai_flg(&flg_ptn, flag_id, mask, TWF_ORW | TWF_CLR);
		if(vos_flag_wait_timeout(&flg_ptn, flag_id, mask, TWF_ORW | TWF_CLR, 100) == 0) {
			if ((flg_ptn & FLGDBGUT_QUIT) != 0U) {
				break;
			}
			time = get_nvt_jiffies();
			while (1) {
				vos_util_delay_ms(1);
				if (((get_nvt_jiffies() - time) >= 25) && ((p_dma->is_start == 1) || (p_dma->monitor_is_start[0] == 1) || (p_dma->monitor_is_start[1] == 1) || (p_dma->monitor_is_start[2] == 1) || (p_dma->monitor_is_start[3] == 1))) {
					run_dma_usage(0, pmodule_info);
					time = get_nvt_jiffies();
				}
				if ((p_dma->is_start == 0) && (p_dma->monitor_is_start[0] == 0) && (p_dma->monitor_is_start[1] == 0) && (p_dma->monitor_is_start[2] == 0) && (p_dma->monitor_is_start[3] == 0)) {
					break;
				}
			}
			/*
			wai_flg(&flg_ptn, flag_id, mask, TWF_ORW | TWF_CLR);
			if ((flg_ptn & FLGDBGUT_DMA_USAGE) != 0U) {
				run_dma_usage();
			}
			if ((flg_ptn & FLGDBGUT_QUIT) != 0U) {
				break;
			}
			*/
		}
	}
	set_flg(flag_id, FLGDBGUT_QUIT_DONE);
	THREAD_RETURN(0);
}
#if HEAVYLOAD_EN

#define RANDOM_MODE 0

#if RANDOM_MODE
static UINT32 mode[4] = {0, 1, 3, 2};
static UINT32 hvy_cnt = 0;
static UINT32 burst[3] = {0, 0, 0};
#endif

static THREAD_DECLARE(heavyload_tsk, arglist)
{
	UINT32 count[1];
	ULONG test1_start, test2_start, test3_start;
	UINT32 test1_size, test2_size, test3_size;
	DMA_HEAVY_LOAD_PARAM hvy_param0, hvy_param1, hvy_param2;
	BOOL err = FALSE;
	FLGPTN flg_ptn = 0;
	ID flag_id = nvt_dram1_heavyload_flag_id;
	FLGPTN mask = FLGDBGUT_DMA_HVYLOAD | FLGDBGUT_QUIT_HVY;

	while (1) {
		wai_flg(&flg_ptn, flag_id, mask, TWF_ORW | TWF_CLR);

		if ((flg_ptn & FLGDBGUT_DMA_HVYLOAD) != 0U) {
			test1_size =
				test2_size =
					test3_size = ALIGN_FLOOR_32(nvt_heavyload_cfg.size / 3);

			test1_start = nvt_heavyload_cfg.addr;
			test2_start = test1_start + test1_size;
			test3_start = test2_start + test2_size;

			get_random_bytes(&count[0], 4);
#if RANDOM_MODE
			count[0] = count[0] & 0xfff;
#else
			count[0] = count[0] & 0xffff;
#endif


#if RANDOM_MODE
			get_random_bytes(&burst[0], 4);
			hvy_param0.burst_len	= burst[0]%64 +1;
			hvy_param0.test_method	= mode[hvy_cnt%4];
			if (mode[hvy_cnt%4] < 2) {
				hvy_param0.random_pattern = 1;
			} else {
				hvy_param0.random_pattern = 0;
			}
#else
			hvy_param0.burst_len	= 64;
			hvy_param0.test_method	= DMA_HEAVY_LOAD_RW_BURST_INTERLEAVE;
			hvy_param0.random_pattern = 1;
#endif
			hvy_param0.dma_size 	= test1_size;
			hvy_param0.start_addr	= test1_start;
			hvy_param0.test_times	= count[0];
			hvy_param0.outstanding_en  = 1;
			hvy_param0.outstanding	= 3;
			hvy_param0.is_phy_addr = 0;




#if RANDOM_MODE
			get_random_bytes(&burst[1], 4);
			hvy_param1.burst_len	= burst[1]%64 +1;
			hvy_param1.test_method	= mode[(hvy_cnt+1)%4];
			if (mode[(hvy_cnt+1)%4] < 2) {
				hvy_param1.random_pattern = 1;
			} else {
				hvy_param1.random_pattern = 0;
			}
#else
			hvy_param1.burst_len	= 64;
			hvy_param1.test_method	= DMA_HEAVY_LOAD_RW_BURST_INTERLEAVE;
			hvy_param1.random_pattern = 1;
#endif

			hvy_param1.dma_size 	= test2_size;
			hvy_param1.start_addr	= test2_start;
			hvy_param1.test_times	= count[0];
			hvy_param1.outstanding_en  = 1;
			hvy_param1.outstanding	= 3;
			hvy_param1.is_phy_addr = 0;

#if RANDOM_MODE
			get_random_bytes(&burst[2], 4);
			hvy_param2.burst_len	= burst[2]%64 +1;
			hvy_param2.test_method	= mode[(hvy_cnt+2)%4];
			if (mode[(hvy_cnt+2)%4] < 2) {
				hvy_param2.random_pattern = 1;
			} else {
				hvy_param2.random_pattern = 0;
			}
#else
			hvy_param2.burst_len	= 64;
			hvy_param2.test_method	= DMA_HEAVY_LOAD_RW_BURST_INTERLEAVE;
			hvy_param2.random_pattern = 1;
#endif
			hvy_param2.dma_size 	= test3_size;
			hvy_param2.start_addr	= test3_start;
			hvy_param2.test_times	= count[0];
			hvy_param2.outstanding_en  = 1;
			hvy_param2.outstanding	= 3;
			hvy_param2.is_phy_addr = 0;


			DBG_IND("*************ConsumeTsk Information*******************\r\n");
			DBG_IND("*   (@)Loading Degree ==================>[     %05d]*\r\n", nvt_heavyload_cfg.load_degree);


			// Easy
			if (nvt_heavyload_cfg.load_degree == DRAM_CONSUME_EASY_LOADING) {
				if (hvy_param0.test_times > 2000) {
					hvy_param0.test_times = 2000;
				}
				DBG_IND("*   (@)Channel0 Burst Length============>[     %05d]*\r\n", hvy_param0.burst_len);
				DBG_IND("*   (@)Channel0 Test DMA addr ==========>[0x%lx]*\r\n", hvy_param0.start_addr);
				DBG_IND("*   (@)Channel0 Test DMA size ==========>[0x%08x]*\r\n", hvy_param0.dma_size);
				DBG_IND("*   (@)Channel0 Test count  ============>[     %05d]*\r\n", hvy_param0.test_times);
				DBG_IND("*************ConsumeTsk Information*******************\r\n");


				if (dma_enable_heavyload(DDR_ARB_1, DMA_HEAVY_LOAD_CH0, &hvy_param0) != E_OK) {
					DBG_ERR("Consume task open fail degree[%d]-1\r\n", nvt_heavyload_cfg.load_degree);
					err = TRUE;
				}

				dma_trig_heavyload(DDR_ARB_1, 0x1);
			}
			// Normal
			else if (nvt_heavyload_cfg.load_degree == DRAM_CONSUME_NORMAL_LOADING) {
				DBG_IND("*   (@)Channel0 Burst Length============>[     %05d]*\r\n", hvy_param0.burst_len);
				DBG_IND("*   (@)Channel0 Test DMA addr ==========>[0x%lx]*\r\n", hvy_param0.start_addr);
				DBG_IND("*   (@)Channel0 Test DMA size ==========>[0x%08x]*\r\n", hvy_param0.dma_size);
				DBG_IND("*   (@)Channel0 Test count  ============>[     %05d]*\r\n", hvy_param0.test_times);
				DBG_IND("*   (@)Channel1 Burst Length============>[     %05d]*\r\n", hvy_param1.burst_len);
				DBG_IND("*   (@)Channel1 Test DMA addr ==========>[0x%lx]*\r\n", hvy_param1.start_addr);
				DBG_IND("*   (@)Channel1 Test DMA size ==========>[0x%08x]*\r\n", hvy_param1.dma_size);
				DBG_IND("*   (@)Channel1 Test count  ============>[     %05d]*\r\n", hvy_param1.test_times);
				DBG_IND("*************ConsumeTsk Information*******************\r\n");
				if (dma_enable_heavyload(DDR_ARB_1, DMA_HEAVY_LOAD_CH0, &hvy_param1) != E_OK) {
					DBG_ERR("Consume task open fail degree[%d]-1\r\n", nvt_heavyload_cfg.load_degree);
					err = TRUE;
				}
				if (dma_enable_heavyload(DDR_ARB_1, DMA_HEAVY_LOAD_CH1, &hvy_param1) != E_OK) {
					DBG_ERR("Consume task open fail degree[%d]-2\r\n", nvt_heavyload_cfg.load_degree);
					err = TRUE;
				}

				dma_trig_heavyload(DDR_ARB_1, 0x3);
			}
			// Heavy
			else {
				DBG_IND("*   (@)Channel0 Burst Length============>[     %05d]*\r\n", hvy_param0.burst_len);
				DBG_IND("*   (@)Channel0 Test DMA addr ==========>[0x%lx]*\r\n", hvy_param0.start_addr);
				DBG_IND("*   (@)Channel0 Test DMA size ==========>[0x%08x]*\r\n", hvy_param0.dma_size);
				DBG_IND("*   (@)Channel0 Test count  ============>[     %05d]*\r\n", hvy_param0.test_times);
				DBG_IND("*   (@)Channel1 Burst Length============>[     %05d]*\r\n", hvy_param1.burst_len);
				DBG_IND("*   (@)Channel1 Test DMA addr ==========>[0x%lx]*\r\n", hvy_param1.start_addr);
				DBG_IND("*   (@)Channel1 Test DMA size ==========>[0x%08x]*\r\n", hvy_param1.dma_size);
				DBG_IND("*   (@)Channel1 Test count  ============>[     %05d]*\r\n", hvy_param1.test_times);
				DBG_IND("*   (@)Channel2 Burst Length============>[     %05d]*\r\n", hvy_param2.burst_len);
				DBG_IND("*   (@)Channel2 Test DMA addr ==========>[0x%lx]*\r\n", hvy_param2.start_addr);
				DBG_IND("*   (@)Channel2 Test DMA size ==========>[0x%08x]*\r\n", hvy_param2.dma_size);
				DBG_IND("*   (@)Channel2 Test count  ============>[     %05d]*\r\n", hvy_param2.test_times);
				DBG_IND("*************ConsumeTsk Information*******************\r\n");


				//dma_setChannelPriority(DMA_CH_CPU, DMA_PRIORITY_HIGH);
				//dma_setChannelPriority(DMA_CH_CPU2, DMA_PRIORITY_HIGH);
				//dma_setChannelPriority(DMA_CH_HLOAD_0, DMA_PRIORITY_MIDDLE);
				//dma_setChannelPriority(DMA_CH_HLOAD_1, DMA_PRIORITY_MIDDLE);
				//dma_setChannelPriority(DMA_CH_HLOAD_2, DMA_PRIORITY_MIDDLE);

				if (dma_enable_heavyload(DDR_ARB_1, DMA_HEAVY_LOAD_CH0, &hvy_param0) != E_OK) {
					DBG_ERR("Consume task open fail degree[%d]-1\r\n", nvt_heavyload_cfg.load_degree);
					err = TRUE;
				}
				if (dma_enable_heavyload(DDR_ARB_1, DMA_HEAVY_LOAD_CH1, &hvy_param1) != E_OK) {
					DBG_ERR("Consume task open fail degree[%d]-2\r\n", nvt_heavyload_cfg.load_degree);
					err = TRUE;
				}
				if (dma_enable_heavyload(DDR_ARB_1, DMA_HEAVY_LOAD_CH2, &hvy_param2) != E_OK) {              // Enable maximun 3 channel at the same time
					DBG_ERR("Consume task open fail degree[%d]-3\r\n", nvt_heavyload_cfg.load_degree);
					err = TRUE;
				}

				dma_trig_heavyload(DDR_ARB_1, 0x7);
			}

			if (err == FALSE) {
				if (nvt_heavyload_cfg.load_degree == DRAM_CONSUME_EASY_LOADING) {
					if (dma_wait_heavyload_done_polling(0, NULL) == TRUE) {
						DBG_IND("SUCCESS\r\n");

						if (nvt_heavyload_cfg.is_start == TRUE) {
							set_flg(nvt_dram1_heavyload_flag_id, FLGDBGUT_DMA_HVYLOAD);
						} else {
							if (vos_mem_release_from_cma(handle) < 0)
								DBG_ERR("fmem release from cma fail\n");
						}
					} else {
						DBG_ERR("FAIL \r\n");
						DBG_ERR("Terminate Dram consume task... \r\n");
					}

				} else if (nvt_heavyload_cfg.load_degree == DRAM_CONSUME_NORMAL_LOADING) {
					if ((dma_wait_heavyload_done_polling(0, NULL) == TRUE) && (dma_wait_heavyload_done_polling(1, NULL) == TRUE)) {
						DBG_IND("SUCCESS\r\n");

						if (nvt_heavyload_cfg.is_start == TRUE) {
							set_flg(nvt_dram1_heavyload_flag_id, FLGDBGUT_DMA_HVYLOAD);
						} else {
							if (vos_mem_release_from_cma(handle) < 0)
								DBG_ERR("fmem release from cma fail\n");
						}
					} else {
						DBG_ERR("FAIL \r\n");
						DBG_ERR("Terminate Dram consume task... \r\n");
					}

				} else {
					if ((dma_wait_heavyload_done_polling(0, NULL) == TRUE) && (dma_wait_heavyload_done_polling(1, NULL) == TRUE) && (dma_wait_heavyload_done_polling(2, NULL) == TRUE)) {
						DBG_IND("SUCCESS\r\n");

						if (nvt_heavyload_cfg.is_start == TRUE) {
							set_flg(nvt_dram1_heavyload_flag_id, FLGDBGUT_DMA_HVYLOAD);
						} else {
							if (vos_mem_release_from_cma(handle) < 0)
								DBG_ERR("fmem release from cma fail\n");
						}
					} else {
						DBG_ERR("FAIL \r\n");
						DBG_ERR("Terminate Dram consume task... \r\n");
					}
				}

#if RANDOM_MODE
				hvy_cnt++;
#endif
			} else {
				nvt_heavyload_cfg.is_start = FALSE;
				if (vos_mem_release_from_cma(handle) < 0)
					DBG_ERR("fmem release from cma fail\n");
			}
		}
		if ((flg_ptn & FLGDBGUT_QUIT_HVY) != 0U) {
			break;
		}
	}
    set_flg(nvt_dram1_heavyload_flag_id, FLGDBGUT_DMA_HVYLOAD_DONE);
	THREAD_RETURN(0);
}
#endif
#if 0
static VOID dbgut_dma_timer_cb(ULONG uiEvent)
{
	set_flg(nvt_ddr_proc_flag_id, FLGDBGUT_DMA_USAGE);
}


static int dbgut_timer_lock(DBGUT_TM_TYPE type)
{
	TIMER_ID *p_id = NULL;
	DRV_CB fp_cb = NULL;
	switch (type) {
	case DBGUT_TM_TYPE_DMA:
		p_id = &nvt_ddr_proc_cfg.timer_id;
		fp_cb = dbgut_dma_timer_cb;
		break;
	default:
		break;
	}

	if (p_id == NULL) {
		nvt_dbg(ERR, "DBGUT_ER_TIMER_LOCK_EXCEPTION");
		return -1;
	}

	if (*p_id != TIMER_INVALID) {
		nvt_dbg(ERR, "DBGUT_ER_TIMER_ALREADY_LOCK");
		return -1;
	}

	if (timer_open(p_id, fp_cb) != E_OK) {
		*p_id = TIMER_INVALID;
		nvt_dbg(ERR, "DBGUT_ER_TIMER_LOCK_FAIL");
		return -1;
	}

	return 0;

}

static int dbgut_timer_unlock(DBGUT_TM_TYPE type)
{
	TIMER_ID *p_id = NULL;
	FLGPTN flag = 0;

	switch (type) {
	case DBGUT_TM_TYPE_DMA:
		p_id = &nvt_ddr_proc_cfg.timer_id;
		flag = FLGDBGUT_DMA_USAGE;
		break;
	default:
		flag = 0;
		break;
	}

	if (p_id == NULL) {
		nvt_dbg(ERR, "DBGUT_ER_TIMER_UNLOCK_EXCEPTION");
		return -1;
	}

	if (*p_id == TIMER_INVALID) {
		nvt_dbg(ERR, "DBGUT_ER_TIMER_ALREADY_UNLOCK");
		return -1;
	}

	timer_close(*p_id);

	if (flag) {
		clr_flg(nvt_ddr_proc_flag_id, flag);
	}

	*p_id = TIMER_INVALID;
	return 0;
}

static int dbgut_timer_cfg(DBGUT_TM_TYPE type)
{
	UINT32 uiUs = 0;
	TIMER_ID *p_id = NULL;

	switch (type) {
	case DBGUT_TM_TYPE_DMA:
		p_id = &nvt_ddr_proc_cfg.timer_id;
		uiUs = nvt_ddr_proc_cfg.cycle_us;
		break;
	default:
		break;
	}

	if (p_id == NULL) {
		nvt_dbg(ERR, "DBGUT_ER_TIMER_CFG_EXCEPTION");
		return -1;
	}

	if (*p_id == TIMER_INVALID) {
		nvt_dbg(ERR, "DBGUT_ER_TIMER_CFG_INVALID_TID");
		return -1;
	}

	if (timer_cfg(*p_id, uiUs, TIMER_MODE_FREE_RUN | TIMER_MODE_ENABLE_INT, TIMER_STATE_PLAY) != E_OK) {
		nvt_dbg(ERR, "DBGUT_ER_TIMER_CFG_FAIL");
		return -1;
	}
	return 0;
}
#endif
static int nvt_ddr_proc_usage_cfg(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	int dram_id = 0;
	DBGUT_CTRL_DMA_USAGE *p_dma;
	int i = 0;

	if (argc < 1) {
		nvt_dbg(WRN, "you need to set dram id 0~1\n");
		return 0;
	}

	sscanf(pargv[0], "%d", (int *)&dram_id);
	//nvt_dbg(ERR, "get interval_ms = %d\n", pmodule_info->nvt_ddr_proc_cfg[dram_id].interval_ms);
	p_dma = &pmodule_info->nvt_ddr_proc_cfg[dram_id];
	if(p_dma == NULL)
	{
		nvt_dbg(ERR, "p_dma == NULL\n");
	}
	p_dma->interval_ms = 1000; //default 1 sec

	if (argc > 1) {
		sscanf(pargv[1], "%d", (int *)&p_dma->interval_ms);
	}

	p_dma->interval_ms = p_dma->interval_ms / DBGUT_DMA_USAGE_HW_PERIOD_MS * DBGUT_DMA_USAGE_HW_PERIOD_MS;

	if (p_dma->interval_ms < DBGUT_DMA_USAGE_HW_PERIOD_MS) {
		p_dma->interval_ms = DBGUT_DMA_USAGE_HW_PERIOD_MS;
	}



	if (p_dma->is_start || p_dma->monitor_is_start[0] || p_dma->monitor_is_start[1] || p_dma->monitor_is_start[2] || p_dma->monitor_is_start[3]) {
		//dbgut_timer_unlock(DBGUT_TM_TYPE_DMA);
		nvt_dbg(WRN, "dma usage or dma monitor is open\n");
		return 0;
	}

	// always sample with 25 ms
	p_dma->reset_cnt = p_dma->interval_ms / 25;

	p_dma->usage = 0xFFFFFFFF;
	p_dma->acc_usage = 0;
	p_dma->efficiency = 0xFFFFFFFF;
	p_dma->acc_efficiency = 0;
	p_dma->remain_cnt = (INT32)p_dma->reset_cnt;
	p_dma->cycle_us = p_dma->interval_ms * 1000 / p_dma->reset_cnt;

	for (i = 0; i < 4; i++) {
		p_dma->monitor[i] = 0xFFFFFFFF;
		p_dma->acc_monitor[i] = 0;
	}
	return 0;
}
static int nvt_ddr_proc_usage_start(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	//int er;
	int dram_id = 0;
	DBGUT_CTRL_DMA_USAGE *p_dma;

	if (argc < 1) {
		nvt_dbg(WRN, "you need to set dram id 0~1\n");
		return 0;
	}

	sscanf(pargv[0], "%d", (int *)&dram_id);
	p_dma = &pmodule_info->nvt_ddr_proc_cfg[dram_id];
	if (p_dma->is_start) {
		nvt_dbg(WRN, "DBGUT_WR_DMA_USAGE_START_TWICE");
		return 0;
	}

	p_dma->interval_cnt = 0;
	p_dma->is_start = TRUE;
	//THREAD_RESUME(nvt_ddr_proc_tsk_id);
	set_flg(pmodule_info->nvt_ddr_proc_flag_id[dram_id], FLGDBGUT_DMA_USAGE);
/*
	er = dbgut_timer_lock(DBGUT_TM_TYPE_DMA);
	if (er != 0) {
		return er;
	}
	er = dbgut_timer_cfg(DBGUT_TM_TYPE_DMA);
	if (er != 0) {
		return er;
	}
*/
    return 0;
}
static int nvt_ddr_proc_usage_stop(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	//int er;
	int dram_id = 0;
	DBGUT_CTRL_DMA_USAGE *p_dma;

	if (argc < 1) {
		nvt_dbg(WRN, "you need to set dram id 0~1\n");
		return 0;
	}

	sscanf(pargv[0], "%d", (int *)&dram_id);

	p_dma = &pmodule_info->nvt_ddr_proc_cfg[dram_id];

	if (p_dma->is_start == FALSE) {
		nvt_dbg(WRN, "DBGUT_WR_DMA_USAGE_ALREADY_STOPED");
		return 0;
	}

	p_dma->is_start = FALSE;

	//er = dbgut_timer_unlock(DBGUT_TM_TYPE_DMA);
	//if (er != 0) {
	//	return er;
	//}
    return 0;
}

static int nvt_dram_protect_start(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	DMA_WRITEPROT_ATTR attr;
	int set, level, id;
	UINT32 v_mask[DMA_CH_GROUP_CNT], channel, channel2, start, size;

	if (argc < 7) {
		nvt_dbg(WRN, "[id 0:dram1, 1: dram2] [set 0~4:IN, 5:OUT] [channel:0~31] [channel2: 32~63] [level 0:WP, 1:WD, 2:RP, 3:RD] [start address(phys):64W align] [size:64W align]\r\n");
	}
	memset((void *)&attr.mask, 0x0, sizeof(DMA_CH_MSK));
	memset((void *)&v_mask, 0x0, sizeof(v_mask));

	sscanf(pargv[0], "%d", &id);
	sscanf(pargv[1], "%d", &set);
	sscanf(pargv[2], "%x", &channel);
	sscanf(pargv[3], "%x", &channel2);
	sscanf(pargv[4], "%d", &level);
	sscanf(pargv[5], "%x", &start);
	sscanf(pargv[6], "%x", &size);
	v_mask[0] = channel;
	v_mask[1] = channel2;
	memcpy(&attr.mask, v_mask, sizeof(DMA_CH_MSK));

	attr.level = (DMA_WRITEPROT_LEVEL)level;
	attr.starting_addr = (UINT32)start;
	attr.size = (UINT32)size;
	arb_enable_wp(id, (DMA_WRITEPROT_SET)set, &attr);

	return 0;
}

static int nvt_dram_protect_stop(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	int set, id;

	if (argc < 2) {
		nvt_dbg(WRN, "[id 0:dram1, 1: dram2] [set 0~4:IN, 5:OUT]\r\n");
	}

	sscanf(pargv[0], "%d", &id);
	sscanf(pargv[1], "%d", &set);

	arb_disable_wp(id, (DMA_WRITEPROT_SET)set);

	return 0;
}



static int nvt_ddr_proc_monitor_hdl(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{

    struct vos_mem_cma_info_t      buf_info = {0};
	int ret = 0;
    void *handle_cpu = NULL;


	ret = vos_mem_init_cma_info(&buf_info, VOS_MEM_CMA_TYPE_NONCACHE, 0x2000000);
	if (ret >= 0) {
		handle_cpu = vos_mem_alloc_from_cma(&buf_info);
	} else {
		nvt_dbg(WRN, "allocate mem fail!!\r\n");
		return ret;
	}
    dma_reset_data_count(0);

    memset((void *)buf_info.vaddr, 0xff, 0x2000000);
    dma_get_data_count(0);

    if (vos_mem_release_from_cma(handle_cpu) < 0)
		DBG_ERR("fmem release from cma fail\n");

	return 0;
}

#if CWP_FUNC
struct perf_event * __percpu *sample_hbp;

static void sample_hbp_handler(struct perf_event *bp,
           struct perf_sample_data *data,
           struct pt_regs *regs)
{
    printk(KERN_INFO "value is changed\r\n");
    dump_stack();
    printk(KERN_INFO "Dump stack from sample_hbp_handler\r\n");
}

static int cpu_enable_watch(UINT64 wp_addr)
{
   struct perf_event_attr attr;
   int    ret = 0;

   hw_breakpoint_init(&attr);
   attr.bp_addr = wp_addr;
   attr.bp_len = HW_BREAKPOINT_LEN_4;
   attr.bp_type = HW_BREAKPOINT_W;
   printk("\r\ncwp address: 0x%llx\r\n", attr.bp_addr);
   sample_hbp = register_wide_hw_breakpoint(&attr, sample_hbp_handler, NULL);
   if (IS_ERR((void __force *)sample_hbp)) {
    ret = PTR_ERR((void __force *)sample_hbp);
    printk("\r\nregister_wide_hw_breakpoint fail\r\n");
  }
  return ret;
}
static void cpu_disable_watch(void)
{
    unregister_wide_hw_breakpoint(sample_hbp);
}

//cat /proc/hdal/comm/task
//protect  Stack Area: va[0x80A8A000 ~ 0x80A8C000]
static int nvt_ddr_proc_cwp_cfg(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
    UINT32  enable=0;
    UINT64  addr=0;

    if(argc<1){
        DBG_DUMP("cwp [enable] [v_addr]");
        return -1;
    }

    sscanf(pargv[0], "%d", &enable);
    if(!enable) {
        cpu_disable_watch();
    } else {
        if(argc>=2) {
        	sscanf(pargv[1], "%llx", &addr);
            cpu_enable_watch(addr);
        }else {
            DBG_DUMP("cwp [enable] [v_addr]");
            return -1;
        }
    }
	return 0;
}
#else
static int nvt_ddr_proc_cwp_cfg(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
    DBG_DUMP("kenrl HW_BREAKPOINT not cfg\r\n");
    return 0;
}
#endif

#if DM_DBG_EN
static int nvt_ddr_proc_monitor_start(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	int ret;
	DMA_MON_ATTR attr;
	int dram_id = 0;
	UINT32 mon_set;
	long ch;
	UINT32 mode;
	CHAR *token;
	CHAR *cur;

	UINT32 v_mask[DMA_CH_GROUP_CNT];

	memset((void *)&attr.mask, 0x0, sizeof(DMA_CH_MSK));
	memset((void *)&v_mask, 0x0, sizeof(v_mask));
	if (argc < 4) {
		nvt_dbg(WRN, "[dram_id] [monitor_set:0~3] [channel:0~60] [0:W, 1:R, 2:RW]\n");
		return 0;
	}

	sscanf(pargv[0], "%d", &dram_id);
	sscanf(pargv[1], "%d", &mon_set);
	// sscanf(pargv[2], "%d", &ch);
	sscanf(pargv[3], "%d", &mode);

	cur = pargv[2];
	while ((token = strsep(&cur, ",")) != NULL) {
		ret = kstrtol(token, 10, &ch);
		if (0 != ret) {
			pr_info("kstrtol parsing failed: %d\n", ret);
			return 0;
		}
		if (ch > 60) {
			pr_info("skip invalid channel: %lu\n", ch);
			continue;
		} else {
			v_mask[(ch/32)] |= (1 << (ch%32));
		}
	}
	memcpy(&attr.mask, v_mask, sizeof(DMA_CH_MSK));

	attr.mode = mode;

#if DM_EFF
    dma_enable_module_eff(dram_id, attr);
#else
	dma_enable_monitor(dram_id, (DMA_MON_CH)mon_set, attr);
#endif
	return 0;
}

static int nvt_ddr_proc_monitor_stop(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	int mon_set;
	int dram_id = 0;

	if (argc < 2) {
		nvt_dbg(WRN, "[dram_id] [monitor_set:0~3]\n");
		return 0;
	}

	sscanf(pargv[0], "%d", &dram_id);
	sscanf(pargv[1], "%d", &mon_set);
#if DM_EFF
    dma_disable_module_eff(dram_id);
#else
	printk("monitor %d, total access %lld Bytes\r\n", mon_set, dma_get_monitor_count(dram_id, mon_set) * 4);
#endif
	dma_reset_monitor_cnt(dram_id, mon_set);
    return 0;
}
#endif

static int nvt_ddr_proc_monitor_start_t(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	mau_ch_mon_start(0, 2, 0);
    return 0;
}
static int nvt_ddr_proc_monitor_stop_t(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	pr_info("CPU %lld bytes\r\n", mau_ch_mon_stop(0, 0));
	return 0;
}

static int nvt_ddr_proc_priority_cfg(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	int channel = 0;
	int w_pri = 0;

    int r_pri = 0;

	if (argc < 3) {
		nvt_dbg(WRN, "[channel 0~60] [w_pri:0~3;  0: urgent, 1: high, 2: mid, 3: low] [r_pri:0~3;  0: urgent, 1: high, 2: mid, 3: low]\n");
		return 0;
	}


	sscanf(pargv[0], "%d", &channel);
	sscanf(pargv[1], "%d", &w_pri);
	sscanf(pargv[2], "%d", &r_pri);
	dma_set_priority(DDR_ARB_1, (DMA_CH)channel, (DMA_PRI)w_pri,(DMA_PRI)r_pri);



	return 0;
}

static PROC_CMD nvt_ddr_proc_cmd_write_list[] = {
	// keyword   function name
	{ "cfg",     nvt_ddr_proc_usage_cfg},
	{ "start",   nvt_ddr_proc_usage_start},
	{ "stop",    nvt_ddr_proc_usage_stop},
	{ "mwp",     nvt_dram_protect_start},
	{ "mwp_dis",   nvt_dram_protect_stop},

	{ "mon",     nvt_ddr_proc_monitor_hdl},

	{ "cwp",     nvt_ddr_proc_cwp_cfg},
	// channel priority
	{ "pri",     nvt_ddr_proc_priority_cfg},
#if DM_DBG_EN
	{ "dm_start",     nvt_ddr_proc_monitor_start},
	{ "dm_stop",     nvt_ddr_proc_monitor_stop},
#endif
	{ "mon_start", nvt_ddr_proc_monitor_start_t},
	{ "mon_stop", nvt_ddr_proc_monitor_stop_t}

};

#define NVT_DDR_PROC_NUM_OF_WRITE_CMD (sizeof(nvt_ddr_proc_cmd_write_list) / sizeof(PROC_CMD))

static int nvt_ddr_proc_show(struct seq_file *sfile, void *v)
{
	#if 1
	UINT64 cal_usage = 0;
	UINT64 cal_efficiency = 0;
	UINT64 cal_uti = 0;
	UINT64 busy = dma_get_utilization(DDR_ARB_1);
	UINT64 data = dma_get_efficiency(DDR_ARB_1);
	UINT64 period = dma_get_monitor_period(DDR_ARB_1);

	cal_usage = busy * 100;
	do_div(cal_usage, period);
	cal_efficiency = data * 100;
	cal_uti = cal_efficiency;
	do_div(cal_efficiency, busy);
	do_div(cal_uti, period);
	seq_printf(sfile, "BUSY: %d\n", (int)cal_usage);
	seq_printf(sfile, "EFF: %d\n", (int)cal_efficiency);
	seq_printf(sfile, "UTI: %d\n", (int)cal_uti);
	#endif
	return 0;
}

static int nvt_ddr_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ddr_proc_show, PDE_DATA(inode));
}

static ssize_t nvt_ddr_proc_cmd_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
        int len = size;
        int ret = -EINVAL;
        char cmd_line[MAX_CMD_LENGTH];
        char *cmdstr = cmd_line;
        const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
        char *argv[MAX_ARG_NUM] = {0};
        unsigned char ucargc = 0;
        unsigned char loop;
		MODULE_INFO *pmodule_info = ((struct seq_file *)file->private_data)->private;

        // check command length
        if (len > (MAX_CMD_LENGTH - 1)) {
                nvt_dbg(ERR, "Command length is too long!\n");
                goto ERR_OUT;
        }

        // copy command string from user space
		if (copy_from_user(cmd_line, buf, len)) {
                goto ERR_OUT;
		}

		if (len == 0) {
                cmd_line[0] = '\0';
		} else {
                cmd_line[len - 1] = '\0';
		}

        // parse command string
        for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
                argv[ucargc] = strsep(&cmdstr, delimiters);

                if (argv[ucargc] == NULL)
                    break;
        }

	// dispatch command handler
        if (strncmp(argv[0], "r", 2) == 0) {
			// nothing to do
        } else if (strncmp(argv[0], "w", 2) == 0)  {
			for (loop = 0 ; loop < NVT_DDR_PROC_NUM_OF_WRITE_CMD ; loop++) {
				if (strncmp(argv[1], nvt_ddr_proc_cmd_write_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
				        ret = nvt_ddr_proc_cmd_write_list[loop].execute(pmodule_info, ucargc - 2, &argv[2]);
				        break;
				}
			}
	        if (loop >= NVT_DDR_PROC_NUM_OF_WRITE_CMD) {
				goto ERR_INVALID_CMD;
	        }
        } else {
            goto ERR_INVALID_CMD;
        }

        return size;

ERR_INVALID_CMD:
        nvt_dbg(ERR, "Invalid CMD \"%s\"\n", cmd_line);

ERR_OUT:
        return -1;
}

static struct proc_ops proc_ddr_fops = {
	.proc_open   = nvt_ddr_proc_help_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
    	.proc_write   = nvt_ddr_proc_cmd_write
};

#if HEAVYLOAD_EN
static int nvt_dram_heavyload_cfg(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	DRAM_CONSUME_ATTR *dram_consume_cfg = &nvt_heavyload_cfg;
	struct vos_mem_cma_info_t      buf_info = {0};
	int ret = 0;

	dram_consume_cfg->load_degree = DRAM_CONSUME_HEAVY_LOADING;
	dram_consume_cfg->size = 0x100000;

	if (argc > 0) {
		sscanf(pargv[0], "%d", (UINT32 *)&dram_consume_cfg->load_degree);
		sscanf(pargv[1], "%x", (UINT32 *)&dram_consume_cfg->size);
	}

	ret = vos_mem_init_cma_info(&buf_info, VOS_MEM_CMA_TYPE_CACHE, dram_consume_cfg->size);
	if (ret >= 0) {
		handle = vos_mem_alloc_from_cma(&buf_info);
	} else {
		nvt_dbg(WRN, "heavyload allocate mem fail!!\r\n");
		return ret;
	}


	dram_consume_cfg->addr = (ULONG)buf_info.vaddr;
	dram_consume_cfg->is_start = FALSE;

	return 0;
}
static int nvt_dram_heavyload_start(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	DRAM_CONSUME_ATTR *dram_consume_cfg = &nvt_heavyload_cfg;

	if (dram_consume_cfg->is_start) {
		nvt_dbg(WRN, "DRAM HEAVYLOAD IS START\r\n");
		return 0;
	}

	dram_consume_cfg->is_start = TRUE;
#if RANDOM_MODE
	hvy_cnt = 0;
#endif
	set_flg(nvt_dram1_heavyload_flag_id, FLGDBGUT_DMA_HVYLOAD);
    return 0;
}
static int nvt_dram_heavyload_stop(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	DRAM_CONSUME_ATTR *dram_consume_cfg = &nvt_heavyload_cfg;

	if (!dram_consume_cfg->is_start) {
		nvt_dbg(WRN, "DRAM HEAVYLOAD IS NOT START\r\n");
		return 0;
	}
	dma_stop_heavyload(DDR_ARB_1, 0x7);
	dram_consume_cfg->is_start = FALSE;
    return 0;
}

static PROC_CMD nvt_dram_heavyload_cmd_write_list[] = {
	// keyword   function name
	{ "cfg",     nvt_dram_heavyload_cfg},
	{ "start",   nvt_dram_heavyload_start},
	{ "stop",    nvt_dram_heavyload_stop},
};

#define NVT_DRAM_HEAVYLOAD_NUM_OF_WRITE_CMD (sizeof(nvt_dram_heavyload_cmd_write_list) / sizeof(PROC_CMD))

static int nvt_dram_proc_heavyload_show(struct seq_file *sfile, void *v)
{
    nvt_dbg(IND, "\n");
    return 0;
}

static int nvt_dram_proc_heavyload_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_dram_proc_heavyload_show, NULL);
}

static ssize_t nvt_dram_proc_heavyload_cmd_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
        int len = size;
        int ret = -EINVAL;
        char cmd_line[MAX_CMD_LENGTH];
        char *cmdstr = cmd_line;
        const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
        char *argv[MAX_ARG_NUM] = {0};
        unsigned char ucargc = 0;
        unsigned char loop;

        // check command length
        if (len > (MAX_CMD_LENGTH - 1)) {
                nvt_dbg(ERR, "Command length is too long!\n");
                goto ERR_OUT;
        }

        // copy command string from user space
		if (copy_from_user(cmd_line, buf, len)) {
                goto ERR_OUT;
		}

		if (len == 0) {
                cmd_line[0] = '\0';
		} else {
                cmd_line[len - 1] = '\0';
		}

        // parse command string
        for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
                argv[ucargc] = strsep(&cmdstr, delimiters);

                if (argv[ucargc] == NULL)
                    break;
        }

	// dispatch command handler
        if (strncmp(argv[0], "r", 2) == 0) {
			// nothing to do
        } else if (strncmp(argv[0], "w", 2) == 0)  {
			for (loop = 0 ; loop < NVT_DRAM_HEAVYLOAD_NUM_OF_WRITE_CMD ; loop++) {
				if (strncmp(argv[1], nvt_dram_heavyload_cmd_write_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
				        ret = nvt_dram_heavyload_cmd_write_list[loop].execute(NULL, ucargc - 2, &argv[2]);
				        break;
				}
			}
	        if (loop >= NVT_DRAM_HEAVYLOAD_NUM_OF_WRITE_CMD) {
				goto ERR_INVALID_CMD;
	        }
        } else {
            goto ERR_INVALID_CMD;
        }

        return size;

ERR_INVALID_CMD:
        nvt_dbg(ERR, "Invalid CMD \"%s\"\n", cmd_line);

ERR_OUT:
        return -1;
}


static struct proc_ops proc_dram_heavyload_fops = {
    .proc_open   = nvt_dram_proc_heavyload_open,
    .proc_release = single_release,
    .proc_read   = seq_read,
    .proc_lseek = seq_lseek,
    .proc_write   = nvt_dram_proc_heavyload_cmd_write
};
#endif
int nvt_ddr_arb_proc_init(PXXX_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("nvt_drv_sys", NULL);
	if (pmodule_root == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_ddr_root = pmodule_root;


	pentry = proc_create_data("dram_info", S_IRUGO | S_IXUGO, pmodule_root, &proc_ddr_fops, &pdrv_info->module_info);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc dram!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_dram1_entry = pentry;
#if HEAVYLOAD_EN
	pentry = proc_create("dram_heavyload", S_IRUGO | S_IXUGO, pmodule_root, &proc_dram_heavyload_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc dram!\n");
		ret = -EINVAL;
		goto remove_dram1_heavyload_proc;
	}
	pdrv_info->pproc_dram1_heavyload_entry = pentry;
#endif
	//nvt_ddr_proc_cfg.timer_id = TIMER_INVALID;
	OS_CONFIG_FLAG(pdrv_info->module_info.nvt_ddr_proc_flag_id[0]);
	THREAD_CREATE(pdrv_info->module_info.nvt_ddr_proc_tsk_id[0], dbgut_tsk, &pdrv_info->module_info, "nvt_ddr_proc_tsk");
	THREAD_RESUME(pdrv_info->module_info.nvt_ddr_proc_tsk_id[0]);
#if HEAVYLOAD_EN
	OS_CONFIG_FLAG(nvt_dram1_heavyload_flag_id);
	THREAD_CREATE(nvt_dram1_heavyload_tsk_id, heavyload_tsk, NULL, "nvt_dram1_heavyload_tsk");
	THREAD_RESUME(nvt_dram1_heavyload_tsk_id);
#endif



#if (DBG_TEST_EN == 1)
	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
        if (pentry == NULL) {
                nvt_dbg(ERR, "failed to create proc cmd!\n");
                ret = -EINVAL;
                goto remove_cmd;
        }
        pdrv_info->pproc_cmd_entry = pentry;
#endif

	return ret;

#if HEAVYLOAD_EN
remove_dram1_heavyload_proc:
	proc_remove(pdrv_info->pproc_dram1_heavyload_entry);
#endif
remove_cmd:
	proc_remove(pdrv_info->pproc_dram1_entry);

remove_root:
	proc_remove(pdrv_info->pproc_ddr_root);
	return ret;
}

int nvt_ddr_arb_proc_remove(PXXX_DRV_INFO pdrv_info)
{
	FLGPTN flg_ptn = 0;
	ID flag_id = nvt_dram1_heavyload_flag_id;
	set_flg(pdrv_info->module_info.nvt_ddr_proc_flag_id[0], FLGDBGUT_QUIT);
    wai_flg(&flg_ptn, pdrv_info->module_info.nvt_ddr_proc_flag_id[0], FLGDBGUT_QUIT_DONE, TWF_ORW | TWF_CLR);
	 //THREAD_DESTROY(pdrv_info->module_info.nvt_ddr_proc_tsk_id[0]);
printk("%s: rm flag\r\n", __func__);
	rel_flg(pdrv_info->module_info.nvt_ddr_proc_flag_id[0]);
printk("%s: rm proc\r\n", __func__);
#if HEAVYLOAD_EN
	set_flg(nvt_dram1_heavyload_flag_id, FLGDBGUT_QUIT_HVY);
	wai_flg(&flg_ptn, flag_id, FLGDBGUT_DMA_HVYLOAD_DONE, TWF_ORW | TWF_CLR);
	//THREAD_DESTROY(nvt_dram1_heavyload_tsk_id);
	rel_flg(nvt_dram1_heavyload_flag_id);
	proc_remove(pdrv_info->pproc_dram1_heavyload_entry);
#endif
	proc_remove(pdrv_info->pproc_dram1_entry);
	proc_remove(pdrv_info->pproc_ddr_root);
	return 0;
}
