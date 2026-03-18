

#include "nvt_mmc_delay_chain.h"

static unsigned int debug_delay_chain_on = 0;
module_param_named(debug_delay_chain_on, debug_delay_chain_on, uint, 0600);

#define SDIO_CHAIN(fmt, ...) do { \
		if (debug_delay_chain_on && host) \
			dev_info(mmc_dev(host->mmc), fmt, ##__VA_ARGS__); \
	} while (0)

u8 tuning_test = 0;

u32 golden_unit[3];
u32 golden_data_phase[3];
u32 golden_cmd_phase[3];
u32 golden_data_dll[3];
u32 golden_cmd_dll[3];

void nvt_mmc_chain_type(void)
{
#if defined (CONFIG_NVT_IVOT_PLAT_NS02301)
	const u32 host_dll_check_default[3] 	= 	{0, 0, 1};
	const u32 golden_unit_default[3] 		= 	{0, 0, 0};
	const u32 golden_data_phase_default[3] 	= 	{0, 0, 3};
	const u32 golden_cmd_phase_default[3] 	= 	{0, 0, 3};
	const u32 golden_data_dll_default[3] 	= 	{0, 0, 0x15};
	const u32 golden_cmd_dll_default[3] 	= 	{0, 0, 0x2C};
	memcpy(host_dll_check, host_dll_check_default, sizeof(host_dll_check_default));
	memcpy(golden_unit, golden_unit_default, sizeof(golden_unit_default));
	memcpy(golden_data_phase, golden_data_phase_default, sizeof(golden_data_phase_default));
	memcpy(golden_cmd_phase, golden_cmd_phase_default, sizeof(golden_cmd_phase_default));
	memcpy(golden_data_dll, golden_data_dll_default, sizeof(golden_data_dll_default));
	memcpy(golden_cmd_dll, golden_cmd_dll_default, sizeof(golden_cmd_dll_default));
#elif defined (CONFIG_NVT_IVOT_PLAT_NS02302)
		  u32 host_dll_check_default[3] 	= 	{0, 0, 1};
	const u32 golden_unit_default[3] 		= 	{0, 0, 0};
	const u32 golden_data_phase_default[3] 	= 	{0, 0, 3};
	const u32 golden_cmd_phase_default[3] 	= 	{0, 0, 3};
	const u32 golden_data_dll_default[3] 	= 	{0, 0, 0x15};
	const u32 golden_cmd_dll_default[3] 	= 	{0, 0, 0x2C};
	
	if (nvt_get_chip_id() == CHIP_NS02402) {
		host_dll_check_default[0] = 1;
		host_dll_check_default[1] = 1;
	}
	memcpy(host_dll_check, host_dll_check_default, sizeof(host_dll_check_default));
	memcpy(golden_unit, golden_unit_default, sizeof(golden_unit_default));
	memcpy(golden_data_phase, golden_data_phase_default, sizeof(golden_data_phase_default));
	memcpy(golden_cmd_phase, golden_cmd_phase_default, sizeof(golden_cmd_phase_default));
	memcpy(golden_data_dll, golden_data_dll_default, sizeof(golden_data_dll_default));
	memcpy(golden_cmd_dll, golden_cmd_dll_default, sizeof(golden_cmd_dll_default));
#elif defined (CONFIG_NVT_IVOT_PLAT_NS02201)
	const u32 host_dll_check_default[3] 	= 	{0, 0, 0};
	const u32 golden_unit_default[3] 		= 	{0, 0, 1};
	const u32 golden_data_phase_default[3] 	= 	{0, 0, 10};
	// for enhance strobe
	const u32 golden_cmd_phase_default[3] 	= 	{0, 0, 3};		// not support
	const u32 golden_data_dll_default[3] 	= 	{0, 0, 0x15};	// not support
	const u32 golden_cmd_dll_default[3] 	= 	{0, 0, 0x2C};	// not support
	memcpy(host_dll_check, host_dll_check_default, sizeof(host_dll_check_default));
	memcpy(golden_unit, golden_unit_default, sizeof(golden_unit_default));
	memcpy(golden_data_phase, golden_data_phase_default, sizeof(golden_data_phase_default));
	memcpy(golden_cmd_phase, golden_cmd_phase_default, sizeof(golden_cmd_phase_default));
	memcpy(golden_data_dll, golden_data_dll_default, sizeof(golden_data_dll_default));
	memcpy(golden_cmd_dll, golden_cmd_dll_default, sizeof(golden_cmd_dll_default));
#elif defined (CONFIG_NVT_IVOT_PLAT_NA51102)
	const u32 host_dll_check_default[3] 	= 	{0, 0, 0};
	const u32 golden_unit_default[3] 		= 	{0, 0, 0};
	const u32 golden_data_phase_default[3] 	= 	{0, 0, 5};
	// for enhance strobe
	const u32 golden_cmd_phase_default[3] 	= 	{0, 0, 3};		// not support
	const u32 golden_data_dll_default[3] 	= 	{0, 0, 0x15};	// not support
	const u32 golden_cmd_dll_default[3] 	= 	{0, 0, 0x2C};	// not support
	memcpy(host_dll_check, host_dll_check_default, sizeof(host_dll_check_default));
	memcpy(golden_unit, golden_unit_default, sizeof(golden_unit_default));
	memcpy(golden_data_phase, golden_data_phase_default, sizeof(golden_data_phase_default));
	memcpy(golden_cmd_phase, golden_cmd_phase_default, sizeof(golden_cmd_phase_default));
	memcpy(golden_data_dll, golden_data_dll_default, sizeof(golden_data_dll_default));
	memcpy(golden_cmd_dll, golden_cmd_dll_default, sizeof(golden_cmd_dll_default));
#elif defined (CONFIG_NVT_IVOT_PLAT_NA51090)
	const u32 host_dll_check_default[3] 	= 	{0, 0, 0};
	const u32 golden_unit_default[3] 		= 	{0, 0, 0};
	const u32 golden_data_phase_default[3] 	= 	{0, 0, 5};
	// for enhance strobe
	const u32 golden_cmd_phase_default[3] 	= 	{0, 0, 3};		// not support
	const u32 golden_data_dll_default[3] 	= 	{0, 0, 0x15};	// not support
	const u32 golden_cmd_dll_default[3] 	= 	{0, 0, 0x2C};	// not support
	memcpy(host_dll_check, host_dll_check_default, sizeof(host_dll_check_default));
	memcpy(golden_unit, golden_unit_default, sizeof(golden_unit_default));
	memcpy(golden_data_phase, golden_data_phase_default, sizeof(golden_data_phase_default));
	memcpy(golden_cmd_phase, golden_cmd_phase_default, sizeof(golden_cmd_phase_default));
	memcpy(golden_data_dll, golden_data_dll_default, sizeof(golden_data_dll_default));
	memcpy(golden_cmd_dll, golden_cmd_dll_default, sizeof(golden_cmd_dll_default));
#else
	const u32 host_dll_check_default[3] 	= 	{0, 0, 0};
	const u32 golden_unit_default[3] 		= 	{0, 0, 0};
	const u32 golden_data_phase_default[3] 	= 	{0, 0, 5};
	// for enhance strobe
	const u32 golden_cmd_phase_default[3] 	= 	{0, 0, 3};		// not support
	const u32 golden_data_dll_default[3] 	= 	{0, 0, 0x15};	// not support
	const u32 golden_cmd_dll_default[3] 	= 	{0, 0, 0x2C};	// not support
	memcpy(host_dll_check, host_dll_check_default, sizeof(host_dll_check_default));
	memcpy(golden_unit, golden_unit_default, sizeof(golden_unit_default));
	memcpy(golden_data_phase, golden_data_phase_default, sizeof(golden_data_phase_default));
	memcpy(golden_cmd_phase, golden_cmd_phase_default, sizeof(golden_cmd_phase_default));
	memcpy(golden_data_dll, golden_data_dll_default, sizeof(golden_data_dll_default));
	memcpy(golden_cmd_dll, golden_cmd_dll_default, sizeof(golden_cmd_dll_default));
#endif
	tuning_test = 0;
}

