/**
    NVT mmc function
    NVT mmc driver
    @file       nvt_mmc.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include "nvt_mmchost.h"
#include "nvt_mmcreg.h"
#include "nvt_mmc_delay_chain.h"
#include <linux/sysfs.h>
#include <linux/platform_device.h>
#include <linux/mmc/sd.h>
#include <plat/nvt-sramctl.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#endif
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/clk-provider.h>
#if defined (CLK_SPREAD_SUPPORT) || defined (CONFIG_NVT_IVOT_PLAT_NS02201)
#include <linux/soc/nvt/nvt-clk-ext.h>
#endif
#include <plat/pad.h>
#include <plat/top.h>
/* For archs that don't support NO_IRQ (such as mips), provide a dummy value */
#ifndef NO_IRQ
#define NO_IRQ 0
#endif

static unsigned rw_threshold = 32;
module_param(rw_threshold, uint, S_IRUGO);
MODULE_PARM_DESC(rw_threshold,
				 "Read/Write threshold. Default = 32");

static unsigned use_dma = 1;
module_param(use_dma, uint, 0);
MODULE_PARM_DESC(use_dma, "Whether to use DMA or not. Default = 1");

/* Log debug interface */
static unsigned int debug_mask = (1 << SDIO_HOST_ID_COUNT) - 1;
module_param_named(debug_mask, debug_mask, int, 0600);

static unsigned int debug_on = 0;
module_param_named(debug_on, debug_on, uint, 0600);

static unsigned int data_debug_on = 0;
module_param_named(data_debug_on, data_debug_on, uint, 0600);

static unsigned int isr_debug_on = 0;
module_param_named(isr_debug_on, isr_debug_on, uint, 0600);

static unsigned int flow_debug_on = 0;
module_param_named(flow_debug_on, flow_debug_on, uint, 0600);

static unsigned int routine_debug_on = 0;
module_param_named(routine_debug_on, routine_debug_on, uint, 0600);

/* Some cards require more response time. The global variables are only used for debugging */
static unsigned int g_read_timeout_ns_on = 0;
module_param_named(g_read_timeout_ns_on, g_read_timeout_ns_on, int, 0600);

static unsigned int g_read_timeout_ns = 100000000UL;
module_param_named(g_read_timeout_ns, g_read_timeout_ns, uint, 0600);

static unsigned int g_write_timeout_ns_on = 0;
module_param_named(g_write_timeout_ns_on, g_write_timeout_ns_on, int, 0600);

static unsigned int g_write_timeout_ns = 3000000000UL;
module_param_named(g_write_timeout_ns, g_write_timeout_ns, uint, 0600);

#define SDIO_TR(fmt, ...) do { \
		if (debug_on && host && (debug_mask & (1 << host->id))) \
			pr_info("SDIO%d: "fmt, host->id, ##__VA_ARGS__); \
	} while (0)

#define SDIO_DATA(fmt, ...) do { \
		if (data_debug_on && host && (debug_mask & (1 << host->id))) \
			pr_info("SDIO%d: "fmt, host->id, ##__VA_ARGS__); \
	} while (0)

#define SDIO_ISR(fmt, ...) do { \
		if (isr_debug_on && host && (debug_mask & (1 << host->id))) \
			pr_info("SDIO%d: "fmt, host->id, ##__VA_ARGS__); \
	} while (0)

#define SDIO_FLOW(fmt, ...) do { \
		if (flow_debug_on && host && (debug_mask & (1 << host->id))) \
			dev_info(mmc_dev(host->mmc), fmt, ##__VA_ARGS__); \
	} while (0)

#define SDIO_ROUTINE(fmt, ...) do { \
		if (routine_debug_on && host && (debug_mask & (1 << host->id))) \
			dev_info(mmc_dev(host->mmc), fmt, ##__VA_ARGS__); \
	} while (0)

static u32 default_pad_driving[SDIO_MAX_MODE_DRIVING] = {
	PAD_DRIVINGSINK_LEVEL_0, PAD_DRIVINGSINK_LEVEL_0, PAD_DRIVINGSINK_LEVEL_0, \
	PAD_DRIVINGSINK_LEVEL_0, PAD_DRIVINGSINK_LEVEL_0, PAD_DRIVINGSINK_LEVEL_0, \
	PAD_DRIVINGSINK_LEVEL_0, PAD_DRIVINGSINK_LEVEL_0, PAD_DRIVINGSINK_LEVEL_0, \
	PAD_DRIVINGSINK_LEVEL_0, PAD_DRIVINGSINK_LEVEL_0, PAD_DRIVINGSINK_LEVEL_0
};

static u32 default_pad_sdio2_driving[SDIO_MAX_MODE_DRIVING] = {
	PAD_DRIVINGSINK_LEVEL_2, PAD_DRIVINGSINK_LEVEL_2, PAD_DRIVINGSINK_LEVEL_2, \
	PAD_DRIVINGSINK_LEVEL_3, PAD_DRIVINGSINK_LEVEL_2, PAD_DRIVINGSINK_LEVEL_2, \
	PAD_DRIVINGSINK_LEVEL_5, PAD_DRIVINGSINK_LEVEL_4, PAD_DRIVINGSINK_LEVEL_4, \
	PAD_DRIVINGSINK_LEVEL_6, PAD_DRIVINGSINK_LEVEL_5, PAD_DRIVINGSINK_LEVEL_5
};

static u32 default_pad_sdio3_driving[SDIO_MAX_MODE_DRIVING] = {
	PAD_DRIVINGSINK_LEVEL_2, PAD_DRIVINGSINK_LEVEL_2, PAD_DRIVINGSINK_LEVEL_2, \
	PAD_DRIVINGSINK_LEVEL_3, PAD_DRIVINGSINK_LEVEL_2, PAD_DRIVINGSINK_LEVEL_2, \
	PAD_DRIVINGSINK_LEVEL_5, PAD_DRIVINGSINK_LEVEL_4, PAD_DRIVINGSINK_LEVEL_4, \
	PAD_DRIVINGSINK_LEVEL_6, PAD_DRIVINGSINK_LEVEL_5, PAD_DRIVINGSINK_LEVEL_5
};

u32 host_dll_check[3];

#define MAX_CMD_LENGTH              30
#define MAX_ARG_NUM                 6

#define NVT_MMC_IS_TUNING_CMD(cmd)  ((cmd == MMC_SEND_TUNING_BLOCK) || (cmd == MMC_SEND_TUNING_BLOCK_HS200))
#define NVT_MMC_IS_MMC_TIMING(para) ((para == MMC_TIMING_MMC_HS) || (para == MMC_TIMING_MMC_DDR52) || \
									 (para == MMC_TIMING_MMC_HS200) || (para == MMC_TIMING_MMC_HS400))

#if defined(CLK_SPREAD_SUPPORT) 
#if defined(CONFIG_NVT_IVOT_PLAT_NS02302)
char sc_clk_name[] = "pll4";
#elif defined (CONFIG_NVT_IVOT_PLAT_NS02301)
char sc_clk_name[] = "pll9";
#elif defined (CONFIG_NVT_IVOT_PLAT_NA51102)
char sc_clk_name[] = "pll6";
#endif
#endif


static void nvt_mmc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios);

#if 1  // for Brcm drv
static struct mmc_nvt_host *nvt_host[] = {NULL, NULL, NULL, NULL};

static void timer_autogating(struct work_struct *work)
{
	struct delayed_work *delayed_work = to_delayed_work(work);
	struct mmc_nvt_host *host = container_of(delayed_work, struct mmc_nvt_host, autogating_handler);
	bool mmcst1 = false;

	// check if still busy
	mmcst1  = sdiohost_getrdy(host, host->id);
	// if still busy schedule delayed work again
	if (mmcst1 == false) {
		SDIO_FLOW("schedule delayed work again\n");
		schedule_delayed_work(&host->autogating_handler, jiffies + 10);
	} else {
		SDIO_FLOW("turn on autogating\n");
    	clk_set_phase(host->clk, 1);
	}
	return;
}

static void work_disable_autogating(struct work_struct *work)
{
	struct mmc_nvt_host *host = container_of(work, struct mmc_nvt_host, disable_autogating_handler);

	if (spin_is_locked(&host->lock)) {
		dev_err(mmc_dev(host->mmc), "relunch timer\n");
		schedule_work(&host->disable_autogating_handler);
		return;
	} else {
		clk_set_phase(host->clk, 0);
	}
	schedule_delayed_work(&host->autogating_handler, jiffies + 10);
}

void nvt_rescan_card(unsigned id, unsigned insert)
{
	struct mmc_nvt_host *host = NULL;
	printk("%s Enter: in \n", __FUNCTION__);
	BUG_ON(id > 3);
	// BUG_ON(nvt_host[id] == NULL);
	if (nvt_host[id] == NULL) {
		printk("%s nvt_host[%d] is null \n", __FUNCTION__, id);
		return;
	}
	host = nvt_host[id];
	host->mmc->rescan_entered = 0;
	mmc_detect_change(host->mmc, 0);
	return;
}
#endif

static int nvt_mmc_auto_tuning(struct mmc_host *mmc, u32 opcode)
{
    struct mmc_nvt_host *host = mmc_priv(mmc);
    u32 ret;
    
    if (host_dll_check[host->id]) {
		SDIO_FLOW("Auto tuning start (with dll)");
        ret = nvt_mmc_auto_tuning_with_dLL(mmc, opcode);
	} else {
		SDIO_FLOW("Auto tuning start (without dll)");
        ret = nvt_mmc_auto_tuning_phase_unit(mmc, opcode);
	}

    return ret;
}

static void nvt_mmc_hs400_complete(struct mmc_host *mmc)
{
    struct mmc_nvt_host *host = mmc_priv(mmc);
    
    if (host_dll_check[host->id]) {
		SDIO_FLOW("HS400 parameter set (with dll)");
        nvt_mmc_hs400_complete_with_dLL(mmc);
	} else {
		SDIO_FLOW("HS400 parameter set (without dll)"); 
        nvt_mmc_hs400_complete_phase_unit(mmc);
	}
}

static void nvt_mmc_set_outdly(struct mmc_nvt_host *host, struct mmc_ios *ios)
{

	if (ios->timing == MMC_TIMING_MMC_HS400) {
		// set OUTDLY_SEL = 0 since ddr 41phase enabled
		sdiohost_setphyclkoutdly(host, 0);
	} else {
		sdiohost_setphyclkoutdly(host, nvt_mmc_get_outdly(host->id, ios->clock));
	}

	// customer parameter
	if (host->outdly_sel >= 0) {
		sdiohost_setphyclkoutdly(host, host->outdly_sel);
	}
}

