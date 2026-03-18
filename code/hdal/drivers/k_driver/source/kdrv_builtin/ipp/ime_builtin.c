#include "kwrap/type.h"
#include "kwrap/file.h"
#include "kwrap/cpu.h"
#include "kdrv_ipp_builtin.h"
#include "kdrv_ipp_builtin_int.h"
#include "ime_eng_int_reg.h"
#include "kdrv_builtin.h"
#if defined(__FREERTOS)
#include "interrupt.h"
#include "io_address.h"
#endif

#include "ime_eng.h"

#if defined(__LINUX)
#include <linux/soc/nvt/nvt-info.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/clk-provider.h> //__clk_is_enabled
#include "ime_eng_int_platform.h"
#endif

static IME_BUILTIN_CTL ime_builtin_ctl;
BOOL ime_auto_gating = 0;
BOOL ime_clock_en = 0;

void ime_builtin_init(UINT32 chip_num, UINT32 eng_num)
{
	if (ime_builtin_ctl.p_hdl != NULL) {
		DBG_ERR("already init\r\n");
		return ;
	}

	ime_builtin_ctl.chip_num = chip_num;
	ime_builtin_ctl.eng_num = eng_num;
	ime_builtin_ctl.total_ch = chip_num * eng_num;
	ime_builtin_ctl.p_hdl = kdrv_ipp_builtin_plat_malloc(sizeof(IME_BUILTIN_HANDLE) * ime_builtin_ctl.total_ch);
	if (ime_builtin_ctl.p_hdl == NULL) {
		DBG_ERR("alloca buffer failed\r\n");
	}
}

void ime_builtin_init_resource(NVT_IME_BUILTIN_RESOURCE *p_resource, UINT32 idx)
{
	UINT32 chip, eng, dev_id;
	IME_BUILTIN_HANDLE *p_hdl;

	if (idx < ime_builtin_ctl.total_ch) {
		p_hdl = &ime_builtin_ctl.p_hdl[idx];
		chip = IME_BUILTIN_IDX_CHIP(idx);
		eng = IME_BUILTIN_IDX_ENG(idx);
		dev_id = KDRV_DEV_ID(chip, eng, 0);

		snprintf(p_hdl->name, sizeof(p_hdl->name), "ime_builtin_%d", (int)idx);
		p_hdl->dev_id = dev_id;
		p_hdl->resource = *p_resource;
		p_hdl->switch_to_hdal = FALSE;
		p_hdl->fastboot_cb = NULL;
		p_hdl->hdal_cb = NULL;
		kdrv_ipp_builtin_plat_request_ime_irq(p_hdl->resource.irq_id, p_hdl->name, (void *)p_hdl);

	}
}

void ime_builtin_request_irq(void)
{
	IME_BUILTIN_HANDLE *p_hdl;

	p_hdl = &ime_builtin_ctl.p_hdl[0];

	kdrv_ipp_builtin_plat_request_ime_irq(p_hdl->resource.irq_id, p_hdl->name, (void *)p_hdl);
}

void ime_builtin_exit(void)
{
	IME_BUILTIN_HANDLE *p_hdl;
	UINT32 i;

	if (ime_builtin_ctl.p_hdl) {
		for (i = 0; i < ime_builtin_ctl.total_ch; i++) {
			p_hdl = &ime_builtin_ctl.p_hdl[i];
			kdrv_ipp_builtin_plat_release_irq(p_hdl->resource.irq_id, (void *)p_hdl);
		}
		kdrv_ipp_builtin_plat_free((void *)ime_builtin_ctl.p_hdl);
	}
	memset((void *)&ime_builtin_ctl, 0, sizeof(IME_BUILTIN_CTL));
}

NVT_IME_BUILTIN_RESOURCE *ime_builtin_get_resource(UINT32 idx)
{
	if ((idx < ime_builtin_ctl.total_ch) && ime_builtin_ctl.p_hdl) {
		return &ime_builtin_ctl.p_hdl[idx].resource;
	}

	return NULL;
}

