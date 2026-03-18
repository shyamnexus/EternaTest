#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt-io.h>
#include <plat/top.h>
#else
#include <string.h>
#include <stdlib.h>
#include <plat/top.h>
#include "kwrap/error_no.h"
#endif
#include <libfdt.h>
#include <fdtfast.h>

#include "kwrap/cpu.h"
#include "kwrap/debug.h"
#include "vdoenc_builtin.h"

#include "iq_reg.h"
#include "iq_builtin.h"
#include "iq_dtsi.h"
#include "iq_lib_nvt.h"

#if (NVT_FAST_ISP_FLOW)
//=============================================================================
// define
//=============================================================================
#define DPC_ENABLE         1
#define ECS_ENABLE         1
#define IQ_BUILTIN_DBG_MSG       0
#define IQ_BUILTIN_DEFOG_DBG_MSG 0
#define IQ_BUILTIN_REG_MSG       0

#define LV_BASE                 1000000
#define IQ_3DNR_STABLE_FRM_CNT  10
#define IQ_LCE_STABLE_CNT       10
#define SUB_NODE_LENGTH 64

#if defined(__KERNEL__)
#define IOREMAP(addr, size) ioremap(addr, size)
#define IOUNMAP(addr) iounmap(addr)
#define SETREG(value, addr) nvt_writel(value, addr)
#define GETREG(addr) nvt_readl(addr)
#else
#define IOREMAP(addr, size) addr
#define IOUNMAP(addr)
#define SETREG(value, addr) OUTW(addr, value)
#define GETREG(addr) INW(addr)
#endif

//=============================================================================
// global
//=============================================================================
static ISOMAP iso_map_tab[IQ_GAIN_ID_MAX_NUM] = {
	{    100,      IQ_GAIN_1X},
	{    200,      IQ_GAIN_2X},
	{    400,      IQ_GAIN_4X},
	{    800,      IQ_GAIN_8X},
	{   1600,     IQ_GAIN_16X},
	{   3200,     IQ_GAIN_32X},
	{   6400,     IQ_GAIN_64X},
	{  12800,    IQ_GAIN_128X},
	{  25600,    IQ_GAIN_256X},
	{  51200,    IQ_GAIN_512X},
	{ 102400,   IQ_GAIN_1024X},
	{ 204800,   IQ_GAIN_2048X},
	{ 409600,   IQ_GAIN_4096X},
	{ 819200,   IQ_GAIN_8192X},
	{1638400,  IQ_GAIN_16384X},
	{3276800,  IQ_GAIN_32768X}
};

static UINT32 iq_sqrt_tab[41] = { // sqrt[x]*256, x = 0, 0.2, 0.4, ..., 8
	0, 114, 161, 198, 228, 256, 280, 302, 323, 343,
	362, 379, 396, 412, 428, 443, 457, 472, 485, 499,
	512, 524, 536, 549, 560, 572, 583, 594, 605, 616,
	627, 637, 647, 657, 667, 677, 686, 696, 705, 714,
	724
};

static UINT16 de_gamma[IQ_EDGE_GAMMA_LEN] = {
	0, 0, 0, 1, 2, 3, 4, 6, 9, 11,
	14, 18, 22, 26, 31, 36, 42, 48, 55, 63,
	70, 79, 88, 97, 107, 118, 129, 141, 153, 166,
	179, 193, 208, 223, 239, 255, 272, 290, 308, 327,
	347, 367, 388, 410, 432, 455, 479, 503, 528, 553,
	580, 607, 635, 663, 692, 722, 752, 784, 816, 848,
	882, 916, 951, 987, 1023};

static UINT32 iq_edge_dir_w_tab[17][4] = {
	{ 0,  0,  0,  0},
	{ 1,  1,  0,  0},
	{ 2,  1,  1,  0},
	{ 2,  2,  1,  0},
	{ 3,  2,  1,  0},
	{ 4,  2,  1,  0},
	{ 4,  4,  2,  0},
	{ 6,  4,  2,  0},
	{ 8,  4,  2,  0},
	{10,  5,  2,  0},
	{12,  6,  3,  0},
	{14,  8,  4,  0},
	{16,  8,  4,  0},
	{16, 10,  6,  2},
	{16, 12,  8,  4},
	{16, 16, 12,  8},
	{16, 16, 16, 16} };

UINT32 curr_gamma[ISP_BUILTIN_ID_MAX_NUM][IQ_GAMMA_LEN] = {0};
static UINT32 *gamma_buffer[ISP_BUILTIN_ID_MAX_NUM] = {NULL};
static ULONG gamma_buffer_pa_addr[ISP_BUILTIN_ID_MAX_NUM];
static UINT32 *ycurve_buffer[ISP_BUILTIN_ID_MAX_NUM] = {NULL};
static ULONG ycurve_buffer_pa_addr[ISP_BUILTIN_ID_MAX_NUM];

#if DPC_ENABLE
static UINT32 *dpc_buffer[ISP_BUILTIN_ID_MAX_NUM] = {NULL};
static ULONG dpc_buffer_pa_addr[ISP_BUILTIN_ID_MAX_NUM];
#endif

#if ECS_ENABLE
static UINT32 *ecs_buffer[ISP_BUILTIN_ID_MAX_NUM] = {NULL};
static ULONG ecs_buffer_pa_addr[ISP_BUILTIN_ID_MAX_NUM];
#endif

static IQ_PARAM_PTR *iq_param[ISP_BUILTIN_ID_MAX_NUM] = {NULL};
static IQ_PARAM_PTR iq_param_memalloc_addr[ISP_BUILTIN_ID_MAX_NUM] = {0};
static BOOL iq_param_memalloc_valid[ISP_BUILTIN_ID_MAX_NUM] = {0};
static UINT32 iq_edge_ker_tab[17][2] = {
	{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8},
	{1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8} };