void nvt_mmc_check_edge(struct mmc_nvt_host *host, u32 edge_result, u32 sample_clk_edge, u32 *zeropos, u32 *zerolength)
{
	u32 max_length = 0;
	u32 max_pos = 0;
	u32 best_pos = 0;
	u32 this_length = 0;
	u32 this_pos = 0;
	u32 pos;

	if (sample_clk_edge == SDIO_HOST_SAMPLE_CLK_EDGE_NEG) {
		edge_result = (edge_result >> SDIO_EDGE_RESULT_LEN);
	} else if (sample_clk_edge == SDIO_HOST_SAMPLE_CLK_EDGE_BOTH) {
		edge_result |= (edge_result >> SDIO_EDGE_RESULT_LEN);
	}

	for (pos = 0; pos < SDIO_DELAY_PHASE_SEL_MAX; pos++) {
		if ((edge_result & (1 << pos)) == 0) {
			if (this_length == 0) {
				this_pos = pos;
			}
			this_length++;
		} else {
			if (this_length > max_length) {
				max_length = this_length;
				max_pos = this_pos;
			}
			this_length = 0;
		}
	}

	if ((max_length == 0) || (this_length > max_length)) {
		max_pos = this_pos;
		max_length = this_length;
	}

	// move BestPos to the center of zero bit
	if (max_length) {
		best_pos = max_pos + (max_length >> 1);
	}

	*zeropos = best_pos;
	*zerolength = max_length;

	SDIO_CHAIN("[sample_clk_edge(%d) edge_result(0x%04x)] [max_length(%d) max_pos(%d) best_pos(%d)]\n", sample_clk_edge, edge_result, max_length, max_pos, best_pos);
}

