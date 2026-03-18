#ifndef _IQ_REG_H_
#define _IQ_REG_H_

#if defined(__KERNEL__)
#include "linux/soc/nvt/rcw_macro.h"
#else
#include "rcw_macro.h"
#endif

// NOTE: NT9853x SIE
/*
    PATGEN_EN      :    [0x0, 0x1],            bits : 1
    DVI_EN         :    [0x0, 0x1],            bits : 2
    OB_AVG_EN      :    [0x0, 0x1],            bits : 3
    OB_SUB_SEL     :    [0x0, 0x1],            bits : 4
    OB_BYPASS_EN   :    [0x0, 0x1],            bits : 5
    OB_FRAME_AVG_EN:    [0x0, 0x1],            bits : 6
    OB_PLANE_SUB_EN:    [0x0, 0x1],            bits : 7
    SHDR_YOUT_EN   :    [0x0, 0x1],            bits : 8
    CGAIN_EN       :    [0x0, 0x1],            bits : 9
    GRID_LINE_EN   :    [0x0, 0x1],            bits : 10
    DPC_EN         :    [0x0, 0x1],            bits : 11
    PFPC_EN        :    [0x0, 0x1],            bits : 13
    ECS_EN         :    [0x0, 0x1],            bits : 15
    DGAIN_EN       :    [0x0, 0x1],            bits : 16
    BS_H_EN        :    [0x0, 0x1],            bits : 17
    BS_V_EN        :    [0x0, 0x1],            bits : 18
    RAWENC_EN      :    [0x0, 0x1],            bits : 19
    STCS_HISTO_Y_EN:    [0x0, 0x1],            bits : 21
    STCS_LA_EN     :    [0x0, 0x1],            bits : 23
    STCS_CA_EN     :    [0x0, 0x1],            bits : 24
    STCS_VA_EN     :    [0x0, 0x1],            bits : 25
    STCS_ETH_EN    :    [0x0, 0x1],            bits : 26
    DRAM_OUT0_EN   :    [0x0, 0x1],            bits : 27
    DRAM_OUT3_EN   :    [0x0, 0x1],            bits : 28
    BAYER_FORMAT   :    [0x0, 0x1],            bits : 30_29
*/
#define R4_ENGINE_FUNCTION_OFS 0x0004
REGDEF_BEGIN(R4_ENGINE_FUNCTION)
REGDEF_BIT(,                           1)
REGDEF_BIT(PATGEN_EN,                  1)
REGDEF_BIT(DVI_EN,                     1)
REGDEF_BIT(OB_AVG_EN,                  1)
REGDEF_BIT(OB_SUB_SEL,                 1)
REGDEF_BIT(OB_BYPASS_EN,               1)
REGDEF_BIT(MASK0,                      1)
REGDEF_BIT(MASK1,                      1)
REGDEF_BIT(MASK2,                      1)
REGDEF_BIT(MASK3,                      1)
REGDEF_BIT(DVS_DECOMP_EN,              1)
REGDEF_BIT(DPC_EN,                     1)
REGDEF_BIT(,                           1)
REGDEF_BIT(PFPC_EN,                    1)
REGDEF_BIT(,                           1)
REGDEF_BIT(ECS_EN,                     1)
REGDEF_BIT(DGAIN_EN,                   1)
REGDEF_BIT(BS_H_EN,                    1)
REGDEF_BIT(BS_V_EN,                    1)
REGDEF_BIT(RAWENC_EN,                  1)
REGDEF_BIT(CGAIN_EN,                   1)
REGDEF_BIT(STCS_HISTO_Y_EN,            1)
REGDEF_BIT(,                           1)
REGDEF_BIT(STCS_LA_EN,                 1)
REGDEF_BIT(STCS_CA_EN,                 1)
REGDEF_BIT(STCS_DGAIN_EN,              1)
REGDEF_BIT(EMBDATA_EXTRACT_EN,         1)
REGDEF_BIT(DRAM_OUT0_EN,               1)
REGDEF_BIT(COMPANDING_EN,              1)
REGDEF_BIT(BAYER_FORMAT,               2)
REGDEF_BIT(ECS_BAYER_MODE,             1)
REGDEF_END(R4_ENGINE_FUNCTION)

/*
    CRP_SZX:    [0x0, 0x3fff],            bits : 13_0
    CRP_SZY:    [0x0, 0x3fff],            bits : 29_16
*/
#define R38_ENGINE_TIMING_OFS 0x0038
REGDEF_BEGIN(R38_ENGINE_TIMING)
REGDEF_BIT(CRP_SZX,        14)
REGDEF_BIT(,         2)
REGDEF_BIT(CRP_SZY,        14)
REGDEF_END(R38_ENGINE_TIMING)

/*
    DRAM_IN1_SAI:    [0x0, 0x7ffffff],            bits : 28_2
*/
#define R48_ENGINE_DRAM_OFS 0x0048
REGDEF_BEGIN(R48_ENGINE_DRAM)
REGDEF_BIT(,         2)
REGDEF_BIT(DRAM_IN1_SAI,        29)
REGDEF_END(R48_ENGINE_DRAM)

/*
    DRAM_IN2_SAI:    [0x0, 0x7ffffff],            bits : 28_2
*/
#define R50_ENGINE_DRAM_OFS 0x0050
REGDEF_BEGIN(R50_ENGINE_DRAM)
REGDEF_BIT(,         2)
REGDEF_BIT(DRAM_IN2_SAI,        29)
REGDEF_END(R50_ENGINE_DRAM)

/*
    STCS_VIG_EN               :    [0x0, 0x1],            bits : 1
    STCS_LA_CG_EN             :    [0x0, 0x1],            bits : 2
    STCS_GAMMA_1_EN           :    [0x0, 0x1],            bits : 3
    STCS_GAMMA_2_EN           :    [0x0, 0x1],            bits : 4
    STCS_HISTO_Y_SEL          :    [0x0, 0x1],            bits : 5
    STCS_CA_TH_EN             :    [0x0, 0x1],            bits : 6
    STCS_VA_CG_EN             :    [0x0, 0x1],            bits : 7
    STCS_LA_RGB2Y_SEL         :    [0x0, 0x1],            bits : 8
    STCS_VA_VUG_PAD_LINE_INTVL:    [0x0, 0x3],            bits : 31_30
*/
#define R1E8_STCS_OFS 0x01e8
REGDEF_BEGIN(R1E8_STCS)
REGDEF_BIT(,        1)
REGDEF_BIT(STCS_VIG_EN,        1)
REGDEF_BIT(STCS_LA_CG_EN,        1)
REGDEF_BIT(STCS_GAMMA_1_EN,        1)
REGDEF_BIT(STCS_GAMMA_2_EN,        1)
REGDEF_BIT(STCS_HISTO_Y_SEL,        1)
REGDEF_BIT(STCS_CA_TH_EN,        1)
REGDEF_BIT(STCS_VA_CG_EN,        1)
REGDEF_BIT(STCS_LA_RGB2Y_SEL,        1)
REGDEF_BIT(STCS_LA_RGB2Y1_MOD,        1)
REGDEF_BIT(STCS_LA_RGB2Y2_MOD,        1)
REGDEF_BIT(STCS_LA_TH_EN,        1)
REGDEF_BIT(STCS_CA_ACCM_SRC,		 1)
REGDEF_BIT(,		3)
REGDEF_BIT(STCS_COMPANDING_SHIFT,	 4)
REGDEF_BIT(,       12)
REGDEF_END(R1E8_STCS)

/*
    SMPL_X_FACT:    [0x0, 0x1fff],            bits : 12_0
    SMPL_Y_FACT:    [0x0, 0x1fff],            bits : 28_16
*/
#define R224_STCS_CA_OFS 0x0224
REGDEF_BEGIN(R224_STCS_CA)
REGDEF_BIT(SMPL_X_FACT,        14)
REGDEF_BIT(,         2)
REGDEF_BIT(SMPL_Y_FACT,        14)
REGDEF_END(R224_STCS_CA)

/*
    CA_CROP_SZX:    [0x0, 0x3fff],            bits : 13_0
    CA_CROP_SZY:    [0x0, 0x3fff],            bits : 29_16
*/
#define R228_STCS_CA_OFS 0x0228
REGDEF_BEGIN(R228_STCS_CA)
REGDEF_BIT(CA_CROP_SZX,        14)
REGDEF_BIT(,         2)
REGDEF_BIT(CA_CROP_SZY,        14)
REGDEF_END(R228_STCS_CA)

/*
    CA_WIN_SZX:    [0x0, 0xff],            bits : 7_0
    CA_WIN_SZY:    [0x0, 0xff],            bits : 15_8
*/
#define R23C_STCS_CA_OFS 0x023c
REGDEF_BEGIN(R23C_STCS_CA)
REGDEF_BIT(CA_WIN_SZX,        8)
REGDEF_BIT(CA_WIN_SZY,        8)
REGDEF_END(R23C_STCS_CA)

/*
    CA_WIN_NUMX:    [0x0, 0x1f],            bits : 4_0
    CA_WIN_NUMY:    [0x0, 0x1f],            bits : 12_8
*/
#define R240_STCS_CA_OFS 0x0240
REGDEF_BEGIN(R240_STCS_CA)
REGDEF_BIT(CA_WIN_NUMX,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(CA_WIN_NUMY,        5)
REGDEF_END(R240_STCS_CA)

/*
    LA_WIN_SZX:    [0x0, 0xff],            bits : 7_0
    LA_WIN_SZY:    [0x0, 0xff],            bits : 15_8
*/
#define R2AC_STCS_LA_OFS 0x02ac
REGDEF_BEGIN(R2AC_STCS_LA)
REGDEF_BIT(LA_WIN_SZX,        8)
REGDEF_BIT(LA_WIN_SZY,        8)
REGDEF_END(R2AC_STCS_LA)

/*
    LA_WIN_NUMX:    [0x0, 0x1f],            bits : 4_0
    LA_WIN_NUMY:    [0x0, 0x1f],            bits : 12_8
    LA1_WIN_SUM:    [0x0, 0x3ffff],            bits : 31_14
*/
#define R2B0_STCS_LA_OFS 0x02b0
REGDEF_BEGIN(R2B0_STCS_LA)
REGDEF_BIT(LA_WIN_NUMX,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(LA_WIN_NUMY,        5)
REGDEF_BIT(,        1)
REGDEF_BIT(LA1_WIN_SUM,        18)
REGDEF_END(R2B0_STCS_LA)

//=============================================================================
// SIE OB
//=============================================================================
/*
    OB_OFS   :    [0x0, 0x7ff],            bits : 10_0
    OBDT_GAIN:    [0x0, 0x3f] ,            bits : 21_16
*/
#define RBC_BASIC_OB_OFS 0x00bc
REGDEF_BEGIN(RBC_BASIC_OB)
REGDEF_BIT(OB_OFS,        11)
REGDEF_BIT(,         5)
REGDEF_BIT(OBDT_GAIN,         6)
REGDEF_END(RBC_BASIC_OB)

/*
    STCS_CA_OB_OFS:    [0x0, 0xFff],            bits : 11_0
    STCS_LA_OB_OFS:    [0x0, 0xFff],            bits : 23_12
*/
#define R2A8_STCS_OB_OFS 0x02a8
REGDEF_BEGIN(R2A8_RESERVED)
REGDEF_BIT(STCS_CA_OB_OFS,        12)
REGDEF_BIT(STCS_LA_OB_OFS,        12)
REGDEF_END(R2A8_STCS_OB)

//=============================================================================
// SIE Dgain
//=============================================================================
/*
    DGAIN_GAIN:    [0x0, 0x3ff],            bits : 9_0
*/
#define R15C_BASIC_DGAIN_OFS 0x015c
REGDEF_BEGIN(R15C_BASIC_DGAIN)
REGDEF_BIT(DGAIN_GAIN,        16)
REGDEF_END(R15C_BASIC_DGAIN)

//=============================================================================
// SIE Cgain
//=============================================================================
/*
    CGAIN_RGAIN:    [0x0, 0x3ff],            bits : 9_0
    CGAIN_GRGAIN:    [0x0, 0x3ff],            bits : 25_16
*/
#define R1B4_BASIC_CG_OFS 0x01b4
REGDEF_BEGIN(R1B4_BASIC_CG)
REGDEF_BIT(CGAIN_RGAIN,    10)
REGDEF_BIT(,        6)
REGDEF_BIT(CGAIN_GRGAIN,    10)
REGDEF_END(R1B4_BASIC_CG)

/*
    CGAIN_GBGAIN    :    [0x0, 0x3ff],          bits : 9_0
    CGAIN_BGAIN    :    [0x0, 0x3ff],          bits : 25_16
*/
#define R1B8_BASIC_CG_OFS 0x01b8
REGDEF_BEGIN(R1B8_BASIC_CG)
REGDEF_BIT(CGAIN_GBGAIN,   10)
REGDEF_BIT(,        6)
REGDEF_BIT(CGAIN_BGAIN,    10)
REGDEF_END(R1B8_BASIC_CG)

/*
    CGAIN_IRGAIN    :    [0x0, 0x3ff],          bits : 25_16
    CGAIN_LEVEL_SEL:    [0x0, 0x1],            bits : 28
*/
#define R1BC_BASIC_CG_OFS 0x01bc
REGDEF_BEGIN(R1BC_BASIC_CG)
REGDEF_BIT(CGAIN_IRGAIN, 10)
REGDEF_BIT(,         6)
REGDEF_BIT(CGAIN_LEVEL_SEL,  1)
REGDEF_END(R1BC_BASIC_CG)

/*
    LA_CG_RGAIN       :    [0x0, 0x3ff],            bits : 9_0
    LA_CG_GGAIN       :    [0x0, 0x3ff],            bits : 25_16
*/
#define R1EC_STCS_LA_CG_OFS 0x01ec
REGDEF_BEGIN(R1EC_STCS_LA_CG)
REGDEF_BIT(LA_CG_RGAIN,        10)
REGDEF_BIT(,         6)
REGDEF_BIT(LA_CG_GGAIN,        10)
REGDEF_END(R1EC_STCS_LA_CG)

/*
    LA_CG_BGAIN       :    [0x0, 0x3ff],            bits : 9_0
    LA_CG_IRGAIN      :    [0x0, 0x3ff],            bits : 25_16
*/
#define R1F0_STCS_LA_CG_OFS 0x01f0
REGDEF_BEGIN(R1F0_STCS_LA_CG)
REGDEF_BIT(LA_CG_BGAIN,        10)
REGDEF_BIT(,         6)
REGDEF_BIT(LA_CG_IRGAIN,        10)
REGDEF_END(R1F0_STCS_LA_CG)

//=============================================================================
// SIE Companding
//=============================================================================
/*
    OBP_TBL_tmp4:    [0x0, 0xffffffff],            bits : 31_0
*/
#define R470_COMP_OFS 0x0470
REGDEF_BEGIN(R470_COMP)
REGDEF_BIT(COMP_FCURVE_L0,        12)
REGDEF_BIT(              ,         4)
REGDEF_BIT(COMP_FCURVE_L1, 		  12)
REGDEF_END(R470_COMP)

/*
    OBP_TBL_tmp36:    [0x0, 0xffffffff],            bits : 31_0
*/
#define R4F0_COMP_OFS 0x04f0
REGDEF_BEGIN(R4F0_COMP)
REGDEF_BIT(COMP_FCURVE_L64,		  12)
REGDEF_END(R4F0_COMP)

/*
    OBP_TBL_tmp37:    [0x0, 0xffffffff],            bits : 31_0
*/
#define R4F4_COMP_OFS 0x04f4
REGDEF_BEGIN(R4F4_COMP)
REGDEF_BIT(COMP_FCURVE_M0,	  12)
REGDEF_BIT( 			 ,	   4)
REGDEF_BIT(COMP_FCURVE_M1,	  12)
REGDEF_END(R4F4_COMP)

/*
    OBP_TBL_tmp45:    [0x0, 0xffffffff],            bits : 31_0
*/
#define R514_COMP_OFS 0x0514
REGDEF_BEGIN(R514_COMP)
REGDEF_BIT(COMP_FCURVE_M16,   12)
REGDEF_END(R514_COMP)

/*
    OBP_TBL_tmp46:    [0x0, 0xffffffff],            bits : 31_0
*/
#define R518_COMP_OFS 0x0518
REGDEF_BEGIN(R518_COMP)
REGDEF_BIT(COMP_FCURVE_R0,   12)
REGDEF_BIT( 			 ,	   4)
REGDEF_BIT(COMP_FCURVE_R1,   12)
REGDEF_END(R518_COMP)

/*
    OBP_TBL_tmp54:    [0x0, 0xffffffff],            bits : 31_0
*/
#define R538_COMP_OFS 0x0538
REGDEF_BEGIN(R538_COMP)
REGDEF_BIT( 			 ,		16)
REGDEF_BIT(COMP_FCURVE_EV_FMT,	 1)
REGDEF_END(R538_COMP)

/*
    OBP_TBL_tmp56:    [0x0, 0xffffffff],            bits : 31_0
*/
#define R540_DECOMP_OFS 0x0540
REGDEF_BEGIN(R540_DECOMP)
REGDEF_BIT(DECOMP_KPX0,		  12)
REGDEF_BIT(DECOMP_KPY0,		  20)
REGDEF_END(R540_DECOMP)

/*
    RESERVED:    [0x0, 0xffffffff],            bits : 31_0
*/
#define R5C0_DECOMP_OFS 0x05c0
REGDEF_BEGIN(R5C0_DECOMP)
REGDEF_BIT(DECOMP_GAIN0,	  10)
REGDEF_BIT(DECOMP_SB0,	       5)
REGDEF_BIT( 		   ,	   1)
REGDEF_BIT(DECOMP_GAIN1,	  10)
REGDEF_BIT(DECOMP_SB1,		   5)
REGDEF_END(R5C0_DECOMP)


//=============================================================================//
//                                                                             //
//                                 NT98538 IFE                                 //
//                                                                             //
//=============================================================================//
	/*
		IFE_MODE		   :	[0x0, 0x3], 		bits : 1_0
		INBIT_16_FMT_SEL   :	[0x0, 0x1], 		bits : 4
		INBIT_DEPTH 	   :	[0x0, 0x3], 		bits : 6_5
		OUTBIT_DEPTH	   :	[0x0, 0x3], 		bits : 8_7
		CFAPAT			   :	[0x0, 0x7], 		bits : 11_9
		IFE_FILT_MODE	   :	[0x0, 0x1], 		bits : 12
		IFE_OUTL_EN 	   :	[0x0, 0x1], 		bits : 13
		IFE_FILTER_EN	   :	[0x0, 0x1], 		bits : 14
		IFE_CGAIN_EN	   :	[0x0, 0x1], 		bits : 15
		IFE_VIG_EN		   :	[0x0, 0x1], 		bits : 16
		IFE_GBAL_EN 	   :	[0x0, 0x1], 		bits : 17
		IFE_BINNING 	   :	[0x0, 0x7], 		bits : 20_18
		BAYER_FORMAT	   :	[0x0, 0x1], 		bits : 21
		IFE_RGBIR_RB_NRFILL:	[0x0, 0x1], 		bits : 22
		IFE_BILAT_TH_EN    :	[0x0, 0x1], 		bits : 23
		IFE_DGAIN_EN	   :	[0x0, 0x1], 		bits : 24
		IFE_F_CG_EN 	   :	[0x0, 0x1], 		bits : 25
		IFE_F_FUSION_EN    :	[0x0, 0x1], 		bits : 26
		IFE_F_FUSION_FNUM  :	[0x0, 0x3], 		bits : 28_27
		IFE_F_FC_EN 	   :	[0x0, 0x1], 		bits : 29
		MIRROR_EN		   :	[0x0, 0x1], 		bits : 30
		IFE_R_DECODE_EN    :	[0x0, 0x1], 		bits : 31
	*/
#define CONTROL_REGISTER_OFS 0x0004
	REGDEF_BEGIN(CONTROL_REGISTER)
	REGDEF_BIT(ife_mode,               2)
	REGDEF_BIT(ife_input_format,       1)
	REGDEF_BIT(ife_subout_sel,         1)
	REGDEF_BIT(inbit_16_fmt_sel,       1)
	REGDEF_BIT(inbit_depth,            2)
	REGDEF_BIT(outbit_depth,           2)
	REGDEF_BIT(cfapat,                 3)
	REGDEF_BIT(filt_mode,              1)
	REGDEF_BIT(outl_en,                1)
	REGDEF_BIT(filter_en,              1)
	REGDEF_BIT(cgain_en,               1)
	REGDEF_BIT(vig_en,                 1)
	REGDEF_BIT(gbal_en,                1)
	REGDEF_BIT(binning,                3)
	REGDEF_BIT(bayer_fmt,              1)
	REGDEF_BIT(rgbir_rb_nrfill,        1)
	REGDEF_BIT(bilat_th_en,            1)
	REGDEF_BIT(dgain_en,               1)
	REGDEF_BIT(f_cg_en,                1)
	REGDEF_BIT(f_fusion_en,            1)
	REGDEF_BIT(f_fusion_fnum,          2)
	REGDEF_BIT(f_fc_en,                1)
	REGDEF_BIT(mirror_en,              1)
	REGDEF_BIT(r_decode_en,            1)
	REGDEF_END(CONTROL_REGISTER)

	/*
		width :    [0x0, 0x3fff],			bits : 15_2
		height:    [0x0, 0x7fff],			bits : 31_17
	*/
#define SOURCE_SIZE_REGISTER_0_OFS 0x0020
	REGDEF_BEGIN(SOURCE_SIZE_REGISTER_0)
	REGDEF_BIT(,		2)
	REGDEF_BIT(width,		 14)
	REGDEF_BIT(,		1)
	REGDEF_BIT(height,		  15)
	REGDEF_END(SOURCE_SIZE_REGISTER_0)

//=============================================================================
// IFE CGain
//=============================================================================
	/*
		cgain_inv		 :	  [0x0, 0x1],			bits : 0
		cgain_hinv		 :	  [0x0, 0x1],			bits : 1
		cgain_range 	 :	  [0x0, 0x1],			bits : 2
		ife_f_cgain_range:	  [0x0, 0x1],			bits : 3
		cgain_mask		 :	  [0x0, 0xfff], 		bits : 19_8
	*/
#define IFE_COLOR_GAIN_REGISTER_0_OFS 0x0070
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_0)
	REGDEF_BIT(cgain_inv,		 1)
	REGDEF_BIT(cgain_hinv,		  1)
	REGDEF_BIT(cgain_range, 	   1)
	REGDEF_BIT(ife_f_cgain_range,		 1)
	REGDEF_BIT(,		4)
	REGDEF_BIT(cgain_mask,		  12)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_0)

	/*
		ife_cgain_r :	 [0x0, 0x3ff],			bits : 9_0
		ife_cgain_gr:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_1_OFS 0x0074
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_1)
	REGDEF_BIT(ife_cgain_r, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_cgain_gr,		10)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_1)

	/*
		ife_cgain_gb:	 [0x0, 0x3ff],			bits : 9_0
		ife_cgain_b :	 [0x0, 0x3ff],			bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_2_OFS 0x0078
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_2)
	REGDEF_BIT(ife_cgain_gb,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_cgain_b, 	   10)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_2)

		/*
			ife_cgain_ir:	 [0x0, 0x3ff],			bits : 9_0
		*/