static UINT8 iq_defog_diff_wt[8][IQ_DEFOG_OUTPUT_BLD_LEN] = {
	{0, 0, 0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
	{3, 3, 3, 6, 9, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12},
	{6, 6, 6, 9, 12, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15},
	{12, 12, 12, 16, 20, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24},
	{18, 18, 18, 22, 26, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
	{24, 24, 24, 28, 32, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36},
	{30, 30, 30, 34, 38, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42},
	{36, 36, 36, 40, 44, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48}
};

static UINT32 iq_frm_cnt[ISP_BUILTIN_ID_MAX_NUM] = {0};

IQ_BUILTIN_FRONT_PARAM iq_front_param_init = {
	IQ_3DNR_STABLE_FRM_CNT,
	{0, 0, 0, 10, 20, 40, 80, 100, 100, 100, 100, 100, 100, 100, 100, 100},
	150,
	150,
	25,
	60,
	60
};
IQ_BUILTIN_FRONT_PARAM iq_front_param_off = {
	0xFFFFFFFF,
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	100,
	100,
	100,
	100,
	100
};

IQ_BUILTIN_FRONT_PARAM iq_front_param[ISP_BUILTIN_ID_MAX_NUM] = {{0} };

IQ_BUILTIN_FRONT_FACTOR iq_front_factor[ISP_BUILTIN_ID_MAX_NUM] = {{0} };

// TODO: isp builtin
//ULONG ipe_subimg_vaddr = 0x0;

static UINT32 final_wdr_strength[IQ_ID_MAX_NUM] = {0};

#if defined(__KERNEL__)
void __iomem *sie1_reg_addr = NULL;
void __iomem *sie2_reg_addr = NULL;
void __iomem *sie3_reg_addr = NULL;
void __iomem *sie4_reg_addr = NULL;
void __iomem *sie5_reg_addr = NULL;
void __iomem *ife_reg_addr = NULL;
void __iomem *ipe_reg_addr = NULL;
void __iomem *ime_reg_addr = NULL;
#else
unsigned int sie1_reg_addr;
unsigned int sie2_reg_addr;
unsigned int sie3_reg_addr;
unsigned int sie4_reg_addr;
unsigned int sie5_reg_addr;
unsigned int ife_reg_addr;
unsigned int ipe_reg_addr;
unsigned int ime_reg_addr;
#endif

//=============================================================================
// iq cal
//=============================================================================
static INT32 iq_cal(INT32 ratio, INT32 int_value, INT32 min_value, INT32 max_value)
{
	if (ratio == 100) {
		return int_value;
	}
	return IQ_CLAMP(int_value * ratio / 100, min_value, max_value);
}

INT32 iq_intpl_value(INT32 ratio, INT32 int_value, INT32 min_value, INT32 max_value)
{
	if (ratio == 100) {
		return int_value;
	} else if (ratio > 100) {
		return iq_builtin_intpl(ratio, int_value, max_value, 100, 200);
	} else {
		return iq_builtin_intpl(ratio, min_value, int_value, 0, 100);
	}
}

static void iq_intpl_tbl_gamma(UINT32 *l_value, UINT32 *h_value, INT32 l_index, INT32 h_index, INT32 index, UINT32 *target)
{
	INT32 i;

	for (i = 0; i < IQ_GAMMA_LEN; i++) {
		target[i] = iq_builtin_intpl(index, l_value[i], h_value[i], l_index, h_index);
	}
}

static void iq_intpl_tbl_cc(INT16 *l_value, INT16 *h_value, INT32 l_index, INT32 h_index, INT32 index, INT16 *target)
{
	INT32 i, E1, E2;

	for (i = 0; i < IQ_CCM_LEN; i++) {
		E1 = l_value[i];
		if (E1 > 0x800) {
			E1 = E1 - 0x1000;
		}

		E2 = h_value[i];
		if (E2 > 0x800) {
			E2 = E2 - 0x1000;
		}
		target[i] = iq_builtin_intpl(index, E1, E2, l_index, h_index);
	}
}

//=============================================================================
// internal functions
//=============================================================================
static void iq_builtin_set_companding(IQ_PARAM_PTR *iq_param)
{
	T_R540_DECOMP                       decomp_kp_reg0;
	T_R5C0_DECOMP                       decomp_reg0;

	T_R470_COMP                         comp_l_reg0;
	T_R4F0_COMP                         comp_l_reg32;
	T_R4F4_COMP                         comp_m_reg0;
	T_R514_COMP                         comp_m_reg8;
	T_R518_COMP                         comp_r_reg0;

	IQ_COMPANDING_PARAM                 *companding;
	#if defined(__KERNEL__)
	void __iomem                        *reg_addr = NULL;
	#else
	unsigned int                        reg_addr;
	#endif

	INT32                               i;

	reg_addr = sie1_reg_addr;

	companding = iq_param->companding;

	for (i = 0; i < IQ_DECOMPANDING_KNEE_NUM; i++) {
		decomp_kp_reg0.bit.DECOMP_KPX0 = companding->decomp_kpx[i];
		decomp_kp_reg0.bit.DECOMP_KPY0 = companding->decomp_kpy[i];
		SETREG(decomp_kp_reg0.reg, reg_addr + (R540_DECOMP_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_DECOMPANDING_KNEE_NUM / 2); i++) {
		decomp_reg0.bit.DECOMP_GAIN0 = companding->decomp_gain[2*i+0];
		decomp_reg0.bit.DECOMP_SB0 = companding->decomp_sb[2*i+0];
		decomp_reg0.bit.DECOMP_GAIN1 = companding->decomp_gain[2*i+1];
		decomp_reg0.bit.DECOMP_SB1 = companding->decomp_sb[2*i+1];
		SETREG(decomp_reg0.reg, reg_addr + (R5C0_DECOMP_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_COMPANDING_LEFT_NUM / 2); i++) {
		comp_l_reg0.bit.COMP_FCURVE_L0 = companding->comp_fcurve_l[2*i+0];
		comp_l_reg0.bit.COMP_FCURVE_L1 = companding->comp_fcurve_l[2*i+0];
		SETREG(comp_l_reg0.reg, reg_addr + (R470_COMP_OFS + (i * 4)));
	}
	comp_l_reg32.bit.COMP_FCURVE_L64 = companding->comp_fcurve_l[64];
	SETREG(comp_l_reg32.reg, reg_addr + R4F0_COMP_OFS);

	for (i = 0; i < (IQ_COMPANDING_MIDDLE_NUM / 2); i++) {
		comp_m_reg0.bit.COMP_FCURVE_M0 = companding->comp_fcurve_m[2*i+0];
		comp_m_reg0.bit.COMP_FCURVE_M1 = companding->comp_fcurve_m[2*i+0];
		SETREG(comp_m_reg0.reg, reg_addr + (R4F4_COMP_OFS + (i * 4)));
	}
	comp_m_reg8.bit.COMP_FCURVE_M16 = companding->comp_fcurve_m[16];
	SETREG(comp_m_reg8.reg, reg_addr + R514_COMP_OFS);

	for (i = 0; i < (IQ_COMPANDING_RIGHT_NUM / 2); i++) {
		comp_r_reg0.bit.COMP_FCURVE_R0 = companding->comp_fcurve_r[2*i+0];
		comp_r_reg0.bit.COMP_FCURVE_R1 = companding->comp_fcurve_r[2*i+0];
		SETREG(comp_r_reg0.reg, reg_addr + (R518_COMP_OFS + (i * 4)));
	}
}

static void iq_builtin_set_sie_ob(UINT32 id, UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_RBC_BASIC_OB              sie_ob_ofs_reg;
	T_R2A8_STCS_OB              ca_la_ob_reg;
	T_R1E8_STCS                 ca_src_reg;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr_m = NULL, *reg_addr_sub = 0;
	#else
	unsigned int                reg_addr_m = 0, reg_addr_sub = 0;
	#endif

	IQ_OB_TUNE_PARAM            *final_ob_tune;

	UINT32                      single_ob;
	UINT32                      iso_idx = 0;
	INT32                       i;

	switch (id) {
	default:
	case 0:
		reg_addr_m = sie1_reg_addr;
		break;
	case 1:
		reg_addr_m = sie2_reg_addr;
		break;
	case 2:
		reg_addr_m = sie3_reg_addr;
		break;
	case 3:
		reg_addr_m = sie4_reg_addr;
		break;
	case 4:
		reg_addr_m = sie5_reg_addr;
		break;
	}

	if (isp_builtin_get_shdr_enable(id)) {
		for (i = ISP_BUILTIN_ID_MAX_NUM - 1; i >= 0; i--) {
			if ((0x1 << i) & isp_builtin_get_shdr_id_mask(id)) {
				break;
			}
		}

		switch (i) {
			default:
			case 0:
				reg_addr_sub = sie1_reg_addr;
				break;
			case 1:
				reg_addr_sub = sie2_reg_addr;
				break;
			case 2:
				reg_addr_sub = sie3_reg_addr;
				break;
			case 3:
				reg_addr_sub = sie4_reg_addr;
				break;
			case 4:
				reg_addr_sub = sie5_reg_addr;
				break;
		}
	}

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->ob->mode == IQ_OP_TYPE_AUTO) {
		final_ob_tune = &iq_param->ob->auto_param[iso_idx];
	} else {
		final_ob_tune = &iq_param->ob->manual_param;
	}
	single_ob = IQ_CLAMP(((final_ob_tune->cofs[1] + final_ob_tune->cofs[2]) >> 1), 0, 4095);

	// set short exp frame SIE
	sie_ob_ofs_reg.reg = GETREG(reg_addr_m + RBC_BASIC_OB_OFS);
	sie_ob_ofs_reg.bit.OB_OFS = single_ob;
	SETREG(sie_ob_ofs_reg.reg, reg_addr_m + RBC_BASIC_OB_OFS);

	// set long exp frame SIE
	if (reg_addr_sub != 0) {
		sie_ob_ofs_reg.reg = GETREG(reg_addr_sub + RBC_BASIC_OB_OFS);
		sie_ob_ofs_reg.bit.OB_OFS = single_ob;
		SETREG(sie_ob_ofs_reg.reg, reg_addr_sub + RBC_BASIC_OB_OFS);
	}

	if (nvt_get_chip_id() == CHIP_NA51055) {
		ca_la_ob_reg.bit.STCS_CA_OB_OFS = 0; // software operation at sie_getCAResultManual()
		ca_la_ob_reg.bit.STCS_LA_OB_OFS = single_ob;

		// set short exp frame SIE
		SETREG(ca_la_ob_reg.reg, reg_addr_m + R2A8_STCS_OB_OFS);

		// set long exp frame SIE
		if (reg_addr_sub != 0) {
			SETREG(ca_la_ob_reg.reg, reg_addr_sub + R2A8_STCS_OB_OFS);
		}
	} else {
		// set short exp frame SIE
		ca_src_reg.reg = GETREG(reg_addr_m + R1E8_STCS_OFS);
		ca_src_reg.bit.STCS_CA_ACCM_SRC = 1; // 520 don't exist this register
		SETREG(ca_src_reg.reg, reg_addr_m + R1E8_STCS_OFS);
		ca_la_ob_reg.bit.STCS_CA_OB_OFS = single_ob;
		ca_la_ob_reg.bit.STCS_LA_OB_OFS = single_ob;
		SETREG(ca_la_ob_reg.reg, reg_addr_m + R2A8_STCS_OB_OFS);

		// set long exp frame SIE
		if (reg_addr_sub != 0) {
			ca_src_reg.reg = GETREG(reg_addr_sub + R1E8_STCS_OFS);
			ca_src_reg.bit.STCS_CA_ACCM_SRC = 1; // 520 don't exist this register
			SETREG(ca_src_reg.reg, reg_addr_sub + R1E8_STCS_OFS);
			ca_la_ob_reg.bit.STCS_CA_OB_OFS = single_ob;
			ca_la_ob_reg.bit.STCS_LA_OB_OFS = single_ob;
			SETREG(ca_la_ob_reg.reg, reg_addr_sub + R2A8_STCS_OB_OFS);
		}
	}
}

static void iq_builtin_set_ife_ob(UINT32 id, UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_COLOR_OFFSET_REGISTER_0   ife_ob_reg_r;
	T_COLOR_OFFSET_REGISTER_1   ife_ob_reg_b;
	T_COLOR_OFFSET_REGISTER_2   ife_ob_reg_ir;
	T_COLOR_OFFSET_REGISTER_3   ife_fusion_ob_0_reg_r;
	T_COLOR_OFFSET_REGISTER_4   ife_fusion_ob_0_reg_b;
	T_COLOR_OFFSET_REGISTER_5   ife_fusion_ob_0_reg_ir;
	T_COLOR_OFFSET_REGISTER_6   ife_fusion_ob_1_reg_r;
	T_COLOR_OFFSET_REGISTER_7   ife_fusion_ob_1_reg_b;
	T_COLOR_OFFSET_REGISTER_8   ife_fusion_ob_1_reg_ir;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif
	IQ_OB_TUNE_PARAM            *final_ob_tune;

	UINT32                      iso_idx = 0;
	INT32                       i;

	reg_addr = ife_reg_addr;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->ob->mode == IQ_OP_TYPE_AUTO) {
		final_ob_tune = &iq_param->ob->auto_param[iso_idx];
	} else {
		final_ob_tune = &iq_param->ob->manual_param;
	}

	if (isp_builtin_get_shdr_enable(id)) {
		ife_ob_reg_r.bit.ife_cofs_r  = 0;
		ife_ob_reg_r.bit.ife_cofs_gr = 0;
		SETREG(ife_ob_reg_r.reg, reg_addr + COLOR_OFFSET_REGISTER_0_OFS);

		ife_ob_reg_b.bit.ife_cofs_gb = 0;
		ife_ob_reg_b.bit.ife_cofs_b  = 0;
		SETREG(ife_ob_reg_b.reg, reg_addr + COLOR_OFFSET_REGISTER_1_OFS);

		ife_ob_reg_ir.bit.ife_cofs_ir = 0;
		SETREG(ife_ob_reg_ir.reg, reg_addr + COLOR_OFFSET_REGISTER_2_OFS);
	} else {
		ife_ob_reg_r.bit.ife_cofs_r  = final_ob_tune->cofs[0];
		ife_ob_reg_r.bit.ife_cofs_gr = final_ob_tune->cofs[1];
		SETREG(ife_ob_reg_r.reg, reg_addr + COLOR_OFFSET_REGISTER_0_OFS);

		ife_ob_reg_b.bit.ife_cofs_gb = final_ob_tune->cofs[2];
		ife_ob_reg_b.bit.ife_cofs_b  = final_ob_tune->cofs[3];
		SETREG(ife_ob_reg_b.reg, reg_addr + COLOR_OFFSET_REGISTER_1_OFS);

		ife_ob_reg_ir.bit.ife_cofs_ir = final_ob_tune->cofs[4];
		SETREG(ife_ob_reg_ir.reg, reg_addr + COLOR_OFFSET_REGISTER_2_OFS);
	}

	if (isp_builtin_get_shdr_enable(id)) {
		ife_fusion_ob_0_reg_r.bit.ife_f_p0_cofs_r = final_ob_tune->cofs[0];
		ife_fusion_ob_0_reg_r.bit.ife_f_p0_cofs_gr = final_ob_tune->cofs[1];
		SETREG(ife_fusion_ob_0_reg_r.reg, reg_addr + COLOR_OFFSET_REGISTER_3_OFS);

		ife_fusion_ob_0_reg_b.bit.ife_f_p0_cofs_gb = final_ob_tune->cofs[2];
		ife_fusion_ob_0_reg_b.bit.ife_f_p0_cofs_b = final_ob_tune->cofs[3];
		SETREG(ife_fusion_ob_0_reg_b.reg, reg_addr + COLOR_OFFSET_REGISTER_4_OFS);

		ife_fusion_ob_0_reg_ir.bit.ife_f_p0_cofs_ir = final_ob_tune->cofs[4];
		SETREG(ife_fusion_ob_0_reg_ir.reg, reg_addr + COLOR_OFFSET_REGISTER_5_OFS);

		ife_fusion_ob_1_reg_r.bit.ife_f_p1_cofs_r = final_ob_tune->cofs[0];
		ife_fusion_ob_1_reg_r.bit.ife_f_p1_cofs_gr = final_ob_tune->cofs[1];
		SETREG(ife_fusion_ob_1_reg_r.reg, reg_addr + COLOR_OFFSET_REGISTER_6_OFS);

		ife_fusion_ob_1_reg_b.bit.ife_f_p1_cofs_gb = final_ob_tune->cofs[2];
		ife_fusion_ob_1_reg_b.bit.ife_f_p1_cofs_b = final_ob_tune->cofs[3];
		SETREG(ife_fusion_ob_1_reg_b.reg, reg_addr + COLOR_OFFSET_REGISTER_7_OFS);

		ife_fusion_ob_1_reg_ir.bit.ife_f_p1_cofs_ir = final_ob_tune->cofs[4];
		SETREG(ife_fusion_ob_1_reg_ir.reg, reg_addr + COLOR_OFFSET_REGISTER_8_OFS);
	} else {
		ife_fusion_ob_0_reg_r.bit.ife_f_p0_cofs_r = 0;
		ife_fusion_ob_0_reg_r.bit.ife_f_p0_cofs_gr = 0;
		SETREG(ife_fusion_ob_0_reg_r.reg, reg_addr + COLOR_OFFSET_REGISTER_3_OFS);

		ife_fusion_ob_0_reg_b.bit.ife_f_p0_cofs_gb = 0;
		ife_fusion_ob_0_reg_b.bit.ife_f_p0_cofs_b = 0;
		SETREG(ife_fusion_ob_0_reg_b.reg, reg_addr + COLOR_OFFSET_REGISTER_4_OFS);

		ife_fusion_ob_0_reg_ir.bit.ife_f_p0_cofs_ir = 0;
		SETREG(ife_fusion_ob_0_reg_ir.reg, reg_addr + COLOR_OFFSET_REGISTER_5_OFS);

		ife_fusion_ob_1_reg_r.bit.ife_f_p1_cofs_r = 0;
		ife_fusion_ob_1_reg_r.bit.ife_f_p1_cofs_gr = 0;
		SETREG(ife_fusion_ob_1_reg_r.reg, reg_addr + COLOR_OFFSET_REGISTER_6_OFS);

		ife_fusion_ob_1_reg_b.bit.ife_f_p1_cofs_gb = 0;
		ife_fusion_ob_1_reg_b.bit.ife_f_p1_cofs_b = 0;
		SETREG(ife_fusion_ob_1_reg_b.reg, reg_addr + COLOR_OFFSET_REGISTER_7_OFS);

		ife_fusion_ob_1_reg_ir.bit.ife_f_p1_cofs_ir = 0;
		SETREG(ife_fusion_ob_1_reg_ir.reg, reg_addr + COLOR_OFFSET_REGISTER_8_OFS);
	}
}

#if DPC_ENABLE
static void iq_builtin_set_dpc(UINT32 id, IQ_PARAM_PTR *iq_param)
{
	T_R48_ENGINE_DRAM                dpc_addr;
	T_R4_ENGINE_FUNCTION             dpc_en;

	#if defined(__KERNEL__)
	void __iomem                     *reg_addr_m = NULL, *reg_addr_sub = 0;
	#else
	unsigned int                     reg_addr_m = 0, reg_addr_sub = 0;
	#endif
	IQ_DPC_PARAM                     *dpc;

	ULONG                            phy_adr;
	INT32                            i;

	switch (id) {
		default:
		case 0:
			reg_addr_m = sie1_reg_addr;
			break;
		case 1:
			reg_addr_m = sie2_reg_addr;
			break;
		case 2:
			reg_addr_m = sie3_reg_addr;
			break;
		case 3:
			reg_addr_m = sie4_reg_addr;
			break;
		case 4:
			reg_addr_m = sie5_reg_addr;
			break;
	}

	if (isp_builtin_get_shdr_enable(id)) {
		for (i = ISP_BUILTIN_ID_MAX_NUM - 1; i >= 0; i--) {
			if ((0x1 << i) & isp_builtin_get_shdr_id_mask(id)) {
				break;
			}
		}

		switch (i) {
			default:
			case 0:
				reg_addr_sub = sie1_reg_addr;
				break;
			case 1:
				reg_addr_sub = sie2_reg_addr;
				break;
			case 2:
				reg_addr_sub = sie3_reg_addr;
				break;
			case 3:
				reg_addr_sub = sie4_reg_addr;
				break;
			case 4:
				reg_addr_sub = sie5_reg_addr;
				break;
		}
	}

	dpc = iq_param->dpc;

	// set short exp frame SIE
	dpc_en.reg = GETREG(reg_addr_m + R4_ENGINE_FUNCTION_OFS);
	dpc_en.bit.DPC_EN = dpc->enable;
	SETREG(dpc_en.reg, reg_addr_m + R4_ENGINE_FUNCTION_OFS);

	// set long exp frame SIE
	if (reg_addr_sub != 0) {
		dpc_en.reg = GETREG(reg_addr_sub + R4_ENGINE_FUNCTION_OFS);
		dpc_en.bit.DPC_EN = dpc->enable;
		SETREG(dpc_en.reg, reg_addr_sub + R4_ENGINE_FUNCTION_OFS);
	}

	if (dpc->enable) {
		int i;
		for (i = 0; i < DPC_TABLE_SIZE; i++) {
			dpc_buffer[id][i] = dpc->table[i];
		}

		vos_cpu_dcache_sync((ULONG)dpc_buffer[id], DPC_BUFFER_SIZE, VOS_DMA_TO_DEVICE);
		phy_adr = dpc_buffer_pa_addr[id];
		dpc_addr.bit.DRAM_IN1_SAI = phy_adr >> 2;

		// set short exp frame SIE
		SETREG(dpc_addr.reg, reg_addr_m + R48_ENGINE_DRAM_OFS);

		// set long exp frame SIE
		if (reg_addr_sub != 0) {
			SETREG(dpc_addr.reg, reg_addr_sub + R48_ENGINE_DRAM_OFS);
		}
	}
}
#endif

#if ECS_ENABLE
static void iq_builtin_set_ecs(UINT32 id, UINT32 ct, IQ_PARAM_PTR *iq_param)
{
	T_R50_ENGINE_DRAM                ecs_addr;
	T_R4_ENGINE_FUNCTION             ecs_en;

	#if defined(__KERNEL__)
	void __iomem                     *reg_addr_m = NULL, *reg_addr_sub = 0;
	#else
	unsigned int                     reg_addr_m = 0, reg_addr_sub = 0;
	#endif
	IQ_SHADING_PARAM                 *shading;
	IQ_SHADING_EXT_PARAM             *shading_ext;

	ULONG                            phy_adr;
	UINT32                           ecs_idx = 0;
	INT32                            i;

	switch (id) {
		default:
		case 0:
			reg_addr_m = sie1_reg_addr;
			break;
		case 1:
			reg_addr_m = sie2_reg_addr;
			break;
		case 2:
			reg_addr_m = sie3_reg_addr;
			break;
		case 3:
			reg_addr_m = sie4_reg_addr;
			break;
		case 4:
			reg_addr_m = sie5_reg_addr;
			break;
	}

	if (isp_builtin_get_shdr_enable(id)) {
		for (i = ISP_BUILTIN_ID_MAX_NUM - 1; i >= 0; i--) {
			if ((0x1 << i) & isp_builtin_get_shdr_id_mask(id)) {
				break;
			}
		}

		switch (i) {
			default:
			case 0:
				reg_addr_sub = sie1_reg_addr;
				break;
			case 1:
				reg_addr_sub = sie2_reg_addr;
				break;
			case 2:
				reg_addr_sub = sie3_reg_addr;
				break;
			case 3:
				reg_addr_sub = sie4_reg_addr;
				break;
			case 4:
				reg_addr_sub = sie5_reg_addr;
				break;
		}
	}

	shading = iq_param->shading;
	shading_ext = iq_param->shading_ext;

	if (ct < (iq_param->shading->ecs_smooth_l_m_ct_lower + iq_param->shading->ecs_smooth_l_m_ct_upper) / 2) {
		ecs_idx = IQ_ECS_TEMPERATURE_L;
	} else if (ct < (iq_param->shading->ecs_smooth_m_h_ct_lower + iq_param->shading->ecs_smooth_m_h_ct_upper) / 2) {
		ecs_idx = IQ_ECS_TEMPERATURE_M;
	} else {
		ecs_idx = IQ_ECS_TEMPERATURE_H;
	}

	if (iq_param->shading->mode == IQ_OP_TYPE_AUTO) {
		for (i = 0; i < ECS_TABLE_SIZE; i++) {
			ecs_buffer[id][i] = shading_ext->ecs_map_tbl[ecs_idx][i];
		}
	} else {
		for (i = 0; i < ECS_TABLE_SIZE; i++) {
			ecs_buffer[id][i] = shading->ecs_map_tbl[i];
		}
	}

	// set short exp frame SIE
	ecs_en.reg = GETREG(reg_addr_m + R4_ENGINE_FUNCTION_OFS);
	if ((ecs_en.bit.ECS_EN == 0) && (shading->ecs_enable == TRUE)) {
		shading->ecs_enable = FALSE;
		DBG_DUMP("loss ECS related register setting, force ECS DISABLE !! \r\n");
	}
	ecs_en.bit.ECS_EN = shading->ecs_enable;
	SETREG(ecs_en.reg, reg_addr_m + R4_ENGINE_FUNCTION_OFS);

	// set long exp frame SIE
	if (reg_addr_sub != 0) {
		ecs_en.reg = GETREG(reg_addr_sub + R4_ENGINE_FUNCTION_OFS);
		ecs_en.bit.ECS_EN = shading->ecs_enable;
		SETREG(ecs_en.reg, reg_addr_sub + R4_ENGINE_FUNCTION_OFS);
	}

	if (shading->ecs_enable) {
		vos_cpu_dcache_sync((ULONG)ecs_buffer[id], ECS_BUFFER_SIZE, VOS_DMA_TO_DEVICE);
		phy_adr = ecs_buffer_pa_addr[id];
		ecs_addr.bit.DRAM_IN2_SAI = phy_adr >> 2;

		// set short exp frame SIE
		SETREG(ecs_addr.reg, reg_addr_m + R50_ENGINE_DRAM_OFS);

		// set long exp frame SIE
		if (reg_addr_sub != 0) {
			SETREG(ecs_addr.reg, reg_addr_sub + R50_ENGINE_DRAM_OFS);
		}
	}
}
#endif

static void iq_builtin_set_dgain(UINT32 id, UINT32 value)
{
	T_R4_ENGINE_FUNCTION        sie_control_reg;
	T_R15C_BASIC_DGAIN          basic_dgain;

	#if defined(__KERNEL__)
	void __iomem                     *reg_addr_m = NULL, *reg_addr_sub = 0;
	#else
	unsigned int                     reg_addr_m = 0, reg_addr_sub = 0;
	#endif
	UINT32                      dgain = 0;
	INT32 i;

	switch (id) {
		default:
		case 0:
			reg_addr_m = sie1_reg_addr;
			break;
		case 1:
			reg_addr_m = sie2_reg_addr;
			break;
		case 2:
			reg_addr_m = sie3_reg_addr;
			break;
		case 3:
			reg_addr_m = sie4_reg_addr;
			break;
		case 4:
			reg_addr_m = sie5_reg_addr;
			break;
	}

	if (isp_builtin_get_shdr_enable(id)) {
		for (i = ISP_BUILTIN_ID_MAX_NUM - 1; i >= 0; i--) {
			if ((0x1 << i) & isp_builtin_get_shdr_id_mask(id)) {
				break;
			}
		}

		switch (i) {
			default:
			case 0:
				reg_addr_sub = sie1_reg_addr;
				break;
			case 1:
				reg_addr_sub = sie2_reg_addr;
				break;
			case 2:
				reg_addr_sub = sie3_reg_addr;
				break;
			case 3:
				reg_addr_sub = sie4_reg_addr;
				break;
			case 4:
				reg_addr_sub = sie5_reg_addr;
				break;
		}
	}

	dgain = value;

	// set short exp frame SIE
	sie_control_reg.reg = GETREG(reg_addr_m + R4_ENGINE_FUNCTION_OFS);
	sie_control_reg.bit.CGAIN_EN = 0;
	sie_control_reg.bit.DGAIN_EN = 1;
	sie_control_reg.bit.STCS_DGAIN_EN = 0;
	SETREG(sie_control_reg.reg, reg_addr_m + R4_ENGINE_FUNCTION_OFS);

	basic_dgain.bit.DGAIN_GAIN = IQ_CLAMP(dgain << 1, 0, 65535); // 3.7 to 8.8
	SETREG(basic_dgain.reg, reg_addr_m + R15C_BASIC_DGAIN_OFS);

	// set long exp frame SIE
	if (reg_addr_sub != 0) {
		sie_control_reg.reg = GETREG(reg_addr_sub + R4_ENGINE_FUNCTION_OFS);
		sie_control_reg.bit.CGAIN_EN = 0;
		sie_control_reg.bit.DGAIN_EN = 1;
		sie_control_reg.bit.STCS_DGAIN_EN = 0;
		SETREG(sie_control_reg.reg, reg_addr_sub + R4_ENGINE_FUNCTION_OFS);

		basic_dgain.bit.DGAIN_GAIN = IQ_CLAMP(dgain << 1, 0, 65535); // 3.7 to 8.8
		SETREG(basic_dgain.reg, reg_addr_sub + R15C_BASIC_DGAIN_OFS);
	}
}

static void iq_builtin_set_cgain(UINT32 id, ISP_BUILTIN_CGAIN *value, IQ_PARAM_PTR *iq_param, UINT32 gain, BOOL ui_night_mode)
{
	ISP_BUILTIN_CGAIN *cgain_temp = (ISP_BUILTIN_CGAIN *) value;
	T_IFE_COLOR_GAIN_REGISTER_0     ife_cgain_control_reg;
	T_IFE_COLOR_GAIN_REGISTER_1     ife_cgain_reg_r;
	T_IFE_COLOR_GAIN_REGISTER_2     ife_cgain_reg_b;
	T_IFE_COLOR_GAIN_REGISTER_3     ife_cgain_reg_ir;
	T_IFE_COLOR_GAIN_REGISTER_4     ife_fusion_cgain_0_reg_r;
	T_IFE_COLOR_GAIN_REGISTER_5     ife_fusion_cgain_0_reg_b;
	T_IFE_COLOR_GAIN_REGISTER_6     ife_fusion_cgain_0_reg_ir;
	T_IFE_COLOR_GAIN_REGISTER_7     ife_fusion_cgain_1_reg_r;
	T_IFE_COLOR_GAIN_REGISTER_8     ife_fusion_cgain_1_reg_b;
	T_IFE_COLOR_GAIN_REGISTER_9     ife_fusion_cgain_1_reg_ir;

	#if defined(__KERNEL__)
	void __iomem                    *reg_addr = NULL;
	#else
	unsigned int                    reg_addr;
	#endif 
	IQ_OB_TUNE_PARAM                *final_ob_tune;

	UINT32                          cgain_r, cgain_g, cgain_b;
	UINT32                          sie_cgain[3] = {0};
	UINT32                          ob_ratio;
	UINT32                          iso_idx = 0;
	INT32                           i;

	reg_addr = ife_reg_addr;

	cgain_r = cgain_temp->r;
	cgain_g = cgain_temp->g;
	cgain_b = cgain_temp->b;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->ob->mode == IQ_OP_TYPE_AUTO) {
		final_ob_tune = &iq_param->ob->auto_param[iso_idx];
	} else {
		final_ob_tune = &iq_param->ob->manual_param;
	}

	ob_ratio = (4096 - ((final_ob_tune->cofs[1] + final_ob_tune->cofs[2]) >> 1));
	sie_cgain[0] = IQ_CLAMP(((cgain_r << 12) + (ob_ratio - 1)) / ob_ratio, 0, 2047);
	sie_cgain[1] = IQ_CLAMP(((cgain_g << 12) + (ob_ratio - 1)) / ob_ratio, 0, 2047);
	sie_cgain[2] = IQ_CLAMP(((cgain_b << 12) + (ob_ratio - 1)) / ob_ratio, 0, 2047);

	if (ui_night_mode == TRUE) {
		sie_cgain[0] = sie_cgain[1];
		sie_cgain[2] = sie_cgain[1];
	}

	if (isp_builtin_get_shdr_enable(id)) {
		ife_cgain_control_reg.reg = GETREG(reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);
		ife_cgain_control_reg.bit.cgain_range = 1; // 3.7
		SETREG(ife_cgain_control_reg.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);

		ife_cgain_reg_r.bit.ife_cgain_r = 128;
		ife_cgain_reg_r.bit.ife_cgain_gr = 128;
		SETREG(ife_cgain_reg_r.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_1_OFS);

		ife_cgain_reg_b.bit.ife_cgain_gb = 128;
		ife_cgain_reg_b.bit.ife_cgain_b = 128;
		SETREG(ife_cgain_reg_b.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_2_OFS);

		ife_cgain_reg_ir.bit.ife_cgain_ir = 128;
		SETREG(ife_cgain_reg_ir.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_3_OFS);
	} else {
		if (sie_cgain[0] > 1023 || sie_cgain[2] > 1023) {
			ife_cgain_control_reg.reg = GETREG(reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);
			ife_cgain_control_reg.bit.cgain_range = 1; // 3.7
			SETREG(ife_cgain_control_reg.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);

			ife_cgain_reg_r.bit.ife_cgain_r = sie_cgain[0] >> 1;
			ife_cgain_reg_r.bit.ife_cgain_gr = sie_cgain[1] >> 1;
			SETREG(ife_cgain_reg_r.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_1_OFS);

			ife_cgain_reg_b.bit.ife_cgain_gb = sie_cgain[1] >> 1;
			ife_cgain_reg_b.bit.ife_cgain_b = sie_cgain[2] >> 1;
			SETREG(ife_cgain_reg_b.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_2_OFS);

			ife_cgain_reg_ir.bit.ife_cgain_ir = sie_cgain[1] >> 1;
			SETREG(ife_cgain_reg_ir.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_3_OFS);
		} else {
			ife_cgain_control_reg.reg = GETREG(reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);
			ife_cgain_control_reg.bit.cgain_range = 0; // 2.8
			SETREG(ife_cgain_control_reg.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);

			ife_cgain_reg_r.bit.ife_cgain_r = sie_cgain[0];
			ife_cgain_reg_r.bit.ife_cgain_gr = sie_cgain[1];
			SETREG(ife_cgain_reg_r.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_1_OFS);

			ife_cgain_reg_b.bit.ife_cgain_gb = sie_cgain[1];
			ife_cgain_reg_b.bit.ife_cgain_b = sie_cgain[2];
			SETREG(ife_cgain_reg_b.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_2_OFS);

			ife_cgain_reg_ir.bit.ife_cgain_ir = sie_cgain[1];
			SETREG(ife_cgain_reg_ir.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_3_OFS);
		}
	}

	if (isp_builtin_get_shdr_enable(id)) {
		if (sie_cgain[0] > 1023 || sie_cgain[2] > 1023) {
			ife_cgain_control_reg.reg = GETREG(reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);
			ife_cgain_control_reg.bit.ife_f_cgain_range = 1; // 3.7
			SETREG(ife_cgain_control_reg.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);

			ife_fusion_cgain_0_reg_r.bit.ife_f_p0_cgain_r = sie_cgain[0] >> 1;
			ife_fusion_cgain_0_reg_r.bit.ife_f_p0_cgain_gr = sie_cgain[1] >> 1;
			SETREG(ife_fusion_cgain_0_reg_r.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_4_OFS);

			ife_fusion_cgain_0_reg_b.bit.ife_f_p0_cgain_gb = sie_cgain[1] >> 1;
			ife_fusion_cgain_0_reg_b.bit.ife_f_p0_cgain_b = sie_cgain[2] >> 1;
			SETREG(ife_fusion_cgain_0_reg_b.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_5_OFS);

			ife_fusion_cgain_0_reg_ir.bit.ife_f_p0_cgain_ir = sie_cgain[1] >> 1;
			SETREG(ife_fusion_cgain_0_reg_ir.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_6_OFS);

			ife_fusion_cgain_1_reg_r.bit.ife_f_p1_cgain_r = sie_cgain[0] >> 1;
			ife_fusion_cgain_1_reg_r.bit.ife_f_p1_cgain_gr = sie_cgain[1] >> 1;
			SETREG(ife_fusion_cgain_1_reg_r.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_7_OFS);

			ife_fusion_cgain_1_reg_b.bit.ife_f_p1_cgain_gb = sie_cgain[1] >> 1;
			ife_fusion_cgain_1_reg_b.bit.ife_f_p1_cgain_b = sie_cgain[2] >> 1;
			SETREG(ife_fusion_cgain_1_reg_b.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_8_OFS);

			ife_fusion_cgain_1_reg_ir.bit.ife_f_p1_cgain_ir = sie_cgain[1] >> 1;
			SETREG(ife_fusion_cgain_1_reg_ir.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_9_OFS);
		} else {
			ife_cgain_control_reg.reg = GETREG(reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);
			ife_cgain_control_reg.bit.ife_f_cgain_range = 0; // 2.8
			SETREG(ife_cgain_control_reg.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);

			ife_fusion_cgain_0_reg_r.bit.ife_f_p0_cgain_r = sie_cgain[0];
			ife_fusion_cgain_0_reg_r.bit.ife_f_p0_cgain_gr = sie_cgain[1];
			SETREG(ife_fusion_cgain_0_reg_r.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_4_OFS);

			ife_fusion_cgain_0_reg_b.bit.ife_f_p0_cgain_gb = sie_cgain[1];
			ife_fusion_cgain_0_reg_b.bit.ife_f_p0_cgain_b = sie_cgain[2];
			SETREG(ife_fusion_cgain_0_reg_b.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_5_OFS);

			ife_fusion_cgain_0_reg_ir.bit.ife_f_p0_cgain_ir = sie_cgain[1];
			SETREG(ife_fusion_cgain_0_reg_ir.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_6_OFS);

			ife_fusion_cgain_1_reg_r.bit.ife_f_p1_cgain_r = sie_cgain[0];
			ife_fusion_cgain_1_reg_r.bit.ife_f_p1_cgain_gr = sie_cgain[1];
			SETREG(ife_fusion_cgain_1_reg_r.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_7_OFS);

			ife_fusion_cgain_1_reg_b.bit.ife_f_p1_cgain_gb = sie_cgain[1];
			ife_fusion_cgain_1_reg_b.bit.ife_f_p1_cgain_b = sie_cgain[2];
			SETREG(ife_fusion_cgain_1_reg_b.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_8_OFS);

			ife_fusion_cgain_1_reg_ir.bit.ife_f_p1_cgain_ir = sie_cgain[1];
			SETREG(ife_fusion_cgain_1_reg_ir.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_9_OFS);
		}
	} else {
		ife_cgain_control_reg.reg = GETREG(reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);
		ife_cgain_control_reg.bit.ife_f_cgain_range = 1; // 3.7
		SETREG(ife_cgain_control_reg.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_0_OFS);

		ife_fusion_cgain_0_reg_r.bit.ife_f_p0_cgain_r = 128;
		ife_fusion_cgain_0_reg_r.bit.ife_f_p0_cgain_gr = 128;
		SETREG(ife_fusion_cgain_0_reg_r.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_4_OFS);

		ife_fusion_cgain_0_reg_b.bit.ife_f_p0_cgain_gb = 128;
		ife_fusion_cgain_0_reg_b.bit.ife_f_p0_cgain_b = 128;
		SETREG(ife_fusion_cgain_0_reg_b.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_5_OFS);

		ife_fusion_cgain_0_reg_ir.bit.ife_f_p0_cgain_ir = 128;
		SETREG(ife_fusion_cgain_0_reg_ir.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_6_OFS);

		ife_fusion_cgain_1_reg_r.bit.ife_f_p1_cgain_r = 128;
		ife_fusion_cgain_1_reg_r.bit.ife_f_p1_cgain_gr = 128;
		SETREG(ife_fusion_cgain_1_reg_r.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_7_OFS);

		ife_fusion_cgain_1_reg_b.bit.ife_f_p1_cgain_gb = 128;
		ife_fusion_cgain_1_reg_b.bit.ife_f_p1_cgain_b = 128;
		SETREG(ife_fusion_cgain_1_reg_b.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_8_OFS);

		ife_fusion_cgain_1_reg_ir.bit.ife_f_p1_cgain_ir = 128;
		SETREG(ife_fusion_cgain_1_reg_ir.reg, reg_addr + IFE_COLOR_GAIN_REGISTER_9_OFS);
	}
}

static void iq_builtin_set_shdr(UINT32 id, UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_CONTROL_REGISTER                                             ife_control_reg;

	T_NRS_CONTROL_REGISTER                                         nrs_control_reg;
	T_NRS_REGISTER0                                                nrs_0_reg0;

	T_RHE_EVRATIO_REGISTER                                         ev_ratio;
	T_RHE_FUSION_REGISTER                                          fusion_reg;
	T_RHE_FUSION_LONG_EXP_NORMAL_BLEND_CURVE                       long_exp_nor;
	T_RHE_FUSION_SHORT_EXP_NORMAL_BLEND_CURVE                      short_exp_nor;
	T_RHE_FUSION_LONG_EXP_DIFF_BLEND_CURVE                         long_exp_diff;
	T_RHE_FUSION_SHORT_EXP_DIFF_BLEND_CURVE                        short_exp_diff;
	T_RHE_FUSION_SLOPE_0                                           slope_nor;
	T_RHE_FUSION_SLOPE_1                                           slope_diff;

	T_RHE_FUSION_MOTION_COMPENSATION                               fusion_mc_reg;
	T_RHE_FUSION_MOTION_COMPENSATION_LUT_0                         fusion_diff_w_lut_reg_0;
	T_RHE_FUSION_MOTION_COMPENSATION_LUT_2                         fusion_diff_w_lut_reg_e;

	T_RHE_FCURVE_CTRL                                              fcurve_control_reg;

	T_RHE_FCURVE_L_INDEX_REGISTER_0                                fcurve_curve_l_reg_0;
	T_RHE_FCURVE_L_INDEX_REGISTER_32                               fcurve_curve_l_reg_e;
	T_RHE_FCURVE_R_INDEX_REGISTER_0                                fcurve_curve_r_reg_0;
	T_RHE_FCURVE_R_INDEX_REGISTER_8                                fcurve_curve_r_reg_e;

	#if defined(__KERNEL__)
	void __iomem                                                   *reg_addr = NULL;
	#else
	unsigned int                                                   reg_addr;
	#endif 
	IQ_SHDR_PARAM                                                  *shdr;
	IQ_SHDR_TUNE_PARAM                                             *final_shdr_tune;

	ISP_BUILTIN_SHDR_EV_RATIO                                      *shdr_ev_ratio_array = NULL;
	UINT32                                                         shdr_frame_num = 0, shdr_ev_ratio = 1024, shdr_tm_ratio = 16384;
	IQLIB_SHDR_FCURVE_INFO                                         fcurve_input;
	IQLIB_SHDR_FCURVE                                              fcurve_output;
	UINT32                                                         final_fcurve_left_lut[IQ_SHDR_FCURVE_LEFT_NUM] = {0};
	UINT32                                                         final_fcurve_right_lut[IQ_SHDR_FCURVE_RIGHT_NUM] = {0};
	ISP_BUILTIN_FUNC                                               func_en = 0;
	UINT32                                                         iso_idx = 0;
	INT32                                                          i;

	reg_addr = ife_reg_addr;

	func_en = isp_builtin_get_func_en(id) & ISP_BUILTIN_FUNC_SHDR;
	if ((func_en == 0) || (isp_builtin_get_shdr_enable(id) == FALSE)) {
		nrs_control_reg.bit.ife_f_nrs0_en = FALSE;
		nrs_control_reg.bit.ife_f_nrs1_en = FALSE;
		SETREG(nrs_control_reg.reg, reg_addr + NRS_CONTROL_REGISTER_OFS);

		ife_control_reg.reg = GETREG(reg_addr + CONTROL_REGISTER_OFS);
		ife_control_reg.bit.f_fc_en = FALSE;
		SETREG(ife_control_reg.reg, reg_addr + CONTROL_REGISTER_OFS);
		return;
	}

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	shdr = iq_param->shdr;
	if (shdr->mode == IQ_OP_TYPE_AUTO) {
		final_shdr_tune = &shdr->auto_param[iso_idx];
	} else {
		final_shdr_tune = &shdr->manual_param;
	}

	for (i = ISP_BUILTIN_ID_MAX_NUM - 1; i >= 0; i--) {
		if ((0x1 << i) & isp_builtin_get_shdr_id_mask(id)) {
			shdr_frame_num++;
		}
	}
	if ((shdr_frame_num == 0) || (shdr_frame_num > ISP_SEN_MFRAME_MAX_NUM)) {
		DBG_DUMP("isp builtin get shdr_frame_num fail \r\n");
		shdr_frame_num = 2;
	}
	shdr_ev_ratio_array = isp_builtin_get_shdr_ev_ratio(id);
	shdr_ev_ratio = IQ_CLAMP(shdr_ev_ratio_array->ratio[1] >> 2, 16, 256);
	shdr_tm_ratio = IQ_CLAMP(isp_builtin_get_shdr_tm_ratio(id), 0, shdr_ev_ratio << 2);

	fcurve_input.dbg_en = FALSE;
	fcurve_input.auto_ev_en = shdr->auto_ev_enable;
	fcurve_input.frame_num = shdr_frame_num;
	fcurve_input.ev_ratio = shdr_ev_ratio;
	fcurve_input.tm_ratio = shdr_tm_ratio;
	fcurve_input.hbs_param.lum_th = 0;
	fcurve_input.hbs_param.w_start = 0;
	fcurve_input.hbs_param.w_slope = 100;
	fcurve_input.left_lut = shdr->fcurve_left_lut;
	fcurve_input.right_lut = shdr->fcurve_right_lut;
	fcurve_output.ev_fmt = 0;
	fcurve_output.left_lut = final_fcurve_left_lut;
	fcurve_output.right_lut = final_fcurve_right_lut;
	iq_lib_shdr_fcurve_nvt(&fcurve_input, &fcurve_output);

	nrs_control_reg.bit.ife_f_nrs0_en = shdr->nrs_enable;
	SETREG(nrs_control_reg.reg, reg_addr + NRS_CONTROL_REGISTER_OFS);

	for (i = 0; i < (IQ_SHDR_NRS_STR_NUM / 2); i++) {
		nrs_0_reg0.bit.ife_f_nrs0_str0 = final_shdr_tune->nrs_s_str[2*i+0];
		nrs_0_reg0.bit.ife_f_nrs0_str1 = final_shdr_tune->nrs_s_str[2*i+1];
		SETREG(nrs_0_reg0.reg, reg_addr + (NRS_REGISTER0_OFS + (i * 4)));
	}

	ev_ratio.bit.ife_f_fusion_evratio = shdr_ev_ratio;
	SETREG(ev_ratio.reg, reg_addr + RHE_EVRATIO_REGISTER_OFS);

	fusion_reg.reg = GETREG(reg_addr + RHE_FUSION_REGISTER_OFS);
	fusion_reg.bit.ife_f_fusion_normal_blend_curve_sel = shdr->fusion_nor_sel;
	fusion_reg.bit.ife_f_fusion_diff_blend_curve_sel = shdr->fusion_dif_sel;
	fusion_reg.bit.ife_f_fusion_mode = 0; // 0:blend, 1:short, 2:long, 3:very short
	SETREG(fusion_reg.reg, reg_addr + RHE_FUSION_REGISTER_OFS);

	if (fusion_reg.bit.ife_f_fusion_mode == 0) {
		ife_control_reg.reg = GETREG(reg_addr + CONTROL_REGISTER_OFS);
		ife_control_reg.bit.f_fc_en = 1;
	} else {
		ife_control_reg.reg = GETREG(reg_addr + CONTROL_REGISTER_OFS);
		ife_control_reg.bit.f_fc_en = 0;
	}
	SETREG(ife_control_reg.reg, reg_addr + CONTROL_REGISTER_OFS);

	long_exp_nor.bit.ife_f_fusion_long_exp_normal_blend_curve_knee_point0 = shdr->fusion_l_nor_knee;
	long_exp_nor.bit.ife_f_fusion_long_exp_normal_blend_curve_range = shdr->fusion_l_nor_range;
	SETREG(long_exp_nor.reg, reg_addr + RHE_FUSION_LONG_EXP_NORMAL_BLEND_CURVE_OFS);

	if (iq_param->shdr->auto_ev_enable == TRUE) {
		short_exp_nor.bit.ife_f_fusion_short_exp_normal_blend_curve_knee_point0 = (shdr->fusion_s_nor_knee * iq_builtin_fcurve_y_bound[256]) / iq_builtin_fcurve_y_bound[shdr_ev_ratio];
		short_exp_nor.bit.ife_f_fusion_short_exp_normal_blend_curve_range = (shdr->fusion_s_nor_range * iq_builtin_fcurve_y_bound[256]) / iq_builtin_fcurve_y_bound[shdr_ev_ratio];
	} else {
		short_exp_nor.bit.ife_f_fusion_short_exp_normal_blend_curve_knee_point0 = shdr->fusion_s_nor_knee;
		short_exp_nor.bit.ife_f_fusion_short_exp_normal_blend_curve_range = shdr->fusion_s_nor_range;
	}
	SETREG(short_exp_nor.reg, reg_addr + RHE_FUSION_SHORT_EXP_NORMAL_BLEND_CURVE_OFS);

	long_exp_diff.bit.ife_f_fusion_long_exp_diff_blend_curve_knee_point0 = shdr->fusion_l_dif_knee;
	long_exp_diff.bit.ife_f_fusion_long_exp_diff_blend_curve_range = shdr->fusion_l_dif_range;
	SETREG(long_exp_diff.reg, reg_addr + RHE_FUSION_LONG_EXP_DIFF_BLEND_CURVE_OFS);

	if (iq_param->shdr->auto_ev_enable == TRUE) {
		short_exp_diff.bit.ife_f_fusion_short_exp_diff_blend_curve_knee_point0 = (shdr->fusion_s_dif_knee * iq_builtin_fcurve_y_bound[256]) / iq_builtin_fcurve_y_bound[shdr_ev_ratio];
		short_exp_diff.bit.ife_f_fusion_short_exp_diff_blend_curve_range = (shdr->fusion_s_dif_range * iq_builtin_fcurve_y_bound[256]) / iq_builtin_fcurve_y_bound[shdr_ev_ratio];
	} else {
		short_exp_diff.bit.ife_f_fusion_short_exp_diff_blend_curve_knee_point0 = shdr->fusion_s_dif_knee;
		short_exp_diff.bit.ife_f_fusion_short_exp_diff_blend_curve_range = shdr->fusion_s_dif_range;
	}
	SETREG(short_exp_diff.reg, reg_addr + RHE_FUSION_SHORT_EXP_DIFF_BLEND_CURVE_OFS);

	slope_nor.bit.ife_f_fusion_bcnl_slope = 256 * 4096 / iq_param->shdr->fusion_l_nor_range;
	slope_nor.bit.ife_f_fusion_bcns_slope = 256 * 4096 / short_exp_nor.bit.ife_f_fusion_short_exp_normal_blend_curve_range;
	SETREG(slope_nor.reg, reg_addr + RHE_FUSION_SLOPE_0_OFS);

	slope_diff.bit.ife_f_fusion_bcdl_slope = 256 * 4096 / iq_param->shdr->fusion_l_dif_range;
	slope_diff.bit.ife_f_fusion_bcds_slope = 256 * 4096 / short_exp_diff.bit.ife_f_fusion_short_exp_diff_blend_curve_range;
	SETREG(slope_diff.reg, reg_addr + RHE_FUSION_SLOPE_1_OFS);

	fusion_mc_reg.reg = GETREG(reg_addr + RHE_FUSION_MOTION_COMPENSATION_OFS);
	fusion_mc_reg.bit.ife_f_fusion_mc_lumthr = shdr->fusion_lum_th;
	SETREG(fusion_mc_reg.reg, reg_addr + RHE_FUSION_MOTION_COMPENSATION_OFS);

	for (i = 0; i < IQ_SHDR_FUSION_DIFF_W_NUM / 6; i++) {
		fusion_diff_w_lut_reg_0.bit.ife_f_fusion_mc_lut_diff_weight0 = shdr->fusion_diff_w[6*i+0];
		fusion_diff_w_lut_reg_0.bit.ife_f_fusion_mc_lut_diff_weight1 = shdr->fusion_diff_w[6*i+1];
		fusion_diff_w_lut_reg_0.bit.ife_f_fusion_mc_lut_diff_weight2 = shdr->fusion_diff_w[6*i+2];
		fusion_diff_w_lut_reg_0.bit.ife_f_fusion_mc_lut_diff_weight3 = shdr->fusion_diff_w[6*i+3];
		fusion_diff_w_lut_reg_0.bit.ife_f_fusion_mc_lut_diff_weight4 = shdr->fusion_diff_w[6*i+4];
		fusion_diff_w_lut_reg_0.bit.ife_f_fusion_mc_lut_diff_weight5 = shdr->fusion_diff_w[6*i+5];
		SETREG(fusion_diff_w_lut_reg_0.reg, reg_addr + (RHE_FUSION_MOTION_COMPENSATION_LUT_0_OFS + (i * 4)));
	}

	fusion_diff_w_lut_reg_e.reg = GETREG(reg_addr + RHE_FUSION_MOTION_COMPENSATION_LUT_2_OFS);
	fusion_diff_w_lut_reg_e.bit.ife_f_fusion_mc_lut_diff_weight12 = shdr->fusion_diff_w[12];
	fusion_diff_w_lut_reg_e.bit.ife_f_fusion_mc_lut_diff_weight13 = shdr->fusion_diff_w[13];
	fusion_diff_w_lut_reg_e.bit.ife_f_fusion_mc_lut_diff_weight14 = shdr->fusion_diff_w[14];
	fusion_diff_w_lut_reg_e.bit.ife_f_fusion_mc_lut_diff_weight15 = shdr->fusion_diff_w[15];
	SETREG(fusion_diff_w_lut_reg_e.reg, reg_addr + RHE_FUSION_MOTION_COMPENSATION_LUT_2_OFS);

	fcurve_control_reg.bit.ife_f_fcurve_ymean_select = shdr->fcurve_y_mean_sel;
	fcurve_control_reg.bit.ife_f_fcurve_yvweight = shdr->fcurve_yv_w;
	SETREG(fcurve_control_reg.reg, reg_addr + RHE_FCURVE_CTRL_OFS);

	for (i = 0; i < IQ_SHDR_FCURVE_LEFT_NUM / 2; i++) {
		fcurve_curve_l_reg_0.bit.ife_f_fcurve_l_0 = final_fcurve_left_lut[2*i+0];
		fcurve_curve_l_reg_0.bit.ife_f_fcurve_l_1 = final_fcurve_left_lut[2*i+1];
		SETREG(fcurve_curve_l_reg_0.reg, reg_addr + (RHE_FCURVE_L_INDEX_REGISTER_0_OFS + (i * 4)));
	}
	fcurve_curve_l_reg_e.bit.ife_f_fcurve_l_64 = final_fcurve_left_lut[64];
	SETREG(fcurve_curve_l_reg_e.reg, reg_addr + RHE_FCURVE_L_INDEX_REGISTER_32_OFS);

	for (i = 0; i < IQ_SHDR_FCURVE_RIGHT_NUM / 2; i++) {
		fcurve_curve_r_reg_0.bit.ife_f_fcurve_r_0 = final_fcurve_right_lut[2*i+0];
		fcurve_curve_r_reg_0.bit.ife_f_fcurve_r_1 = final_fcurve_right_lut[2*i+1];
		SETREG(fcurve_curve_r_reg_0.reg, reg_addr + (RHE_FCURVE_R_INDEX_REGISTER_0_OFS + (i * 4)));
	}
	fcurve_curve_r_reg_e.bit.ife_f_fcurve_ev_fmt = fcurve_output.ev_fmt;
	fcurve_curve_r_reg_e.bit.ife_f_fcurve_r_16 = final_fcurve_right_lut[16];
	SETREG(fcurve_curve_r_reg_e.reg, reg_addr + RHE_FCURVE_R_INDEX_REGISTER_8_OFS);
}

static void iq_builtin_set_nr(UINT32 id, ISP_BUILTIN_IQ_TRIG_MSG msg, UINT32 gain, ISP_BUILTIN_CGAIN *cgain, IQ_PARAM_PTR *iq_param, UINT32 ui_nr_ratio)
{
	T_CONTROL_REGISTER                                                        ife_en_reg;

	T_OUTLIER_THRESHOLD_REGISTER_1                                            ife_outl_th_reg;
	T_OUTLIER_THRESHOLD_REGISTER_6                                            ife_outl_cnt_reg;
	T_OUTLIER_ORDER_REGISTER_0                                                ife_outl_ord_reg;

	T_GBALANCE_REGISTER                                                       gbalance_luma_low_bnd;
	T_GBAL_REGISTER_0                                                         gbalance_reg_0;
	T_GBAL_REGISTER_1                                                         gbalance_reg_1;

	T_RANGE_FILTER_REGISTER_1                                                 ife_filter_a_r_th_reg;
	T_RANGE_FILTER_REGISTER_5                                                 ife_filter_a_gr_th_reg;
	T_RANGE_FILTER_REGISTER_9                                                 ife_filter_a_gb_th_reg;
	T_RANGE_FILTER_REGISTER_13                                                ife_filter_a_b_th_reg;

	T_RANGE_FILTER_REGISTER_17                                                ife_filter_b_r_th_reg;
	T_RANGE_FILTER_REGISTER_21                                                ife_filter_b_gr_th_reg;
	T_RANGE_FILTER_REGISTER_25                                                ife_filter_b_gb_th_reg;
	T_RANGE_FILTER_REGISTER_29                                                ife_filter_b_b_th_reg;

	T_RANGE_FILTER_REGISTER_32                                                ife_filter_a_r_lut_reg_0;
	T_RANGE_FILTER_REGISTER_40                                                ife_filter_a_r_lut_reg_e;
	T_RANGE_FILTER_REGISTER_41                                                ife_filter_a_gr_lut_reg_0;
	T_RANGE_FILTER_REGISTER_49                                                ife_filter_a_gr_lut_reg_e;
	T_RANGE_FILTER_REGISTER_50                                                ife_filter_a_gb_lut_reg_0;
	T_RANGE_FILTER_REGISTER_58                                                ife_filter_a_gb_lut_reg_e;
	T_RANGE_FILTER_REGISTER_59                                                ife_filter_a_b_lut_reg_0;
	T_RANGE_FILTER_REGISTER_67                                                ife_filter_a_b_lut_reg_e;

	T_RANGE_FILTER_REGISTER_68                                                ife_filter_b_r_lut_reg_0;
	T_RANGE_FILTER_REGISTER_76                                                ife_filter_b_r_lut_reg_e;
	T_RANGE_FILTER_REGISTER_77                                                ife_filter_b_gr_lut_reg_0;
	T_RANGE_FILTER_REGISTER_85                                                ife_filter_b_gr_lut_reg_e;
	T_RANGE_FILTER_REGISTER_86                                                ife_filter_b_gb_lut_reg_0;
	T_RANGE_FILTER_REGISTER_94                                                ife_filter_b_gb_lut_reg_e;
	T_RANGE_FILTER_REGISTER_95                                                ife_filter_b_b_lut_reg_0;
	T_RANGE_FILTER_REGISTER_103                                               ife_filter_b_b_lut_reg_e;

	T_OUTPUT_FILTER_REGISTER                                                  ife_filter_clamp_reg;
	T_RANGE_FILTER_REGISTER_0                                                 ife_filter_blend_w_reg;


	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif 
	IQ_NR_TUNE_PARAM            *final_nr_tune;

	UINT32                      iso_idx = 0;
	UINT32                      outl_cnt0, outl_cnt1;
	UINT32                      curr_rgain_sqrt_id = 0, curr_bgain_sqrt_id = 0;
	UINT32                      curr_rgain_sqrt = 0, curr_bgain_sqrt = 0;
	UINT32                      filter_th_r[IQ_NR_TH_NUM], filter_th_g[IQ_NR_TH_NUM], filter_th_b[IQ_NR_TH_NUM];
	UINT32                      filter_lut_r[IQ_NR_TH_LUT], filter_lut_g[IQ_NR_TH_LUT], filter_lut_b[IQ_NR_TH_LUT];
	UINT32                      filter_th_b_r[IQ_NR_TH_NUM], filter_th_b_g[IQ_NR_TH_NUM], filter_th_b_b[IQ_NR_TH_NUM];
	UINT32                      filter_lut_b_r[IQ_NR_TH_LUT], filter_lut_b_g[IQ_NR_TH_LUT], filter_lut_b_b[IQ_NR_TH_LUT];
	UINT32                      filter_clamp_th;
	int                         i;

	reg_addr = ife_reg_addr;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->nr->mode == IQ_OP_TYPE_AUTO) {
		final_nr_tune = &iq_param->nr->auto_param[iso_idx];
	} else {
		final_nr_tune = &iq_param->nr->manual_param;
	}

	if (final_nr_tune->outl_sel == IQ_NR_OUTL_8_NODE) {
		outl_cnt0 = 1;
		outl_cnt1 = 0;
	} else if (final_nr_tune->outl_sel == IQ_NR_OUTL_7_NODE) {
		outl_cnt0 = 3;
		outl_cnt1 = 2;
	} else {
		outl_cnt0 = 1;
		outl_cnt1 = 0;
	}

	// NR_enable
	ife_en_reg.reg = GETREG(reg_addr + CONTROL_REGISTER_OFS);
	ife_en_reg.bit.outl_en = iq_param->nr->outl_enable;
	ife_en_reg.bit.gbal_en = iq_param->nr->gbal_enable;
	ife_en_reg.bit.filter_en = iq_param->nr->filter_enable;
	SETREG(ife_en_reg.reg, reg_addr + CONTROL_REGISTER_OFS);

	// Outlier
	for (i = 0; i < IQ_NR_OUTL_LEN; i++) {
		ife_outl_th_reg.bit.ife_outlth_bri0 = final_nr_tune->outl_bright_th[i];
		ife_outl_th_reg.bit.ife_outlth_dark0 = final_nr_tune->outl_dark_th[i];
		SETREG(ife_outl_th_reg.reg, reg_addr + (OUTLIER_THRESHOLD_REGISTER_1_OFS + (i * 4)));
	}

	ife_outl_cnt_reg.reg = GETREG(reg_addr + OUTLIER_THRESHOLD_REGISTER_6_OFS);
	ife_outl_cnt_reg.bit.ife_outl_compensate_mode = final_nr_tune->outl_avg_mode;
	ife_outl_cnt_reg.bit.ife_outl_cnt1 = outl_cnt0;
	ife_outl_cnt_reg.bit.ife_outl_cnt2 = outl_cnt1;
	SETREG(ife_outl_cnt_reg.reg, reg_addr + OUTLIER_THRESHOLD_REGISTER_6_OFS);

	ife_outl_ord_reg.reg = GETREG(reg_addr + OUTLIER_ORDER_REGISTER_0_OFS);
	ife_outl_ord_reg.bit.ife_ord_protect_th = final_nr_tune->outl_ord_protect_th;
	ife_outl_ord_reg.bit.ife_ord_blend_weight = final_nr_tune->outl_ord_blend_w;
	SETREG(ife_outl_ord_reg.reg, reg_addr + OUTLIER_ORDER_REGISTER_0_OFS);

	// Gbalance
	gbalance_luma_low_bnd.bit.ife_gbal_str_luma_low_bnd = final_nr_tune->gbal_str_luma_low_bnd;
	gbalance_luma_low_bnd.bit.ife_gbal_edge_luma_low_bnd = final_nr_tune->gbal_edge_luma_low_bnd;
	SETREG(gbalance_luma_low_bnd.reg, reg_addr + GBALANCE_REGISTER_OFS);

	gbalance_reg_0.reg = GETREG(reg_addr + GBAL_REGISTER_0_OFS);
	gbalance_reg_0.bit.ife_gbal_diff_thr_str = final_nr_tune->gbal_diff_th_str;
	SETREG(gbalance_reg_0.reg, reg_addr + GBAL_REGISTER_0_OFS);

	gbalance_reg_1.bit.ife_gbal_edge_thr_0 = final_nr_tune->gbal_edge_protect_th;
	gbalance_reg_1.bit.ife_gbal_edge_thr_1 = final_nr_tune->gbal_edge_protect_th * 125 / 100;
	SETREG(gbalance_reg_1.reg, reg_addr + GBAL_REGISTER_1_OFS);

	// 2DNR Filter_th
	if (isp_builtin_get_shdr_enable(id)) {
		if (cgain->g == 0) {
			curr_rgain_sqrt = 256;
			curr_bgain_sqrt = 256;
		} else {
			curr_rgain_sqrt_id = (cgain->r * 5) / cgain->g;
			curr_bgain_sqrt_id = (cgain->b * 5) / cgain->g;
			curr_rgain_sqrt = iq_builtin_intpl(cgain->r, iq_sqrt_tab[curr_rgain_sqrt_id], iq_sqrt_tab[curr_rgain_sqrt_id + 1], (curr_rgain_sqrt_id * cgain->g) / 5, ((curr_rgain_sqrt_id + 1) * cgain->g) / 5);
			curr_bgain_sqrt = iq_builtin_intpl(cgain->b, iq_sqrt_tab[curr_bgain_sqrt_id], iq_sqrt_tab[curr_bgain_sqrt_id + 1], (curr_bgain_sqrt_id * cgain->g) / 5, ((curr_bgain_sqrt_id + 1) * cgain->g) / 5);
		}
	} else {
		curr_rgain_sqrt = 256;
		curr_bgain_sqrt = 256;
	}
	for (i = 0; i < IQ_NR_TH_NUM; i++) {
		filter_th_r[i] = (UINT32)iq_cal((curr_rgain_sqrt * ui_nr_ratio) >> 8, final_nr_tune->filter_th[i], 0, 1023);
		filter_th_r[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm, filter_th_r[i], 0, 1023);

		filter_th_g[i] = (UINT32)iq_cal(ui_nr_ratio, final_nr_tune->filter_th[i], 0, 1023);
		filter_th_g[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm, filter_th_g[i], 0, 1023);

		filter_th_b[i] = (UINT32)iq_cal((curr_bgain_sqrt * ui_nr_ratio) >> 8, final_nr_tune->filter_th[i], 0, 1023);
		filter_th_b[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm, filter_th_b[i], 0, 1023);

		filter_th_b_r[i] = (UINT32)iq_cal((curr_rgain_sqrt * ui_nr_ratio) >> 8, final_nr_tune->filter_th_b[i], 0, 1023);
		filter_th_b_r[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm, filter_th_b_r[i], 0, 1023);

		filter_th_b_g[i] = (UINT32)iq_cal(ui_nr_ratio, final_nr_tune->filter_th_b[i], 0, 1023);
		filter_th_b_g[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm, filter_th_b_g[i], 0, 1023);

		filter_th_b_b[i] = (UINT32)iq_cal((curr_bgain_sqrt * ui_nr_ratio) >> 8, final_nr_tune->filter_th_b[i], 0, 1023);
		filter_th_b_b[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm, filter_th_b_b[i], 0, 1023);
	}
	for (i = 0; i < IQ_NR_TH_LUT; i++) {
		filter_lut_r[i] = (UINT32)iq_cal((curr_rgain_sqrt * ui_nr_ratio) >> 8, final_nr_tune->filter_lut[i], 0, 1023);
		filter_lut_r[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm_lut, filter_lut_r[i], 0, 1023);

		filter_lut_g[i] = (UINT32)iq_cal(ui_nr_ratio, final_nr_tune->filter_lut[i], 0, 1023);
		filter_lut_g[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm_lut, filter_lut_g[i], 0, 1023);

		filter_lut_b[i] = (UINT32)iq_cal((curr_bgain_sqrt * ui_nr_ratio) >> 8, final_nr_tune->filter_lut[i], 0, 1023);
		filter_lut_b[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm_lut, filter_lut_b[i], 0, 1023);

		filter_lut_b_r[i] = (UINT32)iq_cal((curr_rgain_sqrt * ui_nr_ratio) >> 8, final_nr_tune->filter_lut_b[i], 0, 1023);
		filter_lut_b_r[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm_lut, filter_lut_b_r[i], 0, 1023);

		filter_lut_b_g[i] = (UINT32)iq_cal(ui_nr_ratio, final_nr_tune->filter_lut_b[i], 0, 1023);
		filter_lut_b_g[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm_lut, filter_lut_b_g[i], 0, 1023);

		filter_lut_b_b[i] = (UINT32)iq_cal((curr_bgain_sqrt * ui_nr_ratio) >> 8, final_nr_tune->filter_lut_b[i], 0, 1023);
		filter_lut_b_b[i] = (UINT32)iq_cal(iq_front_factor[id].rth_nlm_lut, filter_lut_b_b[i], 0, 1023);
	}
	filter_clamp_th = (UINT32)iq_cal(200 - ui_nr_ratio, final_nr_tune->filter_clamp_th, 0, 1023);

	for (i = 0; i < (IQ_NR_TH_NUM / 2); i++) {
		ife_filter_a_r_th_reg.bit.ife_rth_nlm_c0_0 = filter_th_r[2*i+0];
		ife_filter_a_r_th_reg.bit.ife_rth_nlm_c0_1 = filter_th_r[2*i+1];
		SETREG(ife_filter_a_r_th_reg.reg, reg_addr + (RANGE_FILTER_REGISTER_1_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_NR_TH_NUM / 2); i++) {
		ife_filter_a_gr_th_reg.bit.ife_rth_nlm_c1_0 = filter_th_g[2*i+0];
		ife_filter_a_gr_th_reg.bit.ife_rth_nlm_c1_1 = filter_th_g[2*i+1];
		SETREG(ife_filter_a_gr_th_reg.reg, reg_addr + (RANGE_FILTER_REGISTER_5_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_NR_TH_NUM / 2); i++) {
		ife_filter_a_gb_th_reg.bit.ife_rth_nlm_c2_0 = filter_th_g[2*i+0];
		ife_filter_a_gb_th_reg.bit.ife_rth_nlm_c2_1 = filter_th_g[2*i+1];
		SETREG(ife_filter_a_gb_th_reg.reg, reg_addr + (RANGE_FILTER_REGISTER_9_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_NR_TH_NUM / 2); i++) {
		ife_filter_a_b_th_reg.bit.ife_rth_nlm_c3_0 = filter_th_b[2*i+0];
		ife_filter_a_b_th_reg.bit.ife_rth_nlm_c3_1 = filter_th_b[2*i+1];
		SETREG(ife_filter_a_b_th_reg.reg, reg_addr + (RANGE_FILTER_REGISTER_13_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_NR_TH_NUM / 2); i++) {
		ife_filter_b_r_th_reg.bit.ife_rth_bilat_c0_0 = filter_th_b_r[2*i+0];
		ife_filter_b_r_th_reg.bit.ife_rth_bilat_c0_1 = filter_th_b_r[2*i+1];
		SETREG(ife_filter_b_r_th_reg.reg, reg_addr + (RANGE_FILTER_REGISTER_17_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_NR_TH_NUM / 2); i++) {
		ife_filter_b_gr_th_reg.bit.ife_rth_bilat_c1_0 = filter_th_b_g[2*i+0];
		ife_filter_b_gr_th_reg.bit.ife_rth_bilat_c1_1 = filter_th_b_g[2*i+1];
		SETREG(ife_filter_b_gr_th_reg.reg, reg_addr + (RANGE_FILTER_REGISTER_21_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_NR_TH_NUM / 2); i++) {
		ife_filter_b_gb_th_reg.bit.ife_rth_bilat_c2_0 = filter_th_b_g[2*i+0];
		ife_filter_b_gb_th_reg.bit.ife_rth_bilat_c2_1 = filter_th_b_g[2*i+1];
		SETREG(ife_filter_b_gb_th_reg.reg, reg_addr + (RANGE_FILTER_REGISTER_25_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_NR_TH_NUM / 2); i++) {
		ife_filter_b_b_th_reg.bit.ife_rth_bilat_c3_0 = filter_th_b_b[2*i+0];
		ife_filter_b_b_th_reg.bit.ife_rth_bilat_c3_1 = filter_th_b_b[2*i+1];
		SETREG(ife_filter_b_b_th_reg.reg, reg_addr + (RANGE_FILTER_REGISTER_29_OFS + (i * 4)));
	}

	#if IQ_BUILTIN_DBG_MSG
	if (iq_frm_cnt[id] < iq_front_param[id].smooth_thr) {
		DBG_DUMP("id = %d, cnt = %d, rth_nlm = {%d, %d, %d, %d, %d, %d}, rth_bilat = {%d, %d, %d, %d, %d, %d}\r\n", id, iq_frm_cnt[id],
			filter_th_g[0], filter_th_g[1], filter_th_g[2], filter_th_g[3], filter_th_g[4], filter_th_g[5],
			filter_th_b_g[0], filter_th_b_g[1], filter_th_b_g[2], filter_th_b_g[3], filter_th_b_g[4], filter_th_b_g[5]);
	}
	#endif

	// 2DNR Filter_lut
	for (i = 0; i < (IQ_NR_TH_LUT / 2); i++) {
		ife_filter_a_r_lut_reg_0.bit.ife_rth_nlm_c0_lut_0 = filter_lut_r[2*i+0];
		ife_filter_a_r_lut_reg_0.bit.ife_rth_nlm_c0_lut_1 = filter_lut_r[2*i+1];
		SETREG(ife_filter_a_r_lut_reg_0.reg, reg_addr + (RANGE_FILTER_REGISTER_32_OFS + (i * 4)));
	}
	ife_filter_a_r_lut_reg_e.bit.ife_rth_nlm_c0_lut_16 = filter_lut_r[16];
	SETREG(ife_filter_a_r_lut_reg_e.reg, reg_addr + RANGE_FILTER_REGISTER_40_OFS);

	for (i = 0; i < (IQ_NR_TH_LUT / 2); i++) {
		ife_filter_a_gr_lut_reg_0.bit.ife_rth_nlm_c1_lut_0 = filter_lut_g[2*i+0];
		ife_filter_a_gr_lut_reg_0.bit.ife_rth_nlm_c1_lut_1 = filter_lut_g[2*i+1];
		SETREG(ife_filter_a_gr_lut_reg_0.reg, reg_addr + (RANGE_FILTER_REGISTER_41_OFS + (i * 4)));
	}
	ife_filter_a_gr_lut_reg_e.bit.ife_rth_nlm_c1_lut_16 = filter_lut_g[16];
	SETREG(ife_filter_a_gr_lut_reg_e.reg, reg_addr + RANGE_FILTER_REGISTER_49_OFS);

	for (i = 0; i < (IQ_NR_TH_LUT / 2); i++) {
		ife_filter_a_gb_lut_reg_0.bit.ife_rth_nlm_c2_lut_0 = filter_lut_g[2*i+0];
		ife_filter_a_gb_lut_reg_0.bit.ife_rth_nlm_c2_lut_1 = filter_lut_g[2*i+1];
		SETREG(ife_filter_a_gb_lut_reg_0.reg, reg_addr + (RANGE_FILTER_REGISTER_50_OFS + (i * 4)));
	}
	ife_filter_a_gb_lut_reg_e.bit.ife_rth_nlm_c2_lut_16 = filter_lut_g[16];
	SETREG(ife_filter_a_gb_lut_reg_e.reg, reg_addr + RANGE_FILTER_REGISTER_58_OFS);

	for (i = 0; i < (IQ_NR_TH_LUT / 2); i++) {
		ife_filter_a_b_lut_reg_0.bit.ife_rth_nlm_c3_lut_0 = filter_lut_b[2*i+0];
		ife_filter_a_b_lut_reg_0.bit.ife_rth_nlm_c3_lut_1 = filter_lut_b[2*i+1];
		SETREG(ife_filter_a_b_lut_reg_0.reg, reg_addr + (RANGE_FILTER_REGISTER_59_OFS + (i * 4)));
	}
	ife_filter_a_b_lut_reg_e.bit.ife_rth_nlm_c3_lut_16 = filter_lut_b[16];
	SETREG(ife_filter_a_b_lut_reg_e.reg, reg_addr + RANGE_FILTER_REGISTER_67_OFS);

	for (i = 0; i < (IQ_NR_TH_LUT / 2); i++) {
		ife_filter_b_r_lut_reg_0.bit.ife_rth_bilat_c0_lut_0 = filter_lut_b_r[2*i+0];
		ife_filter_b_r_lut_reg_0.bit.ife_rth_bilat_c0_lut_1 = filter_lut_b_r[2*i+1];
		SETREG(ife_filter_b_r_lut_reg_0.reg, reg_addr + (RANGE_FILTER_REGISTER_68_OFS + (i * 4)));
	}
	ife_filter_b_r_lut_reg_e.bit.ife_rth_bilat_c0_lut_16 = filter_lut_b_r[16];
	SETREG(ife_filter_b_r_lut_reg_e.reg, reg_addr + RANGE_FILTER_REGISTER_76_OFS);

	for (i = 0; i < (IQ_NR_TH_LUT / 2); i++) {
		ife_filter_b_gr_lut_reg_0.bit.ife_rth_bilat_c1_lut_0 = filter_lut_b_g[2*i+0];
		ife_filter_b_gr_lut_reg_0.bit.ife_rth_bilat_c1_lut_1 = filter_lut_b_g[2*i+1];
		SETREG(ife_filter_b_gr_lut_reg_0.reg, reg_addr + (RANGE_FILTER_REGISTER_77_OFS + (i * 4)));
	}
	ife_filter_b_gr_lut_reg_e.bit.ife_rth_bilat_c1_lut_16 = filter_lut_b_g[16];
	SETREG(ife_filter_b_gr_lut_reg_e.reg, reg_addr + RANGE_FILTER_REGISTER_85_OFS);

	for (i = 0; i < (IQ_NR_TH_LUT / 2); i++) {
		ife_filter_b_gb_lut_reg_0.bit.ife_rth_bilat_c2_lut_0 = filter_lut_b_g[2*i+0];
		ife_filter_b_gb_lut_reg_0.bit.ife_rth_bilat_c2_lut_1 = filter_lut_b_g[2*i+1];
		SETREG(ife_filter_b_gb_lut_reg_0.reg, reg_addr + (RANGE_FILTER_REGISTER_86_OFS + (i * 4)));
	}
	ife_filter_b_gb_lut_reg_e.bit.ife_rth_bilat_c2_lut_16 = filter_lut_b_g[16];
	SETREG(ife_filter_b_gb_lut_reg_e.reg, reg_addr + RANGE_FILTER_REGISTER_94_OFS);

	for (i = 0; i < (IQ_NR_TH_LUT / 2); i++) {
		ife_filter_b_b_lut_reg_0.bit.ife_rth_bilat_c3_lut_0 = filter_lut_b_b[2*i+0];
		ife_filter_b_b_lut_reg_0.bit.ife_rth_bilat_c3_lut_1 = filter_lut_b_b[2*i+1];
		SETREG(ife_filter_b_b_lut_reg_0.reg, reg_addr + (RANGE_FILTER_REGISTER_95_OFS + (i * 4)));
	}
	ife_filter_b_b_lut_reg_e.bit.ife_rth_bilat_c3_lut_16 = filter_lut_b_b[16];
	SETREG(ife_filter_b_b_lut_reg_e.reg, reg_addr + RANGE_FILTER_REGISTER_103_OFS);

	#if IQ_BUILTIN_DBG_MSG
	if (iq_frm_cnt[id] < iq_front_param[id].smooth_thr) {
		DBG_DUMP("id = %d, cnt = %d, rth_nlm_lut = {... , %d, %d, %d, %d, %d, %d}, rth_bilat_lut = {... , %d, %d, %d, %d, %d, %d}\r\n", id, iq_frm_cnt[id],
			filter_lut_g[11], filter_lut_g[12], filter_lut_g[13], filter_lut_g[14], filter_lut_g[15], filter_lut_g[16],
			filter_lut_b_g[11], filter_lut_b_g[12], filter_lut_b_g[13], filter_lut_b_g[14], filter_lut_b_g[15], filter_lut_b_g[16]);
	}
	#endif

	// 2DNR Filter setting
	ife_filter_clamp_reg.reg = GETREG(reg_addr + OUTPUT_FILTER_REGISTER_OFS);
	ife_filter_clamp_reg.bit.ife_clamp_th = (filter_clamp_th * iq_front_factor[id].clamp_th) / 100;
	ife_filter_clamp_reg.bit.ife_clamp_mul = final_nr_tune->filter_clamp_mul;
	SETREG(ife_filter_clamp_reg.reg, reg_addr + OUTPUT_FILTER_REGISTER_OFS);

	ife_filter_blend_w_reg.reg = GETREG(reg_addr + RANGE_FILTER_REGISTER_0_OFS);
	ife_filter_blend_w_reg.bit.ife_bilat_w = final_nr_tune->filter_blend_w;
	SETREG(ife_filter_blend_w_reg.reg, reg_addr + RANGE_FILTER_REGISTER_0_OFS);

	#if IQ_BUILTIN_DBG_MSG
	if (iq_frm_cnt[id] < iq_front_param[id].smooth_thr) {
		DBG_DUMP("id = %d, cnt = %d, blend_w = %d, clamp_th = %d\r\n", id, iq_frm_cnt[id], filter_blend_w_reg.bit.ife_bilat_w, filter_clamp_reg.bit.ife_clamp_th);
	}
	#endif
}

static void iq_builtin_set_nr_lca(UINT32 id, ISP_BUILTIN_IQ_TRIG_MSG msg, UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_IME_FUNCTION_CONTROL_REGISTER0                                          ime_function_control_reg0;

	T_LOCAL_CHROMA_ADAPTATION_REGISTER33                                      lca_edge_th_reg_0;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER34                                      lca_edge_th_reg_1;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER19                                      lca_out_wet;

	T_LOCAL_CHROMA_ADAPTATION_REGISTER1                                       lca_ctr_y_th;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER2                                       lca_ctr_u_th;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER3                                       lca_ctr_v_th;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER5                                       lca_still_smooth_th_y_reg_0;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER6                                       lca_still_smooth_th_y_reg_1;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER7                                       lca_still_smooth_th_c_reg_0;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER8                                       lca_still_smooth_th_c_reg_1;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER9                                       lca_still_smooth_th_c_reg_2;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER10                                      lca_still_edge_th_y_reg_0;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER11                                      lca_still_edge_th_y_reg_1;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER12                                      lca_still_edge_th_c_reg_0;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER13                                      lca_still_edge_th_c_reg_1;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER14                                      lca_still_edge_th_c_reg_2;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER15                                      lca_motion_th_y_reg_0;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER16                                      lca_motion_th_y_reg_1;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER17                                      lca_motion_th_c_reg_0;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER18                                      lca_motion_th_c_reg_1;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER19                                      lca_motion_th_c_reg_2;

	T_LOCAL_CHROMA_ADAPTATION_REGISTER20                                      lca_coring_gain_still_y;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER21                                      lca_coring_gain_motion_y;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER22                                      lca_coring_gain_still_u;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER23                                      lca_coring_gain_motion_u;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER24                                      lca_coring_gain_still_v;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER25                                      lca_coring_gain_motion_v;

	T_LOCAL_CHROMA_ADAPTATION_REGISTER26                                      lca_cutoff_still_y;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER27                                      lca_cutoff_motion_y;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER28                                      lca_cutoff_still_u;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER29                                      lca_cutoff_motion_u;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER30                                      lca_cutoff_still_v;
	T_LOCAL_CHROMA_ADAPTATION_REGISTER31                                      lca_cutoff_motion_v;

	T_IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_REGISTER0                 dbcs_reg;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif 
	IQ_NR_TUNE_PARAM            *final_nr_tune;

	ISP_BUILTIN_FUNC            func_en = 0;
	UINT32                      iso_idx = 0;
	int                         i;

	reg_addr = ime_reg_addr;

	func_en = isp_builtin_get_func_en(id) & ISP_BUILTIN_FUNC_YUV_SUBOUT;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->nr->mode == IQ_OP_TYPE_AUTO) {
		final_nr_tune = &iq_param->nr->auto_param[iso_idx];
	} else {
		final_nr_tune = &iq_param->nr->manual_param;
	}

	// DBCS
	ime_function_control_reg0.reg = GETREG(reg_addr + IME_FUNCTION_CONTROL_REGISTER0_OFS);
	ime_function_control_reg0.bit.ime_dbcs_en = iq_param->nr->dbcs_enable;
	SETREG(ime_function_control_reg0.reg, reg_addr + IME_FUNCTION_CONTROL_REGISTER0_OFS);

	dbcs_reg.reg = GETREG(reg_addr + IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_REGISTER0_OFS);
	dbcs_reg.bit.dbcs_step_y = final_nr_tune->dbcs_step_y;
	dbcs_reg.bit.dbcs_step_uv = final_nr_tune->dbcs_step_c;
	SETREG(dbcs_reg.reg, reg_addr + IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_REGISTER0_OFS);

	// LCA
	if ((func_en == FALSE) || (iq_param->nr->lca_enable == FALSE)) {
		lca_out_wet.reg = GETREG(reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER19_OFS);
		lca_out_wet.bit.lca_out_wet_y = 0;
		lca_out_wet.bit.lca_out_wet_uv = 0;
		SETREG(lca_out_wet.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER19_OFS);

		return;
	} else {
		lca_out_wet.reg = GETREG(reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER19_OFS);
		lca_out_wet.bit.lca_out_wet_y = 32;
		lca_out_wet.bit.lca_out_wet_uv = 32;
		SETREG(lca_out_wet.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER19_OFS);
	}

	lca_edge_th_reg_0.bit.lca_edge_th0 = final_nr_tune->lca_edge_th;
	SETREG(lca_edge_th_reg_0.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER33_OFS);

	lca_edge_th_reg_1.bit.lca_edge_th1 = final_nr_tune->lca_edge_th;
	SETREG(lca_edge_th_reg_1.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER34_OFS);

	// LCA filter
	lca_ctr_y_th.bit.lca_ctr_y_th0 = final_nr_tune->lca_y_filter_level[0];
	lca_ctr_y_th.bit.lca_ctr_y_th1 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[0] * 1414 / 1000, 0, 255);
	lca_ctr_y_th.bit.lca_ctr_y_th2 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[0] * 1732 / 1000, 0, 255);
	SETREG(lca_ctr_y_th.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER1_OFS);

	lca_ctr_u_th.bit.lca_ctr_u_th0 = final_nr_tune->lca_c_filter_level[0];
	lca_ctr_u_th.bit.lca_ctr_u_th1 = IQ_CLAMP(final_nr_tune->lca_c_filter_level[0] * 1414 / 1000, 0, 255);
	lca_ctr_u_th.bit.lca_ctr_u_th2 = IQ_CLAMP(final_nr_tune->lca_c_filter_level[0] * 1732 / 1000, 0, 255);
	SETREG(lca_ctr_u_th.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER2_OFS);

	lca_ctr_v_th.bit.lca_ctr_v_th0 = lca_ctr_u_th.bit.lca_ctr_u_th0;
	lca_ctr_v_th.bit.lca_ctr_v_th1 = lca_ctr_u_th.bit.lca_ctr_u_th1;
	lca_ctr_v_th.bit.lca_ctr_v_th2 = lca_ctr_u_th.bit.lca_ctr_u_th2;
	SETREG(lca_ctr_v_th.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER3_OFS);

	lca_still_smooth_th_y_reg_0.bit.lca_still_smooth_y_th0 = final_nr_tune->lca_y_filter_level[1];
	lca_still_smooth_th_y_reg_0.bit.lca_still_smooth_y_th1 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[1] * 1414 / 1000, 0, 255);
	lca_still_smooth_th_y_reg_0.bit.lca_still_smooth_y_th2 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[1] * 1732 / 1000, 0, 255);
	lca_still_smooth_th_y_reg_0.bit.lca_still_smooth_y_th3 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[1] * 2000 / 1000, 0, 255);
	SETREG(lca_still_smooth_th_y_reg_0.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER5_OFS);

	lca_still_smooth_th_y_reg_1.bit.lca_still_smooth_y_th4 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[1] * 2236 / 1000, 0, 255);
	SETREG(lca_still_smooth_th_y_reg_1.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER6_OFS);

	lca_still_smooth_th_c_reg_0.bit.lca_still_smooth_uv_th0 = IQ_CLAMP(final_nr_tune->lca_c_filter_level[1] * final_nr_tune->lca_c_filter_level[1], 0, 65535);
	lca_still_smooth_th_c_reg_0.bit.lca_still_smooth_uv_th1 = IQ_CLAMP(lca_still_smooth_th_c_reg_0.bit.lca_still_smooth_uv_th0 * 1414 / 1000, 0, 65535);
	SETREG(lca_still_smooth_th_c_reg_0.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER7_OFS);

	lca_still_smooth_th_c_reg_1.bit.lca_still_smooth_uv_th2 = IQ_CLAMP(lca_still_smooth_th_c_reg_0.bit.lca_still_smooth_uv_th0 * 1732 / 1000, 0, 65535);
	lca_still_smooth_th_c_reg_1.bit.lca_still_smooth_uv_th3 = IQ_CLAMP(lca_still_smooth_th_c_reg_0.bit.lca_still_smooth_uv_th0 * 2000 / 1000, 0, 65535);
	SETREG(lca_still_smooth_th_c_reg_1.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER8_OFS);

	lca_still_smooth_th_c_reg_2.bit.lca_still_smooth_uv_th4 = IQ_CLAMP(lca_still_smooth_th_c_reg_0.bit.lca_still_smooth_uv_th0 * 2236 / 1000, 0, 65535);
	SETREG(lca_still_smooth_th_c_reg_2.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER9_OFS);

	lca_still_edge_th_y_reg_0.bit.lca_still_edge_y_th0 = final_nr_tune->lca_y_filter_level[0];
	lca_still_edge_th_y_reg_0.bit.lca_still_edge_y_th1 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[0] * 1414 / 1000, 0, 255);
	lca_still_edge_th_y_reg_0.bit.lca_still_edge_y_th2 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[0] * 1732 / 1000, 0, 255);
	lca_still_edge_th_y_reg_0.bit.lca_still_edge_y_th3 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[0] * 2000 / 1000, 0, 255);
	SETREG(lca_still_edge_th_y_reg_0.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER10_OFS);

	lca_still_edge_th_y_reg_1.bit.lca_still_edge_y_th4 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[0] * 2236 / 1000, 0, 255);
	SETREG(lca_still_edge_th_y_reg_1.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER11_OFS);

	lca_still_edge_th_c_reg_0.bit.lca_still_edge_uv_th0 = IQ_CLAMP(final_nr_tune->lca_c_filter_level[0] * final_nr_tune->lca_c_filter_level[0], 0, 65535);
	lca_still_edge_th_c_reg_0.bit.lca_still_edge_uv_th1 = IQ_CLAMP(lca_still_edge_th_c_reg_0.bit.lca_still_edge_uv_th0 * 1414 / 1000, 0, 65535);
	SETREG(lca_still_edge_th_c_reg_0.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER12_OFS);

	lca_still_edge_th_c_reg_1.bit.lca_still_edge_uv_th2 = IQ_CLAMP(lca_still_edge_th_c_reg_0.bit.lca_still_edge_uv_th0 * 1732 / 1000, 0, 65535);
	lca_still_edge_th_c_reg_1.bit.lca_still_edge_uv_th3 = IQ_CLAMP(lca_still_edge_th_c_reg_0.bit.lca_still_edge_uv_th0 * 2000 / 1000, 0, 65535);
	SETREG(lca_still_edge_th_c_reg_1.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER13_OFS);

	lca_still_edge_th_c_reg_2.bit.lca_still_edge_uv_th4 = IQ_CLAMP(lca_still_edge_th_c_reg_0.bit.lca_still_edge_uv_th0 * 2236 / 1000, 0, 65535);
	SETREG(lca_still_edge_th_c_reg_2.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER14_OFS);

	lca_motion_th_y_reg_0.bit.lca_motion_y_th0 = final_nr_tune->lca_y_filter_level[2];
	lca_motion_th_y_reg_0.bit.lca_motion_y_th1 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[2] * 1414 / 1000, 0, 255);
	lca_motion_th_y_reg_0.bit.lca_motion_y_th2 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[2] * 1732 / 1000, 0, 255);
	lca_motion_th_y_reg_0.bit.lca_motion_y_th3 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[2] * 2000 / 1000, 0, 255);
	SETREG(lca_motion_th_y_reg_0.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER15_OFS);

	lca_motion_th_y_reg_1.bit.lca_motion_y_th4 = IQ_CLAMP(final_nr_tune->lca_y_filter_level[2] * 2236 / 1000, 0, 255);
	SETREG(lca_motion_th_y_reg_1.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER16_OFS);

	lca_motion_th_c_reg_0.bit.lca_motion_uv_th0 = IQ_CLAMP(final_nr_tune->lca_c_filter_level[2] * final_nr_tune->lca_c_filter_level[2], 0, 65535);
	lca_motion_th_c_reg_0.bit.lca_motion_uv_th1 = IQ_CLAMP(lca_motion_th_c_reg_0.bit.lca_motion_uv_th0 * 1414 / 1000, 0, 65535);
	SETREG(lca_motion_th_c_reg_0.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER17_OFS);

	lca_motion_th_c_reg_1.bit.lca_motion_uv_th2 = IQ_CLAMP(lca_motion_th_c_reg_0.bit.lca_motion_uv_th0 * 1732 / 1000, 0, 65535);
	lca_motion_th_c_reg_1.bit.lca_motion_uv_th3 = IQ_CLAMP(lca_motion_th_c_reg_0.bit.lca_motion_uv_th0 * 2000 / 1000, 0, 65535);
	SETREG(lca_motion_th_c_reg_1.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER18_OFS);

	lca_motion_th_c_reg_2.reg = GETREG(reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER19_OFS);
	lca_motion_th_c_reg_2.bit.lca_motion_uv_th4 = IQ_CLAMP(lca_motion_th_c_reg_0.bit.lca_motion_uv_th0 * 2236 / 1000, 0, 65535);
	SETREG(lca_motion_th_c_reg_2.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER19_OFS);

	// LCA coring gain
	lca_coring_gain_still_y.bit.lca_still_y_gain0 = IQ_CLAMP(32 - final_nr_tune->lca_y_coring_gain[1], 0, 32);
	lca_coring_gain_still_y.bit.lca_still_y_gain1 = IQ_CLAMP(32 - final_nr_tune->lca_y_coring_gain[0], 0, 32);
	lca_coring_gain_still_y.bit.lca_still_y_gain2 = IQ_CLAMP(32 - final_nr_tune->lca_y_coring_gain[0], 0, 32);
	SETREG(lca_coring_gain_still_y.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER20_OFS);

	lca_coring_gain_motion_y.bit.lca_motion_y_gain0 = IQ_CLAMP(32 - final_nr_tune->lca_y_coring_gain[2], 0, 32);
	lca_coring_gain_motion_y.bit.lca_motion_y_gain1 = IQ_CLAMP(32 - final_nr_tune->lca_y_coring_gain[2], 0, 32);
	lca_coring_gain_motion_y.bit.lca_motion_y_gain2 = IQ_CLAMP(32 - final_nr_tune->lca_y_coring_gain[2], 0, 32);
	SETREG(lca_coring_gain_motion_y.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER21_OFS);

	lca_coring_gain_still_u.bit.lca_still_u_gain0 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[1], 0, 32);
	lca_coring_gain_still_u.bit.lca_still_u_gain1 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[0], 0, 32);
	lca_coring_gain_still_u.bit.lca_still_u_gain2 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[0], 0, 32);
	SETREG(lca_coring_gain_still_u.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER22_OFS);

	lca_coring_gain_motion_u.bit.lca_motion_u_gain0 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[2], 0, 32);
	lca_coring_gain_motion_u.bit.lca_motion_u_gain1 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[2], 0, 32);
	lca_coring_gain_motion_u.bit.lca_motion_u_gain2 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[2], 0, 32);
	SETREG(lca_coring_gain_motion_u.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER23_OFS);

	lca_coring_gain_still_v.bit.lca_still_v_gain0 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[1], 0, 32);
	lca_coring_gain_still_v.bit.lca_still_v_gain1 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[0], 0, 32);
	lca_coring_gain_still_v.bit.lca_still_v_gain2 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[0], 0, 32);
	SETREG(lca_coring_gain_still_v.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER24_OFS);

	lca_coring_gain_motion_v.bit.lca_motion_v_gain0 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[2], 0, 32);
	lca_coring_gain_motion_v.bit.lca_motion_v_gain1 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[2], 0, 32);
	lca_coring_gain_motion_v.bit.lca_motion_v_gain2 = IQ_CLAMP(32 - final_nr_tune->lca_c_coring_gain[2], 0, 32);
	SETREG(lca_coring_gain_motion_v.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER25_OFS);

	// LCA coring cutoff
	lca_cutoff_still_y.bit.lca_still_y_coff0 = final_nr_tune->lca_y_coring_cutoff[1];
	lca_cutoff_still_y.bit.lca_still_y_coff1 = final_nr_tune->lca_y_coring_cutoff[0];
	lca_cutoff_still_y.bit.lca_still_y_coff2 = final_nr_tune->lca_y_coring_cutoff[0];
	SETREG(lca_cutoff_still_y.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER26_OFS);

	lca_cutoff_motion_y.bit.lca_motion_y_coff0 = final_nr_tune->lca_y_coring_cutoff[2];
	lca_cutoff_motion_y.bit.lca_motion_y_coff1 = final_nr_tune->lca_y_coring_cutoff[2];
	lca_cutoff_motion_y.bit.lca_motion_y_coff2 = final_nr_tune->lca_y_coring_cutoff[2];
	SETREG(lca_cutoff_motion_y.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER27_OFS);

	lca_cutoff_still_u.bit.lca_still_u_coff0 = final_nr_tune->lca_c_coring_cutoff[1];
	lca_cutoff_still_u.bit.lca_still_u_coff1 = final_nr_tune->lca_c_coring_cutoff[0];
	lca_cutoff_still_u.bit.lca_still_u_coff2 = final_nr_tune->lca_c_coring_cutoff[0];
	SETREG(lca_cutoff_still_u.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER28_OFS);

	lca_cutoff_motion_u.bit.lca_motion_u_coff0 = final_nr_tune->lca_c_coring_cutoff[2];
	lca_cutoff_motion_u.bit.lca_motion_u_coff1 = final_nr_tune->lca_c_coring_cutoff[2];
	lca_cutoff_motion_u.bit.lca_motion_u_coff2 = final_nr_tune->lca_c_coring_cutoff[2];
	SETREG(lca_cutoff_motion_u.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER29_OFS);

	lca_cutoff_still_v.bit.lca_still_v_coff0 = final_nr_tune->lca_c_coring_cutoff[1];
	lca_cutoff_still_v.bit.lca_still_v_coff1 = final_nr_tune->lca_c_coring_cutoff[0];
	lca_cutoff_still_v.bit.lca_still_v_coff2 = final_nr_tune->lca_c_coring_cutoff[0];
	SETREG(lca_cutoff_still_v.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER30_OFS);

	lca_cutoff_motion_v.bit.lca_motion_v_coff0 = final_nr_tune->lca_c_coring_cutoff[2];
	lca_cutoff_motion_v.bit.lca_motion_v_coff1 = final_nr_tune->lca_c_coring_cutoff[2];
	lca_cutoff_motion_v.bit.lca_motion_v_coff2 = final_nr_tune->lca_c_coring_cutoff[2];
	SETREG(lca_cutoff_motion_v.reg, reg_addr + LOCAL_CHROMA_ADAPTATION_REGISTER31_OFS);
}