static bool nvt_mmc_set_dly_param(struct mmc_nvt_host *host, struct mmc_ios *ios)
{
	bool inhouse_param = 1;

	// inhouse parameter
	if (ios->timing == MMC_TIMING_MMC_HS400) {
		sdiohost_set_ddroutdlyen(host, true, false, false);
		sdiohost_set_ddr41phase(host, true);
		sdiohost_set_cmd41phase(host, true);
		sdiohost_set_ddrstartedge(host, true);
		sdiohost_set_ddrclken(host, true);
		sdiohost_set_datastrobe(host, true);
		// should be always open for hs400
		sdiohost_set_hs400_sync_opt(host, true);

		if (host->ishs400es)
			sdiohost_set_hs400es(host, true);

		sdiohost_set_detdata(host, true);
		sdiohost_set_detclr(host);
	} else {
		sdiohost_set_ddroutdlyen(host, false, false, false);
		sdiohost_set_ddrclken(host, false);
		sdiohost_set_datastrobe(host, false);

		if (ios->timing == MMC_TIMING_MMC_HS200) {
			sdiohost_set_cmd41phase(host, true);
		}
	}

	// set indly to 0 since these speed mode will execute tuning
	if (ios->timing == MMC_TIMING_MMC_HS400 || ios->timing == MMC_TIMING_MMC_HS200 ||
		ios->timing == MMC_TIMING_UHS_SDR104 || ios->timing == MMC_TIMING_UHS_SDR50 ||
		ios->timing == MMC_TIMING_UHS_DDR50) {
		sdiohost_setphyclkindly(host, 0x0);
	} else {
		sdiohost_setphyclkindly(host, nvt_mmc_get_indly(host->id, ios->clock));
	}

	nvt_mmc_set_outdly(host, ios);

	// customer parameter
	if (host->indly_sel >= 0) {
		sdiohost_setphyclkindly(host, host->indly_sel);
	}


	if (host_dll_check[host->id]) {
		// customer parameter for hs200 don't need to tunning
		if (ios->timing == MMC_TIMING_MMC_HS200) {
			if (host->dly_data_phase >= 0 && \
				host->dly_cmd_phase >= 0 && \
				host->dly_cmd_dll >= 0 && \
				host->dly_data_dll >= 0 ) {
				sdiohost_set_phasedata(host, host->dly_data_phase);
				sdiohost_set_phasecmd(host, host->dly_cmd_phase);
				sdiohost_set_cmddll(host, host->dly_cmd_dll);
				sdiohost_set_datadll(host, host->dly_data_dll);
				inhouse_param = 0;
			}
		}
	

		// customer parameter for hs400 don't need to tunning
		if (ios->timing == MMC_TIMING_MMC_HS400) {
			if (host->dly_data_phase >= 0 && \
				host->dly_cmd_phase >= 0 && \
				host->dly_cmd_dll >= 0 && \
				host->dly_data_dll >= 0 ) {
				sdiohost_set_phasedata(host, host->dly_data_phase);
				sdiohost_set_phasecmd(host, host->dly_cmd_phase);
				sdiohost_set_cmddll(host, host->dly_cmd_dll);
				sdiohost_set_datadll(host, host->dly_data_dll);
				inhouse_param = 0;
			}
		}
	} else {
		// customer parameter for hs200 don't need to tunning
		if (ios->timing == MMC_TIMING_MMC_HS200) {
			if (host->dly_data_phase >= 0 && host->dly_phase_unit >= 0 && host->dly_cmd_phase >= 0) {
				sdiohost_set_phasecmd(host, host->dly_cmd_phase);
				sdiohost_set_phasedata(host, host->dly_data_phase);
				sdiohost_set_phaseunit(host, host->dly_phase_unit);
				inhouse_param = 0;
			}
		}

		// customer parameter for hs400 don't need to tunning
		if (ios->timing == MMC_TIMING_MMC_HS400) {
			if (host->dly_data_phase >= 0 && host->dly_phase_unit >= 0 && host->dly_cmd_phase >= 0) {
				sdiohost_set_phasecmd(host, host->dly_cmd_phase);
				sdiohost_set_phasedata(host, host->dly_data_phase);
				sdiohost_set_phaseunit(host, host->dly_phase_unit);
				inhouse_param = 0;
			}
		}

	}

	return inhouse_param;
}

static void nvt_mmc_check_fastboot(void)
{
	u32 m_fastboot = 0x0, preload = 0x0;
	struct device_node *of_node = of_find_node_by_path("/fastboot");
	int ret = 0;

	if (of_node) {
		ret = of_property_read_u32(of_node, "enable", &m_fastboot);
		if (ret) {
			pr_info("get fastboot property: %d, %d\n", ret, m_fastboot);
		}
	}

	of_node = of_find_node_by_path("/fastboot/emmc");

	if (of_node) {
		ret = of_property_read_u32(of_node, "preload", &preload);
		if (ret) {
			pr_info("get preload property: %d, %d\n", ret, preload);
		}
	}

#if !defined(CONFIG_NVT_IVOT_PLAT_NS02301) && !defined(CONFIG_NVT_IVOT_PLAT_NA51103) && !defined(CONFIG_NVT_IVOT_PLAT_NA51090) && !defined(CONFIG_NVT_IVOT_PLAT_NS02401)
	if (m_fastboot && preload) {
		ret = nvt_mmc_check_preload_finish();
		if (ret != 1) {
			pr_err("error with waiting preload %d\n", ret);
		}
	}
#endif
}

/* PIO only */
static void nvt_mmc_sg_to_buf(struct mmc_nvt_host *host)
{
//	host->buffer_bytes_left = sg_dma_len(host->sg);
	host->buffer_bytes_left = host->sg->length;
	host->buffer = sg_virt(host->sg);
	if (host->buffer_bytes_left > host->bytes_left) {
		host->buffer_bytes_left = host->bytes_left;
	}
}

static void nvt_mmc_fifo_data_trans(struct mmc_nvt_host *host, unsigned int n)
{
	if (host->buffer) {
		SDIO_DATA("flush buffer, data_dir(%d) buffer(0x%zx) buffer_bytes_left(0x%x), bytes_left(0x%x)\n",
				  host->data_dir, (size_t)__pa((size_t)host->buffer), host->buffer_bytes_left, host->bytes_left);
		if (host->data_dir == SDIO_HOST_WRITE_DATA) {
			sdiohost_writeblock(host, host->id, (uint8_t *)host->buffer,
								host->buffer_bytes_left);
			host->bytes_left -= host->buffer_bytes_left;
		} else {
			sdiohost_readblock(host, host->id, (uint8_t *)host->buffer,
							   host->buffer_bytes_left);
			host->bytes_left -= host->buffer_bytes_left;
		}
	} else {
		/* Should not be here */ 
		WARN_ON(1);
		printk("[WARN]SDIO%d: buffer is null, data_dir(%d) buffer(0x%zx) buffer_bytes_left(0x%x), bytes_left(0x%x)\n",
			   host->id, host->data_dir, (size_t) __pa((size_t)host->buffer), host->buffer_bytes_left, host->bytes_left);
	}
}

static void nvt_mmc_start_command(struct mmc_nvt_host *host, struct mmc_command *cmd)
{
	u32 cmd_reg = 0;
	unsigned long flags = 0;
	char *s;

	if (host == NULL || cmd == NULL) {
		return;
	}

	switch (mmc_resp_type(cmd)) {
	case MMC_RSP_R1:
		s = ", R1";
		break;
	case MMC_RSP_R1B:
		s = ", R1b";
		break;
	case MMC_RSP_R2:
		s = ", R2";
		break;
	case MMC_RSP_R3:
		s = ", R3/R4";
		break;
	default:
		s = ", Rx";
		break;
	};

	SDIO_TR("CMD%d, arg 0x%08x %s\n", cmd->opcode, cmd->arg, s);

	host->cmd = cmd;

	// reopen autogating when next cmd is cmd13 and check resp is idle
	if (host->final_en_autogating && cmd->opcode == 13) {
		host->cmd_st_check = 1;
	}

	switch (mmc_resp_type(cmd)) {
	case MMC_RSP_R1B: {
		/* There's some spec confusion about when R1B is
		 * allowed, but if the card doesn't issue a BUSY
		 * then it's harmless for us to allow it.
		 */
		/*need to check card busy CARD_BUSY2READY bit or
		 *send _SDIO_SD_SEND_STATUS to check
		*/
		cmd_reg |= SDIO_CMD_REG_NEED_RSP;

		/* card needs clk to update card busy status when 
		 * autogating is enable. So we turn on clk when we   
		 * get R1B command.
		 */
		if (host->final_en_autogating) {
			// turn off timer to avoid race condition.
			cancel_delayed_work(&host->autogating_handler);
			// turn off auto gating to get clk.
			schedule_work(&host->disable_autogating_handler);
		}

	}
	/* FALLTHROUGH */
	case MMC_RSP_R1:        /* 48 bits, CRC */
		cmd_reg |= SDIO_CMD_REG_NEED_RSP;
		break;
	case MMC_RSP_R2:        /* 136 bits, CRC */
		cmd_reg |= SDIO_CMD_REG_LONG_RSP;
		break;
	case MMC_RSP_R3:        /* 48 bits, no CRC */
		cmd_reg |= SDIO_CMD_REG_NEED_RSP;
		// OCR command also need clk to trigger card busy
		if (host->final_en_autogating) {
			// turn off timer to avoid race condition.
			cancel_delayed_work(&host->autogating_handler);
			// turn off auto gating to get clk.
			schedule_work(&host->disable_autogating_handler);
		}
		break;
	default:
		cmd_reg |= 0;
		SDIO_TR("unknown resp_type %04x\n",
				mmc_resp_type(cmd));
		spin_lock_irqsave(&host->lock, flags);
		sdiohost_setinten(host, host->id, SDIO_STATUS_REG_CMD_SEND);
		spin_unlock_irqrestore(&host->lock, flags);
		break;
	}

	/* Set command index */
	cmd_reg |= cmd->opcode;

	sdiohost_sendsdcmd(host, host->id, cmd_reg, cmd->arg);

}