int nvt_mmc_auto_tuning_with_dLL(struct mmc_host *mmc, u32 opcode)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);
	SDIO_HOST_SAMPLE_CLK_EDGE sample_clk_edge;
	u32 det_cmd_out, det_data_out;
	u32 cmd_phase_sel = 0, data_phase_sel = 0;
	u32 sum_dll_cmd = 0, sum_dll_data = 0;
	u32 tuning_repeat;
	u32 cmd_zero_len, data_zero_len;
	u32 ret;

	// use customer setting
	if ((mmc->ios.clock <= 48000000) ||
		(host->need_tuning == false)) {
		SDIO_CHAIN("bypass tuning, timing(%d) clock(%d) need_tuning(%d)\n", mmc->ios.timing, mmc->ios.clock, host->need_tuning);
		return E_OK;
	}

	// use HS200 paremeter to set HS400 setting
	// do not need to setting here, set at complete
	if (mmc->ios.timing == MMC_TIMING_MMC_HS400) {
		SDIO_CHAIN("bypass tuning, HS400\n");
		return E_OK;
	}

	// initial
	mmc->retune_period = 0;
	sdiohost_set_detmode(host, SDIO_HOST_DET_MODE_AUTO);
	sdiohost_set_detdata(host, (mmc->ios.bus_width == MMC_BUS_WIDTH_8));

	// hs200 tunning
	sample_clk_edge = host->neg_sample_edge;
	det_cmd_out = 0;
	det_data_out = 0;

	// dll clock must faster than 100M
	if (mmc->ios.clock > 100000000) {
		// tunning dll
		for (tuning_repeat = 0; tuning_repeat < 3; tuning_repeat++) {
					
			sdiohost_set_dlldetclr(host, 0);
			// receive cmd_rsp/data will trigger cmd/data edge detect respectively
			mmc_send_tuning(mmc, opcode, NULL);

			ret = sdiohost_get_dllresult(host, &cmd_phase_sel, &data_phase_sel);
			if (ret) 
				return -EIO;

			sum_dll_cmd += cmd_phase_sel;
			sum_dll_data += data_phase_sel;
		}
		sum_dll_cmd /= 3;
		sum_dll_data /= 3;

	} else {
		// if slower than 100M, only use phase to scan
		sum_dll_cmd = 10;
		sum_dll_data = 10;
	}
	
	sdiohost_set_dllresult(host, &sum_dll_cmd, &sum_dll_data);
	// temp use
	sdiohost_set_phasecmd(host, 3);
	sdiohost_set_phasedata(host, 3);

	// tunning delay chain
	for (tuning_repeat = 0; tuning_repeat < TUNING_TIMES; tuning_repeat++) {
					
		mmc_send_tuning(mmc, opcode, NULL);

		det_cmd_out |= sdiohost_get_detcmdout(host);
		det_data_out |= sdiohost_get_detdataout(host);
	}

	/* TODO: review threshold */
	nvt_mmc_check_edge(host, det_cmd_out, sample_clk_edge, &cmd_phase_sel, &cmd_zero_len);
	/* TODO: review threshold */
	nvt_mmc_check_edge(host, det_data_out, sample_clk_edge, &data_phase_sel, &data_zero_len);


	// check again
	if (cmd_zero_len >= SDIO_EDGE_RESULT_THD && data_zero_len >= SDIO_EDGE_RESULT_THD) {
		goto tuning_success;
	} else {
		dev_err(mmc_dev(host->mmc), "tuning failed, data_phase(%d) whose zero_len(%d) < threshold(%d)\n", data_phase_sel, data_zero_len, SDIO_EDGE_RESULT_THD);
		dev_err(mmc_dev(host->mmc), "tuning failed, cmd_phase(%d) whose zero_len(%d) < threshold(%d)\n", cmd_phase_sel, cmd_zero_len, SDIO_EDGE_RESULT_THD);
		dev_err(mmc_dev(host->mmc), "dll_cmd = 0x%x, dll_data = 0x%x\n", sum_dll_cmd, sum_dll_data);
		return -EIO;
	}

	
	