static void iq_builtin_set_vig(UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_CONTROL_REGISTER                       ife_en_reg;
	T_SOURCE_SIZE_REGISTER_0                 ife_input_reg;

	T_VIGNETTE_SETTING_REGISTER_0            vig_r_center;
	T_VIGNETTE_SETTING_REGISTER_1            vig_gr_center;
	T_VIGNETTE_SETTING_REGISTER_2            vig_gb_center;
	T_VIGNETTE_SETTING_REGISTER_3            vig_b_center;
	T_VIGNETTE_SETTING_REGISTER_4            vig_gain_reg;

	T_VIGNETTE_REGISTER_0                    vig_r_gain_reg_0;
	T_VIGNETTE_REGISTER_8                    vig_r_gain_reg_8;

	T_VIGNETTE_REGISTER_9                    vig_gr_gain_reg_0;
	T_VIGNETTE_REGISTER_17                   vig_gr_gain_reg_8;

	T_VIGNETTE_REGISTER_18                   vig_gb_gain_reg_0;
	T_VIGNETTE_REGISTER_26                   vig_gb_gain_reg_8;

	T_VIGNETTE_REGISTER_27                   vig_b_gain_reg_0;
	T_VIGNETTE_REGISTER_35                   vig_b_gain_reg_8;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif 
	IQ_SHADING_PARAM            *shading;

	UINT16                      vig_zero[IQ_SHADING_VIG_LEN] = {0};
	UINT32                      curr_tab_gain;
	UINT16                      curr_shading_vig[IQ_SHADING_VIG_LEN] = {0};
	UINT32                      real_gain = 0, max_gain = 0;
	INT32                       i;

	reg_addr = ife_reg_addr;
	shading = iq_param->shading;

	iq_builtin_intpl_tbl_uint16(shading->vig_lut, vig_zero, IQ_SHADING_VIG_LEN, curr_shading_vig, gain, shading->vig_reduce_th, shading->vig_zero_th);

	for (i = 0; i < IQ_SHADING_VIG_LEN; i++) {
		if (max_gain < curr_shading_vig[i]) {
			max_gain = curr_shading_vig[i];
		}
	}

	real_gain = 1 + (max_gain >> 10);
	if (real_gain < 2) {
		curr_tab_gain = 0;
	} else if (real_gain < 3) {
		curr_tab_gain = 1;
	} else if (real_gain < 5) {
		curr_tab_gain = 2;
	} else if (real_gain < 9) {
		curr_tab_gain = 3;
	} else {
		DBG_DUMP("Wrong VIG gain value, force to 0 \r\n");
		curr_tab_gain = 0;
	}

	for (i = 0; i < IQ_SHADING_VIG_LEN; i++) {
		curr_shading_vig[i] = curr_shading_vig[i] >> curr_tab_gain;
	}

	ife_en_reg.reg = GETREG(reg_addr + CONTROL_REGISTER_OFS);
	ife_en_reg.bit.vig_en = shading->vig_enable;
	SETREG(ife_en_reg.reg, reg_addr + CONTROL_REGISTER_OFS);

	ife_input_reg.reg = GETREG(reg_addr + SOURCE_SIZE_REGISTER_0_OFS);

	//vig_center
	vig_r_center.bit.ife_distvgtx_c0 = (ife_input_reg.bit.width << 2) * shading->vig_center_x / 1000;
	vig_r_center.bit.ife_distvgty_c0 = (ife_input_reg.bit.height << 1) * shading->vig_center_y / 1000;
	SETREG(vig_r_center.reg, reg_addr + VIGNETTE_SETTING_REGISTER_0_OFS);

	vig_gr_center.bit.ife_distvgtx_c1 = (ife_input_reg.bit.width << 2) * shading->vig_center_x / 1000;
	vig_gr_center.bit.ife_distvgty_c1 = (ife_input_reg.bit.height << 1) * shading->vig_center_y / 1000;
	SETREG(vig_gr_center.reg, reg_addr + VIGNETTE_SETTING_REGISTER_1_OFS);

	vig_gb_center.bit.ife_distvgtx_c2 = (ife_input_reg.bit.width << 2) * shading->vig_center_x / 1000;
	vig_gb_center.bit.ife_distvgty_c2 = (ife_input_reg.bit.height << 1) * shading->vig_center_y / 1000;
	SETREG(vig_gb_center.reg, reg_addr + VIGNETTE_SETTING_REGISTER_2_OFS);

	vig_b_center.bit.ife_distvgtx_c3 = (ife_input_reg.bit.width << 2) * shading->vig_center_x / 1000;
	vig_b_center.bit.ife_distvgty_c3 = (ife_input_reg.bit.height << 1) * shading->vig_center_y / 1000;
	SETREG(vig_b_center.reg, reg_addr + VIGNETTE_SETTING_REGISTER_3_OFS);

	vig_gain_reg.reg = GETREG(reg_addr + VIGNETTE_SETTING_REGISTER_4_OFS);
	vig_gain_reg.bit.ife_distgain = curr_tab_gain;
	SETREG(vig_gain_reg.reg, reg_addr + VIGNETTE_SETTING_REGISTER_4_OFS);

	//vig_r_gain
	for (i = 0; i < (IQ_SHADING_VIG_LEN / 2); i++) {
		vig_r_gain_reg_0.bit.ife_vig_c0_lut_0 = curr_shading_vig[2*i+0];
		vig_r_gain_reg_0.bit.ife_vig_c0_lut_1 = curr_shading_vig[2*i+1];
		SETREG(vig_r_gain_reg_0.reg, reg_addr + (VIGNETTE_REGISTER_0_OFS + (i * 4)));
	}
	vig_r_gain_reg_8.bit.ife_vig_c0_lut_16 = curr_shading_vig[16];
	SETREG(vig_r_gain_reg_8.reg, reg_addr + VIGNETTE_REGISTER_8_OFS);

	//vig_gr_gain
	for (i = 0; i < (IQ_SHADING_VIG_LEN / 2); i++) {
		vig_gr_gain_reg_0.bit.ife_vig_c1_lut_0 = curr_shading_vig[2*i+0];
		vig_gr_gain_reg_0.bit.ife_vig_c1_lut_1 = curr_shading_vig[2*i+1];
		SETREG(vig_gr_gain_reg_0.reg, reg_addr + (VIGNETTE_REGISTER_9_OFS + (i * 4)));
	}
	vig_gr_gain_reg_8.bit.ife_vig_c1_lut_16 = curr_shading_vig[16];
	SETREG(vig_gr_gain_reg_8.reg, reg_addr + VIGNETTE_REGISTER_17_OFS);

	//vig_gb_gain
	for (i = 0; i < (IQ_SHADING_VIG_LEN / 2); i++) {
		vig_gb_gain_reg_0.bit.ife_vig_c2_lut_0 = curr_shading_vig[2*i+0];
		vig_gb_gain_reg_0.bit.ife_vig_c2_lut_1 = curr_shading_vig[2*i+1];
		SETREG(vig_gb_gain_reg_0.reg, reg_addr + (VIGNETTE_REGISTER_18_OFS + (i * 4)));
	}
	vig_gb_gain_reg_8.bit.ife_vig_c2_lut_16 = curr_shading_vig[16];
	SETREG(vig_gb_gain_reg_8.reg, reg_addr + VIGNETTE_REGISTER_26_OFS);

	//vig_b_gain
	for (i = 0; i < (IQ_SHADING_VIG_LEN / 2); i++) {
		vig_b_gain_reg_0.bit.ife_vig_c3_lut_0 = curr_shading_vig[2*i+0];
		vig_b_gain_reg_0.bit.ife_vig_c3_lut_1 = curr_shading_vig[2*i+1];
		SETREG(vig_b_gain_reg_0.reg, reg_addr + (VIGNETTE_REGISTER_27_OFS + (i * 4)));
	}
	vig_b_gain_reg_8.bit.ife_vig_c3_lut_16 = curr_shading_vig[16];
	SETREG(vig_b_gain_reg_8.reg, reg_addr + VIGNETTE_REGISTER_35_OFS);
}