/*----------------------------------------------------------------------*/
static void nvt_mmc_prepare_data(struct mmc_nvt_host *host, struct mmc_request *req)
{
	/*int fifo_lev = (rw_threshold == 32) ? MMCFIFOCTL_FIFOLEV : 0;*/
	unsigned int timeout;
	struct mmc_data *data = req->data;
	struct mmc_ios *ios = &host->mmc->ios;
#if (SDIO_SCATTER_DMA)
	int sg_len;
#endif

	host->data = data;
	host->data_early = 0;

	if (data == NULL) {
		return;
	}

	if (host->read_timeout_ns_on && !(data->flags & MMC_DATA_WRITE)) {
		SDIO_DATA("read_tout_ns_on: replace tout_ns from (%u) to (%u)\n",
				  data->timeout_ns, host->read_timeout_ns);

		data->timeout_ns = host->read_timeout_ns;
	} else if (host->write_timeout_ns_on && (data->flags & MMC_DATA_WRITE)) {
		SDIO_DATA("write_tout_ns_on: replace tout_ns from (%u) to (%u)\n",
				  data->timeout_ns, host->write_timeout_ns);

		data->timeout_ns = host->write_timeout_ns;
	}

	if (g_read_timeout_ns_on && !(data->flags & MMC_DATA_WRITE)) {
		SDIO_DATA("!!!g_read_tout_ns_on: replace tout_ns from (%u) to (%u)\n",
				  data->timeout_ns, g_read_timeout_ns);

		data->timeout_ns = g_read_timeout_ns;
	} else if (g_write_timeout_ns_on && (data->flags & MMC_DATA_WRITE)) {
		SDIO_DATA("!!!g_write_tout_ns_on: replace tout_ns from (%u) to (%u)\n",
				  data->timeout_ns, g_write_timeout_ns);

		data->timeout_ns = g_write_timeout_ns;
	}

	timeout = data->timeout_clks +
			  (data->timeout_ns / host->ns_in_one_cycle);

	sdiohost_setdatatimeout(host, host->id, timeout);
	sdiohost_setblksize(host, host->id, data->blksz);

	host->buffer = NULL;
	host->bytes_left = data->blocks * data->blksz;
	host->sg_len = data->sg_len;
	host->sg = host->data->sg;
	nvt_mmc_sg_to_buf(host);
	host->data_dir = ((data->flags & MMC_DATA_WRITE) ?
					  SDIO_HOST_WRITE_DATA : SDIO_HOST_READ_DATA);

	SDIO_DATA("%s: [bytes_left(0x%x) blocks(%u) blk_addr(%u) blksz(%u)] [tout(0x%x) tout_clks(%u) tout_ns(%u) cycle_ns(%u)]\n",
			  (data->flags & MMC_DATA_WRITE) ? "write" : "read",
			  host->bytes_left, data->blocks, data->blk_addr, data->blksz,
			  timeout, data->timeout_clks, data->timeout_ns, host->ns_in_one_cycle);

	if (nvt_get_chip_id() == CHIP_NA51090) {
		if (ios && (ios->clock < 24000000)) {
			SDIO_FLOW("clock(%dHz) < 24MHz, force to PIO\n", ios->clock);
			host->force_use_pio = true;
		} else {
			host->force_use_pio = false;
		}
	}

	if (host->use_dma && !host->force_use_pio) {
#if (SDIO_SCATTER_DMA)
		int i = 0;
		size_t dma_align = L1_CACHE_BYTES;

		//if (data->sg_len != 1)
		//  printk("sg_len=%d\r\n", data->sg_len);

		/*Check if sg address/length is cache alignment, do pio process if unalignment*/
		for (i = 0; i < host->sg_len; i++) {
			if ((!IS_ALIGNED((size_t)sg_virt(&host->sg[i]), dma_align)) || \
//				(!IS_ALIGNED((size_t)sg_dma_len(&data->sg[i]), dma_align))) {
				(!IS_ALIGNED((size_t)data->sg[i].length, dma_align))) {
				SDIO_DATA("sg unalignment, sg(0x%lx) sg_dma_len(0x%lx) dma_align(0x%lx)\n",
						  (unsigned long)sg_virt(&host->sg[i]), (unsigned long)data->sg[i].length, (unsigned long)dma_align);
				host->do_dma = 0;
				SDIO_DATA("pio buffer(0x%zx) buffer_bytes_left(0x%x)\r\n", (size_t) __pa((size_t)host->buffer), host->buffer_bytes_left);

				sdiohost_setupdatatransferpio(host, host->id, \
											  (uintptr_t)host->buffer, \
											  host->buffer_bytes_left, host->data_dir);
				return;
			}
		}

		memset((void *)&host->tmpdestable[0], 0,
			   sizeof(struct STRG_SEG_DES) * SDIO_DES_TABLE_NUM);

		if (host->data_dir)
			sg_len = dma_map_sg(mmc_dev(host->mmc), data->sg,
								data->sg_len, DMA_FROM_DEVICE);
		else
			sg_len = dma_map_sg(mmc_dev(host->mmc), data->sg,
								data->sg_len, DMA_TO_DEVICE);

		for (i = 0; i < sg_len; i++) {
			host->tmpdestable[i].uisegaddr = sg_dma_address(&data->sg[i]);
//			host->tmpdestable[i].uisegsize = sg_dma_len(&data->sg[i]);
			host->tmpdestable[i].uisegsize = data->sg[i].length;
			SDIO_DATA("segaddr(0x%lx) segsize(0x%x)\n", host->tmpdestable[i].uisegaddr, host->tmpdestable[i].uisegsize);
		}

		/*if (host->data_dir)
		    dma_sync_sg_for_device(mmc_dev(host->mmc), data->sg,
		            sg_len, DMA_FROM_DEVICE);
		else
		    dma_sync_sg_for_device(mmc_dev(host->mmc), data->sg,
		            sg_len, DMA_TO_DEVICE);*/

		/*for (i = 0; i < data->sg_len; i++) {
		    tmpdestable[i].uisegaddr =
		    (uint32_t)sg_virt(&data->sg[i]);
		    tmpdestable[i].uisegsize = sg_dma_len(&data->sg[i]);

		}*/

		sdiohost_setdestab(host->id, (uintptr_t)&host->tmpdestable[0], sg_len,
						   (uint32_t *)host->vuisdio_destab);
		sdiohost_setdesen(host->id, 1);

		SDIO_DATA("destab(0x%zx) sg_len(%d)\n", (size_t) __pa((size_t)host->vuisdio_destab), sg_len);

		//printk("tab addr 0x%x\r\n", (uint32_t)host->vuisdio_destab);
		//printk("tab addr 0x%x, size %d\r\n", tmpdestable[0].uisegaddr, tmpdestable[0].uisegsize);
#endif

		host->do_dma = 1;
		SDIO_DATA("dma buffer(0x%zx) buffer_bytes_left(0x%x)\r\n", (size_t) __pa((size_t)host->buffer), host->buffer_bytes_left);
		//printk("dirty byte 0x%x\r\n", host->buffer[0x41]);

		sdiohost_setupdatatransferdma(host, host->id,
									  (uintptr_t)host->buffer, data->blocks * data->blksz,
									  host->data_dir, (uint32_t *)host->vuisdio_destab);

		/* zero this to ensure we take no PIO paths */
		host->bytes_left = 0;
	} else {
		host->do_dma = 0;
		SDIO_DATA("pio buffer(0x%zx) buffer_bytes_left(0x%x)\r\n", (size_t)__pa((size_t)host->buffer), host->buffer_bytes_left);
		//printk("dirty byte 0x%x\r\n", host->buffer[0x41]);

		sdiohost_setupdatatransferpio(host, host->id,
									  (uintptr_t)host->buffer,
									  host->buffer_bytes_left, host->data_dir);
	}
}

static int nvt_mmc_get_cd(struct mmc_host *mmc);
static void nvt_mmc_request(struct mmc_host *mmc, struct mmc_request *req)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);
	unsigned long timeout;
	u32 mmcst1 = 0;

	if (nvt_mmc_get_cd(mmc) == 0) {
		mmc_detect_change(host->mmc, 0);
		req->cmd->error = -ENOMEDIUM;
		mmc_request_done(mmc, req);
		return;
	}

	if ((clk_get_phase(host->clk)) && (host->id == SDIO_HOST_ID_1) && \
		(req->cmd->opcode == SD_SWITCH_VOLTAGE)) {
		clk_set_phase(host->clk, 0);
	}

	if (req->cmd->opcode == MMC_SEND_STATUS) {
		timeout = jiffies + msecs_to_jiffies(10);
	} else {
		timeout = jiffies + msecs_to_jiffies(500);
	}

	/* Card may still be sending BUSY after a previous operation,
	 * typically some kind of write.  If so, we can't proceed yet.
	 */
	if (req->data) {

		if (host->final_en_autogating) {
			// turn off auto gating to get clk.
			clk_set_phase(host->clk, 0);
		}

		while (time_before(jiffies, timeout)) {
			mmcst1  = sdiohost_getrdy(host, host->id);
			if (mmcst1 == true) {
				if (host->final_en_autogating) {
					clk_set_phase(host->clk, 1);
				}
				break;
			}
			cpu_relax();
		}

		if (mmcst1 == false) {
			dev_err(mmc_dev(host->mmc), "still BUSY...%d\n", req->cmd->opcode);
			req->cmd->error = -ETIMEDOUT;
			mmc_request_done(mmc, req);
			if (host->final_en_autogating) {
				clk_set_phase(host->clk, 1);
			}
			return;
		}
	}

	host->req = req;

	if (req->sbc) {
		nvt_mmc_start_command(host, req->sbc);
		return;
	}

	nvt_mmc_prepare_data(host, req);
	nvt_mmc_start_command(host, req->cmd);
}

static void nvt_mmc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);
	u32 target_clock;

	SDIO_FLOW("clock(%dHz) busmode(%d) powermode(%d) Vdd(%04x) width(%u) timing(%u)\n",
			  ios->clock, ios->bus_mode, ios->power_mode,
			  ios->vdd, 1 << ios->bus_width, ios->timing);

	if (ios->clock > max(400000u, host->mmc->f_min)) {
		host->need_power_cycle = true;
	}

	if (host->need_power_cycle && (ios->clock <= max(400000u, host->mmc->f_min))) {
		if (host->cd_state) {
			/*A power cycle is needed to restart card identification*/
			SDIO_FLOW("power cycle to restart card identification, clock(%dHz) <= f_min(%dHz)\n", ios->clock, max(400000u, host->mmc->f_min));
			sdiohost_power_cycle(host, 500);
		}

		host->need_power_cycle = false;
	}

	/*The power state should be controlled by NVT instead of MMC flows*/
	if (ios->clock == 0) {
		return;
	}

	if (NVT_MMC_IS_MMC_TIMING(ios->timing)) {
		// HS400es have to downgrade frequency in identification phase
		// Otherwise compatibility issue occur
		if (ios->clock != mmc->f_max) {
			target_clock = mmc->f_max;
			while ((target_clock > ios->clock) && (target_clock > mmc->f_min)) {
				target_clock = (target_clock >> 1);
			}
			ios->clock = target_clock;
		}
	} else {
		if ((host->voltage_switch == 0) && (ios->clock >= SDIO_MODE_DS) && \
			(ios->clock != mmc->f_max)) {
			ios->clock = min(ios->clock, mmc->f_max);
		}
	}

	switch (ios->bus_width) {
	case MMC_BUS_WIDTH_8:
		SDIO_FLOW("Enabling 8 bit mode\n");
		sdiohost_setbuswidth(host, host->id, SDIO_BUS_WIDTH8);
		break;
	case MMC_BUS_WIDTH_4:
		SDIO_FLOW("Enabling 4 bit mode\n");
		sdiohost_setbuswidth(host, host->id, SDIO_BUS_WIDTH4);
		break;
	case MMC_BUS_WIDTH_1:
		SDIO_FLOW("Enabling 1 bit mode\n");
		sdiohost_setbuswidth(host, host->id, SDIO_BUS_WIDTH1);
		break;
	}

	/*Set host sampling strategy according to different speeds and modes*/
	if (ios->clock >= 192000000) {
		sdiohost_setpaddriving(host, SDIO_MODE_SDR104);
	} else if (ios->clock >= 160000000) {
		sdiohost_setpaddriving(host, SDIO_MODE_SDR104);
	} else if (ios->clock >= 120000000) {
		sdiohost_setpaddriving(host, SDIO_MODE_SDR104);
	} else if (ios->clock >= 96000000) {
		sdiohost_setpaddriving(host, SDIO_MODE_SDR50);
	} else if (ios->clock >= 50000000) {
		sdiohost_setpaddriving(host, SDIO_MODE_SDR50);
	} else if (ios->clock >= 25000000) {
		sdiohost_setpaddriving(host, SDIO_MODE_HS);
	} else {
		sdiohost_setpaddriving(host, SDIO_MODE_DS);
	}

	sdiohost_setblkfifoen(host, true);

	// If customer assign any delay parameter, disable auto tuning flow
	host->need_tuning = nvt_mmc_set_dly_param(host, ios);

	// for hs400es only set ios to active
	if (host->ishs400es && \
		ios->timing == MMC_TIMING_MMC_HS400) {
		nvt_mmc_hs400_complete(mmc);
	}	

	/*Reset phy to take effect*/
	sdiohost_setphyrst(host);

	host->timing = ios->timing;
	host->bus_mode = ios->bus_mode;

#if defined(CONFIG_NVT_FPGA_EMULATION)
	// FPGA fix to 6M
	ios->clock = 6000000;
#endif
	sdiohost_setbusclk(host, host->id, ios->clock,
					   (uint32_t *)&host->ns_in_one_cycle);

}