#define IFE_COLOR_GAIN_REGISTER_3_OFS 0x007c
		REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_3)
		REGDEF_BIT(ife_cgain_ir,		10)
		REGDEF_END(IFE_COLOR_GAIN_REGISTER_3)

//=============================================================================
// IFE Fusion CGain
//=============================================================================
	/*
		ife_f_p0_cgain_r :	  [0x0, 0x3ff], 		bits : 9_0
		ife_f_p0_cgain_gr:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_4_OFS 0x0080
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_4)
	REGDEF_BIT(ife_f_p0_cgain_r,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_p0_cgain_gr,		 10)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_4)

	/*
		ife_f_p0_cgain_gb:	  [0x0, 0x3ff], 		bits : 9_0
		ife_f_p0_cgain_b :	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_5_OFS 0x0084
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_5)
	REGDEF_BIT(ife_f_p0_cgain_gb,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_p0_cgain_b,		10)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_5)

	/*
		ife_f_p0_cgain_ir:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define IFE_COLOR_GAIN_REGISTER_6_OFS 0x0088
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_6)
	REGDEF_BIT(ife_f_p0_cgain_ir,		 10)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_6)

	/*
		ife_f_p1_cgain_r :	  [0x0, 0x3ff], 		bits : 9_0
		ife_f_p1_cgain_gr:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_7_OFS 0x008c
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_7)
	REGDEF_BIT(ife_f_p1_cgain_r,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_p1_cgain_gr,		 10)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_7)

	/*
		ife_f_p1_cgain_gb:	  [0x0, 0x3ff], 		bits : 9_0
		ife_f_p1_cgain_b :	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_8_OFS 0x0090
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_8)
	REGDEF_BIT(ife_f_p1_cgain_gb,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_p1_cgain_b,		10)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_8)

	/*
		ife_f_p1_cgain_ir:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define IFE_COLOR_GAIN_REGISTER_9_OFS 0x0094
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_9)
	REGDEF_BIT(ife_f_p1_cgain_ir,		 10)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_9)

//=============================================================================
// IFE OB
//=============================================================================
	/*
		ife_cofs_r :	[0x0, 0x3ff],			bits : 9_0
		ife_cofs_gr:	[0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_0_OFS 0x00b0
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_0)
	REGDEF_BIT(ife_cofs_r,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_cofs_gr, 	   10)
	REGDEF_END(COLOR_OFFSET_REGISTER_0)

	/*
		ife_cofs_gb:	[0x0, 0x3ff],			bits : 9_0
		ife_cofs_b :	[0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_1_OFS 0x00b4
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_1)
	REGDEF_BIT(ife_cofs_gb, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_cofs_b,		  10)
	REGDEF_END(COLOR_OFFSET_REGISTER_1)

	/*
		ife_cofs_ir:	[0x0, 0x3ff],			bits : 9_0
	*/
#define COLOR_OFFSET_REGISTER_2_OFS 0x00b8
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_2)
	REGDEF_BIT(ife_cofs_ir, 	   10)
	REGDEF_END(COLOR_OFFSET_REGISTER_2)

//=============================================================================
// IFE Fusion OB
//=============================================================================
	/*
		ife_f_p0_cofs_r :	 [0x0, 0x3ff],			bits : 9_0
		ife_f_p0_cofs_gr:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_3_OFS 0x00bc
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_3)
	REGDEF_BIT(ife_f_p0_cofs_r, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_p0_cofs_gr,		10)
	REGDEF_END(COLOR_OFFSET_REGISTER_3)

	/*
		ife_f_p0_cofs_gb:	 [0x0, 0x3ff],			bits : 9_0
		ife_f_p0_cofs_b :	 [0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_4_OFS 0x00c0
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_4)
	REGDEF_BIT(ife_f_p0_cofs_gb,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_p0_cofs_b, 	   10)
	REGDEF_END(COLOR_OFFSET_REGISTER_4)

	/*
		ife_f_p0_cofs_ir:	 [0x0, 0x3ff],			bits : 9_0
	*/
#define COLOR_OFFSET_REGISTER_5_OFS 0x00c4
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_5)
	REGDEF_BIT(ife_f_p0_cofs_ir,		10)
	REGDEF_END(COLOR_OFFSET_REGISTER_5)

	/*
		ife_f_p1_cofs_r :	 [0x0, 0x3ff],			bits : 9_0
		ife_f_p1_cofs_gr:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_6_OFS 0x00c8
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_6)
	REGDEF_BIT(ife_f_p1_cofs_r, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_p1_cofs_gr,		10)
	REGDEF_END(COLOR_OFFSET_REGISTER_6)

	/*
		ife_f_p1_cofs_gb:	 [0x0, 0x3ff],			bits : 9_0
		ife_f_p1_cofs_b :	 [0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_7_OFS 0x00cc
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_7)
	REGDEF_BIT(ife_f_p1_cofs_gb,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_p1_cofs_b, 	   10)
	REGDEF_END(COLOR_OFFSET_REGISTER_7)

	/*
		ife_f_p1_cofs_ir:	 [0x0, 0x3ff],			bits : 9_0
	*/
#define COLOR_OFFSET_REGISTER_8_OFS 0x00d0
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_8)
	REGDEF_BIT(ife_f_p1_cofs_ir,		10)
	REGDEF_END(COLOR_OFFSET_REGISTER_8)

//=============================================================================
// IFE OUTLIER
//=============================================================================
	/*
		ife_outlth_bri0 :	 [0x0, 0xfff],			bits : 11_0
		ife_outlth_dark0:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_1_OFS 0x0190
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_1)
	REGDEF_BIT(ife_outlth_bri0, 	   12)
	REGDEF_BIT(ife_outlth_dark0,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_1)

	/*
		ife_outl_compensate_mode :	  [0x0, 0x3],			bits : 1_0
		ife_outl_edge_smooth_en  :	  [0x0, 0x1],			bits : 2
		ife_outl_extre_detect_idx:	  [0x0, 0x7],			bits : 6_4
		ife_outl_weight  :	  [0x0, 0xff],			bits : 15_8
		ife_outl_cnt1	 :	  [0x0, 0x1f],			bits : 20_16
		ife_outl_cnt2	 :	  [0x0, 0x1f],			bits : 28_24
	*/
#define OUTLIER_THRESHOLD_REGISTER_6_OFS 0x01a4
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_6)
		REGDEF_BIT(ife_outl_compensate_mode ,		 2)
		REGDEF_BIT(ife_outl_edge_smooth_en	,		 1)
		REGDEF_BIT( 						,		 1)
		REGDEF_BIT(ife_outl_extre_detect_idx,		 3)
		REGDEF_BIT( 						,		 1)
	REGDEF_BIT(ife_outl_weight, 	   8)
	REGDEF_BIT(ife_outl_cnt1,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_outl_cnt2,		 5)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_6)

	/*
		IFE_ORD_RANGE_BRI :    [0x0, 0x7],			bits : 2_0
		ife_ord_range_dark:    [0x0, 0x7],			bits : 6_4
		ife_ord_protect_th:    [0x0, 0x3ff],			bits : 17_8
		IFE_ORD_BLEND_W   :    [0x0, 0xff], 		bits : 27_20
	*/
#define OUTLIER_ORDER_REGISTER_0_OFS 0x01a8
	REGDEF_BEGIN(OUTLIER_ORDER_REGISTER_0)
	REGDEF_BIT(ife_ord_range_bright,	 3)
	REGDEF_BIT(,		1)
	REGDEF_BIT(ife_ord_range_dark,		  3)
	REGDEF_BIT(,		1)
	REGDEF_BIT(ife_ord_protect_th,		  10)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_ord_blend_weight,	  8)
	REGDEF_END(OUTLIER_ORDER_REGISTER_0)

//=============================================================================
// IFE NR_GBalance
//=============================================================================
	/*
		ife_gbal_str_luma_low_bnd :    [0x0, 0xfff],			bits : 11_0
		ife_gbal_edge_luma_low_bnd:    [0x0, 0xfff],			bits : 27_16
	*/
#define GBALANCE_REGISTER_OFS 0x00f4
	REGDEF_BEGIN(GBALANCE_REGISTER)
	REGDEF_BIT(ife_gbal_str_luma_low_bnd,		 12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(ife_gbal_edge_luma_low_bnd,		  12)
	REGDEF_END(GBALANCE_REGISTER)

	/*
		ife_gbal_edge_protect_en:	 [0x0, 0x1],			bits : 0
		ife_gbal_diff_thr_str	:	 [0x0, 0xfff],			bits : 19_8
		ife_gbal_diff_w_max 	:	 [0x0, 0xf],			bits : 23_20
	*/
#define GBAL_REGISTER_0_OFS 0x01b4
	REGDEF_BEGIN(GBAL_REGISTER_0)
	REGDEF_BIT(ife_gbal_edge_protect_en,		1)
	REGDEF_BIT(,		7)
	REGDEF_BIT(ife_gbal_diff_thr_str,		 12)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_diff_w_max, 	   4)
	REGDEF_END(GBAL_REGISTER_0)

	/*
		ife_gbal_edge_thr_1:	[0x0, 0xfff],			bits : 11_0
		ife_gbal_edge_thr_0:	[0x0, 0xfff],			bits : 27_16
	*/
#define GBAL_REGISTER_1_OFS 0x01b8
	REGDEF_BEGIN(GBAL_REGISTER_1)
		REGDEF_BIT(ife_gbal_edge_thr_1, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(ife_gbal_edge_thr_0, 	   12)
	REGDEF_END(GBAL_REGISTER_1)

//=============================================================================
// IFE 2DNR Filter_TH
//=============================================================================
		/*
			ife_rth_nlm_c0_0:	 [0x0, 0x3ff],			bits : 9_0
			ife_rth_nlm_c0_1:	 [0x0, 0x3ff],			bits : 25_16
		*/
#define RANGE_FILTER_REGISTER_1_OFS 0x0110
		REGDEF_BEGIN(RANGE_FILTER_REGISTER_1)
		REGDEF_BIT(ife_rth_nlm_c0_0,		10)
		REGDEF_BIT(,		6)
		REGDEF_BIT(ife_rth_nlm_c0_1,		10)
		REGDEF_END(RANGE_FILTER_REGISTER_1)

		/*
			ife_rth_nlm_c1_0:	 [0x0, 0x3ff],			bits : 9_0
			ife_rth_nlm_c1_1:	 [0x0, 0x3ff],			bits : 25_16
		*/
#define RANGE_FILTER_REGISTER_5_OFS 0x0120
		REGDEF_BEGIN(RANGE_FILTER_REGISTER_5)
		REGDEF_BIT(ife_rth_nlm_c1_0,		10)
		REGDEF_BIT(,		6)
		REGDEF_BIT(ife_rth_nlm_c1_1,		10)
		REGDEF_END(RANGE_FILTER_REGISTER_5)

		/*
			ife_rth_nlm_c2_0:	 [0x0, 0x3ff],			bits : 9_0
			ife_rth_nlm_c2_1:	 [0x0, 0x3ff],			bits : 25_16
		*/
#define RANGE_FILTER_REGISTER_9_OFS 0x0130
		REGDEF_BEGIN(RANGE_FILTER_REGISTER_9)
		REGDEF_BIT(ife_rth_nlm_c2_0,		10)
		REGDEF_BIT(,		6)
		REGDEF_BIT(ife_rth_nlm_c2_1,		10)
		REGDEF_END(RANGE_FILTER_REGISTER_9)

	/*
		ife_rth_nlm_c3_0:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c3_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_13_OFS 0x0140
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_13)
	REGDEF_BIT(ife_rth_nlm_c3_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_13)

	/*
		ife_rth_bilat_c0_0:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_17_OFS 0x0150
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_17)
	REGDEF_BIT(ife_rth_bilat_c0_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_17)

	/*
		ife_rth_bilat_c1_0:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_21_OFS 0x0160
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_21)
	REGDEF_BIT(ife_rth_bilat_c1_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_21)

	/*
		ife_rth_bilat_c2_0:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_25_OFS 0x0170
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_25)
	REGDEF_BIT(ife_rth_bilat_c2_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_25)

	/*
		ife_rth_bilat_c3_0:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_29_OFS 0x0180
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_29)
	REGDEF_BIT(ife_rth_bilat_c3_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_29)

//=============================================================================
// IFE 2DNR Filter_LUT
//=============================================================================
	/*
		ife_rth_nlm_c0_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c0_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_32_OFS 0x01c0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_32)
	REGDEF_BIT(ife_rth_nlm_c0_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_lut_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_32)

	/*
		ife_rth_nlm_c0_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_40_OFS 0x01e0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_40)
	REGDEF_BIT(ife_rth_nlm_c0_lut_16,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_40)

	/*
		ife_rth_nlm_c1_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c1_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_41_OFS 0x01e4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_41)
	REGDEF_BIT(ife_rth_nlm_c1_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_lut_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_41)

	/*
		ife_rth_nlm_c1_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_49_OFS 0x0204
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_49)
	REGDEF_BIT(ife_rth_nlm_c1_lut_16,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_49)

	/*
		ife_rth_nlm_c2_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c2_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_50_OFS 0x0208
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_50)
	REGDEF_BIT(ife_rth_nlm_c2_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_lut_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_50)

	/*
		ife_rth_nlm_c2_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_58_OFS 0x0228
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_58)
	REGDEF_BIT(ife_rth_nlm_c2_lut_16,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_58)

	/*
		ife_rth_nlm_c3_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c3_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_59_OFS 0x022c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_59)
	REGDEF_BIT(ife_rth_nlm_c3_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_lut_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_59)

	/*
		ife_rth_nlm_c3_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_67_OFS 0x024c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_67)
	REGDEF_BIT(ife_rth_nlm_c3_lut_16,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_67)

	/*
		ife_rth_bilat_c0_lut_0:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_lut_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_68_OFS 0x0250
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_68)
	REGDEF_BIT(ife_rth_bilat_c0_lut_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_lut_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_68)

	/*
		ife_rth_bilat_c0_lut_16:	[0x0, 0x3ff],			bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_76_OFS 0x0270
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_76)
	REGDEF_BIT(ife_rth_bilat_c0_lut_16, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_76)

	/*
		ife_rth_bilat_c1_lut_0:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_lut_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_77_OFS 0x0274
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_77)
	REGDEF_BIT(ife_rth_bilat_c1_lut_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_lut_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_77)

	/*
		ife_rth_bilat_c1_lut_16:	[0x0, 0x3ff],			bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_85_OFS 0x0294
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_85)
	REGDEF_BIT(ife_rth_bilat_c1_lut_16, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_85)

	/*
		ife_rth_bilat_c2_lut_0:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_lut_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_86_OFS 0x0298
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_86)
	REGDEF_BIT(ife_rth_bilat_c2_lut_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_lut_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_86)

	/*
		ife_rth_bilat_c2_lut_16:	[0x0, 0x3ff],			bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_94_OFS 0x02b8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_94)
	REGDEF_BIT(ife_rth_bilat_c2_lut_16, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_94)

	/*
		ife_rth_bilat_c3_lut_0:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_lut_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_95_OFS 0x02bc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_95)
	REGDEF_BIT(ife_rth_bilat_c3_lut_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_lut_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_95)

	/*
		ife_rth_bilat_c3_lut_16:	[0x0, 0x3ff],			bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_103_OFS 0x02dc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_103)
	REGDEF_BIT(ife_rth_bilat_c3_lut_16, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_103)

//=============================================================================
// IFE 2DNR Filter
//=============================================================================
	/*
		ife_clamp_th :	  [0x0, 0xfff], 		bits : 11_0
		ife_clamp_mul:	  [0x0, 0xff],			bits : 19_12
		ife_clamp_dlt:	  [0x0, 0xfff], 		bits : 31_20
	*/
#define OUTPUT_FILTER_REGISTER_OFS 0x0100
	REGDEF_BEGIN(OUTPUT_FILTER_REGISTER)
	REGDEF_BIT(ife_clamp_th,		12)
	REGDEF_BIT(ife_clamp_mul,		 8)
	REGDEF_BIT(ife_clamp_dlt,		 12)
	REGDEF_END(OUTPUT_FILTER_REGISTER)

	/*
		ife_bilat_w  :	  [0x0, 0xf],			bits : 3_0
		ife_rth_w	 :	  [0x0, 0xf],			bits : 7_4
		ife_bilat_th1:	  [0x0, 0x3ff], 		bits : 17_8
		ife_bilat_th2:	  [0x0, 0x3ff], 		bits : 27_18
		ife_bilat_cen_sel:[0x0, 0x1],			bits : 28
	*/
#define RANGE_FILTER_REGISTER_0_OFS 0x0104
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_0)
	REGDEF_BIT(ife_bilat_w, 	   4)
	REGDEF_BIT(ife_rth_w,		 4)
	REGDEF_BIT(ife_bilat_th1,		 10)
	REGDEF_BIT(ife_bilat_th2,		 10)
	REGDEF_BIT(ife_bilat_cen_sel,	 1)
	REGDEF_END(RANGE_FILTER_REGISTER_0)

//=============================================================================
// IFE VIG
//=============================================================================
	/*
		ife_distvgtx_c0:	[0x0, 0xffff],			bits : 15_0
		ife_distvgty_c0:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_0_OFS 0x02e0
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_0)
		REGDEF_BIT(ife_distvgtx_c0, 	   16)
		REGDEF_BIT(ife_distvgty_c0, 	   16)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_0)

	/*
		ife_distvgtx_c1:	[0x0, 0xffff],			bits : 15_0
		ife_distvgty_c1:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_1_OFS 0x02e4
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_1)
		REGDEF_BIT(ife_distvgtx_c1, 	   16)
		REGDEF_BIT(ife_distvgty_c1, 	   16)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_1)

	/*
		ife_distvgtx_c2:	[0x0, 0xffff],			bits : 15_0
		ife_distvgty_c2:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_2_OFS 0x02e8
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_2)
		REGDEF_BIT(ife_distvgtx_c2, 	   16)
		REGDEF_BIT(ife_distvgty_c2, 	   16)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_2)

	/*
		ife_distvgtx_c3:	[0x0, 0xffff],			bits : 15_0
		ife_distvgty_c3:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_3_OFS 0x02ec
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_3)
		REGDEF_BIT(ife_distvgtx_c3, 	   16)
		REGDEF_BIT(ife_distvgty_c3, 	   16)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_3)

	/*
		ife_distvgxdiv:    [0x0, 0xfff],			bits : 11_0
		ife_distvgydiv:    [0x0, 0xfff],			bits : 23_12
		ife_distgain  :    [0x0, 0x3],			bits : 29_28
	*/