void ime_builtin_reg_fastboot_cb(UINT32 dev_id, IME_BUILTIN_ISR_CB cb)
{
	UINT32 idx;

	idx = IME_BUILTIN_CONV2_HDL_IDX(KDRV_DEV_ID_CHIP(dev_id), KDRV_DEV_ID_ENGINE(dev_id));
	if (idx >= ime_builtin_ctl.total_ch)
		return ;

	ime_builtin_ctl.p_hdl[idx].fastboot_cb = cb;
}

void ime_builtin_unreg_fastboot_cb(UINT32 dev_id)
{
	UINT32 idx;

	idx = IME_BUILTIN_CONV2_HDL_IDX(KDRV_DEV_ID_CHIP(dev_id), KDRV_DEV_ID_ENGINE(dev_id));
	if (idx >= ime_builtin_ctl.total_ch)
		return ;

	ime_builtin_ctl.p_hdl[idx].fastboot_cb = NULL;
}

void ime_builtin_reg_isr_cb(UINT32 dev_id, IME_BUILTIN_ISR_CB cb)
{
	UINT32 idx;

	idx = IME_BUILTIN_CONV2_HDL_IDX(KDRV_DEV_ID_CHIP(dev_id), KDRV_DEV_ID_ENGINE(dev_id));
	if (idx >= ime_builtin_ctl.total_ch)
		return ;

	ime_builtin_ctl.p_hdl[idx].hdal_cb = cb;
}

void ime_builtin_set_switch_to_hdal(UINT32 dev_id)
{
	UINT32 idx;

	idx = IME_BUILTIN_CONV2_HDL_IDX(KDRV_DEV_ID_CHIP(dev_id), KDRV_DEV_ID_ENGINE(dev_id));
	if (idx >= ime_builtin_ctl.total_ch)
		return ;

	ime_builtin_ctl.p_hdl[idx].switch_to_hdal = TRUE;
}

#if 0
#endif