static void nvt_mmc_xfer_done(struct mmc_nvt_host *host, struct mmc_data *data)
{
	host->data = NULL;

	if (host->do_dma) {
		if (host->data_dir)
			dma_unmap_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
						 DMA_FROM_DEVICE);
		else
			dma_unmap_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
						 DMA_TO_DEVICE);
	}

	if (!data->stop) {
		SDIO_ISR("%s: !data->stop, call mmc_request_done\r\n", __func__);
		mmc_request_done(host->mmc, data->mrq);
	} else if (host->cmd) {
		if (host->cmd->error) {
			SDIO_ISR("%s: host->cmd->error, call mmc_request_done\r\n", __func__);
			mmc_request_done(host->mmc, data->mrq);
		}
	} else {
		SDIO_ISR("%s: data->stop, call nvt_mmc_start_command\r\n", __func__);
		nvt_mmc_start_command(host, data->stop);
	}
}

static void nvt_mmc_cmd_done(struct mmc_nvt_host *host, struct mmc_command *cmd)
{
	struct mmc_data *data;

	if (host == NULL || cmd == NULL) {
		return;
	}

	data = host->data;

	if ((host->req->sbc) && (host->cmd == host->req->sbc)) {
		host->cmd = NULL;
		host->req->sbc = NULL;
		nvt_mmc_prepare_data(host, host->req);
		nvt_mmc_start_command(host, host->req->cmd);
		return;
	}

	host->cmd = NULL;

	if (cmd->flags & MMC_RSP_PRESENT) {
		if (cmd->flags & MMC_RSP_136) {
			/* response type 2 */
			sdiohost_getlongrsp(host, host->id,
								(uint32_t *)&cmd->resp[0], (uint32_t *)&cmd->resp[1],
								(uint32_t *)&cmd->resp[2], (uint32_t *)&cmd->resp[3]);
			SDIO_TR("LRsp %x %x %x %x\r\n", cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);
		} else {
			/* response types 1, 1b, 3, 4, 5, 6 */
			sdiohost_getshortrsp(host, host->id,
								 (uint32_t *)&cmd->resp[0]);
			SDIO_TR("SRsp %x \r\n", cmd->resp[0]);
		}
	}

	if (host->final_en_autogating) {
		if (host->cmd_st_check && cmd->resp[0] == 0x900) {
			// turn off timer to avoid race condition.
			cancel_delayed_work(&host->autogating_handler);
			// turn off timer to avoid race condition.
			cancel_work_sync(&host->disable_autogating_handler);
			// turn on timer to open autogating
			schedule_delayed_work(&host->autogating_handler, HZ/10);
			host->cmd_st_check = 0;
		}
	}

	if (data && host->data_early) {
		SDIO_ISR("%s: hoat->data, call nvt_mmc_xfer_done\r\n", __func__);
		nvt_mmc_xfer_done(host, data);
	}

	if (data == NULL) {
		SDIO_ISR("%s: cmd done, call mmc_request_done\r\n", __func__);
		mmc_request_done(host->mmc, cmd->mrq);
	} else if (cmd->error) {
		if (cmd->error == -ETIMEDOUT) {
			cmd->mrq->cmd->retries = 0;
			mmc_request_done(host->mmc, cmd->mrq);
			SDIO_ISR("%s: cmd timeout, call mmc_request_done\r\n", __func__);
		} else if (cmd->error == -EILSEQ) {
			cmd->mrq->cmd->retries = 0;
		}
	}
}

static void nvt_mmc_abort_data(struct mmc_nvt_host *host, struct mmc_data *data)
{
	if (host->cmd) {
		pr_info("SDIO%d nvt_mmc_abort_data %c sts 0x%x (CMD%d) \r\n", host->id, \
				host->data_dir ? 'R' : 'W', host->status, host->cmd->opcode);
	} else {
		pr_info("SDIO%d nvt_mmc_abort_data %c sts 0x%x\r\n", host->id, \
				host->data_dir ? 'R' : 'W', host->status);
	}

	sdiohost_resetdata(host, host->id);
}

static irqreturn_t mmc_irq_cd(int irq, void *dev_id)
{
	struct mmc_nvt_host *host = (struct mmc_nvt_host *)dev_id;

	SDIO_ROUTINE("%s:\n", __func__);

	if (host) {
		mmc_detect_change(host->mmc, msecs_to_jiffies(200));
	} else {
		printk("[WARN]: SD ISR %d NULL host\n", irq);
	}

	return IRQ_HANDLED;
}

static irqreturn_t nvt_mmc_irq(int irq, void *dev_id)
{
	struct mmc_nvt_host *host = (struct mmc_nvt_host *)dev_id;
	unsigned int qstatus;
	uintptr_t end_command = 0;
	int end_transfer = 0;
	struct mmc_data *data = NULL;

	if (!host) {
		SDIO_ISR("mmc_nvt_host is null\n");
		return IRQ_HANDLED;
	} else if (!host->mmc) {
		SDIO_ISR("mmc_host is null\n");
		return IRQ_HANDLED;
	}

	data = host->data;

	sdiohost_disinten(host, host->id, SDIO_STATUS_REG_CMD_SEND);
	qstatus = sdiohost_getstatus(host, host->id);
	host->status = qstatus;

	SDIO_ISR("isr status(0x%x)\n", qstatus);

	if (sdiohost_getiointen(host, host->id) && (qstatus & MMCST_SDIO_INT)) {
		sdiohost_setstatus(host, host->id, MMCST_SDIO_INT);
		mmc_signal_sdio_irq(host->mmc);
		goto out;
	}

	if (((qstatus & MMCST_RSP_CRC_OK) || (qstatus & MMCST_CMD_SENT)) && !(qstatus & MMCST_RSP_TIMEOUT)) {
		/* End of command phase */
		if (data == NULL) {
			end_command = (uintptr_t) host->cmd;
		} else {
			if ((host->do_dma == 0) && (host->bytes_left > 0)) {
				/* if datasize < rw_threshold, no RX ints are generated */
				nvt_mmc_fifo_data_trans(host, host->bytes_left);

				/* The data status may change and needs to be updated here */
				qstatus = sdiohost_getstatus(host, host->id);
				host->status = qstatus;
				SDIO_ISR("flush buffer done, update status(0x%x)\n", qstatus);
			}
		}

		if (qstatus & MMCST_RSP_CRC_OK) {
			sdiohost_setstatus(host, host->id, MMCST_RSP_CRC_OK);
		}
		if (qstatus & MMCST_CMD_SENT) {
			sdiohost_setstatus(host, host->id, MMCST_CMD_SENT);
		}
	}

	if (qstatus & MMCST_RSP_TIMEOUT) {
		/* Command timeout */
		if (host->cmd) {
			host->cmd->error = -ETIMEDOUT;
			if (data) {
				end_transfer = 1;
				end_command = 1;
				nvt_mmc_abort_data(host, data);
				data->bytes_xfered = 0;
				data->error = -ETIMEDOUT;
				SDIO_ISR("cmd with data rsp timeout (CMD%d), status(0x%x)\n", host->cmd->opcode, qstatus);
			} else {
				end_command = 1;
				SDIO_ISR("cmd rsp timeout (CMD%d), status(0x%x)\n", host->cmd->opcode, qstatus);
			}
		}

		sdiohost_setstatus(host, host->id, MMCST_RSP_TIMEOUT);
	}

	if (qstatus & MMCST_RSP_CRC_FAIL) {
		/* Command CRC error */
		if (host->cmd) {
			host->cmd->error = -EILSEQ;
			end_command = 1;
			// must print cmd crc error.
			printk("cmd response CRC error (CMD%d), status(0x%x)\n", host->cmd->opcode, qstatus);
		}

		sdiohost_setstatus(host, host->id, MMCST_RSP_CRC_FAIL);
	}

	if (qstatus & MMCST_DATA_TIMEOUT) {
		/* Data timeout */
		if (data) {
			data->error = -ETIMEDOUT;
			end_transfer = 1;
			nvt_mmc_abort_data(host, data);
			if (host->cmd) {
				SDIO_ISR("data timeout (CMD%d), status(0x%x)\n", host->cmd->opcode, qstatus);
			} else {
				SDIO_ISR("data timeout, status(0x%x)\n", qstatus);
			}
		} else {
			if (host->cmd) {
				printk("[WARN]SDIO%d: data timeout with no host->data, (CMD%d), status(0x%x)\n", host->id, host->cmd->opcode, qstatus);
			} else {
				printk("[WARN]SDIO%d: data timeout with no host->data, status(0x%x)\n", host->id, qstatus);
			}
		}

		sdiohost_setstatus(host, host->id, MMCST_DATA_TIMEOUT);
	}

	if (qstatus & MMCST_DATA_CRC_FAIL) {
		/* Data CRC error */
		if (data) {
			end_transfer = 1;
			nvt_mmc_abort_data(host, data);
			if (host->cmd)
				SDIO_ISR("%s data %s error (CMD%d), status(0x%x)\n",
						 (data->flags & MMC_DATA_WRITE) ? "write" : "read",
						 (data->error == -ETIMEDOUT) ? "timeout" : "CRC", host->cmd->opcode, qstatus);
			else
				SDIO_ISR("%s data %s error, status(0x%x)\n",
						 (data->flags & MMC_DATA_WRITE) ? "write" : "read",
						 (data->error == -ETIMEDOUT) ? "timeout" : "CRC", qstatus);
		} else {
			if (host->cmd) {
				printk("[WARN]SDIO%d: data crc error with no host->data, (CMD%d), status(0x%x)\n", host->id, host->cmd->opcode, qstatus);
			} else {
				printk("[WARN]SDIO%d: data crc error with no host->data, status(0x%x)\n", host->id, qstatus);
			}
		}

		sdiohost_setstatus(host, host->id, MMCST_DATA_CRC_FAIL);
	}

	if (qstatus & MMCST_RSP_CRC_OK) {
		end_command = (uintptr_t) host->cmd;
	}

	if ((qstatus & MMCST_DATA_END) || (qstatus & MMCST_DATA_CRC_OK)) {
		if (data) {
			end_transfer = 1;
			if (host->do_dma) {
				data->bytes_xfered = data->blocks * data->blksz;
			} else {
				data->bytes_xfered = host->buffer_bytes_left;
			}

			if (qstatus & MMCST_DATA_CRC_FAIL) {
				data->bytes_xfered = 0;
			}
		} else {
			if (host->cmd) {
				printk("[WARN]SDIO%d: data transfer done with no host->data, (CMD%d), status(0x%x)\n", host->id, host->cmd->opcode, qstatus);
			} else {
				printk("[WARN]SDIO%d: data transfer done with no host->data, status(0x%x)\n", host->id, qstatus);
			}
		}

		if (qstatus & MMCST_DATA_END) {
			sdiohost_setstatus(host, host->id, MMCST_DATA_END);
		}

		if (qstatus & MMCST_DATA_CRC_OK) {
			sdiohost_setstatus(host, host->id, MMCST_DATA_CRC_OK);
		}
	}

	if (qstatus & (MMCST_VOL_SWITCH_END | MMCST_VOL_SWITCH_TIMEOUT)) {
		if (qstatus & MMCST_VOL_SWITCH_TIMEOUT) {
			host->voltage_switch_timeout = 1;
			sdiohost_setstatus(host, host->id, \
							   MMCST_VOL_SWITCH_TIMEOUT);
		}

		if (qstatus & MMCST_VOL_SWITCH_END) {
			sdiohost_setstatus(host, host->id, \
							   MMCST_VOL_SWITCH_END);
		}
		complete(&host->voltage_switch_complete);
	}

	if (end_command) {
		if (host->cmd) {
			nvt_mmc_cmd_done(host, host->cmd);
		}
	}

	if (end_transfer) {
		if (host->cmd) {
			/*
			 * Data managed to finish before the
			 * command completed. Make sure we do
			 * things in the proper order.
			 */
			host->data_early = 1;
			SDIO_ISR("data early, end_command(%ld), end_transfer(%d)\r\n",
					 end_command, end_transfer);
		} else if (data) {
			nvt_mmc_xfer_done(host, data);
		}
	}

out:
	
	return IRQ_HANDLED;
}