static void iq_builtin_set_tonecurve(UINT32 id, UINT32 lv, IQ_PARAM_PTR *iq_param, UINT32 ui_shdr_tone_lv)
{
	T_WDR_CONTROL_REGISTER0                            dce_func_reg;
	T_IFE_WDR_YV_BLD_REGISTER_0                        yv_blend_lut_reg_0;
	T_IFE_WDR_YV_BLD_REGISTER_2                        yv_blend_lut_reg_2;

	T_IFE_WDR_TONE_CURVE_LEFT_REGISTER_0               tone_curve_l_reg0;
	T_IFE_WDR_TONE_CURVE_LEFT_REGISTER_32              tone_curve_l_reg32;
	T_IFE_WDR_TONE_CURVE_RIGHT_REGISTER_0              tone_curve_r_reg0;
	T_IFE_WDR_TONE_CURVE_RIGHT_REGISTER_8              tone_curve_r_reg8;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif 
	IQ_TONE_PARAM               *tone;
	IQ_TONE_AUTO_PARAM          *tone_l, *tone_h;

	UINT32                      tone_idx_l = 0, tone_idx_h = 0;
	UINT32                      tone_start = 0, tone_end = 0;
	UINT32                      curr_tone_level;
	UINT16                      iq_tone_curve_l[IQ_TONE_LEFT_NUM];
	UINT16                      iq_tone_curve_r[IQ_TONE_RIGHT_NUM];
	INT32                       i = 0;

	reg_addr = ife_reg_addr;

	tone = iq_param->tone;

	if (tone->mode == IQ_OP_TYPE_AUTO) {
		if (tone->auto_sel == IQ_TONE_AUTO_BY_AE_LV) {
			if (lv <= (tone->auto_param[IQ_TONE_SET4].lv * LV_BASE)) {
				tone_idx_l = IQ_TONE_SET4;
				tone_idx_h = IQ_TONE_SET4;
				tone_start = tone->auto_param[IQ_TONE_SET4].lv * 100;
				tone_end = tone->auto_param[IQ_TONE_SET4].lv * 100;
			}
			for (i = 1; i < IQ_TONE_ID_MAX_NUM; i++) {
				if (lv > (tone->auto_param[i].lv * LV_BASE)) {
					tone_idx_l = i;
					tone_idx_h = i - 1;
					tone_start = tone->auto_param[i].lv * 100;
					tone_end = tone->auto_param[i - 1].lv * 100;
					break;
				}
			}

			tone_l = &tone->auto_param[tone_idx_l];
			tone_h = &tone->auto_param[tone_idx_h];

			curr_tone_level = iq_builtin_intpl(lv / (LV_BASE / 100), tone_l->tone_level, tone_h->tone_level, tone_start, tone_end);
			/*DBG_DUMP("curr_tone_level = %d, lv = %d, idx_l = %d (lv:%d, level:%d), idx_h = %d (lv:%d, level:%d) \r\n"
				, curr_tone_level, lv
				, tone_idx_l, tone_l->lv * 100, tone_l->tone_level
				, tone_idx_h, tone_h->lv * 100, tone_h->tone_level);*/
		} else {
			curr_tone_level = ui_shdr_tone_lv;
			//DBG_DUMP("curr_tone_level = %d \r\n", curr_tone_level);
		}

		if (curr_tone_level <= tone->auto_set0_level) {
			memcpy(iq_tone_curve_l, tone->auto_set0_lut_left, sizeof(UINT16) * IQ_TONE_LEFT_NUM);
			memcpy(iq_tone_curve_r, tone->auto_set0_lut_right, sizeof(UINT16) * IQ_TONE_RIGHT_NUM);
		} else if (curr_tone_level <= tone->auto_set1_level) {
			iq_builtin_intpl_tbl_uint16(tone->auto_set0_lut_left, tone->auto_set1_lut_left, IQ_TONE_LEFT_NUM, iq_tone_curve_l, curr_tone_level, tone->auto_set0_level, tone->auto_set1_level);
			iq_builtin_intpl_tbl_uint16(tone->auto_set0_lut_right, tone->auto_set1_lut_right, IQ_TONE_RIGHT_NUM, iq_tone_curve_r, curr_tone_level, tone->auto_set0_level, tone->auto_set1_level);
		} else if (curr_tone_level <= tone->auto_set2_level) {
			iq_builtin_intpl_tbl_uint16(tone->auto_set1_lut_left, tone->auto_set2_lut_left, IQ_TONE_LEFT_NUM, iq_tone_curve_l, curr_tone_level, tone->auto_set1_level, tone->auto_set2_level);
			iq_builtin_intpl_tbl_uint16(tone->auto_set1_lut_right, tone->auto_set2_lut_right, IQ_TONE_RIGHT_NUM, iq_tone_curve_r, curr_tone_level, tone->auto_set1_level, tone->auto_set2_level);
		} else if (curr_tone_level <= tone->auto_set3_level) {
			iq_builtin_intpl_tbl_uint16(tone->auto_set2_lut_left, tone->auto_set3_lut_left, IQ_TONE_LEFT_NUM, iq_tone_curve_l, curr_tone_level, tone->auto_set2_level, tone->auto_set3_level);
			iq_builtin_intpl_tbl_uint16(tone->auto_set2_lut_right, tone->auto_set3_lut_right, IQ_TONE_RIGHT_NUM, iq_tone_curve_r, curr_tone_level, tone->auto_set2_level, tone->auto_set3_level);
		} else if (curr_tone_level <= tone->auto_set4_level) {
			iq_builtin_intpl_tbl_uint16(tone->auto_set3_lut_left, tone->auto_set4_lut_left, IQ_TONE_LEFT_NUM, iq_tone_curve_l, curr_tone_level, tone->auto_set3_level, tone->auto_set4_level);
			iq_builtin_intpl_tbl_uint16(tone->auto_set3_lut_right, tone->auto_set4_lut_right, IQ_TONE_RIGHT_NUM, iq_tone_curve_r, curr_tone_level, tone->auto_set3_level, tone->auto_set4_level);
		} else {
			memcpy(iq_tone_curve_l, tone->auto_set4_lut_left, sizeof(UINT16) * IQ_TONE_LEFT_NUM);
			memcpy(iq_tone_curve_r, tone->auto_set4_lut_right, sizeof(UINT16) * IQ_TONE_RIGHT_NUM);
		}
	}else {
		memcpy(iq_tone_curve_l, tone->manual_lut_left, sizeof(UINT16) * IQ_TONE_LEFT_NUM);
		memcpy(iq_tone_curve_r, tone->manual_lut_right, sizeof(UINT16) * IQ_TONE_RIGHT_NUM);
	}

	dce_func_reg.reg = GETREG(reg_addr + WDR_CONTROL_REGISTER0_OFS);
	dce_func_reg.bit.ife_wdr_tcurve_en = tone->enable;
	SETREG(dce_func_reg.reg, reg_addr + WDR_CONTROL_REGISTER0_OFS);

	for (i = 0; i < (IQ_TONE_INPUT_YV_BLD_NUM / 4); i++) {
		yv_blend_lut_reg_0.bit.ife_wdr_input_yv_bld_lut0 = tone->tone_in_yv_blend_lut[4*i+0];
		yv_blend_lut_reg_0.bit.ife_wdr_input_yv_bld_lut1 = tone->tone_in_yv_blend_lut[4*i+1];
		yv_blend_lut_reg_0.bit.ife_wdr_input_yv_bld_lut2 = tone->tone_in_yv_blend_lut[4*i+2];
		yv_blend_lut_reg_0.bit.ife_wdr_input_yv_bld_lut3 = tone->tone_in_yv_blend_lut[4*i+3];
		SETREG(yv_blend_lut_reg_0.reg, reg_addr + (IFE_WDR_YV_BLD_REGISTER_0_OFS + (i * 4)));
	}

	yv_blend_lut_reg_2.bit.ife_wdr_input_yv_bld_lut8 = tone->tone_in_yv_blend_lut[8];
	SETREG(yv_blend_lut_reg_2.reg, reg_addr + IFE_WDR_YV_BLD_REGISTER_2_OFS);

	if (tone->enable) {
		for (i = 0; i < (IQ_TONE_LEFT_NUM / 2); i++) {
			tone_curve_l_reg0.bit.ife_wdr_tcurve_val_lut_l0 = iq_tone_curve_l[2*i+0];
			tone_curve_l_reg0.bit.ife_wdr_tcurve_val_lut_l1 = iq_tone_curve_l[2*i+1];
			SETREG(tone_curve_l_reg0.reg, reg_addr + (IFE_WDR_TONE_CURVE_LEFT_REGISTER_0_OFS + (i * 4)));
		}

		tone_curve_l_reg32.bit.ife_wdr_tcurve_val_lut_l64 = iq_tone_curve_l[64];
		SETREG(tone_curve_l_reg32.reg, reg_addr + IFE_WDR_TONE_CURVE_LEFT_REGISTER_32_OFS);

		for (i = 0; i < (IQ_TONE_RIGHT_NUM / 2); i++) {
			tone_curve_r_reg0.bit.ife_wdr_tcurve_val_lut_r0 = iq_tone_curve_r[2*i+0];
			tone_curve_r_reg0.bit.ife_wdr_tcurve_val_lut_r1 = iq_tone_curve_r[2*i+1];
			SETREG(tone_curve_r_reg0.reg, reg_addr + (IFE_WDR_TONE_CURVE_RIGHT_REGISTER_0_OFS + (i * 4)));
		}

		tone_curve_r_reg8.bit.ife_wdr_tcurve_val_lut_r16 = iq_tone_curve_r[16];
		SETREG(tone_curve_r_reg8.reg, reg_addr + IFE_WDR_TONE_CURVE_RIGHT_REGISTER_8_OFS);
	}
}

#define IQ_WDR_SMOOTH_FACTOR 1
static void iq_builtin_set_wdr(UINT32 id, ISP_BUILTIN_IQ_TRIG_MSG msg, UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_WDR_CONTROL_REGISTER0                                wdr_en_reg;
	T_IFE_WDR_SUBIMAGE_REGISTER_0                          wdr_subimg_size;
	T_IFE_WDR_SUBIMAGE_REGISTER_2                          wdr_subimg_ofsi;
	T_IFE_WDR_SUBIMAGE_REGISTER_4                          wdr_subimg_ofso;
	T_IFE_WDR_OUPUT_BLENDING_REGISTER                      wdr_str_reg;
	T_IFE_WDR_CONTROL_REGISTER1                            wdr_gain_reg;
	T_IFE_WDR_TONE_MAPPING_REGISTER_0                      wdr_halo_reg;
	T_IFE_WDR_FBC_REGISTER_0                               wdr_fbc_reg;
	T_IFE_WDR_GAIN_STRENGTH_REGISTER                       wdr_gain_strength_reg;
	T_IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_0        wdr_out_bld_l_reg_0;
	T_IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_32       wdr_out_bld_l_reg_e;
	T_IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_0       wdr_out_bld_r_reg_0;
	T_IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_8       wdr_out_bld_r_reg_e;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif 
	IQ_WDR_PARAM                *wdr;
	IQ_WDR_AUTO_PARAM           *wdr_auto;

	ISP_BUILTIN_FUNC            func_en = 0;
	UINT32                      iso_idx = 0;
	UINT32                      wdr_target_level = 0;
	UINT32                      wdr_lib_level;
	INT32                       i;

	reg_addr = ife_reg_addr;

	func_en = isp_builtin_get_func_en(id) & ISP_BUILTIN_FUNC_WDR;
	if (func_en == FALSE) {
		if ((msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) && (func_en == 0) && (iq_param->wdr->enable == TRUE)) {
			DBG_DUMP("[builtin IQ] wdr_enable = 0 in nvt-fastboot-ipp.dtsi, please check !! \r\n");
		}
		wdr_en_reg.reg = GETREG(reg_addr + WDR_CONTROL_REGISTER0_OFS);
		wdr_en_reg.bit.ife_wdr_en = FALSE;
		SETREG(wdr_en_reg.reg, reg_addr + WDR_CONTROL_REGISTER0_OFS);

		return;
	}

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	wdr = iq_param->wdr;
	wdr_auto = &iq_param->wdr->auto_param[iso_idx];

	if (wdr->mode == IQ_OP_TYPE_AUTO) {
		wdr_lib_level = iq_lib_wdr_nvt(id);
		wdr_lib_level = IQ_CLAMP(wdr_lib_level, 0, 240);
		wdr_target_level = (wdr_auto->level * wdr_lib_level) >> 7;
		wdr_target_level = IQ_CLAMP(wdr_target_level, wdr_auto->strength_min, wdr_auto->strength_max);
	} else {
		wdr_target_level = wdr->manual_param.strength;
	}

	//prevent dramatic change
	if (final_wdr_strength[id] <= wdr_target_level) {
		final_wdr_strength[id] = IQ_CLAMP((final_wdr_strength[id] * IQ_WDR_SMOOTH_FACTOR + wdr_target_level * 1 + IQ_WDR_SMOOTH_FACTOR) / (IQ_WDR_SMOOTH_FACTOR + 1), 0, 255); // Unconditional carry
	} else {
		final_wdr_strength[id] = IQ_CLAMP((final_wdr_strength[id] * IQ_WDR_SMOOTH_FACTOR + wdr_target_level * 1) / (IQ_WDR_SMOOTH_FACTOR + 1), 0, 255); // Unconditional chop
	}

	// fastboot 2.0, linux builtin
	#if defined(__KERNEL__)
	if ((msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) && (isp_builtin_get_fastboot_version() == 2)) {
		final_wdr_strength[id] = wdr_target_level;
	}
	#endif

	if (wdr->enable == FALSE) {
		final_wdr_strength[id] = 0;
	}

	wdr_en_reg.reg = GETREG(reg_addr + WDR_CONTROL_REGISTER0_OFS);
	if (msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) {
		#if defined(__KERNEL__)
		if (isp_builtin_get_fastboot_version() == 2) {
			wdr_en_reg.bit.ife_wdr_en = wdr->enable;
		} else {
			wdr_en_reg.bit.ife_wdr_en = FALSE;
		}
		#else
		wdr_en_reg.bit.ife_wdr_en = FALSE;
		#endif
	} else {
		wdr_en_reg.bit.ife_wdr_en = wdr->enable;
	}
	wdr_en_reg.bit.ife_wdr_histogram_sel = 0;
	SETREG(wdr_en_reg.reg, reg_addr + WDR_CONTROL_REGISTER0_OFS);

	wdr_subimg_size.reg = GETREG(reg_addr + IFE_WDR_SUBIMAGE_REGISTER_0_OFS);
	wdr_subimg_size.bit.ife_wdr_subimg_width = IQ_CLAMP((wdr->subimg_size_h - 1), IQ_WDR_SUBIMG_MIN, IQ_WDR_SUBIMG_MAX);
	wdr_subimg_size.bit.ife_wdr_subimg_height = IQ_CLAMP((wdr->subimg_size_v - 1), IQ_WDR_SUBIMG_MIN, IQ_WDR_SUBIMG_MAX);
	SETREG(wdr_subimg_size.reg, reg_addr + IFE_WDR_SUBIMAGE_REGISTER_0_OFS);

	wdr_subimg_ofsi.reg = GETREG(reg_addr + IFE_WDR_SUBIMAGE_REGISTER_2_OFS);
	wdr_subimg_ofsi.bit.ife_wdr_subimg_ofsi = (wdr->subimg_size_h * 8) >> 2;
	SETREG(wdr_subimg_ofsi.reg, reg_addr + IFE_WDR_SUBIMAGE_REGISTER_2_OFS);

	wdr_subimg_ofso.reg = GETREG(reg_addr + IFE_WDR_SUBIMAGE_REGISTER_4_OFS);
	wdr_subimg_ofso.bit.ife_wdr_subimg_ofso = (wdr->subimg_size_h * 8) >> 2;
	SETREG(wdr_subimg_ofso.reg, reg_addr + IFE_WDR_SUBIMAGE_REGISTER_4_OFS);

	wdr_str_reg.bit.ife_wdr_strength = final_wdr_strength[id];
	SETREG(wdr_str_reg.reg, reg_addr + IFE_WDR_OUPUT_BLENDING_REGISTER_OFS);

	wdr_gain_reg.reg = GETREG(reg_addr + IFE_WDR_CONTROL_REGISTER1_OFS);
	wdr_gain_reg.bit.ife_wdr_maxgain = IQ_CLAMP(wdr->max_gain, 1, 255);
	wdr->min_gain = IQ_CLAMP(wdr->min_gain, 1, 255);
	wdr_gain_reg.bit.ife_wdr_mingain = 255 / wdr->min_gain;
	SETREG(wdr_gain_reg.reg, reg_addr + IFE_WDR_CONTROL_REGISTER1_OFS);

	wdr_halo_reg.reg = GETREG(reg_addr + IFE_WDR_TONE_MAPPING_REGISTER_0_OFS);
	wdr_halo_reg.bit.ife_wdr_halo_ratio = wdr->halo_ratio;
	wdr_halo_reg.bit.ife_wdr_halo_slope = wdr->halo_slope;
	SETREG(wdr_halo_reg.reg, reg_addr + IFE_WDR_TONE_MAPPING_REGISTER_0_OFS);

	wdr_fbc_reg.reg = GETREG(reg_addr + IFE_WDR_FBC_REGISTER_0_OFS);
	wdr_fbc_reg.bit.ife_wdr_fbc_en = TRUE;
	wdr_fbc_reg.bit.ife_wdr_fbc_rto = wdr->fbc_ratio;
	SETREG(wdr_fbc_reg.reg, reg_addr + IFE_WDR_FBC_REGISTER_0_OFS);

	wdr_gain_strength_reg.reg = GETREG(reg_addr + IFE_WDR_GAIN_STRENGTH_REGISTER_OFS);
	wdr_gain_strength_reg.bit.ife_wdr_gain_prot_str = wdr->gain_protect_str;
	SETREG(wdr_gain_strength_reg.reg, reg_addr + IFE_WDR_GAIN_STRENGTH_REGISTER_OFS);

	for (i = 0; i < (IQ_WDR_LEFT_NUM / 2); i++) {
		wdr_out_bld_l_reg_0.bit.ife_wdr_outbld_val_lut_l0 = wdr->lut_left[2*i+0];
		wdr_out_bld_l_reg_0.bit.ife_wdr_outbld_val_lut_l1 = wdr->lut_left[2*i+1];
		SETREG(wdr_out_bld_l_reg_0.reg, reg_addr + (IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_0_OFS + (i * 4)));
	}

	wdr_out_bld_l_reg_e.bit.ife_wdr_outbld_val_lut_l64 = wdr->lut_left[64];
	SETREG(wdr_out_bld_l_reg_e.reg, reg_addr + IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_32_OFS);

	for (i = 0; i < (IQ_WDR_RIGHT_NUM / 2); i++) {
		wdr_out_bld_r_reg_0.bit.ife_wdr_outbld_val_lut_r0 = wdr->lut_right[2*i+0];
		wdr_out_bld_r_reg_0.bit.ife_wdr_outbld_val_lut_r1 = wdr->lut_right[2*i+1];
		SETREG(wdr_out_bld_r_reg_0.reg, reg_addr + (IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_0_OFS + (i * 4)));
	}

	wdr_out_bld_r_reg_e.bit.ife_wdr_outbld_val_lut_r16 = wdr->lut_right[16];
	SETREG(wdr_out_bld_r_reg_e.reg, reg_addr + IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_8_OFS);
}

static void iq_builtin_set_cfa(UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_IPE_COLOR_INTERPOLATION_REGISTER1         cfa_edg_dth_reg;
	T_IPE_COLOR_INTERPOLATION_REGISTER9         cfa_freq_th_reg;
	T_IPE_COLOR_INTERPOLATION_REGISTER10        cfa_fcs_weight_reg;
	T_IPE_COLOR_INTERPOLATION_REGISTER11        cfa_fcs_strength_reg_0;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif 
	IQ_CFA_TUNE_PARAM           *final_cfa_tune;

	UINT32                      iso_idx = 0;
	INT32                       i;

	reg_addr = ipe_reg_addr;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->cfa->mode == IQ_OP_TYPE_AUTO) {
		final_cfa_tune = &iq_param->cfa->auto_param[iso_idx];
	} else {
		final_cfa_tune = &iq_param->cfa->manual_param;
	}

	cfa_edg_dth_reg.bit.cfa_edge_dth = final_cfa_tune->edge_dth;
	cfa_edg_dth_reg.bit.cfa_edge_dth2 = final_cfa_tune->edge_dth2;
	SETREG(cfa_edg_dth_reg.reg, reg_addr + IPE_COLOR_INTERPOLATION_REGISTER1_OFS);

	cfa_freq_th_reg.reg = GETREG(reg_addr + IPE_COLOR_INTERPOLATION_REGISTER9_OFS);
	cfa_freq_th_reg.bit.cfa_freq_th = final_cfa_tune->freq_th;
	SETREG(cfa_freq_th_reg.reg, reg_addr + IPE_COLOR_INTERPOLATION_REGISTER9_OFS);

	cfa_fcs_weight_reg.reg = GETREG(reg_addr + IPE_COLOR_INTERPOLATION_REGISTER10_OFS);
	cfa_fcs_weight_reg.bit.cfa_fcs_weight = final_cfa_tune->fcs_weight;
	SETREG(cfa_fcs_weight_reg.reg, reg_addr + IPE_COLOR_INTERPOLATION_REGISTER10_OFS);

	for (i = 0; i < (IQ_CFA_FCS_NUM / 8); i++) {
		cfa_fcs_strength_reg_0.bit.cfa_fcs_strength0 = final_cfa_tune->fcs_strength[8*i+0];
		cfa_fcs_strength_reg_0.bit.cfa_fcs_strength1 = final_cfa_tune->fcs_strength[8*i+1];
		cfa_fcs_strength_reg_0.bit.cfa_fcs_strength2 = final_cfa_tune->fcs_strength[8*i+2];
		cfa_fcs_strength_reg_0.bit.cfa_fcs_strength3 = final_cfa_tune->fcs_strength[8*i+3];
		cfa_fcs_strength_reg_0.bit.cfa_fcs_strength4 = final_cfa_tune->fcs_strength[8*i+4];
		cfa_fcs_strength_reg_0.bit.cfa_fcs_strength5 = final_cfa_tune->fcs_strength[8*i+5];
		cfa_fcs_strength_reg_0.bit.cfa_fcs_strength6 = final_cfa_tune->fcs_strength[8*i+6];
		cfa_fcs_strength_reg_0.bit.cfa_fcs_strength7 = final_cfa_tune->fcs_strength[8*i+7];
		SETREG(cfa_fcs_strength_reg_0.reg, reg_addr + (IPE_COLOR_INTERPOLATION_REGISTER11_OFS + (i * 4)));
	}
}

static void iq_builtin_set_pfr(UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_IPE_MODE_REGISTER_1                                       ipe_control_reg;
	T_PURPLE_FRINGE_REDUCTION_REGISTER0                         pfr_control_reg;
	T_PURPLE_FRINGE_REDUCTION_REGISTER2                         pfr_color_set0_reg;
	T_PURPLE_FRINGE_REDUCTION_REGISTER3                         pfr_color_set1_reg;
	T_PURPLE_FRINGE_REDUCTION_REGISTER4                         pfr_color_set2_reg;
	T_PURPLE_FRINGE_REDUCTION_REGISTER5                         pfr_color_set3_reg;
	T_PURPLE_FRINGE_REDUCTION_REGISTER14                        pfr_luma_lut_reg_0;
	T_PURPLE_FRINGE_REDUCTION_REGISTER17                        pfr_luma_lut_reg_e;

	#if defined(__KERNEL__)
	void __iomem                                                *reg_addr = NULL;
	#else
	unsigned int                                                reg_addr;
	#endif 
	IQ_PFR_PARAM                                                *pfr;
	IQ_PFR_TUNE_PARAM                                           *final_pfr_tune;

	UINT32                                                      iso_idx = 0;
	INT32                                                       i;

	reg_addr = ipe_reg_addr;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	pfr = iq_param->pfr;
	if (pfr->mode == IQ_OP_TYPE_AUTO) {
		final_pfr_tune = &iq_param->pfr->auto_param[iso_idx];
	} else {
		final_pfr_tune = &iq_param->pfr->manual_param;
	}

	ipe_control_reg.reg = GETREG(reg_addr + IPE_MODE_REGISTER_1_OFS);
	ipe_control_reg.bit.pfr_en = pfr->enable;
	if ((pfr->set0_en == FALSE) && (pfr->set1_en == FALSE) && (pfr->set2_en == FALSE) && (pfr->set3_en == FALSE)) {
		ipe_control_reg.bit.pfr_en = FALSE;
	}
	SETREG(ipe_control_reg.reg, reg_addr + IPE_MODE_REGISTER_1_OFS);

	for (i = 0; i < (IQ_PFR_LUMA_LEN / 4); i++) {
		pfr_luma_lut_reg_0.bit.pfr_luma_level0 = pfr->luma_lut[4*i+0];
		pfr_luma_lut_reg_0.bit.pfr_luma_level1 = pfr->luma_lut[4*i+1];
		pfr_luma_lut_reg_0.bit.pfr_luma_level2 = pfr->luma_lut[4*i+2];
		pfr_luma_lut_reg_0.bit.pfr_luma_level3 = pfr->luma_lut[4*i+3];
		SETREG(pfr_luma_lut_reg_0.reg, reg_addr + (PURPLE_FRINGE_REDUCTION_REGISTER14_OFS + (i * 4)));
	}

	pfr_luma_lut_reg_e.bit.pfr_luma_level12 = pfr->luma_lut[12];
	pfr_luma_lut_reg_e.bit.pfr_luma_th = final_pfr_tune->luma_th;
	SETREG(pfr_luma_lut_reg_e.reg, reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER17_OFS);

	pfr_control_reg.reg = GETREG(reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER0_OFS);
	pfr_control_reg.bit.pfr_set0_en = pfr->set0_en;
	pfr_control_reg.bit.pfr_set1_en = pfr->set1_en;
	pfr_control_reg.bit.pfr_set2_en = pfr->set2_en;
	pfr_control_reg.bit.pfr_set3_en = pfr->set3_en;
	pfr_control_reg.bit.pfr_out_wet = final_pfr_tune->pfr_strength;
	SETREG(pfr_control_reg.reg, reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER0_OFS);

	pfr_color_set0_reg.reg = GETREG(reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER2_OFS);
	pfr_color_set0_reg.bit.pfr_color_u0 = pfr->set0_color_u;
	pfr_color_set0_reg.bit.pfr_color_v0 = pfr->set0_color_v;
	SETREG(pfr_color_set0_reg.reg, reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER2_OFS);

	pfr_color_set1_reg.reg = GETREG(reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER3_OFS);
	pfr_color_set1_reg.bit.pfr_color_u1 = pfr->set1_color_u;
	pfr_color_set1_reg.bit.pfr_color_v1 = pfr->set1_color_v;
	SETREG(pfr_color_set1_reg.reg, reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER3_OFS);

	pfr_color_set2_reg.reg = GETREG(reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER4_OFS);
	pfr_color_set2_reg.bit.pfr_color_u2 = pfr->set2_color_u;
	pfr_color_set2_reg.bit.pfr_color_v2 = pfr->set2_color_v;
	SETREG(pfr_color_set2_reg.reg, reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER4_OFS);

	pfr_color_set3_reg.reg = GETREG(reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER5_OFS);
	pfr_color_set3_reg.bit.pfr_color_u3 = pfr->set1_color_u;
	pfr_color_set3_reg.bit.pfr_color_v3 = pfr->set1_color_v;
	SETREG(pfr_color_set3_reg.reg, reg_addr + PURPLE_FRINGE_REDUCTION_REGISTER5_OFS);
}