void ime_builtin_get_path_info(KDRV_IPP_BUILTIN_PATH_ID pid, KDRV_IPP_BUILTIN_IMG_INFO *info)
{
	static KDRV_IPP_BUILTIN_IMG_INFO ime_path_info[KDRV_IPP_BUILTIN_PATH_ID_MAX] = {0};
	//static UINT32 is_init[KDRV_IPP_BUILTIN_PATH_ID_MAX] = {FALSE, FALSE, FALSE, FALSE};
	NT98538_IME_ENG_REG_STRUCT *p_ime;
	KDRV_IPP_BUILTIN_IMG_INFO *p_path;
	UINT8 out_type = 0;
	UINT8 omat = 0;
	UINT8 yuv_compress = 0;

	if (ime_builtin_ctl.p_hdl == NULL) {
		return ;
	}

	if (pid >= 5) {
		DBG_ERR("unsupport pid %d\r\n", pid);
		return ;
	}

	if (1/*is_init[pid] == FALSE*/) {

		p_ime = (NT98538_IME_ENG_REG_STRUCT *)ime_builtin_ctl.p_hdl[0].resource.p_io_addr;
		p_path = &ime_path_info[pid];

		switch (pid) {
		case KDRV_IPP_BUILTIN_PATH_ID_1:
			p_path->enable = p_ime->reg_1.bit.ime_p0_en;
			p_path->size.w = p_ime->reg_41.bit.ime_p0_h_osize;
			p_path->size.h = p_ime->reg_41.bit.ime_p0_v_osize;
			p_path->loff[0] = p_ime->reg_44.word;
			p_path->loff[1] = p_ime->reg_45.word;
			p_path->phyaddr[0] = p_ime->reg_46.word;
			p_path->phyaddr[1] = p_ime->reg_47.word;
			p_path->phyaddr[2] = p_ime->reg_48.word;

			out_type = p_ime->reg_33.bit.ime_p0_out_type;
			omat = p_ime->reg_33.bit.ime_p0_omat;
			yuv_compress = p_ime->reg_2.bit.ime_p0_enc_en;
			break;

		case KDRV_IPP_BUILTIN_PATH_ID_2:
			p_path->enable = p_ime->reg_1.bit.ime_p1_en;
			p_path->size.w = p_ime->reg_83.bit.ime_p1_h_osize;
			p_path->size.h = p_ime->reg_83.bit.ime_p1_v_osize;
			p_path->loff[0] = p_ime->reg_86.word;
			p_path->loff[1] = p_ime->reg_87.word;
			p_path->phyaddr[0] = p_ime->reg_88.word;
			p_path->phyaddr[1] = p_ime->reg_89.word;

			out_type = p_ime->reg_64.bit.ime_p1_out_type;
			omat = p_ime->reg_64.bit.ime_p1_omat;

			break;

		case KDRV_IPP_BUILTIN_PATH_ID_3:
			p_path->enable = p_ime->reg_1.bit.ime_p2_en;
			p_path->size.w = p_ime->reg_127.bit.ime_p2_h_osize;
			p_path->size.h = p_ime->reg_127.bit.ime_p2_v_osize;
			p_path->loff[0] = p_ime->reg_130.word;
			p_path->loff[1] = p_ime->reg_131.word;
			p_path->phyaddr[0] = p_ime->reg_132.word;
			p_path->phyaddr[1] = p_ime->reg_133.word;

			out_type = p_ime->reg_108.bit.ime_p2_out_type;
			omat = p_ime->reg_108.bit.ime_p2_omat;
			break;

		case KDRV_IPP_BUILTIN_PATH_ID_4:
			p_path->enable = p_ime->reg_1.bit.ime_p3_en;
			p_path->size.w = p_ime->reg_171.bit.ime_p3_h_osize;
			p_path->size.h = p_ime->reg_171.bit.ime_p3_v_osize;
			p_path->loff[0] = p_ime->reg_174.word;
			p_path->loff[1] = p_ime->reg_175.word;
			p_path->phyaddr[0] = p_ime->reg_176.word;
			p_path->phyaddr[1] = p_ime->reg_177.word;

			out_type = p_ime->reg_152.bit.ime_p3_out_type;
			omat = p_ime->reg_152.bit.ime_p3_omat;
			break;

		case KDRV_IPP_BUILTIN_PATH_ID_5: //3dnr reference output path
			p_path->enable = p_ime->reg_1.bit.ime_3dnr_ref_out_en;
			p_path->size.w = p_ime->reg_8.bit.ime_in_h_size << 2; //in-width is 4x unit
			p_path->size.h = p_ime->reg_8.bit.ime_in_v_size << 2; //in-height is 4x unit
			p_path->loff[0] = p_ime->reg_580.word;
			p_path->loff[1] = p_ime->reg_581.word;
			p_path->phyaddr[0] = p_ime->reg_582.word;
			p_path->phyaddr[1] = p_ime->reg_583.word;

			break;

		default:
			break;

		}

		/* output format */
		if (omat == 0) {
			if (out_type == 0) {
				p_path->fmt = KDRV_IPP_BUILTIN_FMT_YUV420_PLANAR;
			} else {
				if (yuv_compress) {
					p_path->fmt = KDRV_IPP_BUILTIN_FMT_NVX2;
				} else {
					p_path->fmt = KDRV_IPP_BUILTIN_FMT_YUV420;
				}
			}
		} else {
			p_path->fmt = KDRV_IPP_BUILTIN_FMT_Y8;
		}

		//ime_get_output_path_info(drv_path[pid], &ime_path_info[pid]);
		//is_init[pid] = TRUE;
	}

	*info = ime_path_info[pid];

	return;

}

INT32 ime_builtin_rtos_init(void)
{
#if defined(__FREERTOS)
	UINT32 i;
	UINT32 chip, eng, dev_id;
	IME_BUILTIN_HANDLE *p_hdl;

	ime_builtin_init(NVT_IME_BUILTIN_CHIP_COUNT, NVT_IME_BUILTIN_MINOR_COUNT);

	for (i = 0; i < ime_builtin_ctl.total_ch; i++) {
		p_hdl = &ime_builtin_ctl.p_hdl[i];
		chip = IME_BUILTIN_IDX_CHIP(i);
		eng = IME_BUILTIN_IDX_ENG(i);
		dev_id = KDRV_DEV_ID(chip, eng, 0);

		snprintf(p_hdl->name, sizeof(p_hdl->name), "ime_builtin_%d", (int)i);
		p_hdl->dev_id = dev_id;
		p_hdl->resource.irq_id = INT_ID_IME;
		p_hdl->resource.p_io_addr = (void *)IOADDR_IME_REG_BASE;
		p_hdl->resource.p_mclk = NULL;
		p_hdl->switch_to_hdal = FALSE;
		p_hdl->fastboot_cb = NULL;
		p_hdl->hdal_cb = NULL;
		kdrv_ipp_builtin_plat_request_ime_irq(p_hdl->resource.irq_id, p_hdl->name, (void *)p_hdl);
	}
#endif
	return 0;
}