static int nvt_mmc_get_cd(struct mmc_host *mmc)
{
	struct platform_device *pdev = to_platform_device(mmc->parent);
	struct nvt_mmc_config *config = pdev->dev.platform_data;

	if (!config || !config->get_cd) {
		return -ENOSYS;
	}
	return config->get_cd(mmc);
}

static int nvt_mmc_get_ro(struct mmc_host *mmc)
{
	struct platform_device *pdev = to_platform_device(mmc->parent);
	struct nvt_mmc_config *config = pdev->dev.platform_data;

	if (!config || !config->get_ro) {
		return -ENOSYS;
	}
	return config->get_ro(mmc);
}

static void nvt_mmc_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);

	if (enable) {
		unsigned int status = sdiohost_getstatus(host, host->id);
		/*pr_info("%x\r\n",status);*/
		if (status & MMCST_SDIO_INT) {
			sdiohost_setstatus(host, host->id, MMCST_SDIO_INT);
			status = sdiohost_getstatus(host, host->id);
			sdiohost_setstatus(host, host->id,
							   status & MMCST_SDIO_INT);
			mmc_signal_sdio_irq(host->mmc);
		} else {
			host->sdio_int = true;
			sdiohost_setiointen(host, host->id, TRUE);
			// sdiohost_getiointen(host, host->id);
		}
	} else {
		host->sdio_int = false;
		sdiohost_setiointen(host, host->id, FALSE);
		// sdiohost_getiointen(host, host->id);
	}
}

static int nvt_start_signal_voltage_switch(struct mmc_host *mmc,
		struct mmc_ios *ios)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);
	unsigned long flags = 0;
	int ret;

	switch (ios->signal_voltage) {
	case MMC_SIGNAL_VOLTAGE_330:
		if (sdiohost_getpower(host) == SDIO_HOST_SETPOWER_VOL_3P3) {
			return 0;
		}

		sdiohost_setpower(host, SDIO_HOST_SETPOWER_VOL_3P3);

		return 0;
	case MMC_SIGNAL_VOLTAGE_180:
		if (sdiohost_getpower(host) == SDIO_HOST_SETPOWER_VOL_1P8) {
			return 0;
		}

		sdiohost_disclockout(host, host->id);

		sdiohost_setpower(host, SDIO_HOST_SETPOWER_VOL_1P8);

		mdelay(400);

		sdiohost_enclockout(host, host->id);
		mdelay(20);

		ret = sdiohost_set_voltage_switch(host);

		if (!(clk_get_phase(host->clk)) && (host->id == SDIO_HOST_ID_1)) {
			if (spin_is_locked(&host->lock)) {
				dev_err(mmc_dev(host->mmc), "temp release spin lock for initial\n");
				spin_unlock_irqrestore(&host->lock, flags);
				clk_set_phase(host->clk, 1);
				spin_lock_irqsave(&host->lock, flags);	
			} else {
				clk_set_phase(host->clk, 1);
			}
		}

		return ret;
	default:
		/* No signal voltage switch required */
		return 0;
	}
}

static int nvt_card_busy(struct mmc_host *mmc)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);

	if (nvt_mmc_get_cd(mmc) == 0) {
		return 0;
	}

	/* Check whether DAT[0] is 0 */
	return !(sdiohost_getrdy(host, host->id));
}

static void nvt_init_card(struct mmc_host *mmc, struct mmc_card *card)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);
	BOOL spin_lock_relock = FALSE;
	unsigned long flags = 0;

	if (spin_is_locked(&host->lock)) {
		dev_err(mmc_dev(host->mmc), "temp release spin lock for initial\n");
		spin_unlock_irqrestore(&host->lock, flags);
		spin_lock_relock = TRUE;
	} 

	if (host->clk && !(host->fixed_autogating)) {
		if (card->type == MMC_TYPE_SDIO || card->type == MMC_TYPE_SD_COMBO) {
			clk_set_phase(host->clk, 0);
			SDIO_FLOW("cardtype(%u) is SDIO, force disable autogating(%d)\n", card->type, clk_get_phase(host->clk));
		} else {
			if (host->en_autogating == 0) {
				clk_set_phase(host->clk, 0);
				SDIO_FLOW("cardtype(%u) is SD/MMC, disable autogating(%d)\n", card->type, clk_get_phase(host->clk));
			} else if (host->en_autogating == 1) {
				clk_set_phase(host->clk, 1);
				SDIO_FLOW("cardtype(%u) is SD/MMC, enable autogating(%d)\n", card->type, clk_get_phase(host->clk));
			} else {
				SDIO_FLOW("cardtype(%u) is SD/MMC, use default autogating(%d)\n", card->type, clk_get_phase(host->clk));
			}
		}
	} else {
		SDIO_FLOW("bypass switch autogating, fixed_autogating(%d) clk(0x%p)\n", host->fixed_autogating, host->clk);
	}

	host->final_en_autogating = clk_get_phase(host->clk);
	SDIO_FLOW("st_autogating(%d) clk(0x%p)\n", host->final_en_autogating , host->clk);

	if (spin_lock_relock) 
		spin_lock_irqsave(&host->lock, flags);	
	
}

static int nvt_mmc_select_driver_strength(struct mmc_card *card,
		unsigned int max_dtr, int host_drv,
		int card_drv, int *drv_type)
{
	struct mmc_host *mmc = card->host;
	struct mmc_nvt_host *host = mmc_priv(mmc);
	int drive_strength = 0;

	if (host->strobe_driving && (card_drv & (1 << host->strobe_driving))) {
		drive_strength = host->strobe_driving;
		pr_info("mmc%d set peer device driver strength %d\n", host->id, drive_strength);
	}

	return drive_strength;
}

static struct mmc_host_ops nvt_mmc_ops = {
	.request    = nvt_mmc_request,
	.set_ios    = nvt_mmc_set_ios,
	.get_cd     = nvt_mmc_get_cd,
	.get_ro     = nvt_mmc_get_ro,
	.enable_sdio_irq = nvt_mmc_enable_sdio_irq,
	.start_signal_voltage_switch = nvt_start_signal_voltage_switch,
	.execute_tuning = nvt_mmc_auto_tuning,
	.card_busy  = nvt_card_busy,
	.init_card  = nvt_init_card,
	.select_drive_strength = nvt_mmc_select_driver_strength,
};

static struct mmc_host_ops nvt_fixed_mmc_ops = {
	.request    = nvt_mmc_request,
	.set_ios    = nvt_mmc_set_ios,
	.get_cd     = nvt_mmc_get_cd,
	.get_ro     = nvt_mmc_get_ro,
	.enable_sdio_irq = nvt_mmc_enable_sdio_irq,
	.execute_tuning = nvt_mmc_auto_tuning,
	.card_busy  = nvt_card_busy,
	.init_card  = nvt_init_card,
	.select_drive_strength = nvt_mmc_select_driver_strength,
	.hs400_complete = nvt_mmc_hs400_complete,
	//.prepare_hs400_tuning = nvt_prepare_hs400_tuning,  /* TODO: we do not support mmc_retune now */
	//.hs400_enhanced_strobe = nvt_mmc_hs400_enhanced_strobe,  /* TODO: we do not support enhance strobe now */
};

/*----------------------------------------------------------------------*/
static void __init init_mmcsd_host(struct mmc_nvt_host *host, int voltage_switch)
{
	sdiohost_open(host, host->id, voltage_switch);
}

int nvt_mmc_get_cd_polled(struct mmc_host *mmc)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);
	struct mmc_ios temp_ios;
	int cd_val;

	// return to default ios to avoid card seeing high-speed clock before card identification
	temp_ios.clock = 400000;
	temp_ios.bus_width = MMC_BUS_WIDTH_1;
	temp_ios.timing = MMC_TIMING_LEGACY;

	cd_val = gpio_get_value(host->cd_gpio);

	SDIO_ROUTINE("%s: cd_gpio(0x%x) cd_val(%d)\n", __func__, host->cd_gpio, cd_val);
	if (host->cd_gpio && (host->fake_cd == -1)) {
		if (host->power_en) {
			if (gpio_get_value(host->power_en) != host->power_detect_edge) {
				sdiohost_power_switch(host, 0);
				return 0;
			} else {
				sdiohost_power_switch(host, 1);
			}
		}

		if (cd_val == host->cd_detect_edge) {
			if (!host->cd_state) {
				// bypass phy reset to avoid timeout caused by SD clock not output
				host->bypass_phyrst = true;
				nvt_mmc_set_ios(mmc, &temp_ios);
				host->bypass_phyrst = false;
				if (host->cp_gpio) {
					sdiohost_power_cycle(host, host->force_power_cycle_period);
				} else {
					sdiohost_power_up(host);
				}
			}

			host->cd_state = 1;
			return 1;
		} else {
			if (host->cd_state) {
				sdiohost_power_down(host);
			}

			host->cd_state = 0;
			return 0;
		}
	} else {
		if (host->fake_cd == 1) {
			SDIO_ROUTINE("%s: fake_cd(%d) power up\n", __func__, host->fake_cd);

			nvt_mmc_set_ios(mmc, &temp_ios);
			if (host->cp_gpio) {
				sdiohost_power_cycle(host, host->force_power_cycle_period);
			} else {
				sdiohost_power_up(host);
			}

			host->cd_state = 1;
			return 1;
		} else if (host->fake_cd == 0) {
			SDIO_ROUTINE("%s: fake_cd(%d) power down\n", __func__, host->fake_cd);

			sdiohost_power_down(host);

			host->cd_state = 0;
			return 0;
		}

		return 1;
	}
}

int nvt_mmc_get_ro_polled(struct mmc_host *mmc)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);

	SDIO_ROUTINE("nvt_mmc_get_ro! 0\n");
	if (host->ro_gpio) {
		return ((gpio_get_value(host->ro_gpio) == host->ro_detect_edge) ? 1 : 0);
	} else {
		return 0;
	}
}