static void iq_builtin_set_ccm(UINT32 id, UINT32 ct, IQ_PARAM_PTR *iq_param, BOOL ui_night_mode)
{
	T_COLOR_CORRECTION_REGISTER_0                      ccm_reg_0;
	T_COLOR_CORRECTION_REGISTER_1                      ccm_reg_1;
	T_COLOR_CORRECTION_REGISTER_2                      ccm_reg_2;
	T_COLOR_CORRECTION_REGISTER_3                      ccm_reg_3;
	T_COLOR_CORRECTION_REGISTER_4                      ccm_reg_4;
	T_COLOR_CONTROL_INTENSITY_MAPPING_REGISTER_0       int_reg_0;
	T_COLOR_CONTROL_HUE_MAPPING_REGISTER_0             hue_reg_0;
	T_COLOR_CONTROL_SATURATION_MAPPING_REGISTER_0      sat_reg_0;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif 
	IQ_CCM_PARAM                *ccm;
	IQ_CCM_AUTO_PARAM           *ccm_l, *ccm_h;

	UINT32                      color_idx_l = 0, color_idx_h = 0;
	UINT32                      color_start = 0, color_end = 0;
	INT16                       night_coef[IQ_CCM_LEN] = {0x100, 0x0, 0x0, 0x0, 0x100, 0x0, 0x0, 0x0, 0x100};
	INT16                       final_coef[IQ_CCM_LEN] = {0x100, 0x0, 0x0, 0x0, 0x100, 0x0, 0x0, 0x0, 0x100};
	UINT8                       final_hue_tab[IQ_CCM_HUETAB_LEN] = {0};
	INT32                       final_sat_tab[IQ_CCM_SATTAB_LEN] = {0};
	INT32                       final_int_tab[IQ_CCM_INTTAB_LEN] = {0};
	INT32                       i;

	reg_addr = ipe_reg_addr;
	ccm = iq_param->ccm;

	if (ccm->mode == IQ_OP_TYPE_AUTO) {
		if (ct <=  ccm->auto_param[IQ_COLOR_TEMPERATURE_4].ct) {
			color_idx_l = IQ_COLOR_TEMPERATURE_4;
			color_idx_h = IQ_COLOR_TEMPERATURE_4;
			color_start = ccm->auto_param[IQ_COLOR_TEMPERATURE_4].ct;
			color_end = ccm->auto_param[IQ_COLOR_TEMPERATURE_4].ct;
		}
		for (i = 1; i < IQ_COLOR_ID_MAX_NUM; i++) {
			if (ct > ccm->auto_param[i].ct) {
				color_idx_l = i;
				color_idx_h = i-1;
				color_start = ccm->auto_param[i].ct;
				color_end = ccm->auto_param[i-1].ct;
				break;
			}
		}

		ccm_l = &ccm->auto_param[color_idx_l];
		ccm_h = &ccm->auto_param[color_idx_h];

		iq_intpl_tbl_cc(ccm_l->coef, ccm_h->coef, color_start, color_end, ct, final_coef);
		for (i = 0; i < IQ_CCM_HUETAB_LEN; i++) {
			final_hue_tab[i] = (UINT8)iq_builtin_intpl(ct, ccm_l->hue_tab[i], ccm_h->hue_tab[i], color_start, color_end);
		}
		for (i = 0; i < IQ_CCM_SATTAB_LEN; i++) {
			final_sat_tab[i] = (INT32)iq_builtin_intpl(ct, ccm_l->sat_tab[i], ccm_h->sat_tab[i], color_start, color_end);
		}
		for (i = 0; i < IQ_CCM_INTTAB_LEN; i++) {
			final_int_tab[i] = (INT32)iq_builtin_intpl(ct, ccm_l->int_tab[i], ccm_h->int_tab[i], color_start, color_end);
		}
	} else {
		memcpy(final_coef, ccm->manual_param.coef, sizeof(INT16) * IQ_CCM_LEN);
		memcpy(final_hue_tab, ccm->manual_param.hue_tab, sizeof(UINT8) * IQ_CCM_HUETAB_LEN);
		memcpy(final_sat_tab, ccm->manual_param.sat_tab, sizeof(INT8) * IQ_CCM_SATTAB_LEN);
		memcpy(final_int_tab, ccm->manual_param.int_tab, sizeof(INT8) * IQ_CCM_INTTAB_LEN);
	}

	if ((ccm->enable == FALSE) || (ui_night_mode == TRUE)) {
		memcpy(final_coef, night_coef, sizeof(INT16) * IQ_CCM_LEN);
		for (i = 1; i < IQ_COLOR_ID_MAX_NUM; i++) {
			final_hue_tab[i] = 128;
		}
		memset(final_sat_tab, 0, sizeof(INT32) * IQ_CCM_SATTAB_LEN);
		memset(final_int_tab, 0, sizeof(INT32) * IQ_CCM_INTTAB_LEN);
	}

	ccm_reg_0.reg = GETREG(reg_addr + COLOR_CORRECTION_REGISTER_0_OFS);
	ccm_reg_0.bit.coef_rr = final_coef[0];
	SETREG(ccm_reg_0.reg, reg_addr + COLOR_CORRECTION_REGISTER_0_OFS);

	ccm_reg_1.bit.coef_rg = final_coef[1];
	ccm_reg_1.bit.coef_rb = final_coef[2];
	SETREG(ccm_reg_1.reg, reg_addr + COLOR_CORRECTION_REGISTER_1_OFS);

	ccm_reg_2.bit.coef_gr = final_coef[3];
	ccm_reg_2.bit.coef_gg = final_coef[4];
	SETREG(ccm_reg_2.reg, reg_addr + COLOR_CORRECTION_REGISTER_2_OFS);

	ccm_reg_3.bit.coef_gb = final_coef[5];
	ccm_reg_3.bit.coef_br = final_coef[6];
	SETREG(ccm_reg_3.reg, reg_addr + COLOR_CORRECTION_REGISTER_3_OFS);

	ccm_reg_4.bit.coef_bg = final_coef[7];
	ccm_reg_4.bit.coef_bb = final_coef[8];
	SETREG(ccm_reg_4.reg, reg_addr + COLOR_CORRECTION_REGISTER_4_OFS);

	for (i = 0; i < (IQ_CCM_SATTAB_LEN / 4); i++) {
		hue_reg_0.bit.chuem0 = final_hue_tab[4*i+0];
		hue_reg_0.bit.chuem1 = final_hue_tab[4*i+1];
		hue_reg_0.bit.chuem2 = final_hue_tab[4*i+2];
		hue_reg_0.bit.chuem3 = final_hue_tab[4*i+3];
		SETREG(hue_reg_0.reg, reg_addr + (COLOR_CONTROL_HUE_MAPPING_REGISTER_0_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_CCM_SATTAB_LEN / 4); i++) {
		sat_reg_0.bit.csatm0 = final_sat_tab[4*i+0];
		sat_reg_0.bit.csatm1 = final_sat_tab[4*i+1];
		sat_reg_0.bit.csatm2 = final_sat_tab[4*i+2];
		sat_reg_0.bit.csatm3 = final_sat_tab[4*i+3];
		SETREG(sat_reg_0.reg, reg_addr + (COLOR_CONTROL_SATURATION_MAPPING_REGISTER_0_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_CCM_INTTAB_LEN / 4); i++) {
		int_reg_0.bit.cintm0 = final_int_tab[4*i+0];
		int_reg_0.bit.cintm1 = final_int_tab[4*i+1];
		int_reg_0.bit.cintm2 = final_int_tab[4*i+2];
		int_reg_0.bit.cintm3 = final_int_tab[4*i+3];
		SETREG(int_reg_0.reg, reg_addr + (COLOR_CONTROL_INTENSITY_MAPPING_REGISTER_0_OFS + (i * 4)));
	}
}

static UINT32 *iq_builtin_gammacurve_conv(UINT32 id, UINT32 *r_lut, UINT32 *g_lut, UINT32 *b_lut)
{
	INT32 i, page;

	if (gamma_buffer[id] == NULL) {
		return NULL;
	}

	page = 0;
	for (i = 0; i < IQ_GAMMA_LEN; i++) {
		if (i % 2 == 0) {
			gamma_buffer[id][page + i / 2] = r_lut[i];
		} else {
			gamma_buffer[id][page + i / 2] |= (r_lut[i] << 10);
		}
	}

	page = 65;
	for (i = 0; i < IQ_GAMMA_LEN; i++) {
		if (i % 2 == 0) {
			gamma_buffer[id][page + i / 2] = g_lut[i];
		} else {
			gamma_buffer[id][page + i / 2] |= (g_lut[i] << 10);
		}
	}

	page = 65 * 2;
	for (i = 0; i < IQ_GAMMA_LEN; i++) {
		if (i % 2 == 0) {
			gamma_buffer[id][page + i / 2] = b_lut[i];
		} else {
			gamma_buffer[id][page + i / 2] |= (b_lut[i] << 10);
		}
	}

	vos_cpu_dcache_sync((ULONG)gamma_buffer[id], GAMMA_BUFFER_SIZE, VOS_DMA_TO_DEVICE);

	return gamma_buffer[id];
}

static void iq_builtin_set_gamma(UINT32 id, UINT32 lv, IQ_PARAM_PTR *iq_param, UINT32 ui_gamma_lv)
{
	T_IPE_CONTROL_REGISTER           ipe_control;
	T_IPE_MODE_REGISTER_1            ipe_en_reg;
	T_DMA_TO_IPE_REGISTER_5          ipe_reg_5;
	T_EDGE_GAMMA0                    ipe_edge_gamma_reg;
	T_EDGE_GAMMA21                   ipe_edge_gamma_reg21;

	#if defined(__KERNEL__)
	void __iomem                     *reg_addr = NULL;
	#else
	unsigned int                     reg_addr;
	#endif 
	IQ_GAMMA_PARAM                   *gamma;
	IQ_GAMMA_AUTO_PARAM              *gamma_l, *gamma_h;

	ISP_BUILTIN_FUNC                 func_en = 0;
	ULONG                            input_addr[3];
	UINT32                           gamma_idx_l = 0, gamma_idx_h = 0;
	UINT32                           curr_gamma_level;
	UINT32                           curr_edge_gamma[IQ_EDGE_GAMMA_LEN] = {0};
	ULONG                            final_buffer_addr, phy_adr;
	INT32                            i = 0, j = 0;

	reg_addr = ipe_reg_addr;
	gamma = iq_param->gamma;

	if (gamma->mode == IQ_OP_TYPE_AUTO) {
		if (gamma->auto_sel == IQ_GAMMA_AUTO_BY_AE_LV) {
			if (lv <= (gamma->auto_param[IQ_GAMMA_SET4].lv * LV_BASE)) {
				gamma_idx_l = IQ_GAMMA_SET4;
				gamma_idx_h = IQ_GAMMA_SET4;
			}
			for (i = 1; i < IQ_GAMMA_ID_MAX_NUM; i++) {
				if (lv > (gamma->auto_param[i].lv * LV_BASE)) {
					gamma_idx_l = i;
					gamma_idx_h = i - 1;
					break;
				}
			}

			gamma_l = &gamma->auto_param[gamma_idx_l];
			gamma_h = &gamma->auto_param[gamma_idx_h];

			curr_gamma_level = iq_builtin_intpl(lv / (LV_BASE / 100), gamma_l->gamma_level, gamma_h->gamma_level, gamma_l->lv * 100, gamma_h->lv * 100);
			//DBG_DUMP("curr_gamma_level = %d, lv = %d, idx_l = %d (lv:%d, level:%d), idx_h = %d (lv:%d, level:%d) \r\n"
			//	, curr_gamma_level, lv
			//	, gamma_idx_l, gamma_l->lv * 100, gamma_l->gamma_level
			//	, gamma_idx_h, gamma_h->lv * 100, gamma_h->gamma_level);
		} else {
			curr_gamma_level = ui_gamma_lv;
			//DBG_DUMP("ui_gamma_lv = %d \r\n", ui_gamma_lv);
		}

		if (curr_gamma_level <= gamma->auto_set0_level) {
			memcpy(curr_gamma[id], gamma->auto_set0_lut, sizeof(UINT32) * IQ_GAMMA_LEN);
		} else if (curr_gamma_level <= gamma->auto_set1_level) {
			iq_intpl_tbl_gamma(gamma->auto_set0_lut, gamma->auto_set1_lut, gamma->auto_set0_level, gamma->auto_set1_level, curr_gamma_level, curr_gamma[id]);
		} else if (curr_gamma_level <= gamma->auto_set2_level) {
			iq_intpl_tbl_gamma(gamma->auto_set1_lut, gamma->auto_set2_lut, gamma->auto_set1_level, gamma->auto_set2_level, curr_gamma_level, curr_gamma[id]);
		} else if (curr_gamma_level <= gamma->auto_set3_level) {
			iq_intpl_tbl_gamma(gamma->auto_set2_lut, gamma->auto_set3_lut, gamma->auto_set2_level, gamma->auto_set3_level, curr_gamma_level, curr_gamma[id]);
		} else if (curr_gamma_level <= gamma->auto_set4_level) {
			iq_intpl_tbl_gamma(gamma->auto_set3_lut, gamma->auto_set4_lut, gamma->auto_set3_level, gamma->auto_set4_level, curr_gamma_level, curr_gamma[id]);
		} else {
			memcpy(curr_gamma[id], gamma->auto_set4_lut, sizeof(UINT32) * IQ_GAMMA_LEN);
		}
	} else {
		memcpy(curr_gamma[id],  gamma->manual_lut, sizeof(UINT32) * IQ_GAMMA_LEN);
	}

	input_addr[0] = (ULONG)&curr_gamma[id][0];
	input_addr[1] = (ULONG)&curr_gamma[id][0];
	input_addr[2] = (ULONG)&curr_gamma[id][0];

	ipe_en_reg.reg = GETREG(reg_addr + IPE_MODE_REGISTER_1_OFS);
	ipe_en_reg.bit.rgbgamma_en = gamma->enable;
	SETREG(ipe_en_reg.reg, reg_addr + IPE_MODE_REGISTER_1_OFS);

	final_buffer_addr = (ULONG)iq_builtin_gammacurve_conv(id, (UINT32 *)input_addr[0], (UINT32 *)input_addr[1], (UINT32 *)input_addr[2]);
	if (final_buffer_addr != 0) {
		//phy_adr = vos_cpu_get_phy_addr((unsigned int)final_buffer_addr);
		phy_adr = gamma_buffer_pa_addr[id];
		ipe_reg_5.bit.dram_sai_gamma = phy_adr >> 2;
		SETREG(ipe_reg_5.reg, reg_addr + DMA_TO_IPE_REGISTER_5_OFS);

		ipe_control.reg = GETREG(reg_addr + IPE_CONTROL_REGISTER_OFS);
		ipe_control.bit.ipe_rwgamma = 1;
		ipe_control.bit.ipe_rwgamma_opt = 3;
		SETREG(ipe_control.reg, reg_addr + IPE_CONTROL_REGISTER_OFS);
	}

	// edge gamma
	func_en = isp_builtin_get_func_en(id) & ISP_BUILTIN_FUNC_SHDR;
	if ((func_en == TRUE) && (gamma->enable == FALSE)) {
		memcpy(curr_edge_gamma, de_gamma, sizeof(UINT16) * IQ_EDGE_GAMMA_LEN);
	} else {
		for(i = 0; i < IQ_GAMMA_LEN; i += 2){
			if (gamma->enable) {
				curr_edge_gamma[j++] = curr_gamma[id][i];
			} else {
				curr_edge_gamma[j++] = IQ_CLAMP(i << 3, 0, 1023);
			}
		}
	}

	for (i = 0; i < (IQ_EDGE_GAMMA_LEN / 3); i++) {
		ipe_edge_gamma_reg.bit.edge_lut0 = curr_edge_gamma[3*i+0];
		ipe_edge_gamma_reg.bit.edge_lut1 = curr_edge_gamma[3*i+1];
		ipe_edge_gamma_reg.bit.edge_lut2 = curr_edge_gamma[3*i+2];
		SETREG(ipe_edge_gamma_reg.reg, reg_addr + (EDGE_GAMMA0_OFS + (i * 4)));
	}

	ipe_edge_gamma_reg21.bit.edge_lut63 = curr_edge_gamma[63];
	ipe_edge_gamma_reg21.bit.edge_lut64 = curr_edge_gamma[64];
	SETREG(ipe_edge_gamma_reg21.reg, reg_addr + EDGE_GAMMA21_OFS);
}

static void iq_builtin_set_cst(UINT32 id, UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_COLOR_SPACE_TRANSFORM_REGISTER_0          cst_reg_0;
	T_COLOR_SPACE_TRANSFORM_REGISTER_1          cst_reg_1;
	T_COLOR_SPACE_TRANSFORM_REGISTER_2          cst_reg_2;

	#if defined(__KERNEL__)
	void __iomem                     *reg_addr = NULL;
	#else
	unsigned int                     reg_addr;
	#endif 
	IQ_CST_PARAM                     *cst;
	IQ_CST_TUNE_PARAM                *final_cst_tune;

	UINT32                      iso_idx = 0;
	INT32                       i;

	reg_addr = ipe_reg_addr;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->cst->mode == IQ_OP_TYPE_AUTO) {
		final_cst_tune = &iq_param->cst->auto_param[iso_idx];
	} else {
		final_cst_tune = &iq_param->cst->manual_param;
	}
	cst = iq_param->cst;

	cst_reg_0.bit.coef_yr = cst->cst_coef[0];
	cst_reg_0.bit.coef_yg = cst->cst_coef[1];
	cst_reg_0.bit.coef_yb = cst->cst_coef[2];
	cst_reg_0.bit.cstp_rat = final_cst_tune->cstp_ratio;
	SETREG(cst_reg_0.reg, reg_addr + COLOR_SPACE_TRANSFORM_REGISTER_0_OFS);

	cst_reg_1.bit.coef_ur = cst->cst_coef[3];
	cst_reg_1.bit.coef_ug = cst->cst_coef[4];
	cst_reg_1.bit.coef_ub = cst->cst_coef[5];
	SETREG(cst_reg_1.reg, reg_addr + COLOR_SPACE_TRANSFORM_REGISTER_1_OFS);

	cst_reg_2.bit.coef_vr = cst->cst_coef[6];
	cst_reg_2.bit.coef_vg = cst->cst_coef[7];
	cst_reg_2.bit.coef_vb = cst->cst_coef[8];
	SETREG(cst_reg_2.reg, reg_addr + COLOR_SPACE_TRANSFORM_REGISTER_2_OFS);

	// y_ofs implement at iq_builtin_set_contrast
	// cb_ofs and cr_ofs implement at iq_builtin_set_color
}

static void iq_builtin_set_defog(UINT32 id, ISP_BUILTIN_IQ_TRIG_MSG msg, UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_IPE_MODE_REGISTER_1                        ipe_mode_reg;
	T_DEFOG_FOG_MODIFY_REGISTER_0                fog_modify_reg_0;
	T_DEFOG_FOG_MODIFY_REGISTER_8                fog_modify_reg_8;
	T_DEFOG_STRENGTH_CONTROL_REGISTER_5          strength_control_reg_5;
	T_DEFOG_STRENGTH_CONTROL_REGISTER_6          strength_control_reg_6;
	T_DEFOG_OUTPUT_BLENDING_REGISTER_0           output_blending_reg_0;
	T_DEFOG_OUTPUT_BLENDING_REGISTER_4           output_blending_reg_4;
	T_DEFOG_OUTPUT_BLENDING_REGISTER_5           output_blending_reg_5;
	T_DEFOG_OUTPUT_BLENDING_REGISTER_8           output_blending_reg_8;
	T_DEFOG_AIRLIGHT_REGISTER_0                  defog_airlight_reg_0;
	T_DEFOG_AIRLIGHT_REGISTER_1                  defog_airlight_reg_1;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif 
	IQ_DEFOG_PARAM              *defog;
	IQ_DEFOG_AUTO_PARAM         *defog_auto;
	IQ_DEFOG_MANUAL_PARAM       *defog_manual;

	ISP_BUILTIN_FUNC            func_en = 0;
	UINT32                      iso_idx = 0;
	UINT16                      fog_mod_level;
	IQLIB_DEFOG                 defog_result;
	UINT16                      dfg_airlight_result[ISP_DFG_AIRLIGHT_NUM] = {1023, 1023, 1023};
	static IQLIB_DEFOG          defog_preset[IQ_ID_MAX_NUM];
	static UINT16               dfg_airlight_preset[IQ_ID_MAX_NUM][ISP_DFG_AIRLIGHT_NUM];

	UINT16                      final_fog_mod_lut[IQ_DEFOG_FOG_MOD_LEN];
	UINT8                       final_fog_ratio;
	UINT8                       final_gain_th;
	UINT8                       final_outbld_lum_wt[IQ_DEFOG_OUTPUT_BLD_LEN];
	UINT16                      final_dfg_airlight[ISP_DFG_AIRLIGHT_NUM];
	UINT16                      final_air_min = 1023;
	INT32                       i;

	reg_addr = ipe_reg_addr;

	func_en = isp_builtin_get_func_en(id) & ISP_BUILTIN_FUNC_DEFOG;
	if (func_en == 0) {
		if ((msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) && (func_en == 0) && (iq_param->defog->enable == TRUE)) {
			DBG_DUMP("[builtin IQ] defog_enable = 0 in nvt-fastboot-ipp.dtsi, please check !! \r\n");
		}
		ipe_mode_reg.reg = GETREG(reg_addr + IPE_MODE_REGISTER_1_OFS);
		ipe_mode_reg.bit.defog_en = FALSE;
		SETREG(ipe_mode_reg.reg, reg_addr + IPE_MODE_REGISTER_1_OFS);

		return;
	}

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	defog = iq_param->defog;
	defog_auto = &iq_param->defog->auto_param[iso_idx];
	defog_manual = &iq_param->defog->manual_param;

	if (msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) {
		for (i = 0; i < IQ_ID_MAX_NUM; i++) {
			defog_preset[i].fog_level = 0,
			defog_preset[i].outbld_lum_wt = 192,
			defog_preset[i].gain_th = 96,
			dfg_airlight_preset[i][0] = 1023;
			dfg_airlight_preset[i][1] = 1023;
			dfg_airlight_preset[i][2] = 1023;
		}

		ipe_mode_reg.reg = GETREG(reg_addr + IPE_MODE_REGISTER_1_OFS);
		ipe_mode_reg.bit.defog_en = FALSE;
		SETREG(ipe_mode_reg.reg, reg_addr + IPE_MODE_REGISTER_1_OFS);
		return;
	}

	if (iq_param->defog->mode == IQ_OP_TYPE_AUTO) {
		defog_result = iq_lib_defog_strength_nvt(id, defog_auto->dr_th, (1024 - defog_auto->fog_level_max));

		//prevent dramatic change (fog_mod_level)
		if (defog_preset[id].fog_level > defog_result.fog_level) {
			defog_preset[id].fog_level -= IQ_MAX(1, (defog_preset[id].fog_level - defog_result.fog_level) / 3);
		} else if (defog_preset[id].fog_level < defog_result.fog_level) {
			defog_preset[id].fog_level += IQ_MAX(1, (defog_result.fog_level - defog_preset[id].fog_level) / 3);
		} else {
			defog_preset[id].fog_level = defog_result.fog_level;
		}
		fog_mod_level = defog_preset[id].fog_level;

		final_fog_ratio = defog_auto->fog_ratio;

		//prevent dramatic change (gain_th)
		if (defog_preset[id].gain_th > defog_result.gain_th) {
			defog_preset[id].gain_th -= IQ_MAX(1, (defog_preset[id].gain_th - defog_result.gain_th) / 3);
		} else if (defog_preset[id].gain_th < defog_result.gain_th) {
			defog_preset[id].gain_th += IQ_MAX(1, (defog_result.gain_th - defog_preset[id].gain_th) / 3);
		} else {
			defog_preset[id].gain_th = defog_result.gain_th;
		}
		final_gain_th = IQ_CLAMP(defog_preset[id].gain_th, 32, 255);

		// 0: strong, 255: weak
		defog_result.outbld_lum_wt = IQ_CLAMP(defog_result.outbld_lum_wt + (((255 - defog_result.outbld_lum_wt) * defog_auto->outbld_wt) >> 8), 0, 255);
		//prevent dramatic change (outbld_lum_wt)
		if (defog_preset[id].outbld_lum_wt > defog_result.outbld_lum_wt) {
			defog_preset[id].outbld_lum_wt -= IQ_MAX(1, (defog_preset[id].outbld_lum_wt - defog_result.outbld_lum_wt) / 3);
		} else if (defog_preset[id].outbld_lum_wt < defog_result.outbld_lum_wt) {
			defog_preset[id].outbld_lum_wt += IQ_MAX(1, (defog_result.outbld_lum_wt - defog_preset[id].outbld_lum_wt) / 3);
		} else {
			defog_preset[id].outbld_lum_wt = defog_result.outbld_lum_wt;
		}
		for (i = 0; i < IQ_DEFOG_OUTPUT_BLD_LEN; i++) {
			final_outbld_lum_wt[i] =  IQ_CLAMP(defog_preset[id].outbld_lum_wt, 0, 255);
		}
	} else {
		fog_mod_level = defog_manual->fog_level;
		final_fog_ratio = defog_manual->fog_ratio;
		final_gain_th = IQ_CLAMP(defog_manual->gain_th, 32, 255);
		memcpy(final_outbld_lum_wt, defog_manual->outbld_lum_wt, sizeof(UINT8) * 17);
	}

	iq_lib_defog_fog_mod_level_to_lut_nvt(id, fog_mod_level, final_fog_mod_lut);

	iq_lib_defog_airlight_nvt(id, &dfg_airlight_result[0], &dfg_airlight_result[1], &dfg_airlight_result[2]);
	//prevent dramatic change
	for (i = 0; i < ISP_DFG_AIRLIGHT_NUM; i++) {
		if (dfg_airlight_preset[id][i] > dfg_airlight_result[i]) {
			dfg_airlight_preset[id][i] -= IQ_MAX(1, (dfg_airlight_preset[id][i] - dfg_airlight_result[i]) / 2);
		} else if (dfg_airlight_preset[id][i] < dfg_airlight_result[i]) {
			dfg_airlight_preset[id][i] += IQ_MAX(1, (dfg_airlight_result[i] - dfg_airlight_preset[id][i]) / 2);
		} else {
			dfg_airlight_preset[id][i] = dfg_airlight_result[i];
		}
	}
	final_dfg_airlight[0] = dfg_airlight_preset[id][0];
	final_dfg_airlight[1] = dfg_airlight_preset[id][1];
	final_dfg_airlight[2] = dfg_airlight_preset[id][2];

	for (i = 0; i < ISP_DFG_AIRLIGHT_NUM; i++) {
		if (final_dfg_airlight[i] < final_air_min) {
			final_air_min = final_dfg_airlight[i];
		}
	}

	ipe_mode_reg.reg = GETREG(reg_addr + IPE_MODE_REGISTER_1_OFS);
	ipe_mode_reg.bit.defog_en = defog->enable;
	SETREG(ipe_mode_reg.reg, reg_addr + IPE_MODE_REGISTER_1_OFS);

	output_blending_reg_4.reg = GETREG(reg_addr + DEFOG_OUTPUT_BLENDING_REGISTER_4_OFS);
	output_blending_reg_4.bit.defog_local_outbld_en = defog->outbld_local_en;
	SETREG(output_blending_reg_4.reg, reg_addr + DEFOG_OUTPUT_BLENDING_REGISTER_4_OFS);

	defog->outbld_diff_wt = IQ_CLAMP(defog->outbld_diff_wt, 0, 7);

	for (i = 0; i < (IQ_DEFOG_OUTPUT_BLD_LEN / 5); i++) {
		output_blending_reg_5.bit.defog_outbld_diffwt0 = iq_defog_diff_wt[defog->outbld_diff_wt][5*i+0];
		output_blending_reg_5.bit.defog_outbld_diffwt1 = iq_defog_diff_wt[defog->outbld_diff_wt][5*i+1];
		output_blending_reg_5.bit.defog_outbld_diffwt2 = iq_defog_diff_wt[defog->outbld_diff_wt][5*i+2];
		output_blending_reg_5.bit.defog_outbld_diffwt3 = iq_defog_diff_wt[defog->outbld_diff_wt][5*i+3];
		output_blending_reg_5.bit.defog_outbld_diffwt4 = iq_defog_diff_wt[defog->outbld_diff_wt][5*i+4];
		SETREG(output_blending_reg_5.reg, reg_addr + (DEFOG_OUTPUT_BLENDING_REGISTER_5_OFS + (i * 4)));
	}

	output_blending_reg_8.bit.defog_outbld_diffwt15 = iq_defog_diff_wt[defog->outbld_diff_wt][15];
	output_blending_reg_8.bit.defog_outbld_diffwt16 = iq_defog_diff_wt[defog->outbld_diff_wt][16];
	SETREG(output_blending_reg_8.reg, reg_addr + DEFOG_OUTPUT_BLENDING_REGISTER_8_OFS);

	// fog_mod_lut
	for (i = 0; i < (IQ_DEFOG_FOG_MOD_LEN / 2); i++) {
		fog_modify_reg_0.bit.defog_mod_lut0 = final_fog_mod_lut[2*i+0];
		fog_modify_reg_0.bit.defog_mod_lut1 = final_fog_mod_lut[2*i+1];
		SETREG(fog_modify_reg_0.reg, reg_addr + (DEFOG_FOG_MODIFY_REGISTER_0_OFS + (i * 4)));
	}

	fog_modify_reg_8.bit.defog_mod_lut16 = final_fog_mod_lut[16];
	SETREG(fog_modify_reg_8.reg, reg_addr + DEFOG_FOG_MODIFY_REGISTER_8_OFS);

	// fog_ratio
	strength_control_reg_5.reg = GETREG(reg_addr + DEFOG_STRENGTH_CONTROL_REGISTER_5_OFS);
	strength_control_reg_5.bit.defog_fog_rto = final_fog_ratio;
	strength_control_reg_5.bit.defog_min_diff = final_air_min * defog->min_diff_ratio / 16;
	SETREG(strength_control_reg_5.reg, reg_addr + DEFOG_STRENGTH_CONTROL_REGISTER_5_OFS);

	// gain_th
	strength_control_reg_6.bit.defog_gain_th = final_gain_th;
	SETREG(strength_control_reg_6.reg, reg_addr + DEFOG_STRENGTH_CONTROL_REGISTER_6_OFS);

	// outbld_lum_wt
	for (i = 0; i < (IQ_DEFOG_OUTPUT_BLD_LEN / 4); i++) {
		output_blending_reg_0.bit.defog_outbld_lumwt0 = final_outbld_lum_wt[4*i+0];
		output_blending_reg_0.bit.defog_outbld_lumwt1 = final_outbld_lum_wt[4*i+1];
		output_blending_reg_0.bit.defog_outbld_lumwt2 = final_outbld_lum_wt[4*i+2];
		output_blending_reg_0.bit.defog_outbld_lumwt3 = final_outbld_lum_wt[4*i+3];
		SETREG(output_blending_reg_0.reg, reg_addr + (DEFOG_OUTPUT_BLENDING_REGISTER_0_OFS + (i * 4)));
	}

	output_blending_reg_4.reg = GETREG(reg_addr + DEFOG_OUTPUT_BLENDING_REGISTER_4_OFS);
	output_blending_reg_4.bit.defog_outbld_lumwt16 = final_outbld_lum_wt[16];
	SETREG(output_blending_reg_4.reg, reg_addr + DEFOG_OUTPUT_BLENDING_REGISTER_4_OFS);

	defog_airlight_reg_0.bit.defog_air0 = final_dfg_airlight[0];
	defog_airlight_reg_0.bit.defog_air1 = final_dfg_airlight[1];
	SETREG(defog_airlight_reg_0.reg, reg_addr + DEFOG_AIRLIGHT_REGISTER_0_OFS);

	defog_airlight_reg_1.bit.defog_air2 = final_dfg_airlight[2];
	SETREG(defog_airlight_reg_1.reg, reg_addr + DEFOG_AIRLIGHT_REGISTER_1_OFS);

	#if IQ_BUILTIN_DEFOG_DBG_MSG
	DBG_DUMP("*DEFOG frame(%d)*", iq_frm_cnt[id]);
	DBG_DUMP(" ==> mode %d", defog->mode);
	DBG_DUMP(" ==> enable %d", defog->enable);
	DBG_DUMP(" ==> mod_lut %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d"
		, final_fog_mod_lut[0], final_fog_mod_lut[1], final_fog_mod_lut[2], final_fog_mod_lut[3]
		, final_fog_mod_lut[4], final_fog_mod_lut[5], final_fog_mod_lut[6], final_fog_mod_lut[7]
		, final_fog_mod_lut[8], final_fog_mod_lut[9], final_fog_mod_lut[10], final_fog_mod_lut[11]
		, final_fog_mod_lut[12], final_fog_mod_lut[13], final_fog_mod_lut[14], final_fog_mod_lut[15], final_fog_mod_lut[16]);
	DBG_DUMP(" ==> final_fog_ratio %d", final_fog_ratio);
	DBG_DUMP(" ==> final_gain_th %d", final_gain_th);
	DBG_DUMP(" ==> outbld_lum_wt %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d"
		, final_outbld_lum_wt[0], final_outbld_lum_wt[1], final_outbld_lum_wt[2], final_outbld_lum_wt[3]
		, final_outbld_lum_wt[4], final_outbld_lum_wt[5], final_outbld_lum_wt[6], final_outbld_lum_wt[7]
		, final_outbld_lum_wt[8], final_outbld_lum_wt[9], final_outbld_lum_wt[10], final_outbld_lum_wt[11]
		, final_outbld_lum_wt[12], final_outbld_lum_wt[13], final_outbld_lum_wt[14], final_outbld_lum_wt[15], final_outbld_lum_wt[16]);
	DBG_DUMP(" ==> air %d, %d, %d", final_dfg_airlight[0], final_dfg_airlight[1], final_dfg_airlight[2]);
	#endif
}

static UINT32 *iq_builtin_ycurve_conv(UINT32 id, UINT32 *lut)
{
	INT32 i;

	if (ycurve_buffer[id] == NULL) {
		return NULL;
	}

	for (i = 0; i < IQ_YCURVE_LEN; i++) {
		if (i % 2 == 0) {
			ycurve_buffer[id][i / 2] = lut[i];
		} else {
			ycurve_buffer[id][i / 2] |= (lut[i] << 10);
		}
	}

	vos_cpu_dcache_sync((ULONG)ycurve_buffer[id], YCURVE_BUFFER_SIZE, VOS_DMA_TO_DEVICE);

	return ycurve_buffer[id];
}

static void iq_builtin_set_ycurve(UINT32 id, IQ_PARAM_PTR *iq_param)
{
	T_IPE_CONTROL_REGISTER           ipe_control;
	T_IPE_MODE_REGISTER_1            ipe_en_reg;
	T_DMA_TO_IPE_REGISTER_4          ipe_reg_4;

	#if defined(__KERNEL__)
	void __iomem                     *reg_addr = NULL;
	#else
	unsigned int                     reg_addr;
	#endif 
	ULONG                            final_buffer_addr, phy_adr;

	reg_addr = ipe_reg_addr;

	ipe_en_reg.reg = GETREG(reg_addr + IPE_MODE_REGISTER_1_OFS);
	ipe_en_reg.bit.ycurve_en = iq_param->ycurve->enable;
	SETREG(ipe_en_reg.reg, reg_addr + IPE_MODE_REGISTER_1_OFS);

	final_buffer_addr = (ULONG)iq_builtin_ycurve_conv(id, &iq_param->ycurve->ycurve_lut[0]);
	if (final_buffer_addr != 0) {
		//phy_adr = vos_cpu_get_phy_addr((unsigned int)final_buffer_addr);
		phy_adr = ycurve_buffer_pa_addr[id];
		ipe_reg_4.bit.dram_sai_ycurve = phy_adr >> 2;
		SETREG(ipe_reg_4.reg, reg_addr + DMA_TO_IPE_REGISTER_4_OFS);

		ipe_control.reg = GETREG(reg_addr + IPE_CONTROL_REGISTER_OFS);
		ipe_control.bit.ipe_rwgamma = 1;
		ipe_control.bit.ipe_rwgamma_opt = 3;
		SETREG(ipe_control.reg, reg_addr + IPE_CONTROL_REGISTER_OFS);
	}
}

static void iq_builtin_set_color(UINT32 id, UINT32 gain, IQ_PARAM_PTR *iq_param, BOOL ui_night_mode, UINT32 ui_con_ratio, UINT32 ui_sat_ratio)
{
	T_COLOR_COMPONENT_ADJUSTMENT_REGISTER_5                        color_adjust_reg;

	T_COLOR_CORRECTION_STAB_MAPPING_REGISTER_0                     fstab_reg_0;
	T_COLOR_CORRECTION_DTAB_MAPPING_REGISTER_0                     fdtab_reg_0;
	T_COLOR_COMPONENT_ADJUSTMENT_REGISTER_9                        ccontab_reg_0;
	T_COLOR_COMPONENT_ADJUSTMENT_REGISTER_14                       ccontab_reg_e;

	#if defined(__KERNEL__)
	void __iomem                *reg_addr = NULL;
	#else
	unsigned int                reg_addr;
	#endif 
	IQ_COLOR_TUNE_PARAM         *final_color_tune;
	IQ_CST_PARAM                *cst;

	UINT32                      iso_idx = 0;
	UINT8                       con2sat_adj;
	UINT8                       c_con;
	UINT8                       fstab[IQ_COLOR_FSTAB_LEN];
	UINT8                       fdtab[IQ_COLOR_FDTAB_LEN];
	UINT32                      cconlut[IQ_COLOR_CCON_LEN];
	INT32                       i;

	reg_addr = ipe_reg_addr;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->color->mode == IQ_OP_TYPE_AUTO) {
		final_color_tune = &iq_param->color->auto_param[iso_idx];
	} else {
		final_color_tune = &iq_param->color->manual_param;
	}
	cst = iq_param->cst;

	if (iq_param->color->enable == TRUE) {
		if (ui_con_ratio > 100) {
			con2sat_adj = (UINT8)iq_intpl_value(ui_con_ratio, 0, 0, 18);
			c_con = (UINT8)IQ_CLAMP(final_color_tune->c_con + con2sat_adj, 0, 255);
		} else {
			con2sat_adj = (UINT8)iq_intpl_value(ui_con_ratio, 0, 18, 0);
			c_con = (UINT8)IQ_CLAMP(final_color_tune->c_con - con2sat_adj, 0, 255);
		}
		memcpy(fstab, final_color_tune->fstab, sizeof(UINT8) * IQ_COLOR_FSTAB_LEN);
		memcpy(fdtab, final_color_tune->fdtab, sizeof(UINT8) * IQ_COLOR_FDTAB_LEN);
		memcpy(cconlut, final_color_tune->cconlut, sizeof(UINT16) * IQ_COLOR_CCON_LEN);
	} else {
		c_con = 128;
		memset(fstab, 0, sizeof(UINT8) * IQ_COLOR_FSTAB_LEN);
		memset(fdtab, 0, sizeof(UINT8) * IQ_COLOR_FDTAB_LEN);
		for (i = 1; i < IQ_COLOR_CCON_LEN; i++) {
			cconlut[i] = 128;
		}
	}

	c_con = (UINT8)iq_intpl_value(ui_sat_ratio, c_con, 0, 255);

	color_adjust_reg.reg = GETREG(reg_addr + COLOR_COMPONENT_ADJUSTMENT_REGISTER_5_OFS);
	if (ui_night_mode == TRUE) {
		color_adjust_reg.bit.c_cbofs = 128;
		color_adjust_reg.bit.c_crofs = 128;
		color_adjust_reg.bit.c_con = 0;
	} else {
		color_adjust_reg.bit.c_cbofs = cst->cb_ofs;
		color_adjust_reg.bit.c_crofs = cst->cr_ofs;
		color_adjust_reg.bit.c_con = (c_con * iq_front_factor[id].c_con) / 100;
	}
	SETREG(color_adjust_reg.reg, reg_addr + COLOR_COMPONENT_ADJUSTMENT_REGISTER_5_OFS);

	#if IQ_BUILTIN_DBG_MSG
	if (iq_frm_cnt[id] < iq_front_param[id].smooth_thr) {
		DBG_DUMP("id = %d, cnt = %d, c_con = %d\r\n", id, iq_frm_cnt[id], color_adjust_reg.bit.c_con);
	}
	#endif

	for (i = 0; i < (IQ_COLOR_FSTAB_LEN / 4); i++) {
		fstab_reg_0.bit.fstab0 = fstab[4*i+0];
		fstab_reg_0.bit.fstab1 = fstab[4*i+1];
		fstab_reg_0.bit.fstab2 = fstab[4*i+2];
		fstab_reg_0.bit.fstab3 = fstab[4*i+3];
		SETREG(fstab_reg_0.reg, reg_addr + (COLOR_CORRECTION_STAB_MAPPING_REGISTER_0_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_COLOR_FDTAB_LEN / 4); i++) {
		fdtab_reg_0.bit.fdtab0 = fdtab[4*i+0];
		fdtab_reg_0.bit.fdtab1 = fdtab[4*i+1];
		fdtab_reg_0.bit.fdtab2 = fdtab[4*i+2];
		fdtab_reg_0.bit.fdtab3 = fdtab[4*i+3];
		SETREG(fdtab_reg_0.reg, reg_addr + COLOR_CORRECTION_DTAB_MAPPING_REGISTER_0_OFS + (i * 4));
	}

	for (i = 0; i < (IQ_COLOR_CCON_LEN / 3); i++) {
		ccontab_reg_0.bit.ccontab0 = cconlut[3*i+0];
		ccontab_reg_0.bit.ccontab1 = cconlut[3*i+1];
		ccontab_reg_0.bit.ccontab2 = cconlut[3*i+2];
		SETREG(ccontab_reg_0.reg, reg_addr + ((COLOR_COMPONENT_ADJUSTMENT_REGISTER_9_OFS + (i * 4))));
	}

	ccontab_reg_e.bit.ccontab15 = cconlut[15];
	ccontab_reg_e.bit.ccontab16 = cconlut[16];
	ccontab_reg_e.bit.ccontab_sel = 0;
	SETREG(ccontab_reg_e.reg, reg_addr + COLOR_COMPONENT_ADJUSTMENT_REGISTER_14_OFS);
}

static void iq_builtin_set_contrast(UINT32 id, ISP_BUILTIN_IQ_TRIG_MSG msg, UINT32 gain, IQ_PARAM_PTR *iq_param, UINT32 ui_bright_ratio, UINT32 ui_con_ratio)
{
	T_COLOR_COMPONENT_ADJUSTMENT_REGISTER_1                     ycon_reg;
	T_LCE_REGISTER1                                             lce_reg_0;
	T_LCE_REGISTER3                                             lce_reg_e;
	T_IPE_MODE_REGISTER_1                                       ipe_mode_reg;
	T_COLOR_CONTROL_REGISTER                                    int_reg;

	#if defined(__KERNEL__)
	void __iomem                                                *reg_addr = NULL;
	#else
	unsigned int                                                reg_addr;
	#endif 
	IQ_CONTRAST_TUNE_PARAM                                      *final_contrast_tune;
	IQ_CST_PARAM                                                *cst;
	UINT32                                                      iso_idx = 0;
	UINT8                                                       lce_lum_wt_lut[IQ_CONTRAST_LCE_LEN];
	INT16                                                       con2int;
	INT16                                                       int_ofs;
	UINT8                                                       y_con;
	INT32                                                       i;

	reg_addr = ipe_reg_addr;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->contrast->mode == IQ_OP_TYPE_AUTO) {
		final_contrast_tune = &iq_param->contrast->auto_param[iso_idx];
	} else {
		final_contrast_tune = &iq_param->contrast->manual_param;
	}
	cst = iq_param->cst;

	y_con = (UINT8)iq_intpl_value(ui_con_ratio, final_contrast_tune->y_con, 0, 255);

	ycon_reg.reg = GETREG(reg_addr + COLOR_COMPONENT_ADJUSTMENT_REGISTER_1_OFS);
	ycon_reg.bit.y_con = y_con;
	SETREG(ycon_reg.reg, reg_addr + COLOR_COMPONENT_ADJUSTMENT_REGISTER_1_OFS);

	int_ofs = (INT16)iq_intpl_value(ui_bright_ratio, cst->y_ofs, -128, 127);
	con2int = 128 - y_con;

	int_reg.reg = GETREG(reg_addr + COLOR_CONTROL_REGISTER_OFS);
	int_reg.bit.int_ofs = (INT16)IQ_CLAMP(int_ofs + con2int, -128, 127);
	SETREG(int_reg.reg, reg_addr + COLOR_CONTROL_REGISTER_OFS);

	ipe_mode_reg.reg = GETREG(reg_addr + IPE_MODE_REGISTER_1_OFS);
	if (msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) {
		#if defined(__KERNEL__)
		if (isp_builtin_get_fastboot_version() == 2) {
			ipe_mode_reg.bit.lce_en = iq_param->contrast->lce_enable;
		} else {
			ipe_mode_reg.bit.lce_en = FALSE;
		}
		#else
		ipe_mode_reg.bit.lce_en = FALSE;
		#endif
	} else {
		ipe_mode_reg.bit.lce_en = iq_param->contrast->lce_enable;
	}
	SETREG(ipe_mode_reg.reg, reg_addr + IPE_MODE_REGISTER_1_OFS);

	if (iq_frm_cnt[id] < IQ_LCE_STABLE_CNT) {
		for (i = 0; i < IQ_CONTRAST_LCE_LEN; i++) {
			lce_lum_wt_lut[i] = (UINT32)iq_cal(iq_frm_cnt[id] * 100 / IQ_LCE_STABLE_CNT, final_contrast_tune->lce_lum_wt_lut[i], 0, 191);
		}
		#if defined(__KERNEL__)
		if (isp_builtin_get_fastboot_version() == 2) {
			memcpy(lce_lum_wt_lut, final_contrast_tune->lce_lum_wt_lut, sizeof(UINT8) * IQ_CONTRAST_LCE_LEN);
		}
		#endif
	} else {
		memcpy(lce_lum_wt_lut, final_contrast_tune->lce_lum_wt_lut, sizeof(UINT8) * IQ_CONTRAST_LCE_LEN);
	}

	for (i = 0; i < (IQ_CONTRAST_LCE_LEN / 4); i++) {
		lce_reg_0.bit.lce_lum_adj_lut0 = IQ_CLAMP((lce_lum_wt_lut[4*i+0] + 64), 0, 255);
		lce_reg_0.bit.lce_lum_adj_lut1 = IQ_CLAMP((lce_lum_wt_lut[4*i+1] + 64), 0, 255);
		lce_reg_0.bit.lce_lum_adj_lut2 = IQ_CLAMP((lce_lum_wt_lut[4*i+2] + 64), 0, 255);
		lce_reg_0.bit.lce_lum_adj_lut3 = IQ_CLAMP((lce_lum_wt_lut[4*i+3] + 64), 0, 255);
		SETREG(lce_reg_0.reg, reg_addr + (LCE_REGISTER1_OFS + (i * 4)));
	}

	lce_reg_e.bit.lce_lum_adj_lut8 = IQ_CLAMP((lce_lum_wt_lut[8] + 64), 0, 255);;
	SETREG(lce_reg_e.reg, reg_addr + LCE_REGISTER3_OFS);

	#if IQ_BUILTIN_DBG_MSG
	if (iq_frm_cnt[id] < IQ_LCE_STABLE_CNT) {
		DBG_DUMP("id = %d, cnt = %d, lce_lum_wt_lut = {%d, %d, %d, %d, %d, %d, %d, %d, %d}\r\n", id, iq_frm_cnt[id],
			lce_lum_wt_lut[0], lce_lum_wt_lut[1], lce_lum_wt_lut[2], lce_lum_wt_lut[3],
			lce_lum_wt_lut[4], lce_lum_wt_lut[5], lce_lum_wt_lut[6], lce_lum_wt_lut[7],
			lce_lum_wt_lut[8]);
	}
	#endif
}