void ime_builtin_rtos_uninit(void)
{
#if defined(__FREERTOS)
	IME_BUILTIN_HANDLE *p_hdl;
	UINT32 i;

	if (ime_builtin_ctl.p_hdl) {
		for (i = 0; i < ime_builtin_ctl.total_ch; i++) {
			p_hdl = &ime_builtin_ctl.p_hdl[i];
			kdrv_ipp_builtin_plat_release_irq(p_hdl->resource.irq_id, (void *)&p_hdl);
		}
		kdrv_ipp_builtin_plat_free((void *)ime_builtin_ctl.p_hdl);
	}
	memset((void *)&ime_builtin_ctl, 0, sizeof(IME_BUILTIN_CTL));
#endif
}


#ifdef CONFIG_PM

UINT32 ime_builtin_get_dtsi_clock(CHAR eng_name[8])
{
#define KDRV_IPP_UTIL_CLOCK_DFT	(450000000)

#if defined(__LINUX)

	struct device_node *from = NULL;
	struct device_node* of_node = of_find_node_by_name(from, eng_name);
	UINT32 clk = KDRV_IPP_UTIL_CLOCK_DFT;

	//DBG_DUMP("ENG clock name: %s\r\n", eng_name);

	if (of_node) {
		if (of_property_read_u32(of_node, "current_rate", &clk) != 0) {
			DBG_WRN("cannot find /%s/current_rate\r\n", eng_name);
		}
	} else {
		DBG_WRN("cannot find node %s\r\n", eng_name);
		clk = KDRV_IPP_UTIL_CLOCK_DFT;
	}

	/* return clk unit is MHz */
	return (clk / 1000000);

#elif defined(__FREERTOS)
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */
	UINT32 clk = KDRV_IPP_UTIL_CLOCK_DFT;

	// get linux space
	if (p_fdt != NULL) {
		nodeoffset = fdt_path_offset(p_fdt, "/kdrv_ipp");
		if (nodeoffset < 0) {
			DBG_WRN("failed to offset for  %s = %d \r\n", "/kdrv_ipp", nodeoffset);
		} else {
			nodep = fdt_getprop(p_fdt, nodeoffset, "clock-frequency", &len);
			if (len == 0 || nodep == NULL) {
				DBG_WRN("failed to access clock-frequency.\r\n");
			} else {
				clk = be32_to_cpu(*(unsigned int *)nodep);
			}
		}
	} else {
		DBG_WRN("null fdt_base\r\n");
	}

	/* return clk unit is MHz */
	return (clk / 1000000);
#endif
}


UINT32 ime_builtin_eng_platform_get_chip_id(VOID)
{
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
}

// unit: byte
UINT32 ime_builtin_get_reg_base_buf_size(VOID)
{
	return (IME_ENG_REG_NUM << 2);
}

VOID ime_builtin_prepare_clk(IME_BUILTIN_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
	if (IS_ERR(p_eng->resource.p_mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->resource.p_mclk);
	} else {

		if(nvt_get_chip_ver()==CHIPVER_A){
			clk_prepare(p_eng->resource.p_mclk);
		} else/* if (nvt_get_chip_ver()==CHIPVER_B)*/{
			clk_prepare(p_eng->resource.p_mclk);
			parent_clk = clk_get(NULL, "pll17");
			clk_prepare(parent_clk);
			clk_put(parent_clk);
		}
	}
#elif defined (__FREERTOS)

	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		pll_enableSystemReset(IME_RSTN);
		pll_disableSystemReset(IME_RSTN);
	} else {

	}