/* rescan property */
static ssize_t nvtmmc_sysfs_set_rescan(struct device *dev,
									   struct device_attribute *attr,
									   const char *buf,
									   size_t count)
{
	int is_rescan = 0;
	struct mmc_nvt_host *host = NULL;

	host = dev_get_drvdata(dev);
	if (!host) {
		return -EINVAL;
	}

	if (kstrtoint(buf, 10, &is_rescan) < 0) {
		return -EINVAL;
	}

	if (is_rescan < 2) {
		nvt_rescan_card(host->id, is_rescan);
	} else {
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR(rescan, S_IWUSR | S_IRUGO,
				   NULL, nvtmmc_sysfs_set_rescan);

/* fake card detection property */
static ssize_t nvt_sysfs_set_fake_cd(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf,
				     size_t count)
{
	int fake_cd = -1;
	struct mmc_nvt_host *host = NULL;

	host = dev_get_drvdata(dev);
	if (!host) {
		return -EINVAL;
	}

	if (kstrtoint(buf, 10, &fake_cd) < 0) {
		return -EINVAL;
	}

	if (fake_cd == 1) {
		dev_info(mmc_dev(host->mmc), "set [fake_cd = 1] to power up\n");
		host->fake_cd = fake_cd;
		mmc_detect_change(host->mmc, 0);
	} else if (fake_cd == 0) {
		dev_info(mmc_dev(host->mmc), "set [fake_cd = 0] to power down\n");
		host->fake_cd = fake_cd;
		mmc_detect_change(host->mmc, 0);
	} else {
		dev_info(mmc_dev(host->mmc), "set [fake_cd = -1] to disable feature\n");
		host->fake_cd = -1;
		mmc_detect_change(host->mmc, 0);
	}

	return count;
}

static DEVICE_ATTR(fake_cd, S_IWUSR | S_IRUGO,
				   NULL, nvt_sysfs_set_fake_cd);

/* mmc clk driving property */
static ssize_t nvt_sysfs_set_mmc_clk_driving(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf,
				     size_t count)
{
	struct mmc_nvt_host *host = NULL;
	PAD_DRIVINGSINK driving = 0;
	uint32_t pad_ds;
	int temp_driving = -1;
	ER ret = E_OK;

	host = dev_get_drvdata(dev);
	if (!host) {
		return -EINVAL;
	}

	if (kstrtoint(buf, 10, &temp_driving) < 0) {
		// get clk pad_driving
        pad_ds = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_DS, host->mmc_pinmux, SDIO_PIN_CLK);
		if (pad_ds != 0)
        	ret = pad_get_drivingsink(pad_ds, &driving);
		if (ret < 0)
			return ret;

		dev_info(mmc_dev(host->mmc), "clk driving is %d\n", driving);
		return count;
	}

	if (CHECK_SDIO_SUP(host->id)) {
		if (temp_driving > 0)
			driving = temp_driving;
		else {
			dev_info(mmc_dev(host->mmc), "please enter driving > 0\n");
			return ret;
		}

		dev_info(mmc_dev(host->mmc), "set %d to clk\n", driving);
		// set cmd pad_driving
        pad_ds = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_DS, host->mmc_pinmux, SDIO_PIN_CLK);
		if (pad_ds != 0)
        	ret = pad_set_drivingsink(pad_ds, driving);
		if (ret < 0) {
			dev_info(mmc_dev(host->mmc), "not support\n");
			return ret;
		}
	} else {
		dev_info(mmc_dev(host->mmc), "not support\n");
		return -EINVAL;
	}

	return count;
}

/* mmc cmd driving property */
static ssize_t nvt_sysfs_set_mmc_cmd_driving(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf,
				     size_t count)
{
	struct mmc_nvt_host *host = NULL;
	PAD_DRIVINGSINK driving = 0;
	uint32_t pad_ds;
	int temp_driving = -1;
	ER ret = E_OK;

	host = dev_get_drvdata(dev);
	if (!host) {
		return -EINVAL;
	}

	if (kstrtoint(buf, 10, &temp_driving) < 0) {
		// get cmd pad_driving
        pad_ds = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_DS, host->mmc_pinmux, SDIO_PIN_CMD);
		if (pad_ds != 0)
        	ret = pad_get_drivingsink(pad_ds, &driving);
		if (ret < 0)
			return ret;

		dev_info(mmc_dev(host->mmc), "cmd driving is %d\n", driving);
		return count;
	}

	if (CHECK_SDIO_SUP(host->id)) {
		if (temp_driving > 0)
			driving = temp_driving;
		else {
			dev_info(mmc_dev(host->mmc), "please enter driving > 0\n");
			return ret;
		}

		dev_info(mmc_dev(host->mmc), "set %d to cmd\n", driving);
		// set cmd pad_driving
        pad_ds = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_DS, host->mmc_pinmux, SDIO_PIN_CMD);
		if (pad_ds != 0)
        	ret = pad_set_drivingsink(pad_ds, driving);
		if (ret < 0) {
			dev_info(mmc_dev(host->mmc), "not support\n");
			return ret;
		}
	} else {
		dev_info(mmc_dev(host->mmc), "not support\n");
		return -EINVAL;
	}

	return count;
}

/* mmc data driving property */
static ssize_t nvt_sysfs_set_mmc_data_driving(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf,
				     size_t count)
{
	struct mmc_nvt_host *host = NULL;
	PAD_DRIVINGSINK driving = 0;
	uint32_t pad_ds;
	uint8_t pin_num = SDIO_PIN_NUM_4_BIT;
	int temp_driving = -1;
	int i = 0;
	ER ret = E_OK;

	host = dev_get_drvdata(dev);
	if (!host) {
		return -EINVAL;
	}

	if (kstrtoint(buf, 10, &temp_driving) < 0) {
		if ((host->mmc->caps & MMC_CAP_8_BIT_DATA) &&
            CHECK_SDIO_8_BITS_SUP(host->id)) {
            pin_num = SDIO_PIN_NUM_8_BIT;
        }
		// get data pad_driving
        for (i = SDIO_PIN_DATA0; i < pin_num; i++)
        {
            pad_ds = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_DS, host->mmc_pinmux, i);
			if (pad_ds != 0)
            	ret = pad_get_drivingsink(pad_ds, &driving);
			if (ret < 0) {
				dev_info(mmc_dev(host->mmc), "not support\n");
				return ret;
			}
			dev_info(mmc_dev(host->mmc), "data[%d] driving is %d\n",i ,driving);
        }
		return count;
	}

	if (CHECK_SDIO_SUP(host->id)) {
		if (temp_driving > 0)
			driving = temp_driving;
		else {
			dev_info(mmc_dev(host->mmc), "please enter driving > 0\n");
			return ret;
		}

		dev_info(mmc_dev(host->mmc), "set %d to data\n", driving);
		if ((host->mmc->caps & MMC_CAP_8_BIT_DATA) &&
            CHECK_SDIO_8_BITS_SUP(host->id)) {
            pin_num = SDIO_PIN_NUM_8_BIT;
        }
		// set data pad_driving
        for (i = SDIO_PIN_DATA0; i < pin_num; i++)
        {
            pad_ds = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_DS, host->mmc_pinmux, i);
			if (pad_ds != 0)
            	ret = pad_set_drivingsink(pad_ds, driving);
			if (ret < 0) {
				dev_info(mmc_dev(host->mmc), "not support\n");
				return ret;
			}
        }
	} else {
		dev_info(mmc_dev(host->mmc), "not support\n");
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR(clk_driving, S_IWUSR | S_IRUGO,
				   NULL, nvt_sysfs_set_mmc_clk_driving);
static DEVICE_ATTR(cmd_driving, S_IWUSR | S_IRUGO,
				   NULL, nvt_sysfs_set_mmc_cmd_driving);
static DEVICE_ATTR(data_driving, S_IWUSR | S_IRUGO,
				   NULL, nvt_sysfs_set_mmc_data_driving);

static struct attribute *nvtmmc_sysfs_attributes[] = {
	&dev_attr_rescan.attr,
	&dev_attr_fake_cd.attr,
	&dev_attr_clk_driving.attr,
	&dev_attr_cmd_driving.attr,
	&dev_attr_data_driving.attr,
	NULL,
};

static const struct attribute_group nvtmmc_sysfs_attr_group = {
	.attrs = nvtmmc_sysfs_attributes,
};

static int nvt_mmc_sysfs_init(struct device *dev)
{
	return sysfs_create_group(&dev->kobj, &nvtmmc_sysfs_attr_group);
}

static void nvt_mmc_sysfs_remove(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &nvtmmc_sysfs_attr_group);
}