#define VIGNETTE_SETTING_REGISTER_4_OFS 0x02f0
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_4)
		REGDEF_BIT(ife_distvgxdiv,		  12)
		REGDEF_BIT(ife_distvgydiv,		  12)
		REGDEF_BIT( 			 ,		  4)
		REGDEF_BIT(ife_distgain  ,		  2)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_4)

	/*
		ife_vig_c0_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c0_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_0_OFS 0x02f8
	REGDEF_BEGIN(VIGNETTE_REGISTER_0)
	REGDEF_BIT(ife_vig_c0_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c0_lut_1,		10)
	REGDEF_END(VIGNETTE_REGISTER_0)

	/*
		ife_vig_c0_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define VIGNETTE_REGISTER_8_OFS 0x0318
	REGDEF_BEGIN(VIGNETTE_REGISTER_8)
	REGDEF_BIT(ife_vig_c0_lut_16,		 10)
	REGDEF_END(VIGNETTE_REGISTER_8)

	/*
		ife_vig_c1_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c1_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_9_OFS 0x031c
	REGDEF_BEGIN(VIGNETTE_REGISTER_9)
	REGDEF_BIT(ife_vig_c1_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c1_lut_1,		10)
	REGDEF_END(VIGNETTE_REGISTER_9)

	/*
		ife_vig_c1_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define VIGNETTE_REGISTER_17_OFS 0x033c
	REGDEF_BEGIN(VIGNETTE_REGISTER_17)
	REGDEF_BIT(ife_vig_c1_lut_16,		 10)
	REGDEF_END(VIGNETTE_REGISTER_17)

	/*
		ife_vig_c2_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c2_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_18_OFS 0x0340
	REGDEF_BEGIN(VIGNETTE_REGISTER_18)
	REGDEF_BIT(ife_vig_c2_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c2_lut_1,		10)
	REGDEF_END(VIGNETTE_REGISTER_18)

	/*
		ife_vig_c2_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define VIGNETTE_REGISTER_26_OFS 0x0360
	REGDEF_BEGIN(VIGNETTE_REGISTER_26)
	REGDEF_BIT(ife_vig_c2_lut_16,		 10)
	REGDEF_END(VIGNETTE_REGISTER_26)

	/*
		ife_vig_c3_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c3_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_27_OFS 0x0364
	REGDEF_BEGIN(VIGNETTE_REGISTER_27)
	REGDEF_BIT(ife_vig_c3_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c3_lut_1,		10)
	REGDEF_END(VIGNETTE_REGISTER_27)

	/*
		ife_vig_c3_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define VIGNETTE_REGISTER_35_OFS 0x0384
	REGDEF_BEGIN(VIGNETTE_REGISTER_35)
	REGDEF_BIT(ife_vig_c3_lut_16,		 10)
	REGDEF_END(VIGNETTE_REGISTER_35)

//=============================================================================
// IFE HDR NRS
//=============================================================================
	/*
		ife_f_nrs0_en:	  [0x0, 0x1],			bits : 0
		ife_f_nrs1_en:	  [0x0, 0x1],			bits : 8
	*/
#define NRS_CONTROL_REGISTER_OFS 0x0500
	REGDEF_BEGIN(NRS_REGISTER)
	REGDEF_BIT(ife_f_nrs0_en,		 1)
	REGDEF_BIT(,	   7)
	REGDEF_BIT(ife_f_nrs1_en,		 1)
	REGDEF_END(NRS_CONTROL_REGISTER)

	/*
		IFE_F_NRS0_STR0 :	 [0x0, 0x3FF],			  bits : 9_0
		IFE_F_NRS0_STR1 :	 [0x0, 0x3FF],			  bits : 25_16
	*/
#define NRS_REGISTER0_OFS 0x0504
	REGDEF_BEGIN(NRS_REGISTER0)
	REGDEF_BIT(ife_f_nrs0_str0, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_nrs0_str1, 	   10)
	REGDEF_END(NRS_REGISTER0)

	/*
		IFE_F_NRS1_STR0 :	 [0x0, 0x3FF],			  bits : 9_0
		IFE_F_NRS1_STR1 :	 [0x0, 0x3FF],			  bits : 25_16
	*/
#define NRS_REGISTER3_OFS 0x0510
	REGDEF_BEGIN(NRS_REGISTER3)
	REGDEF_BIT(ife_f_nrs1_str0, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_nrs1_str1, 	   10)
	REGDEF_END(NRS_REGISTER3)

//=============================================================================
// IFE HDR Fusion
//=============================================================================
	/*
		IFE_F_FUSION_YM_SEL 	  :    [0x0, 0x3],			bits : 1_0
		IFE_F_FUSION_BCN_SEL (0~2):    [0x0, 0x3],			bits : 5_4
		IFE_F_FUSION_BCD_SEL (0~2):    [0x0, 0x3],			bits : 9_8
		ife_f_fusion_mode		  :    [0x0, 0x3],			bits : 13_12
		ife_f_fusion_debug_mode   :    [0x0, 0xff], 		bits : 31_28
	*/
#define RHE_FUSION_REGISTER_OFS 0x0520
	REGDEF_BEGIN(RHE_FUSION_REGISTER)
	REGDEF_BIT(ife_f_fusion_ymean_sel,		  2)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_f_fusion_normal_blend_curve_sel, 	   2)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_f_fusion_diff_blend_curve_sel,		 2)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_f_fusion_mode,		 2)
	REGDEF_BIT(,		14)
	REGDEF_BIT(ife_f_fusion_dbg_mode,		 4)
	REGDEF_END(RHE_FUSION_REGISTER)

		/*
			IFE_F_FU_EVRATIO:	 [0x0, 0x1ff],			bits : 8_0
		*/
#define RHE_EVRATIO_REGISTER_OFS 0x053c
		REGDEF_BEGIN(RHE_EVRATIO_REGISTER)
		REGDEF_BIT(ife_f_fusion_evratio,		9)
		REGDEF_END(RHE_EVRATIO_REGISTER)

//=============================================================================
// IFE HDR Fusion Curve
//=============================================================================
		/*
			IFE_F_FUSION_BCNL_P0			 :	  [0x0, 0xfff], 		   bits : 11_0
			IFE_F_FUSION_BCNL_RANGE (16~4095):	  [0x0, 0xfff], 		   bits : 27_16
		*/
#define RHE_FUSION_LONG_EXP_NORMAL_BLEND_CURVE_OFS 0x0524
		REGDEF_BEGIN(RHE_FUSION_LONG_EXPOSURE_NORMAL_BLENDING_CURVE)
		REGDEF_BIT(ife_f_fusion_long_exp_normal_blend_curve_knee_point0,		12)
		REGDEF_BIT(,		4)
		REGDEF_BIT(ife_f_fusion_long_exp_normal_blend_curve_range,		  12)
		REGDEF_END(RHE_FUSION_LONG_EXP_NORMAL_BLEND_CURVE)

		/*
			IFE_F_FUSION_BCNS_P0		  :    [0x0, 0xfff],			bits : 11_0
			IFE_F_FUSION_BCNS_RANGE 	  :    [0x0, 0xfff],			bits : 27_16
		*/
#define RHE_FUSION_SHORT_EXP_NORMAL_BLEND_CURVE_OFS 0x0528
		REGDEF_BEGIN(RHE_FUSION_SHORT_EXP_NORMAL_BLEND_CURVE)
		REGDEF_BIT(ife_f_fusion_short_exp_normal_blend_curve_knee_point0,		 12)
		REGDEF_BIT(,		4)
		REGDEF_BIT(ife_f_fusion_short_exp_normal_blend_curve_range, 	   12)
		REGDEF_END(RHE_FUSION_SHORT_EXP_NORMAL_BLEND_CURVE)

		/*
			IFE_F_FUSION_BCDL_P0		  :    [0x0, 0xfff],			bits : 11_0
			IFE_F_FUSION_BCDL_RANGE 	  :    [0x0, 0xfff],			bits : 27_16
		*/
#define RHE_FUSION_LONG_EXP_DIFF_BLEND_CURVE_OFS 0x052c
		REGDEF_BEGIN(RHE_FUSION_LONG_EXP_DIFF_BLEND_CURVE)
		REGDEF_BIT(ife_f_fusion_long_exp_diff_blend_curve_knee_point0,		  12)
		REGDEF_BIT(,		4)
		REGDEF_BIT(ife_f_fusion_long_exp_diff_blend_curve_range,		12)
		REGDEF_END(RHE_FUSION_LONG_EXP_DIFF_BLEND_CURVE)

		/*
			IFE_F_FUSION_BCDS_P0		  :    [0x0, 0xfff],			bits : 11_0
			IFE_F_FUSION_BCDS_RANGE (0~12):    [0x0, 0xf],			bits : 15_12
			IFE_F_FUSION_BCDS_P1		  :    [0x0, 0xfff],			bits : 27_16
			IFE_F_FUSION_BCDS_WEDGE 	  :    [0x0, 0x1],			bits : 31
		*/
#define RHE_FUSION_SHORT_EXP_DIFF_BLEND_CURVE_OFS 0x0530
		REGDEF_BEGIN(RHE_FUSION_SHORT_EXP_DIFF_BLEND_CURVE)
		REGDEF_BIT(ife_f_fusion_short_exp_diff_blend_curve_knee_point0, 	   12)
		REGDEF_BIT(,		4)
		REGDEF_BIT(ife_f_fusion_short_exp_diff_blend_curve_range,		 12)
		REGDEF_END(RHE_FUSION_SHORT_EXP_DIFF_BLEND_CURVE)

		/*
			IFE_F_FU_BCNL_SLPOE:	[0x0, 0xffff],			bits : 15_0
			IFE_F_FU_BCNS_SLPOE:	[0x0, 0xffff],			bits : 31_16
		*/
#define RHE_FUSION_SLOPE_0_OFS 0x0534
		REGDEF_BEGIN(RHE_FUSION_SLOPE_0)
		REGDEF_BIT(ife_f_fusion_bcnl_slope, 	   16)
		REGDEF_BIT(ife_f_fusion_bcns_slope, 	   16)
		REGDEF_END(RHE_FUSION_SLOPE_0)

		/*
			IFE_F_FU_BCDL_SLPOE:	[0x0, 0xffff],			bits : 15_0
			IFE_F_FU_BCDS_SLPOE:	[0x0, 0xffff],			bits : 31_16
		*/
#define RHE_FUSION_SLOPE_1_OFS 0x0538
		REGDEF_BEGIN(RHE_FUSION_SLOPE_1)
		REGDEF_BIT(ife_f_fusion_bcdl_slope, 	   16)
		REGDEF_BIT(ife_f_fusion_bcds_slope, 	   16)
		REGDEF_END(RHE_FUSION_SLOPE_1)

		/*
			ife_f_fusion_mc_lumthr	  :    [0x0, 0xfff],			bits : 11_0
			ife_f_fusion_mc_diff_ratio:    [0x0, 0x3],			bits : 13_12
		*/
#define RHE_FUSION_MOTION_COMPENSATION_OFS 0x0540
		REGDEF_BEGIN(RHE_FUSION_MOTION_COMPENSATION)
		REGDEF_BIT(ife_f_fusion_mc_lumthr,		  12)
		REGDEF_BIT(ife_f_fusion_mc_diff_ratio,		  2)
		REGDEF_END(RHE_FUSION_MOTION_COMPENSATION)

		/*
			ife_f_fusion_mc_lut_diff_weight0 (0~16):	[0x0, 0x1f],		   bits : 4_0
			ife_f_fusion_mc_lut_diff_weight1 (0~16):	[0x0, 0x1f],		   bits : 9_5
			ife_f_fusion_mc_lut_diff_weight2 (0~16):	[0x0, 0x1f],		   bits : 14_10
			ife_f_fusion_mc_lut_diff_weight3 (0~16):	[0x0, 0x1f],		   bits : 19_15
			ife_f_fusion_mc_lut_diff_weight4 (0~16):	[0x0, 0x1f],		   bits : 24_20
			ife_f_fusion_mc_lut_diff_weight5 (0~16):	[0x0, 0x1f],		   bits : 29_25
		*/
#define RHE_FUSION_MOTION_COMPENSATION_LUT_0_OFS 0x0544
		REGDEF_BEGIN(RHE_FUSION_MOTION_COMPENSATION_LUT_0)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight0,		5)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight1,		5)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight2,		5)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight3,		5)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight4,		5)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight5,		5)
		REGDEF_END(RHE_FUSION_MOTION_COMPENSATION_LUT_0)

		/*
			ife_f_fusion_mc_lut_diff_weight12 (0~16):	 [0x0, 0x1f],		   bits : 4_0
			ife_f_fusion_mc_lut_diff_weight13 (0~16):	 [0x0, 0x1f],		   bits : 9_5
			ife_f_fusion_mc_lut_diff_weight14 (0~16):	 [0x0, 0x1f],		   bits : 14_10
			ife_f_fusion_mc_lut_diff_weight15 (0~16):	 [0x0, 0x1f],		   bits : 19_15
			IFE_F_FUSION_MC_LUT_DWD (0~16)	:	 [0x0, 0x1f],			bits : 24_20
		*/
#define RHE_FUSION_MOTION_COMPENSATION_LUT_2_OFS 0x054c
		REGDEF_BEGIN(RHE_FUSION_MOTION_COMPENSATION_LUT_2)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight12,		 5)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight13,		 5)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight14,		 5)
		REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight15,		 5)
		REGDEF_BIT(ife_f_fusion_mc_lut_difflumth_diff_weight,		 5)
		REGDEF_END(RHE_FUSION_MOTION_COMPENSATION_LUT_2)

//=============================================================================
// IFE HDR Fcurve
//=============================================================================
		/*
			IFE_F_FCURVE_YM_SEL (0~2)  :	[0x0, 0x3], 		bits : 1_0
			ife_f_fcurve_yvweight (0~8):	[0x0, 0xf], 		bits : 5_2
		*/
#define RHE_FCURVE_CTRL_OFS 0x0560
		REGDEF_BEGIN(RHE_FCURVE_CTRL)
		REGDEF_BIT(ife_f_fcurve_ymean_select,		 2)
		REGDEF_BIT(ife_f_fcurve_yvweight,		 4)
		REGDEF_END(RHE_FCURVE_CTRL)

		/*
			ife_f_fcurve_yweight_lut0:	  [0x0, 0xff],			bits : 7_0
			ife_f_fcurve_yweight_lut1:	  [0x0, 0xff],			bits : 15_8
			ife_f_fcurve_yweight_lut2:	  [0x0, 0xff],			bits : 23_16
			ife_f_fcurve_yweight_lut3:	  [0x0, 0xff],			bits : 31_24
		*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER0_OFS 0x0564
		REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER0)
		REGDEF_BIT(ife_f_fcurve_yweight_lut0,		 8)
		REGDEF_BIT(ife_f_fcurve_yweight_lut1,		 8)
		REGDEF_BIT(ife_f_fcurve_yweight_lut2,		 8)
		REGDEF_BIT(ife_f_fcurve_yweight_lut3,		 8)
		REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER0)

		/*
			ife_f_fcurve_yweight_lut16:    [0x0, 0xff], 		bits : 7_0
		*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER4_OFS 0x0574
		REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER4)
		REGDEF_BIT(ife_f_fcurve_yweight_lut16,		  8)
		REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER4)

		/*
			ife_f_fcurve_l_0:	 [0x0, 0xffff], 		bits : 15_0
			ife_f_fcurve_l_1:	 [0x0, 0xffff], 		bits : 31_16
		*/
#define RHE_FCURVE_L_INDEX_REGISTER_0_OFS 0x0578
		REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_0)
			REGDEF_BIT(ife_f_fcurve_l_0,		16)
			REGDEF_BIT(ife_f_fcurve_l_1,		16)
		REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_0)

		/*
			ife_f_fcurve_l_64:	  [0x0, 0xffff],			bits : 15_0
		*/
#define RHE_FCURVE_L_INDEX_REGISTER_32_OFS 0x05f8
		REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_32)
			REGDEF_BIT(ife_f_fcurve_l_64,		 16)
		REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_32)

		/*
			ife_f_fcurve_r_0:	 [0x0, 0xffff], 			bits : 15_0
			ife_f_fcurve_r_1:	 [0x0, 0xfffff],			bits : 19_0
		*/
#define RHE_FCURVE_R_INDEX_REGISTER_0_OFS 0x067c
		REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_0)
			REGDEF_BIT(ife_f_fcurve_r_0,		16)
			REGDEF_BIT(ife_f_fcurve_r_1,		16)
		REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_0)

		/*
			ife_f_fcurve_r_16		 :	  [0x0, 0xffff],			bits : 15_0
			IFE_F_FCURVE_EV_FMT (0~4):	  [0x0, 0x7],			bits : 26_24
		*/
#define RHE_FCURVE_R_INDEX_REGISTER_8_OFS 0x069c
		REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_8)
			REGDEF_BIT(ife_f_fcurve_r_16		,		 16)
			REGDEF_BIT( 						,		 8)
			REGDEF_BIT(ife_f_fcurve_ev_fmt		,		 3)
		REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_8)

//=============================================================================
// IFE WDR
//=============================================================================
	/*
		ife_wdr_en			 :	  [0x0, 0x1],			bits : 0
		ife_wdr_subimg_out_en:	  [0x0, 0x1],			bits : 1
		ife_wdr_histogram_en :	  [0x0, 0x1],			bits : 2
		ife_wdr_histogram_sel:	  [0x0, 0x1],			bits : 3
		ife_wdr_tcurve_en	 :	  [0x0, 0x1],			bits : 4
		ife_wdr_d2d_rand	 :	  [0x0, 0x1],			bits : 5
		ife_wdr_d2d_rand_rst :	  [0x0, 0x1],			bits : 6
	*/
#define WDR_CONTROL_REGISTER0_OFS 0x0900
	REGDEF_BEGIN(WDR_CONTROL_REGISTER0)
		REGDEF_BIT(ife_wdr_en			,		 1)
		REGDEF_BIT(ife_wdr_subimg_out_en,		 1)
		REGDEF_BIT(ife_wdr_histogram_en ,		 1)
		REGDEF_BIT(ife_wdr_histogram_sel,		 1)
		REGDEF_BIT(ife_wdr_tcurve_en	,		 1)
		REGDEF_BIT(ife_wdr_d2d_rand 	,		 1)
		REGDEF_BIT(ife_wdr_d2d_rand_rst ,		 1)
		REGDEF_BIT( 					,	    24)
		REGDEF_BIT(ife_wdr_bypass_mode  ,		 1)
	REGDEF_END(WDR_CONTROL_REGISTER0)

	/*
		ife_wdr_subimg_width  :    [0x0, 0x3f], 		bits : 5_0
		ife_wdr_subimg_height :    [0x0, 0x3f], 		bits : 13_8
		ife_wdr_dithering_en  :    [0x0, 0x1],			bits : 16
		ife_wdr_rand_reset	  :    [0x0, 0x1],			bits : 17
		ife_wdr_rand_sel_(0~2):    [0x0, 0x3],			bits : 19_18
	*/
#define IFE_WDR_SUBIMAGE_REGISTER_0_OFS 0x0904
	REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_0)
		REGDEF_BIT(ife_wdr_subimg_width  ,		  6)
		REGDEF_BIT( 					 ,		  2)
		REGDEF_BIT(ife_wdr_subimg_height ,		  6)
		REGDEF_BIT( 					 ,		  2)
		REGDEF_BIT(ife_wdr_dithering_en  ,		  1)
		REGDEF_BIT(ife_wdr_rand_reset	 ,		  1)
		REGDEF_BIT(ife_wdr_rand_sel 	 ,		  2)
	REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_0)

	/*
		ife_wdr_subimg_sai:    [0x0, 0x3fffffff],			bits : 31_2
	*/
#define IFE_WDR_SUBIMAGE_REGISTER_1_OFS 0x0908
	REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_1)
		REGDEF_BIT( 				 ,		  2)
		REGDEF_BIT(ife_wdr_subimg_sai,		  30)
	REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_1)

	/*
		ife_wdr_subimg_ofsi    :	[0x0, 0x3fff],			bits : 15_2
		ife_wdr_subimg_sai_high:	[0x0, 0xf], 		bits : 27_24
	*/
#define IFE_WDR_SUBIMAGE_REGISTER_2_OFS 0x090c
	REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_2)
		REGDEF_BIT( 					  , 	   2)
		REGDEF_BIT(ife_wdr_subimg_ofsi	  , 	   14)
		REGDEF_BIT( 					  , 	   8)
		REGDEF_BIT(ife_wdr_subimg_sai_high, 	   4)
	REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_2)

		/*
			ife_wdr_subimg_ofso    :	[0x0, 0x3fff],			bits : 15_2
			ife_wdr_subimg_sao_high:	[0x0, 0xf], 		bits : 27_24
		*/
#define IFE_WDR_SUBIMAGE_REGISTER_4_OFS 0x0914
		REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_4)
			REGDEF_BIT( 					  , 	   2)
			REGDEF_BIT(ife_wdr_subimg_ofso	  , 	   14)
			REGDEF_BIT( 					  , 	   8)
			REGDEF_BIT(ife_wdr_subimg_sao_high, 	   4)
		REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_4)

	/*
		ife_wdr_gain_prot_str_(0~256):	  [0x0, 0x1ff], 		bits : 8_0
	*/
#define IFE_WDR_GAIN_STRENGTH_REGISTER_OFS 0x0924
	REGDEF_BEGIN(IFE_WDR_GAIN_STRENGTH_REGISTER)
		REGDEF_BIT(ife_wdr_gain_prot_str,		 9)
	REGDEF_END(IFE_WDR_GAIN_STRENGTH_REGISTER)

	/*
		ife_wdr_outbld_table_en:	[0x0, 0x1], 		bits : 0
		ife_wdr_gainctrl_en    :	[0x0, 0x1], 		bits : 1
		ife_wdr_maxgain 	   :	[0x0, 0xff],			bits : 23_16
		ife_wdr_mingain 	   :	[0x0, 0xff],			bits : 31_24
	*/
#define IFE_WDR_CONTROL_REGISTER1_OFS 0x0948
	REGDEF_BEGIN(IFE_WDR_CONTROL_REGISTER1)
		REGDEF_BIT(ife_wdr_outbld_table_en, 	   1)
		REGDEF_BIT(ife_wdr_gainctrl_en	  , 	   1)
		REGDEF_BIT( 					  , 	   14)
		REGDEF_BIT(ife_wdr_maxgain		  , 	   8)
		REGDEF_BIT(ife_wdr_mingain		  , 	   8)
	REGDEF_END(IFE_WDR_CONTROL_REGISTER1)

	/*
		ife_wdr_outbld_val_lut_l0:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_l1:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_0_OFS 0x09f8
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_0)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l0,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l1,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_0)

	/*
		ife_wdr_outbld_val_lut_l64:    [0x0, 0xfff],			bits : 11_0
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_32_OFS 0x0a78
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_32)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l64,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_32)

	/*
		ife_wdr_outbld_val_lut_r0:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_r1:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_0_OFS 0x0a7c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_0)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r0,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r1,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_0)

	/*
		ife_wdr_outbld_val_lut_r16:    [0x0, 0xfff],			bits : 11_0
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_8_OFS 0x0a9c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_8)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r16,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_8)

	/*
		ife_wdr_strength_:	  [0x0, 0xff],			bits : 7_0
	*/