static void iq_builtin_set_edge(UINT32 id, UINT32 gain, IQ_PARAM_PTR *iq_param, UINT32 ui_sharp_ratio, ISP_BUILTIN_AE_STATUS ae_status)
{
	T_IPE_MODE_REGISTER_1                         ipe_en_reg;
	T_COLOR_CONTROL_EDGE_MAPPING_REGISTER_0       color_edge_lut_reg_0;

	T_COLOR_COMPONENT_ADJUSTMENT_REGISTER_0       edge_str_reg;

	T_EDGE_REGION_EXTRACTION_REGISTER_0           edge_w_reg;
	T_EDGE_REGION_EXTRACTION_REGISTER_1           edge_w_ker_reg;
	T_EDGE_REGION_EXTRACTION_REGISTER_2           edge_w_hld_reg;
	T_EDGE_REGION_EXTRACTION_REGISTER_3           edge_th_reg;
	T_EDGE_REGION_EXTRACTION_REGISTER_4           edge_th_hld_reg;
	T_EDGE_REGION_EXTRACTION_REGISTER_5           edge_w_slope_reg;
	T_EDGE_EXTRACTION_REGISTER_9                  edge_dir_blend_w_reg;

	T_EDGE_REGION_STRENGTH_REGISTER2              edge_str_slope_reg;
	T_EDGE_REGION_STRENGTH_REGISTER3              edge_region_str_reg;

	T_EDGE_LUMINANCE_PROCESS_REGISTER_0           edge_estab_reg;
	T_EDGE_DMAP_PROCESS_REGISTER_0                edge_edtab_reg;
	T_EDGE_LUMINANCE_PROCESS_REGISTER_1           edge_eslut_reg_0;
	T_EDGE_DMAP_PROCESS_REGISTER_1                edge_edlut_reg_0;

	T_OVERSHOOTING_CONTROL_REGISTER_1             edge_overshoot_th_reg;
	T_OVERSHOOTING_CONTROL_REGISTER_3             edge_overshoot_slope_reg;

	#if defined(__KERNEL__)
	void __iomem                                  *reg_addr = NULL;
	#else
	unsigned int                                  reg_addr;
	#endif 
	IQ_EDGE_PARAM                                 *edge;
	IQ_EDGE_TUNE_PARAM                            *final_edge_tune;

	UINT32                                        iso_idx = 0;
	UINT32                                        edge_enh_p;
	UINT32                                        edge_enh_n;
	INT32                                         curr_slope_flat, curr_slope_edge;
	UINT32                                        slope_con_eng, slope_con_eng_hld, wlow, whigh, th_edge, th_flat;
	INT32                                         i;

	reg_addr = ipe_reg_addr;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	edge = iq_param->edge;
	if (edge->mode == IQ_OP_TYPE_AUTO) {
		final_edge_tune = &iq_param->edge->auto_param[iso_idx];
	} else {
		final_edge_tune = &iq_param->edge->manual_param;
	}

	edge_enh_p = (UINT32)iq_cal(ui_sharp_ratio, final_edge_tune->edge_enh_p, 0, 1023);
	edge_enh_n = (UINT32)iq_cal(ui_sharp_ratio, final_edge_tune->edge_enh_n, 0, 1023);
	if (ae_status == ISP_BUILTIN_AE_STATUS_COARSE) {
		edge_enh_p = (edge_enh_p * iq_param->edge->reduce_ratio) >> 7;
		edge_enh_n = (edge_enh_n * iq_param->edge->reduce_ratio) >> 7;
	}

	ipe_en_reg.reg = GETREG(reg_addr + IPE_MODE_REGISTER_1_OFS);
	ipe_en_reg.bit.cadj_yenh_en = edge->enable;
	SETREG(ipe_en_reg.reg, reg_addr + IPE_MODE_REGISTER_1_OFS);

	edge_str_reg.reg = GETREG(reg_addr + COLOR_COMPONENT_ADJUSTMENT_REGISTER_0_OFS);
	edge_str_reg.bit.y_enh_p = (edge_enh_p * iq_front_factor[id].overshoot) / 100;
	edge_str_reg.bit.y_enh_n = (edge_enh_n * iq_front_factor[id].overshoot) / 100;
	SETREG(edge_str_reg.reg, reg_addr + COLOR_COMPONENT_ADJUSTMENT_REGISTER_0_OFS);

	#if IQ_BUILTIN_DBG_MSG
	if (iq_frm_cnt[id] < iq_front_param[id].smooth_thr) {
		DBG_DUMP("id = %d, cnt = %d, y_enh_p = %d, y_enh_n = %d\r\n", id, iq_frm_cnt[id], edge_str_reg.bit.reg_y_enh_p, edge_str_reg.bit.reg_y_enh_n);
	}
	#endif

	edge_w_reg.reg = GETREG(reg_addr + EDGE_REGION_EXTRACTION_REGISTER_0_OFS);
	edge_w_reg.bit.w_low = final_edge_tune->wt_low;
	edge_w_reg.bit.w_high = final_edge_tune->wt_high;
	SETREG(edge_w_reg.reg, reg_addr + EDGE_REGION_EXTRACTION_REGISTER_0_OFS);

	edge_w_ker_reg.bit.w_ker_thin = iq_edge_ker_tab[final_edge_tune->thin_freq][0];
	edge_w_ker_reg.bit.w_ker_robust = iq_edge_ker_tab[final_edge_tune->robust_freq][0];
	edge_w_ker_reg.bit.iso_ker_thin = iq_edge_ker_tab[final_edge_tune->thin_freq][1];
	edge_w_ker_reg.bit.iso_ker_robust = iq_edge_ker_tab[final_edge_tune->robust_freq][1];
	SETREG(edge_w_ker_reg.reg, reg_addr + EDGE_REGION_EXTRACTION_REGISTER_1_OFS);

	edge_w_hld_reg.bit.w_hld_low = final_edge_tune->wt_low;
	edge_w_hld_reg.bit.w_hld_high = final_edge_tune->wt_high;
	edge_w_hld_reg.bit.w_ker_thin_hld = iq_edge_ker_tab[IQ_CLAMP(final_edge_tune->thin_freq + 2, 0, 16)][0];;
	edge_w_hld_reg.bit.w_ker_robust_hld = iq_edge_ker_tab[final_edge_tune->robust_freq][0];
	edge_w_hld_reg.bit.iso_ker_thin_hld = iq_edge_ker_tab[IQ_CLAMP(final_edge_tune->thin_freq + 2, 0, 16)][1];
	edge_w_hld_reg.bit.iso_ker_robust_hld = iq_edge_ker_tab[final_edge_tune->robust_freq][1];
	SETREG(edge_w_hld_reg.reg, reg_addr + EDGE_REGION_EXTRACTION_REGISTER_2_OFS);

	edge_th_reg.bit.th_flat = final_edge_tune->th_flat_high;
	edge_th_reg.bit.th_edge = final_edge_tune->th_edge_low;
	SETREG(edge_th_reg.reg, reg_addr + EDGE_REGION_EXTRACTION_REGISTER_3_OFS);

	edge_th_hld_reg.reg = GETREG(reg_addr + EDGE_REGION_EXTRACTION_REGISTER_4_OFS);
	edge_th_hld_reg.bit.th_hld_flat = (final_edge_tune->th_flat_high * 200) >> 8;
	edge_th_hld_reg.bit.th_hld_edge = final_edge_tune->th_edge_low;
	SETREG(edge_th_hld_reg.reg, reg_addr + EDGE_REGION_EXTRACTION_REGISTER_4_OFS);

	curr_slope_flat = (final_edge_tune->th_flat_high == final_edge_tune->th_flat_low) ? 0x7fff : ((64 - (INT32)final_edge_tune->str_flat) << 10) / ((INT32)final_edge_tune->th_flat_high - (INT32)final_edge_tune->th_flat_low);
	if (curr_slope_flat > 0x7fff) {
		curr_slope_flat = 0x7fff;
	}
	edge_str_slope_reg.bit.slope_flat = 0 - (INT16)curr_slope_flat;
	curr_slope_edge = (final_edge_tune->th_edge_high == final_edge_tune->th_edge_low) ? 0x7fff : (((INT32)final_edge_tune->str_edge - 64) << 10) / ((INT32)final_edge_tune->th_edge_high - (INT32)final_edge_tune->th_edge_low);
	if (curr_slope_edge > 0x7fff) {
		curr_slope_edge = 0x7fff;
	}
	edge_str_slope_reg.bit.slope_edge = (INT16)curr_slope_edge;
	SETREG(edge_str_slope_reg.reg, reg_addr + EDGE_REGION_STRENGTH_REGISTER2_OFS);

	edge_region_str_reg.bit.str_flat = final_edge_tune->str_flat;
	edge_region_str_reg.bit.str_edge = final_edge_tune->str_edge;
	SETREG(edge_region_str_reg.reg, reg_addr + EDGE_REGION_STRENGTH_REGISTER3_OFS);

	wlow = final_edge_tune->wt_low;
	whigh = final_edge_tune->wt_high;
	th_edge = final_edge_tune->th_edge_low;
	th_flat = final_edge_tune->th_flat_high;
	slope_con_eng = (th_edge == th_flat) ? 65535 : (((whigh -  wlow) * 1024) / abs(th_edge - th_flat));

	wlow = final_edge_tune->wt_low;
	whigh = final_edge_tune->wt_high;
	th_edge = final_edge_tune->th_edge_low;
	th_flat = (final_edge_tune->th_flat_high * 200) >> 8;
	slope_con_eng_hld = (th_edge == th_flat) ? 65535 : (((whigh -  wlow) * 1024) / abs(th_edge - th_flat));

	edge_w_slope_reg.bit.slope_con_eng = slope_con_eng;
	edge_w_slope_reg.bit.slope_hld_con_eng = slope_con_eng_hld;
	SETREG(edge_w_slope_reg.reg, reg_addr + EDGE_REGION_EXTRACTION_REGISTER_5_OFS);

	edge_dir_blend_w_reg.reg = GETREG(reg_addr + EDGE_EXTRACTION_REGISTER_9_OFS);
	edge_dir_blend_w_reg.bit.dir_eng_blend_w1 = iq_edge_dir_w_tab[IQ_CLAMP(final_edge_tune->dir_eng_blend_w, 0, 16)][0];
	edge_dir_blend_w_reg.bit.dir_eng_blend_w2 = iq_edge_dir_w_tab[IQ_CLAMP(final_edge_tune->dir_eng_blend_w, 0, 16)][1];
	edge_dir_blend_w_reg.bit.dir_eng_blend_w3 = iq_edge_dir_w_tab[IQ_CLAMP(final_edge_tune->dir_eng_blend_w, 0, 16)][2];
	edge_dir_blend_w_reg.bit.dir_eng_blend_w4 = iq_edge_dir_w_tab[IQ_CLAMP(final_edge_tune->dir_eng_blend_w, 0, 16)][3];
	SETREG(edge_dir_blend_w_reg.reg, reg_addr + EDGE_EXTRACTION_REGISTER_9_OFS);

	// EStab & EDtab
	edge_estab_reg.bit.esthrl = final_edge_tune->es_ethr_low;
	edge_estab_reg.bit.esthrh = final_edge_tune->es_ethr_high;
	edge_estab_reg.bit.establ = final_edge_tune->es_etab_low;
	edge_estab_reg.bit.estabh = final_edge_tune->es_etab_high;
	SETREG(edge_estab_reg.reg, reg_addr + EDGE_LUMINANCE_PROCESS_REGISTER_0_OFS);

	edge_edtab_reg.bit.edthrl = final_edge_tune->edge_ethr_low;
	edge_edtab_reg.bit.edthrh = final_edge_tune->edge_ethr_high;
	edge_edtab_reg.bit.edtabl = final_edge_tune->edge_etab_low;
	edge_edtab_reg.bit.edtabh = final_edge_tune->edge_etab_high;
	edge_edtab_reg.bit.edinsel = 1;
	SETREG(edge_edtab_reg.reg, reg_addr + EDGE_DMAP_PROCESS_REGISTER_0_OFS);

	for (i = 0; i < (IQ_EDGE_LUT_LEN / 4); i++) {
		edge_eslut_reg_0.bit.eslutl0 = edge->es_map_lut[4*i+0];
		edge_eslut_reg_0.bit.eslutl1 = edge->es_map_lut[4*i+1];
		edge_eslut_reg_0.bit.eslutl2 = edge->es_map_lut[4*i+2];
		edge_eslut_reg_0.bit.eslutl3 = edge->es_map_lut[4*i+3];
		SETREG(edge_eslut_reg_0.reg, reg_addr + (EDGE_LUMINANCE_PROCESS_REGISTER_1_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_EDGE_LUT_LEN / 4); i++) {
		edge_edlut_reg_0.bit.edlutl0 = edge->edge_map_lut[4*i+0];
		edge_edlut_reg_0.bit.edlutl1 = edge->edge_map_lut[4*i+1];
		edge_edlut_reg_0.bit.edlutl2 = edge->edge_map_lut[4*i+2];
		edge_edlut_reg_0.bit.edlutl3 = edge->edge_map_lut[4*i+3];
		SETREG(edge_edlut_reg_0.reg, reg_addr + (EDGE_DMAP_PROCESS_REGISTER_1_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_EDGE_HUE_NUM / 4); i++) {
		color_edge_lut_reg_0.bit.cedgm0 = edge->edge_tab[4*i+0];
		color_edge_lut_reg_0.bit.cedgm1 = edge->edge_tab[4*i+1];
		color_edge_lut_reg_0.bit.cedgm2 = edge->edge_tab[4*i+2];
		color_edge_lut_reg_0.bit.cedgm3 = edge->edge_tab[4*i+3];
		SETREG(color_edge_lut_reg_0.reg, reg_addr + (COLOR_CONTROL_EDGE_MAPPING_REGISTER_0_OFS + (i * 4)));
	}

	// overshoot
	edge_overshoot_th_reg.reg = GETREG(reg_addr + OVERSHOOTING_CONTROL_REGISTER_1_OFS);
	edge_overshoot_th_reg.bit.th_overshoot = edge->th_overshoot;
	edge_overshoot_th_reg.bit.th_undershoot = edge->th_undershoot;
	SETREG(edge_overshoot_th_reg.reg, reg_addr + OVERSHOOTING_CONTROL_REGISTER_1_OFS);

	edge_overshoot_slope_reg.bit.slope_overshoot = final_edge_tune->overshoot_str;
	edge_overshoot_slope_reg.bit.slope_undershoot = final_edge_tune->undershoot_str;
	SETREG(edge_overshoot_slope_reg.reg, reg_addr + OVERSHOOTING_CONTROL_REGISTER_3_OFS);
}

static void iq_builtin_set_3dnr(UINT32 id, ISP_BUILTIN_IQ_TRIG_MSG msg, UINT32 gain, IQ_PARAM_PTR *iq_param, BOOL ui_night_mode, UINT32 ui_3dnr_ratio)
{
	T_IME_FUNCTION_CONTROL_REGISTER0                           ime_function_control_reg0;
	T_IME_TMNR_CONTROL_REGISTER1                               ime_3dnr_control_reg_1;

	T_IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER0             ime_3dnr_sad_penalty_0;
	T_IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER1             ime_3dnr_sad_penalty_1;
	T_IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER2             ime_3dnr_sad_penalty_2;
	T_IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER3             ime_3dnr_switch_th_0;
	T_IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER5             ime_3dnr_me_control_reg5;
	T_IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER6             ime_3dnr_detail_penalty;
	T_IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER7             ime_3dnr_probability;
	T_IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER8             ime_3dnr_pf_str;

	T_IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER0              ime_3dnr_md_sad_coeffa_0;
	T_IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER2              ime_3dnr_md_sad_coeffb_0;
	T_IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER6              ime_3dnr_md_sad_std_0;
	T_IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER10             ime_3dnr_md_k;

	T_IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER0           ime_3dnr_sad_base_0;
	T_IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER4           ime_3dnr_mc_sad_coeffa_0;
	T_IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER6           ime_3dnr_mc_sad_coeffb_0;
	T_IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER10          ime_3dnr_mc_sad_std_0;
	T_IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER14          ime_3dnr_mc_k;

	T_IME_3DNR_TF0_REGISTER0                                   ime_3dnr_tf0_reg0;
	T_IME_3DNR_TF0_REGISTER1                                   ime_3dnr_tf0_reg1;
	T_IME_3DNR_TF0_REGISTER2                                   ime_3dnr_tf0_reg2;
	T_IME_3DNR_PS_CONTROL_REGISTER2                            ime_3dnr_tf0_uv_reg;

	T_IME_3DNR_PS_CONTROL_REGISTER0                            ime_3dnr_mv_th;
	T_IME_3DNR_PS_CONTROL_REGISTER1                            ime_3dnr_ps_mix_ratio;
	T_IME_3DNR_PS_CONTROL_REGISTER4                            ime_3dnr_ps_ds_th;

	T_IME_3DNR_NR_CONTROL_REGISTER0                            ime_3dnr_residue;
	T_IME_3DNR_NR_CONTROL_REGISTER1                            ime_3dnr_freq_w;
	T_IME_3DNR_NR_CONTROL_REGISTER3                            ime_3dnr_luma_w_0;
	T_IME_3DNR_NR_CONTROL_REGISTER6                            ime_3dnr_pre_filter_str;
	T_IME_3DNR_NR_CONTROL_REGISTER8                            ime_3dnr_pre_filter_ratio;
	T_IME_3DNR_NR_CONTROL_REGISTER9                            ime_3dnr_snr_tnr_str;
	T_IME_3DNR_NR_CONTROL_REGISTER10                           ime_3dnr_nr_base_th;
	T_IME_3DNR_NR_CONTROL_REGISTER11                           ime_3dnr_luma_3d_rto;
	T_IME_3DNR_NR_CONTROL_REGISTER12                           ime_3dnr_luma_3d_lut_0;
	T_IME_3DNR_NR_CONTROL_REGISTER14                           ime_3dnr_chroma_3d_lut_0;
	T_IME_3DNR_NR_CONTROL_REGISTER16                           ime_3dnr_chroma_3d_ratio;

	T_IME_3DNR_FAST_CONVERGE_CONTROL_REGISTER0                 ime_3dnr_fast_converge;

	T_IME_3DNR_PS_CONTROL_REGISTER3                            ime_3dnr_motion_sat_reg;
	T_IME_3DNR_COLOR_DITHERING_REGISTER0                       ime_3dnr_cshk_th_reg_0;
	T_IME_3DNR_COLOR_DITHERING_REGISTER4                       ime_3dnr_cshk_val_reg;

	#if defined(__KERNEL__)
	void __iomem                    *reg_addr = NULL;
	#else
	unsigned int                    reg_addr;
	#endif 
	IQ_3DNR_TUNE_PARAM              *final_3dnr_tune;

	ISP_BUILTIN_FUNC                func_en = 0;
	UINT32                          iso_idx = 0;
	UINT32                          cur_probability;
	static UINT32                   probability_auto_shift[IQ_ID_MAX_NUM];
	UINT32                          probability[TMNR_PROBABILITY_LEN]= {0}, final_probability[TMNR_PROBABILITY_LEN]= {0};
	UINT32                          pre_filter_str[IQ_3DNR_FREQ_NUM];
	UINT32                          tnr_base_th;
	UINT32                          luma_3d_lut[IQ_3DNR_F4_LUT_LEN];
	UINT32                          chroma_3d_lut[IQ_3DNR_F4_LUT_LEN];
	UINT32                          y_residue_th[IQ_3DNR_OBJECT_NUM];
	UINT32                          scene_change_w = 0;
	static UINT32                   chroma_3d_lut_linear[8] = {0, 16, 32, 48, 64, 80, 96, 112};
	static UINT32                   count[IQ_ID_MAX_NUM];
	INT32                           i;

	reg_addr = ime_reg_addr;

	func_en = isp_builtin_get_func_en(id) & ISP_BUILTIN_FUNC_3DNR;
	if (func_en == FALSE) {
		if ((msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) && (func_en == 0) && (iq_param->_3dnr->enable == TRUE)) {
			DBG_DUMP("[builtin IQ] 3dnr_enable = 0 in nvt-fastboot-ipp.dtsi, please check !! \r\n");
		}
		ime_function_control_reg0.reg = GETREG(reg_addr + IME_FUNCTION_CONTROL_REGISTER0_OFS);
		ime_function_control_reg0.bit.ime_3dnr_en = FALSE;
		SETREG(ime_function_control_reg0.reg, reg_addr + IME_FUNCTION_CONTROL_REGISTER0_OFS);

		return;
	}

	if(isp_builtin_get_night_mode(id) == 1) {
		scene_change_w = 100;
	} else {
		scene_change_w = isp_builtin_get_scene_chg_w(id);
	}

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->_3dnr->mode == IQ_OP_TYPE_AUTO) {
		final_3dnr_tune = &iq_param->_3dnr->auto_param[iso_idx];
	} else {
		final_3dnr_tune = &iq_param->_3dnr->manual_param;
	}

	for (i = 0; i < IQ_3DNR_FREQ_NUM; i++) {
		pre_filter_str[i] = iq_cal(ui_3dnr_ratio, final_3dnr_tune->pre_filter_str[i], 0, 255);
	}
	tnr_base_th = iq_cal(ui_3dnr_ratio, final_3dnr_tune->tnr_base_th, 0, 65535);
	for (i = 0; i < IQ_3DNR_F4_LUT_LEN; i++) {
		luma_3d_lut[i] = iq_cal(200 - ui_3dnr_ratio, final_3dnr_tune->luma_3d_lut[i], 0, 16 * i);
		chroma_3d_lut[i] = iq_cal(200 - ui_3dnr_ratio, final_3dnr_tune->chroma_3d_lut[i], 0, 16 * i);
	}
	if (ui_night_mode == TRUE) {
		for (i = 0; i < IQ_3DNR_F4_LUT_LEN; i++) {
			chroma_3d_lut[i] = chroma_3d_lut_linear[i];
		}
	}

	ime_function_control_reg0.reg = GETREG(reg_addr + IME_FUNCTION_CONTROL_REGISTER0_OFS);
	if (msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) {
		#if defined(__KERNEL__)
		if (isp_builtin_get_fastboot_version() == 2) {
			ime_function_control_reg0.bit.ime_3dnr_en = iq_param->_3dnr->enable;
		} else {
			ime_function_control_reg0.bit.ime_3dnr_en = FALSE;
		}
		#else
		ime_function_control_reg0.bit.ime_3dnr_en = FALSE;
		#endif
	} else {
		ime_function_control_reg0.bit.ime_3dnr_en = iq_param->_3dnr->enable;
	}
	SETREG(ime_function_control_reg0.reg, reg_addr + IME_FUNCTION_CONTROL_REGISTER0_OFS);

	ime_3dnr_control_reg_1.reg = GETREG(reg_addr + IME_TMNR_CONTROL_REGISTER1_OFS);
	ime_3dnr_control_reg_1.bit.ime_3dnr_dbg_mode = 0;
	if (ime_3dnr_control_reg_1.bit.ime_3dnr_ps_fastc_en == 0) {
		if ((msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) && (iq_param->_3dnr->fcvg_enable == TRUE)) {
			DBG_DUMP("[builtin IQ] fcvg_enable = 0 in nvt-fastboot-ipp.dtsi, please check !! \r\n");
		}
	} else {
		ime_3dnr_control_reg_1.bit.ime_3dnr_ps_fastc_en = iq_param->_3dnr->fcvg_enable;
	}
	SETREG(ime_3dnr_control_reg_1.reg, reg_addr + IME_TMNR_CONTROL_REGISTER1_OFS);

	// MEMC
	ime_3dnr_pf_str.bit.ime_3dnr_pf_str = final_3dnr_tune->pf_str;
	SETREG(ime_3dnr_pf_str.reg, reg_addr + IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER8_OFS);

	ime_3dnr_sad_penalty_0.bit.ime_3dnr_me_sad_penalty_0 = iq_builtin_intpl(scene_change_w, (INT32)1023, (INT32)final_3dnr_tune->sad_penalty[0], 0, 100);
	ime_3dnr_sad_penalty_0.bit.ime_3dnr_me_sad_penalty_1 = iq_builtin_intpl(scene_change_w, (INT32)1023, (INT32)final_3dnr_tune->sad_penalty[1], 0, 100);
	ime_3dnr_sad_penalty_0.bit.ime_3dnr_me_sad_penalty_2 = iq_builtin_intpl(scene_change_w, (INT32)0, (INT32)final_3dnr_tune->sad_penalty[2], 0, 100);
	SETREG(ime_3dnr_sad_penalty_0.reg, reg_addr + IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER0_OFS);

	ime_3dnr_sad_penalty_1.bit.ime_3dnr_me_sad_penalty_3 = iq_builtin_intpl(scene_change_w, (INT32)1023, (INT32)final_3dnr_tune->sad_penalty[3], 0, 100);
	ime_3dnr_sad_penalty_1.bit.ime_3dnr_me_sad_penalty_4 = iq_builtin_intpl(scene_change_w, (INT32)1023, (INT32)final_3dnr_tune->sad_penalty[4], 0, 100);
	ime_3dnr_sad_penalty_1.bit.ime_3dnr_me_sad_penalty_5 = iq_builtin_intpl(scene_change_w, (INT32)1023, (INT32)final_3dnr_tune->sad_penalty[5], 0, 100);
	SETREG(ime_3dnr_sad_penalty_1.reg, reg_addr + IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER1_OFS);

	ime_3dnr_sad_penalty_2.bit.ime_3dnr_me_sad_penalty_6 = iq_builtin_intpl(scene_change_w, (INT32)1023, (INT32)final_3dnr_tune->sad_penalty[6], 0, 100);
	ime_3dnr_sad_penalty_2.bit.ime_3dnr_me_sad_penalty_7 = iq_builtin_intpl(scene_change_w, (INT32)1023, (INT32)final_3dnr_tune->sad_penalty[7], 0, 100);
	SETREG(ime_3dnr_sad_penalty_2.reg, reg_addr + IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER2_OFS);

	#if IQ_BUILTIN_DBG_MSG
	if (iq_frm_cnt[id] < iq_front_param[id].smooth_thr) {
		DBG_DUMP("id = %d, cnt = %d, w = %d, sad_penalty = {%d, %d, %d, %d, %d, %d, %d, %d}\r\n",id, iq_frm_cnt[id], scene_change_w,
			ime_3dnr_sad_penalty_0.bit.ime_3dnr_me_sad_penalty_0, ime_3dnr_sad_penalty_0.bit.ime_3dnr_me_sad_penalty_1,
			ime_3dnr_sad_penalty_0.bit.ime_3dnr_me_sad_penalty_2, ime_3dnr_sad_penalty_1.bit.ime_3dnr_me_sad_penalty_3,
			ime_3dnr_sad_penalty_1.bit.ime_3dnr_me_sad_penalty_4, ime_3dnr_sad_penalty_1.bit.ime_3dnr_me_sad_penalty_5,
			ime_3dnr_sad_penalty_2.bit.ime_3dnr_me_sad_penalty_6, ime_3dnr_sad_penalty_2.bit.ime_3dnr_me_sad_penalty_7);
	}
	#endif

	ime_3dnr_detail_penalty.bit.ime_3dnr_me_detail_penalty0 = iq_builtin_intpl(scene_change_w, (INT32)15, (INT32)final_3dnr_tune->detail_penalty[0], 0, 100);
	ime_3dnr_detail_penalty.bit.ime_3dnr_me_detail_penalty1 = iq_builtin_intpl(scene_change_w, (INT32)15, (INT32)final_3dnr_tune->detail_penalty[1], 0, 100);
	ime_3dnr_detail_penalty.bit.ime_3dnr_me_detail_penalty2 = iq_builtin_intpl(scene_change_w, (INT32)0, (INT32)final_3dnr_tune->detail_penalty[2], 0, 100);
	ime_3dnr_detail_penalty.bit.ime_3dnr_me_detail_penalty3 = iq_builtin_intpl(scene_change_w, (INT32)15, (INT32)final_3dnr_tune->detail_penalty[3], 0, 100);
	ime_3dnr_detail_penalty.bit.ime_3dnr_me_detail_penalty4 = iq_builtin_intpl(scene_change_w, (INT32)15, (INT32)final_3dnr_tune->detail_penalty[4], 0, 100);
	ime_3dnr_detail_penalty.bit.ime_3dnr_me_detail_penalty5 = iq_builtin_intpl(scene_change_w, (INT32)15, (INT32)final_3dnr_tune->detail_penalty[5], 0, 100);
	ime_3dnr_detail_penalty.bit.ime_3dnr_me_detail_penalty6 = iq_builtin_intpl(scene_change_w, (INT32)15, (INT32)final_3dnr_tune->detail_penalty[6], 0, 100);
	ime_3dnr_detail_penalty.bit.ime_3dnr_me_detail_penalty7 = iq_builtin_intpl(scene_change_w, (INT32)15, (INT32)final_3dnr_tune->detail_penalty[7], 0, 100);
	SETREG(ime_3dnr_detail_penalty.reg, reg_addr + IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER6_OFS);

	for (i = 0; i < (IQ_3DNR_SWITCH_LEN / 4); i++) {
		ime_3dnr_switch_th_0.bit.ime_3dnr_me_switch_th0 = final_3dnr_tune->switch_th[4*i+0];
		ime_3dnr_switch_th_0.bit.ime_3dnr_me_switch_th1 = final_3dnr_tune->switch_th[4*i+1];
		ime_3dnr_switch_th_0.bit.ime_3dnr_me_switch_th2 = final_3dnr_tune->switch_th[4*i+2];
		ime_3dnr_switch_th_0.bit.ime_3dnr_me_switch_th3 = final_3dnr_tune->switch_th[4*i+3];
		SETREG(ime_3dnr_switch_th_0.reg, reg_addr + (IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER3_OFS + (i * 4)));
	}

	ime_3dnr_me_control_reg5.bit.ime_3dnr_me_switch_ratio = final_3dnr_tune->switch_rto;
	ime_3dnr_me_control_reg5.bit.ime_3dnr_me_cost_blend = final_3dnr_tune->cost_blend;
	SETREG(ime_3dnr_me_control_reg5.reg, reg_addr + IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER5_OFS);

	ime_3dnr_probability.reg = GETREG(reg_addr + IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER7_OFS);
	cur_probability = final_3dnr_tune->probability;
	for (i = cur_probability; i > 0; i--) {
		probability[i - 1] = 1;
		cur_probability--;
	}
	for (i = 0; i < TMNR_PROBABILITY_LEN; i++) {
		final_probability[i] = probability[(i + probability_auto_shift[id]) % TMNR_PROBABILITY_LEN];
	}
	probability_auto_shift[id]++;
	ime_3dnr_probability.bit.ime_3dnr_me_probability0 = final_probability[0];
	ime_3dnr_probability.bit.ime_3dnr_me_probability1 = final_probability[1];
	ime_3dnr_probability.bit.ime_3dnr_me_probability2 = final_probability[2];
	ime_3dnr_probability.bit.ime_3dnr_me_probability3 = final_probability[3];
	ime_3dnr_probability.bit.ime_3dnr_me_probability4 = final_probability[4];
	ime_3dnr_probability.bit.ime_3dnr_me_probability5 = final_probability[5];
	ime_3dnr_probability.bit.ime_3dnr_me_probability6 = final_probability[6];
	ime_3dnr_probability.bit.ime_3dnr_me_probability7 = final_probability[7];
	SETREG(ime_3dnr_probability.reg, reg_addr + IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER7_OFS);

	// NE MD
	for (i = 0; i < (IQ_3DNR_COEFA_LEN / 4); i++) {
		ime_3dnr_md_sad_coeffa_0.bit.ime_3dnr_md_sad_coef_a0 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->sad_coefa[4*i+0], 0, 100);
		ime_3dnr_md_sad_coeffa_0.bit.ime_3dnr_md_sad_coef_a1 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->sad_coefa[4*i+1], 0, 100);
		ime_3dnr_md_sad_coeffa_0.bit.ime_3dnr_md_sad_coef_a2 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->sad_coefa[4*i+2], 0, 100);
		ime_3dnr_md_sad_coeffa_0.bit.ime_3dnr_md_sad_coef_a3 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->sad_coefa[4*i+3], 0, 100);
		SETREG(ime_3dnr_md_sad_coeffa_0.reg, reg_addr + (IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER0_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_3DNR_COEFB_LEN / 2); i++) {
		ime_3dnr_md_sad_coeffb_0.bit.ime_3dnr_md_sad_coef_b0 = final_3dnr_tune->sad_coefb[2*i+0];
		ime_3dnr_md_sad_coeffb_0.bit.ime_3dnr_md_sad_coef_b1 = final_3dnr_tune->sad_coefb[2*i+1];
		SETREG(ime_3dnr_md_sad_coeffb_0.reg, reg_addr + (IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER2_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_3DNR_STD_LEN / 2); i++) {
		ime_3dnr_md_sad_std_0.bit.ime_3dnr_md_sad_std0 = final_3dnr_tune->sad_std[2*i+0];
		ime_3dnr_md_sad_std_0.bit.ime_3dnr_md_sad_std1 = final_3dnr_tune->sad_std[2*i+1];
		SETREG(ime_3dnr_md_sad_std_0.reg, reg_addr + (IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER6_OFS + (i * 4)));
	}

	ime_3dnr_md_k.bit.ime_3dnr_md_k1 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->fth[0], 0, 100);
	ime_3dnr_md_k.bit.ime_3dnr_md_k2 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->fth[1], 0, 100);
	SETREG(ime_3dnr_md_k.reg, reg_addr + IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER10_OFS);

	// NE MC
	for (i = 0; i < (IQ_3DNR_BASE_LEN / 2); i++) {
		ime_3dnr_sad_base_0.bit.ime_3dnr_mc_sad_base0 = final_3dnr_tune->sad_base[2*i+0];
		ime_3dnr_sad_base_0.bit.ime_3dnr_mc_sad_base1 = final_3dnr_tune->sad_base[2*i+1];
		SETREG(ime_3dnr_sad_base_0.reg, reg_addr + (IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER0_OFS + (i * 4)));
	}
	for (i = 0; i < (IQ_3DNR_COEFA_LEN / 4); i++) {
		ime_3dnr_mc_sad_coeffa_0.bit.ime_3dnr_mc_sad_coef_a0 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->sad_coefa[4*i+0], 0, 100);
		ime_3dnr_mc_sad_coeffa_0.bit.ime_3dnr_mc_sad_coef_a1 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->sad_coefa[4*i+1], 0, 100);
		ime_3dnr_mc_sad_coeffa_0.bit.ime_3dnr_mc_sad_coef_a2 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->sad_coefa[4*i+2], 0, 100);
		ime_3dnr_mc_sad_coeffa_0.bit.ime_3dnr_mc_sad_coef_a3 = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->sad_coefa[4*i+3], 0, 100);
		SETREG(ime_3dnr_mc_sad_coeffa_0.reg, reg_addr + (IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER4_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_3DNR_COEFB_LEN / 2); i++) {
		ime_3dnr_mc_sad_coeffb_0.bit.ime_3dnr_mc_sad_coef_b0 = final_3dnr_tune->sad_coefb[2*i+0];
		ime_3dnr_mc_sad_coeffb_0.bit.ime_3dnr_mc_sad_coef_b1 = final_3dnr_tune->sad_coefb[2*i+1];
		SETREG(ime_3dnr_mc_sad_coeffb_0.reg, reg_addr + (IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER6_OFS + (i * 4)));
	}

	for (i = 0; i < (IQ_3DNR_STD_LEN / 2); i++) {
		ime_3dnr_mc_sad_std_0.bit.ime_3dnr_mc_sad_std0 = final_3dnr_tune->sad_std[2*i+0];
		ime_3dnr_mc_sad_std_0.bit.ime_3dnr_mc_sad_std1 = final_3dnr_tune->sad_std[2*i+1];
		SETREG(ime_3dnr_mc_sad_std_0.reg, reg_addr + (IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER10_OFS + (i * 4)));
	}

	ime_3dnr_mc_k.bit.ime_3dnr_mc_k1 = ime_3dnr_md_k.bit.ime_3dnr_md_k1;
	ime_3dnr_mc_k.bit.ime_3dnr_mc_k2 = ime_3dnr_md_k.bit.ime_3dnr_md_k2;
	SETREG(ime_3dnr_mc_k.reg, reg_addr + IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER14_OFS);

	// PS
	ime_3dnr_mv_th.reg = GETREG(reg_addr + IME_3DNR_PS_CONTROL_REGISTER0_OFS);
	ime_3dnr_mv_th.bit.ime_3dnr_ps_mv_th = iq_builtin_intpl(scene_change_w, (INT32)63, (INT32)final_3dnr_tune->mv_th, 0, 100);
	SETREG(ime_3dnr_mv_th.reg, reg_addr + IME_3DNR_PS_CONTROL_REGISTER0_OFS);

	ime_3dnr_ps_mix_ratio.bit.ime_3dnr_ps_mix_ratio0 = final_3dnr_tune->mix_ratio[0];
	ime_3dnr_ps_mix_ratio.bit.ime_3dnr_ps_mix_ratio1 = final_3dnr_tune->mix_ratio[1];
	SETREG(ime_3dnr_ps_mix_ratio.reg, reg_addr + IME_3DNR_PS_CONTROL_REGISTER1_OFS);

	ime_3dnr_ps_ds_th.reg = GETREG(reg_addr + IME_3DNR_PS_CONTROL_REGISTER4_OFS);
	ime_3dnr_ps_ds_th.bit.ime_3dnr_ps_ds_th = iq_builtin_intpl(scene_change_w, (INT32)31, (INT32)final_3dnr_tune->ds_th, 0, 100);
	SETREG(ime_3dnr_ps_ds_th.reg, reg_addr + IME_3DNR_PS_CONTROL_REGISTER4_OFS);

	// ResidueTH
	for (i = 0; i < IQ_3DNR_OBJECT_NUM; i++) {
		y_residue_th[i] = iq_builtin_intpl(scene_change_w, (INT32)1, (INT32)final_3dnr_tune->luma_residue_th[i], 0, 100);
	}

	// real setting: 0,0,0,1,0,0,0,1,0,0...
	if (count[id] < 3) {
		count[id]++;
	} else {
		for (i = 0; i < IQ_3DNR_OBJECT_NUM; i++) {
			if (y_residue_th[i] == 0) {
				y_residue_th[i] = 1;
			}
		}
		count[id] = 0;
	}

	ime_3dnr_residue.bit.ime_3dnr_nr_residue_th_y0 = y_residue_th[0];
	ime_3dnr_residue.bit.ime_3dnr_nr_residue_th_y1 = y_residue_th[1];
	ime_3dnr_residue.bit.ime_3dnr_nr_residue_th_y2 = y_residue_th[2];
	ime_3dnr_residue.bit.ime_3dnr_nr_residue_th_c = final_3dnr_tune->chroma_residue_th;
	SETREG(ime_3dnr_residue.reg, reg_addr + IME_3DNR_NR_CONTROL_REGISTER0_OFS);

	// Filter TF0
	ime_3dnr_tf0_reg0.bit.ime_3dnr_nr_y_tf0_blur_str0 = final_3dnr_tune->tf0_blur_str[0];
	ime_3dnr_tf0_reg0.bit.ime_3dnr_nr_y_tf0_blur_str1 = final_3dnr_tune->tf0_blur_str[1];
	ime_3dnr_tf0_reg0.bit.ime_3dnr_nr_y_tf0_blur_str2 = final_3dnr_tune->tf0_blur_str[2];
	ime_3dnr_tf0_reg0.bit.ime_3dnr_nr_y_tf0_str0 = final_3dnr_tune->tf0_y_str[0];
	SETREG(ime_3dnr_tf0_reg0.reg, reg_addr + IME_3DNR_TF0_REGISTER0_OFS);

	ime_3dnr_tf0_reg1.bit.ime_3dnr_nr_y_tf0_str1 = final_3dnr_tune->tf0_y_str[1];
	ime_3dnr_tf0_reg1.bit.ime_3dnr_nr_y_tf0_str2 = final_3dnr_tune->tf0_y_str[2];
	ime_3dnr_tf0_reg1.bit.ime_3dnr_nr_c_tf0_str0 = final_3dnr_tune->tf0_c_str[0];
	ime_3dnr_tf0_reg1.bit.ime_3dnr_nr_c_tf0_str1 = final_3dnr_tune->tf0_c_str[1];
	SETREG(ime_3dnr_tf0_reg1.reg, reg_addr + IME_3DNR_TF0_REGISTER1_OFS);

	ime_3dnr_tf0_reg2.bit.ime_3dnr_nr_c_tf0_str2 = final_3dnr_tune->tf0_c_str[2];
	ime_3dnr_tf0_reg2.bit.ime_3dnr_nr_y_tf0_blur_estr = final_3dnr_tune->tf0_blur_estr;
	SETREG(ime_3dnr_tf0_reg2.reg, reg_addr + IME_3DNR_TF0_REGISTER2_OFS);

	ime_3dnr_tf0_uv_reg.bit.ime_3dnr_nr_u_tf0_md_th = final_3dnr_tune->tf0_u_th;
	ime_3dnr_tf0_uv_reg.bit.ime_3dnr_nr_v_tf0_md_th = final_3dnr_tune->tf0_v_th;
	ime_3dnr_tf0_uv_reg.bit.ime_3dnr_nr_c_tf0_ratio_0 = IQ_CLAMP(255 -final_3dnr_tune->tf0_uv_ratio[0], 0, 255);
	ime_3dnr_tf0_uv_reg.bit.ime_3dnr_nr_c_tf0_ratio_1 = IQ_CLAMP(255 -final_3dnr_tune->tf0_uv_ratio[1], 0, 255);
	SETREG(ime_3dnr_tf0_uv_reg.reg, reg_addr + IME_3DNR_PS_CONTROL_REGISTER2_OFS);

	// Filter PreFilter & Filter
	ime_3dnr_pre_filter_str.bit.ime_3dnr_nr_pre_filtering_str0 = pre_filter_str[0];
	ime_3dnr_pre_filter_str.bit.ime_3dnr_nr_pre_filtering_str1 = pre_filter_str[1];
	ime_3dnr_pre_filter_str.bit.ime_3dnr_nr_pre_filtering_str2 = pre_filter_str[2];
	ime_3dnr_pre_filter_str.bit.ime_3dnr_nr_pre_filtering_str3 = pre_filter_str[3];
	SETREG(ime_3dnr_pre_filter_str.reg, reg_addr + IME_3DNR_NR_CONTROL_REGISTER6_OFS);

	ime_3dnr_pre_filter_ratio.bit.ime_3dnr_nr_pre_filtering_ratio0 = iq_builtin_intpl(scene_change_w, (INT32)255, (INT32)final_3dnr_tune->pre_filter_rto[0], 0, 100);
	ime_3dnr_pre_filter_ratio.bit.ime_3dnr_nr_pre_filtering_ratio1 = iq_builtin_intpl(scene_change_w, (INT32)255, (INT32)final_3dnr_tune->pre_filter_rto[1], 0, 100);
	ime_3dnr_pre_filter_ratio.bit.ime_3dnr_nr_snr_str0 = final_3dnr_tune->snr_str[0];
	ime_3dnr_pre_filter_ratio.bit.ime_3dnr_nr_snr_str1 = iq_builtin_intpl(scene_change_w, (INT32)64, (INT32)final_3dnr_tune->snr_str[1], 0, 100);
	SETREG(ime_3dnr_pre_filter_ratio.reg, reg_addr + IME_3DNR_NR_CONTROL_REGISTER8_OFS);

	ime_3dnr_snr_tnr_str.bit.ime_3dnr_nr_snr_str2 = iq_builtin_intpl(scene_change_w, (INT32)64, (INT32)final_3dnr_tune->snr_str[2], 0, 100);
	ime_3dnr_snr_tnr_str.bit.ime_3dnr_nr_tnr_str0 = iq_builtin_intpl(scene_change_w, (INT32)64, (INT32)final_3dnr_tune->tnr_str[0], 0, 100);
	ime_3dnr_snr_tnr_str.bit.ime_3dnr_nr_tnr_str1 = iq_builtin_intpl(scene_change_w, (INT32)64, (INT32)final_3dnr_tune->tnr_str[1], 0, 100);
	ime_3dnr_snr_tnr_str.bit.ime_3dnr_nr_tnr_str2 = iq_builtin_intpl(scene_change_w, (INT32)64, (INT32)final_3dnr_tune->tnr_str[2], 0, 100);
	SETREG(ime_3dnr_snr_tnr_str.reg, reg_addr + IME_3DNR_NR_CONTROL_REGISTER9_OFS);

	ime_3dnr_nr_base_th.bit.ime_3dnr_nr_base_th_snr = final_3dnr_tune->snr_base_th;
	ime_3dnr_nr_base_th.bit.ime_3dnr_nr_base_th_tnr = tnr_base_th;
	SETREG(ime_3dnr_nr_base_th.reg, reg_addr + IME_3DNR_NR_CONTROL_REGISTER10_OFS);

	ime_3dnr_freq_w.bit.ime_3dnr_nr_freq_w0 = final_3dnr_tune->freq_wet[0];
	ime_3dnr_freq_w.bit.ime_3dnr_nr_freq_w1 = final_3dnr_tune->freq_wet[1];
	ime_3dnr_freq_w.bit.ime_3dnr_nr_freq_w2 = final_3dnr_tune->freq_wet[2];
	ime_3dnr_freq_w.bit.ime_3dnr_nr_freq_w3 = final_3dnr_tune->freq_wet[3];
	SETREG(ime_3dnr_freq_w.reg, reg_addr + IME_3DNR_NR_CONTROL_REGISTER1_OFS);

	for (i = 0; i < (IQ_3DNR_F3_LUT_LEN / 4); i++) {
		ime_3dnr_luma_w_0.bit.ime_3dnr_nr_luma_w0 = final_3dnr_tune->luma_wet[4*i+0];
		ime_3dnr_luma_w_0.bit.ime_3dnr_nr_luma_w1 = final_3dnr_tune->luma_wet[4*i+1];
		ime_3dnr_luma_w_0.bit.ime_3dnr_nr_luma_w2 = final_3dnr_tune->luma_wet[4*i+2];
		ime_3dnr_luma_w_0.bit.ime_3dnr_nr_luma_w3 = final_3dnr_tune->luma_wet[4*i+3];
		SETREG(ime_3dnr_luma_w_0.reg, reg_addr + (IME_3DNR_NR_CONTROL_REGISTER3_OFS + (i * 4)));
	}

	// Filter PostFilter
	for (i = 0; i < (IQ_3DNR_F4_LUT_LEN / 4); i++) {
		ime_3dnr_luma_3d_lut_0.bit.ime_3dnr_nr_y_3d_lut0 = iq_builtin_intpl(scene_change_w, (INT32)(16*(4*i+0)), luma_3d_lut[4*i+0], 0, 100);
		ime_3dnr_luma_3d_lut_0.bit.ime_3dnr_nr_y_3d_lut1 = iq_builtin_intpl(scene_change_w, (INT32)(16*(4*i+1)), luma_3d_lut[4*i+1], 0, 100);
		ime_3dnr_luma_3d_lut_0.bit.ime_3dnr_nr_y_3d_lut2 = iq_builtin_intpl(scene_change_w, (INT32)(16*(4*i+2)), luma_3d_lut[4*i+2], 0, 100);
		ime_3dnr_luma_3d_lut_0.bit.ime_3dnr_nr_y_3d_lut3 = iq_builtin_intpl(scene_change_w, (INT32)(16*(4*i+3)), luma_3d_lut[4*i+3], 0, 100);
		SETREG(ime_3dnr_luma_3d_lut_0.reg, reg_addr + (IME_3DNR_NR_CONTROL_REGISTER12_OFS + (i * 4)));
	}

	ime_3dnr_luma_3d_rto.bit.ime_3dnr_nr_y_3d_ratio0 = final_3dnr_tune->luma_3d_rto[0];
	ime_3dnr_luma_3d_rto.bit.ime_3dnr_nr_y_3d_ratio1 = final_3dnr_tune->luma_3d_rto[1];
	ime_3dnr_luma_3d_rto.bit.ime_3dnr_nr_luma_comp_str = final_3dnr_tune->luma_comp_str;
	SETREG(ime_3dnr_luma_3d_rto.reg, reg_addr + IME_3DNR_NR_CONTROL_REGISTER11_OFS);

	for (i = 0; i < (IQ_3DNR_F4_LUT_LEN / 4); i++) {
		ime_3dnr_chroma_3d_lut_0.bit.ime_3dnr_nr_c_3d_lut0 = chroma_3d_lut[4*i+0];
		ime_3dnr_chroma_3d_lut_0.bit.ime_3dnr_nr_c_3d_lut1 = chroma_3d_lut[4*i+1];
		ime_3dnr_chroma_3d_lut_0.bit.ime_3dnr_nr_c_3d_lut2 = chroma_3d_lut[4*i+2];
		ime_3dnr_chroma_3d_lut_0.bit.ime_3dnr_nr_c_3d_lut3 = chroma_3d_lut[4*i+3];
		SETREG(ime_3dnr_chroma_3d_lut_0.reg, reg_addr + (IME_3DNR_NR_CONTROL_REGISTER14_OFS + (i * 4)));
	}

	ime_3dnr_chroma_3d_ratio.bit.ime_3dnr_nr_c_3d_ratio0 = final_3dnr_tune->chroma_3d_rto[0];
	ime_3dnr_chroma_3d_ratio.bit.ime_3dnr_nr_c_3d_ratio1 = final_3dnr_tune->chroma_3d_rto[1];
	SETREG(ime_3dnr_chroma_3d_ratio.reg, reg_addr + IME_3DNR_NR_CONTROL_REGISTER16_OFS);

	// FCVG
	ime_3dnr_fast_converge.bit.ime_3dnr_fast_converge_sp = final_3dnr_tune->fcvg_start_point;
	ime_3dnr_fast_converge.bit.ime_3dnr_fast_converge_step = final_3dnr_tune->fcvg_step_size;
	SETREG(ime_3dnr_fast_converge.reg, reg_addr + IME_3DNR_FAST_CONVERGE_CONTROL_REGISTER0_OFS);

	// Motion Color & CSHK
	ime_3dnr_motion_sat_reg.reg = GETREG(reg_addr + IME_3DNR_PS_CONTROL_REGISTER3_OFS);
	ime_3dnr_motion_sat_reg.bit.ime_3dnr_nr_motion_sat_ratio = final_3dnr_tune->motion_sat_ratio;
	SETREG(ime_3dnr_motion_sat_reg.reg, reg_addr + IME_3DNR_PS_CONTROL_REGISTER3_OFS);

	for (i = 0; i < (IQ_3DNR_LUMA_LEN / 2); i++) {
		ime_3dnr_cshk_th_reg_0.bit.ime_3dnr_nr_cshk_th0 = final_3dnr_tune->cshk_th[2*i+0];
		ime_3dnr_cshk_th_reg_0.bit.ime_3dnr_nr_cshk_th1 = final_3dnr_tune->cshk_th[2*i+1];
		SETREG(ime_3dnr_cshk_th_reg_0.reg, reg_addr + (IME_3DNR_COLOR_DITHERING_REGISTER0_OFS + (i * 4)));
	}

	ime_3dnr_cshk_val_reg.bit.ime_3dnr_nr_cshk_val0 = final_3dnr_tune->cshk_val[0];
	ime_3dnr_cshk_val_reg.bit.ime_3dnr_nr_cshk_val1 = final_3dnr_tune->cshk_val[1];
	ime_3dnr_cshk_val_reg.bit.ime_3dnr_nr_cshk_val2 = final_3dnr_tune->cshk_val[2];
	ime_3dnr_cshk_val_reg.bit.ime_3dnr_nr_cshk_val3 = final_3dnr_tune->cshk_val[3];
	ime_3dnr_cshk_val_reg.bit.ime_3dnr_nr_cshk_val4 = final_3dnr_tune->cshk_val[4];
	ime_3dnr_cshk_val_reg.bit.ime_3dnr_nr_cshk_val5 = final_3dnr_tune->cshk_val[5];
	ime_3dnr_cshk_val_reg.bit.ime_3dnr_nr_cshk_val6 = final_3dnr_tune->cshk_val[6];
	ime_3dnr_cshk_val_reg.bit.ime_3dnr_nr_cshk_val7 = final_3dnr_tune->cshk_val[7];
	SETREG(ime_3dnr_cshk_val_reg.reg, reg_addr + IME_3DNR_COLOR_DITHERING_REGISTER4_OFS);
}