tuning_success:
	
	if (mmc->ios.timing == MMC_TIMING_MMC_HS200) {
		dev_info(mmc_dev(host->mmc), "HS200 tuning pass\n");

		sdiohost_set_phasecmd(host, cmd_phase_sel);
		sdiohost_set_phasedata(host, data_phase_sel);
		sdiohost_setphyrst(host);

		// HS400 5.0 will used cmd phase
		host->dly_cmd_phase = cmd_phase_sel;
		host->dly_cmd_dll = sum_dll_cmd;
	}
	else if (mmc->ios.timing == MMC_TIMING_UHS_SDR50 \
	|| mmc->ios.timing == MMC_TIMING_UHS_SDR104) {

		dev_info(mmc_dev(host->mmc), "SDR50/SDR104 tuning pass\n");

		sdiohost_set_phasecmd(host, cmd_phase_sel);
		sdiohost_set_phasedata(host, data_phase_sel);
		sdiohost_setphyrst(host);

		// reset to -1 for sdr50 and sdr104
		host->dly_cmd_phase = -1;
		host->dly_cmd_dll = -1;
		host->dly_data_phase = -1;
		host->dly_data_dll = -1;
	}

	SDIO_CHAIN("[cmd_det(0x%08x) data_det(0x%08x)] [cmd_phase(%d) data_phase(%d) ]\n", det_cmd_out, det_data_out, cmd_phase_sel, data_phase_sel);
	SDIO_CHAIN("dll_cmd = 0x%x, dll_data = 0x%x\n", sum_dll_cmd, sum_dll_data);

	return E_OK;
}