#define IFE_WDR_OUPUT_BLENDING_REGISTER_OFS 0x0aa4
	REGDEF_BEGIN(IFE_WDR_OUPUT_BLENDING_REGISTER)
		REGDEF_BIT(ife_wdr_strength,		8)
	REGDEF_END(IFE_WDR_OUPUT_BLENDING_REGISTER)

	/*
		ife_wdr_mode		 :	  [0x0, 0x1],			bits : 0
		ife_wdr_anti_halo_opt:	  [0x0, 0x1],			bits : 1
		ife_wdr_halo_ratio	 :	  [0x0, 0xff],			bits : 15_8
		ife_wdr_halo_slope	 :	  [0x0, 0xff],			bits : 23_16
	*/
#define IFE_WDR_TONE_MAPPING_REGISTER_0_OFS 0x0aac
	REGDEF_BEGIN(IFE_WDR_TONE_MAPPING_REGISTER_0)
		REGDEF_BIT(ife_wdr_mode 		,		 1)
		REGDEF_BIT(ife_wdr_anti_halo_opt,		 1)
		REGDEF_BIT( 					,		 6)
		REGDEF_BIT(ife_wdr_halo_ratio	,		 8)
		REGDEF_BIT(ife_wdr_halo_slope	,		 8)
	REGDEF_END(IFE_WDR_TONE_MAPPING_REGISTER_0)

	/*
		ife_wdr_fbc_en :	[0x0, 0x1], 		bits : 0
		ife_wdr_fbc_rto:	[0x0, 0xff],			bits : 15_8
		ife_wdr_fbc_th0:	[0x0, 0x1fff],			bits : 28_16
	*/
#define IFE_WDR_FBC_REGISTER_0_OFS 0x0bb8
	REGDEF_BEGIN(IFE_WDR_FBC_REGISTER_0)
		REGDEF_BIT(ife_wdr_fbc_en , 	   1)
		REGDEF_BIT( 			  , 	   7)
		REGDEF_BIT(ife_wdr_fbc_rto, 	   8)
		REGDEF_BIT(ife_wdr_fbc_th0, 	   13)
	REGDEF_END(IFE_WDR_FBC_REGISTER_0)

//=============================================================================
// IFE WDR ToneCurve
//=============================================================================
	/*
		ife_wdr_tcurve_val_lut_l0:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_l1:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_0_OFS 0x094c
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_0)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l0,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l1,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_0)

	/*
		ife_wdr_tcurve_val_lut_l64:    [0x0, 0xfff],			bits : 11_0
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_32_OFS 0x09cc
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_32)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l64,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_32)

	/*
		ife_wdr_tcurve_val_lut_r0:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_r1:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_0_OFS 0x09d0
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_0)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r0,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r1,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_0)

	/*
		ife_wdr_tcurve_val_lut_r16:    [0x0, 0xfff],			bits : 11_0
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_8_OFS 0x09f0
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_8)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r16,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_8)

	/*
		ife_wdr_input_yv_bld_lut0:	  [0x0, 0x3f],			bits : 5_0
		ife_wdr_input_yv_bld_lut1:	  [0x0, 0x3f],			bits : 13_8
		ife_wdr_input_yv_bld_lut2:	  [0x0, 0x3f],			bits : 21_16
		ife_wdr_input_yv_bld_lut3:	  [0x0, 0x3f],			bits : 29_24
	*/
#define IFE_WDR_YV_BLD_REGISTER_0_OFS 0x0bc8
	REGDEF_BEGIN(IFE_WDR_YV_BLD_REGISTER_0)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut0,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut1,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut2,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut3,		 6)
	REGDEF_END(IFE_WDR_YV_BLD_REGISTER_0)

	/*
		ife_wdr_input_yv_bld_lut8:	  [0x0, 0x3f],			bits : 5_0
	*/
#define IFE_WDR_YV_BLD_REGISTER_2_OFS 0x0bd0
	REGDEF_BEGIN(IFE_WDR_YV_BLD_REGISTER_2)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut8,		 6)
	REGDEF_END(IFE_WDR_YV_BLD_REGISTER_2)

//=============================================================================
// DCE HIST
//=============================================================================
	/*
		ife_wdr_histogram_bin0:    [0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin1:    [0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_0_OFS 0x0ab8
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_0)
		REGDEF_BIT(ife_wdr_histogram_bin0,		  16)
		REGDEF_BIT(ife_wdr_histogram_bin1,		  16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_0)

//=============================================================================//
//                                                                             //
//                                 NT98538 IPE                                 //
//                                                                             //
//=============================================================================//
/*
    ipe_swrst          :    [0x0, 0x1],         bits : 0
    ipe_start          :    [0x0, 0x1],         bits : 1
    ipe_load_start     :    [0x0, 0x1],         bits : 2
    ipe_load_fd        :    [0x0, 0x1],         bits : 3
    ipe_load_fs        :    [0x0, 0x1],         bits : 4
    ipe_gbl_load_en    :    [0x0, 0x1],         bits : 5
    ipe_rwgamma        :    [0x0, 0x3],         bits : 9_8
    ipe_rwgamma_opt    :    [0x0, 0x3],         bits : 11_10
    ipe_dmach_dis      :    [0x0, 0x1],         bits : 27
    ll_fire            :    [0x0, 0x1],         bits : 28
*/
#define IPE_CONTROL_REGISTER_OFS 0x0000
REGDEF_BEGIN(IPE_CONTROL_REGISTER)
REGDEF_BIT(ipe_swrst,           1)
REGDEF_BIT(ipe_start,           1)
REGDEF_BIT(ipe_load_start,      1)
REGDEF_BIT(ipe_load_fd,         1)
REGDEF_BIT(ipe_load_fs,         1)
REGDEF_BIT(ipe_gbl_load_en,     1)
REGDEF_BIT(,                    2)
REGDEF_BIT(ipe_rwgamma,         2)
REGDEF_BIT(ipe_rwgamma_opt,     2)
REGDEF_BIT(,                    15)
REGDEF_BIT(ipe_dmach_dis,       1)
REGDEF_BIT(ll_fire,             1)
REGDEF_END(IPE_CONTROL_REGISTER)

/*
      rgblpf_en          :    [0x0, 0x1],         bits : 0
      rgbgamma_en        :    [0x0, 0x1],         bits : 1
      ycurve_en          :    [0x0, 0x1],         bits : 2
      cr_en              :    [0x0, 0x1],         bits : 3
      defog_subimg_out_en:    [0x0, 0x1],         bits : 4
      defog_en           :    [0x0, 0x1],         bits : 5
      //lce_subimg_out_en  :    [0x0, 0x1],					bits : 6      
      lce_en             :    [0x0, 0x1],         bits : 7
      cst_en             :    [0x0, 0x1],         bits : 8
      ctrl_en            :    [0x0, 0x1],         bits : 9
      hadj_en            :    [0x0, 0x1],         bits : 10
      cadj_en            :    [0x0, 0x1],         bits : 11
      cadj_yenh_en       :    [0x0, 0x1],         bits : 12
      cadj_ycon_en       :    [0x0, 0x1],         bits : 13
      cadj_ccon_en       :    [0x0, 0x1],         bits : 14
      cadj_ycth_en       :    [0x0, 0x1],         bits : 15
      cstprotect_en      :    [0x0, 0x1],         bits : 16
      edge_dbg_en        :    [0x0, 0x1],         bits : 17
      vaccen             :    [0x0, 0x1],         bits : 19
      win0_vaen          :    [0x0, 0x1],         bits : 20
      win1_vaen          :    [0x0, 0x1],         bits : 21
      win2_vaen          :    [0x0, 0x1],         bits : 22
      win3_vaen          :    [0x0, 0x1],         bits : 23
      win4_vaen          :    [0x0, 0x1],         bits : 24
      pfr_en             :    [0x0, 0x1],         bits : 25
      va_pre_filter_mode :    [0x0, 0x2],         bits : 27_26
      va_win_ldg_en      :    [0x0, 0x1],         bits : 28
      cc3d_en      :    [0x0, 0x1],         bits : 29   
      ipe_ycc_shift_mode_en      :    [0x0, 0x1],         bits : 30
*/
#define IPE_MODE_REGISTER_1_OFS 0x0008
REGDEF_BEGIN(IPE_MODE_REGISTER_1)
REGDEF_BIT(rgblpf_en,                  1)
REGDEF_BIT(rgbgamma_en,                1)
REGDEF_BIT(ycurve_en,                  1)
REGDEF_BIT(cr_en,                      1)
REGDEF_BIT(defog_subimg_out_en,        1)
REGDEF_BIT(defog_en,                   1)
REGDEF_BIT(  ,                         1)
REGDEF_BIT(lce_en,                     1)
REGDEF_BIT(cst_en,                     1)
REGDEF_BIT(ctrl_en,                    1)
REGDEF_BIT(hadj_en,                    1)
REGDEF_BIT(cadj_en,                    1)
REGDEF_BIT(cadj_yenh_en,               1)
REGDEF_BIT(cadj_ycon_en,               1)
REGDEF_BIT(cadj_ccon_en,               1)
REGDEF_BIT(cadj_ycth_en,               1)
REGDEF_BIT(cstprotect_en,              1)
REGDEF_BIT(edge_dbg_en,                1)
REGDEF_BIT(,                           1)
REGDEF_BIT(vacc_en,                    1)
REGDEF_BIT(win0_vaen,                  1)
REGDEF_BIT(win1_vaen,                  1)
REGDEF_BIT(win2_vaen,                  1)
REGDEF_BIT(win3_vaen,                  1)
REGDEF_BIT(win4_vaen,                  1)
REGDEF_BIT(pfr_en,                     1)
REGDEF_BIT(va_pre_filter_mode,         2)
REGDEF_BIT(va_win_ldg_en,              1)
REGDEF_BIT(cc3d_en,                    1)
REGDEF_BIT(ipe_ycc_shift_mode_en,      1)
REGDEF_END(IPE_MODE_REGISTER_1)

//=============================================================================
// IPE CFA
//=============================================================================
/*
    cfa_en                 :    [0x0, 0x1],			bits : 0
    cfa_subimg_out_en      :    [0x0, 0x1],			bits : 1
    cfa_subimg_out_flip_en :    [0x0, 0x1],			bits : 2
    cfa_pinkr_en           :    [0x0, 0x1],			bits : 3
*/
#define IPE_MODE_REGISTER_2_OFS 0x0900
REGDEF_BEGIN(IPE_MODE_REGISTER_2)
REGDEF_BIT(cfa_en,        1)
REGDEF_BIT(cfa_subimg_out_en,        1)
REGDEF_BIT(cfa_subimg_out_flip_en,        1)
REGDEF_BIT(cfa_pinkr_en,        1)
REGDEF_END(IPE_MODE_REGISTER_2)

/*
    cfa_edge_dth :    [0x0, 0xfff],			bits : 11_0
    cfa_edge_dth2:    [0x0, 0xfff],			bits : 23_12
    reserved     :    [0x0, 0xff],			bits : 31_24
*/
#define IPE_COLOR_INTERPOLATION_REGISTER1_OFS 0x0914
REGDEF_BEGIN(IPE_COLOR_INTERPOLATION_REGISTER1)
REGDEF_BIT(cfa_edge_dth,        12)
REGDEF_BIT(cfa_edge_dth2,        12)
REGDEF_BIT(reserved,        8)
REGDEF_END(IPE_COLOR_INTERPOLATION_REGISTER1)

/*
    cfa_luma_weight16:    [0x0, 0xff],          bits : 7_0
    cfa_freq_th      :    [0x0, 0xfff],         bits : 27_16
*/
#define IPE_COLOR_INTERPOLATION_REGISTER9_OFS 0x0934
REGDEF_BEGIN(IPE_COLOR_INTERPOLATION_REGISTER9)
REGDEF_BIT(cfa_luma_weight16,        8)
REGDEF_BIT(,        8)
REGDEF_BIT(cfa_freq_th,        12)
REGDEF_END(IPE_COLOR_INTERPOLATION_REGISTER9)

/*
    cfa_fcs_coring:    [0x0, 0xff],         bits : 7_0
    cfa_fcs_weight:    [0x0, 0xff],         bits : 15_8
    cfa_fcs_dirsel:    [0x0, 0x1],          bits : 16
*/
#define IPE_COLOR_INTERPOLATION_REGISTER10_OFS 0x0938
REGDEF_BEGIN(IPE_COLOR_INTERPOLATION_REGISTER10)
REGDEF_BIT(cfa_fcs_coring,        8)
REGDEF_BIT(cfa_fcs_weight,        8)
REGDEF_BIT(cfa_fcs_dirsel,        1)
REGDEF_END(IPE_COLOR_INTERPOLATION_REGISTER10)

/*
    cfa_fcs_strength0:    [0x0, 0xf],           bits : 3_0
    cfa_fcs_strength1:    [0x0, 0xf],           bits : 7_4
    cfa_fcs_strength2:    [0x0, 0xf],           bits : 11_8
    cfa_fcs_strength3:    [0x0, 0xf],           bits : 15_12
    cfa_fcs_strength4:    [0x0, 0xf],           bits : 19_16
    cfa_fcs_strength5:    [0x0, 0xf],           bits : 23_20
    cfa_fcs_strength6:    [0x0, 0xf],           bits : 27_24
    cfa_fcs_strength7:    [0x0, 0xf],           bits : 31_28
*/
#define IPE_COLOR_INTERPOLATION_REGISTER11_OFS 0x093c
REGDEF_BEGIN(IPE_COLOR_INTERPOLATION_REGISTER11)
REGDEF_BIT(cfa_fcs_strength0,        4)
REGDEF_BIT(cfa_fcs_strength1,        4)
REGDEF_BIT(cfa_fcs_strength2,        4)
REGDEF_BIT(cfa_fcs_strength3,        4)
REGDEF_BIT(cfa_fcs_strength4,        4)
REGDEF_BIT(cfa_fcs_strength5,        4)
REGDEF_BIT(cfa_fcs_strength6,        4)
REGDEF_BIT(cfa_fcs_strength7,        4)
REGDEF_END(IPE_COLOR_INTERPOLATION_REGISTER11)

/*
    cfa_fcs_strength8 :    [0x0, 0xf],          bits : 3_0
    cfa_fcs_strength9 :    [0x0, 0xf],          bits : 7_4
    cfa_fcs_strength10:    [0x0, 0xf],          bits : 11_8
    cfa_fcs_strength11:    [0x0, 0xf],          bits : 15_12
    cfa_fcs_strength12:    [0x0, 0xf],          bits : 19_16
    cfa_fcs_strength13:    [0x0, 0xf],          bits : 23_20
    cfa_fcs_strength14:    [0x0, 0xf],          bits : 27_24
    cfa_fcs_strength15:    [0x0, 0xf],          bits : 31_28
*/
#define IPE_COLOR_INTERPOLATION_REGISTER12_OFS 0x0940
REGDEF_BEGIN(IPE_COLOR_INTERPOLATION_REGISTER12)
REGDEF_BIT(cfa_fcs_strength8,        4)
REGDEF_BIT(cfa_fcs_strength9,        4)
REGDEF_BIT(cfa_fcs_strength10,        4)
REGDEF_BIT(cfa_fcs_strength11,        4)
REGDEF_BIT(cfa_fcs_strength12,        4)
REGDEF_BIT(cfa_fcs_strength13,        4)
REGDEF_BIT(cfa_fcs_strength14,        4)
REGDEF_BIT(cfa_fcs_strength15,        4)
REGDEF_END(IPE_COLOR_INTERPOLATION_REGISTER12)

//=============================================================================
// IPE PFR
//=============================================================================
/*
    pfr_uv_filt_en   :    [0x0, 0x1],			bits : 0
    pfr_luma_level_en:    [0x0, 0x1],			bits : 1
    pfr_set0_en      :    [0x0, 0x1],			bits : 2
    pfr_set1_en      :    [0x0, 0x1],			bits : 3
    pfr_set2_en      :    [0x0, 0x1],			bits : 4
    pfr_set3_en      :    [0x0, 0x1],			bits : 5
    pfr_out_wet      :    [0x0, 0xff],			bits : 23_16

*/
#define PURPLE_FRINGE_REDUCTION_REGISTER0_OFS 0x0440
REGDEF_BEGIN(PURPLE_FRINGE_REDUCTION_REGISTER0)
	REGDEF_BIT(pfr_uv_filt_en   ,        1)
	REGDEF_BIT(pfr_luma_level_en,        1)
	REGDEF_BIT(pfr_set0_en      ,        1)
	REGDEF_BIT(pfr_set1_en      ,        1)
	REGDEF_BIT(pfr_set2_en      ,        1)
	REGDEF_BIT(pfr_set3_en      ,        1)
	REGDEF_BIT(                 ,        10)
	REGDEF_BIT(pfr_out_wet      ,        8)
REGDEF_END(PURPLE_FRINGE_REDUCTION_REGISTER0)

/*
    pfr_color_u0:    [0x0, 0xff],			bits : 7_0
    pfr_color_v0:    [0x0, 0xff],			bits : 15_8
    pfr_r_wet0  :    [0x0, 0xff],			bits : 23_16
    pfr_b_wet0  :    [0x0, 0xff],			bits : 31_24

*/
#define PURPLE_FRINGE_REDUCTION_REGISTER2_OFS 0x0448
REGDEF_BEGIN(PURPLE_FRINGE_REDUCTION_REGISTER2)
	REGDEF_BIT(pfr_color_u0,        8)
	REGDEF_BIT(pfr_color_v0,        8)
	REGDEF_BIT(pfr_r_wet0  ,        8)
	REGDEF_BIT(pfr_b_wet0  ,        8)
REGDEF_END(PURPLE_FRINGE_REDUCTION_REGISTER2)

/*
    pfr_color_u1:    [0x0, 0xff],			bits : 7_0
    pfr_color_v1:    [0x0, 0xff],			bits : 15_8
    pfr_r_wet1  :    [0x0, 0xff],			bits : 23_16
    pfr_b_wet1  :    [0x0, 0xff],			bits : 31_24

*/
#define PURPLE_FRINGE_REDUCTION_REGISTER3_OFS 0x044c
REGDEF_BEGIN(PURPLE_FRINGE_REDUCTION_REGISTER3)
	REGDEF_BIT(pfr_color_u1,        8)
	REGDEF_BIT(pfr_color_v1,        8)
	REGDEF_BIT(pfr_r_wet1  ,        8)
	REGDEF_BIT(pfr_b_wet1  ,        8)
REGDEF_END(PURPLE_FRINGE_REDUCTION_REGISTER3)

/*
    pfr_color_u2:    [0x0, 0xff],			bits : 7_0
    pfr_color_v2:    [0x0, 0xff],			bits : 15_8
    pfr_r_wet2  :    [0x0, 0xff],			bits : 23_16
    pfr_b_wet2  :    [0x0, 0xff],			bits : 31_24

*/
#define PURPLE_FRINGE_REDUCTION_REGISTER4_OFS 0x0450
REGDEF_BEGIN(PURPLE_FRINGE_REDUCTION_REGISTER4)
	REGDEF_BIT(pfr_color_u2,        8)
	REGDEF_BIT(pfr_color_v2,        8)
	REGDEF_BIT(pfr_r_wet2  ,        8)
	REGDEF_BIT(pfr_b_wet2  ,        8)
REGDEF_END(PURPLE_FRINGE_REDUCTION_REGISTER4)

/*
    pfr_color_u3:    [0x0, 0xff],			bits : 7_0
    pfr_color_v3:    [0x0, 0xff],			bits : 15_8
    pfr_r_wet3  :    [0x0, 0xff],			bits : 23_16
    pfr_b_wet3  :    [0x0, 0xff],			bits : 31_24

*/
#define PURPLE_FRINGE_REDUCTION_REGISTER5_OFS 0x0454
REGDEF_BEGIN(PURPLE_FRINGE_REDUCTION_REGISTER5)
	REGDEF_BIT(pfr_color_u3,        8)
	REGDEF_BIT(pfr_color_v3,        8)
	REGDEF_BIT(pfr_r_wet3  ,        8)
	REGDEF_BIT(pfr_b_wet3  ,        8)
REGDEF_END(PURPLE_FRINGE_REDUCTION_REGISTER5)

/*
    pfr_luma_level0:    [0x0, 0xff],			bits : 7_0
    pfr_luma_level1:    [0x0, 0xff],			bits : 15_8
    pfr_luma_level2:    [0x0, 0xff],			bits : 23_16
    pfr_luma_level3:    [0x0, 0xff],			bits : 31_24

*/
#define PURPLE_FRINGE_REDUCTION_REGISTER14_OFS 0x0478
REGDEF_BEGIN(PURPLE_FRINGE_REDUCTION_REGISTER14)
	REGDEF_BIT(pfr_luma_level0,        8)
	REGDEF_BIT(pfr_luma_level1,        8)
	REGDEF_BIT(pfr_luma_level2,        8)
	REGDEF_BIT(pfr_luma_level3,        8)