#if 0
	else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		pll_enableSystemReset(IME2_RSTN);
		pll_disableSystemReset(IME2_RSTN);
	} else {

	}
#endif
#else
#endif
}

VOID ime_builtin_unprepare_clk(IME_BUILTIN_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;

	if (IS_ERR(p_eng->resource.p_mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->resource.p_mclk);
	} else {
		if(nvt_get_chip_ver()==CHIPVER_A){
			clk_unprepare(p_eng->resource.p_mclk);
		} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
			clk_unprepare(p_eng->resource.p_mclk);
			parent_clk = clk_get(NULL, "pll17");
			clk_unprepare(parent_clk);
			clk_put(parent_clk);
		}
	}
#elif defined (__FREERTOS)
#else
#endif
}

VOID ime_builtin_enable_clk(IME_BUILTIN_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;

	if (IS_ERR(p_eng->resource.p_mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->resource.p_mclk);
	} else {
		if(nvt_get_chip_ver()==CHIPVER_A){
			clk_enable(p_eng->resource.p_mclk);
		} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
			clk_enable(p_eng->resource.p_mclk);
			parent_clk = clk_get(NULL, "pll17");
			clk_enable(parent_clk);
			clk_put(parent_clk);
		}
	}
#elif defined (__FREERTOS)
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		pll_enableClock(IME_CLK);
	} else {

	}

#if 0
	else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		pll_enableClock(IME2_CLK);
	} else {

	}
#endif
#else
#endif
}

VOID ime_builtin_disable_clk(IME_BUILTIN_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;

	if (IS_ERR(p_eng->resource.p_mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->resource.p_mclk);
	} else {

		if(nvt_get_chip_ver()==CHIPVER_A){
			clk_disable(p_eng->resource.p_mclk);
		} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
			clk_disable(p_eng->resource.p_mclk);
			parent_clk = clk_get(NULL, "pll17");
			clk_disable(parent_clk);
			clk_put(parent_clk);
		}

	}
#elif defined (__FREERTOS)
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		pll_disableClock(IME_CLK);
	} else {

	}

#if 0
	else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		pll_disableClock(IME2_CLK);
	} else {

	}
#endif
#else
#endif
}


INT32 ime_builtin_set_clk_rate(IME_BUILTIN_HANDLE *p_eng)
{

#if defined (__LINUX)
	struct clk *parent_clk;

	UINT32 source_clk;
	//ULONG pll_clock_rate;
	//int ret;

	if (IS_ERR(p_eng->resource.p_mclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->resource.p_mclk);
		return E_SYS;
	}

	source_clk = ime_builtin_get_dtsi_clock("clk_ime");
	if (ime_builtin_eng_platform_get_chip_id() == CHIP_NS02302) {
		if (source_clk == 240) {
			parent_clk = clk_get(NULL, "fix240m");
		} else if (source_clk == 320) {
			parent_clk = clk_get(NULL, "fix320m");
		} else {

			if(nvt_get_chip_ver()==CHIPVER_A){
				if(source_clk >IME_CHIP_A_PLL_13_MAX){
					DBG_ERR("IME(A): set clk rate fail...%d(Mhz) > %d(Mhz)\r\n", source_clk, IME_CHIP_A_PLL_13_MAX);
					return E_SYS;
				}
				parent_clk = clk_get(NULL, "pll13");
				clk_set_rate(parent_clk, source_clk);
			} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
				if(source_clk >IME_CHIP_B_PLL_17_MAX){
					DBG_ERR("IME(B): set clk rate fail...%d(Mhz) > %d(Mhz)\r\n", source_clk, IME_CHIP_B_PLL_17_MAX);
					return E_SYS;
				}
				//parent_clk = clk_get(NULL, "pll13");
				parent_clk = clk_get(NULL, "pll17");
				clk_set_rate(parent_clk, source_clk);
			}

			//pll_clock_rate = 450000000;
			//ret = clk_set_rate(parent_clk, pll_clock_rate);
		}
	} else {
		parent_clk = clk_get(NULL, "fix240m");
	}


	clk_set_parent(p_eng->resource.p_mclk, parent_clk);
	clk_put(parent_clk);