static void iq_builtin_set_post_sharpen_1(UINT32 id, UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	T_IME_FUNCTION_CONTROL_REGISTER0                           ime_function_control_reg0;
	T_IME_POST_SHARPEN_REGISTER0                               post_sharpen_reg_0;
	T_IME_POST_SHARPEN_REGISTER1                               post_sharpen_reg_1;
	T_IME_POST_SHARPEN_REGISTER2                               post_sharpen_reg_2;
	T_IME_POST_SHARPEN_REGISTER3                               post_sharpen_reg_3;
	T_IME_POST_SHARPEN_REGISTER4                               post_sharpen_reg_4;
	T_IME_POST_SHARPEN_REGISTER5                               post_sharpen_reg_5;
	T_IME_POST_SHARPEN_REGISTER6                               noise_curve_reg_6;
	T_IME_POST_SHARPEN_REGISTER10                              noise_curve_reg_10;

	#if defined(__KERNEL__)
	void __iomem                         *reg_addr = NULL;
	#else
	unsigned int                         reg_addr;
	#endif 
	IQ_POST_SHARPEN_1_TUNE_PARAM         *final_post_sharpen_1;

	UINT32                               iso_idx = 0;
	INT32                                i;

	reg_addr = ime_reg_addr;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i-1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->post_sharpen_1->mode == IQ_OP_TYPE_AUTO) {
		final_post_sharpen_1 = &iq_param->post_sharpen_1->auto_param[iso_idx];
	} else {
		final_post_sharpen_1 = &iq_param->post_sharpen_1->manual_param;
	}

	ime_function_control_reg0.reg = GETREG(reg_addr + IME_FUNCTION_CONTROL_REGISTER0_OFS);
	ime_function_control_reg0.bit.ime_post_shp_en = iq_param->post_sharpen_1->enable;
	SETREG(ime_function_control_reg0.reg, reg_addr + IME_FUNCTION_CONTROL_REGISTER0_OFS);

	post_sharpen_reg_0.reg = GETREG(reg_addr + IME_POST_SHARPEN_REGISTER0_OFS);
	post_sharpen_reg_0.bit.shp_noise_level = final_post_sharpen_1->noise_level;
	post_sharpen_reg_0.bit.shp_edge_weight_th = final_post_sharpen_1->edge_weight_th;
	post_sharpen_reg_0.bit.shp_edge_weight_gain = final_post_sharpen_1->edge_weight_gain;
	post_sharpen_reg_0.bit.shp_jnd_filter_size = final_post_sharpen_1->edge_filt_sel;
	SETREG(post_sharpen_reg_0.reg, reg_addr + IME_POST_SHARPEN_REGISTER0_OFS);

	for (i = 0; i < (IQ_POST_SHARPEN_CURVE_NUM / 4); i++) {
		noise_curve_reg_6.bit.shp_noise_curve0 = final_post_sharpen_1->noise_curve[4*i+0];
		noise_curve_reg_6.bit.shp_noise_curve1 = final_post_sharpen_1->noise_curve[4*i+1];
		noise_curve_reg_6.bit.shp_noise_curve2 = final_post_sharpen_1->noise_curve[4*i+2];
		noise_curve_reg_6.bit.shp_noise_curve3 = final_post_sharpen_1->noise_curve[4*i+3];
		SETREG(noise_curve_reg_6.reg, reg_addr + (IME_POST_SHARPEN_REGISTER6_OFS + (i * 4)));
	}

	noise_curve_reg_10.bit.shp_noise_curve16 = final_post_sharpen_1->noise_curve[16];
	SETREG(noise_curve_reg_10.reg, reg_addr + IME_POST_SHARPEN_REGISTER10_OFS);

	post_sharpen_reg_3.bit.shp_flat_th = final_post_sharpen_1->th_flat;
	post_sharpen_reg_3.bit.shp_edge_th = final_post_sharpen_1->th_edge;
	SETREG(post_sharpen_reg_3.reg, reg_addr + IME_POST_SHARPEN_REGISTER3_OFS);

	post_sharpen_reg_4.bit.shp_flat_region_str = final_post_sharpen_1->flat_region_str;
	post_sharpen_reg_4.bit.shp_edge_region_str = final_post_sharpen_1->edge_region_str;
	if (final_post_sharpen_1->th_edge == final_post_sharpen_1->th_flat) {
		post_sharpen_reg_4.bit.shp_slope_con_eng = 4095;
	} else {
		if (final_post_sharpen_1->edge_region_str < final_post_sharpen_1->flat_region_str) {
			post_sharpen_reg_4.bit.shp_slope_con_eng = 0;
		} else {
			post_sharpen_reg_4.bit.shp_slope_con_eng = IQ_CLAMP(256 * (final_post_sharpen_1->edge_region_str - final_post_sharpen_1->flat_region_str) / (final_post_sharpen_1->th_edge - final_post_sharpen_1->th_flat), 0, 255);
		}
	}
	SETREG(post_sharpen_reg_4.reg, reg_addr + IME_POST_SHARPEN_REGISTER4_OFS);

	post_sharpen_reg_5.bit.shp_motion_edge_weight_str = final_post_sharpen_1->motion_edge_w_str;
	post_sharpen_reg_5.bit.shp_static_edge_weight_str = final_post_sharpen_1->static_edge_w_str;
	post_sharpen_reg_5.bit.shp_trans_edge_weight_str = final_post_sharpen_1->tarnsition_edge_w_str;
	SETREG(post_sharpen_reg_5.reg, reg_addr + IME_POST_SHARPEN_REGISTER5_OFS);

	post_sharpen_reg_1.bit.shp_blend_inv_gamma = final_post_sharpen_1->blend_inv_gamma;
	post_sharpen_reg_1.bit.shp_edge_str = final_post_sharpen_1->sharp_str;
	post_sharpen_reg_1.bit.shp_coring_th = final_post_sharpen_1->coring_th;
	post_sharpen_reg_1.bit.shp_w_con_eng = 0;
	SETREG(post_sharpen_reg_1.reg, reg_addr + IME_POST_SHARPEN_REGISTER1_OFS);

	post_sharpen_reg_2.bit.shp_bright_halo_clip = final_post_sharpen_1->bright_halo_clip;
	post_sharpen_reg_2.bit.shp_dark_halo_clip = final_post_sharpen_1->dark_halo_clip;
	SETREG(post_sharpen_reg_2.reg, reg_addr + IME_POST_SHARPEN_REGISTER2_OFS);
}

static void iq_builtin_set_post_sharpen_2(UINT32 id, UINT32 gain, IQ_PARAM_PTR *iq_param)
{
	KDRV_H26XENC_SPN               enc_post_sharpen_cfg = {0};

	IQ_POST_SHARPEN_2_TUNE_PARAM   *final_post_sharpen_2;

	UINT32                         iso_idx = 0;
	static UINT16                  usEWG[9] = {0, 4, 8, 40, 128, 312, 648, 1200, 2016};
	INT32                          i;

	if (gain >= iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].iso) {
		iso_idx = iso_map_tab[IQ_GAIN_ID_MAX_NUM - 1].index;
	}

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i-1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if (iq_param->post_sharpen_2->mode == IQ_OP_TYPE_AUTO) {
		final_post_sharpen_2 = &iq_param->post_sharpen_2->auto_param[iso_idx];
	} else {
		final_post_sharpen_2 = &iq_param->post_sharpen_2->manual_param;
	}

	enc_post_sharpen_cfg.bEnable = iq_param->post_sharpen_2->enable;
	if (final_post_sharpen_2->th_edge == final_post_sharpen_2->th_flat) {
		enc_post_sharpen_cfg.usSlopConEng = 4095;
	} else {
		if (final_post_sharpen_2->edge_region_str < final_post_sharpen_2->flat_region_str) {
			enc_post_sharpen_cfg.usSlopConEng = 0;
		} else {
			enc_post_sharpen_cfg.usSlopConEng = IQ_CLAMP(256 * (final_post_sharpen_2->edge_region_str - final_post_sharpen_2->flat_region_str) / (final_post_sharpen_2->th_edge - final_post_sharpen_2->th_flat), 0, 255);
		}
	}
	enc_post_sharpen_cfg.ucBHC = final_post_sharpen_2->bright_halo_clip;
	enc_post_sharpen_cfg.ucDHC = final_post_sharpen_2->dark_halo_clip;
	enc_post_sharpen_cfg.ucEWT = final_post_sharpen_2->edge_weight_th;
	enc_post_sharpen_cfg.ucEWG = final_post_sharpen_2->edge_weight_gain;
	enc_post_sharpen_cfg.ucEdgeSharpStr1 = final_post_sharpen_2->sharp_str;
	enc_post_sharpen_cfg.ucCT = final_post_sharpen_2->coring_th;
	enc_post_sharpen_cfg.ucNL = final_post_sharpen_2->noise_level;
	enc_post_sharpen_cfg.ucBIG = final_post_sharpen_2->blend_inv_gamma;
	enc_post_sharpen_cfg.usFlatTh = final_post_sharpen_2->th_flat;
	enc_post_sharpen_cfg.usEdgeTh = final_post_sharpen_2->th_edge;
	enc_post_sharpen_cfg.ucEdgeStr = final_post_sharpen_2->edge_region_str;
	enc_post_sharpen_cfg.ucTransitionStr = final_post_sharpen_2->tarnsition_edge_w_str;
	enc_post_sharpen_cfg.ucMotionStr = final_post_sharpen_2->motion_edge_w_str;
	enc_post_sharpen_cfg.ucStaticStr = final_post_sharpen_2->static_edge_w_str;
	enc_post_sharpen_cfg.ucFlatStr = final_post_sharpen_2->flat_region_str;
	memcpy(enc_post_sharpen_cfg.ucNC, final_post_sharpen_2->noise_curve, sizeof(UINT8) * IQ_POST_SHARPEN_CURVE_NUM);
	memcpy(enc_post_sharpen_cfg.usEWG, usEWG, sizeof(UINT16) * 9);
	enc_post_sharpen_cfg.bShowSharpInfo = FALSE;

	// TODO: isp builtin
	#if defined(__KERNEL__)
	vdoenc_builtin_set(id, BUILTIN_VDOENC_ISP_ITEM_SPN, &enc_post_sharpen_cfg);
	#else
	DBG_DUMP("iq_builtin_get_ipe_subout not available in RTOS \r\n");
	#endif
}