REGDEF_END(PURPLE_FRINGE_REDUCTION_REGISTER14)

/*
    pfr_luma_level12:    [0x0, 0xff],			bits : 7_0
    pfr_luma_th     :    [0x0, 0x3ff],			bits : 25_16

*/
#define PURPLE_FRINGE_REDUCTION_REGISTER17_OFS 0x0484
REGDEF_BEGIN(PURPLE_FRINGE_REDUCTION_REGISTER17)
	REGDEF_BIT(pfr_luma_level12,        8)
	REGDEF_BIT(                ,        8)
	REGDEF_BIT(pfr_luma_th     ,        10)
REGDEF_END(PURPLE_FRINGE_REDUCTION_REGISTER17)

//=============================================================================
// IPE YCURVE
//=============================================================================
/*
    dram_sai_ycurve:    [0x0, 0x3fffffff],           bits : 31_2
*/
#define DMA_TO_IPE_REGISTER_4_OFS 0x0028
REGDEF_BEGIN(DMA_TO_IPE_REGISTER_4)
REGDEF_BIT(,                            2)
REGDEF_BIT(dram_sai_ycurve,        30)
REGDEF_END(DMA_TO_IPE_REGISTER_4)

//=============================================================================
// IPE GAMMA
//=============================================================================
/*
    dram_sai_gamma:    [0x0, 0x3fffffff],            bits : 31_2
*/
#define DMA_TO_IPE_REGISTER_5_OFS 0x002c
REGDEF_BEGIN(DMA_TO_IPE_REGISTER_5)
REGDEF_BIT(,                           2)
REGDEF_BIT(dram_sai_gamma,        30)
REGDEF_END(DMA_TO_IPE_REGISTER_5)

//=============================================================================
// IPE CCM
//=============================================================================
/*
    ccrange   :    [0x0, 0x3],          bits : 1_0
    cc2_sel   :    [0x0, 0x1],          bits : 2
    ccr_gamsel:    [0x0, 0x1],          bits : 3
    ccstab_sel:    [0x0, 0x3],          bits : 5_4
    ccofs_sel :    [0x0, 0x3],          bits : 7_6
    coef_rr   :    [0x0, 0xfff],            bits : 27_16
*/
#define COLOR_CORRECTION_REGISTER_0_OFS 0x0100
REGDEF_BEGIN(COLOR_CORRECTION_REGISTER_0)
REGDEF_BIT(ccrange,        2)
REGDEF_BIT(cc2_sel,        1)
REGDEF_BIT(ccr_gamsel,        1)
REGDEF_BIT(ccstab_sel,        2)
REGDEF_BIT(ccofs_sel,        2)
REGDEF_BIT(,        8)
REGDEF_BIT(coef_rr,        12)
REGDEF_END(COLOR_CORRECTION_REGISTER_0)

/*
    coef_rg:    [0x0, 0xfff],           bits : 11_0
    coef_rb:    [0x0, 0xfff],           bits : 27_16
*/
#define COLOR_CORRECTION_REGISTER_1_OFS 0x0104
REGDEF_BEGIN(COLOR_CORRECTION_REGISTER_1)
REGDEF_BIT(coef_rg,        12)
REGDEF_BIT(,        4)
REGDEF_BIT(coef_rb,        12)
REGDEF_END(COLOR_CORRECTION_REGISTER_1)

/*
    coef_gr:    [0x0, 0xfff],           bits : 11_0
    coef_gg:    [0x0, 0xfff],           bits : 27_16
*/
#define COLOR_CORRECTION_REGISTER_2_OFS 0x0108
REGDEF_BEGIN(COLOR_CORRECTION_REGISTER_2)
REGDEF_BIT(coef_gr,        12)
REGDEF_BIT(,        4)
REGDEF_BIT(coef_gg,        12)
REGDEF_END(COLOR_CORRECTION_REGISTER_2)

/*
    coef_gb:    [0x0, 0xfff],           bits : 11_0
    coef_br:    [0x0, 0xfff],           bits : 27_16
*/
#define COLOR_CORRECTION_REGISTER_3_OFS 0x010c
REGDEF_BEGIN(COLOR_CORRECTION_REGISTER_3)
REGDEF_BIT(coef_gb,        12)
REGDEF_BIT(,        4)
REGDEF_BIT(coef_br,        12)
REGDEF_END(COLOR_CORRECTION_REGISTER_3)

/*
    coef_bg:    [0x0, 0xfff],           bits : 11_0
    coef_bb:    [0x0, 0xfff],           bits : 27_16
*/
#define COLOR_CORRECTION_REGISTER_4_OFS 0x0110
REGDEF_BEGIN(COLOR_CORRECTION_REGISTER_4)
REGDEF_BIT(coef_bg,        12)
REGDEF_BIT(,        4)
REGDEF_BIT(coef_bb,        12)
REGDEF_END(COLOR_CORRECTION_REGISTER_4)

//=============================================================================
// IPE CST
//=============================================================================
/*
    coef_yr :    [0x0, 0x1ff],          bits : 8_0
    coef_yg :    [0x0, 0x1ff],          bits : 17_9
    coef_yb :    [0x0, 0x1ff],          bits : 26_18
    cstp_rat:    [0x0, 0x1f],           bits : 31_27
*/
#define COLOR_SPACE_TRANSFORM_REGISTER_0_OFS 0x0138
REGDEF_BEGIN(COLOR_SPACE_TRANSFORM_REGISTER_0)
	REGDEF_BIT(coef_yr ,        9)
	REGDEF_BIT(coef_yg ,        9)
	REGDEF_BIT(coef_yb ,        9)
	REGDEF_BIT(cstp_rat,        5)
REGDEF_END(COLOR_SPACE_TRANSFORM_REGISTER_0)

/*
    coef_ur   :    [0x0, 0x1ff],            bits : 8_0
    coef_ug   :    [0x0, 0x1ff],            bits : 17_9
    coef_ub   :    [0x0, 0x1ff],            bits : 26_18
    cstoff_sel:    [0x0, 0x1],          bits : 28
*/
#define COLOR_SPACE_TRANSFORM_REGISTER_1_OFS 0x013c
REGDEF_BEGIN(COLOR_SPACE_TRANSFORM_REGISTER_1)
	REGDEF_BIT(coef_ur   ,        9)
	REGDEF_BIT(coef_ug   ,        9)
	REGDEF_BIT(coef_ub   ,        9)
	REGDEF_BIT(          ,        1)
	REGDEF_BIT(cstoff_sel,        1)
REGDEF_END(COLOR_SPACE_TRANSFORM_REGISTER_1)

/*
    coef_vr:    [0x0, 0x1ff],           bits : 8_0
    coef_vg:    [0x0, 0x1ff],           bits : 17_9
    coef_vb:    [0x0, 0x1ff],           bits : 26_18
*/
#define COLOR_SPACE_TRANSFORM_REGISTER_2_OFS 0x0140
REGDEF_BEGIN(COLOR_SPACE_TRANSFORM_REGISTER_2)
	REGDEF_BIT(coef_vr,        9)
	REGDEF_BIT(coef_vg,        9)
	REGDEF_BIT(coef_vb,        9)
REGDEF_END(COLOR_SPACE_TRANSFORM_REGISTER_2)

//=============================================================================
// IPE COLOR
//=============================================================================
/*
    c_cbofs  :    [0x0, 0xff],          bits : 7_0
    c_crofs  :    [0x0, 0xff],          bits : 15_8
    c_con    :    [0x0, 0xff],          bits : 23_16
    c_rand_en:    [0x0, 0x1],           bits : 27
    c_rand   :    [0x0, 0x7],           bits : 30_28
*/
#define COLOR_COMPONENT_ADJUSTMENT_REGISTER_5_OFS 0x01cc
REGDEF_BEGIN(COLOR_COMPONENT_ADJUSTMENT_REGISTER_5)
REGDEF_BIT(c_cbofs,        8)
REGDEF_BIT(c_crofs,        8)
REGDEF_BIT(c_con,        8)
REGDEF_BIT(,        3)
REGDEF_BIT(c_rand_en,        1)
REGDEF_BIT(c_rand,        3)
REGDEF_END(COLOR_COMPONENT_ADJUSTMENT_REGISTER_5)

/*
    fstab0:    [0x0, 0xff],         bits : 7_0
    fstab1:    [0x0, 0xff],         bits : 15_8
    fstab2:    [0x0, 0xff],         bits : 23_16
    fstab3:    [0x0, 0xff],         bits : 31_24
*/
#define COLOR_CORRECTION_STAB_MAPPING_REGISTER_0_OFS 0x0114
REGDEF_BEGIN(COLOR_CORRECTION_STAB_MAPPING_REGISTER_0)
REGDEF_BIT(fstab0,        8)
REGDEF_BIT(fstab1,        8)
REGDEF_BIT(fstab2,        8)
REGDEF_BIT(fstab3,        8)
REGDEF_END(COLOR_CORRECTION_STAB_MAPPING_REGISTER_0)

/*
    fdtab0:    [0x0, 0xff],         bits : 7_0
    fdtab1:    [0x0, 0xff],         bits : 15_8
    fdtab2:    [0x0, 0xff],         bits : 23_16
    fdtab3:    [0x0, 0xff],         bits : 31_24
*/
#define COLOR_CORRECTION_DTAB_MAPPING_REGISTER_0_OFS 0x0124
REGDEF_BEGIN(COLOR_CORRECTION_DTAB_MAPPING_REGISTER_0)
REGDEF_BIT(fdtab0,        8)
REGDEF_BIT(fdtab1,        8)
REGDEF_BIT(fdtab2,        8)
REGDEF_BIT(fdtab3,        8)
REGDEF_END(COLOR_CORRECTION_DTAB_MAPPING_REGISTER_0)

/*
    chuem0:    [0x0, 0xff],         bits : 7_0
    chuem1:    [0x0, 0xff],         bits : 15_8
    chuem2:    [0x0, 0xff],         bits : 23_16
    chuem3:    [0x0, 0xff],         bits : 31_24
*/
#define COLOR_CONTROL_HUE_MAPPING_REGISTER_0_OFS 0x014c
REGDEF_BEGIN(COLOR_CONTROL_HUE_MAPPING_REGISTER_0)
REGDEF_BIT(chuem0,        8)
REGDEF_BIT(chuem1,        8)
REGDEF_BIT(chuem2,        8)
REGDEF_BIT(chuem3,        8)
REGDEF_END(COLOR_CONTROL_HUE_MAPPING_REGISTER_0)

/*
    cintm0:    [0x0, 0xff],         bits : 7_0
    cintm1:    [0x0, 0xff],         bits : 15_8
    cintm2:    [0x0, 0xff],         bits : 23_16
    cintm3:    [0x0, 0xff],         bits : 31_24
*/
#define COLOR_CONTROL_INTENSITY_MAPPING_REGISTER_0_OFS 0x0164
REGDEF_BEGIN(COLOR_CONTROL_INTENSITY_MAPPING_REGISTER_0)
REGDEF_BIT(cintm0,        8)
REGDEF_BIT(cintm1,        8)
REGDEF_BIT(cintm2,        8)
REGDEF_BIT(cintm3,        8)
REGDEF_END(COLOR_CONTROL_INTENSITY_MAPPING_REGISTER_0)

/*
    csatm0:    [0x0, 0xff],         bits : 7_0
    csatm1:    [0x0, 0xff],         bits : 15_8
    csatm2:    [0x0, 0xff],         bits : 23_16
    csatm3:    [0x0, 0xff],         bits : 31_24
*/
#define COLOR_CONTROL_SATURATION_MAPPING_REGISTER_0_OFS 0x017c
REGDEF_BEGIN(COLOR_CONTROL_SATURATION_MAPPING_REGISTER_0)
REGDEF_BIT(csatm0,        8)
REGDEF_BIT(csatm1,        8)
REGDEF_BIT(csatm2,        8)
REGDEF_BIT(csatm3,        8)
REGDEF_END(COLOR_CONTROL_SATURATION_MAPPING_REGISTER_0)

/*
    ccontab0:    [0x0, 0x3ff],          bits : 9_0
    ccontab1:    [0x0, 0x3ff],          bits : 19_10
    ccontab2:    [0x0, 0x3ff],          bits : 29_20
*/
#define COLOR_COMPONENT_ADJUSTMENT_REGISTER_9_OFS 0x01dc
REGDEF_BEGIN(COLOR_COMPONENT_ADJUSTMENT_REGISTER_9)
REGDEF_BIT(ccontab0,        10)
REGDEF_BIT(ccontab1,        10)
REGDEF_BIT(ccontab2,        10)
REGDEF_END(COLOR_COMPONENT_ADJUSTMENT_REGISTER_9)

/*
    ccontab15  :    [0x0, 0x3ff],           bits : 9_0
    ccontab16  :    [0x0, 0x3ff],           bits : 19_10
    ccontab_sel:    [0x0, 0x1],         bits : 31
*/
#define COLOR_COMPONENT_ADJUSTMENT_REGISTER_14_OFS 0x01f0
REGDEF_BEGIN(COLOR_COMPONENT_ADJUSTMENT_REGISTER_14)
REGDEF_BIT(ccontab15,        10)
REGDEF_BIT(ccontab16,        10)
REGDEF_BIT(,        11)
REGDEF_BIT(ccontab_sel,        1)
REGDEF_END(COLOR_COMPONENT_ADJUSTMENT_REGISTER_14)