#elif defined (__FREERTOS )
	UINT32 selected_clock = PLL_CLKSEL_IME_240;
	UINT32 source_clk;

	// Turn on power
	// select clock
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		source_clk = p_eng->clock_rate;
		if (ime_eng_platform_get_chip_id() == CHIP_NS02302) {
			if (source_clk == 240) {
				selected_clock = PLL_CLKSEL_IME_240;
			} else if (source_clk == 320) {
				selected_clock = PLL_CLKSEL_IME_320;
			} else {
				selected_clock = PLL_CLKSEL_IME_PLL13;

				//if (pll_getPLLEn(PLL_ID_13) == TRUE) {
				//	pll_set_pll_enable(PLL_ID_13, FALSE);
				//}

				//pll_set_pll_freq(PLL_ID_13, 450000000);

				//if (pll_getPLLEn(PLL_ID_13) == FALSE) {
				//	pll_setPLLEn(PLL_ID_13, TRUE);
				//}
			}

		} else {
			selected_clock = PLL_CLKSEL_IME_240;
		}

		pll_setClockRate(PLL_CLKSEL_IME, selected_clock);
	}
#if 0
	else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		source_clk = p_eng->clock_rate;
		if (ime_eng_platform_get_chip_id() == CHIP_NA51102) {
			if (source_clk == 240) {
				selected_clock = PLL_CLKSEL_IME2_240;
			} else if (source_clk == 320) {
				selected_clock = PLL_CLKSEL_IME2_320;
			} else if (source_clk == 480) {
				selected_clock = PLL_CLKSEL_IME2_480;
			} else {
				selected_clock = PLL_CLKSEL_IME2_PLL26;

				if (pll_getPLLEn(PLL_ID_26) == TRUE) {
					pll_set_pll_enable(PLL_ID_26, FALSE);
				}

				pll_set_pll_freq(PLL_ID_26, 400000000);

				if (pll_getPLLEn(PLL_ID_26) == FALSE) {
					pll_setPLLEn(PLL_ID_26, TRUE);
				}
			}

		} else {
			selected_clock = PLL_CLKSEL_IME2_240;
		}

		pll_setClockRate(PLL_CLKSEL_IME2, selected_clock);
	} else {

	}
#endif
#else
#endif

#if 0
#if defined (__LINUX)

#elif defined (__FREERTOS )
	pll_setClockRate(PLL_CLKSEL_IME, PLL_CLKSEL_IME_240);
#endif
#endif
	return E_OK;
}