int nvt_mmcsd_probe(struct platform_device *pdev, const struct of_device_id *of_dt_ids, const char *proc_path)
{
	struct mmc_nvt_host *host = NULL;
	struct mmc_host *mmc = NULL;
	struct resource *r, *mem = NULL;
	struct device_node *top_node, *pinmux_node;
	int ret = 0, irq = 0, i;
	u32 cd_gpio[3] = {0}, ro_gpio[3] = {0}, power_en[2] = {0};
	u32 card_power_gpio[2] = {0};
	u32 ldo_gpio[2] = {0};
	u32 force_power_cycle[2] = {0};
	u32 power_down_delay_ms = 0;
	u32 power_up_delay_ms = 0;
	u32 power_up_clk_period_ms = 0;
	u32 fixed_autogating = 0;
	int en_autogating = -1;
	u32 read_timeout_ns_on = 0, read_timeout_ns = 0;
	u32 write_timeout_ns_on = 0, write_timeout_ns = 0;
	u32 pad_driving[SDIO_MAX_MODE_DRIVING] = {0};
	u32 ss_clk[3] = {0};
	size_t mem_size;
	u32 voltage_switch = 0, max_voltage = 0, sample_edge = 0;
	u32 strobe_driving = 0;
	u32 prop_array[12];
	u32 prop_val;
	u32 is1v8wifi = 0;
	int indly_sel = -1, outdly_sel = -1;
	int dly_cmd_phase = -1, dly_data_phase = -1;

	int dly_cmd_dll = -1, dly_data_dll = -1;
	bool ishs400es = FALSE;
	int dly_phase_unit = -1;

	int idx;
	int mmc_index = -1;
	const char *clk_parent = NULL;
	const char *dly_scan_partition = NULL;

#ifndef CONFIG_OF
	struct nvt_mmc_config *pdata = pdev->dev.platform_data;
#else
	struct nvt_mmc_config *pdata = NULL;
	const struct of_device_id *of_id;

	nvt_mmc_check_fastboot();

	nvt_mmc_chain_type();

	/* REVISIT:  when we're fully converted, fail if pdata is NULL */
	of_id = of_match_device(of_dt_ids, &pdev->dev);
	if (!of_id) {
		dev_err(&pdev->dev, "[NVT MMC] OF not found\n");
		return -EINVAL;
	}
	pdata = (struct nvt_mmc_config *) of_id->data;
	pdev->dev.platform_data = pdata;

	ret = dma_set_mask(&pdev->dev, DMA_BIT_MASK(36));
	if (ret != 0) {
		dev_err(&pdev->dev, "set dma mask fail\n");
		return -ENXIO;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "voltage-switch",
							   &prop_val);
	if (ret == 0) {
		voltage_switch = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "max-voltage",
							   &prop_val);
	if (ret == 0) {
		max_voltage = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "neg-sample-edge",
							   &prop_val);
	if (ret == 0) {
		sample_edge = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "indly_sel",
							   &prop_val);
	if (ret == 0) {
		indly_sel = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "outdly_sel",
							   &prop_val);
	if (ret == 0) {
		outdly_sel = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "power_down_delay_ms",
							   &prop_val);
	if (ret == 0) {
		power_down_delay_ms = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "power_up_delay_ms",
							   &prop_val);
	if (ret == 0) {
		power_up_delay_ms = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "power_up_clk_period_ms",
							   &prop_val);
	if (ret == 0) {
		power_up_clk_period_ms = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "fixed_autogating",
							   &prop_val);
	if (ret == 0) {
		fixed_autogating = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "en_autogating",
							   &prop_val);
	if (ret == 0) {
		en_autogating = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "read_timeout_ns_on",
							   &prop_val);
	if (ret == 0) {
		read_timeout_ns_on = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "read_timeout_ns",
							   &prop_val);
	if (ret == 0) {
		read_timeout_ns = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "write_timeout_ns_on",
							   &prop_val);
	if (ret == 0) {
		write_timeout_ns_on = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "write_timeout_ns",
							   &prop_val);
	if (ret == 0) {
		write_timeout_ns = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "dly_cmd_phase", &prop_val);
	if (ret == 0) {
		dly_cmd_phase = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "dly_data_phase", &prop_val);
	if (ret == 0) {
		dly_data_phase = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "dly_cmd_dll", &prop_val);
	if (ret == 0) {
		dly_cmd_dll = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "dly_data_dll", &prop_val);
	if (ret == 0) {
		dly_data_dll = prop_val;
	}

	ishs400es = device_property_read_bool(&pdev->dev, "mmc-hs400-enhanced-strobe");

	ret = of_property_read_u32(pdev->dev.of_node, "dly_phase_unit", &prop_val);
	if (ret == 0) {
		dly_phase_unit = prop_val;
	}

	/* To specified mmcblkX, X is mmc_index */
	ret = of_property_read_u32(pdev->dev.of_node, "mmc_index", &prop_val);
	if (ret == 0) {
		mmc_index = prop_val;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "strobe_driving", &prop_val);
	if (ret == 0) {
		strobe_driving = prop_val;
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, "cd_gpio", prop_array, 3);
	if (ret == 0) {
		memcpy((void *)cd_gpio, prop_array, sizeof(u32) * 3);
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, "ro_gpio", prop_array, 3);
	if (ret == 0) {
		memcpy((void *)ro_gpio, prop_array, sizeof(u32) * 3);
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, "power_en", prop_array, 2);
	if (ret == 0) {
		memcpy((void *)power_en, prop_array, sizeof(u32) * 2);
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, \
									 "card_power_gpio", prop_array, 2);
	if (ret == 0) {
		memcpy((void *)card_power_gpio, prop_array, sizeof(u32) * 2);
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, \
									 "ldo_gpio", prop_array, 2);
	if (ret == 0) {
		memcpy((void *)ldo_gpio, prop_array, sizeof(u32) * 2);
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, "force_power_cycle", prop_array, 2);
	if (ret == 0) {
		memcpy((void *)force_power_cycle, prop_array, sizeof(u32) * 2);
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, "driving", prop_array, 12);
	if (ret == 0) {
		memcpy((void *)pad_driving, prop_array, sizeof(u32) * 12);
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, "ss_clk", prop_array, 3);
	if (ret == 0) {
		memcpy((void *)ss_clk, prop_array, sizeof(u32) * 3);
	}

	ret = of_property_read_string(pdev->dev.of_node, "clk_parent", &clk_parent);
	if (ret) {
		dev_dbg(&pdev->dev, "mmc get clk_parent failed\n");
	}

	ret = of_property_read_string(pdev->dev.of_node, "dly_scan_partition", &dly_scan_partition);
	if (ret) {
		dev_dbg(&pdev->dev, "mmc get dly_scan_partition failed\n");
	}

	idx = of_alias_get_id(pdev->dev.of_node, "mmc");
#endif

	ret = -ENODEV;
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	if (!r || irq == NO_IRQ) {
		goto out;
	}

	ret = -EBUSY;
	mem_size = resource_size(r);
	mem = request_mem_region(r->start, mem_size, pdev->name);
	if (!mem) {
		goto out;
	}


	ret = -ENOMEM;
	mmc = mmc_alloc_host(sizeof(struct mmc_nvt_host), &pdev->dev);
	if (!mmc) {
		goto out;
	}

	host = mmc_priv(mmc);
	host->mmc = mmc;    /* Important */
	host->mem_res = mem;
	host->base = ioremap(mem->start, mem_size);/*mem->start;*/
	if (!host->base) {
		goto out;
	}

	if (idx >= 0) {
		if (mmc_index >= 0) {
			mmc->index = mmc_index;
		} else {
			mmc->index = idx;
		}

		host->id = idx;

		// todo
		//if (host->id == (SDIO_HOST_ID_COUNT - 1)) {
		//  nvt_disable_sram_shutdown(SDIO2_SD);
		//}
	} else {
		dev_err(&pdev->dev, "mmc of_alias_get_id failed\n");
		goto out;
	}

#if defined (CONFIG_NVT_IVOT_PLAT_NS02301)
	// turn on descript mode
	if (host->id == SDIO_HOST_ID_3) {
		sdiohost_set_swreset(host);
	}
#endif

	// coverity[side_effect_free]: spin_lock_init is kernel API, do not change
	spin_lock_init(&host->lock);
	spin_lock_init(&host->int_mask_lock);

	//printk("host id = %d, mmc index = %d\r\n", host->id, mmc->index);

	ret = -ENXIO;

	for (i = 0; i < SDIO_MAX_MODE_DRIVING; i++) {
		if (pad_driving[i]) {
			host->pad_driving[i] = pad_driving[i];
		} else {
			if (host->id == SDIO_HOST_ID_1) {
				host->pad_driving[i] = default_pad_driving[i];
			} else if (host->id == SDIO_HOST_ID_2) {
				host->pad_driving[i] = default_pad_sdio2_driving[i];
			} else if (host->id == SDIO_HOST_ID_3) {
				host->pad_driving[i] = default_pad_sdio3_driving[i];
			}
		}
	}

	host->use_dma = use_dma;
	host->force_use_pio = false;
	host->mmc_irq = irq;
	host->mmc_cd_irq = gpio_to_irq(cd_gpio[0]);
	host->cd_gpio = cd_gpio[0];
	host->cd_detect_edge = cd_gpio[1];
	host->fake_cd = -1;
	host->ro_gpio = ro_gpio[0];
	host->ro_detect_edge = ro_gpio[1];
	host->power_en = power_en[0];
	host->power_detect_edge = power_en[1];
	host->cp_gpio = card_power_gpio[0];
	host->cp_gpio_value = card_power_gpio[1];
	host->ldo_gpio = ldo_gpio[0];
	host->ldo_gpio_value = ldo_gpio[1];
	host->force_power_cycle = force_power_cycle[0];
	host->force_power_cycle_period = force_power_cycle[1];
	host->voltage_switch = voltage_switch;
	host->neg_sample_edge = sample_edge;
	host->indly_sel = indly_sel;
	host->outdly_sel = outdly_sel;
	host->power_down_delay_ms = power_down_delay_ms;
	host->power_up_delay_ms = power_up_delay_ms;
	host->power_up_clk_period_ms = power_up_clk_period_ms;
	host->fixed_autogating = fixed_autogating;
	host->en_autogating = en_autogating;
	host->read_timeout_ns_on = read_timeout_ns_on;
	host->read_timeout_ns = read_timeout_ns;
	host->write_timeout_ns_on = write_timeout_ns_on;
	host->write_timeout_ns = write_timeout_ns;
	host->dly_cmd_phase = dly_cmd_phase;
	host->dly_data_phase = dly_data_phase;

	if (host_dll_check[host->id]) {
		host->dly_cmd_dll= dly_cmd_dll;
		host->dly_data_dll = dly_data_dll;
		host->ishs400es = ishs400es;
	} else {
		host->ishs400es = FALSE;
		host->dly_phase_unit = dly_phase_unit;
	}
	host->strobe_driving = strobe_driving;
	host->host_power = SDIO_HOST_SETPOWER_VOL_0;

	/* Initialize CG */
	host->clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (!IS_ERR(host->clk)) {
		/* Set parent by dts */
		if (clk_parent) {
			if (clk_set_parent(host->clk, clk_get(NULL, clk_parent))) {
				dev_err(mmc_dev(host->mmc), "%s: dts set clk_parent failed\n", __func__);
			} else {
				dev_info(mmc_dev(host->mmc), "%s: dts set clk_parent to %s\n", __func__, clk_parent);
			}
		}

#if defined (CLK_SPREAD_SUPPORT)
		/* Setup Spread Spectrum Clock*/
		if (ss_clk[0]) {
			struct clk* sc_clk = clk_get(NULL, sc_clk_name);

			if (!IS_ERR(sc_clk)) {
				clk_set_rate(sc_clk, ss_clk[0]);
					
				nvt_spread_spectrum(sc_clk, ss_clk[1], 1, ss_clk[2]);
				clk_set_parent(host->clk, sc_clk);
			} else {
				dev_err(mmc_dev(host->mmc), "%s: %s not found\n", sc_clk_name, __func__);
				goto out;
			}
		}
#endif
		/* Set to 312500 Hz to guarantee init freq should <=400Khz TG1.1-15 */
		sdiohost_setbusclk(host, host->id, 312500, NULL);
		clk_prepare(host->clk);
		clk_enable(host->clk);
		INIT_DELAYED_WORK(&host->autogating_handler, timer_autogating);
		INIT_WORK(&host->disable_autogating_handler, work_disable_autogating);

	} else {
		dev_err(mmc_dev(host->mmc), "%s: %s not found\n", __func__, dev_name(&pdev->dev));
		goto out;
	}

	/* Set PAD Power */
	if (max_voltage < VOLTAGE_3300) {
		sdiohost_setpower(host, SDIO_HOST_SETPOWER_VOL_1P8);
	} else {
		sdiohost_setpower(host, SDIO_HOST_SETPOWER_VOL_3P3);
	}
	
	/* Initialize TOP */
	sdiohost_set_paddirection(host);

#ifdef CONFIG_OF
	/* get property 'pinmux' on "/top@2,f0010000/sdio" or "/top@f0010000/sdio" */
	top_node = of_find_node_by_name(NULL, "top");
	if (top_node) {
		pinmux_node = of_find_node_by_name(top_node, "sdio");
		if (pinmux_node) {
			if (of_property_read_u32(pinmux_node, "pinmux", &host->mmc_pinmux)) {
				dev_err(mmc_dev(host->mmc), "%s: get pinmux error\n", __func__);
			}

			of_node_put(pinmux_node);  // release node
		} else {
			dev_err(mmc_dev(host->mmc), "%s: get top/sdio node error\n", __func__);
		}

		of_node_put(top_node);  // release node
	} else {
		dev_err(mmc_dev(host->mmc), "%s: get top node error\n", __func__);
	}
#endif

	if (host->cd_gpio) {
		host->cd_state = (gpio_get_value(host->cd_gpio) == host->cd_detect_edge) ? 1 : 0;

		if (!host->cd_state) {
			sdiohost_power_down(host);
			/*Disable card power if card power was defined*/
			if (host->cp_gpio) {
				gpio_direction_output(host->cp_gpio, !host->cp_gpio_value);
			}
		} else {
			/*Enable card power if card power was defined*/
			if (host->cp_gpio) {
				gpio_direction_output(host->cp_gpio, host->cp_gpio_value);
			}
		}
	} else {
		host->cd_state = 1;
		/*Enable card power if card power was defined*/
		if (host->cp_gpio) {
			gpio_direction_output(host->cp_gpio, host->cp_gpio_value);
		}
	}

	// voltage_switch = 1 && max_voltage = 1v8 => 1v8 wifi
	if (voltage_switch && (max_voltage < VOLTAGE_3300)) {
		is1v8wifi = 1;
	}

	/* Initialize SD controller */
	init_mmcsd_host(host, voltage_switch);

	if (host->cd_state && host->force_power_cycle) {
		if (host->cp_gpio) {
			/* In general, the power cycle takes 500ms to 1s */
			dev_info(mmc_dev(host->mmc), "%s: Enable force power cycle, off-on transition delay (%dms)\n", __func__, host->force_power_cycle_period);
			sdiohost_power_cycle(host, host->force_power_cycle_period);
		} else {
			dev_info(mmc_dev(host->mmc), "%s: Enable force power cycle failed, cp_gpio(%d) is not defined\n", __func__, host->cp_gpio);
		}
	} else if (host->force_power_cycle_period) {
		dev_info(mmc_dev(host->mmc), "%s: Disable force power cycle, off-on transition (%dms)\n", __func__, host->force_power_cycle_period);
	}

	// enable the clock output after card detection
	sdiohost_enclockout(host, host->id);

	/* Initialize mmc framework */
	mmc->f_min = 312500;
	mmc->f_max = 24000000;
#ifndef CONFIG_OF
	if (pdata && (pdata->wires == 4 || pdata->wires == 0)) {
		mmc->caps |= MMC_CAP_4_BIT_DATA;
	}

	if (pdata && (pdata->wires == 8)) {
		mmc->caps |= (MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA);
	}

	if (pdata && pdata->max_freq) {
		mmc->f_max = pdata->max_freq;
	}
#else
	mmc_of_parse(host->mmc);
#endif

	/* REVISIT:  someday, support IRQ-driven card detection.  */
	/*mmc->caps |= MMC_CAP_NEEDS_POLL;*/

	/*mmc->caps |= MMC_CAP_WAIT_WHILE_BUSY;*/

	mmc->caps |= MMC_CAP_CMD23;
	mmc->caps |= MMC_CAP_SDIO_IRQ;

	if (pdata) {
		host->version = pdata->version;
	}

	ret = nvt_mmc_sysfs_init(&pdev->dev);
	if (ret < 0) {
		pr_err("mmc sysfs can't be created.\n");
	}

	if (voltage_switch && (max_voltage > VOLTAGE_1800)) {
		mmc->ops = &nvt_mmc_ops;
		host->max_voltage = SDIO_HOST_SETPOWER_VOL_3P3;
	} else {
		if (is1v8wifi) {
			mmc->ops = &nvt_mmc_ops;
			host->max_voltage = SDIO_HOST_SETPOWER_VOL_1P8;
		} else {
			mmc->ops = &nvt_fixed_mmc_ops;
			if (max_voltage > VOLTAGE_1800) {
				host->max_voltage = SDIO_HOST_SETPOWER_VOL_3P3;
			} else {
				host->max_voltage = SDIO_HOST_SETPOWER_VOL_1P8;
			}
		}
	}

	if (pdata && pdata->caps) {
		mmc->caps |= pdata->caps;
	}

	if (voltage_switch) {
		// for sdio3.0 sd and sdio OCR only support 3.3V, use voltage switch to tell host voltage is 1.8V
		mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;
		mmc->ocr_avail_sdio = MMC_VDD_32_33 | MMC_VDD_33_34;
		mmc->caps |= MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 | \
					 MMC_CAP_UHS_SDR50;

		if (mmc->f_max > SDIO_MODE_SDR50) {
			mmc->caps |= MMC_CAP_UHS_SDR104;
		}
	} else {
		if (max_voltage > VOLTAGE_1800) {
			mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
		} else {
			mmc->ocr_avail = MMC_VDD_165_195;
		}
	}

#if SDIO_SCATTER_DMA
	mmc->max_segs       = SDIO_DES_TABLE_NUM;/*SDIO_DES_TABLE_NUM;*/
#else
	mmc->max_segs       = 1 + host->n_link;
#endif

	/*mmc->max_seg_size = (32*1024);*/
	/* MMC/SD controller limits for multiblock requests */
	mmc->max_blk_size   = 2048;
	mmc->max_blk_count  = (32 * 1024);
	mmc->max_req_size   = mmc->max_blk_size * mmc->max_blk_count;
	mmc->max_seg_size   = mmc->max_req_size;
	dev_dbg(mmc_dev(host->mmc), "max_segs=%d\n", mmc->max_segs);
	dev_dbg(mmc_dev(host->mmc), "max_blk_size=%d\n", mmc->max_blk_size);
	dev_dbg(mmc_dev(host->mmc), "max_req_size=%d\n", mmc->max_req_size);
	dev_dbg(mmc_dev(host->mmc), "max_seg_size=%d\n", mmc->max_seg_size);
#ifdef CONFIG_NVT_WIFI_BRCM_PM
	if (host->id == SDIO_HOST_ID_2) {
		mmc->pm_caps |= MMC_PM_KEEP_POWER ;
		mmc->caps |= MMC_CAP_NONREMOVABLE;
	}
#endif

	if (cd_gpio[2]) {
		if (cd_gpio[0]) {
			if (gpio_is_valid(cd_gpio[0])) {
				ret = request_irq(host->mmc_cd_irq, mmc_irq_cd,
								  IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "mmc_cd", host);
				if (ret) {
					mmc->caps |= MMC_CAP_NEEDS_POLL;
					dev_err(mmc_dev(host->mmc), "cd_gpio request irq failed\n");
				}
			} else {
				dev_err(mmc_dev(host->mmc), "cd_gpio is invalid, ret(%d)\n", gpio_is_valid(cd_gpio[0]));
				mmc->caps |= MMC_CAP_NEEDS_POLL;
			}

			if (mmc->caps & MMC_CAP_NEEDS_POLL) {
				dev_err(mmc_dev(host->mmc), "cd_gpio still in polling mode\n");
			} else {
				dev_err(mmc_dev(host->mmc), "cd_gpio in interrupt mode\n");
			}
		} else {
			dev_info(mmc_dev(host->mmc), "cd_gpio disabled\n");
		}
	} else {
		mmc->caps |= MMC_CAP_NEEDS_POLL;
		dev_info(mmc_dev(host->mmc), "cd_gpio in polling mode\n");
	}

	platform_set_drvdata(pdev, host);
#if 1  // for Brcm drv
	nvt_host[host->id] = host;
#endif

	ret = request_threaded_irq(irq, NULL, nvt_mmc_irq, IRQF_ONESHOT, mmc_hostname(mmc), host);
	if (ret) {
		goto out;
	}

	if (voltage_switch) {
		init_completion(&host->voltage_switch_complete);
	}

	rename_region(mem, mmc_hostname(mmc));

	sdio_copy_info(host);

	// ret = nvt_mmc_proc_init(proc_path);
	// if (ret)
	//  goto out;

	// nvt mmc host has already finish initial, do not need to wait again.
	mmc->ios.power_delay_ms = 0;
	ret = mmc_add_host(mmc);
	if (ret < 0) {
		goto out;
	}

	dev_info(mmc_dev(host->mmc), "Using %s, %d-bit mode sampling %s edge, mmc%d, index%d, pinmux(0x%x) caps(0x%x) f_max(%d)\n",
			 host->use_dma ? "DMA" : "PIO",
			 (mmc->caps & MMC_CAP_8_BIT_DATA) ? 8 :
			 (mmc->caps & MMC_CAP_4_BIT_DATA) ? 4 : 1,
			 host->neg_sample_edge ? "Negtive" : "Positive",
			 host->id, mmc->index, host->mmc_pinmux, mmc->caps, mmc->f_max);

	if (host_dll_check[host->id]) {
		SDIO_FLOW("[indly(%d) outdly(%d)] [cmd_phase(%d) data_phase(%d) cmd_dll(%d) data_dll(%d)]\n",
			  	host->indly_sel, host->outdly_sel, host->dly_cmd_phase, host->dly_data_phase, host->dly_cmd_dll, host->dly_data_dll);
	} else {
		SDIO_FLOW("[indly(%d) outdly(%d)] [cmd_phase(%d) data_phase(%d) phase_unit(%d)]\n",
			  	host->indly_sel, host->outdly_sel, host->dly_cmd_phase, host->dly_data_phase, host->dly_phase_unit);
	}
	SDIO_FLOW("DS/HS driving: %d %d %d %d %d %d\n",
			  host->pad_driving[SDIO_DS_MODE_CLK], host->pad_driving[SDIO_DS_MODE_CMD], host->pad_driving[SDIO_DS_MODE_DATA],
			  host->pad_driving[SDIO_HS_MODE_CLK], host->pad_driving[SDIO_HS_MODE_CMD], host->pad_driving[SDIO_HS_MODE_DATA]);
	SDIO_FLOW("SDR50/SDR104 driving: %d %d %d %d %d %d\n",
			  host->pad_driving[SDIO_SDR50_MODE_CLK], host->pad_driving[SDIO_SDR50_MODE_CMD], host->pad_driving[SDIO_SDR50_MODE_DATA],
			  host->pad_driving[SDIO_SDR104_MODE_CLK], host->pad_driving[SDIO_SDR104_MODE_CMD], host->pad_driving[SDIO_SDR104_MODE_DATA]);

	return 0;

out:
	if (host) {
		if (host->clk) {
			clk_disable(host->clk);
			clk_put(host->clk);
		}

		if (host->base) {
			iounmap(host->base);
		}

#if 1 // for Brcm drv
		nvt_host[host->id] = NULL;
#endif
	}

	if (mmc) {
		mmc_free_host(mmc);
	}

	if (mem) {
		release_mem_region(mem->start, mem_size);
	}

	pr_err("probe err %d\n", ret);

	return ret;
}