//=============================================================================
// IPE CONTRAST
//=============================================================================
/*
    int_ofs   :    [0x0, 0xff],         bits : 7_0
    sat_ofs   :    [0x0, 0xff],         bits : 15_8
    chue_roten:    [0x0, 0x1],          bits : 16
    chue_c2gen:    [0x0, 0x1],          bits : 17
    cctrl_sel :    [0x0, 0x3],          bits : 21_20
    vdet_div  :    [0x0, 0xff],         bits : 31_24
*/
#define COLOR_CONTROL_REGISTER_OFS 0x0148
REGDEF_BEGIN(COLOR_CONTROL_REGISTER)
REGDEF_BIT(int_ofs,        8)
REGDEF_BIT(sat_ofs,        8)
REGDEF_BIT(chue_roten,        1)
REGDEF_BIT(chue_c2gen,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(cctrl_sel,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(vdet_div,        8)
REGDEF_END(COLOR_CONTROL_REGISTER)

/*
    y_con       :    [0x0, 0xff],           bits : 7_0
    yc_randreset:    [0x0, 0x1],            bits : 8
    y_rand_en   :    [0x0, 0x1],            bits : 9
    y_rand      :    [0x0, 0x7],            bits : 14_12
    y_ethy      :    [0x0, 0x3ff],          bits : 25_16
*/
#define COLOR_COMPONENT_ADJUSTMENT_REGISTER_1_OFS 0x01bc
REGDEF_BEGIN(COLOR_COMPONENT_ADJUSTMENT_REGISTER_1)
REGDEF_BIT(y_con,        8)
REGDEF_BIT(yc_randreset,        1)
REGDEF_BIT(y_rand_en,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(y_rand,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(y_ethy,        10)
REGDEF_END(COLOR_COMPONENT_ADJUSTMENT_REGISTER_1)

/*
    lce_lum_adj_lut0:    [0x0, 0xff],           bits : 7_0
    lce_lum_adj_lut1:    [0x0, 0xff],           bits : 15_8
    lce_lum_adj_lut2:    [0x0, 0xff],           bits : 23_16
    lce_lum_adj_lut3:    [0x0, 0xff],           bits : 31_24
*/
#define LCE_REGISTER1_OFS 0x03e8
REGDEF_BEGIN(LCE_REGISTER1)
REGDEF_BIT(lce_lum_adj_lut0,        8)
REGDEF_BIT(lce_lum_adj_lut1,        8)
REGDEF_BIT(lce_lum_adj_lut2,        8)
REGDEF_BIT(lce_lum_adj_lut3,        8)
REGDEF_END(LCE_REGISTER1)

/*
    lce_lum_adj_lut8:    [0x0, 0xff],           bits : 7_0
*/
#define LCE_REGISTER3_OFS 0x03f0
REGDEF_BEGIN(LCE_REGISTER3)
REGDEF_BIT(lce_lum_adj_lut8,        8)
REGDEF_END(LCE_REGISTER3)

//=============================================================================
// IPE DEFOG statistic
//=============================================================================
/*
    defog_subimg_dramsai:    [0x0, 0x3fffffff],           bits : 31_2
*/
#define DMA_DEFOG_SUBIMG_INPUT_CHANNEL_REGISTER_OFS 0x0314
REGDEF_BEGIN(DMA_DEFOG_SUBIMG_INPUT_CHANNEL_REGISTER)
REGDEF_BIT(,        2)
REGDEF_BIT(defog_subimg_dramsai,        30)
REGDEF_END(DMA_DEFOG_SUBIMG_INPUT_CHANNEL_REGISTER)

/*
    defog_subimg_lofsi:    [0x0, 0x3fff],         bits : 15_2
*/
#define DMA_DEFOG_SUBIMG_INPUT_CHANNEL_LINEOFFSET_REGISTER_OFS 0x0318
REGDEF_BEGIN(DMA_DEFOG_SUBIMG_INPUT_CHANNEL_LINEOFFSET_REGISTER)
REGDEF_BIT(,        2)
REGDEF_BIT(defog_subimg_lofsi,        14)
REGDEF_END(DMA_DEFOG_SUBIMG_INPUT_CHANNEL_LINEOFFSET_REGISTER)

/*
    defog_statistics_air0:    [0x0, 0x3ff],			bits : 9_0
    defog_statistics_air1:    [0x0, 0x3ff],			bits : 25_16
*/
#define DEFOG_STATISTICS_REGISTER_1_OFS 0x039c
REGDEF_BEGIN(DEFOG_STATISTICS_REGISTER_1)
	REGDEF_BIT(defog_statistics_air0,        10)
	REGDEF_BIT(                     ,        6)
	REGDEF_BIT(defog_statistics_air1,        10)
REGDEF_END(DEFOG_STATISTICS_REGISTER_1)

/*
    defog_statistics_air2:    [0x0, 0x3ff],			bits : 9_0
*/
#define DEFOG_STATISTICS_REGISTER_2_OFS 0x03a0
REGDEF_BEGIN(DEFOG_STATISTICS_REGISTER_2)
	REGDEF_BIT(defog_statistics_air2,        10)
REGDEF_END(DEFOG_STATISTICS_REGISTER_2)

//=============================================================================
// IPE DEFOG
//=============================================================================
/*
    defog_mod_lut0:    [0x0, 0x3ff],			bits : 9_0
    defog_mod_lut1:    [0x0, 0x3ff],			bits : 25_16
*/
#define DEFOG_FOG_MODIFY_REGISTER_0_OFS 0x0358
REGDEF_BEGIN(DEFOG_FOG_MODIFY_REGISTER_0)
	REGDEF_BIT(defog_mod_lut0,        10)
	REGDEF_BIT(               ,        6)
	REGDEF_BIT(defog_mod_lut1,        10)
REGDEF_END(DEFOG_FOG_MODIFY_REGISTER_0)

/*
    defog_mod_lut16:    [0x0, 0x3ff],			bits : 9_0
*/
#define DEFOG_FOG_MODIFY_REGISTER_8_OFS 0x0378
REGDEF_BEGIN(DEFOG_FOG_MODIFY_REGISTER_8)
	REGDEF_BIT(defog_mod_lut16,        10)
REGDEF_END(DEFOG_FOG_MODIFY_REGISTER_8)

/*
    defog_air0:    [0x0, 0x3ff],			bits : 9_0
    defog_air1:    [0x0, 0x3ff],			bits : 25_16
*/
#define DEFOG_AIRLIGHT_REGISTER_0_OFS 0x0390
REGDEF_BEGIN(DEFOG_AIRLIGHT_REGISTER_0)
	REGDEF_BIT(defog_air0,        10)
	REGDEF_BIT(          ,        6)
	REGDEF_BIT(defog_air1,        10)
REGDEF_END(DEFOG_AIRLIGHT_REGISTER_0)

/*
    defog_air2:    [0x0, 0x3ff],			bits : 9_0
*/
#define DEFOG_AIRLIGHT_REGISTER_1_OFS 0x0394
REGDEF_BEGIN(DEFOG_AIRLIGHT_REGISTER_1)
	REGDEF_BIT(defog_air2,        10)
REGDEF_END(DEFOG_AIRLIGHT_REGISTER_1)

/*
    defog_fog_rto    :    [0x0, 0xff],			bits : 7_0
    defog_min_diff   :    [0x0, 0x3ff],			bits : 17_8
    defog_dgain_ratio:    [0x0, 0xff],			bits : 27_20
    defog_selfcmp_en :    [0x0, 0x1],			bits : 30
    defog_mode_sel   :    [0x0, 0x1],			bits : 31
*/
#define DEFOG_STRENGTH_CONTROL_REGISTER_5_OFS 0x03a8
REGDEF_BEGIN(DEFOG_STRENGTH_CONTROL_REGISTER_5)
	REGDEF_BIT(defog_fog_rto    ,        8)
	REGDEF_BIT(defog_min_diff   ,        10)
	REGDEF_BIT(                 ,        2)
	REGDEF_BIT(defog_dgain_ratio,        8)
	REGDEF_BIT(                 ,        2)
	REGDEF_BIT(defog_selfcmp_en ,        1)
	REGDEF_BIT(defog_mode_sel   ,        1)
REGDEF_END(DEFOG_STRENGTH_CONTROL_REGISTER_5)

/*
    defog_gain_th:    [0x0, 0xff],			bits : 7_0
*/
#define DEFOG_STRENGTH_CONTROL_REGISTER_6_OFS 0x03ac
REGDEF_BEGIN(DEFOG_STRENGTH_CONTROL_REGISTER_6)
	REGDEF_BIT(defog_gain_th,        8)
REGDEF_END(DEFOG_STRENGTH_CONTROL_REGISTER_6)

/*
    defog_outbld_lumwt0:    [0x0, 0xff],			bits : 7_0
    defog_outbld_lumwt1:    [0x0, 0xff],			bits : 15_8
    defog_outbld_lumwt2:    [0x0, 0xff],			bits : 23_16
    defog_outbld_lumwt3:    [0x0, 0xff],			bits : 31_24
*/
#define DEFOG_OUTPUT_BLENDING_REGISTER_0_OFS 0x03b0
REGDEF_BEGIN(DEFOG_OUTPUT_BLENDING_REGISTER_0)
	REGDEF_BIT(defog_outbld_lumwt0,        8)
	REGDEF_BIT(defog_outbld_lumwt1,        8)
	REGDEF_BIT(defog_outbld_lumwt2,        8)
	REGDEF_BIT(defog_outbld_lumwt3,        8)
REGDEF_END(DEFOG_OUTPUT_BLENDING_REGISTER_0)

/*
    defog_outbld_lumwt16 :    [0x0, 0xff],			bits : 7_0
    defog_wet_ref        :    [0x0, 0x1],			bits : 16
    defog_local_outbld_en:    [0x0, 0x1],			bits : 17
*/
#define DEFOG_OUTPUT_BLENDING_REGISTER_4_OFS 0x03c0
REGDEF_BEGIN(DEFOG_OUTPUT_BLENDING_REGISTER_4)
	REGDEF_BIT(defog_outbld_lumwt16 ,        8)
	REGDEF_BIT(                     ,        8)
	REGDEF_BIT(defog_wet_ref        ,        1)
	REGDEF_BIT(defog_local_outbld_en,        1)
REGDEF_END(DEFOG_OUTPUT_BLENDING_REGISTER_4)

/*
    defog_outbld_diffwt0:    [0x0, 0x3f],			bits : 5_0
    defog_outbld_diffwt1:    [0x0, 0x3f],			bits : 11_6
    defog_outbld_diffwt2:    [0x0, 0x3f],			bits : 17_12
    defog_outbld_diffwt3:    [0x0, 0x3f],			bits : 23_18
    defog_outbld_diffwt4:    [0x0, 0x3f],			bits : 29_24
*/
#define DEFOG_OUTPUT_BLENDING_REGISTER_5_OFS 0x03c4
REGDEF_BEGIN(DEFOG_OUTPUT_BLENDING_REGISTER_5)
	REGDEF_BIT(defog_outbld_diffwt0,        6)
	REGDEF_BIT(defog_outbld_diffwt1,        6)
	REGDEF_BIT(defog_outbld_diffwt2,        6)
	REGDEF_BIT(defog_outbld_diffwt3,        6)
	REGDEF_BIT(defog_outbld_diffwt4,        6)
REGDEF_END(DEFOG_OUTPUT_BLENDING_REGISTER_5)

/*
    defog_outbld_diffwt15:    [0x0, 0x3f],			bits : 5_0
    defog_outbld_diffwt16:    [0x0, 0x3f],			bits : 11_6
*/
#define DEFOG_OUTPUT_BLENDING_REGISTER_8_OFS 0x03d0
REGDEF_BEGIN(DEFOG_OUTPUT_BLENDING_REGISTER_8)
	REGDEF_BIT(defog_outbld_diffwt15,        6)
	REGDEF_BIT(defog_outbld_diffwt16,        6)
REGDEF_END(DEFOG_OUTPUT_BLENDING_REGISTER_8)

//=============================================================================
// IPE EDGE
//=============================================================================
/*
    w_con_eng:    [0x0, 0xf],           bits : 3_0
    w_low    :    [0x0, 0x1f],          bits : 8_4
    w_high   :    [0x0, 0x1f],          bits : 16_12
*/
#define EDGE_REGION_EXTRACTION_REGISTER_0_OFS 0x0088
REGDEF_BEGIN(EDGE_REGION_EXTRACTION_REGISTER_0)
REGDEF_BIT(w_con_eng,        4)
REGDEF_BIT(w_low,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(w_high,        5)
REGDEF_END(EDGE_REGION_EXTRACTION_REGISTER_0)

/*
    w_ker_thin    :    [0x0, 0xf],          bits : 3_0
    w_ker_robust  :    [0x0, 0xf],          bits : 7_4
    iso_ker_thin  :    [0x0, 0xf],          bits : 11_8
    iso_ker_robust:    [0x0, 0xf],          bits : 15_12
*/
#define EDGE_REGION_EXTRACTION_REGISTER_1_OFS 0x008c
REGDEF_BEGIN(EDGE_REGION_EXTRACTION_REGISTER_1)
REGDEF_BIT(w_ker_thin,        4)
REGDEF_BIT(w_ker_robust,        4)
REGDEF_BIT(iso_ker_thin,        4)
REGDEF_BIT(iso_ker_robust,        4)
REGDEF_END(EDGE_REGION_EXTRACTION_REGISTER_1)

/*
    w_hld_low         :    [0x0, 0x1f],         bits : 4_0
    w_hld_high        :    [0x0, 0x1f],         bits : 12_8
    w_ker_thin_hld    :    [0x0, 0xf],          bits : 19_16
    w_ker_robust_hld  :    [0x0, 0xf],          bits : 23_20
    iso_ker_thin_hld  :    [0x0, 0xf],          bits : 27_24
    iso_ker_robust_hld:    [0x0, 0xf],          bits : 31_28
*/
#define EDGE_REGION_EXTRACTION_REGISTER_2_OFS 0x0090
REGDEF_BEGIN(EDGE_REGION_EXTRACTION_REGISTER_2)
REGDEF_BIT(w_hld_low,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(w_hld_high,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(w_ker_thin_hld,        4)
REGDEF_BIT(w_ker_robust_hld,        4)
REGDEF_BIT(iso_ker_thin_hld,        4)
REGDEF_BIT(iso_ker_robust_hld,        4)
REGDEF_END(EDGE_REGION_EXTRACTION_REGISTER_2)

/*
    th_flat:    [0x0, 0x3ff],           bits : 9_0
    th_edge:    [0x0, 0x3ff],           bits : 21_12
*/
#define EDGE_REGION_EXTRACTION_REGISTER_3_OFS 0x0094
REGDEF_BEGIN(EDGE_REGION_EXTRACTION_REGISTER_3)
REGDEF_BIT(th_flat,        10)
REGDEF_BIT(,        2)
REGDEF_BIT(th_edge,        10)
REGDEF_END(EDGE_REGION_EXTRACTION_REGISTER_3)

/*
    th_hld_flat:    [0x0, 0x3ff],			bits : 9_0
    th_hld_edge:    [0x0, 0x3ff],			bits : 21_12
    th_hld_lum :    [0x0, 0x3ff],			bits : 31_22
*/
#define EDGE_REGION_EXTRACTION_REGISTER_4_OFS 0x0098
REGDEF_BEGIN(EDGE_REGION_EXTRACTION_REGISTER_4)
	REGDEF_BIT(th_hld_flat,        10)
	REGDEF_BIT(           ,        2)
	REGDEF_BIT(th_hld_edge,        10)
	REGDEF_BIT(th_hld_lum ,        10)
REGDEF_END(EDGE_REGION_EXTRACTION_REGISTER_4)

/*
    slope_con_eng    :    [0x0, 0xffff],            bits : 15_0
    slope_hld_con_eng:    [0x0, 0xffff],            bits : 31_16
*/
#define EDGE_REGION_EXTRACTION_REGISTER_5_OFS 0x009c
REGDEF_BEGIN(EDGE_REGION_EXTRACTION_REGISTER_5)
REGDEF_BIT(slope_con_eng,        16)
REGDEF_BIT(slope_hld_con_eng,        16)
REGDEF_END(EDGE_REGION_EXTRACTION_REGISTER_5)

/*
    slope_flat:    [0x0, 0xffff],           bits : 15_0
    slope_edge:    [0x0, 0xffff],           bits : 31_16
*/
#define EDGE_REGION_STRENGTH_REGISTER2_OFS 0x00e4
REGDEF_BEGIN(EDGE_REGION_STRENGTH_REGISTER2)
REGDEF_BIT(slope_flat,        16)
REGDEF_BIT(slope_edge,        16)
REGDEF_END(EDGE_REGION_STRENGTH_REGISTER2)

/*
    str_flat:    [0x0, 0xff],           bits : 7_0
    str_edge:    [0x0, 0xff],           bits : 15_8
*/
#define EDGE_REGION_STRENGTH_REGISTER3_OFS 0x00e8
REGDEF_BEGIN(EDGE_REGION_STRENGTH_REGISTER3)
REGDEF_BIT(str_flat,        8)
REGDEF_BIT(str_edge,        8)
REGDEF_END(EDGE_REGION_STRENGTH_REGISTER3)

/*
    y_enh_p :    [0x0, 0x3ff],          bits : 9_0
    y_enh_n :    [0x0, 0x3ff],          bits : 19_10
    y_einv_p:    [0x0, 0x1],            bits : 30
    y_einv_n:    [0x0, 0x1],            bits : 31
*/
#define COLOR_COMPONENT_ADJUSTMENT_REGISTER_0_OFS 0x01b8
REGDEF_BEGIN(COLOR_COMPONENT_ADJUSTMENT_REGISTER_0)
REGDEF_BIT(y_enh_p,        10)
REGDEF_BIT(y_enh_n,        10)
REGDEF_BIT(,        10)
REGDEF_BIT(y_einv_p,        1)
REGDEF_BIT(y_einv_n,        1)
REGDEF_END(COLOR_COMPONENT_ADJUSTMENT_REGISTER_0)

//=============================================================================
// IPE EDGE EStab & EDtab & Colortab
//=============================================================================
/*
    esthrl:    [0x0, 0x3ff],            bits : 9_0
    esthrh:    [0x0, 0x3ff],            bits : 19_10
    establ:    [0x0, 0x7],          bits : 22_20
    estabh:    [0x0, 0x7],          bits : 26_24
*/
#define EDGE_LUMINANCE_PROCESS_REGISTER_0_OFS 0x00b4
REGDEF_BEGIN(EDGE_LUMINANCE_PROCESS_REGISTER_0)
REGDEF_BIT(esthrl,        10)
REGDEF_BIT(esthrh,        10)
REGDEF_BIT(establ,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(estabh,        3)
REGDEF_END(EDGE_LUMINANCE_PROCESS_REGISTER_0)

/*
    eslutl0:    [0x0, 0xff],            bits : 7_0
    eslutl1:    [0x0, 0xff],            bits : 15_8
    eslutl2:    [0x0, 0xff],            bits : 23_16
    eslutl3:    [0x0, 0xff],            bits : 31_24
*/
#define EDGE_LUMINANCE_PROCESS_REGISTER_1_OFS 0x00b8
REGDEF_BEGIN(EDGE_LUMINANCE_PROCESS_REGISTER_1)
REGDEF_BIT(eslutl0,        8)
REGDEF_BIT(eslutl1,        8)
REGDEF_BIT(eslutl2,        8)
REGDEF_BIT(eslutl3,        8)
REGDEF_END(EDGE_LUMINANCE_PROCESS_REGISTER_1)

/*
    edthrl :    [0x0, 0x3ff],           bits : 9_0
    edthrh :    [0x0, 0x3ff],           bits : 19_10
    edtabl :    [0x0, 0x7],         bits : 22_20
    edtabh :    [0x0, 0x7],         bits : 26_24
    edinsel:    [0x0, 0x3],         bits : 29_28
*/
#define EDGE_DMAP_PROCESS_REGISTER_0_OFS 0x00c8
REGDEF_BEGIN(EDGE_DMAP_PROCESS_REGISTER_0)
REGDEF_BIT(edthrl,        10)
REGDEF_BIT(edthrh,        10)
REGDEF_BIT(edtabl,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(edtabh,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(edinsel,        2)
REGDEF_END(EDGE_DMAP_PROCESS_REGISTER_0)

/*
    edlutl0:    [0x0, 0xff],            bits : 7_0
    edlutl1:    [0x0, 0xff],            bits : 15_8
    edlutl2:    [0x0, 0xff],            bits : 23_16
    edlutl3:    [0x0, 0xff],            bits : 31_24
*/
#define EDGE_DMAP_PROCESS_REGISTER_1_OFS 0x00cc
REGDEF_BEGIN(EDGE_DMAP_PROCESS_REGISTER_1)
REGDEF_BIT(edlutl0,        8)
REGDEF_BIT(edlutl1,        8)
REGDEF_BIT(edlutl2,        8)
REGDEF_BIT(edlutl3,        8)
REGDEF_END(EDGE_DMAP_PROCESS_REGISTER_1)

/*
    cedgm0:    [0x0, 0xff],         bits : 7_0
    cedgm1:    [0x0, 0xff],         bits : 15_8
    cedgm2:    [0x0, 0xff],         bits : 23_16
    cedgm3:    [0x0, 0xff],         bits : 31_24
*/
#define COLOR_CONTROL_EDGE_MAPPING_REGISTER_0_OFS 0x0194
REGDEF_BEGIN(COLOR_CONTROL_EDGE_MAPPING_REGISTER_0)
REGDEF_BIT(cedgm0,        8)
REGDEF_BIT(cedgm1,        8)
REGDEF_BIT(cedgm2,        8)
REGDEF_BIT(cedgm3,        8)
REGDEF_END(COLOR_CONTROL_EDGE_MAPPING_REGISTER_0)
/*
    EEXT_DIR_S_TH_MUL  :    [0x0, 0x7],			bits : 2_0
    EEXT_DIR_S_TH_SHIFT:    [0x0, 0x3],			bits : 5_4
    DIR_ENG_BLEND_W1   :    [0x0, 0x1f],			bits : 12_8
    DIR_ENG_BLEND_W2   :    [0x0, 0x1f],			bits : 17_13
    DIR_ENG_BLEND_W3   :    [0x0, 0x1f],			bits : 22_18
    DIR_ENG_BLEND_W4   :    [0x0, 0x1f],			bits : 27_23
    S_COUNT_TH         :    [0x0, 0x7],			bits : 30_28
*/
#define EDGE_EXTRACTION_REGISTER_9_OFS 0x0200
REGDEF_BEGIN(EDGE_EXTRACTION_REGISTER_9)
	REGDEF_BIT(eext_dir_s_th_mul  ,        3)
	REGDEF_BIT(                   ,        1)
	REGDEF_BIT(eext_dir_s_th_shift,        2)
	REGDEF_BIT(                   ,        2)
	REGDEF_BIT(dir_eng_blend_w1   ,        5)
	REGDEF_BIT(dir_eng_blend_w2   ,        5)
	REGDEF_BIT(dir_eng_blend_w3   ,        5)
	REGDEF_BIT(dir_eng_blend_w4   ,        5)
	REGDEF_BIT(s_count_th         ,        3)
REGDEF_END(EDGE_EXTRACTION_REGISTER_9)

//=============================================================================
// IPE EDGE OverShoot
//=============================================================================
/*
    th_overshoot     :    [0x0, 0xff],          bits : 7_0
    th_undershoot    :    [0x0, 0xff],          bits : 15_8
    th_undershoot_lum:    [0x0, 0xff],          bits : 23_16
    th_undershoot_eng:    [0x0, 0xff],          bits : 31_24
*/
#define OVERSHOOTING_CONTROL_REGISTER_1_OFS 0x00a4
REGDEF_BEGIN(OVERSHOOTING_CONTROL_REGISTER_1)
REGDEF_BIT(th_overshoot,        8)
REGDEF_BIT(th_undershoot,        8)
REGDEF_BIT(th_undershoot_lum,        8)
REGDEF_BIT(th_undershoot_eng,        8)
REGDEF_END(OVERSHOOTING_CONTROL_REGISTER_1)

/*
    slope_overshoot :    [0x0, 0x7fff],         bits : 14_0
    slope_undershoot:    [0x0, 0x7fff],         bits : 30_16
*/
#define OVERSHOOTING_CONTROL_REGISTER_3_OFS 0x00ac
REGDEF_BEGIN(OVERSHOOTING_CONTROL_REGISTER_3)
REGDEF_BIT(slope_overshoot,        15)
REGDEF_BIT(,        1)
REGDEF_BIT(slope_undershoot,        15)
REGDEF_END(OVERSHOOTING_CONTROL_REGISTER_3)

//=============================================================================
// IPE EDGE Gamma
//=============================================================================
/*
    edge_lut0:    [0x0, 0x3ff],         bits : 9_0
    edge_lut1:    [0x0, 0x3ff],         bits : 19_10
    edge_lut2:    [0x0, 0x3ff],         bits : 29_20
*/
#define EDGE_GAMMA0_OFS 0x08a0
REGDEF_BEGIN(EDGE_GAMMA0)
REGDEF_BIT(edge_lut0,        10)
REGDEF_BIT(edge_lut1,        10)
REGDEF_BIT(edge_lut2,        10)
REGDEF_END(EDGE_GAMMA0)

/*
    edge_lut63:    [0x0, 0x3ff],            bits : 9_0
    edge_lut64:    [0x0, 0x3ff],            bits : 19_10
*/
#define EDGE_GAMMA21_OFS 0x08f4
REGDEF_BEGIN(EDGE_GAMMA21)
REGDEF_BIT(edge_lut63,        10)
REGDEF_BIT(edge_lut64,        10)
REGDEF_END(EDGE_GAMMA21)

//=============================================================================//
//                                                                             //
//                                 NT98538 IME                                 //
//                                                                             //
//=============================================================================//
/*
    ime_src                :    [0x0, 0x1],         bits : 0
    ime_dir_ctrl           :    [0x0, 0x1],         bits : 1
    ime_p0_en              :    [0x0, 0x1],         bits : 2
    ime_p1_en              :    [0x0, 0x1],         bits : 3
    ime_p2_en              :    [0x0, 0x1],         bits : 4
    ime_p3_en              :    [0x0, 0x1],         bits : 5
    ime_lca_en             :    [0x0, 0x1],         bits : 10
    ime_dbcs_en            :    [0x0, 0x1],         bits : 13
    ime_fisheye_mask_en    :    [0x0, 0x1],         bits : 15
    ime_nn_isp_p2_3dnr_en  :    [0x0, 0x1],         bits : 16
    ime_nn_isp_p3_2dnr_en  :    [0x0, 0x1],         bits : 17
    ime_post_shp_en        :    [0x0, 0x1],         bits : 24
    ime_3dnr_en            :    [0x0, 0x1],         bits : 27
    ime_3dnr_ref_out_sel   :    [0x0, 0x1],         bits : 28
    ime_3dnr_ref_in_dec_en :    [0x0, 0x1],         bits : 29
    ime_3dnr_ref_out_en    :    [0x0, 0x1],         bits : 30
    ime_3dnr_ref_out_enc_en:    [0x0, 0x1],         bits : 31
*/
#define IME_FUNCTION_CONTROL_REGISTER0_OFS 0x0004
REGDEF_BEGIN(IME_FUNCTION_CONTROL_REGISTER0)
REGDEF_BIT(ime_src,        1)
REGDEF_BIT(ime_dir_ctrl,        1)
REGDEF_BIT(ime_p0_en,        1)
REGDEF_BIT(ime_p1_en,        1)
REGDEF_BIT(ime_p2_en,        1)
REGDEF_BIT(ime_p3_en,        1)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_lca_en,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_dbcs_en,        1)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_fisheye_mask_en,        1)
REGDEF_BIT(ime_nn_isp_p2_3dnr_en,        1)
REGDEF_BIT(ime_nn_isp_p3_2dnr_en,        1)
REGDEF_BIT(,        6)
REGDEF_BIT(ime_post_shp_en,        1)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_en,        1)
REGDEF_BIT(ime_3dnr_ref_out_sel,        1)
REGDEF_BIT(ime_3dnr_ref_in_dec_en,        1)
REGDEF_BIT(ime_3dnr_ref_out_en,        1)
REGDEF_BIT(ime_3dnr_ref_out_enc_en,        1)
REGDEF_END(IME_FUNCTION_CONTROL_REGISTER0)

//=============================================================================
// IME LCA
//=============================================================================
/*
    lca_edge_th0:    [0x0, 0x1ffff],            bits : 16_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER33_OFS 0x0bf4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER33)
REGDEF_BIT(lca_edge_th0,        17)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER33)

/*
    lca_edge_th1:    [0x0, 0x1ffff],            bits : 16_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER34_OFS 0x0bf8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER34)
REGDEF_BIT(lca_edge_th1,        17)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER34)

//=============================================================================
// IME LCA filter
//=============================================================================
/*
    lca_ctr_y_th0:    [0x0, 0xff],          bits : 7_0
    lca_ctr_y_th1:    [0x0, 0xff],          bits : 15_8
    lca_ctr_y_th2:    [0x0, 0xff],          bits : 23_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER1_OFS 0x0b74
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER1)
REGDEF_BIT(lca_ctr_y_th0,        8)
REGDEF_BIT(lca_ctr_y_th1,        8)
REGDEF_BIT(lca_ctr_y_th2,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER1)

/*
    lca_ctr_u_th0:    [0x0, 0xff],          bits : 7_0
    lca_ctr_u_th1:    [0x0, 0xff],          bits : 15_8
    lca_ctr_u_th2:    [0x0, 0xff],          bits : 23_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER2_OFS 0x0b78
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER2)
REGDEF_BIT(lca_ctr_u_th0,        8)
REGDEF_BIT(lca_ctr_u_th1,        8)
REGDEF_BIT(lca_ctr_u_th2,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER2)

/*
    lca_ctr_v_th0:    [0x0, 0xff],          bits : 7_0
    lca_ctr_v_th1:    [0x0, 0xff],          bits : 15_8
    lca_ctr_v_th2:    [0x0, 0xff],          bits : 23_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER3_OFS 0x0b7c
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER3)
REGDEF_BIT(lca_ctr_v_th0,        8)
REGDEF_BIT(lca_ctr_v_th1,        8)
REGDEF_BIT(lca_ctr_v_th2,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER3)

/*
    lca_still_smooth_y_th0:    [0x0, 0xff],         bits : 7_0
    lca_still_smooth_y_th1:    [0x0, 0xff],         bits : 15_8
    lca_still_smooth_y_th2:    [0x0, 0xff],         bits : 23_16
    lca_still_smooth_y_th3:    [0x0, 0xff],         bits : 31_24
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER5_OFS 0x0b84
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER5)
REGDEF_BIT(lca_still_smooth_y_th0,        8)
REGDEF_BIT(lca_still_smooth_y_th1,        8)
REGDEF_BIT(lca_still_smooth_y_th2,        8)
REGDEF_BIT(lca_still_smooth_y_th3,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER5)

/*
    lca_still_smooth_y_th4:    [0x0, 0xff],         bits : 7_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER6_OFS 0x0b88
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER6)
REGDEF_BIT(lca_still_smooth_y_th4,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER6)

/*
    lca_still_smooth_uv_th0:    [0x0, 0xffff],          bits : 15_0
    lca_still_smooth_uv_th1:    [0x0, 0xffff],          bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER7_OFS 0x0b8c
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER7)
REGDEF_BIT(lca_still_smooth_uv_th0,        16)
REGDEF_BIT(lca_still_smooth_uv_th1,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER7)

/*
    lca_still_smooth_uv_th2:    [0x0, 0xffff],          bits : 15_0
    lca_still_smooth_uv_th3:    [0x0, 0xffff],          bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER8_OFS 0x0b90
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER8)
REGDEF_BIT(lca_still_smooth_uv_th2,        16)
REGDEF_BIT(lca_still_smooth_uv_th3,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER8)

/*
    lca_still_smooth_uv_th4:    [0x0, 0xffff],          bits : 15_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER9_OFS 0x0b94
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER9)
REGDEF_BIT(lca_still_smooth_uv_th4,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER9)

/*
    lca_still_edge_y_th0:    [0x0, 0xff],           bits : 7_0
    lca_still_edge_y_th1:    [0x0, 0xff],           bits : 15_8
    lca_still_edge_y_th2:    [0x0, 0xff],           bits : 23_16
    lca_still_edge_y_th3:    [0x0, 0xff],           bits : 31_24
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER10_OFS 0x0b98
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER10)
REGDEF_BIT(lca_still_edge_y_th0,        8)
REGDEF_BIT(lca_still_edge_y_th1,        8)
REGDEF_BIT(lca_still_edge_y_th2,        8)
REGDEF_BIT(lca_still_edge_y_th3,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER10)

/*
    lca_still_edge_y_th4:    [0x0, 0xff],           bits : 7_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER11_OFS 0x0b9c
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER11)
REGDEF_BIT(lca_still_edge_y_th4,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER11)

/*
    lca_still_edge_uv_th0:    [0x0, 0xffff],            bits : 15_0
    lca_still_edge_uv_th1:    [0x0, 0xffff],            bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER12_OFS 0x0ba0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER12)
REGDEF_BIT(lca_still_edge_uv_th0,        16)
REGDEF_BIT(lca_still_edge_uv_th1,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER12)

/*
    lca_still_edge_uv_th2:    [0x0, 0xffff],            bits : 15_0
    lca_still_edge_uv_th3:    [0x0, 0xffff],            bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER13_OFS 0x0ba4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER13)
REGDEF_BIT(lca_still_edge_uv_th2,        16)
REGDEF_BIT(lca_still_edge_uv_th3,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER13)

/*
    lca_still_edge_uv_th4:    [0x0, 0xffff],            bits : 15_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER14_OFS 0x0ba8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER14)
REGDEF_BIT(lca_still_edge_uv_th4,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER14)

/*
    lca_motion_y_th0:    [0x0, 0xff],           bits : 7_0
    lca_motion_y_th1:    [0x0, 0xff],           bits : 15_8
    lca_motion_y_th2:    [0x0, 0xff],           bits : 23_16
    lca_motion_y_th3:    [0x0, 0xff],           bits : 31_24
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER15_OFS 0x0bac
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER15)
REGDEF_BIT(lca_motion_y_th0,        8)
REGDEF_BIT(lca_motion_y_th1,        8)
REGDEF_BIT(lca_motion_y_th2,        8)
REGDEF_BIT(lca_motion_y_th3,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER15)

/*
    lca_motion_y_th4:    [0x0, 0xff],           bits : 7_0
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER16_OFS 0x0bb0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER16)
REGDEF_BIT(lca_motion_y_th4,        8)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER16)

/*
    lca_motion_uv_th0:    [0x0, 0xffff],            bits : 15_0
    lca_motion_uv_th1:    [0x0, 0xffff],            bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER17_OFS 0x0bb4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER17)
REGDEF_BIT(lca_motion_uv_th0,        16)
REGDEF_BIT(lca_motion_uv_th1,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER17)

/*
    lca_motion_uv_th2:    [0x0, 0xffff],            bits : 15_0
    lca_motion_uv_th3:    [0x0, 0xffff],            bits : 31_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER18_OFS 0x0bb8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER18)
REGDEF_BIT(lca_motion_uv_th2,        16)
REGDEF_BIT(lca_motion_uv_th3,        16)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER18)

/*
    lca_motion_uv_th4:    [0x0, 0xffff],            bits : 15_0
    lca_out_wet_y    :    [0x0, 0x3f],          bits : 25_20
    lca_out_wet_uv   :    [0x0, 0x3f],          bits : 31_26
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER19_OFS 0x0bbc
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER19)
REGDEF_BIT(lca_motion_uv_th4,        16)
REGDEF_BIT(,        4)
REGDEF_BIT(lca_out_wet_y,        6)
REGDEF_BIT(lca_out_wet_uv,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER19)

//=============================================================================
// IME LCA coring gain
//=============================================================================
/*
    lca_still_y_gain0:    [0x0, 0x3f],          bits : 5_0
    lca_still_y_gain1:    [0x0, 0x3f],          bits : 13_8
    lca_still_y_gain2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER20_OFS 0x0bc0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER20)
REGDEF_BIT(lca_still_y_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_y_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_y_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER20)

/*
    lca_motion_y_gain0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_y_gain1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_y_gain2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER21_OFS 0x0bc4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER21)
REGDEF_BIT(lca_motion_y_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_y_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_y_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER21)

/*
    lca_still_u_gain0:    [0x0, 0x3f],          bits : 5_0
    lca_still_u_gain1:    [0x0, 0x3f],          bits : 13_8
    lca_still_u_gain2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER22_OFS 0x0bc8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER22)
REGDEF_BIT(lca_still_u_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_u_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_u_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER22)

/*
    lca_motion_u_gain0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_u_gain1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_u_gain2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER23_OFS 0x0bcc
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER23)
REGDEF_BIT(lca_motion_u_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_u_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_u_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER23)

/*
    lca_still_v_gain0:    [0x0, 0x3f],          bits : 5_0
    lca_still_v_gain1:    [0x0, 0x3f],          bits : 13_8
    lca_still_v_gain2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER24_OFS 0x0bd0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER24)
REGDEF_BIT(lca_still_v_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_v_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_v_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER24)

/*
    lca_motion_v_gain0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_v_gain1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_v_gain2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER25_OFS 0x0bd4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER25)
REGDEF_BIT(lca_motion_v_gain0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_v_gain1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_v_gain2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER25)

//=============================================================================
// IME LCA cutoff
//=============================================================================
/*
    lca_still_y_coff0:    [0x0, 0x3f],          bits : 5_0
    lca_still_y_coff1:    [0x0, 0x3f],          bits : 13_8
    lca_still_y_coff2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER26_OFS 0x0bd8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER26)
REGDEF_BIT(lca_still_y_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_y_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_y_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER26)

/*
    lca_motion_y_coff0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_y_coff1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_y_coff2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER27_OFS 0x0bdc
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER27)
REGDEF_BIT(lca_motion_y_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_y_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_y_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER27)

/*
    lca_still_u_coff0:    [0x0, 0x3f],          bits : 5_0
    lca_still_u_coff1:    [0x0, 0x3f],          bits : 13_8
    lca_still_u_coff2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER28_OFS 0x0be0
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER28)
REGDEF_BIT(lca_still_u_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_u_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_u_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER28)

/*
    lca_motion_u_coff0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_u_coff1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_u_coff2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER29_OFS 0x0be4
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER29)
REGDEF_BIT(lca_motion_u_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_u_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_u_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER29)

/*
    lca_still_v_coff0:    [0x0, 0x3f],          bits : 5_0
    lca_still_v_coff1:    [0x0, 0x3f],          bits : 13_8
    lca_still_v_coff2:    [0x0, 0x3f],          bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER30_OFS 0x0be8
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER30)
REGDEF_BIT(lca_still_v_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_v_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_still_v_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER30)

/*
    lca_motion_v_coff0:    [0x0, 0x3f],         bits : 5_0
    lca_motion_v_coff1:    [0x0, 0x3f],         bits : 13_8
    lca_motion_v_coff2:    [0x0, 0x3f],         bits : 21_16
*/
#define LOCAL_CHROMA_ADAPTATION_REGISTER31_OFS 0x0bec
REGDEF_BEGIN(LOCAL_CHROMA_ADAPTATION_REGISTER31)
REGDEF_BIT(lca_motion_v_coff0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_v_coff1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(lca_motion_v_coff2,        6)
REGDEF_END(LOCAL_CHROMA_ADAPTATION_REGISTER31)

//=============================================================================
// IME DBCS
//=============================================================================
/*
    dbcs_ctr_u  :    [0x0, 0xff],           bits : 7_0
    dbcs_ctr_v  :    [0x0, 0xff],           bits : 15_8
    dbcs_mode   :    [0x0, 0x3],            bits : 17_16
    dbcs_step_y :    [0x0, 0x3],            bits : 21_20
    dbcs_step_uv:    [0x0, 0x3],            bits : 23_22
*/
#define IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_REGISTER0_OFS 0x0380
REGDEF_BEGIN(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_REGISTER0)
REGDEF_BIT(dbcs_ctr_u,        8)
REGDEF_BIT(dbcs_ctr_v,        8)
REGDEF_BIT(dbcs_mode,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(dbcs_step_y,        2)
REGDEF_BIT(dbcs_step_uv,        2)
REGDEF_END(IME_DARK_AND_BRIGHT_REGION_CHROMA_SUPPRESSION_REGISTER0)

//=============================================================================
// IME TMNR4 DBG
//=============================================================================
/*
    ime_3dnr_ne_sample_step_x   :    [0x0, 0xff],           bits : 7_0
    ime_3dnr_ne_sample_step_y   :    [0x0, 0xff],           bits : 15_8
    ime_3dnr_statistic_output_en:    [0x0, 0x1],            bits : 16
    ime_3dnr_ps_fastc_en        :    [0x0, 0x1],            bits : 17
    ime_3dnr_dbg_mv0            :    [0x0, 0x1],            bits : 18
    ime_3dnr_dbg_mode           :    [0x0, 0xf],            bits : 31_28
*/
#define IME_TMNR_CONTROL_REGISTER1_OFS 0x0704
REGDEF_BEGIN(IME_TMNR_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_ne_sample_step_x,        8)
REGDEF_BIT(ime_3dnr_ne_sample_step_y,        8)
REGDEF_BIT(ime_3dnr_statistic_output_en,        1)
REGDEF_BIT(ime_3dnr_ps_fastc_en,        1)
REGDEF_BIT(ime_3dnr_dbg_mv0,        1)
REGDEF_BIT(,        9)
REGDEF_BIT(ime_3dnr_dbg_mode,        4)
REGDEF_END(IME_TMNR_CONTROL_REGISTER1)

//=============================================================================
// IME TMNR4 MEMC
//=============================================================================
/*
    ime_3dnr_me_sad_penalty_0:    [0x0, 0x3ff],         bits : 9_0
    ime_3dnr_me_sad_penalty_1:    [0x0, 0x3ff],         bits : 19_10
    ime_3dnr_me_sad_penalty_2:    [0x0, 0x3ff],         bits : 29_20
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER0_OFS 0x0728
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_me_sad_penalty_0,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_1,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_2,        10)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER0)

/*
    ime_3dnr_me_sad_penalty_3:    [0x0, 0x3ff],         bits : 9_0
    ime_3dnr_me_sad_penalty_4:    [0x0, 0x3ff],         bits : 19_10
    ime_3dnr_me_sad_penalty_5:    [0x0, 0x3ff],         bits : 29_20
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER1_OFS 0x072c
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_me_sad_penalty_3,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_4,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_5,        10)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER1)

/*
    ime_3dnr_me_sad_penalty_6:    [0x0, 0x3ff],         bits : 9_0
    ime_3dnr_me_sad_penalty_7:    [0x0, 0x3ff],         bits : 19_10
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER2_OFS 0x0730
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER2)
REGDEF_BIT(ime_3dnr_me_sad_penalty_6,        10)
REGDEF_BIT(ime_3dnr_me_sad_penalty_7,        10)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER2)

/*
    ime_3dnr_me_switch_th0:    [0x0, 0xff],         bits : 7_0
    ime_3dnr_me_switch_th1:    [0x0, 0xff],         bits : 15_8
    ime_3dnr_me_switch_th2:    [0x0, 0xff],         bits : 23_16
    ime_3dnr_me_switch_th3:    [0x0, 0xff],         bits : 31_24
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER3_OFS 0x0734
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER3)
REGDEF_BIT(ime_3dnr_me_switch_th0,        8)
REGDEF_BIT(ime_3dnr_me_switch_th1,        8)
REGDEF_BIT(ime_3dnr_me_switch_th2,        8)
REGDEF_BIT(ime_3dnr_me_switch_th3,        8)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER3)

/*
    ime_3dnr_me_switch_ratio:    [0x0, 0xff],           bits : 7_0
    ime_3dnr_me_cost_blend  :    [0x0, 0xf],            bits : 11_8
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER5_OFS 0x073c
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER5)
REGDEF_BIT(ime_3dnr_me_switch_ratio,        8)
REGDEF_BIT(ime_3dnr_me_cost_blend,        4)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER5)

/*
    ime_3dnr_me_detail_penalty0:    [0x0, 0xf],         bits : 3_0
    ime_3dnr_me_detail_penalty1:    [0x0, 0xf],         bits : 7_4
    ime_3dnr_me_detail_penalty2:    [0x0, 0xf],         bits : 11_8
    ime_3dnr_me_detail_penalty3:    [0x0, 0xf],         bits : 15_12
    ime_3dnr_me_detail_penalty4:    [0x0, 0xf],         bits : 19_16
    ime_3dnr_me_detail_penalty5:    [0x0, 0xf],         bits : 23_20
    ime_3dnr_me_detail_penalty6:    [0x0, 0xf],         bits : 27_24
    ime_3dnr_me_detail_penalty7:    [0x0, 0xf],         bits : 31_28
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER6_OFS 0x0740
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER6)
REGDEF_BIT(ime_3dnr_me_detail_penalty0,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty1,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty2,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty3,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty4,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty5,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty6,        4)
REGDEF_BIT(ime_3dnr_me_detail_penalty7,        4)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER6)

/*
    ime_3dnr_me_probability0:    [0x0, 0x1],            bits : 0
    ime_3dnr_me_probability1:    [0x0, 0x1],            bits : 1
    ime_3dnr_me_probability2:    [0x0, 0x1],            bits : 2
    ime_3dnr_me_probability3:    [0x0, 0x1],            bits : 3
    ime_3dnr_me_probability4:    [0x0, 0x1],            bits : 4
    ime_3dnr_me_probability5:    [0x0, 0x1],            bits : 5
    ime_3dnr_me_probability6:    [0x0, 0x1],            bits : 6
    ime_3dnr_me_probability7:    [0x0, 0x1],            bits : 7
    ime_3dnr_me_rand_bit_x  :    [0x0, 0x7],            bits : 10_8
    ime_3dnr_me_rand_bit_y  :    [0x0, 0x7],            bits : 14_12
    ime_3dnr_me_min_detail  :    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER7_OFS 0x0744
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER7)
REGDEF_BIT(ime_3dnr_me_probability0,        1)
REGDEF_BIT(ime_3dnr_me_probability1,        1)
REGDEF_BIT(ime_3dnr_me_probability2,        1)
REGDEF_BIT(ime_3dnr_me_probability3,        1)
REGDEF_BIT(ime_3dnr_me_probability4,        1)
REGDEF_BIT(ime_3dnr_me_probability5,        1)
REGDEF_BIT(ime_3dnr_me_probability6,        1)
REGDEF_BIT(ime_3dnr_me_probability7,        1)
REGDEF_BIT(ime_3dnr_me_rand_bit_x,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_me_rand_bit_y,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_me_min_detail,        14)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER7)

/*
    ime_3dnr_pf_str:    [0x0, 0xff],            bits : 7_0
*/
#define IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER8_OFS 0x0748
REGDEF_BEGIN(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER8)
REGDEF_BIT(ime_3dnr_pf_str,        8)
REGDEF_END(IME_3DNR_MOTION_ESTIMATION_CONTROL_REGISTER8)

//=============================================================================
// IME TMNR4 FCVG
//=============================================================================
/*
    ime_3dnr_fast_converge_sp  :    [0x0, 0xf],         bits : 3_0
    ime_3dnr_fast_converge_step:    [0x0, 0xf],         bits : 7_4
*/
#define IME_3DNR_FAST_CONVERGE_CONTROL_REGISTER0_OFS 0x0754
REGDEF_BEGIN(IME_3DNR_FAST_CONVERGE_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_fast_converge_sp,        4)
REGDEF_BIT(ime_3dnr_fast_converge_step,        4)
REGDEF_END(IME_3DNR_FAST_CONVERGE_CONTROL_REGISTER0)

//=============================================================================
// IME TMNR4 Color Noise Supression
//=============================================================================
/*
    ime_3dnr_nr_motion_sat_ratio :    [0x0, 0xff],          bits : 7_0
    ime_3dnr_nr_c_tf0_residual_th:    [0x0, 0xf],           bits : 11_8
*/
#define IME_3DNR_PS_CONTROL_REGISTER3_OFS 0x082c
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER3)
REGDEF_BIT(ime_3dnr_nr_motion_sat_ratio,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_residual_th,        4)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER3)

/*
    ime_3dnr_nr_cshk_th0:    [0x0, 0x3ff],          bits : 9_0
    ime_3dnr_nr_cshk_th1:    [0x0, 0x3ff],          bits : 25_16
*/
#define IME_3DNR_COLOR_DITHERING_REGISTER0_OFS 0x0830
REGDEF_BEGIN(IME_3DNR_COLOR_DITHERING_REGISTER0)
REGDEF_BIT(ime_3dnr_nr_cshk_th0,        10)
REGDEF_BIT(,        6)
REGDEF_BIT(ime_3dnr_nr_cshk_th1,        10)
REGDEF_END(IME_3DNR_COLOR_DITHERING_REGISTER0)

/*
    ime_3dnr_nr_cshk_val0:    [0x0, 0x7],           bits : 2_0
    ime_3dnr_nr_cshk_val1:    [0x0, 0x7],           bits : 6_4
    ime_3dnr_nr_cshk_val2:    [0x0, 0x7],           bits : 10_8
    ime_3dnr_nr_cshk_val3:    [0x0, 0x7],           bits : 14_12
    ime_3dnr_nr_cshk_val4:    [0x0, 0x7],           bits : 18_16
    ime_3dnr_nr_cshk_val5:    [0x0, 0x7],           bits : 22_20
    ime_3dnr_nr_cshk_val6:    [0x0, 0x7],           bits : 26_24
    ime_3dnr_nr_cshk_val7:    [0x0, 0x7],           bits : 30_28
*/
#define IME_3DNR_COLOR_DITHERING_REGISTER4_OFS 0x0840
REGDEF_BEGIN(IME_3DNR_COLOR_DITHERING_REGISTER4)
REGDEF_BIT(ime_3dnr_nr_cshk_val0,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val1,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val2,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val3,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val4,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val5,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val6,        3)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_cshk_val7,        3)
REGDEF_END(IME_3DNR_COLOR_DITHERING_REGISTER4)

//=============================================================================
// IME TMNR4 NE (MD)
//=============================================================================
/*
    ime_3dnr_md_sad_coef_a0:    [0x0, 0x3f],            bits : 5_0
    ime_3dnr_md_sad_coef_a1:    [0x0, 0x3f],            bits : 13_8
    ime_3dnr_md_sad_coef_a2:    [0x0, 0x3f],            bits : 21_16
    ime_3dnr_md_sad_coef_a3:    [0x0, 0x3f],            bits : 29_24
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER0_OFS 0x0760
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_md_sad_coef_a0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_a1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_a2,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_a3,        6)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER0)

/*
    ime_3dnr_md_sad_coef_b0:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_md_sad_coef_b1:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER2_OFS 0x0768
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER2)
REGDEF_BIT(ime_3dnr_md_sad_coef_b0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_coef_b1,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER2)

/*
    ime_3dnr_md_sad_std0:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_md_sad_std1:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER6_OFS 0x0778
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER6)
REGDEF_BIT(ime_3dnr_md_sad_std0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_sad_std1,        14)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER6)

/*
    ime_3dnr_md_k1:    [0x0, 0x3f],         bits : 5_0
    ime_3dnr_md_k2:    [0x0, 0x3f],         bits : 13_8
*/
#define IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER10_OFS 0x0788
REGDEF_BEGIN(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER10)
REGDEF_BIT(ime_3dnr_md_k1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_md_k2,        6)
REGDEF_END(IME_3DNR_MOTION_DETECTION_CONTROL_REGISTER10)

//=============================================================================
// IME TMNR4 NE (MC)
//=============================================================================
/*
    ime_3dnr_mc_sad_base0:    [0x0, 0x3fff],            bits : 13_0
    ime_3dnr_mc_sad_base1:    [0x0, 0x3fff],            bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER0_OFS 0x07a0
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_mc_sad_base0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_base1,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER0)

/*
    ime_3dnr_mc_sad_coef_a0:    [0x0, 0x3f],            bits : 5_0
    ime_3dnr_mc_sad_coef_a1:    [0x0, 0x3f],            bits : 13_8
    ime_3dnr_mc_sad_coef_a2:    [0x0, 0x3f],            bits : 21_16
    ime_3dnr_mc_sad_coef_a3:    [0x0, 0x3f],            bits : 29_24
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER4_OFS 0x07b0
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER4)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a0,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a2,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_a3,        6)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER4)

/*
    ime_3dnr_mc_sad_coef_b0:    [0x0, 0x3fff],          bits : 13_0
    ime_3dnr_mc_sad_coef_b1:    [0x0, 0x3fff],          bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER6_OFS 0x07b8
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER6)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_coef_b1,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER6)

/*
    ime_3dnr_mc_sad_std0:    [0x0, 0x3fff],         bits : 13_0
    ime_3dnr_mc_sad_std1:    [0x0, 0x3fff],         bits : 29_16
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER10_OFS 0x07c8
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER10)
REGDEF_BIT(ime_3dnr_mc_sad_std0,        14)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_sad_std1,        14)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER10)

/*
    ime_3dnr_mc_k1:    [0x0, 0x3f],         bits : 5_0
    ime_3dnr_mc_k2:    [0x0, 0x3f],         bits : 13_8
*/
#define IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER14_OFS 0x07d8
REGDEF_BEGIN(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER14)
REGDEF_BIT(ime_3dnr_mc_k1,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_mc_k2,        6)
REGDEF_END(IME_3DNR_MOTION_COMPENSATION_CONTROL_REGISTER14)

//=============================================================================
// IME TMNR4 PS
//=============================================================================
/*
    ime_3dnr_ps_mv_th    :    [0x0, 0x3f],          bits : 5_0
    ime_3dnr_ps_roi_mv_th:    [0x0, 0x3f],          bits : 13_8
    ime_3dnr_ps_fs_th    :    [0x0, 0x3fff],            bits : 29_16
*/
#define IME_3DNR_PS_CONTROL_REGISTER0_OFS 0x0820
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_ps_mv_th,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ps_roi_mv_th,        6)
REGDEF_BIT(,        2)
REGDEF_BIT(ime_3dnr_ps_fs_th,        14)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER0)

/*
    ime_3dnr_ps_mix_ratio0:    [0x0, 0xff],         bits : 7_0
    ime_3dnr_ps_mix_ratio1:    [0x0, 0xff],         bits : 15_8
*/
#define IME_3DNR_PS_CONTROL_REGISTER1_OFS 0x0824
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_ps_mix_ratio0,        8)
REGDEF_BIT(ime_3dnr_ps_mix_ratio1,        8)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER1)

/*
    ime_3dnr_ps_ds_th    :    [0x0, 0x1f],          bits : 4_0
    ime_3dnr_ps_ds_th_roi:    [0x0, 0x1f],          bits : 12_8
*/
#define IME_3DNR_PS_CONTROL_REGISTER4_OFS 0x0850
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER4)
REGDEF_BIT(ime_3dnr_ps_ds_th,        5)
REGDEF_BIT(,        3)
REGDEF_BIT(ime_3dnr_ps_ds_th_roi,        5)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER4)