#if defined(__LINUX)
INT32 ime_builtin_suspend(UINT32 chip, UINT32 engine)
{
	UINT32 i;
	UINT32 reg_ofs;
	uintptr_t ime_reg_io_base = 0;
	IME_BUILTIN_HANDLE *p_hdl;

	if (!ime_builtin_ctl.total_ch)
		return E_OK;

	if (ime_builtin_ctl.p_hdl) {
		for (i = 0; i < ime_builtin_ctl.total_ch; i++) {
			p_hdl = &ime_builtin_ctl.p_hdl[i];
		}
	}

	if (p_hdl == NULL || (!p_hdl->hdal_cb) || !__clk_is_enabled(p_hdl->resource.p_mclk)) {
		DBG_IND("ime driver is not init, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_OK;
	}

	//alloc
	ime_builtin_ctl.reg_data = kdrv_ipp_builtin_plat_malloc(ime_builtin_get_reg_base_buf_size());
	if (ime_builtin_ctl.reg_data == NULL) {
		DBG_ERR("kmalloc reg_data buf failed\r\n");
		return E_SYS;
	}


	ime_reg_io_base = (uintptr_t)p_hdl->resource.p_io_addr;

	DBG_IND("ime_reg_io_base_pa %lx for suspend\r\n", vos_cpu_get_phy_addr(ime_reg_io_base));
	//DBG_ERR("ime_builtin_ctl.reg_data_pa %lx\r\n", vos_cpu_get_phy_addr((ULONG)ime_builtin_ctl.reg_data));

	if (!ime_reg_io_base) {
		DBG_ERR("ime base addr get fail!\r\n");
		return E_SYS;
	}

	/* flush output buffer */
	vos_cpu_dcache_sync(ime_reg_io_base, ime_builtin_get_reg_base_buf_size(), VOS_DMA_FROM_DEVICE);

	for (i = 0; i < IME_ENG_REG_NUM; i++) {
		reg_ofs = i << 2;

		ime_builtin_ctl.reg_data[i] = kdrv_ipp_builtin_getreg(ime_reg_io_base + reg_ofs);
	}
	if (!IS_ERR(p_hdl->resource.p_mclk) && __clk_is_enabled(p_hdl->resource.p_mclk)) {
		ime_builtin_disable_clk(p_hdl);
		ime_builtin_unprepare_clk(p_hdl);
		ime_clock_en = 1;
	} else {
		ime_clock_en = 0;
	}

	if (clk_get_phase(p_hdl->resource.p_mclk)) {
		ime_auto_gating = 1;
		clk_set_phase(p_hdl->resource.p_mclk, 0);
	} else
		ime_auto_gating = 0;


#if 0
	#if defined(__FREERTOS)
		debug_dumpmem(ime_reg_io_base, ime_builtin_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
	#else
		debug_dumpmem(vos_cpu_get_phy_addr(ime_reg_io_base), ime_builtin_get_reg_base_buf_size());
	#endif
#endif
	return 0;
}

INT32 ime_builtin_resume(UINT32 chip, UINT32 engine)
{
	UINT32 i;
	ULONG reg_ofs;
	uintptr_t ime_reg_io_base = 0;
	IME_BUILTIN_HANDLE *p_hdl;

	if (!ime_builtin_ctl.total_ch) {
		return E_OK;
	}

	if (ime_builtin_ctl.p_hdl) {
		for (i = 0; i < ime_builtin_ctl.total_ch; i++) {
			p_hdl = &ime_builtin_ctl.p_hdl[i];
		}
	}

	if (p_hdl == NULL || (!p_hdl->hdal_cb) || ime_clock_en == 0) {
		DBG_IND("ime driver is not init, null handle of chip 0x%.8x, engine 0x%.8x\r\n", (unsigned int)chip, (unsigned int)engine);
		return E_OK;
	}


	if (ime_builtin_ctl.reg_data == NULL) {
		DBG_ERR("HDL tempory register buffer is not init\r\n");
		return E_SYS;
	}

	ime_reg_io_base =(ULONG)p_hdl->resource.p_io_addr;

	DBG_IND("ime_reg_io_base_pa %lx for resume\r\n", vos_cpu_get_phy_addr(ime_reg_io_base));

	if (!ime_reg_io_base) {
		DBG_ERR("ime base addr get fail!\r\n");
		return E_SYS;
	}

	//clock set
	if (!IS_ERR(p_hdl->resource.p_mclk) && ime_clock_en) {
		ime_builtin_set_clk_rate(p_hdl); //including set parent
		ime_builtin_prepare_clk(p_hdl);
		ime_builtin_enable_clk(p_hdl);
	}

	if (ime_auto_gating) {
		clk_set_phase(p_hdl->resource.p_mclk, 1);
	}


	for (i = 0; i < IME_ENG_REG_NUM; i++) {
		reg_ofs = i << 2;
		kdrv_ipp_builtin_setreg((ULONG)(ime_reg_io_base + reg_ofs), ime_builtin_ctl.reg_data[i]);
	}

	//flush
	vos_cpu_dcache_sync(ime_reg_io_base, ime_builtin_get_reg_base_buf_size(), VOS_DMA_TO_DEVICE);

#if 0
	#if defined(__FREERTOS)
			debug_dumpmem(ime_reg_io_base, ime_builtin_get_reg_base_buf_size()); // reg_io_base is paddr already in rtos
	#else
			debug_dumpmem(vos_cpu_get_phy_addr(ime_reg_io_base), ime_builtin_get_reg_base_buf_size());
	#endif
#endif

	//release
	if(ime_builtin_ctl.reg_data) {
		kdrv_ipp_builtin_plat_free(ime_builtin_ctl.reg_data);
		ime_builtin_ctl.reg_data = NULL;
	}

	return 0;
}
#endif
#endif