//=============================================================================
// external functions
//=============================================================================
void iq_builtin_get_histo(UINT32 id, ISP_BUILTIN_HISTO_RSLT *histo_rslt)
{
	UINT16 *p_stcs;
	UINT32 addr_ofs, i = 0;
	T_WDR_CONTROL_REGISTER0                      wdr_en_reg;
	T_IFE_WDR_HISTOGRAM_REGISTER_0               ife_histo_reg_0;
	#if defined(__KERNEL__)
	void __iomem                                 *reg_addr = NULL;
	#else
	unsigned int                                 reg_addr;
	#endif 

	if (histo_rslt == NULL) {
		return;
	}

	reg_addr = ife_reg_addr;

	wdr_en_reg.reg = GETREG(reg_addr + WDR_CONTROL_REGISTER0_OFS);
	if (wdr_en_reg.bit.ife_wdr_histogram_sel == 0) { // before WDR
		p_stcs = histo_rslt->hist_stcs_pre_wdr;
	} else if (wdr_en_reg.bit.ife_wdr_histogram_sel == 1) { // post WDR
		p_stcs = histo_rslt->hist_stcs_post_wdr;
	}

	for (i = 0; i < (ISP_HISTO_MAX_SIZE / 2); i++) {
		addr_ofs = i * 4;
		ife_histo_reg_0.reg = GETREG(reg_addr + IFE_WDR_HISTOGRAM_REGISTER_0_OFS + addr_ofs);

		*(p_stcs + 2 * i) = (ife_histo_reg_0.bit.ife_wdr_histogram_bin0);
		*(p_stcs + 2 * i + 1) = (ife_histo_reg_0.bit.ife_wdr_histogram_bin1);
	}
}

void iq_builtin_get_airlight(UINT32 id, ISP_BUILTIN_DEFOG_STCS *defog_stcs)
{
	UINT16 *p_stcs = defog_stcs->dfg_airlight;
	T_DEFOG_STATISTICS_REGISTER_1                defog_statis_reg_1;
	T_DEFOG_STATISTICS_REGISTER_2                defog_statis_reg_2;
	#if defined(__KERNEL__)
	void __iomem                                 *reg_addr = NULL;
	#else
	unsigned int                                 reg_addr;
	#endif 

	if (p_stcs == NULL) {
		return;
	}

	reg_addr = ipe_reg_addr;

	defog_statis_reg_1.reg = GETREG(reg_addr + DEFOG_STATISTICS_REGISTER_1_OFS);
	defog_stcs->dfg_airlight[0] = (defog_statis_reg_1.bit.defog_statistics_air0);
	defog_stcs->dfg_airlight[1] = (defog_statis_reg_1.bit.defog_statistics_air1);

	defog_statis_reg_2.reg = GETREG(reg_addr + DEFOG_STATISTICS_REGISTER_2_OFS);
	defog_stcs->dfg_airlight[2] = (defog_statis_reg_2.bit.defog_statistics_air2);
}

void iq_builtin_get_subimg_addr(void)
{
	// TODO: isp builtin
	#if 0
	ULONG subimg_addr;
	T_DMA_DEFOG_SUBIMG_INPUT_CHANNEL_REGISTER defog_subimg_addr_reg;
	#if defined(__KERNEL__)
	void __iomem *reg_addr = NULL;
	#else
	unsigned int reg_addr;
	#endif 

	reg_addr = ipe_reg_addr;

	defog_subimg_addr_reg.reg = GETREG(reg_addr + DMA_DEFOG_SUBIMG_INPUT_CHANNEL_REGISTER_OFS);
	subimg_addr = (defog_subimg_addr_reg.bit.defog_subimg_dramsai) << 2;
	ipe_subimg_vaddr = nvtmpp_buitin_sys_pa2va(subimg_addr);
	#endif
}

void iq_builtin_get_ipe_subout(UINT32 id, ISP_BUILTIN_IPE_SUBOUT_BUF *ipe_subout)
{
	// TODO: isp builtin
	#if 0
	UINT32 read_value;
	UINT16 *p_min, *p_avg;
	UINT32 i = 0;
	#if defined(__KERNEL__)
	void __iomem *reg_addr = NULL;
	#else
	unsigned int reg_addr;
	#endif 

	if (ipe_subout == NULL) {
		return;
	}

	p_min = ipe_subout->min;
	p_avg = ipe_subout->avg;
	if ((p_min == NULL) || (p_avg == NULL)) {
		return;
	}

	reg_addr = ipe_reg_addr;

	vos_cpu_dcache_sync((ULONG)ipe_subimg_vaddr, ISP_SUBOUT_MAX_SIZE * 4, VOS_DMA_FROM_DEVICE);

	if (ipe_subimg_vaddr != 0) {
		for (i = 0; i < ISP_SUBOUT_MAX_SIZE; i++) {
			read_value = GETREG(((UINT32 *)ipe_subimg_vaddr + i));
			p_min[i] = read_value & 0x3FF;
			p_avg[i] = read_value >> 16 & 0x3FF;
		}
	}
	#else
	DBG_DUMP("iq_builtin_get_ipe_subout not available \r\n");
	#endif
}

void iq_builtin_set_ca_crop(UINT32 id, BOOL enable)
{
	T_R38_ENGINE_TIMING r38_eng_timing;
	T_R224_STCS_CA r224_stcs_ca;
	T_R228_STCS_CA r228_stcs_ca;
	#if defined(__KERNEL__)
	void __iomem *reg_addr = NULL;
	#else
	unsigned int reg_addr;
	#endif 
	INT32 i;

	switch (id) {
		default:
		case 0:
			reg_addr = sie1_reg_addr;
			break;
		case 1:
			reg_addr = sie2_reg_addr;
			break;
		case 2:
			reg_addr = sie3_reg_addr;
			break;
		case 3:
			reg_addr = sie4_reg_addr;
			break;
		case 4:
			reg_addr = sie5_reg_addr;
			break;
	}

	r38_eng_timing.reg = GETREG(reg_addr + R38_ENGINE_TIMING_OFS);

	r228_stcs_ca.reg = GETREG(reg_addr + R228_STCS_CA_OFS);
	if (enable) {
		r228_stcs_ca.bit.CA_CROP_SZY = r38_eng_timing.bit.CRP_SZY * 9 / 10;  // reduce 10% of ca window
	} else {
		r228_stcs_ca.bit.CA_CROP_SZY = r38_eng_timing.bit.CRP_SZY;
	}
	SETREG(r228_stcs_ca.reg, reg_addr + R228_STCS_CA_OFS);

	r224_stcs_ca.reg = GETREG(reg_addr + R224_STCS_CA_OFS);
	r224_stcs_ca.bit.SMPL_Y_FACT  =  ((r228_stcs_ca.bit.CA_CROP_SZY/2)-1)*1024/255;
	SETREG(r224_stcs_ca.reg, reg_addr + R224_STCS_CA_OFS);

	if (isp_builtin_get_shdr_enable(id)) {
		for (i = ISP_BUILTIN_ID_MAX_NUM - 1; i >= 0; i--) {
			if ((0x1 << i) & isp_builtin_get_shdr_id_mask(id)) {
				break;
			}
		}

		switch (i) {
			default:
			case 0:
				reg_addr = sie1_reg_addr;
				break;
			case 1:
				reg_addr = sie2_reg_addr;
				break;
			case 2:
				reg_addr = sie3_reg_addr;
				break;
			case 3:
				reg_addr = sie4_reg_addr;
				break;
			case 4:
				reg_addr = sie5_reg_addr;
				break;
	}

		SETREG(r228_stcs_ca.reg, reg_addr + R228_STCS_CA_OFS);
		SETREG(r224_stcs_ca.reg, reg_addr + R224_STCS_CA_OFS);
	}
	DBG_DUMP("img crop = %d, ca crop = %d, factor = %d \r\n", r38_eng_timing.bit.CRP_SZY, r228_stcs_ca.bit.CA_CROP_SZY, r224_stcs_ca.bit.SMPL_Y_FACT);
}

static INT32 iq_builtin_get_param_addr(IQ_ID id)
{
	UINT32 total_param_size;
	ULONG temp_addr = 0;
	IQ_PARAM_PTR *iq_param_temp = NULL;
	void *param_mem_addr = NULL;
	static BOOL use_param_phy_addr = TRUE;
	int align_byte = 4;

	if (use_param_phy_addr == TRUE) {
		use_param_phy_addr = FALSE;
		iq_param_get_param(&temp_addr);
		iq_param[id] = (IQ_PARAM_PTR *)temp_addr;
	} else {
		total_param_size = ALIGN_CEIL(sizeof(IQ_OB_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_NR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_CFA_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_VA_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_GAMMA_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_CCM_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_COLOR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_CONTRAST_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_EDGE_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_3DNR_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_PFR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_WDR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_DEFOG_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_SHDR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_COMPANDING_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_RGBIR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_RGBIR_ENH_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_2_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_YCURVE_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_CST_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_1_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_TONE_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_WDR_ENH_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_RAW_VA_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_FPN_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_BNR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_AIISP_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_AIISP_CUSTOM_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_DPC_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_SHADING_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_SHADING_EXT_PARAM), align_byte);

		param_mem_addr = isp_builtin_kmem_alloc(total_param_size);
		if (param_mem_addr == NULL) {
			DBG_DUMP("fail to allocate iq parameter fail!\n");
			return -E_SYS;
		}

		iq_param_memalloc_addr[id].ob = (IQ_OB_PARAM *)param_mem_addr;
		iq_param_memalloc_addr[id].nr = (IQ_NR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].ob + ALIGN_CEIL(sizeof(IQ_OB_PARAM), align_byte));
		iq_param_memalloc_addr[id].cfa = (IQ_CFA_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].nr + ALIGN_CEIL(sizeof(IQ_NR_PARAM), align_byte));
		iq_param_memalloc_addr[id].va = (IQ_VA_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].cfa + ALIGN_CEIL(sizeof(IQ_CFA_PARAM), align_byte));
		iq_param_memalloc_addr[id].gamma = (IQ_GAMMA_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].va + ALIGN_CEIL(sizeof(IQ_VA_PARAM), align_byte));
		iq_param_memalloc_addr[id].ccm = (IQ_CCM_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].gamma + ALIGN_CEIL(sizeof(IQ_GAMMA_PARAM), align_byte));
		iq_param_memalloc_addr[id].color = (IQ_COLOR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].ccm + ALIGN_CEIL(sizeof(IQ_CCM_PARAM), align_byte));
		iq_param_memalloc_addr[id].contrast = (IQ_CONTRAST_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].color + ALIGN_CEIL(sizeof(IQ_COLOR_PARAM), align_byte));
		iq_param_memalloc_addr[id].edge = (IQ_EDGE_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].contrast + ALIGN_CEIL(sizeof(IQ_CONTRAST_PARAM), align_byte));
		iq_param_memalloc_addr[id]._3dnr = (IQ_3DNR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].edge + ALIGN_CEIL(sizeof(IQ_EDGE_PARAM), align_byte));
		iq_param_memalloc_addr[id].pfr = (IQ_PFR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id]._3dnr + ALIGN_CEIL(sizeof(IQ_3DNR_PARAM), align_byte));
		iq_param_memalloc_addr[id].wdr = (IQ_WDR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].pfr + ALIGN_CEIL(sizeof(IQ_PFR_PARAM), align_byte));
		iq_param_memalloc_addr[id].defog = (IQ_DEFOG_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].wdr + ALIGN_CEIL(sizeof(IQ_WDR_PARAM), align_byte));
		iq_param_memalloc_addr[id].shdr = (IQ_SHDR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].defog + ALIGN_CEIL(sizeof(IQ_DEFOG_PARAM), align_byte));
		iq_param_memalloc_addr[id].companding = (IQ_COMPANDING_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].shdr + ALIGN_CEIL(sizeof(IQ_SHDR_PARAM), align_byte));
		iq_param_memalloc_addr[id].rgbir = (IQ_RGBIR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].companding + ALIGN_CEIL(sizeof(IQ_COMPANDING_PARAM), align_byte));
		iq_param_memalloc_addr[id].rgbir_enh = (IQ_RGBIR_ENH_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].rgbir + ALIGN_CEIL(sizeof(IQ_RGBIR_PARAM), align_byte));
		iq_param_memalloc_addr[id].post_sharpen_2 = (IQ_POST_SHARPEN_2_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].rgbir_enh + ALIGN_CEIL(sizeof(IQ_RGBIR_ENH_PARAM), align_byte));
		iq_param_memalloc_addr[id].ycurve = (IQ_YCURVE_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].post_sharpen_2 + ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_2_PARAM), align_byte));
		iq_param_memalloc_addr[id].cst = (IQ_CST_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].ycurve + ALIGN_CEIL(sizeof(IQ_YCURVE_PARAM), align_byte));
		iq_param_memalloc_addr[id].post_sharpen_1 = (IQ_POST_SHARPEN_1_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].cst + ALIGN_CEIL(sizeof(IQ_CST_PARAM), align_byte));
		iq_param_memalloc_addr[id].tone = (IQ_TONE_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].post_sharpen_1 + ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_1_PARAM), align_byte));
		iq_param_memalloc_addr[id].wdr_enh = (IQ_WDR_ENH_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].tone + ALIGN_CEIL(sizeof(IQ_TONE_PARAM), align_byte));
		iq_param_memalloc_addr[id].raw_va = (IQ_RAW_VA_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].wdr_enh + ALIGN_CEIL(sizeof(IQ_WDR_ENH_PARAM), align_byte));
		iq_param_memalloc_addr[id].fpn = (IQ_FPN_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].raw_va + ALIGN_CEIL(sizeof(IQ_RAW_VA_PARAM), align_byte));
		iq_param_memalloc_addr[id].bnr = (IQ_BNR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].fpn + ALIGN_CEIL(sizeof(IQ_FPN_PARAM), align_byte));
		iq_param_memalloc_addr[id].aiisp = (IQ_AIISP_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].bnr + ALIGN_CEIL(sizeof(IQ_BNR_PARAM), align_byte));
		iq_param_memalloc_addr[id].aiisp_custom = (IQ_AIISP_CUSTOM_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].aiisp + ALIGN_CEIL(sizeof(IQ_AIISP_PARAM), align_byte));
		// DPC, SHADING...
		iq_param_memalloc_addr[id].dpc = (IQ_DPC_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].fpn + ALIGN_CEIL(sizeof(IQ_AIISP_CUSTOM_PARAM), align_byte));
		iq_param_memalloc_addr[id].shading = (IQ_SHADING_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].dpc + ALIGN_CEIL(sizeof(IQ_DPC_PARAM), align_byte));
		iq_param_memalloc_addr[id].shading_ext = (IQ_SHADING_EXT_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].shading + ALIGN_CEIL(sizeof(IQ_SHADING_PARAM), align_byte));
		// DPC, SHADING...
		iq_param_get_param(&temp_addr);
		iq_param_temp = (IQ_PARAM_PTR *)temp_addr;
		memcpy(iq_param_memalloc_addr[id].ob, iq_param_temp->ob, sizeof(IQ_OB_PARAM));
		memcpy(iq_param_memalloc_addr[id].nr, iq_param_temp->nr, sizeof(IQ_NR_PARAM));
		memcpy(iq_param_memalloc_addr[id].cfa, iq_param_temp->cfa, sizeof(IQ_CFA_PARAM));
		memcpy(iq_param_memalloc_addr[id].va, iq_param_temp->va, sizeof(IQ_VA_PARAM));
		memcpy(iq_param_memalloc_addr[id].gamma, iq_param_temp->gamma, sizeof(IQ_GAMMA_PARAM));
		memcpy(iq_param_memalloc_addr[id].ccm, iq_param_temp->ccm, sizeof(IQ_CCM_PARAM));
		memcpy(iq_param_memalloc_addr[id].color, iq_param_temp->color, sizeof(IQ_COLOR_PARAM));
		memcpy(iq_param_memalloc_addr[id].contrast, iq_param_temp->contrast, sizeof(IQ_CONTRAST_PARAM));
		memcpy(iq_param_memalloc_addr[id].edge, iq_param_temp->edge, sizeof(IQ_EDGE_PARAM));
		memcpy(iq_param_memalloc_addr[id]._3dnr, iq_param_temp->_3dnr, sizeof(IQ_3DNR_PARAM));
		memcpy(iq_param_memalloc_addr[id].pfr, iq_param_temp->pfr, sizeof(IQ_PFR_PARAM));
		memcpy(iq_param_memalloc_addr[id].wdr, iq_param_temp->wdr, sizeof(IQ_WDR_PARAM));
		memcpy(iq_param_memalloc_addr[id].defog, iq_param_temp->defog, sizeof(IQ_DEFOG_PARAM));
		memcpy(iq_param_memalloc_addr[id].shdr, iq_param_temp->shdr, sizeof(IQ_SHDR_PARAM));
		memcpy(iq_param_memalloc_addr[id].companding, iq_param_temp->companding, sizeof(IQ_COMPANDING_PARAM));
		memcpy(iq_param_memalloc_addr[id].rgbir, iq_param_temp->rgbir, sizeof(IQ_RGBIR_PARAM));
		memcpy(iq_param_memalloc_addr[id].rgbir_enh, iq_param_temp->rgbir_enh, sizeof(IQ_RGBIR_ENH_PARAM));
		memcpy(iq_param_memalloc_addr[id].post_sharpen_2, iq_param_temp->post_sharpen_2, sizeof(IQ_POST_SHARPEN_2_PARAM));
		memcpy(iq_param_memalloc_addr[id].ycurve, iq_param_temp->ycurve, sizeof(IQ_YCURVE_PARAM));
		memcpy(iq_param_memalloc_addr[id].cst, iq_param_temp->cst, sizeof(IQ_CST_PARAM));
		memcpy(iq_param_memalloc_addr[id].post_sharpen_1, iq_param_temp->post_sharpen_1, sizeof(IQ_POST_SHARPEN_1_PARAM));
		memcpy(iq_param_memalloc_addr[id].tone, iq_param_temp->tone, sizeof(IQ_TONE_PARAM));
		memcpy(iq_param_memalloc_addr[id].wdr_enh, iq_param_temp->wdr_enh, sizeof(IQ_WDR_ENH_PARAM));
		memcpy(iq_param_memalloc_addr[id].raw_va, iq_param_temp->raw_va, sizeof(IQ_RAW_VA_PARAM));
		memcpy(iq_param_memalloc_addr[id].fpn, iq_param_temp->fpn, sizeof(IQ_FPN_PARAM));
		memcpy(iq_param_memalloc_addr[id].bnr, iq_param_temp->bnr, sizeof(IQ_BNR_PARAM));
		memcpy(iq_param_memalloc_addr[id].aiisp, iq_param_temp->aiisp, sizeof(IQ_AIISP_PARAM));
		memcpy(iq_param_memalloc_addr[id].aiisp_custom, iq_param_temp->aiisp_custom, sizeof(IQ_AIISP_CUSTOM_PARAM));
		// DPC, SHADING...
		memcpy(iq_param_memalloc_addr[id].dpc, iq_param_temp->dpc, sizeof(IQ_DPC_PARAM));
		memcpy(iq_param_memalloc_addr[id].shading, iq_param_temp->shading, sizeof(IQ_SHADING_PARAM));
		memcpy(iq_param_memalloc_addr[id].shading_ext, iq_param_temp->shading_ext, sizeof(IQ_SHADING_EXT_PARAM));
		// DPC, SHADING...
		iq_param[id] = &iq_param_memalloc_addr[id];
		iq_param_memalloc_valid[id] = TRUE;
	}
	return E_OK;
}

void iq_builtin_get_param(UINT32 id, ULONG *param)
{
	*param = (ULONG)(&iq_param[id]->ob);
}

INT32 iq_builtin_init(UINT32 id)
{
	ER rt = E_OK;

	gamma_buffer[id] = isp_builtin_kmem_alloc(GAMMA_BUFFER_SIZE);
	gamma_buffer_pa_addr[id] = vos_cpu_get_phy_addr((ULONG)gamma_buffer[id]);
	if (gamma_buffer[id] == NULL) {
		DBG_DUMP("allocate gamma_buffer fail!\n");
		return -E_SYS;
	}
	ycurve_buffer[id] = isp_builtin_kmem_alloc(YCURVE_BUFFER_SIZE);
	ycurve_buffer_pa_addr[id] = vos_cpu_get_phy_addr((ULONG)ycurve_buffer[id]);
	if (ycurve_buffer[id] == NULL) {
		DBG_DUMP("allocate ycurve_buffer fail!\n");
		return -E_SYS;
	}

	#if DPC_ENABLE
	dpc_buffer[id] = isp_builtin_kmem_alloc(DPC_BUFFER_SIZE);
	dpc_buffer_pa_addr[id] = vos_cpu_get_phy_addr((ULONG)dpc_buffer[id]);
	if (dpc_buffer[id] == NULL) {
		DBG_DUMP("allocate dpc_buffer fail!\n");
		return -E_SYS;
	}
	#endif

	#if ECS_ENABLE
	ecs_buffer[id] = isp_builtin_kmem_alloc(ECS_BUFFER_SIZE);
	ecs_buffer_pa_addr[id] = vos_cpu_get_phy_addr((ULONG)ecs_buffer[id]);
	if (ecs_buffer[id] == NULL) {
		DBG_DUMP("allocate ecs_buffer fail!\n");
		return -E_SYS;
	}
	#endif

	rt = iq_builtin_get_param_addr(id);
	if (rt != E_OK) {
		return rt;
	}

	#if (ISP_BUILTIN_RELOAD_DTSI)
	iq_dtsi_load(id, iq_param[id]);
	#endif

	iq_front_param[id] = iq_front_param_init;
	#if (ISP_BUILTIN_RELOAD_DTSI)
	#if defined(__KERNEL__)
	if (isp_builtin_get_fastboot_version() == 1) {
		iq_front_param[id] = iq_front_param_init;
		iq_builtin_front_dtsi_load(id, &iq_front_param[id]);
	} else {
		iq_front_param[id] = iq_front_param_off;
	}
	#else
	iq_front_param[id] = iq_front_param_init;
	#endif
	#endif

	sie1_reg_addr = IOREMAP(SIE_BASE_ADDR, SIE_SIZE);
	sie2_reg_addr = IOREMAP(SIE2_BASE_ADDR, SIE_SIZE);
	sie3_reg_addr = IOREMAP(SIE3_BASE_ADDR, SIE_SIZE);
	sie4_reg_addr = IOREMAP(SIE4_BASE_ADDR, SIE_SIZE);
	sie5_reg_addr = IOREMAP(SIE5_BASE_ADDR, SIE_SIZE);
	ife_reg_addr = IOREMAP(IFE_BASE_ADDR, IFE_SIZE);
	ipe_reg_addr = IOREMAP(IPE_BASE_ADDR, IPE_SIZE);
	ime_reg_addr = IOREMAP(IME_BASE_ADDR, IME_SIZE);

	return 0;
}

INT32 iq_builtin_uninit(UINT32 id)
{
	if (gamma_buffer[id]) {
		isp_builtin_kmem_free(gamma_buffer[id]);
		gamma_buffer[id] = NULL;
	}

	if (ycurve_buffer[id]) {
		isp_builtin_kmem_free(ycurve_buffer[id]);
		ycurve_buffer[id] = NULL;
	}

	#if DPC_ENABLE
	if (dpc_buffer[id]) {
		isp_builtin_kmem_free(dpc_buffer[id]);
		dpc_buffer[id] = NULL;
	}
	#endif

	#if ECS_ENABLE
	if (ecs_buffer[id]) {
		isp_builtin_kmem_free(ecs_buffer[id]);
		ecs_buffer[id] = NULL;
	}
	#endif

	IOUNMAP(sie1_reg_addr);
	IOUNMAP(sie2_reg_addr);
	IOUNMAP(sie3_reg_addr);
	IOUNMAP(sie4_reg_addr);
	IOUNMAP(sie5_reg_addr);
	IOUNMAP(ife_reg_addr);
	IOUNMAP(ipe_reg_addr);
	IOUNMAP(ime_reg_addr);

	return 0;
}

INT32 iq_builtin_trig(UINT32 id, ISP_BUILTIN_IQ_TRIG_MSG msg)
{
	UINT32 gain = 0, gain_enh = 0, dgain = 0;
	UINT32 ct = 0, lv = 0;
	ISP_BUILTIN_AE_STATUS ae_status = ISP_BUILTIN_AE_STATUS_STABLE;
	ISP_BUILTIN_CGAIN *awb_cgain = NULL;
	UINT32 ui_nr_ratio;
	UINT32 ui_3dnr_ratio;
	UINT32 ui_sharp_ratio;
	UINT32 ui_sat_ratio;
	UINT32 ui_con_ratio;
	UINT32 ui_bright_ratio;
	UINT32 ui_night_mode;
	UINT32 ui_shdr_tone_lv = 50;
	UINT32 ui_gamma_lv = 50;
	UINT32 i, iso_idx = 0, iso_idx_enh = 0;

	gain = isp_builtin_get_total_gain(id);
	if (gain == 0) {
		gain = 100;
		DBG_DUMP("isp builtin get total gain fail \r\n");
	}
	dgain = isp_builtin_get_dgain(id);
	if (dgain == 0) {
		dgain = 128;
		DBG_DUMP("isp builtin get dgain fail \r\n");
	}
	ct = isp_builtin_get_ct(id);
	if(ct == 0){
		ct = 6000;
		DBG_DUMP("isp builtin get ct fail \r\n");
	}
	lv = isp_builtin_get_lv(id);
	if(lv == 0){
		lv = 8 * LV_BASE;
		DBG_DUMP("isp builtin get lv fail \r\n");
	}
	ae_status = isp_builtin_get_ae_status(id);
	awb_cgain = isp_builtin_get_cgain(id);
	if ((awb_cgain->r == 0) | (awb_cgain->g == 0) | (awb_cgain->b == 0)) {
		awb_cgain->r = 256;
		awb_cgain->g = 256;
		awb_cgain->b = 256;
		DBG_DUMP("isp builtin get cgain fail \r\n");
	}

	ui_nr_ratio = isp_builtin_get_nr_lv(id);
	ui_3dnr_ratio = isp_builtin_get_3dnr_lv(id);
	ui_sharp_ratio = isp_builtin_get_sharpness_lv(id);
	ui_sat_ratio = isp_builtin_get_saturation_lv(id);
	ui_con_ratio = isp_builtin_get_contrast_lv(id);
	ui_bright_ratio = isp_builtin_get_brightness_lv(id);
	ui_night_mode = isp_builtin_get_night_mode(id);

	iq_front_factor[id].rth_nlm = iq_builtin_intpl(iq_frm_cnt[id], (INT32)iq_front_param[id].rth_nlm, (INT32)100, 0, iq_front_param[id].smooth_thr);
	iq_front_factor[id].rth_nlm_lut = iq_builtin_intpl(iq_frm_cnt[id], (INT32)iq_front_param[id].rth_nlm_lut, (INT32)100, 0, iq_front_param[id].smooth_thr);
	iq_front_factor[id].clamp_th = iq_builtin_intpl(iq_frm_cnt[id], (INT32)iq_front_param[id].clamp_th, (INT32)100, 0, iq_front_param[id].smooth_thr);
	iq_front_factor[id].c_con = iq_builtin_intpl(iq_frm_cnt[id], (INT32)iq_front_param[id].c_con, (INT32)100, 0, iq_front_param[id].smooth_thr);
	iq_front_factor[id].overshoot = iq_builtin_intpl(iq_frm_cnt[id], (INT32)iq_front_param[id].overshoot, (INT32)100, 0, iq_front_param[id].smooth_thr);

	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain < iso_map_tab[i].iso) {
			if (gain < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx = iso_map_tab[i - 1].index;
			} else {
				iso_idx = iso_map_tab[i].index;
			}
			break;
		}
	}

	if ((isp_builtin_get_func_en(id) & ISP_BUILTIN_FUNC_WDR) && (iq_param[id]->wdr->enable == TRUE) && (iq_param[id]->wdr_enh->enable == TRUE)) {
		gain_enh = gain * iq_param[id]->wdr_enh->enh_ratio[IQ_CLAMP(final_wdr_strength[id] >> 4, 0, IQ_WDR_STR_LEN - 1)] >> 7;
	} else {
		gain_enh = gain;
	}
	for (i = 1; i < IQ_GAIN_ID_MAX_NUM; i++) {
		if (gain_enh < iso_map_tab[i].iso) {
			if (gain_enh < (iso_map_tab[i].iso + iso_map_tab[i - 1].iso) / 2) {
				iso_idx_enh = iso_map_tab[i - 1].index;
			} else {
				iso_idx_enh = iso_map_tab[i].index;
			}
			break;
		}
	}

	iq_front_factor[id].rth_nlm = ((iq_front_factor[id].rth_nlm * iq_front_param[id].iso_w[iso_idx]) + (100 * (100 - iq_front_param[id].iso_w[iso_idx]))) / 100;
	iq_front_factor[id].rth_nlm_lut = ((iq_front_factor[id].rth_nlm_lut * iq_front_param[id].iso_w[iso_idx]) + (100 * (100 - iq_front_param[id].iso_w[iso_idx]))) / 100;
	iq_front_factor[id].clamp_th = ((iq_front_factor[id].clamp_th * iq_front_param[id].iso_w[iso_idx]) + (100 * (100 - iq_front_param[id].iso_w[iso_idx]))) / 100;
	iq_front_factor[id].c_con = ((iq_front_factor[id].c_con * iq_front_param[id].iso_w[iso_idx]) + (100 * (100 - iq_front_param[id].iso_w[iso_idx]))) / 100;
	iq_front_factor[id].overshoot = ((iq_front_factor[id].overshoot * iq_front_param[id].iso_w[iso_idx_enh]) + (100 * (100 - iq_front_param[id].iso_w[iso_idx_enh]))) / 100;

	#if IQ_BUILTIN_DBG_MSG
	if (iq_frm_cnt[id] < iq_front_param[id].smooth_thr) {
		DBG_DUMP("id = %d, cnt = %d, gain = %d, gain_enh = %d, iso_idx = %d, iso_idx_enh = %d, iso_w = %d, iso_w_enh = %d \n"
			, id, iq_frm_cnt[id], gain, gain_enh, iso_idx, iso_idx_enh, iq_front_param[id].iso_w[iso_idx], iq_front_param[id].iso_w[iso_idx_enh]);
		DBG_DUMP("nlm = %3d, lut = %3d, clamp = %3d, c_con = %3d, overshoot = %d\n"
			, iq_front_factor[id].rth_nlm, iq_front_factor[id].rth_nlm_lut, iq_front_factor[id].clamp_th, iq_front_factor[id].c_con, iq_front_factor[id].overshoot);
	}
	#endif

	if (msg == ISP_BUILTIN_IQ_TRIG_SIE_RESET) {
		iq_builtin_set_sie_ob(id, gain, iq_param[id]);
		#if DPC_ENABLE
		iq_builtin_set_dpc(id, iq_param[id]);
		#endif
		#if ECS_ENABLE
		iq_builtin_set_ecs(id, ct, iq_param[id]);
		#endif
		iq_builtin_set_dgain(id, dgain);
		iq_builtin_set_companding(iq_param[id]);
	}

	if (msg == ISP_BUILTIN_IQ_TRIG_IPP_RESET) {
		//iq_builtin_set_fpn(id, gain, iq_param[id]);
		iq_builtin_set_ife_ob(id, gain, iq_param[id]);
		iq_builtin_set_cgain(id, awb_cgain, iq_param[id], gain, ui_night_mode);
		iq_builtin_set_shdr(id, gain, iq_param[id]);
		iq_builtin_set_nr(id, msg, gain, awb_cgain, iq_param[id], ui_nr_ratio);
		iq_builtin_set_nr_lca(id, msg, gain_enh, iq_param[id]);
		iq_builtin_set_vig(gain, iq_param[id]);
		iq_builtin_set_tonecurve(id, lv, iq_param[id], ui_shdr_tone_lv);
		iq_builtin_set_wdr(id, msg, gain, iq_param[id]);
		iq_builtin_set_cfa(gain, iq_param[id]);
		iq_builtin_set_pfr(gain, iq_param[id]);
		iq_builtin_set_ccm(id, ct, iq_param[id], ui_night_mode);
		iq_builtin_set_gamma(id, lv, iq_param[id], ui_gamma_lv);
		iq_builtin_set_defog(id, msg, gain, iq_param[id]);
		iq_builtin_set_cst(id, gain, iq_param[id]);
		iq_builtin_set_ycurve(id, iq_param[id]);
		iq_builtin_set_color(id, gain, iq_param[id], ui_night_mode, ui_con_ratio, ui_sat_ratio);
		iq_builtin_set_contrast(id, msg, gain, iq_param[id], ui_bright_ratio, ui_con_ratio);
		iq_builtin_set_edge(id, gain_enh, iq_param[id], ui_sharp_ratio, ae_status);
		iq_builtin_set_3dnr(id, msg, gain_enh, iq_param[id], ui_night_mode, ui_3dnr_ratio);
		iq_builtin_set_post_sharpen_1(id, gain_enh, iq_param[id]);
		iq_builtin_get_subimg_addr();
	}

	if (msg == ISP_BUILTIN_IQ_TRIG_CGAIN) {
		iq_builtin_set_cgain(id, awb_cgain, iq_param[id], gain, ui_night_mode);
	}

	if (msg == ISP_BUILTIN_IQ_TRIG_SIE) {
		iq_builtin_set_sie_ob(id, gain, iq_param[id]);
		#if DPC_ENABLE
		iq_builtin_set_dpc(id, iq_param[id]);
		#endif
		#if ECS_ENABLE
		iq_builtin_set_ecs(id, ct, iq_param[id]);
		#endif
		iq_builtin_set_dgain(id, dgain);
		iq_builtin_set_companding(iq_param[id]);
	}

	if (msg == ISP_BUILTIN_IQ_TRIG_IPP) {
		//iq_builtin_set_fpn(id, gain, iq_param[id]);
		iq_builtin_set_ife_ob(id, gain, iq_param[id]);

		iq_builtin_set_shdr(id, gain, iq_param[id]);
		iq_builtin_set_nr(id, msg, gain, awb_cgain, iq_param[id], ui_nr_ratio);
		iq_builtin_set_nr_lca(id, msg, gain_enh, iq_param[id]);
		iq_builtin_set_vig(gain, iq_param[id]);
		iq_builtin_set_tonecurve(id, lv, iq_param[id], ui_shdr_tone_lv);
		iq_builtin_set_wdr(id, msg, gain, iq_param[id]);
		iq_builtin_set_cfa(gain, iq_param[id]);
		iq_builtin_set_pfr(gain, iq_param[id]);
		iq_builtin_set_ccm(id, ct, iq_param[id], ui_night_mode);
		iq_builtin_set_gamma(id, lv, iq_param[id], ui_gamma_lv);
		iq_builtin_set_defog(id, msg, gain, iq_param[id]);
		iq_builtin_set_cst(id, gain, iq_param[id]);
		iq_builtin_set_ycurve(id, iq_param[id]);
		iq_builtin_set_color(id, gain, iq_param[id], ui_night_mode, ui_con_ratio, ui_sat_ratio);
		iq_builtin_set_contrast(id, msg, gain, iq_param[id], ui_bright_ratio, ui_con_ratio);
		iq_builtin_set_edge(id, gain_enh, iq_param[id], ui_sharp_ratio, ae_status);
		iq_builtin_set_3dnr(id, msg, gain_enh, iq_param[id], ui_night_mode, ui_3dnr_ratio);
		iq_builtin_set_post_sharpen_1(id, gain_enh, iq_param[id]);
		iq_frm_cnt[id]++;

		#if IQ_BUILTIN_REG_MSG
		if (iq_frm_cnt[id] == 70) {
			DBG_DUMP("================builtin================");
			DBG_DUMP("IFE(%d) :", id);
			debug_dumpmem(IFE_BASE_ADDR, IFE_SIZE);
			DBG_DUMP("IPE(%d) :", id);
			debug_dumpmem(IPE_BASE_ADDR, IPE_SIZE);
			DBG_DUMP("IME(%d) :", id);
			debug_dumpmem(IME_BASE_ADDR, IME_SIZE);
			DBG_DUMP("ENC(%d) :", id);
			debug_dumpmem(ENC_BASE_ADDR, ENC_SIZE);
		}
		#endif
	}

	if (msg == ISP_BUILTIN_IQ_TRIG_ENC) {
		iq_builtin_set_post_sharpen_2(id, gain_enh, iq_param[id]);
	}

	return 0;
}

#endif