//=============================================================================
// IME TMNR4 Filter TF0
//=============================================================================
/*
    ime_3dnr_nr_y_tf0_blur_str0:    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_y_tf0_blur_str1:    [0x0, 0xff],            bits : 15_8
    ime_3dnr_nr_y_tf0_blur_str2:    [0x0, 0xff],            bits : 23_16
    ime_3dnr_nr_y_tf0_str0     :    [0x0, 0xff],            bits : 31_24
*/
#define IME_3DNR_TF0_REGISTER0_OFS 0x0804
REGDEF_BEGIN(IME_3DNR_TF0_REGISTER0)
REGDEF_BIT(ime_3dnr_nr_y_tf0_blur_str0,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_blur_str1,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_blur_str2,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_str0,        8)
REGDEF_END(IME_3DNR_TF0_REGISTER0)

/*
    ime_3dnr_nr_y_tf0_str1:    [0x0, 0xff],         bits : 7_0
    ime_3dnr_nr_y_tf0_str2:    [0x0, 0xff],         bits : 15_8
    ime_3dnr_nr_c_tf0_str0:    [0x0, 0xff],         bits : 23_16
    ime_3dnr_nr_c_tf0_str1:    [0x0, 0xff],         bits : 31_24
*/
#define IME_3DNR_TF0_REGISTER1_OFS 0x0808
REGDEF_BEGIN(IME_3DNR_TF0_REGISTER1)
REGDEF_BIT(ime_3dnr_nr_y_tf0_str1,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_str2,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_str0,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_str1,        8)
REGDEF_END(IME_3DNR_TF0_REGISTER1)