void nvt_mmc_hs400_complete_with_dLL(struct mmc_host *mmc)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);
	u32 cmd_dll_sel = 0, data_dll_sel = 0;

	if (host->ishs400es) {

		cmd_dll_sel = golden_cmd_dll[host->id];
		data_dll_sel = golden_data_dll[host->id];
		host->dly_data_phase = golden_data_phase[host->id];
		host->dly_data_dll = data_dll_sel;
		host->dly_cmd_phase = golden_cmd_phase[host->id];
		host->dly_cmd_dll = cmd_dll_sel;
		sdiohost_set_dllresult(host, &cmd_dll_sel, &data_dll_sel);

		sdiohost_set_phasecmd(host, host->dly_cmd_phase); 
		sdiohost_set_phasedata(host, host->dly_data_phase);
		sdiohost_setphyrst(host);

		dev_info(mmc_dev(host->mmc), "HS400es: [cmd_phase(%d) data_phase(%d) dll_cmd(%d), dll_data(%d)]\n", \
		host->dly_cmd_phase, host->dly_data_phase, host->dly_cmd_dll, host->dly_data_dll);

	} else {

		// use customer setting
		if ((host->need_tuning == false)) {
			return;
		}

		cmd_dll_sel = host->dly_cmd_dll;
		data_dll_sel = golden_data_dll[host->id];
		host->dly_data_phase = golden_data_phase[host->id];
		host->dly_data_dll = data_dll_sel;
		sdiohost_set_dllresult(host, &cmd_dll_sel, &data_dll_sel);

		sdiohost_set_phasecmd(host, host->dly_cmd_phase); 
		sdiohost_set_phasedata(host, host->dly_data_phase);
		sdiohost_setphyrst(host);

		dev_info(mmc_dev(host->mmc), "HS400: [cmd_phase(%d) data_phase(%d) dll_cmd(%d), dll_data(%d)]\n", \
		 host->dly_cmd_phase, host->dly_data_phase, host->dly_cmd_dll, host->dly_data_dll);
	
	}

	// for set ios check tunning
	host->dly_cmd_phase = -1;
	host->dly_data_phase = -1;
	host->dly_cmd_dll = -1;
	host->dly_data_dll = -1;

}

int nvt_mmc_auto_tuning_phase_unit(struct mmc_host *mmc, u32 opcode)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);
	SDIO_HOST_SAMPLE_CLK_EDGE sample_clk_edge;
	u32 det_cmd_out, det_data_out;
	u32 cmd_phase_sel = 0, data_phase_sel = 0;
	u32 dly_phase_unit = golden_unit[host->id];
	u32 repeat, indly_step, tuning_repeat;
	u32 cmd_zero_len, data_zero_len;

	// use customer setting
	if ((mmc->ios.clock <= 48000000) ||
		(host->need_tuning == false)) {
		SDIO_CHAIN("bypass tuning, timing(%d) clock(%d) need_tuning(%d)\n", mmc->ios.timing, mmc->ios.clock, host->need_tuning);
		return E_OK;
	}

	// use HS200 paremeter to set HS400 setting
	// do not need to setting here, set at complete
	if (mmc->ios.timing == MMC_TIMING_MMC_HS400 && tuning_test == 0) {
		SDIO_CHAIN("bypass tuning, HS400\n");
		return E_OK;
	}

	// initial
	mmc->retune_period = 0;
	sdiohost_set_detmode(host, SDIO_HOST_DET_MODE_AUTO);
	sdiohost_set_detdata(host, (mmc->ios.bus_width == MMC_BUS_WIDTH_8));

	// hs200 tunning
	sample_clk_edge = host->neg_sample_edge;
	det_cmd_out = 0;
	det_data_out = 0;

	sdiohost_set_phaseunit(host, dly_phase_unit);
	sdiohost_setphyrst(host);

	/* TODO: add loop to zoom in phase_unit (according to ios.clock) to see the edges */
	for (indly_step = 0; indly_step < SDIO_AUTO_TUNE_INDLT_CNT; indly_step++) {
		for (repeat = 0; repeat < SDIO_AUTO_TUNE_REPEAT; repeat++) {

			sdiohost_set_detclr(host);
			sdiohost_setphyclkindly(host, (128/SDIO_AUTO_TUNE_INDLT_STEP) * indly_step);
			sdiohost_setphyrst(host);
			// receive cmd_rsp/data will trigger cmd/data edge detect respectively
			mmc_send_tuning(mmc, opcode, NULL);

			det_cmd_out |= sdiohost_get_detcmdout(host);
			det_data_out |= sdiohost_get_detdataout(host);

			/* TODO: review threshold */
			nvt_mmc_check_edge(host, det_cmd_out, sample_clk_edge, &cmd_phase_sel, &cmd_zero_len);
			
			/* TODO: review threshold */
			nvt_mmc_check_edge(host, det_data_out, sample_clk_edge, &data_phase_sel, &data_zero_len);

			if (cmd_zero_len >= SDIO_EDGE_RESULT_THD && data_zero_len >= SDIO_EDGE_RESULT_THD) {
				// this indly setting may ok, start tuning
				for (tuning_repeat = 0; tuning_repeat < TUNING_TIMES; tuning_repeat++) {
					sdiohost_set_detclr(host);

					// receive cmd_rsp/data will trigger cmd/data edge detect respectively
					mmc_send_tuning(mmc, opcode, NULL);

					det_cmd_out |= sdiohost_get_detcmdout(host);
					det_data_out |= sdiohost_get_detdataout(host);
				}

				/* TODO: review threshold */
				nvt_mmc_check_edge(host, det_cmd_out, sample_clk_edge, &cmd_phase_sel, &cmd_zero_len);
				/* TODO: review threshold */
				nvt_mmc_check_edge(host, det_data_out, sample_clk_edge, &data_phase_sel, &data_zero_len);

				// check again
				if (cmd_zero_len >= SDIO_EDGE_RESULT_THD && data_zero_len >= SDIO_EDGE_RESULT_THD) {
					goto tuning_success;
				} else {
					break;
				}

			} else {
				dev_err(mmc_dev(host->mmc), "tuning failed, data_phase(%d) whose zero_len(%d) < threshold(%d)\n", data_phase_sel, data_zero_len, SDIO_EDGE_RESULT_THD);
				dev_err(mmc_dev(host->mmc), "tuning failed, cmd_phase(%d) whose zero_len(%d) < threshold(%d)\n", cmd_phase_sel, cmd_zero_len, SDIO_EDGE_RESULT_THD);
				dev_err(mmc_dev(host->mmc), "indly = 0x%x\n", (128/SDIO_AUTO_TUNE_INDLT_STEP) * indly_step);

				if (repeat == (SDIO_AUTO_TUNE_REPEAT - 1) \
						&& indly_step == (SDIO_AUTO_TUNE_INDLT_STEP - 1)) {
					// reset indly			
					sdiohost_setphyclkindly(host, 0);
					return -EIO;
				} 
			}
		}
	}
	