int nvt_mmcsd_remove(struct platform_device *pdev)
{
	struct mmc_nvt_host *host = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	if (host) {
		mmc_remove_host(host->mmc);
		free_irq(host->mmc_irq, host);

		if ((host->cd_gpio) && !(host->mmc->caps & MMC_CAP_NEEDS_POLL)) {
			free_irq(host->mmc_cd_irq, host);
		}

		clk_disable(host->clk);
		clk_put(host->clk);

		iounmap(host->base);

		release_mem_region(host->mem_res->start, resource_size(host->mem_res));

#if 1  // for Brcm drv
		nvt_host[host->id] = NULL;
#endif

		mmc_free_host(host->mmc);

		nvt_mmc_sysfs_remove(&pdev->dev);
	}

	return 0;
}

int nvt_mmcsd_suspend(struct device *dev)
{
#if IS_ENABLED(CONFIG_MMC_NVT_QUICK_SUSPEND)
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_nvt_host *host = platform_get_drvdata(pdev);

	clk_disable(host->clk);
	disable_irq(host->mmc_irq);
	host->suspend_cd_state = nvt_mmc_get_cd_polled(host->mmc);
	if (host->suspend_cd_state == 0) {
		sdiohost_power_up(host);
	}
#else
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_nvt_host *host = platform_get_drvdata(pdev);

	sdiohost_resetdata(host, host->id);
	clk_disable(host->clk);
	disable_irq(host->mmc_irq);
	sdiohost_setpower(host, SDIO_HOST_SETPOWER_VOL_0);
	sdiohost_power_down(host);
#endif

	return 0;
}

int nvt_mmcsd_resume(struct device *dev)
{
#if IS_ENABLED(CONFIG_MMC_NVT_QUICK_SUSPEND)
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_nvt_host *host = platform_get_drvdata(pdev);

	if (host->suspend_cd_state == 0) {
		sdiohost_power_down(host);
	}
	clk_enable(host->clk);
	enable_irq(host->mmc_irq);
#else

	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_nvt_host *host = platform_get_drvdata(pdev);

	clk_enable(host->clk);
//	msleep(50);
	sdiohost_setpower(host, host->max_voltage);
	sdiohost_power_up(host);
	init_mmcsd_host(host, host->voltage_switch);
	// enable the clock output after card detection
	sdiohost_enclockout(host, host->id);
	enable_irq(host->mmc_irq);
#endif

	return 0;
}