/*
    ime_3dnr_nr_c_tf0_str2     :    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_y_tf0_blur_estr:    [0x0, 0xff],            bits : 15_8
*/
#define IME_3DNR_TF0_REGISTER2_OFS 0x080c
REGDEF_BEGIN(IME_3DNR_TF0_REGISTER2)
REGDEF_BIT(ime_3dnr_nr_c_tf0_str2,        8)
REGDEF_BIT(ime_3dnr_nr_y_tf0_blur_estr,        8)
REGDEF_END(IME_3DNR_TF0_REGISTER2)

/*
    ime_3dnr_nr_u_tf0_md_th  :    [0x0, 0xff],          bits : 7_0
    ime_3dnr_nr_v_tf0_md_th  :    [0x0, 0xff],          bits : 15_8
    ime_3dnr_nr_c_tf0_ratio_0:    [0x0, 0xff],          bits : 23_16
    ime_3dnr_nr_c_tf0_ratio_1:    [0x0, 0xff],          bits : 31_24
*/
#define IME_3DNR_PS_CONTROL_REGISTER2_OFS 0x0828
REGDEF_BEGIN(IME_3DNR_PS_CONTROL_REGISTER2)
REGDEF_BIT(ime_3dnr_nr_u_tf0_md_th,        8)
REGDEF_BIT(ime_3dnr_nr_v_tf0_md_th,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_ratio_0,        8)
REGDEF_BIT(ime_3dnr_nr_c_tf0_ratio_1,        8)
REGDEF_END(IME_3DNR_PS_CONTROL_REGISTER2)

//=============================================================================
// IME TMNR4 Filter ResidueTH
//=============================================================================
/*
    ime_3dnr_nr_residue_th_y0:    [0x0, 0xf],           bits : 3_0
    ime_3dnr_nr_residue_th_y1:    [0x0, 0xf],           bits : 11_8
    ime_3dnr_nr_residue_th_y2:    [0x0, 0xf],           bits : 19_16
    ime_3dnr_nr_residue_th_c :    [0x0, 0xf],           bits : 27_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER0_OFS 0x085c
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER0)
REGDEF_BIT(ime_3dnr_nr_residue_th_y0,        4)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_3dnr_nr_residue_th_y1,        4)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_3dnr_nr_residue_th_y2,        4)
REGDEF_BIT(,        4)
REGDEF_BIT(ime_3dnr_nr_residue_th_c,        4)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER0)

//=============================================================================
// IME TMNR4 Filter PreFilter & Filter
//=============================================================================
/*
    ime_3dnr_nr_freq_w0:    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_freq_w1:    [0x0, 0xff],            bits : 15_8
    ime_3dnr_nr_freq_w2:    [0x0, 0xff],            bits : 23_16
    ime_3dnr_nr_freq_w3:    [0x0, 0xff],            bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER1_OFS 0x0860
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER1)
REGDEF_BIT(ime_3dnr_nr_freq_w0,        8)
REGDEF_BIT(ime_3dnr_nr_freq_w1,        8)
REGDEF_BIT(ime_3dnr_nr_freq_w2,        8)
REGDEF_BIT(ime_3dnr_nr_freq_w3,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER1)

/*
    ime_3dnr_nr_luma_w0:    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_luma_w1:    [0x0, 0xff],            bits : 15_8
    ime_3dnr_nr_luma_w2:    [0x0, 0xff],            bits : 23_16
    ime_3dnr_nr_luma_w3:    [0x0, 0xff],            bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER3_OFS 0x0880
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER3)
REGDEF_BIT(ime_3dnr_nr_luma_w0,        8)
REGDEF_BIT(ime_3dnr_nr_luma_w1,        8)
REGDEF_BIT(ime_3dnr_nr_luma_w2,        8)
REGDEF_BIT(ime_3dnr_nr_luma_w3,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER3)

/*
    ime_3dnr_nr_pre_filtering_str0:    [0x0, 0xff],         bits : 7_0
    ime_3dnr_nr_pre_filtering_str1:    [0x0, 0xff],         bits : 15_8
    ime_3dnr_nr_pre_filtering_str2:    [0x0, 0xff],         bits : 23_16
    ime_3dnr_nr_pre_filtering_str3:    [0x0, 0xff],         bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER6_OFS 0x08a0
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER6)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_str0,        8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_str1,        8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_str2,        8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_str3,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER6)

/*
    ime_3dnr_nr_pre_filtering_ratio0:    [0x0, 0xff],           bits : 7_0
    ime_3dnr_nr_pre_filtering_ratio1:    [0x0, 0xff],           bits : 15_8
    ime_3dnr_nr_snr_str0            :    [0x0, 0xff],           bits : 23_16
    ime_3dnr_nr_snr_str1            :    [0x0, 0xff],           bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER8_OFS 0x08b0
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_ratio0,        8)
REGDEF_BIT(ime_3dnr_nr_pre_filtering_ratio1,        8)
REGDEF_BIT(ime_3dnr_nr_snr_str0,        8)
REGDEF_BIT(ime_3dnr_nr_snr_str1,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER8)

/*
    ime_3dnr_nr_snr_str2:    [0x0, 0xff],           bits : 7_0
    ime_3dnr_nr_tnr_str0:    [0x0, 0xff],           bits : 15_8
    ime_3dnr_nr_tnr_str1:    [0x0, 0xff],           bits : 23_16
    ime_3dnr_nr_tnr_str2:    [0x0, 0xff],           bits : 31_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER9_OFS 0x08b4
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER9)
REGDEF_BIT(ime_3dnr_nr_snr_str2,        8)
REGDEF_BIT(ime_3dnr_nr_tnr_str0,        8)
REGDEF_BIT(ime_3dnr_nr_tnr_str1,        8)
REGDEF_BIT(ime_3dnr_nr_tnr_str2,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER9)

/*
    ime_3dnr_nr_base_th_snr:    [0x0, 0xffff],          bits : 15_0
    ime_3dnr_nr_base_th_tnr:    [0x0, 0xffff],          bits : 31_16
*/
#define IME_3DNR_NR_CONTROL_REGISTER10_OFS 0x08b8
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER10)
REGDEF_BIT(ime_3dnr_nr_base_th_snr,        16)
REGDEF_BIT(ime_3dnr_nr_base_th_tnr,        16)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER10)

//=============================================================================
// IME TMNR4 Filter PostFilter
//=============================================================================
/*
    ime_3dnr_nr_y_3d_ratio0  :    [0x0, 0xff],          bits : 7_0
    ime_3dnr_nr_y_3d_ratio1  :    [0x0, 0xff],          bits : 15_8
    ime_3dnr_nr_luma_comp_str:    [0x0, 0xff],          bits : 23_16
*/
#define IME_3DNR_NR_CONTROL_REGISTER11_OFS 0x08bc
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER11)
REGDEF_BIT(ime_3dnr_nr_y_3d_ratio0,        8)
REGDEF_BIT(ime_3dnr_nr_y_3d_ratio1,        8)
REGDEF_BIT(ime_3dnr_nr_luma_comp_str,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER11)

/*
    ime_3dnr_nr_y_3d_lut0:    [0x0, 0x7f],          bits : 6_0
    ime_3dnr_nr_y_3d_lut1:    [0x0, 0x7f],          bits : 14_8
    ime_3dnr_nr_y_3d_lut2:    [0x0, 0x7f],          bits : 22_16
    ime_3dnr_nr_y_3d_lut3:    [0x0, 0x7f],          bits : 30_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER12_OFS 0x08c0
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER12)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut0,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut1,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut2,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_y_3d_lut3,        7)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER12)

/*
    ime_3dnr_nr_c_3d_lut0:    [0x0, 0x7f],          bits : 6_0
    ime_3dnr_nr_c_3d_lut1:    [0x0, 0x7f],          bits : 14_8
    ime_3dnr_nr_c_3d_lut2:    [0x0, 0x7f],          bits : 22_16
    ime_3dnr_nr_c_3d_lut3:    [0x0, 0x7f],          bits : 30_24
*/
#define IME_3DNR_NR_CONTROL_REGISTER14_OFS 0x08c8
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER14)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut0,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut1,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut2,        7)
REGDEF_BIT(,        1)
REGDEF_BIT(ime_3dnr_nr_c_3d_lut3,        7)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER14)

/*
    ime_3dnr_nr_c_3d_ratio0:    [0x0, 0xff],            bits : 7_0
    ime_3dnr_nr_c_3d_ratio1:    [0x0, 0xff],            bits : 15_8
*/
#define IME_3DNR_NR_CONTROL_REGISTER16_OFS 0x08d0
REGDEF_BEGIN(IME_3DNR_NR_CONTROL_REGISTER16)
REGDEF_BIT(ime_3dnr_nr_c_3d_ratio0,        8)
REGDEF_BIT(ime_3dnr_nr_c_3d_ratio1,        8)
REGDEF_END(IME_3DNR_NR_CONTROL_REGISTER16)

//=============================================================================
// IME SHARPEN
//=============================================================================
/*
    shp_edge_weight_src_sel:    [0x0, 0x1],         bits : 0
    shp_motion_bit_en      :    [0x0, 0x1],         bits : 1
    shp_show_info          :    [0x0, 0x3],         bits : 3_2
    shp_edge_weight_th     :    [0x0, 0xff],            bits : 15_8
    shp_edge_weight_gain   :    [0x0, 0xff],            bits : 23_16
    shp_noise_level        :    [0x0, 0xff],            bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER0_OFS 0x0590
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER0)
REGDEF_BIT(shp_edge_weight_src_sel,        1)
REGDEF_BIT(shp_motion_bit_en,        1)
REGDEF_BIT(shp_show_info,        2)
REGDEF_BIT(shp_jnd_filter_size,        2)
REGDEF_BIT(,        2)
REGDEF_BIT(shp_edge_weight_th,        8)
REGDEF_BIT(shp_edge_weight_gain,        8)
REGDEF_BIT(shp_noise_level,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER0)

/*
    shp_blend_inv_gamma:    [0x0, 0xff],            bits : 7_0
    shp_edge_str       :    [0x0, 0xff],            bits : 15_8
    shp_coring_th      :    [0x0, 0xff],            bits : 23_16
    shp_w_con_eng      :    [0x0, 0xf],         bits : 27_24
*/
#define IME_POST_SHARPEN_REGISTER1_OFS 0x0594
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER1)
REGDEF_BIT(shp_blend_inv_gamma,        8)
REGDEF_BIT(shp_edge_str,        8)
REGDEF_BIT(shp_coring_th,        8)
REGDEF_BIT(shp_w_con_eng,        4)
REGDEF_END(IME_POST_SHARPEN_REGISTER1)

/*
    shp_bright_halo_clip:    [0x0, 0xff],           bits : 7_0
    shp_dark_halo_clip  :    [0x0, 0xff],           bits : 15_8
*/
#define IME_POST_SHARPEN_REGISTER2_OFS 0x0598
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER2)
REGDEF_BIT(shp_bright_halo_clip,        8)
REGDEF_BIT(shp_dark_halo_clip,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER2)

/*
    shp_flat_th:    [0x0, 0x7ff],           bits : 10_0
    shp_edge_th:    [0x0, 0x7ff],           bits : 26_16
*/
#define IME_POST_SHARPEN_REGISTER3_OFS 0x059c
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER3)
REGDEF_BIT(shp_flat_th,        11)
REGDEF_BIT(,        5)
REGDEF_BIT(shp_edge_th,        11)
REGDEF_END(IME_POST_SHARPEN_REGISTER3)

/*
    shp_slope_con_eng  :    [0x0, 0xfff],           bits : 11_0
    shp_flat_region_str:    [0x0, 0xff],            bits : 23_16
    shp_edge_region_str:    [0x0, 0xff],            bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER4_OFS 0x05a0
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER4)
REGDEF_BIT(shp_slope_con_eng,        12)
REGDEF_BIT(,        4)
REGDEF_BIT(shp_flat_region_str,        8)
REGDEF_BIT(shp_edge_region_str,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER4)

/*
    shp_motion_edge_weight_str:    [0x0, 0xff],         bits : 7_0
    shp_static_edge_weight_str:    [0x0, 0xff],         bits : 15_8
    shp_trans_edge_weight_str :    [0x0, 0xff],         bits : 23_16
*/
#define IME_POST_SHARPEN_REGISTER5_OFS 0x05a4
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER5)
REGDEF_BIT(shp_motion_edge_weight_str,        8)
REGDEF_BIT(shp_static_edge_weight_str,        8)
REGDEF_BIT(shp_trans_edge_weight_str,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER5)

/*
    shp_noise_curve0:    [0x0, 0xff],           bits : 7_0
    shp_noise_curve1:    [0x0, 0xff],           bits : 15_8
    shp_noise_curve2:    [0x0, 0xff],           bits : 23_16
    shp_noise_curve3:    [0x0, 0xff],           bits : 31_24
*/
#define IME_POST_SHARPEN_REGISTER6_OFS 0x05a8
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER6)
REGDEF_BIT(shp_noise_curve0,        8)
REGDEF_BIT(shp_noise_curve1,        8)
REGDEF_BIT(shp_noise_curve2,        8)
REGDEF_BIT(shp_noise_curve3,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER6)

/*
    shp_noise_curve16:    [0x0, 0xff],          bits : 7_0
*/
#define IME_POST_SHARPEN_REGISTER10_OFS 0x05b8
REGDEF_BEGIN(IME_POST_SHARPEN_REGISTER10)
REGDEF_BIT(shp_noise_curve16,        8)
REGDEF_END(IME_POST_SHARPEN_REGISTER10)

//post sharpen
typedef struct {
	BOOL  bEnable;          ///< [r/w] Enable sharpen
	UINT8 ucConEng;         ///< [r/w] The weight of the local contrast  0~8
	UINT16 usSlopConEng;    ///< [r/w] Transition region slop 0~4095
	UINT8 ucBHC;            ///< [r/w] Bright halo clip ratio 0~128
	UINT8 ucDHC;            ///< [r/w] Dark halo clip ratio 0~128
	UINT8 ucEWT;            ///< [r/w] Edge weight coring threshold 0~255
	UINT8 ucEWG;            ///< [r/w] Edge weight gain 0~255
	UINT8 ucEdgeSharpStr1;  ///< [r/w] Sharpen strength 1 of edge region 0~255
	UINT8 ucCT;             ///< [r/w] Coring threshold 0~255
	UINT8 ucNL;             ///< [r/w] Noise level 0~255
	UINT8 ucBIG;            ///< [r/w] Blending ratio of HPF results 0~255
	UINT16 usFlatTh;        ///< [r/w] flat region threshold 0~2047
	UINT16 usEdgeTh;        ///< [r/w] Edge region threshold 0~2047
	UINT8 ucEdgeStr;        ///< [r/w] Edge weight strength 0~255
	UINT8 ucTransitionStr;  ///< [r/w] Transition region weight strength 0~255
	UINT8 ucMotionStr;      ///< [r/w] Motion region edge weight 0~255
	UINT8 ucStaticStr;      ///< [r/w] Static region edge weight strength 0~255
	UINT8 ucFlatStr;        ///< [r/w] Flag region weight strength 0~255
	UINT8 ucNC[17];         ///< [r/w] 17 control points of noise modulation curve 0~255
	UINT16 usEWG[9];        ///< [r/w] 9 control points of EWG
	BOOL bShowSharpInfo;    ///< [r/w] Show debug mode info(0: normal, 1: regionclass)
} KDRV_H26XENC_SPN;


#endif