tuning_success:
	// HS400 5.0 will used cmd phase
	dev_info(mmc_dev(host->mmc), "HS200 tuning pass\n");
	host->dly_cmd_phase = cmd_phase_sel;
	sdiohost_set_phasecmd(host, cmd_phase_sel);
	sdiohost_set_phasedata(host, data_phase_sel);
	sdiohost_set_phaseunit(host, dly_phase_unit);
	sdiohost_setphyrst(host);

	SDIO_CHAIN("[cmd_det(0x%08x) data_det(0x%08x)] [cmd_phase(%d) data_phase(%d) phase_unit(%d)]\n", det_cmd_out, det_data_out, cmd_phase_sel, data_phase_sel, dly_phase_unit);
	SDIO_CHAIN("indly = 0x%x\n", (128/SDIO_AUTO_TUNE_INDLT_STEP) * indly_step);

	return E_OK;
}

void nvt_mmc_hs400_complete_phase_unit(struct mmc_host *mmc)
{
	struct mmc_nvt_host *host = mmc_priv(mmc);

	// use customer setting
	if ((host->need_tuning == false)) {
		return;
	}

	// force to set inhouse setting when speed < 160000000
	if (mmc->ios.clock <= 160000000) {
		host->dly_data_phase = golden_data_phase[host->id];
		host->dly_phase_unit = golden_unit[host->id];
	}

	sdiohost_set_phasecmd(host, host->dly_cmd_phase);
	sdiohost_set_phasedata(host, host->dly_data_phase);
	sdiohost_set_phaseunit(host, host->dly_phase_unit);
	sdiohost_setphyrst(host);

	SDIO_CHAIN("HS400: [cmd_phase(%d) data_phase(%d) phase_unit(%d)]\n", host->dly_cmd_phase, host->dly_data_phase, host->dly_phase_unit);

	// for set ios check tunning
	host->dly_cmd_phase = -1;
	host->dly_data_phase = -1;
	host->dly_phase_unit = -1;
}
