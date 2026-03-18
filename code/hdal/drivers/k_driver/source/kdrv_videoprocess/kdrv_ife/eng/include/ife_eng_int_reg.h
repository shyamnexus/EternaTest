#ifndef _IFE_INT_REGISTER_H_
#define _IFE_INT_REGISTER_H_

#ifdef __cplusplus
extern "C" {
#endif


#if defined (__LINUX)

//#include "mach/rcw_macro.h"  //tmp change
#include "linux/soc/nvt/rcw_macro.h"
//#include "rcw_macro.h"


#include "plat/top.h"
#include "kwrap/type.h"
#include "kwrap/nvt_type.h"

//=========================================================================
#elif defined (__FREERTOS)


#include "rcw_macro.h"
#include "plat/top.h"
#include "kwrap/type.h"
#include "kwrap/nvt_type.h"

#else


#endif



typedef struct {

	union {
		struct {
			unsigned ife_sw_rst             :  1;            //bits : 0
			unsigned ife_start              :  1;            //bits : 1
			unsigned ife_load_start         :  1;            //bits : 2
			unsigned ife_load_fd            :  1;            //bits : 3
			unsigned ife_load_frmstart      :  1;            //bits : 4
			unsigned ife_global_load_en     :  1;            //bits : 5
			unsigned                        : 22;
			unsigned ife_ll_fire            :  1;            //bits : 28
		} bit;
		UINT32 word;
	} ife_register_0; // 0x00;

	union {
		struct {
			unsigned ife_mode               :  2;            //bits : 1..0
			unsigned input_format           :  1;
			unsigned output_sel             :  1;
			unsigned inbit_16_fmt_sel       :  1;            //bits : 4
			unsigned inbit_depth            :  2;            //bits : 6..5
			unsigned outbit_depth           :  2;            //bits : 8..7
			unsigned cfapat                 :  3;            //bits : 11..9
			unsigned                        :  1;            //bits : 12
			unsigned outl_en            :  1;            //bits : 13
			unsigned filter_en          :  1;            //bits : 14
			unsigned cgain_en           :  1;            //bits : 15
			unsigned vig_en             :  1;            //bits : 16
			unsigned gbal_en            :  1;            //bits : 17
			unsigned ife_binning            :  3;            //bits : 20..18
			unsigned bayer_format           :  1;            //bits : 21
			unsigned rgbir_rb_nrfill    :  1;            //bits : 22
			unsigned bilat_th_en        :  1;            //bits : 23
			unsigned dgain_en           :  1;            //bits : 24
			unsigned f_cg_en            :  1;            //bits : 25
			unsigned f_fusion_en        :  1;            //bits : 26
			unsigned f_fusion_fnum      :  2;            //bits : 28..27
			unsigned f_fc_en            :  1;            //bits : 29
			unsigned mirror_en          :  1;            //bits : 30
            unsigned thermal_mode     : 1;		// bits : 31
		} bit;
		UINT32 word;
	} ife_register_1; // 0x04;

	union {
		struct {
			unsigned inte_frmend            : 1;        // bits : 0
			unsigned inte_ife_r_dec1_err    : 1;        // bits : 1
			unsigned inte_ife_r_dec2_err    : 1;        // bits : 2
			unsigned inte_llend             : 1;        // bits : 3
			unsigned inte_llerror           : 1;        // bits : 4
			unsigned inte_llerror2          : 1;        // bits : 5
			unsigned inte_lljobend          : 1;        // bits : 6
			unsigned inte_ife_bufovfl       : 1;        // bits : 7
			unsigned inte_ife_ringbuf_err   : 1;        // bits : 8
			unsigned inte_ife_frame_error   : 1;        // bits : 9
			unsigned                        : 1;        // bits : 10
			unsigned                        : 1;
			unsigned inte_sie_frame_start   : 1;        // bits : 12
			unsigned inte_sie2_frame_start  : 1;        // bits : 13
			unsigned inte_ife_frame_start   : 1;        // bits : 14
			unsigned                        : 1;
			unsigned inte_ife_nn_isp_p0_set_slice_ready   : 1;        // bits : 16
			unsigned inte_ife_nn_isp_p0_get_slice_clear   : 1;        // bits : 17
			unsigned inte_ife_nn_isp_p1_set_slice_ready   : 1;        // bits : 18
			unsigned inte_ife_nn_isp_p1_get_slice_clear   : 1;        // bits : 19
		} bit;
		UINT32 word;
	} ife_register_2; // 0x08;

	union {
		struct {
			unsigned int_frmend             : 1;        // bits : 0
			unsigned int_ife_r_dec1_err     : 1;        // bits : 1
			unsigned int_ife_r_dec2_err     : 1;        // bits : 2
			unsigned int_llend              : 1;        // bits : 3
			unsigned int_llerror            : 1;        // bits : 4
			unsigned int_llerror2           : 1;        // bits : 5
			unsigned int_lljobend           : 1;        // bits : 6
			unsigned int_ife_bufovfl        : 1;        // bits : 7
			unsigned int_ife_ringbuf_err    : 1;        // bits : 8
			unsigned int_ife_frame_error    : 1;        // bits : 9
			unsigned                        : 1;        // bits : 10
			unsigned                        : 1;
			unsigned int_sie_frame_start    : 1;        // bits : 12
			unsigned int_sie2_frame_start   : 1;        // bits : 13
			unsigned int_ife_frame_start    : 1;        // bits : 14
			unsigned                        : 1;
			unsigned int_ife_nn_isp_p0_set_slice_ready   : 1;        // bits : 16
			unsigned int_ife_nn_isp_p0_get_slice_clear   : 1;        // bits : 17
			unsigned int_ife_nn_isp_p1_set_slice_ready   : 1;        // bits : 18
			unsigned int_ife_nn_isp_p1_get_slice_clear   : 1;        // bits : 19
		} bit;
		UINT32 word;
	} ife_register_3; // 0x0c;

	union {
		struct {
			unsigned ife_busy               :  1;       //bits : 0
			unsigned                        : 19;
			unsigned ife_frmstr_rst         :  1;       // bits : 20
			unsigned ife_checksum_en        :  1;       // bits : 21
		} bit;
		UINT32 word;
	} ife_register_4; // 0x10;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_saill        : 30;        // bits : 31_2
		} bit;
		UINT32 word;
	} ife_register_5; // 0x14;

	union {
		struct {
			unsigned cfapat_2               : 3;
			unsigned                        : 21;
			unsigned dram_saill_msb        : 4;        // bits : 31_2
		} bit;
		UINT32 word;
	} ife_register_6; // 0x18;

	union {
		struct {
			unsigned ife_dmach_idle                       : 1;             //bits : 0
			unsigned ife_dmach_dis                        : 1;             //bits : 1
			unsigned ife_dma_output_en                    : 1;
			unsigned                                      : 5;
			unsigned ife_dich_line_buf_ctrl               : 2;             //bits : 9..8
			unsigned                                      : 6;
			unsigned ife_dma_sync_dich_line_buf_disable   : 1;             //bits : 16
			unsigned ife_dma1_wait_sie2_start_disable     : 1;             //bits : 17
		} bit;
		UINT32 word;
	} ife_register_7; // 0x1c;

	union {
		struct {
			unsigned                        :  2;
			unsigned width                  : 14;            //bits : 15..2
			unsigned                        :  1;
			unsigned height                 : 15;            //bits : 31..17
		} bit;
		UINT32 word;
	} ife_register_8; // 0x20;

	union {
		struct {
			unsigned                        :  2;
			unsigned crop_width             : 14;            //bits : 15..2
			unsigned                        :  1;
			unsigned crop_height            : 15;            //bits : 31..17
		} bit;
		UINT32 word;
	} ife_register_9; // 0x24;

	union {
		struct {
			unsigned crop_hpos              : 16;            //bits : 15..0
			unsigned crop_vpos              : 16;            //bits : 31..16
		} bit;
		UINT32 word;
	} ife_register_10; // 0x28;

	union {
		struct {
			unsigned ife_ll_terminate        : 1;       // bits : 0
		} bit;
		UINT32 word;
	} ife_register_11; // 0x2c;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_sai0              : 30;            //bits : 31..2
		} bit;
		UINT32 word;
	} ife_register_12; // 0x30;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_ofsi0             : 14;            //bits : 15..2
			unsigned                        :  8;
			unsigned dram_sai0_msb          :  4;
		} bit;
		UINT32 word;
	} ife_register_13; // 0x34;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_sai1              : 30;            //bits : 31..2
		} bit;
		UINT32 word;
	} ife_register_14; // 0x38;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_ofsi1             : 14;            //bits : 15..2
			unsigned                        :  8;
			unsigned dram_sai1_msb         :  4;
		} bit;
		UINT32 word;
	} ife_register_15; // 0x3c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_16; // 0x40;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_17; // 0x44;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_sao               : 30;            //bits : 31..2
		} bit;
		UINT32 word;
	} ife_register_18; // 0x48;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_ofso              : 14;            //bits : 15..2
			unsigned                        :  8;
			unsigned dram_sao_msb          :  4;
		} bit;
		UINT32 word;
	} ife_register_19; // 0x4c;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_sao_uv            : 30;            //bits : 31..2
		} bit;
		UINT32 word;
	} ife_register_20; // 0x50;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_ofso_uv           : 14;            //bits : 15..2
			unsigned                        :  8;
			unsigned dram_sao_uv_msb       :  4;
		} bit;
		UINT32 word;
	} ife_register_21; // 0x54;

	union {
		struct {
			unsigned ife_yuv_in_fmt         : 2;
			unsigned                        : 2;
			unsigned ife_yuv2rgb_en         : 1;            //bits : 4
			unsigned ife_yuv2rgb_fmt        : 2;
		} bit;
		UINT32 word;
	} ife_register_22; // 0x58;

	union {
		struct {
			unsigned input_burst_mode       :  1;            //bits : 0
			unsigned                        :  3;
			unsigned output_burst_mode      :  1;            //bits : 4
			unsigned                        :  7;
			unsigned dmaloop_line           : 11;            //bits : 22..12
			unsigned                        :  1;
			unsigned dmaloop_en             :  1;            //bits : 24
			unsigned dmaloop_ctrl           :  1;            //bits : 25
			unsigned                        :  2;
			unsigned sie2_line_count_align_to_height :  1;   //bits : 28
		} bit;
		UINT32 word;
	} ife_register_23; // 0x5c;

	union {
		struct {
			unsigned ife_out0_single_en       : 1;         // bits : 0
			unsigned ife_out1_single_en       : 1;         // bits : 1
			unsigned ife_out2_single_en       : 1;         // bits : 2
			unsigned ife_hist_out_single_en   : 1;
			unsigned ife_indep_va_single_en   : 1;         // bits : 4
			unsigned                          : 26;
			unsigned ife_dram_out_mode        : 1;         //bits : 31
		} bit;
		UINT32 word;
	} ife_register_24; // 0x60;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_25; // 0x64;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_26; // 0x68;

	union {
		struct {
			unsigned ife_dgain             :  16;            //bits : 0
		} bit;
		UINT32 word;
	} ife_register_27; // 0x6c;

	union {
		struct {
			unsigned cgain_inv              :  1;            //bits : 0
			unsigned cgain_hinv             :  1;            //bits : 1
			unsigned cgain_range            :  1;            //bits : 2
			unsigned ife_f_cgain_range      :  1;            //bits : 3
			unsigned                        :  4;
			unsigned cgain_mask             : 12;            //bits : 19..8
		} bit;
		UINT32 word;
	} ife_register_28; // 0x70;

	union {
		struct {
			unsigned ife_cgain_r            : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_cgain_gr           : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_29; // 0x74;

	union {
		struct {
			unsigned ife_cgain_gb           : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_cgain_b            : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_30; // 0x78;

	union {
		struct {
			unsigned ife_cgain_ir           : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_31; // 0x7c;

	union {
		struct {
			unsigned ife_f_p0_cgain_r       : 16;            //bits : 9..0
			unsigned ife_f_p0_cgain_gr      : 16;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_32; // 0x80;

	union {
		struct {
			unsigned ife_f_p0_cgain_gb      : 16;            //bits : 9..0
			unsigned ife_f_p0_cgain_b       : 16;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_33; // 0x84;

	union {
		struct {
			unsigned ife_f_p0_cgain_ir      : 16;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_34; // 0x88;

	union {
		struct {
			unsigned ife_f_p1_cgain_r       : 16;            //bits : 9..0
			unsigned ife_f_p1_cgain_gr      : 16;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_35; // 0x8c;

	union {
		struct {
			unsigned ife_f_p1_cgain_gb      : 16;            //bits : 9..0
			unsigned ife_f_p1_cgain_b       : 16;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_36; // 0x90;

	union {
		struct {
			unsigned ife_f_p1_cgain_ir      : 16;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_37; // 0x94;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_38; // 0x98;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_39; // 0x9c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_40; // 0xa0;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_41; // 0xa4;

	union {
		struct {
			unsigned ife_dma1_line_count    : 16;
			unsigned sie2_line_count        : 16;
		} bit;
		UINT32 word;
	} ife_register_42; // 0xa8;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_43; // 0xac;

	union {
		struct {
			unsigned ife_cofs_r             : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_cofs_gr            : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_44; // 0xb0;

	union {
		struct {
			unsigned ife_cofs_gb            : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_cofs_b             : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_45; // 0xb4;

	union {
		struct {
			unsigned ife_cofs_ir            : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_46; // 0xb8;

	union {
		struct {
			unsigned ife_f_p0_cofs_r        : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_f_p0_cofs_gr       : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_47; // 0xbc;

	union {
		struct {
			unsigned ife_f_p0_cofs_gb       : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_f_p0_cofs_b        : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_48; // 0xc0;

	union {
		struct {
			unsigned ife_f_p0_cofs_ir       : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_49; // 0xc4;

	union {
		struct {
			unsigned ife_f_p1_cofs_r        : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_f_p1_cofs_gr       : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_50; // 0xc8;

	union {
		struct {
			unsigned ife_f_p1_cofs_gb       : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_f_p1_cofs_b        : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_51; // 0xcc;

	union {
		struct {
			unsigned ife_f_p1_cofs_ir       : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_52; // 0xd0;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_53; // 0xd4;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_54; // 0xd8;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_55; // 0xdc;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_56; // 0xe0;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_57; // 0xe4;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_58; // 0xe8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_59; // 0xec;

	union {
		struct {
			unsigned ife_outl_rgbir_rb_w        : 8;        // bits : 7_0
			unsigned ife_ord_rgbir_rb_w         : 8;        // bits : 15_8
		} bit;
		UINT32 word;
	} ife_register_60; // 0xf0;

	union {
		struct {
			unsigned ife_gbal_str_luma_low_bnd         : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_gbal_edge_luma_low_bnd        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_61; // 0xf4;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_62; // 0xf8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_63; // 0xfc;

	union {
		struct {
			unsigned ife_clamp_th           : 12;            //bits : 11..0
			unsigned ife_clamp_mul          :  8;            //bits : 19..12
			unsigned ife_clamp_dlt          : 12;            //bits : 31..20
		} bit;
		UINT32 word;
	} ife_register_64; // 0x100;

	union {
		struct {
			unsigned ife_bilat_w            :  4;            //bits : 3..0
			unsigned ife_rth_w              :  4;            //bits : 7..4
			unsigned ife_bilat_th1          : 10;            //bits : 17..8
			unsigned ife_bilat_th2          : 10;            //bits : 27..18
			unsigned ife_bilat_cen_sel      :  1;            //bits : 28
		} bit;
		UINT32 word;
	} ife_register_65; // 0x104;

	union {
		struct {
			unsigned ife_s_weight0          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_s_weight1          :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_s_weight2          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_s_weight3          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_66; // 0x108;

	union {
		struct {
			unsigned ife_s_weight4          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_s_weight5          :  5;            //bits : 12..8
		} bit;
		UINT32 word;
	} ife_register_67; // 0x10c;

	union {
		struct {
			unsigned ife_rth_a_c0_0         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c0_1         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_68; // 0x110;

	union {
		struct {
			unsigned ife_rth_a_c0_2         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c0_3         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_69; // 0x114;

	union {
		struct {
			unsigned ife_rth_a_c0_4         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c0_5         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_70; // 0x118;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_71; // 0x11c;

	union {
		struct {
			unsigned ife_rth_a_c1_0         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c1_1         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_72; // 0x120;

	union {
		struct {
			unsigned ife_rth_a_c1_2         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c1_3         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_73; // 0x124;

	union {
		struct {
			unsigned ife_rth_a_c1_4         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c1_5         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_74; // 0x128;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_75; // 0x12c;

	union {
		struct {
			unsigned ife_rth_a_c2_0         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c2_1         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_76; // 0x130;

	union {
		struct {
			unsigned ife_rth_a_c2_2         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c2_3         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_77; // 0x134;

	union {
		struct {
			unsigned ife_rth_a_c2_4         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c2_5         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_78; // 0x138;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_79; // 0x13c;

	union {
		struct {
			unsigned ife_rth_a_c3_0         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c3_1         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_80; // 0x140;

	union {
		struct {
			unsigned ife_rth_a_c3_2         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c3_3         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_81; // 0x144;

	union {
		struct {
			unsigned ife_rth_a_c3_4         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_a_c3_5         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_82; // 0x148;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_83; // 0x14c;

	union {
		struct {
			unsigned ife_rth_b_c0_0         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c0_1         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_84; // 0x150;

	union {
		struct {
			unsigned ife_rth_b_c0_2         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c0_3         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_85; // 0x154;

	union {
		struct {
			unsigned ife_rth_b_c0_4         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c0_5         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_86; // 0x158;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_87; // 0x15c;

	union {
		struct {
			unsigned ife_rth_b_c1_0         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c1_1         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_88; // 0x160;

	union {
		struct {
			unsigned ife_rth_b_c1_2         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c1_3         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_89; // 0x164;

	union {
		struct {
			unsigned ife_rth_b_c1_4         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c1_5         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_90; // 0x168;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_91; // 0x16c;

	union {
		struct {
			unsigned ife_rth_b_c2_0         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c2_1         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_92; // 0x170;

	union {
		struct {
			unsigned ife_rth_b_c2_2         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c2_3         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_93; // 0x174;

	union {
		struct {
			unsigned ife_rth_b_c2_4         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c2_5         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_94; // 0x178;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_95; // 0x17c;

	union {
		struct {
			unsigned ife_rth_b_c3_0         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c3_1         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_96; // 0x180;

	union {
		struct {
			unsigned ife_rth_b_c3_2         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c3_3         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_97; // 0x184;

	union {
		struct {
			unsigned ife_rth_b_c3_4         : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned ife_rth_b_c3_5         : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_98; // 0x188;

	union {
		struct {
			unsigned ife_outl_bright_ofs    : 12;            //bits : 11..0
			unsigned ife_outl_dark_ofs      : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} ife_register_99; // 0x18c;

	union {
		struct {
			unsigned ife_outlth_bri0        : 12;            //bits : 11..0
			unsigned ife_outlth_dark0       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} ife_register_100; // 0x190;

	union {
		struct {
			unsigned ife_outlth_bri1        : 12;            //bits : 11..0
			unsigned ife_outlth_dark1       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} ife_register_101; // 0x194;

	union {
		struct {
			unsigned ife_outlth_bri2        : 12;            //bits : 11..0
			unsigned ife_outlth_dark2       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} ife_register_102; // 0x198;

	union {
		struct {
			unsigned ife_outlth_bri3        : 12;            //bits : 11..0
			unsigned ife_outlth_dark3       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} ife_register_103; // 0x19c;

	union {
		struct {
			unsigned ife_outlth_bri4        : 12;            //bits : 11..0
			unsigned ife_outlth_dark4       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} ife_register_104; // 0x1a0;

	union {
		struct {
			unsigned ife_outl_compensate_mode :  1;            //bits : 0
			unsigned                          :  7;
			unsigned ife_outl_weight          :  8;
			unsigned ife_outl_cnt1          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_outl_cnt2          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_105; // 0x1a4;

	union {
		struct {
			unsigned ife_ord_range_bri      :  3;            //bits : 2..0
			unsigned                        :  1;
			unsigned ife_ord_range_dark     :  3;            //bits : 6..4
			unsigned                        :  1;
			unsigned ife_ord_protect_th     : 10;            //bits : 17..8
			unsigned                        :  2;
			unsigned ife_ord_blend_w        :  8;            //bits : 27..20
		} bit;
		UINT32 word;
	} ife_register_106; // 0x1a8;

	union {
		struct {
			unsigned ife_ord_bri_wlut0      :  4;            //bits : 3..0
			unsigned ife_ord_bri_wlut1      :  4;            //bits : 7..4
			unsigned ife_ord_bri_wlut2      :  4;            //bits : 11..8
			unsigned ife_ord_bri_wlut3      :  4;            //bits : 15..12
			unsigned ife_ord_bri_wlut4      :  4;            //bits : 19..16
			unsigned ife_ord_bri_wlut5      :  4;            //bits : 23..20
			unsigned ife_ord_bri_wlut6      :  4;            //bits : 27..24
			unsigned ife_ord_bri_wlut7      :  4;            //bits : 31..28
		} bit;
		UINT32 word;
	} ife_register_107; // 0x1ac;

	union {
		struct {
			unsigned ife_ord_dark_wlut0     :  4;            //bits : 3..0
			unsigned ife_ord_dark_wlut1     :  4;            //bits : 7..4
			unsigned ife_ord_dark_wlut2     :  4;            //bits : 11..8
			unsigned ife_ord_dark_wlut3     :  4;            //bits : 15..12
			unsigned ife_ord_dark_wlut4     :  4;            //bits : 19..16
			unsigned ife_ord_dark_wlut5     :  4;            //bits : 23..20
			unsigned ife_ord_dark_wlut6     :  4;            //bits : 27..24
			unsigned ife_ord_dark_wlut7     :  4;            //bits : 31..28
		} bit;
		UINT32 word;
	} ife_register_108; // 0x1b0;

	union {
		struct {
			unsigned ife_gbal_edge_protect_en :  1;         //bits : 0
			unsigned                          :  7;
			unsigned ife_gbal_diff_thr_str    : 12;         // bits : 19_8
			unsigned ife_gbal_diff_w_max      :  4;         //bits : 23..20
		} bit;
		UINT32 word;
	} ife_register_109; // 0x1b4;

	union {
		struct {
			unsigned ife_gbal_edge_thr_1    : 12;            //bits : 11..0
			unsigned                        :  4;
			unsigned ife_gbal_edge_thr_0    : 12;            //bits : 27..16
		} bit;
		UINT32 word;
	} ife_register_110; // 0x1b8;

	union {
		struct {
			unsigned ife_gbal_edge_w_max    :  8;            //bits : 7..0
			unsigned ife_gbal_edge_w_min    :  8;            //bits : 15..8
		} bit;
		UINT32 word;
	} ife_register_111; // 0x1bc;

	union {
		struct {
			unsigned ife_rth_a_c0_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c0_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_112; // 0x1c0;

	union {
		struct {
			unsigned ife_rth_a_c0_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c0_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_113; // 0x1c4;

	union {
		struct {
			unsigned ife_rth_a_c0_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c0_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_114; // 0x1c8;

	union {
		struct {
			unsigned ife_rth_a_c0_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c0_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_115; // 0x1cc;

	union {
		struct {
			unsigned ife_rth_a_c0_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c0_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_116; // 0x1d0;

	union {
		struct {
			unsigned ife_rth_a_c0_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c0_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_117; // 0x1d4;

	union {
		struct {
			unsigned ife_rth_a_c0_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c0_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_118; // 0x1d8;

	union {
		struct {
			unsigned ife_rth_a_c0_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c0_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_119; // 0x1dc;

	union {
		struct {
			unsigned ife_rth_a_c0_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_120; // 0x1e0;

	union {
		struct {
			unsigned ife_rth_a_c1_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c1_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_121; // 0x1e4;

	union {
		struct {
			unsigned ife_rth_a_c1_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c1_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_122; // 0x1e8;

	union {
		struct {
			unsigned ife_rth_a_c1_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c1_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_123; // 0x1ec;

	union {
		struct {
			unsigned ife_rth_a_c1_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c1_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_124; // 0x1f0;

	union {
		struct {
			unsigned ife_rth_a_c1_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c1_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_125; // 0x1f4;

	union {
		struct {
			unsigned ife_rth_a_c1_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c1_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_126; // 0x1f8;

	union {
		struct {
			unsigned ife_rth_a_c1_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c1_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_127; // 0x1fc;

	union {
		struct {
			unsigned ife_rth_a_c1_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c1_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_128; // 0x200;

	union {
		struct {
			unsigned ife_rth_a_c1_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_129; // 0x204;

	union {
		struct {
			unsigned ife_rth_a_c2_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c2_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_130; // 0x208;

	union {
		struct {
			unsigned ife_rth_a_c2_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c2_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_131; // 0x20c;

	union {
		struct {
			unsigned ife_rth_a_c2_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c2_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_132; // 0x210;

	union {
		struct {
			unsigned ife_rth_a_c2_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c2_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_133; // 0x214;

	union {
		struct {
			unsigned ife_rth_a_c2_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c2_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_134; // 0x218;

	union {
		struct {
			unsigned ife_rth_a_c2_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c2_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_135; // 0x21c;

	union {
		struct {
			unsigned ife_rth_a_c2_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c2_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_136; // 0x220;

	union {
		struct {
			unsigned ife_rth_a_c2_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c2_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_137; // 0x224;

	union {
		struct {
			unsigned ife_rth_a_c2_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_138; // 0x228;

	union {
		struct {
			unsigned ife_rth_a_c3_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c3_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_139; // 0x22c;

	union {
		struct {
			unsigned ife_rth_a_c3_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c3_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_140; // 0x230;

	union {
		struct {
			unsigned ife_rth_a_c3_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c3_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_141; // 0x234;

	union {
		struct {
			unsigned ife_rth_a_c3_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c3_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_142; // 0x238;

	union {
		struct {
			unsigned ife_rth_a_c3_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c3_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_143; // 0x23c;

	union {
		struct {
			unsigned ife_rth_a_c3_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c3_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_144; // 0x240;

	union {
		struct {
			unsigned ife_rth_a_c3_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c3_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_145; // 0x244;

	union {
		struct {
			unsigned ife_rth_a_c3_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_a_c3_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_146; // 0x248;

	union {
		struct {
			unsigned ife_rth_a_c3_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_147; // 0x24c;

	union {
		struct {
			unsigned ife_rth_b_c0_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c0_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_148; // 0x250;

	union {
		struct {
			unsigned ife_rth_b_c0_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c0_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_149; // 0x254;

	union {
		struct {
			unsigned ife_rth_b_c0_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c0_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_150; // 0x258;

	union {
		struct {
			unsigned ife_rth_b_c0_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c0_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_151; // 0x25c;

	union {
		struct {
			unsigned ife_rth_b_c0_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c0_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_152; // 0x260;

	union {
		struct {
			unsigned ife_rth_b_c0_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c0_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_153; // 0x264;

	union {
		struct {
			unsigned ife_rth_b_c0_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c0_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_154; // 0x268;

	union {
		struct {
			unsigned ife_rth_b_c0_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c0_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_155; // 0x26c;

	union {
		struct {
			unsigned ife_rth_b_c0_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_156; // 0x270;

	union {
		struct {
			unsigned ife_rth_b_c1_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c1_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_157; // 0x274;

	union {
		struct {
			unsigned ife_rth_b_c1_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c1_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_158; // 0x278;

	union {
		struct {
			unsigned ife_rth_b_c1_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c1_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_159; // 0x27c;

	union {
		struct {
			unsigned ife_rth_b_c1_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c1_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_160; // 0x280;

	union {
		struct {
			unsigned ife_rth_b_c1_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c1_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_161; // 0x284;

	union {
		struct {
			unsigned ife_rth_b_c1_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c1_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_162; // 0x288;

	union {
		struct {
			unsigned ife_rth_b_c1_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c1_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_163; // 0x28c;

	union {
		struct {
			unsigned ife_rth_b_c1_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c1_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_164; // 0x290;

	union {
		struct {
			unsigned ife_rth_b_c1_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_165; // 0x294;

	union {
		struct {
			unsigned ife_rth_b_c2_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c2_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_166; // 0x298;

	union {
		struct {
			unsigned ife_rth_b_c2_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c2_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_167; // 0x29c;

	union {
		struct {
			unsigned ife_rth_b_c2_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c2_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_168; // 0x2a0;

	union {
		struct {
			unsigned ife_rth_b_c2_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c2_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_169; // 0x2a4;

	union {
		struct {
			unsigned ife_rth_b_c2_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c2_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_170; // 0x2a8;

	union {
		struct {
			unsigned ife_rth_b_c2_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c2_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_171; // 0x2ac;

	union {
		struct {
			unsigned ife_rth_b_c2_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c2_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_172; // 0x2b0;

	union {
		struct {
			unsigned ife_rth_b_c2_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c2_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_173; // 0x2b4;

	union {
		struct {
			unsigned ife_rth_b_c2_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_174; // 0x2b8;

	union {
		struct {
			unsigned ife_rth_b_c3_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c3_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_175; // 0x2bc;

	union {
		struct {
			unsigned ife_rth_b_c3_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c3_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_176; // 0x2c0;

	union {
		struct {
			unsigned ife_rth_b_c3_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c3_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_177; // 0x2c4;

	union {
		struct {
			unsigned ife_rth_b_c3_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c3_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_178; // 0x2c8;

	union {
		struct {
			unsigned ife_rth_b_c3_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c3_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_179; // 0x2cc;

	union {
		struct {
			unsigned ife_rth_b_c3_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c3_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_180; // 0x2d0;

	union {
		struct {
			unsigned ife_rth_b_c3_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c3_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_181; // 0x2d4;

	union {
		struct {
			unsigned ife_rth_b_c3_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_rth_b_c3_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_182; // 0x2d8;

	union {
		struct {
			unsigned ife_rth_b_c3_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_183; // 0x2dc;

	union {
		struct {
			unsigned ife_distvgtx_c0        : 14;            //bits : 13..0
			unsigned                        :  2;
			unsigned ife_distvgty_c0        : 14;            //bits : 29..16
		} bit;
		UINT32 word;
	} ife_register_184; // 0x2e0;

	union {
		struct {
			unsigned ife_distvgtx_c1        : 14;            //bits : 13..0
			unsigned                        :  2;
			unsigned ife_distvgty_c1        : 14;            //bits : 29..16
		} bit;
		UINT32 word;
	} ife_register_185; // 0x2e4;

	union {
		struct {
			unsigned ife_distvgtx_c2        : 14;            //bits : 13..0
			unsigned                        :  2;
			unsigned ife_distvgty_c2        : 14;            //bits : 29..16
		} bit;
		UINT32 word;
	} ife_register_186; // 0x2e8;

	union {
		struct {
			unsigned ife_distvgtx_c3        : 14;            //bits : 13..0
			unsigned                        :  2;
			unsigned ife_distvgty_c3        : 14;            //bits : 29..16
		} bit;
		UINT32 word;
	} ife_register_187; // 0x2ec;

	union {
		struct {
			unsigned ife_distvgxdiv         : 12;            //bits : 11..0
			unsigned ife_distvgydiv         : 12;            //bits : 23..12
			unsigned                        : 4;
			unsigned ife_distgain           : 2;            //bits : 29..28
		} bit;
		UINT32 word;
	} ife_register_188; // 0x2f0;

	union {
		struct {
			unsigned distdthr_en             :  1;            //bits : 0
			unsigned                         :  3;
			unsigned distdthr_rst            :  1;            //bits : 4
			unsigned                         :  3;
			unsigned distthr                 : 10;            //bits : 17..8
		} bit;
		UINT32 word;
	} ife_register_189; // 0x2f4;

	union {
		struct {
			unsigned ife_vig_c0_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c0_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_190; // 0x2f8;

	union {
		struct {
			unsigned ife_vig_c0_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c0_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_191; // 0x2fc;

	union {
		struct {
			unsigned ife_vig_c0_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c0_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_192; // 0x300;

	union {
		struct {
			unsigned ife_vig_c0_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c0_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_193; // 0x304;

	union {
		struct {
			unsigned ife_vig_c0_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c0_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_194; // 0x308;

	union {
		struct {
			unsigned ife_vig_c0_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c0_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_195; // 0x30c;

	union {
		struct {
			unsigned ife_vig_c0_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c0_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_196; // 0x310;

	union {
		struct {
			unsigned ife_vig_c0_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c0_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_197; // 0x314;

	union {
		struct {
			unsigned ife_vig_c0_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_198; // 0x318;

	union {
		struct {
			unsigned ife_vig_c1_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c1_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_199; // 0x31c;

	union {
		struct {
			unsigned ife_vig_c1_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c1_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_200; // 0x320;

	union {
		struct {
			unsigned ife_vig_c1_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c1_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_201; // 0x324;

	union {
		struct {
			unsigned ife_vig_c1_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c1_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_202; // 0x328;

	union {
		struct {
			unsigned ife_vig_c1_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c1_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_203; // 0x32c;

	union {
		struct {
			unsigned ife_vig_c1_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c1_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_204; // 0x330;

	union {
		struct {
			unsigned ife_vig_c1_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c1_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_205; // 0x334;

	union {
		struct {
			unsigned ife_vig_c1_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c1_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_206; // 0x338;

	union {
		struct {
			unsigned ife_vig_c1_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_207; // 0x33c;

	union {
		struct {
			unsigned ife_vig_c2_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c2_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_208; // 0x340;

	union {
		struct {
			unsigned ife_vig_c2_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c2_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_209; // 0x344;

	union {
		struct {
			unsigned ife_vig_c2_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c2_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_210; // 0x348;

	union {
		struct {
			unsigned ife_vig_c2_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c2_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_211; // 0x34c;

	union {
		struct {
			unsigned ife_vig_c2_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c2_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_212; // 0x350;

	union {
		struct {
			unsigned ife_vig_c2_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c2_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_213; // 0x354;

	union {
		struct {
			unsigned ife_vig_c2_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c2_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_214; // 0x358;

	union {
		struct {
			unsigned ife_vig_c2_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c2_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_215; // 0x35c;

	union {
		struct {
			unsigned ife_vig_c2_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_216; // 0x360;

	union {
		struct {
			unsigned ife_vig_c3_lut00   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c3_lut01   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_217; // 0x364;

	union {
		struct {
			unsigned ife_vig_c3_lut02   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c3_lut03   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_218; // 0x368;

	union {
		struct {
			unsigned ife_vig_c3_lut04   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c3_lut05   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_219; // 0x36c;

	union {
		struct {
			unsigned ife_vig_c3_lut06   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c3_lut07   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_220; // 0x370;

	union {
		struct {
			unsigned ife_vig_c3_lut08   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c3_lut09   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_221; // 0x374;

	union {
		struct {
			unsigned ife_vig_c3_lut10   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c3_lut11   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_222; // 0x378;

	union {
		struct {
			unsigned ife_vig_c3_lut12   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c3_lut13   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_223; // 0x37c;

	union {
		struct {
			unsigned ife_vig_c3_lut14   : 10;            //bits : 9..0
			unsigned                      :  6;
			unsigned ife_vig_c3_lut15   : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} ife_register_224; // 0x380;

	union {
		struct {
			unsigned ife_vig_c3_lut16   : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} ife_register_225; // 0x384;

	union {
		struct {
			unsigned ife_gbal_ofs_lut_00    :  6;            //bits : 5..0
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_01    :  6;            //bits : 13..8
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_02    :  6;            //bits : 21..16
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_03    :  6;            //bits : 29..24
		} bit;
		UINT32 word;
	} ife_register_226; // 0x388;

	union {
		struct {
			unsigned ife_gbal_ofs_lut_04    :  6;            //bits : 5..0
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_05    :  6;            //bits : 13..8
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_06    :  6;            //bits : 21..16
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_07    :  6;            //bits : 29..24
		} bit;
		UINT32 word;
	} ife_register_227; // 0x38c;

	union {
		struct {
			unsigned ife_gbal_ofs_lut_08    :  6;            //bits : 5..0
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_09    :  6;            //bits : 13..8
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_10    :  6;            //bits : 21..16
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_11    :  6;            //bits : 29..24
		} bit;
		UINT32 word;
	} ife_register_228; // 0x390;

	union {
		struct {
			unsigned ife_gbal_ofs_lut_12    :  6;            //bits : 5..0
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_13    :  6;            //bits : 13..8
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_14    :  6;            //bits : 21..16
			unsigned                        :  2;
			unsigned ife_gbal_ofs_lut_15    :  6;            //bits : 29..24
		} bit;
		UINT32 word;
	} ife_register_229; // 0x394;

	union {
		struct {
			unsigned ife_gbal_ofs_lut_16    :  6;            //bits : 5..0
		} bit;
		UINT32 word;
	} ife_register_230; // 0x398;

	union {
		struct {
			unsigned ife_rbluma00           :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbluma01           :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbluma02           :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbluma03           :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_231; // 0x39c;

	union {
		struct {
			unsigned ife_rbluma04           :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbluma05           :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbluma06           :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbluma07           :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_232; // 0x3a0;

	union {
		struct {
			unsigned ife_rbluma08           :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbluma09           :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbluma10           :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbluma11           :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_233; // 0x3a4;

	union {
		struct {
			unsigned ife_rbluma12           :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbluma13           :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbluma14           :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbluma15           :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_234; // 0x3a8;

	union {
		struct {
			unsigned ife_rbluma16           :  5;            //bits : 4..0
		} bit;
		UINT32 word;
	} ife_register_235; // 0x3ac;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_236; // 0x3b0;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_237; // 0x3b4;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_238; // 0x3b8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_239; // 0x3bc;

	union {
		struct {
			unsigned ife_rbratio00          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbratio01          :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbratio02          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbratio03          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_240; // 0x3c0;

	union {
		struct {
			unsigned ife_rbratio04          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbratio05          :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbratio06          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbratio07          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_241; // 0x3c4;

	union {
		struct {
			unsigned ife_rbratio08          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbratio09          :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbratio10          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbratio11          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_242; // 0x3c8;

	union {
		struct {
			unsigned ife_rbratio12          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbratio13          :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbratio14          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbratio15          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_243; // 0x3cc;

	union {
		struct {
			unsigned ife_rbratio16          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbratio17          :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbratio18          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbratio19          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_244; // 0x3d0;

	union {
		struct {
			unsigned ife_rbratio20          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbratio21          :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbratio22          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbratio23          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_245; // 0x3d4;

	union {
		struct {
			unsigned ife_rbratio24          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbratio25          :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbratio26          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbratio27          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} ife_register_246; // 0x3d8;

	union {
		struct {
			unsigned ife_rbratio28          :  5;            //bits : 4..0
			unsigned                        :  3;
			unsigned ife_rbratio29          :  5;            //bits : 12..8
			unsigned                        :  3;
			unsigned ife_rbratio30          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned ife_rbratio31          :  5;            //bits : 28..24
			unsigned rbratio_mode           :  2;            //bits : 30..29
		} bit;
		UINT32 word;
	} ife_register_247; // 0x3dc;

	union {
		struct {
			unsigned ife_ll_table_index0        : 8;        // bits : 7_0
			unsigned ife_ll_table_index1        : 8;        // bits : 15_8
			unsigned ife_ll_table_index2        : 8;        // bits : 23_16
			unsigned ife_ll_table_index3        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_248; // 0x3e0;

	union {
		struct {
			unsigned ife_ll_table_index4        : 8;        // bits : 7_0
			unsigned ife_ll_table_index5        : 8;        // bits : 15_8
			unsigned ife_ll_table_index6        : 8;        // bits : 23_16
			unsigned ife_ll_table_index7        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_249; // 0x3e4;

	union {
		struct {
			unsigned ife_ll_table_index8         : 8;       // bits : 7_0
			unsigned ife_ll_table_index9         : 8;       // bits : 15_8
			unsigned ife_ll_table_index10        : 8;       // bits : 23_16
			unsigned ife_ll_table_index11        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_250; // 0x3e8;

	union {
		struct {
			unsigned ife_ll_table_index12        : 8;       // bits : 7_0
			unsigned ife_ll_table_index13        : 8;       // bits : 15_8
			unsigned ife_ll_table_index14        : 8;       // bits : 23_16
			unsigned ife_ll_table_index15        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_251; // 0x3ec;

	union {
		struct {
			unsigned ife_axi_disable           : 1;     // bits : 0
			unsigned ife_axi_ch_idle           : 1;     // bits : 1
			unsigned                           : 6;
			unsigned ife_axi_channel_en        : 9;     // bits : 16_8
			unsigned ife_axi_lock_dis          : 9;     // bits : 25_17
		} bit;
		UINT32 word;
	} ife_register_252; // 0x3f0;

	union {
		struct {
			unsigned ife_axi_ch_sta        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_253; // 0x3f4;

	union {
		struct {
			unsigned ife_r_ostd_num            : 8;     // bits : 7_0
			unsigned ife_w_ostd_num            : 8;     // bits : 15_8
		} bit;
		UINT32 word;
	} ife_register_254; // 0x3f8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_255; // 0x3fc;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_256; // 0x400;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_257; // 0x404;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_258; // 0x408;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_259; // 0x40c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_260; // 0x410;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_261; // 0x414;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_262; // 0x418;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_263; // 0x41c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_264; // 0x420;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_265; // 0x424;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_266; // 0x428;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_267; // 0x42c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_268; // 0x430;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_269; // 0x434;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_270; // 0x438;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_271; // 0x43c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_272; // 0x440;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_273; // 0x444;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_274; // 0x448;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_275; // 0x44c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_276; // 0x450;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_277; // 0x454;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_278; // 0x458;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_279; // 0x45c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_280; // 0x460;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_281; // 0x464;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_282; // 0x468;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_283; // 0x46c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_284; // 0x470;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_285; // 0x474;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_286; // 0x478;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_287; // 0x47c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_288; // 0x480;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_289; // 0x484;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_290; // 0x488;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_291; // 0x48c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_292; // 0x490;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_293; // 0x494;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_294; // 0x498;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_295; // 0x49c;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_296; // 0x4a0;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_297; // 0x4a4;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_298; // 0x4a8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_299; // 0x4ac;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_300; // 0x4b0;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_301; // 0x4b4;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_302; // 0x4b8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_303; // 0x4bc;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_304; // 0x4c0;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_305; // 0x4c4;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_306; // 0x4c8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_307; // 0x4cc;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_308; // 0x4d0;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_309; // 0x4d4;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_310; // 0x4d8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_311; // 0x4dc;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_312; // 0x4e0;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_313; // 0x4e4;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_314; // 0x4e8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_315; // 0x4ec;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_316; // 0x4f0;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_317; // 0x4f4;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_318; // 0x4f8;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_319; // 0x4fc;

	union {
		struct {
			unsigned ife_f_nrs0_en        : 1;      // bits : 0
		} bit;
		UINT32 word;
	} ife_register_320; // 0x500;

	union {
		struct {
			unsigned ife_f_nrs0_str0        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned ife_f_nrs0_str1        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} ife_register_321; // 0x504;

	union {
		struct {
			unsigned ife_f_nrs0_str2        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned ife_f_nrs0_str3        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} ife_register_322; // 0x508;

	union {
		struct {
			unsigned ife_f_nrs0_str4        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned ife_f_nrs0_str5        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} ife_register_323; // 0x50c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_324; // 0x510;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_325; // 0x514;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_326; // 0x518;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_327; // 0x51c;

	union {
		struct {
			unsigned ife_f_fu_ym_sel               : 2;     // bits : 1_0
			unsigned                               : 2;
			unsigned ife_f_fu_bcn_sel              : 2;     // bits : 5_4
			unsigned                               : 2;
			unsigned ife_f_fu_bcd_sel              : 2;     // bits : 9_8
			unsigned                               : 2;
			unsigned ife_f_fu_mode                 : 2;     // bits : 13_12
			unsigned                               : 14;
			unsigned ife_f_fu_dbg_mode             : 4;     // bits : 31_28
		} bit;
		UINT32 word;
	} ife_register_328; // 0x520;

	union {
		struct {
			unsigned ife_f_fu_bcnl_p0                     : 12;     // bits : 11_0
			unsigned                                      : 4;
			unsigned ife_f_fu_bcnl_range                  : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_329; // 0x524;

	union {
		struct {
			unsigned ife_f_fu_bcns_p0                     : 12;     // bits : 11_0
			unsigned                                      : 4;
			unsigned ife_f_fu_bcns_range                  : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_330; // 0x528;

	union {
		struct {
			unsigned ife_f_fu_bcdl_p0                     : 12;     // bits : 11_0
			unsigned                                      : 4;
			unsigned ife_f_fu_bcdl_range                  : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_331; // 0x52c;

	union {
		struct {
			unsigned ife_f_fu_bcds_p0                     : 12;     // bits : 11_0
			unsigned                                      : 4;
			unsigned ife_f_fu_bcds_range                  : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_332; // 0x530;

	union {
		struct {
			unsigned ife_f_fu_bcnl_slope        : 16;       // bits : 15_0
			unsigned ife_f_fu_bcns_slope        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_333; // 0x534;

	union {
		struct {
			unsigned ife_f_fu_bcdl_slope        : 16;       // bits : 15_0
			unsigned ife_f_fu_bcds_slope        : 16;       // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_334; // 0x538;

	union {
		struct {
			unsigned ife_f_fu_evratio        : 9;       // bits : 8_0
		} bit;
		UINT32 word;
	} ife_register_335; // 0x53c;

	union {
		struct {
			unsigned ife_f_fu_mc_lumthr     :  8;            //bits : 7..0
			unsigned                        :  4;            //bits : 11..8
			unsigned ife_f_fu_mc_diff_ratio :  2;            //bits : 13..12
		} bit;
		UINT32 word;
	} ife_register_336; // 0x540;

	union {
		struct {
			unsigned ife_f_fu_mc_lut_dwp0   :  5;            //bits : 4..0
			unsigned ife_f_fu_mc_lut_dwp1   :  5;            //bits : 9..5
			unsigned ife_f_fu_mc_lut_dwp2   :  5;            //bits : 14..10
			unsigned ife_f_fu_mc_lut_dwp3   :  5;            //bits : 19..15
			unsigned ife_f_fu_mc_lut_dwp4   :  5;            //bits : 24..20
			unsigned ife_f_fu_mc_lut_dwp5   :  5;            //bits : 29..25
		} bit;
		UINT32 word;
	} ife_register_337; // 0x544;

	union {
		struct {
			unsigned ife_f_fu_mc_lut_dwp6   :  5;            //bits : 4..0
			unsigned ife_f_fu_mc_lut_dwp7   :  5;            //bits : 9..5
			unsigned ife_f_fu_mc_lut_dwp8   :  5;            //bits : 14..10
			unsigned ife_f_fu_mc_lut_dwp9   :  5;            //bits : 19..15
			unsigned ife_f_fu_mc_lut_dwp10  :  5;            //bits : 24..20
			unsigned ife_f_fu_mc_lut_dwp11  :  5;            //bits : 29..25
		} bit;
		UINT32 word;
	} ife_register_338; // 0x548;

	union {
		struct {
			unsigned ife_f_fu_mc_lut_dwp12  :  5;            //bits : 4..0
			unsigned ife_f_fu_mc_lut_dwp13  :  5;            //bits : 9..5
			unsigned ife_f_fu_mc_lut_dwp14  :  5;            //bits : 14..10
			unsigned ife_f_fu_mc_lut_dwp15  :  5;            //bits : 19..15
			unsigned ife_f_fu_mc_lut_dwd    :  5;            //bits : 24..20
		} bit;
		UINT32 word;
	} ife_register_339; // 0x54c;

	union {
		struct {
			unsigned ife_f_ds0_th           : 12;            //bits : 11..0
			unsigned                        :  4;
			unsigned ife_f_ds0_step         :  8;            //bits : 23..16
			unsigned ife_f_ds0_lb           :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} ife_register_340; // 0x550;

	union {
		struct {
			unsigned ife_f_ds1_th           : 12;            //bits : 11..0
			unsigned                        :  4;
			unsigned ife_f_ds1_step         :  8;            //bits : 23..16
			unsigned ife_f_ds1_lb           :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} ife_register_341; // 0x554;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_342; // 0x558;

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_343; // 0x55c;

	union {
		struct {
			unsigned ife_f_fc_ym_sel        :  2;            //bits : 1..0
			unsigned ife_f_fc_yvweight      :  4;            //bits : 5..2
		} bit;
		UINT32 word;
	} ife_register_344; // 0x560;

	union {
		struct {
			unsigned ife_f_fc_yweight_lut0  :  8;            //bits : 7..0
			unsigned ife_f_fc_yweight_lut1  :  8;            //bits : 15..8
			unsigned ife_f_fc_yweight_lut2  :  8;            //bits : 23..16
			unsigned ife_f_fc_yweight_lut3  :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} ife_register_345; // 0x564;

	union {
		struct {
			unsigned ife_f_fc_yweight_lut4  :  8;            //bits : 7..0
			unsigned ife_f_fc_yweight_lut5  :  8;            //bits : 15..8
			unsigned ife_f_fc_yweight_lut6  :  8;            //bits : 23..16
			unsigned ife_f_fc_yweight_lut7  :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} ife_register_346; // 0x568;

	union {
		struct {
			unsigned ife_f_fc_yweight_lut8  :  8;            //bits : 7..0
			unsigned ife_f_fc_yweight_lut9  :  8;            //bits : 15..8
			unsigned ife_f_fc_yweight_lut10 :  8;            //bits : 23..16
			unsigned ife_f_fc_yweight_lut11 :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} ife_register_347; // 0x56c;

	union {
		struct {
			unsigned ife_f_fc_yweight_lut12 :  8;            //bits : 7..0
			unsigned ife_f_fc_yweight_lut13 :  8;            //bits : 15..8
			unsigned ife_f_fc_yweight_lut14 :  8;            //bits : 23..16
			unsigned ife_f_fc_yweight_lut15 :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} ife_register_348; // 0x570;

	union {
		struct {
			unsigned ife_f_fc_yweight_lut16 :  8;            //bits : 7..0
		} bit;
		UINT32 word;
	} ife_register_349; // 0x574;

	union {
		struct {
			unsigned ife_f_fcurve_l_0        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_1        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_350; // 0x578;

	union {
		struct {
			unsigned ife_f_fcurve_l_2        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_3        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_351; // 0x57c;

	union {
		struct {
			unsigned ife_f_fcurve_l_4        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_5        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_352; // 0x580;

	union {
		struct {
			unsigned ife_f_fcurve_l_6        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_7        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_353; // 0x584;

	union {
		struct {
			unsigned ife_f_fcurve_l_8        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_9        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_354; // 0x588;

	union {
		struct {
			unsigned ife_f_fcurve_l_10        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_11        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_355; // 0x58c;

	union {
		struct {
			unsigned ife_f_fcurve_l_12        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_13        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_356; // 0x590;

	union {
		struct {
			unsigned ife_f_fcurve_l_14        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_15        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_357; // 0x594;

	union {
		struct {
			unsigned ife_f_fcurve_l_16        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_17        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_358; // 0x598;

	union {
		struct {
			unsigned ife_f_fcurve_l_18        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_19        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_359; // 0x59c;

	union {
		struct {
			unsigned ife_f_fcurve_l_20        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_21        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_360; // 0x5a0;

	union {
		struct {
			unsigned ife_f_fcurve_l_22        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_23        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_361; // 0x5a4;

	union {
		struct {
			unsigned ife_f_fcurve_l_24        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_25        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_362; // 0x5a8;

	union {
		struct {
			unsigned ife_f_fcurve_l_26        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_27        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_363; // 0x5ac;

	union {
		struct {
			unsigned ife_f_fcurve_l_28        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_29        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_364; // 0x5b0;

	union {
		struct {
			unsigned ife_f_fcurve_l_30        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_31        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_365; // 0x5b4;

	union {
		struct {
			unsigned ife_f_fcurve_l_32        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_33        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_366; // 0x5b8;

	union {
		struct {
			unsigned ife_f_fcurve_l_34        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_35        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_367; // 0x5bc;

	union {
		struct {
			unsigned ife_f_fcurve_l_36        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_37        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_368; // 0x5c0;

	union {
		struct {
			unsigned ife_f_fcurve_l_38        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_39        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_369; // 0x5c4;

	union {
		struct {
			unsigned ife_f_fcurve_l_40        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_41        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_370; // 0x5c8;

	union {
		struct {
			unsigned ife_f_fcurve_l_42        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_43        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_371; // 0x5cc;

	union {
		struct {
			unsigned ife_f_fcurve_l_44        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_45        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_372; // 0x5d0;

	union {
		struct {
			unsigned ife_f_fcurve_l_46        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_47        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_373; // 0x5d4;

	union {
		struct {
			unsigned ife_f_fcurve_l_48        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_49        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_374; // 0x5d8;

	union {
		struct {
			unsigned ife_f_fcurve_l_50        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_51        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_375; // 0x5dc;

	union {
		struct {
			unsigned ife_f_fcurve_l_52        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_53        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_376; // 0x5e0;

	union {
		struct {
			unsigned ife_f_fcurve_l_54        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_55        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_377; // 0x5e4;

	union {
		struct {
			unsigned ife_f_fcurve_l_56        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_57        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_378; // 0x5e8;

	union {
		struct {
			unsigned ife_f_fcurve_l_58        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_59        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_379; // 0x5ec;

	union {
		struct {
			unsigned ife_f_fcurve_l_60        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_61        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_380; // 0x5f0;

	union {
		struct {
			unsigned ife_f_fcurve_l_62        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_l_63        : 16;      // bits : 31_16

		} bit;
		UINT32 word;
	} ife_register_381; // 0x5f4;

	union {
		struct {
			unsigned ife_f_fcurve_l_64        : 16;      // bits : 15_0

		} bit;
		UINT32 word;
	} ife_register_382; // 0x5f8;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_383; // 0x5fc;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_384; // 0x600;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_385; // 0x604;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_386; // 0x608;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_387; // 0x60c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_388; // 0x610;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_389; // 0x614;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_390; // 0x618;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_391; // 0x61c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_392; // 0x620;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_393; // 0x624;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_394; // 0x628;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_395; // 0x62c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_396; // 0x630;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_397; // 0x634;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_398; // 0x638;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_399; // 0x63c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_400; // 0x640;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_401; // 0x644;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_402; // 0x648;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_403; // 0x64c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_404; // 0x650;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_405; // 0x654;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_406; // 0x658;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_407; // 0x65c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_408; // 0x660;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_409; // 0x664;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_410; // 0x668;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_411; // 0x66c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_412; // 0x670;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_413; // 0x674;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_414; // 0x678;

	union {
		struct {
			unsigned ife_f_fcurve_r_0        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_r_1        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_415; // 0x67c;

	union {
		struct {
			unsigned ife_f_fcurve_r_2        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_r_3        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_416; // 0x680;

	union {
		struct {
			unsigned ife_f_fcurve_r_4        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_r_5        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_417; // 0x684;

	union {
		struct {
			unsigned ife_f_fcurve_r_6        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_r_7        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_418; // 0x688;

	union {
		struct {
			unsigned ife_f_fcurve_r_8        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_r_9        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_419; // 0x68c;

	union {
		struct {
			unsigned ife_f_fcurve_r_10        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_r_11        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_420; // 0x690;

	union {
		struct {
			unsigned ife_f_fcurve_r_12        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_r_13        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_421; // 0x694;

	union {
		struct {
			unsigned ife_f_fcurve_r_14        : 16;      // bits : 15_0
			unsigned ife_f_fcurve_r_15        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_422; // 0x698;

	union {
		struct {
			unsigned ife_f_fcurve_r_16        : 16;      // bits : 15_0
			unsigned                            :  8;
			unsigned ife_f_fcurve_ev_fmt        :  3;       // bits : 26_24
		} bit;
		UINT32 word;
	} ife_register_423; // 0x69c;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_424; // 0x6a0;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_425; // 0x6a4;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_426; // 0x6a8;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_427; // 0x6ac;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_428; // 0x6b0;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_429; // 0x6b4;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_430; // 0x6b8;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_431; // 0x6bc;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_432; // 0x6c0;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_433; // 0x6c4;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_434; // 0x6c8;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_435; // 0x6cc;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_436; // 0x6d0;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_437; // 0x6d4;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_438; // 0x6d8;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_439; // 0x6dc;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_440; // 0x6e0;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_441; // 0x6e4;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_442; // 0x6e8;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_443; // 0x6ec;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_444; // 0x6f0;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_445; // 0x6f4;

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_446; // 0x6f8;

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_447; // 0x6fc;

	/*  union
	  {
	      struct
	      {
	          unsigned ife_r_degamma_en       :  1;            //bits : 0
	          unsigned                        :  3;
	          unsigned ife_r_dith_en          :  1;            //bits : 4
	          unsigned                        :  3;
	          unsigned ife_r_segbitno         :  2;            //bits : 9..8
	          unsigned                        :  2;
	          unsigned ife_r_dith_rst         :  1;            //bits : 12
	      } bit;
	      UINT32 word;
	  } ife_register_448; // 0x700;

	  union
	  {
	      struct
	      {
	          unsigned ife_r_dct_qtbl0_idx    :  5;            //bits : 4..0
	          unsigned                        :  3;
	          unsigned ife_r_dct_qtbl1_idx    :  5;            //bits : 12..8
	          unsigned                        :  3;
	          unsigned ife_r_dct_qtbl2_idx    :  5;            //bits : 20..16
	          unsigned                        :  3;
	          unsigned ife_r_dct_qtbl3_idx    :  5;            //bits : 28..24
	      } bit;
	      UINT32 word;
	  } ife_register_449; // 0x704;

	  union
	  {
	      struct
	      {
	          unsigned ife_r_dct_qtbl4_idx    :  5;            //bits : 4..0
	          unsigned                        :  3;
	          unsigned ife_r_dct_qtbl5_idx    :  5;            //bits : 12..8
	          unsigned                        :  3;
	          unsigned ife_r_dct_qtbl6_idx    :  5;            //bits : 20..16
	          unsigned                        :  3;
	          unsigned ife_r_dct_qtbl7_idx    :  5;            //bits : 28..24
	      } bit;
	      UINT32 word;
	  } ife_register_450; // 0x708;

	  union
	  {
	      struct
	      {
	          unsigned ife_r_out_rand1_init1        : 4;      // bits : 3_0
	          unsigned ife_r_out_rand1_init2        : 15;     // bits : 18_4
	      } bit;
	      UINT32 word;
	  } ife_register_451; // 0x70c;

	  union
	  {
	      struct
	      {
	          unsigned ife_r_out_rand2_init1        : 4;      // bits : 3_0
	          unsigned ife_r_out_rand2_init2        : 15;     // bits : 18_4
	      } bit;
	      UINT32 word;
	  } ife_register_452; // 0x710;

	  union
	  {
	      struct
	      {
	          unsigned ife_r_out_rand3_init1        : 4;      // bits : 3_0
	          unsigned ife_r_out_rand3_init2        : 15;     // bits : 18_4
	      } bit;
	      UINT32 word;
	  } ife_register_453; // 0x714;*/


	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_448; // 0x0700

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_449; // 0x0704

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_450; // 0x0708

	union {
		struct {
			unsigned ife_fpn_en                 : 1;        // bits : 0
			unsigned ife_fpn_cgain_range        : 1;        // bits : 1
		} bit;
		UINT32 word;
	} ife_register_451; // 0x070c

	union {
		struct {
			unsigned ife_fpn_p0_cgain_r         : 10;       // bits : 9_0
			unsigned                            : 6;
			unsigned ife_fpn_p0_cgain_gr        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} ife_register_452; // 0x0710

	union {
		struct {
			unsigned ife_fpn_p0_cgain_gb        : 10;       // bits : 9_0
			unsigned                            : 6;
			unsigned ife_fpn_p0_cgain_b         : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} ife_register_453; // 0x0714

	union {
		struct {
			unsigned ife_fpn_p0_cgain_ir        : 10;       // bits : 9_0
		} bit;
		UINT32 word;
	} ife_register_454; // 0x0718

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_455; // 0x071c

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_456; // 0x0720

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_457; // 0x0724

	union {
		struct {
			unsigned ife_fpn_p0_cofs_r         : 10;        // bits : 9_0
			unsigned                           : 6;
			unsigned ife_fpn_p0_cofs_gr        : 10;        // bits : 25_16
		} bit;
		UINT32 word;
	} ife_register_458; // 0x0728

	union {
		struct {
			unsigned ife_fpn_p0_cofs_gb        : 10;        // bits : 9_0
			unsigned                           : 6;
			unsigned ife_fpn_p0_cofs_b         : 10;        // bits : 25_16
		} bit;
		UINT32 word;
	} ife_register_459; // 0x072c

	union {
		struct {
			unsigned ife_fpn_p0_cofs_ir        : 10;        // bits : 9_0
		} bit;
		UINT32 word;
	} ife_register_460; // 0x0730

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_461; // 0x0734

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_462; // 0x0738

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_463; // 0x073c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_464; // 0x0740

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_465; // 0x0744

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_466; // 0x0748

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_467; // 0x074c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_468; // 0x0750

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_469; // 0x0754

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_470; // 0x0758

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_471; // 0x075c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_472; // 0x0760

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_473; // 0x0764

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_474; // 0x0768

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_475; // 0x076c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_476; // 0x0770

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_477; // 0x0774

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_478; // 0x0778

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_479; // 0x077c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_480; // 0x0780

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_481; // 0x0784

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_482; // 0x0788

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_483; // 0x078c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_484; // 0x0790

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_485; // 0x0794

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_486; // 0x0798

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_487; // 0x079c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_488; // 0x07a0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_489; // 0x07a4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_490; // 0x07a8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_491; // 0x07ac

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_492; // 0x07b0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_493; // 0x07b4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_494; // 0x07b8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_495; // 0x07bc

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_496; // 0x07c0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_497; // 0x07c4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_498; // 0x07c8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_499; // 0x07cc

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_500; // 0x07d0

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_501; // 0x07d4

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_502; // 0x07d8

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_503; // 0x07dc

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_504; // 0x07e0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_505; // 0x07e4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_506; // 0x07e8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_507; // 0x07ec

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_508; // 0x07f0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_509; // 0x07f4

	union {
		struct {
			unsigned ife_ll_cmd_start_addr_info        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_510; // 0x07f8

	union {
		struct {
			unsigned ife_ll_cmdprs_cnt        : 20;     // bits : 19_0
		} bit;
		UINT32 word;
	} ife_register_511; // 0x07fc

	union {
		struct {
			unsigned ife_r_degamma_en        : 1;       // bits : 0
			unsigned ife_r_decode_en         : 1;       // bits : 1
			unsigned ife_r_decode2_en        : 1;       // bits : 2
			unsigned                         : 1;
			unsigned ife_r_dith_en           : 1;       // bits : 4
			unsigned                         : 3;
			unsigned ife_r_segbitno          : 2;       // bits : 8
			unsigned                         : 2;
			unsigned ife_r_dith_rst          : 1;       // bits : 12
		} bit;
		UINT32 word;
	} ife_register_512; // 0x0800

	union {
		struct {
			unsigned ife_r_dct_qtbl0_idx        : 5;        // bits : 4_0
			unsigned                            : 3;
			unsigned ife_r_dct_qtbl1_idx        : 5;        // bits : 12_8
			unsigned                            : 3;
			unsigned ife_r_dct_qtbl2_idx        : 5;        // bits : 20_16
			unsigned                            : 3;
			unsigned ife_r_dct_qtbl3_idx        : 5;        // bits : 28_24
		} bit;
		UINT32 word;
	} ife_register_513; // 0x0804

	union {
		struct {
			unsigned ife_r_dct_qtbl4_idx        : 5;        // bits : 4_0
			unsigned                            : 3;
			unsigned ife_r_dct_qtbl5_idx        : 5;        // bits : 12_8
			unsigned                            : 3;
			unsigned ife_r_dct_qtbl6_idx        : 5;        // bits : 20_16
			unsigned                            : 3;
			unsigned ife_r_dct_qtbl7_idx        : 5;        // bits : 28_24
		} bit;
		UINT32 word;
	} ife_register_514; // 0x0808

	union {
		struct {
			unsigned ife_r_out_rand1_init1        : 4;      // bits : 3_0
			unsigned ife_r_out_rand1_init2        : 15;     // bits : 18_4
		} bit;
		UINT32 word;
	} ife_register_515; // 0x080c

	union {
		struct {
			unsigned ife_r_out_rand2_init1        : 4;      // bits : 3_0
			unsigned ife_r_out_rand2_init2        : 15;     // bits : 18_4
		} bit;
		UINT32 word;
	} ife_register_516; // 0x0810

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_517; // 0x0814

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_518; // 0x0818

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_519; // 0x081c

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_520; // 0x0820

	union {
		struct {
			
		} bit;
		UINT32 word;
	} ife_register_521; // 0x0824

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_522; // 0x0828

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_523; // 0x082c

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_524; // 0x0830

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_525; // 0x0834

	union {
		struct {
			unsigned ife_stripe_size0        : 11;      // bits : 10_0
			unsigned                         : 1;
			unsigned ife_stripe_num          : 4;       // bits : 15_12
			unsigned ife_stripe_size1        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} ife_register_526; // 0x0838

	union {
		struct {
			unsigned ife_stripe_size2        : 11;      // bits : 10_0
			unsigned                         : 5;
			unsigned ife_stripe_size3        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} ife_register_527; // 0x083c

	union {
		struct {
			unsigned ife_stripe_size4        : 11;      // bits : 10_0
			unsigned                         : 5;
			unsigned ife_stripe_size5        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} ife_register_528; // 0x0840

	union {
		struct {
			unsigned ife_stripe_size6        : 11;      // bits : 10_0
			unsigned                         : 5;
			unsigned ife_stripe_size7        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} ife_register_529; // 0x0844

	union {
		struct {
			unsigned                         : 3;
			unsigned ife_stripe_overlap      : 7;      // bits : 9_3
		} bit;
		UINT32 word;
	} ife_register_530; // 0x0848

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_531; // 0x084c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_532; // 0x0850

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_533; // 0x0854

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_534; // 0x0858

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_535; // 0x085c

	union {
		struct {
			unsigned ife_sub_ipp_cst_en          : 1;       // bits : 0
			unsigned ife_sub_ipp_gamma_en        : 1;       // bits : 1
			unsigned ife_sub_ipp_cfa_en          : 1;       // bits : 2
			unsigned ife_sub_ipp_out_yuv_fmt     : 1;       // bits : 3
		} bit;
		UINT32 word;
	} ife_register_536; // 0x0860

	union {
		struct {
			unsigned ife_sub_ipp_coef_rr        : 12;       // bits : 11_0
			unsigned                            : 4;
			unsigned ife_sub_ipp_coef_rg        : 12;       // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_537; // 0x0864

	union {
		struct {
			unsigned ife_sub_ipp_coef_rb        : 12;       // bits : 11_0
			unsigned                            : 4;
			unsigned ife_sub_ipp_coef_gr        : 12;       // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_538; // 0x0868

	union {
		struct {
			unsigned ife_sub_ipp_coef_gg        : 12;       // bits : 11_0
			unsigned                            : 4;
			unsigned ife_sub_ipp_coef_gb        : 12;       // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_539; // 0x086c

	union {
		struct {
			unsigned ife_sub_ipp_coef_br        : 12;       // bits : 11_0
			unsigned                            : 4;
			unsigned ife_sub_ipp_coef_bg        : 12;       // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_540; // 0x0870

	union {
		struct {
			unsigned ife_sub_ipp_coef_bb        : 12;       // bits : 11_0
		} bit;
		UINT32 word;
	} ife_register_541; // 0x0874

	union {
		struct {
			unsigned ife_sub_ipp_smpl_x_rate         : 5;       // bits : 2_0
			unsigned                                 : 3;
			unsigned ife_sub_ipp_smpl_y_rate         : 5;       // bits : 6_4
		} bit;
		UINT32 word;
	} ife_register_542; // 0x0878

	union {
		struct {
			unsigned ife_sub_ipp_gamma_lut_0        : 8;        // bits : 7_0
			unsigned ife_sub_ipp_gamma_lut_1        : 8;        // bits : 15_8
			unsigned ife_sub_ipp_gamma_lut_2        : 8;        // bits : 23_16
			unsigned ife_sub_ipp_gamma_lut_3        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_543; // 0x087c

	union {
		struct {
			unsigned ife_sub_ipp_gamma_lut_4        : 8;        // bits : 7_0
			unsigned ife_sub_ipp_gamma_lut_5        : 8;        // bits : 15_8
			unsigned ife_sub_ipp_gamma_lut_6        : 8;        // bits : 23_16
			unsigned ife_sub_ipp_gamma_lut_7        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_544; // 0x0880

	union {
		struct {
			unsigned ife_sub_ipp_gamma_lut_8        : 8;        // bits : 7_0
			unsigned ife_sub_ipp_gamma_lut_9        : 8;        // bits : 15_8
			unsigned ife_sub_ipp_gamma_lut_10        : 8;       // bits : 23_16
			unsigned ife_sub_ipp_gamma_lut_11        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_545; // 0x0884

	union {
		struct {
			unsigned ife_sub_ipp_gamma_lut_12        : 8;       // bits : 7_0
			unsigned ife_sub_ipp_gamma_lut_13        : 8;       // bits : 15_8
			unsigned ife_sub_ipp_gamma_lut_14        : 8;       // bits : 23_16
			unsigned ife_sub_ipp_gamma_lut_15        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_546; // 0x0888

	union {
		struct {
			unsigned ife_sub_ipp_gamma_lut_16        : 8;       // bits : 7_0
		} bit;
		UINT32 word;
	} ife_register_547; // 0x088c

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_548; // 0x0890

	union {
		struct {
			unsigned                            : 1;
			unsigned sub_ipp_crop_width         : 15;       // bits : 15_1
			unsigned                            : 1;
			unsigned sub_ipp_crop_height        : 15;       // bits : 31_17
		} bit;
		UINT32 word;
	} ife_register_549; // 0x0894

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_550; // 0x0898

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_551; // 0x089c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_552; // 0x08a0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_553; // 0x08a4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_554; // 0x08a8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_555; // 0x08ac

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_556; // 0x08b0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_557; // 0x08b4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_558; // 0x08b8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_559; // 0x08bc

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_560; // 0x08c0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_561; // 0x08c4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_562; // 0x08c8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_563; // 0x08cc

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_564; // 0x08d0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_565; // 0x08d4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_566; // 0x08d8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_567; // 0x08dc

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_568; // 0x08e0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_569; // 0x08e4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_570; // 0x08e8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_571; // 0x08ec

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_572; // 0x08f0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_573; // 0x08f4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_574; // 0x08f8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_575; // 0x08fc


	union {
		struct {
			unsigned ife_wdr_en                    : 1;     // bits : 0
			unsigned ife_wdr_subimg_out_en         : 1;     // bits : 1
			unsigned ife_wdr_histogram_en          : 1;     // bits : 2
			unsigned ife_wdr_histogram_sel         : 1;     // bits : 3
			unsigned ife_wdr_tcurve_en             : 1;     // bits : 4
			unsigned ife_wdr_d2d_rand              : 1;     // bits : 5
			unsigned ife_wdr_d2d_rand_rst          : 1;     // bits : 6
		} bit;
		UINT32 word;
	} ife_register_576; // 0x0900

	union {
		struct {
			unsigned ife_wdr_subimg_width          : 6;     // bits : 5_0
			unsigned                               : 2;
			unsigned ife_wdr_subimg_height         : 6;     // bits : 13_8
			unsigned                               : 2;
			unsigned ife_wdr_dithering_en          : 1;     // bits : 16
			unsigned ife_wdr_rand_reset            : 1;     // bits : 17
			unsigned ife_wdr_rand_sel              : 2;     // bits : 19_18
		} bit;
		UINT32 word;
	} ife_register_577; // 0x0904

	union {
		struct {
			unsigned                            : 2;
			unsigned ife_wdr_subimg_sai         : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} ife_register_578; // 0x0908

	union {
		struct {
			unsigned                             : 2;
			unsigned ife_wdr_subimg_ofsi         : 14;      // bits : 15_2
			unsigned                                 : 8;
			unsigned ife_wdr_subimg_sai_msb         : 4;       // bits : 27_24
		} bit;
		UINT32 word;
	} ife_register_579; // 0x090c

	union {
		struct {
			unsigned                            : 2;
			unsigned ife_wdr_subimg_sao         : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} ife_register_580; // 0x0910

	union {
		struct {
			unsigned                                 : 2;
			unsigned ife_wdr_subimg_ofso             : 14;      // bits : 15_2
			unsigned                                 : 8;
			unsigned ife_wdr_subimg_sao_msb         : 4;       // bits : 27_24
		} bit;
		UINT32 word;
	} ife_register_581; // 0x0914

	union {
		struct {
			unsigned ife_wdr_subimg_ratio_hori         : 16;        // bits : 15_0
			unsigned ife_wdr_subimg_ratio_vert         : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_582; // 0x0918

	union {
		struct {
			unsigned ife_wdr_subimg_ratio_hori_div        : 22;     // bits : 21_0
		} bit;
		UINT32 word;
	} ife_register_583; // 0x091c

	union {
		struct {
			unsigned ife_wdr_subimg_ratio_vert_div        : 22;     // bits : 21_0
		} bit;
		UINT32 word;
	} ife_register_584; // 0x0920

	union {
		struct {
			unsigned ife_wdr_gain_port_str        : 9;      // bits : 8_0
		} bit;
		UINT32 word;
	} ife_register_585; // 0x0924

	union {
		struct {
			unsigned ife_wdr_input_bldrto0         : 8;     // bits : 7_0
			unsigned ife_wdr_input_bldrto1         : 8;     // bits : 15_8
			unsigned ife_wdr_input_bldrto2         : 8;     // bits : 23_16
			unsigned ife_wdr_input_bldrto3         : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_586; // 0x0928

	union {
		struct {
			unsigned ife_wdr_input_bldrto4         : 8;     // bits : 7_0
			unsigned ife_wdr_input_bldrto5         : 8;     // bits : 15_8
			unsigned ife_wdr_input_bldrto6         : 8;     // bits : 23_16
			unsigned ife_wdr_input_bldrto7         : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_587; // 0x092c

	union {
		struct {
			unsigned ife_wdr_input_bldrto8         : 8;     // bits : 7_0
			unsigned ife_wdr_input_bldrto9         : 8;     // bits : 15_8
			unsigned ife_wdr_input_bldrto10        : 8;     // bits : 23_16
			unsigned ife_wdr_input_bldrto11        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_588; // 0x0930

	union {
		struct {
			unsigned ife_wdr_input_bldrto12        : 8;     // bits : 7_0
			unsigned ife_wdr_input_bldrto13        : 8;     // bits : 15_8
			unsigned ife_wdr_input_bldrto14        : 8;     // bits : 23_16
			unsigned ife_wdr_input_bldrto15        : 8;     // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_589; // 0x0934

	union {
		struct {
			unsigned ife_wdr_input_bldrto16          : 8;       // bits : 7_0
			unsigned                                 : 18;
			unsigned ife_wdr_input_bldsrc_sel        : 2;       // bits : 27_26
		} bit;
		UINT32 word;
	} ife_register_590; // 0x0938

	union {
		struct {
			unsigned ife_wdr_lpf_c0        : 3;     // bits : 2_0
			unsigned ife_wdr_lpf_c1        : 3;     // bits : 5_3
			unsigned ife_wdr_lpf_c2        : 3;     // bits : 8_6
		} bit;
		UINT32 word;
	} ife_register_591; // 0x093c

	union {
		struct {
			unsigned ife_wdr_coeff1        : 13;        // bits : 12_0
			unsigned                       : 3;
			unsigned ife_wdr_coeff2        : 13;        // bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_592; // 0x0940

	union {
		struct {
			unsigned ife_wdr_coeff3        : 13;        // bits : 12_0
			unsigned                       : 3;
			unsigned ife_wdr_coeff4        : 13;        // bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_593; // 0x0944

	union {
		struct {
			unsigned ife_wdr_outbld_table_en        : 1;        // bits : 0
			unsigned ife_wdr_gainctrl_en            : 1;        // bits : 1
			unsigned                                : 14;
			unsigned ife_wdr_maxgain                : 8;        // bits : 23_16
			unsigned ife_wdr_mingain                : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_594; // 0x0948

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l0        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_l1        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_595; // 0x094c

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l2        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_l3        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_596; // 0x0950

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l4        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_l5        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_597; // 0x0954

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l6        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_l7        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_598; // 0x0958

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l8        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_l9        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_599; // 0x095c

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l10        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l11        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_600; // 0x0960

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l12        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l13        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_601; // 0x0964

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l14        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l15        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_602; // 0x0968

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l16        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l17        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_603; // 0x096c

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l18        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l19        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_604; // 0x0970

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l20        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l21        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_605; // 0x0974

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l22        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l23        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_606; // 0x0978

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l24        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l25        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_607; // 0x097c

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l26        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l27        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_608; // 0x0980

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l28        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l29        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_609; // 0x0984

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l30        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l31        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_610; // 0x0988

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l32        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l33        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_611; // 0x098c

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l34        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l35        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_612; // 0x0990

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l36        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l37        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_613; // 0x0994

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l38        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l39        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_614; // 0x0998

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l40        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l41        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_615; // 0x099c

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l42        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l43        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_616; // 0x09a0

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l44        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l45        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_617; // 0x09a4

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l46        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l47        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_618; // 0x09a8

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l48        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l49        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_619; // 0x09ac

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l50        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l51        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_620; // 0x09b0

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l52        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l53        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_621; // 0x09b4

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l54        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l55        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_622; // 0x09b8

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l56        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l57        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_623; // 0x09bc

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l58        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l59        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_624; // 0x09c0

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l60        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l61        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_625; // 0x09c4

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l62        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_l63        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_626; // 0x09c8

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_l64        : 12;        // bits : 11_0
		} bit;
		UINT32 word;
	} ife_register_627; // 0x09cc

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_r0        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_r1        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_628; // 0x09d0

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_r2        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_r3        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_629; // 0x09d4

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_r4        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_r5        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_630; // 0x09d8

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_r6        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_r7        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_631; // 0x09dc

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_r8        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_tcurve_val_lut_r9        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_632; // 0x09e0

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_r10        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_r11        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_633; // 0x09e4

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_r12        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_r13        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_634; // 0x09e8

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_r14        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_tcurve_val_lut_r15        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_635; // 0x09ec

	union {
		struct {
			unsigned ife_wdr_tcurve_val_lut_r16        : 12;        // bits : 11_0
		} bit;
		UINT32 word;
	} ife_register_636; // 0x09f0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_637; // 0x09f4

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l0        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_l1        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_638; // 0x09f8

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l2        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_l3        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_639; // 0x09fc

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l4        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_l5        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_640; // 0x0a00

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l6        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_l7        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_641; // 0x0a04

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l8        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_l9        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_642; // 0x0a08

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l10        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l11        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_643; // 0x0a0c

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l12        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l13        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_644; // 0x0a10

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l14        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l15        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_645; // 0x0a14

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l16        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l17        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_646; // 0x0a18

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l18        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l19        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_647; // 0x0a1c

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l20        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l21        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_648; // 0x0a20

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l22        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l23        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_649; // 0x0a24

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l24        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l25        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_650; // 0x0a28

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l26        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l27        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_651; // 0x0a2c

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l28        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l29        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_652; // 0x0a30

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l30        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l31        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_653; // 0x0a34

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l32        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l33        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_654; // 0x0a38

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l34        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l35        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_655; // 0x0a3c

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l36        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l37        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_656; // 0x0a40

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l38        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l39        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_657; // 0x0a44

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l40        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l41        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_658; // 0x0a48

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l42        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l43        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_659; // 0x0a4c

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l44        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l45        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_660; // 0x0a50

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l46        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l47        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_661; // 0x0a54

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l48        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l49        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_662; // 0x0a58

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l50        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l51        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_663; // 0x0a5c

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l52        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l53        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_664; // 0x0a60

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l54        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l55        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_665; // 0x0a64

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l56        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l57        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_666; // 0x0a68

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l58        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l59        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_667; // 0x0a6c

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l60        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l61        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_668; // 0x0a70

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l62        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_l63        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_669; // 0x0a74

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_l64        : 12;        // bits : 11_0
		} bit;
		UINT32 word;
	} ife_register_670; // 0x0a78

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_r0        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_r1        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_671; // 0x0a7c

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_r2        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_r3        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_672; // 0x0a80

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_r4        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_r5        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_673; // 0x0a84

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_r6        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_r7        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_674; // 0x0a88

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_r8        : 12;     // bits : 11_0
			unsigned                                  : 4;
			unsigned ife_wdr_outbld_val_lut_r9        : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_675; // 0x0a8c

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_r10        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_r11        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_676; // 0x0a90

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_r12        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_r13        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_677; // 0x0a94

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_r14        : 12;        // bits : 11_0
			unsigned                                   : 4;
			unsigned ife_wdr_outbld_val_lut_r15        : 12;        // bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_678; // 0x0a98

	union {
		struct {
			unsigned ife_wdr_outbld_val_lut_r16        : 12;        // bits : 11_0
		} bit;
		UINT32 word;
	} ife_register_679; // 0x0a9c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_680; // 0x0aa0

	union {
		struct {
			unsigned ife_wdr_strength         : 8;      // bits : 7_0
		} bit;
		UINT32 word;
	} ife_register_681; // 0x0aa4

	union {
		struct {
			unsigned ife_wdr_sat_th             : 12;       // bits : 11_0
			unsigned                            : 4;
			unsigned ife_wdr_sat_wt_low         : 8;        // bits : 23_16
			unsigned ife_wdr_sat_delta          : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} ife_register_682; // 0x0aa8

	union {
		struct {
			unsigned ife_wdr_mode                 : 1;      // bits : 0
			unsigned ife_wdr_anti_halo_opt        : 1;      // bits : 1
			unsigned                              : 6;
			unsigned ife_wdr_halo_ratio           : 8;      // bits : 15_8
			unsigned ife_wdr_halo_slope           : 8;      // bits : 23_16
		} bit;
		UINT32 word;
	} ife_register_683; // 0x0aac

	union {
		struct {
			unsigned ife_wdr_b2p_var        : 12;       // bits : 11_0
		} bit;
		UINT32 word;
	} ife_register_684; // 0x0ab0

	union {
		struct {
			unsigned ife_wdr_histogram_h_step         : 5;      // bits : 4_0
			unsigned                                  : 11;
			unsigned ife_wdr_histogram_v_step         : 5;      // bits : 20_16
		} bit;
		UINT32 word;
	} ife_register_685; // 0x0ab4

	union {
		struct {
			unsigned ife_wdr_histogram_bin0        : 16;        // bits : 15_0
			unsigned ife_wdr_histogram_bin1        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_686; // 0x0ab8

	union {
		struct {
			unsigned ife_wdr_histogram_bin2        : 16;        // bits : 15_0
			unsigned ife_wdr_histogram_bin3        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_687; // 0x0abc

	union {
		struct {
			unsigned ife_wdr_histogram_bin4        : 16;        // bits : 15_0
			unsigned ife_wdr_histogram_bin5        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_688; // 0x0ac0

	union {
		struct {
			unsigned ife_wdr_histogram_bin6        : 16;        // bits : 15_0
			unsigned ife_wdr_histogram_bin7        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_689; // 0x0ac4

	union {
		struct {
			unsigned ife_wdr_histogram_bin8        : 16;        // bits : 15_0
			unsigned ife_wdr_histogram_bin9        : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_690; // 0x0ac8

	union {
		struct {
			unsigned ife_wdr_histogram_bin10        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin11        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_691; // 0x0acc

	union {
		struct {
			unsigned ife_wdr_histogram_bin12        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin13        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_692; // 0x0ad0

	union {
		struct {
			unsigned ife_wdr_histogram_bin14        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin15        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_693; // 0x0ad4

	union {
		struct {
			unsigned ife_wdr_histogram_bin16        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin17        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_694; // 0x0ad8

	union {
		struct {
			unsigned ife_wdr_histogram_bin18        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin19        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_695; // 0x0adc

	union {
		struct {
			unsigned ife_wdr_histogram_bin20        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin21        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_696; // 0x0ae0

	union {
		struct {
			unsigned ife_wdr_histogram_bin22        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin23        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_697; // 0x0ae4

	union {
		struct {
			unsigned ife_wdr_histogram_bin24        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin25        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_698; // 0x0ae8

	union {
		struct {
			unsigned ife_wdr_histogram_bin26        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin27        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_699; // 0x0aec

	union {
		struct {
			unsigned ife_wdr_histogram_bin28        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin29        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_700; // 0x0af0

	union {
		struct {
			unsigned ife_wdr_histogram_bin30        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin31        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_701; // 0x0af4

	union {
		struct {
			unsigned ife_wdr_histogram_bin32        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin33        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_702; // 0x0af8

	union {
		struct {
			unsigned ife_wdr_histogram_bin34        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin35        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_703; // 0x0afc

	union {
		struct {
			unsigned ife_wdr_histogram_bin36        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin37        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_704; // 0x0b00

	union {
		struct {
			unsigned ife_wdr_histogram_bin38        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin39        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_705; // 0x0b04

	union {
		struct {
			unsigned ife_wdr_histogram_bin40        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin41        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_706; // 0x0b08

	union {
		struct {
			unsigned ife_wdr_histogram_bin42        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin43        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_707; // 0x0b0c

	union {
		struct {
			unsigned ife_wdr_histogram_bin44        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin45        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_708; // 0x0b10

	union {
		struct {
			unsigned ife_wdr_histogram_bin46        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin47        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_709; // 0x0b14

	union {
		struct {
			unsigned ife_wdr_histogram_bin48        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin49        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_710; // 0x0b18

	union {
		struct {
			unsigned ife_wdr_histogram_bin50        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin51        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_711; // 0x0b1c

	union {
		struct {
			unsigned ife_wdr_histogram_bin52        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin53        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_712; // 0x0b20

	union {
		struct {
			unsigned ife_wdr_histogram_bin54        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin55        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_713; // 0x0b24

	union {
		struct {
			unsigned ife_wdr_histogram_bin56        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin57        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_714; // 0x0b28

	union {
		struct {
			unsigned ife_wdr_histogram_bin58        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin59        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_715; // 0x0b2c

	union {
		struct {
			unsigned ife_wdr_histogram_bin60        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin61        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_716; // 0x0b30

	union {
		struct {
			unsigned ife_wdr_histogram_bin62        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin63        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_717; // 0x0b34

	union {
		struct {
			unsigned ife_wdr_histogram_bin64        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin65        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_718; // 0x0b38

	union {
		struct {
			unsigned ife_wdr_histogram_bin66        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin67        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_719; // 0x0b3c

	union {
		struct {
			unsigned ife_wdr_histogram_bin68        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin69        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_720; // 0x0b40

	union {
		struct {
			unsigned ife_wdr_histogram_bin70        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin71        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_721; // 0x0b44

	union {
		struct {
			unsigned ife_wdr_histogram_bin72        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin73        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_722; // 0x0b48

	union {
		struct {
			unsigned ife_wdr_histogram_bin74        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin75        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_723; // 0x0b4c

	union {
		struct {
			unsigned ife_wdr_histogram_bin76        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin77        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_724; // 0x0b50

	union {
		struct {
			unsigned ife_wdr_histogram_bin78        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin79        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_725; // 0x0b54

	union {
		struct {
			unsigned ife_wdr_histogram_bin80        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin81        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_726; // 0x0b58

	union {
		struct {
			unsigned ife_wdr_histogram_bin82        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin83        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_727; // 0x0b5c

	union {
		struct {
			unsigned ife_wdr_histogram_bin84        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin85        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_728; // 0x0b60

	union {
		struct {
			unsigned ife_wdr_histogram_bin86        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin87        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_729; // 0x0b64

	union {
		struct {
			unsigned ife_wdr_histogram_bin88        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin89        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_730; // 0x0b68

	union {
		struct {
			unsigned ife_wdr_histogram_bin90        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin91        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_731; // 0x0b6c

	union {
		struct {
			unsigned ife_wdr_histogram_bin92        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin93        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_732; // 0x0b70

	union {
		struct {
			unsigned ife_wdr_histogram_bin94        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin95        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_733; // 0x0b74

	union {
		struct {
			unsigned ife_wdr_histogram_bin96        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin97        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_734; // 0x0b78

	union {
		struct {
			unsigned ife_wdr_histogram_bin98        : 16;       // bits : 15_0
			unsigned ife_wdr_histogram_bin99        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_735; // 0x0b7c

	union {
		struct {
			unsigned ife_wdr_histogram_bin100        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin101        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_736; // 0x0b80

	union {
		struct {
			unsigned ife_wdr_histogram_bin102        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin103        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_737; // 0x0b84

	union {
		struct {
			unsigned ife_wdr_histogram_bin104        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin105        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_738; // 0x0b88

	union {
		struct {
			unsigned ife_wdr_histogram_bin106        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin107        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_739; // 0x0b8c

	union {
		struct {
			unsigned ife_wdr_histogram_bin108        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin109        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_740; // 0x0b90

	union {
		struct {
			unsigned ife_wdr_histogram_bin110        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin111        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_741; // 0x0b94

	union {
		struct {
			unsigned ife_wdr_histogram_bin112        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin113        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_742; // 0x0b98

	union {
		struct {
			unsigned ife_wdr_histogram_bin114        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin115        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_743; // 0x0b9c

	union {
		struct {
			unsigned ife_wdr_histogram_bin116        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin117        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_744; // 0x0ba0

	union {
		struct {
			unsigned ife_wdr_histogram_bin118        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin119        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_745; // 0x0ba4

	union {
		struct {
			unsigned ife_wdr_histogram_bin120        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin121        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_746; // 0x0ba8

	union {
		struct {
			unsigned ife_wdr_histogram_bin122        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin123        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_747; // 0x0bac

	union {
		struct {
			unsigned ife_wdr_histogram_bin124        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin125        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_748; // 0x0bb0

	union {
		struct {
			unsigned ife_wdr_histogram_bin126        : 16;      // bits : 15_0
			unsigned ife_wdr_histogram_bin127        : 16;      // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_749; // 0x0bb4

	union {
		struct {
			unsigned ife_wdr_fbc_en         : 1;        // bits : 0
			unsigned                        : 7;
			unsigned ife_wdr_fbc_rto        : 8;        // bits : 15_8
			unsigned ife_wdr_fbc_th0        : 13;       // bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_750; // 0x0bb8

	union {
		struct {
			unsigned ife_wdr_fbc_th1        : 13;       // bits : 12_0
			unsigned                        : 3;
			unsigned ife_wdr_fbc_th2        : 13;       // bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_751; // 0x0bbc

	union {
		struct {
			unsigned ife_wdr_fbc_th3        : 13;       // bits : 12_0
			unsigned                        : 3;
			unsigned ife_wdr_fbc_th4        : 13;       // bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_752; // 0x0bc0

	union {
		struct {
			unsigned ife_wdr_fbc_th5        : 13;       // bits : 12_0
		} bit;
		UINT32 word;
	} ife_register_753; // 0x0bc4

	union {
		struct {
			unsigned ife_wdr_input_yv_bld_lut0        : 6;      // bits : 5_0
			unsigned                                  : 2;
			unsigned ife_wdr_input_yv_bld_lut1        : 6;      // bits : 13_8
			unsigned                                  : 2;
			unsigned ife_wdr_input_yv_bld_lut2        : 6;      // bits : 21_16
			unsigned                                  : 2;
			unsigned ife_wdr_input_yv_bld_lut3        : 6;      // bits : 29_24
		} bit;
		UINT32 word;
	} ife_register_754; // 0x0bc8

	union {
		struct {
			unsigned ife_wdr_input_yv_bld_lut4        : 6;      // bits : 5_0
			unsigned                                  : 2;
			unsigned ife_wdr_input_yv_bld_lut5        : 6;      // bits : 13_8
			unsigned                                  : 2;
			unsigned ife_wdr_input_yv_bld_lut6        : 6;      // bits : 21_16
			unsigned                                  : 2;
			unsigned ife_wdr_input_yv_bld_lut7        : 6;      // bits : 29_24
		} bit;
		UINT32 word;
	} ife_register_755; // 0x0bcc

	union {
		struct {
			unsigned ife_wdr_input_yv_bld_lut8        : 6;      // bits : 5_0
		} bit;
		UINT32 word;
	} ife_register_756; // 0x0bd0

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_757; // 0x0bd4

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_758; // 0x0bd8

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_759; // 0x0bdc

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_760; // 0x0be0

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_761; // 0x0be4

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_762; // 0x0be8

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_763; // 0x0bec

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_764; // 0x0bf0

	union {
		struct {
			unsigned vdetgh2_iir2_a   : 10;       // bits : 9_0
			unsigned vdetgh2_iir2_b   : 10;       // bits : 19_10
			unsigned vdetgh2_iir2_e   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} ife_register_765; // 0x0bf4

	union {
		struct {
			unsigned vdetgh2_iir2_f   : 10;       // bits : 9_0
			unsigned vdetgh2_iir3_a   : 10;       // bits : 19_10
			unsigned vdetgh2_iir3_b   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} ife_register_766; // 0x0bf8

	union {
		struct {
			unsigned vdetgh2_iir3_e   : 10;       // bits : 9_0
			unsigned vdetgh2_iir3_f   : 10;       // bits : 19_10
		} bit;
		UINT32 word;
	} ife_register_767; // 0x0bfc

	union {
		struct {
			unsigned vacc_en                   : 1;     // bits : 0
			unsigned win0_vaen                 : 1;     // bits : 1
			unsigned win1_vaen                 : 1;     // bits : 2
			unsigned win2_vaen                 : 1;     // bits : 3
			unsigned win3_vaen                 : 1;     // bits : 4
			unsigned win4_vaen                 : 1;     // bits : 5
			unsigned va_pre_filter_mode        : 2;     // bits : 7_ 6
			unsigned va_gamma_sel              : 1;     // bits : 8
			unsigned va_win_ldg_en             : 1;     // bits : 9
			unsigned                           : 21;    // bits : 30_10
			unsigned va_force_write_en         : 1;     // bits : 31
		} bit;
		UINT32 word;
	} ife_register_768; // 0x0c00

	union {
		struct {
			unsigned                    : 2;
			unsigned dram_sao_va        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} ife_register_769; // 0x0c04

	union {
		struct {
			unsigned dram_sao_va_msb        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} ife_register_770; // 0x0c08

	union {
		struct {
			unsigned                         : 2;
			unsigned dram_ofso_va            : 14;      // bits : 15_2
		} bit;
		UINT32 word;
	} ife_register_771; // 0x0c0c

	union {
		struct {
			unsigned va_win_cnt_out_sel         : 1;        // bits : 0
			unsigned                            : 3;
			unsigned va_win_high_luma_th        : 8;        // bits : 11_4
			unsigned va_energy_w                : 5;        // bits : 16_12
		} bit;
		UINT32 word;
	} ife_register_772; // 0x0c10

	union {
		struct {
			unsigned vdetgh1_filter_select : 1;       // bits : 0
			unsigned vdetgh1_iir_input_sel  : 1;    // bits : 1
			unsigned                        : 1;    //
			unsigned vdetgh1_iir_shift_bit : 4;     // bits : 6_3
			unsigned vdetgh1_bcd_op        : 1;       // bits : 7
			unsigned vdetgh1_iir2_shift_bit: 4;     // bits : 11_8
			unsigned vdetgh1_iir2_bcd_op   : 1;       // bits : 12
			unsigned vdetgh1_iir3_shift_bit: 4;     // bits : 16_13
			unsigned vdetgh1_iir3_bcd_op   : 1;       // bits : 17
			unsigned vdetgh1_iir2_en       : 1;       // bits : 18
			unsigned vdetgh1_iir3_en       : 1;       // bits : 19
			unsigned                       : 4;
			unsigned vdetgh1_fsize         : 2;     // bits : 25_24
			unsigned                       : 2;
			unsigned vdetgh1_div           : 4;     // bits : 31_28
		} bit;
		UINT32 word;
	} ife_register_773; // 0x0c14

	union {
		struct {
			unsigned vdetgv1a              : 5;     // bits : 4_0
			unsigned                       : 2;
			unsigned vdetgv1_bcd_op        : 1;     // bits : 7
			unsigned vdetgv1b              : 5;     // bits : 12_8
			unsigned                       : 3;
			unsigned vdetgv1c              : 4;     // bits : 19_16
			unsigned vdetgv1d              : 4;     // bits : 23_20
			unsigned vdetgv1_fsize         : 2;     // bits : 25_24
			unsigned                       : 2;
			unsigned vdetgv1_div           : 4;     // bits : 31_28
		} bit;
		UINT32 word;
	} ife_register_774; // 0x0c18

	union {
		struct {
			unsigned vdetgh2_filter_select : 1;     // bits : 0
			unsigned vdetgh2_iir_input_sel  : 1;
			unsigned                        : 1;    //
			unsigned vdetgh2_iir_shift_bit : 4;     // bits : 6_3
			unsigned vdetgh2_bcd_op        : 1;     // bits : 7
			unsigned vdetgh2_iir2_shift_bit: 4;     // bits : 11_8
			unsigned vdetgh2_iir2_bcd_op   : 1;       // bits : 12
			unsigned vdetgh2_iir3_shift_bit: 4;     // bits : 16_13
			unsigned vdetgh2_iir3_bcd_op   : 1;       // bits : 17
			unsigned vdetgh2_iir2_en       : 1;       // bits : 18
			unsigned vdetgh2_iir3_en       : 1;       // bits : 19
			unsigned                       : 4;
			unsigned vdetgh2_fsize         : 2;     // bits : 25_24
			unsigned                       : 2;
			unsigned vdetgh2_div           : 4;     // bits : 31_28
		} bit;
		UINT32 word;
	} ife_register_775; // 0x0c1c

	union {
		struct {
			unsigned vdetgv2a              : 5;     // bits : 4_0
			unsigned                       : 2;
			unsigned vdetgv2_bcd_op        : 1;     // bits : 7
			unsigned vdetgv2b              : 5;     // bits : 12_8
			unsigned                       : 3;
			unsigned vdetgv2c              : 4;     // bits : 19_16
			unsigned vdetgv2d              : 4;     // bits : 23_20
			unsigned vdetgv2_fsize         : 2;     // bits : 25_24
			unsigned                       : 2;
			unsigned vdetgv2_div           : 4;     // bits : 31_28
		} bit;
		UINT32 word;
	} ife_register_776; // 0x0c20

	union {
		struct {
			unsigned vacc_outsel        : 1;        // bits : 0
			unsigned                    : 2;
			unsigned va_stx             : 13;	// bits : 15_3
			unsigned va_sty             : 13;	// bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_777; // 0x0c24

	union {
		struct {
			unsigned va_g1hthl          : 12;		// bits : 11_0
			unsigned                    : 4;			
			unsigned va_g1hthh          : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_778; // 0x0c28

	union {
		struct {
			unsigned va_g1vthl          : 12;		// bits : 11_0
			unsigned                    : 4;			
			unsigned va_g1vthh          : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_779; // 0x0c2c

	union {
		struct {
			unsigned va_win_szx          : 11;		// bits : 10_0
			unsigned                     : 1;
			unsigned va_win_szy          : 11;		// bits : 22_12
			unsigned                     : 7;
			unsigned va_g1_cnt_en        : 1;		// bits : 30
			unsigned va_g2_cnt_en        : 1;		// bits : 31
		} bit;
		UINT32 word;
	} ife_register_780; // 0x0c30

	union {
		struct {
			unsigned va_win_numx         : 3;		// bits : 2_0
			unsigned                     : 1;
			unsigned va_win_numy         : 3;		// bits : 6_4
			unsigned                     : 9;
			unsigned va_win_skipx        : 6;		// bits : 21_16
			unsigned                     : 2;
			unsigned va_win_skipy        : 6;		// bits : 29_24
		} bit;
		UINT32 word;
	} ife_register_781; // 0x0c34

	union {
		struct {
			unsigned win0_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win0_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_782; // 0x0c38

	union {
		struct {
			unsigned win0_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win0_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} ife_register_783; // 0x0c3c

	union {
		struct {
			unsigned win1_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win1_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_784; // 0x0c40

	union {
		struct {
			unsigned win1_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win1_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} ife_register_785; // 0x0c44

	union {
		struct {
			unsigned win2_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win2_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_786; // 0x0c48

	union {
		struct {
			unsigned win2_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win2_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} ife_register_787; // 0x0c4c

	union {
		struct {
			unsigned win3_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win3_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_788; // 0x0c50

	union {
		struct {
			unsigned win3_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win3_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} ife_register_789; // 0x0c54

	union {
		struct {
			unsigned win4_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win4_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} ife_register_790; // 0x0c58

	union {
		struct {
			unsigned win4_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win4_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} ife_register_791; // 0x0c5c

	union {
		struct {
			unsigned va_win0_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_792; // 0x0c60

	union {
		struct {
			unsigned va_win0g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win0g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_793; // 0x0c64

	union {
		struct {
			unsigned va_win0g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win0g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_794; // 0x0c68

	union {
		struct {
			unsigned va_win0g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win0g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_795; // 0x0c6c

	union {
		struct {
			unsigned va_win0g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win0g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_796; // 0x0c70

	union {
		struct {
			unsigned va_win1_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_797; // 0x0c74

	union {
		struct {
			unsigned va_win1g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win1g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_798; // 0x0c78

	union {
		struct {
			unsigned va_win1g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win1g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_799; // 0x0c7c

	union {
		struct {
			unsigned va_win1g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win1g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_800; // 0x0c80

	union {
		struct {
			unsigned va_win1g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win1g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_801; // 0x0c84

	union {
		struct {
			unsigned va_win2_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_802; // 0x0c88

	union {
		struct {
			unsigned va_win2g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win2g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_803; // 0x0c8c

	union {
		struct {
			unsigned va_win2g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win2g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_804; // 0x0c90

	union {
		struct {
			unsigned va_win2g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win2g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_805; // 0x0c94

	union {
		struct {
			unsigned va_win2g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win2g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_806; // 0x0c98

	union {
		struct {
			unsigned va_win3_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_807; // 0x0c9c

	union {
		struct {
			unsigned va_win3g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win3g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_808; // 0x0ca0

	union {
		struct {
			unsigned va_win3g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win3g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_809; // 0x0ca4

	union {
		struct {
			unsigned va_win3g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win3g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_810; // 0x0ca8

	union {
		struct {
			unsigned va_win3g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win3g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_811; // 0x0cac

	union {
		struct {
			unsigned va_win4_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_812; // 0x0cb0

	union {
		struct {
			unsigned va_win4g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win4g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_813; // 0x0cb4

	union {
		struct {
			unsigned va_win4g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win4g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_814; // 0x0cb8

	union {
		struct {
			unsigned va_win4g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win4g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_815; // 0x0cbc

	union {
		struct {
			unsigned va_win4g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win4g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_816; // 0x0cc0

	union {
		struct {
			unsigned va_g2hthl        : 12;		// bits : 11_0
			unsigned                  :  4;			
			unsigned va_g2hthh        : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_817; // 0x0cc4

	union {
		struct {
			unsigned va_g2vthl        : 12;		// bits : 11_0
			unsigned                  :  4;
			unsigned va_g2vthh        : 12;		// bits : 27_16			
		} bit;
		UINT32 word;
	} ife_register_818; // 0x0cc8

	union {
		struct {
		} bit;
		UINT32 word;
	} ife_register_819; // 0x0ccc

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_820; // 0x0cd0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_821; // 0x0cd4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_822; // 0x0cd8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_823; // 0x0cdc

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_824; // 0x0ce0

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_825; // 0x0ce4

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_826; // 0x0ce8

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_827; // 0x0cec

	union {
		struct {
			unsigned                              : 4;
			unsigned va_win_ldg_low_slope         : 4;		// bits : 7_4
			unsigned va_win_ldg_high_slope        : 4;		// bits : 11_8		
		} bit;
		UINT32 word;
	} ife_register_828; // 0x0cf0

	union {
		struct {
			unsigned va_win_ldg_low_th           : 8;		// bits : 7_0
			unsigned va_win_ldg_high_th          : 8;		// bits : 15_8
			unsigned va_win_ldg_low_gain         : 8;		// bits : 23_16
			unsigned va_win_ldg_high_gain        : 8;		// bits : 31_24		
		} bit;
		UINT32 word;
	} ife_register_829; // 0x0cf4

	union {
		struct {
			unsigned va_gamma_0         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_1         : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_830; // 0x0cf8

	union {
		struct {
			unsigned va_gamma_2         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_3         : 12;		// bits : 27_16		
		} bit;
		UINT32 word;
	} ife_register_831; // 0x0cfc

	union {
		struct {
			unsigned va_gamma_4         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_5         : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_832; // 0x0d00

	union {
		struct {
			unsigned va_gamma_6         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_7         : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_833; // 0x0d04

	union {
		struct {
			unsigned va_gamma_8         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_9         : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_834; // 0x0d08

	union {
		struct {
			unsigned va_gamma_10        : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_11        : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_835; // 0x0d0c

	union {
		struct {
			unsigned va_gamma_12        : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_13        : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_836; // 0x0d10

	union {
		struct {
			unsigned va_gamma_14        : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_15        : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} ife_register_837; // 0x0d14

	union {
		struct {
			unsigned va_gamma_16        : 12;		// bits : 11_0
		} bit;
		UINT32 word;
	} ife_register_838; // 0x0d18

	union {
		struct {
			unsigned vdetgh1a         : 10;       // bits : 9_0
			unsigned vdetgh1b         : 10;       // bits : 19_10
			unsigned vdetgh1c         : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} ife_register_839; // 0x0d1c

	union {
		struct {
			unsigned vdetgh1d         : 10;       // bits : 9_0
			unsigned vdetgh1_iir1_e   : 10;       // bits : 19_10
			unsigned vdetgh1_iir1_f   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} ife_register_840; // 0x0d20

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_841; // 0x0d24

	union {
		struct {
			unsigned vdetgh2a         : 10;       // bits : 9_0
			unsigned vdetgh2b         : 10;       // bits : 19_10
			unsigned vdetgh2c         : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} ife_register_842; // 0x0d28

	union {
		struct {
			unsigned vdetgh2d         : 10;       // bits : 9_0
			unsigned vdetgh2_iir1_e   : 10;       // bits : 19_10
			unsigned vdetgh2_iir1_f   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} ife_register_843; // 0x0d2c

	union {
		struct {

		} bit;
		UINT32 word;
	} ife_register_844; // 0x0d30

	union {
		struct {
			unsigned vdetgh1_iir2_a   : 10;       // bits : 9_0
			unsigned vdetgh1_iir2_b   : 10;       // bits : 19_10
			unsigned vdetgh1_iir2_e   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} ife_register_845; // 0x0d34

	union {
		struct {
			unsigned vdetgh1_iir2_f   : 10;       // bits : 9_0
			unsigned vdetgh1_iir3_a   : 10;       // bits : 19_10
			unsigned vdetgh1_iir3_b   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} ife_register_846; // 0x0d38

	union {
		struct {
			unsigned vdetgh1_iir3_e   : 10;       // bits : 9_0
			unsigned vdetgh1_iir3_f   : 10;       // bits : 19_10
		} bit;
		UINT32 word;
	} ife_register_847; // 0x0d3c

	union {
		struct {
			unsigned ife_nn_isp_p0_en                     : 1;        // bits : 0
			unsigned ife_nn_isp_p1_en                     : 1;        // bits : 1
			unsigned ife_nn_isp_p0_handshake_mode         : 1;        // bits : 2
			unsigned ife_nn_isp_p1_handshake_mode         : 1;        // bits : 3
			unsigned ife_nn_isp_p0_ringbuf_slice_clear    : 1;      // bits : 4
			unsigned ife_nn_isp_p0_outbuf_slice_ready     : 1;      // bits : 5
			unsigned ife_nn_isp_p1_ringbuf_slice_clear    : 1;      // bits : 6
			unsigned ife_nn_isp_p1_outbuf_slice_ready     : 1;      // bits : 7
			unsigned ife_nn_isp_p0_input_burst_mode       : 1;        // bits : 8
			unsigned ife_nn_isp_p0_output_burst_mode      : 1;        // bits : 9
			unsigned ife_nn_isp_p1_input_burst_mode       : 1;        // bits : 10
			unsigned ife_nn_isp_p1_output_burst_mode      : 1;        // bits : 11
		} bit;
		UINT32 word;
	} ife_register_848; // 0x0d40

	union {
		struct {
			unsigned                              : 1;
			unsigned nn_isp_p0_slice_height       : 15;       // bits : 15_1
			unsigned nn_isp_p0_ringbuf_height     : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_849; // 0x0d44

	union {
		struct {
			unsigned nn_isp_p0_slice_ovlp         : 10;       // bits : 9_0
			unsigned        : 6;
			unsigned nn_isp_p0_outbuf_height     : 16;        // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_850; // 0x0d48

	union {
		struct {
			unsigned ife_nn_isp_p0_set_slice_ready_status         : 1;        // bits : 0
			unsigned ife_nn_isp_p0_get_slice_clear_status         : 1;        // bits : 1
			unsigned ife_nn_isp_p1_set_slice_ready_status         : 1;        // bits : 2
			unsigned ife_nn_isp_p1_get_slice_clear_status         : 1;        // bits : 3
		} bit;
		UINT32 word;
	} ife_register_851; // 0x0d4c

	union {
		struct {
			unsigned nn_isp_p0_sai              :  32;		// bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_852; // 0x0d50

	union {
		struct {
			unsigned                      : 2;
			unsigned nn_isp_p0_ofsi       : 14;       // bits : 15_2
			unsigned                      : 8;
			unsigned nn_isp_p0_sai_msb   : 4;	// bits : 27_24
		} bit;
		UINT32 word;
	} ife_register_853; // 0x0d54

	union {
		struct {
			unsigned nn_isp_p0_sao        : 32;       // bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_854; // 0x0d58

	union {
		struct {
			unsigned                      : 2;
			unsigned nn_isp_p0_ofso       : 14;       // bits : 15_2
			unsigned                      : 8;
			unsigned nn_isp_p0_sao_msb   : 4;        // bits : 27_24
		} bit;
		UINT32 word;
	} ife_register_855; // 0x0d5c

	union {
		struct {
			unsigned                              : 1;
			unsigned nn_isp_p1_slice_height       : 15;       // bits : 15_1
			unsigned nn_isp_p1_ringbuf_height     : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_856; // 0x0d60

	union {
		struct {
			unsigned nn_isp_p1_slice_ovlp     : 10;       // bits : 9_0
			unsigned                          : 6;
			unsigned nn_isp_p1_outbuf_height  : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} ife_register_857; // 0x0d64

	union {
		struct {
			unsigned      : 32;
		} bit;
		UINT32 word;
	} ife_register_858; // 0x0d68

	union {
		struct {
			unsigned nn_isp_p1_sai     : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_859; // 0x0d6c

	union {
		struct {
			unsigned                      : 2;
			unsigned nn_isp_p1_ofsi       : 14;       // bits : 15_2
			unsigned                      : 8;
			unsigned nn_isp_p1_sai_msb   : 4;        // bits : 27_24
		} bit;
		UINT32 word;
	} ife_register_860; // 0x0d70

	union {
		struct {
			unsigned nn_isp_p1_sao                     : 32;      // bits : 31_0
		} bit;
		UINT32 word;
	} ife_register_861; // 0x0d74

	union {
		struct {
			unsigned                          : 2;
			unsigned nn_isp_p1_ofso           : 14;       // bits : 15_2
			unsigned                          : 8;
			unsigned nn_isp_p1_sao_msb       : 4;        // bits : 27_24
		} bit;
		UINT32 word;
	} ife_register_862; // 0x0d78

	union {
		struct {
			unsigned                          : 31;
			unsigned ife_nn_isp_p0_ringbuf_slice_clear       : 1;     // bits : 31
		} bit;
		UINT32 word;
	} ife_register_863; // 0x0d7c

	union {
		struct {
			unsigned                          : 31;
			unsigned ife_nn_isp_p0_outbuf_slice_ready       : 1;      // bits : 31
		} bit;
		UINT32 word;
	} ife_register_864; // 0x0d80

	union {
		struct {
			unsigned                          : 31;
			unsigned ife_nn_isp_p1_ringbuf_slice_clear       : 1;     // bits : 31
		} bit;
		UINT32 word;
	} ife_register_865; // 0x0d84

	union {
		struct {
			unsigned                          : 31;
			unsigned ife_nn_isp_p1_outbuf_slice_ready       : 1;      // bits : 31
		} bit;
		UINT32 word;
	} ife_register_866; // 0x0d88


} NT98538_IFE_REGISTER_STRUCT;


/*
#define IFE_REG_ADDR(ofs)        (_IFE_REG_BASE_ADDR+(ofs))
#define IFE_SETREG(ofs, value)   OUTW((_IFE_REG_BASE_ADDR + ofs), value)
#define IFE_GETREG(ofs)          INW(_IFE_REG_BASE_ADDR + ofs)
*/
	/*
		ife_sw_rst		 :	  [0x0, 0x1],			bits : 0
		ife_start		 :	  [0x0, 0x1],			bits : 1
		ife_load_start	 :	  [0x0, 0x1],			bits : 2
		ife_load_fd 	 :	  [0x0, 0x1],			bits : 3
		ife_load_frmstart:	  [0x0, 0x1],			bits : 4
		ife_ll_fire 	 :	  [0x0, 0x1],			bits : 28
		ife_ll_terminate :	  [0x0, 0x1],			bits : 29
	*/
#define FILTER_OPERATION_CONTROL_REGISTER_OFS 0x0000
	REGDEF_BEGIN(FILTER_OPERATION_CONTROL_REGISTER)
	REGDEF_BIT(ife_sw_rst,		  1)
	REGDEF_BIT(ife_start,		 1)
	REGDEF_BIT(ife_load_start,		  1)
	REGDEF_BIT(ife_load_fd, 	   1)
	REGDEF_BIT(ife_load_frmstart,		 1)
	REGDEF_BIT(ife_global_load_en,		  1)
	REGDEF_BIT(,		22)
	REGDEF_BIT(ife_ll_fire, 	   1)
	REGDEF_END(FILTER_OPERATION_CONTROL_REGISTER)
	
	
	/*
		IFE_MODE		   :	[0x0, 0x3], 		bits : 1_0
		INBIT_16_FMT_SEL   :	[0x0, 0x1], 		bits : 4
		INBIT_DEPTH 	   :	[0x0, 0x3], 		bits : 6_5
		OUTBIT_DEPTH	   :	[0x0, 0x3], 		bits : 8_7
		CFAPAT			   :	[0x0, 0x7], 		bits : 11_9
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
	REGDEF_BIT(ife_mode,		2)
	REGDEF_BIT(ife_input_format   , 	   1)
	REGDEF_BIT(ife_subout_sel	  , 	   1)
	REGDEF_BIT(inbit_16_fmt_sel,	1)
	REGDEF_BIT(inbit_depth, 	   2)
	REGDEF_BIT(outbit_depth,		2)
	REGDEF_BIT(cfapat,		  3)
	REGDEF_BIT(,		 1)
	REGDEF_BIT(outl_en, 	   1)
	REGDEF_BIT(filter_en,		 1)
	REGDEF_BIT(cgain_en,		1)
	REGDEF_BIT(vig_en,		  1)
	REGDEF_BIT(gbal_en, 	   1)
	REGDEF_BIT(binning, 	   3)
	REGDEF_BIT(bayer_fmt,		 1)
	REGDEF_BIT(rgbir_rb_nrfill, 	   1)
	REGDEF_BIT(bilat_th_en, 	   1)
	REGDEF_BIT(dgain_en,		1)
	REGDEF_BIT(f_cg_en, 	   1)
	REGDEF_BIT(f_fusion_en, 	   1)
	REGDEF_BIT(f_fusion_fnum,		 2)
	REGDEF_BIT(f_fc_en, 	   1)
	REGDEF_BIT(mirror_en,		 1)
	REGDEF_BIT(ife_thermal_mode, 	   1)
	REGDEF_END(CONTROL_REGISTER)
	
	
	/*
		inte_frmend 		 :	  [0x0, 0x1],			bits : 0
		inte_ife_r_dec_err1  :	  [0x0, 0x1],			bits : 1
		inte_ife_r_dec_err2  :	  [0x0, 0x1],			bits : 2
		inte_llend			 :	  [0x0, 0x1],			bits : 3
		inte_llerror		 :	  [0x0, 0x1],			bits : 4
		inte_llerror2		 :	  [0x0, 0x1],			bits : 5
		inte_lljobend		 :	  [0x0, 0x1],			bits : 6
		inte_ife_bufovfl	 :	  [0x0, 0x1],			bits : 7
		inte_ife_ringbuf_err1:	  [0x0, 0x1],			bits : 8
		inte_ife_frame_err	 :	  [0x0, 0x1],			bits : 9
		inte_sie_frame_start :	  [0x0, 0x1],			bits : 12
		inte_sie2_frame_start:    [0x0, 0x1],			bits : 13
		inte_ife_frame_start :	  [0x0, 0x1],			bits : 14
	    inte_ife_nn_isp_p0_set_slice_ready:    [0x0, 0x1],			bits : 16
	    inte_ife_nn_isp_p0_get_slice_clear:    [0x0, 0x1],			bits : 17
	    inte_ife_nn_isp_p1_set_slice_ready:    [0x0, 0x1],			bits : 18
	    inte_ife_nn_isp_p1_get_slice_clear:    [0x0, 0x1],			bits : 19		
	*/
#define FILTER_INTERRUPT_ENABLE_REGISTER_OFS 0x0008
	REGDEF_BEGIN(FILTER_INTERRUPT_ENABLE_REGISTER)
	REGDEF_BIT(inte_frmend, 			1)
	REGDEF_BIT(inte_ife_r_dec_err1,    1)
	REGDEF_BIT(inte_ife_r_dec_err2,    1)
	REGDEF_BIT(inte_llend,				1)
	REGDEF_BIT(inte_llerror,			1)
	REGDEF_BIT(inte_llerror2,			1)
	REGDEF_BIT(inte_lljobend,			1)
	REGDEF_BIT(inte_ife_bufovfl,		1)
	REGDEF_BIT(inte_ife_ringbuf_err1,		 1)
	REGDEF_BIT(inte_ife_frame_err,	 1)
	REGDEF_BIT(                  ,      2)
	REGDEF_BIT(inte_sie_frame_start,	1)
	REGDEF_BIT(inte_sie2_frame_start,   1)
	REGDEF_BIT(inte_ife_frame_start,	1)
	REGDEF_BIT(                                  ,        1)
	REGDEF_BIT(inte_ife_nn_isp_p0_set_slice_ready,        1)
	REGDEF_BIT(inte_ife_nn_isp_p0_get_slice_clear,        1)
	REGDEF_BIT(inte_ife_nn_isp_p1_set_slice_ready,        1)
	REGDEF_BIT(inte_ife_nn_isp_p1_get_slice_clear,        1)	
	REGDEF_END(FILTER_INTERRUPT_ENABLE_REGISTER)
	
	
	/*
		int_frmend			:	 [0x0, 0x1],			bits : 0
		int_ife_r_dec_err1	:	 [0x0, 0x1],			bits : 1
		int_ife_r_dec_err2	:	 [0x0, 0x1],			bits : 2
		int_llend			:	 [0x0, 0x1],			bits : 3
		int_llerror 		:	 [0x0, 0x1],			bits : 4
		int_llerror2		:	 [0x0, 0x1],			bits : 5
		int_lljobend		:	 [0x0, 0x1],			bits : 6
		int_ife_bufovfl 	:	 [0x0, 0x1],			bits : 7
		int_ife_ringbuf_err1:	 [0x0, 0x1],			bits : 8
		int_ife_frame_err	:	 [0x0, 0x1],			bits : 9
		int_ife_r_dec_err3	:	 [0x0, 0x1],			bits : 10
		int_ife_ringbuf_err2:	 [0x0, 0x1],			bits : 11
		int_sie_frame_start:	 [0x0, 0x1],			bits : 12
		int_sie2_frame_start:    [0x0, 0x1],			bits : 13
		int_ife_frame_start:	 [0x0, 0x1],			bits : 14
	    int_ife_nn_isp_p0_set_slice_ready:    [0x0, 0x1],			bits : 16
	    int_ife_nn_isp_p0_get_slice_clear:    [0x0, 0x1],			bits : 17
	    int_ife_nn_isp_p1_set_slice_ready:    [0x0, 0x1],			bits : 18
	    int_ife_nn_isp_p1_get_slice_clear:    [0x0, 0x1],			bits : 19			
	*/
#define FILTER_INTERRUPT_STATUS_REGISTER_OFS 0x000c
	REGDEF_BEGIN(FILTER_INTERRUPT_STATUS_REGISTER)
	REGDEF_BIT(int_frmend,			1)
	REGDEF_BIT(int_ife_r_dec_err1, 1)
	REGDEF_BIT(int_ife_r_dec_err2, 1)
	REGDEF_BIT(int_llend,				1)
	REGDEF_BIT(int_llerror, 		1)
	REGDEF_BIT(int_llerror2,			1)
	REGDEF_BIT(int_lljobend,			1)
	REGDEF_BIT(int_ife_bufovfl, 	1)
	REGDEF_BIT(int_ife_ringbuf_err1,		1)
	REGDEF_BIT(int_ife_frame_err,	1)
	REGDEF_BIT(                    ,    2)
	REGDEF_BIT(int_sie_frame_start, 1)
	REGDEF_BIT(int_sie2_frame_start,   1)
	REGDEF_BIT(int_ife_frame_start, 1)
	REGDEF_BIT(                                 ,        1)
	REGDEF_BIT(int_ife_nn_isp_p0_set_slice_ready,        1)
	REGDEF_BIT(int_ife_nn_isp_p0_get_slice_clear,        1)
	REGDEF_BIT(int_ife_nn_isp_p1_set_slice_ready,        1)
	REGDEF_BIT(int_ife_nn_isp_p1_get_slice_clear,        1)		
	REGDEF_END(FILTER_INTERRUPT_STATUS_REGISTER)
	
	
	/*
		ife_busy	   :	[0x0, 0x1], 		bits : 0
		reserved	   :	[0x0, 0xf], 		bits : 7_4
		ife_checksum_en:	[0x0, 0x1], 		bits : 21
		sram_ls_en	   :	[0x0, 0x3ff],		bits : 31_22
	*/
#define DEBUG_REGISTER_OFS 0x0010
	REGDEF_BEGIN(DEBUG_REGISTER)
	REGDEF_BIT(ife_busy,		1)
	REGDEF_BIT(,	   19)
	REGDEF_BIT(ife_frmstr_rst,	1)
	REGDEF_BIT(ife_checksum_en, 	   1)
	REGDEF_BIT(sram_ls_en,		  10)
	REGDEF_END(DEBUG_REGISTER)
	
	
	/*
		dram_saill:    [0x0, 0x3fffffff],			bits : 31_2
	*/
#define SOURCE_ADDRESS_REGISTER_3_OFS 0x0014
	REGDEF_BEGIN(SOURCE_ADDRESS_REGISTER_3)
	REGDEF_BIT(,		2)
	REGDEF_BIT(dram_saill,		  30)
	REGDEF_END(SOURCE_ADDRESS_REGISTER_3)
	
	/*
		cfapat_2       :    [0x0, 0x7],			bits : 2_0
		dram_saill_msb:    [0x0, 0x3fffffff],			bits : 27_24
	*/
#define SOURCE_ADDRESS_REGISTER_4_OFS 0x0018
	REGDEF_BEGIN(SOURCE_ADDRESS_REGISTER_4)
	REGDEF_BIT(cfapat_2      ,        3)
	REGDEF_BIT(              ,        21)
	REGDEF_BIT(dram_saill_msb,		  4)
	REGDEF_END(SOURCE_ADDRESS_REGISTER_4)
	
	
	/*
		ife_dmach_idle :	[0x0, 0x1], 		 bits : 0
		ife_dmach_dis :    [0x0, 0x1],			 bits : 1
	*/
#define IFE_DMA_DISABLE_REGISTER_OFS 0x001c
	REGDEF_BEGIN(IFE_DMA_DISABLE_REGISTER)
	REGDEF_BIT(ife_dmach_idle,		  1)
	REGDEF_BIT(ife_dmach_dis,		  1)
	REGDEF_BIT(ife_dma_output_en,		 1)
	REGDEF_BIT(,		  5)
	REGDEF_BIT(ife_dich_line_buf_ctrl,		  2)
	REGDEF_BIT(,		  6)
	REGDEF_BIT(ife_dma_sync_dich_line_buf_disable,		  1)
	REGDEF_BIT(ife_dma1_wait_sie2_start_disable,		  1)
	REGDEF_END(IFE_DMA_DISABLE_REGISTER)
	
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
	
	
	/*
		crop_width :	[0x0, 0x3fff],			bits : 15_2
		crop_height:	[0x0, 0x7fff],			bits : 31_17
	*/
#define SOURCE_SIZE_REGISTER_1_OFS 0x0024
	REGDEF_BEGIN(SOURCE_SIZE_REGISTER_1)
	REGDEF_BIT(,		2)
	REGDEF_BIT(crop_width,		  14)
	REGDEF_BIT(,		1)
	REGDEF_BIT(crop_height, 	   15)
	REGDEF_END(SOURCE_SIZE_REGISTER_1)
	
	
	/*
		crop_hpos:	  [0x0, 0xffff],			bits : 15_0
		crop_vpos:	  [0x0, 0xffff],			bits : 31_16
	*/
#define SOURCE_SIZE_REGISTER_2_OFS 0x0028
	REGDEF_BEGIN(SOURCE_SIZE_REGISTER_2)
	REGDEF_BIT(crop_hpos,		 16)
	REGDEF_BIT(crop_vpos,		 16)
	REGDEF_END(SOURCE_SIZE_REGISTER_2)
	
	/*
		ife_ll_terminate:	 [0x0, 0x1],			bits : 0
	*/
#define LINK_LIST_CONTROL_REGISTER_OFS 0x002C
	REGDEF_BEGIN(LINK_LIST_CONTROL_REGISTER)
	REGDEF_BIT(ife_ll_terminate,		1)
	REGDEF_END(LINK_LIST_CONTROL_REGISTER)
	
	/*
		dram_sai0:	  [0x0, 0x3fffffff],			bits : 31_2
	*/
#define SOURCE_ADDRESS_REGISTER_0_OFS 0x0030
	REGDEF_BEGIN(SOURCE_ADDRESS_REGISTER_0)
	REGDEF_BIT(,		2)
	REGDEF_BIT(dram_sai0,		 30)
	REGDEF_END(SOURCE_ADDRESS_REGISTER_0)
	
	
	/*
		dram_ofsi0	  :    [0x0, 0x3fff],			bits : 15_2
		dram_sai0_msb:    [0x0, 0xf],			bits : 27_24
	*/
#define SOURCE_LINE_OFFSET_REGISTER_0_OFS 0x0034
	REGDEF_BEGIN(SOURCE_LINE_OFFSET_REGISTER_0)
	REGDEF_BIT(,		2)
	REGDEF_BIT(dram_ofsi0   ,		 14)
	REGDEF_BIT( 			,		  8)
	REGDEF_BIT(dram_sai0_msb,		  4)
	REGDEF_END(SOURCE_LINE_OFFSET_REGISTER_0)
	
	
	/*
		dram_sai1:	  [0x0, 0x3fffffff],			bits : 31_2
	*/
#define SOURCE_ADDRESS_REGISTER_1_OFS 0x0038
	REGDEF_BEGIN(SOURCE_ADDRESS_REGISTER_1)
	REGDEF_BIT(,		2)
	REGDEF_BIT(dram_sai1,		 30)
	REGDEF_END(SOURCE_ADDRESS_REGISTER_1)
	
	
	/*
		dram_ofsi1:    [0x0, 0x3fff],			bits : 15_2
	*/
#define SOURCE_LINE_OFFSET_REGISTER_1_OFS 0x003c
	REGDEF_BEGIN(SOURCE_LINE_OFFSET_REGISTER_1)
		REGDEF_BIT( 			 ,		  2)
		REGDEF_BIT(dram_ofsi1	 ,		  14)
		REGDEF_BIT( 			 ,		  8)
		REGDEF_BIT(dram_sai1_msb,		  4)
	REGDEF_END(SOURCE_LINE_OFFSET_REGISTER_1)
	
	/*
		reserved:	 [0x0, 0xffffffff], 		bits : 31_0
	*/
#define RESERVE1_OFS 0x0040
	REGDEF_BEGIN(RESERVE1_OFS)
		REGDEF_BIT(reserved,		32)
	REGDEF_END(RESERVE1_OFS)

	
	
		/*
			reserved:	 [0x0, 0xffffffff], 		bits : 31_0
		*/
#define RESERVE2_OFS 0x0044
		REGDEF_BEGIN(RESERVE2_OFS)
			REGDEF_BIT(reserved,		32)
		REGDEF_END(RESERVE2_OFS)

	
	/*
		dram_sao:	 [0x0, 0x3fffffff], 		bits : 31_2
	*/
#define DESTINATION_ADDRESS_REGISTER_OFS 0x0048
	REGDEF_BEGIN(DESTINATION_ADDRESS_REGISTER)
	REGDEF_BIT(,		2)
	REGDEF_BIT(dram_sao,		30)
	REGDEF_END(DESTINATION_ADDRESS_REGISTER)
	
	
	/*
		dram_ofso	 :	  [0x0, 0x3fff],			bits : 15_2
		dram_sao_msb:	  [0x0, 0xf],			bits : 27_24
	*/
#define DESTINATION_LINE_OFFSET_REGISTER_OFS 0x004c
	REGDEF_BEGIN(DESTINATION_LINE_OFFSET_REGISTER)
		REGDEF_BIT( 			,		 2)
		REGDEF_BIT(dram_ofso	,		 14)
		REGDEF_BIT( 			,		 8)
		REGDEF_BIT(dram_sao_msb,		 4)
	REGDEF_END(DESTINATION_LINE_OFFSET_REGISTER)
	
	
	/*
		dram_sao_uv:	[0x0, 0x3fffffff],			bits : 31_2
	*/
#define UV_DESTINATION_ADDRESS_REGISTER_OFS 0x0050
	REGDEF_BEGIN(UV_DESTINATION_ADDRESS_REGISTER)
		REGDEF_BIT( 		  , 	   2)
		REGDEF_BIT(dram_sao_uv, 	   30)
	REGDEF_END(UV_DESTINATION_ADDRESS_REGISTER)
	
	
	/*
		dram_ofso	 :	  [0x0, 0x3fff],			bits : 15_2
		dram_sao_msb:	  [0x0, 0xf],			bits : 27_24
	*/
#define UV_DESTINATION_LINE_OFFSET_REGISTER_OFS 0x0054
	REGDEF_BEGIN(UV_DESTINATION_LINE_OFFSET_REGISTER)
		REGDEF_BIT( 			,		 2)
		REGDEF_BIT(dram_ofso	,		 14)
		REGDEF_BIT( 			,		 8)
		REGDEF_BIT(dram_sao_msb,		 4)
	REGDEF_END(UV_DESTINATION_LINE_OFFSET_REGISTER)
	
	
	/*
		ife_yuv_in_fmt_(0~2) :	  [0x0, 0x3],			bits : 1_0
		ife_yuv2rgb_en		 :	  [0x0, 0x1],			bits : 4
		ife_yuv2rgb_fmt_(0~2):	  [0x0, 0x3],			bits : 6_5
	*/
#define YUV_CONTROL_REGISTER_OFS 0x0058
	REGDEF_BEGIN(YUV_CONTROL_REGISTER)
		REGDEF_BIT(ife_yuv_in_fmt , 	   2)
		REGDEF_BIT( 					,		 2)
		REGDEF_BIT(ife_yuv2rgb_en		,		 1)
		REGDEF_BIT(ife_yuv2rgb_fmt, 	   2)
	REGDEF_END(YUV_CONTROL_REGISTER)
	
	
	/*
		input_burst_mode :	  [0x0, 0x1],			bits : 0
		output_burst_mode:	  [0x0, 0x1],			bits : 4
		dmaloop_line	 :	  [0x0, 0x7ff], 		bits : 22_12
		dmaloop_en		 :	  [0x0, 0x1],			bits : 24
		dmaloop_ctrl	 :	  [0x0, 0x1],			bits : 25
	*/
#define DRAM_SETTINGS_OFS 0x005c
	REGDEF_BEGIN(DRAM_SETTINGS)
	REGDEF_BIT(input_burst_mode ,		 1)
	REGDEF_BIT( 				,		 3)
	REGDEF_BIT(output_burst_mode,		 1)
	REGDEF_BIT(,		7)
	REGDEF_BIT(dmaloop_line,		11)
	REGDEF_BIT(,		1)
	REGDEF_BIT(dmaloop_en,		  1)
	REGDEF_BIT(dmaloop_ctrl,		1)
	REGDEF_BIT(,		2)
	REGDEF_BIT(sie2_line_count_align_to_height, 	1)
	REGDEF_END(DRAM_SETTINGS)
	
	
	/*
		ife_dram_out0_single_en:	[0x0, 0x1], 		bits : 0
		ife_dram_out1_single_en:	[0x0, 0x1], 		bits : 1
		ife_dram_out2_single_en:	[0x0, 0x1], 		bits : 2
    	ife_hist_out_single_en    :    [0x0, 0x1],			bits : 3
    	ife_indep_va_out_single_en:    [0x0, 0x1],			bits : 4		
		ife_dram_out_mode	   :	[0x0, 0x1], 		bits : 31
	*/
#define IFE_DMA_OUTPUT_CHANNEL_ENABLE_REGISTER_OFS 0x0060
	REGDEF_BEGIN(IFE_DMA_OUTPUT_CHANNEL_ENABLE_REGISTER)
		REGDEF_BIT(ife_dram_out0_single_en, 	   1)
		REGDEF_BIT(ife_dram_out1_single_en, 	   1)
		REGDEF_BIT(ife_dram_out2_single_en, 	   1)
		REGDEF_BIT(ife_hist_out_single_en    ,     1)
		REGDEF_BIT(ife_indep_va_out_single_en,     1)
		REGDEF_BIT(                          ,    26)
		REGDEF_BIT(ife_dram_out_mode	  , 	   1)
	REGDEF_END(IFE_DMA_OUTPUT_CHANNEL_ENABLE_REGISTER)
	
	
	/*
		ife_dgain:	  [0x0, 0xffff],			bits : 15_0
	*/
#define DIGITAL_GAIN_REGISTER_OFS 0x006c
	REGDEF_BEGIN(DIGITAL_GAIN_REGISTER)
	REGDEF_BIT(ife_dgain,		 16)
	REGDEF_END(DIGITAL_GAIN_REGISTER)
	
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
	
	
	/*
		ife_f_p0_cgain_r :	  [0x0, 0xffff], 		bits : 15_0
		ife_f_p0_cgain_gr:	  [0x0, 0xffff], 		bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_4_OFS 0x0080
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_4)
	REGDEF_BIT(ife_f_p0_cgain_r,		16)
	REGDEF_BIT(ife_f_p0_cgain_gr,		 16)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_4)
	
	
	/*
		ife_f_p0_cgain_gb:	  [0x0, 0xffff], 		bits : 15_0
		ife_f_p0_cgain_b :	  [0x0, 0xffff], 		bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_5_OFS 0x0084
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_5)
	REGDEF_BIT(ife_f_p0_cgain_gb,		 16)
	REGDEF_BIT(ife_f_p0_cgain_b,		16)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_5)
	
	
	/*
		ife_f_p0_cgain_ir:	  [0x0, 0xffff], 		bits : 15_0
	*/
#define IFE_COLOR_GAIN_REGISTER_6_OFS 0x0088
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_6)
	REGDEF_BIT(ife_f_p0_cgain_ir,		 16)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_6)
	
	
	/*
		ife_f_p1_cgain_r :	  [0x0, 0xffff], 		bits : 15_0
		ife_f_p1_cgain_gr:	  [0x0, 0xffff], 		bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_7_OFS 0x008c
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_7)
	REGDEF_BIT(ife_f_p1_cgain_r,		16)
	REGDEF_BIT(ife_f_p1_cgain_gr,		 16)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_7)
	
	
	/*
		ife_f_p1_cgain_gb:	  [0x0, 0xffff], 		bits : 15_0
		ife_f_p1_cgain_b :	  [0x0, 0xffff], 		bits : 25_16
	*/
#define IFE_COLOR_GAIN_REGISTER_8_OFS 0x0090
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_8)
	REGDEF_BIT(ife_f_p1_cgain_gb,		 16)
	REGDEF_BIT(ife_f_p1_cgain_b,		16)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_8)
	
	
	/*
		ife_f_p1_cgain_ir:	  [0x0, 0xffff], 		bits : 15_0
	*/
#define IFE_COLOR_GAIN_REGISTER_9_OFS 0x0094
	REGDEF_BEGIN(IFE_COLOR_GAIN_REGISTER_9)
	REGDEF_BIT(ife_f_p1_cgain_ir,		 16)
	REGDEF_END(IFE_COLOR_GAIN_REGISTER_9)
	
	
	/*
		ife_line_count :	[0x0, 0xffff],			  bits : 15_0
		sie2_line_count:	[0x0, 0xffff],			  bits : 31_16
	*/
#define DMALOOP_ERROR_LINE_COUNT_REGISTER_OFS 0x00A8
	REGDEF_BEGIN(DMALOOP_ERROR_LINE_COUNT_REGISTER)
	REGDEF_BIT(ife_line_count,		  16)
	REGDEF_BIT(sie2_line_count, 	  16)
	REGDEF_END(DMALOOP_ERROR_LINE_COUNT_REGISTER)
	
	
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
	
	
	/*
		ife_outl_rgbir_rb_w:	[0x0, 0xff],			bits : 7_0
		ife_ord_rgbir_rb_w :	[0x0, 0xff],			bits : 15_8
	*/
#define OUTLIER_REGISTER_OFS 0x00f0
	REGDEF_BEGIN(OUTLIER_REGISTER)
	REGDEF_BIT(ife_outl_rgbir_rb_w, 	   8)
	REGDEF_BIT(ife_ord_rgbir_rb_w,		  8)
	REGDEF_END(OUTLIER_REGISTER)
	
	
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
	
	
	/*
		IFE_S_WEIGHT0:	  [0x0, 0x1f],			bits : 4_0
		IFE_S_WEIGHT1:	  [0x0, 0x1f],			bits : 12_8
		IFE_S_WEIGHT2:	  [0x0, 0x1f],			bits : 20_16
		IFE_S_WEIGHT3:	  [0x0, 0x1f],			bits : 28_24
	*/
#define SPATIAL_FILTER_REGISTER_1_OFS 0x0108
	REGDEF_BEGIN(SPATIAL_FILTER_REGISTER_1)
	REGDEF_BIT(ife_spatial_weight0, 	   5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_spatial_weight1, 	   5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_spatial_weight2, 	   5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_spatial_weight3, 	   5)
	REGDEF_END(SPATIAL_FILTER_REGISTER_1)
	
	
	/*
		IFE_S_WEIGHT4:	  [0x0, 0x1f],			bits : 4_0
		IFE_S_WEIGHT5:	  [0x0, 0x1f],			bits : 12_8
	*/
#define SPATIAL_FILTER_REGISTER_2_OFS 0x010c
	REGDEF_BEGIN(SPATIAL_FILTER_REGISTER_2)
	REGDEF_BIT(ife_spatial_weight4, 	   5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_spatial_weight5, 	   5)
	REGDEF_END(SPATIAL_FILTER_REGISTER_2)
	
	
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
		ife_rth_nlm_c0_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c0_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_2_OFS 0x0114
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_2)
	REGDEF_BIT(ife_rth_nlm_c0_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_2)
	
	
	/*
		ife_rth_nlm_c0_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c0_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_3_OFS 0x0118
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_3)
	REGDEF_BIT(ife_rth_nlm_c0_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_3)
	
	
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
		ife_rth_nlm_c1_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c1_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_6_OFS 0x0124
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_6)
	REGDEF_BIT(ife_rth_nlm_c1_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_6)
	
	
	/*
		ife_rth_nlm_c1_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c1_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_7_OFS 0x0128
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_7)
	REGDEF_BIT(ife_rth_nlm_c1_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_7)
	
	
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
		ife_rth_nlm_c2_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c2_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_10_OFS 0x0134
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_10)
	REGDEF_BIT(ife_rth_nlm_c2_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_10)
	
	
	/*
		ife_rth_nlm_c2_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c2_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_11_OFS 0x0138
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_11)
	REGDEF_BIT(ife_rth_nlm_c2_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_11)
	
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
		ife_rth_nlm_c3_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c3_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_14_OFS 0x0144
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_14)
	REGDEF_BIT(ife_rth_nlm_c3_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_14)
	
	
	/*
		ife_rth_nlm_c3_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c3_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_15_OFS 0x0148
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_15)
	REGDEF_BIT(ife_rth_nlm_c3_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_15)
	
	
	/*
		reserved:	 [0x0, 0xffffffff], 		bits : 31_0
	*/
#define RANGE_FILTER_REGISTER_16_OFS 0x014c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_16)
	REGDEF_BIT(reserved,		32)
	REGDEF_END(RANGE_FILTER_REGISTER_16)
	
	
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
		ife_rth_bilat_c0_2:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_18_OFS 0x0154
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_18)
	REGDEF_BIT(ife_rth_bilat_c0_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_18)
	
	
	/*
		ife_rth_bilat_c0_4:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_19_OFS 0x0158
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_19)
	REGDEF_BIT(ife_rth_bilat_c0_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_19)
	
	
	/*
		reserved:	 [0x0, 0xffffffff], 		bits : 31_0
	*/
#define RANGE_FILTER_REGISTER_20_OFS 0x015c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_20)
	REGDEF_BIT(reserved,		32)
	REGDEF_END(RANGE_FILTER_REGISTER_20)
	
	
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
		ife_rth_bilat_c1_2:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_22_OFS 0x0164
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_22)
	REGDEF_BIT(ife_rth_bilat_c1_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_22)
	
	
	/*
		ife_rth_bilat_c1_4:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_23_OFS 0x0168
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_23)
	REGDEF_BIT(ife_rth_bilat_c1_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_23)
	
	
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
		ife_rth_bilat_c2_2:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_26_OFS 0x0174
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_26)
	REGDEF_BIT(ife_rth_bilat_c2_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_26)
	
	
	/*
		ife_rth_bilat_c2_4:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_27_OFS 0x0178
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_27)
	REGDEF_BIT(ife_rth_bilat_c2_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_27)
	
	
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
	
	
	/*
		ife_rth_bilat_c3_2:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_30_OFS 0x0184
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_30)
	REGDEF_BIT(ife_rth_bilat_c3_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_30)
	
	
	/*
		ife_rth_bilat_c3_4:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_31_OFS 0x0188
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_31)
	REGDEF_BIT(ife_rth_bilat_c3_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_31)
	
	
	/*
		ife_outl_bright_ofs:	[0x0, 0xfff],			bits : 11_0
		ife_outl_dark_ofs  :	[0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_0_OFS 0x018c
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_0)
	REGDEF_BIT(ife_outl_bright_ofs, 	   12)
	REGDEF_BIT(ife_outl_dark_ofs,		 12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_0)
	
	
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
		ife_outlth_bri1 :	 [0x0, 0xfff],			bits : 11_0
		ife_outlth_dark1:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_2_OFS 0x0194
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_2)
	REGDEF_BIT(ife_outlth_bri1, 	   12)
	REGDEF_BIT(ife_outlth_dark1,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_2)
	
	
	/*
		ife_outlth_bri2 :	 [0x0, 0xfff],			bits : 11_0
		ife_outlth_dark2:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_3_OFS 0x0198
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_3)
	REGDEF_BIT(ife_outlth_bri2, 	   12)
	REGDEF_BIT(ife_outlth_dark2,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_3)
	
	
	/*
		ife_outlth_bri3 :	 [0x0, 0xfff],			bits : 11_0
		ife_outlth_dark3:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_4_OFS 0x019c
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_4)
	REGDEF_BIT(ife_outlth_bri3, 	   12)
	REGDEF_BIT(ife_outlth_dark3,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_4)
	
	
	/*
		ife_outlth_bri4 :	 [0x0, 0xfff],			bits : 11_0
		ife_outlth_dark4:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_5_OFS 0x01a0
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_5)
	REGDEF_BIT(ife_outlth_bri4, 	   12)
	REGDEF_BIT(ife_outlth_dark4,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_5)
	
	
	/*
		ife_outl_compensate_mode :	  [0x0, 0x3],			bits : 1_0
		ife_outl_weight  :	  [0x0, 0xff],			bits : 15_8
		ife_outl_cnt1	 :	  [0x0, 0x1f],			bits : 20_16
		ife_outl_cnt2	 :	  [0x0, 0x1f],			bits : 28_24
	*/
#define OUTLIER_THRESHOLD_REGISTER_6_OFS 0x01a4
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_6)
	REGDEF_BIT(ife_outl_compensate_mode ,		 2)
	REGDEF_BIT( 						,		 7)
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
	
	
	/*
		ife_ord_bright_weight_lut0 (0~8):	 [0x0, 0xf],			bits : 3_0
		ife_ord_bright_weight_lut1 (0~8):	 [0x0, 0xf],			bits : 7_4
		ife_ord_bright_weight_lut2 (0~8):	 [0x0, 0xf],			bits : 11_8
		ife_ord_bright_weight_lut3 (0~8):	 [0x0, 0xf],			bits : 15_12
		ife_ord_bright_weight_lut4 (0~8):	 [0x0, 0xf],			bits : 19_16
		ife_ord_bright_weight_lut5 (0~8):	 [0x0, 0xf],			bits : 23_20
		ife_ord_bright_weight_lut6 (0~8):	 [0x0, 0xf],			bits : 27_24
		ife_ord_bright_weight_lut7 (0~8):	 [0x0, 0xf],			bits : 31_28
	*/
#define OUTLIER_ORDER_REGISTER_1_OFS 0x01ac
	REGDEF_BEGIN(OUTLIER_ORDER_REGISTER_1)
	REGDEF_BIT(ife_ord_bright_weight_lut0,		  4)
	REGDEF_BIT(ife_ord_bright_weight_lut1,		  4)
	REGDEF_BIT(ife_ord_bright_weight_lut2,		  4)
	REGDEF_BIT(ife_ord_bright_weight_lut3,		  4)
	REGDEF_BIT(ife_ord_bright_weight_lut4,		  4)
	REGDEF_BIT(ife_ord_bright_weight_lut5,		  4)
	REGDEF_BIT(ife_ord_bright_weight_lut6,		  4)
	REGDEF_BIT(ife_ord_bright_weight_lut7,		  4)
	REGDEF_END(OUTLIER_ORDER_REGISTER_1)
	
	
	/*
		ife_ord_dark_weight_lut0 (0~8):    [0x0, 0xf],			bits : 3_0
		ife_ord_dark_weight_lut1 (0~8):    [0x0, 0xf],			bits : 7_4
		ife_ord_dark_weight_lut2 (0~8):    [0x0, 0xf],			bits : 11_8
		ife_ord_dark_weight_lut3 (0~8):    [0x0, 0xf],			bits : 15_12
		ife_ord_dark_weight_lut4 (0~8):    [0x0, 0xf],			bits : 19_16
		ife_ord_dark_weight_lut5 (0~8):    [0x0, 0xf],			bits : 23_20
		ife_ord_dark_weight_lut6 (0~8):    [0x0, 0xf],			bits : 27_24
		ife_ord_dark_weight_lut7 (0~8):    [0x0, 0xf],			bits : 31_28
	*/
#define OUTLIER_ORDER_REGISTER_2_OFS 0x01b0
	REGDEF_BEGIN(OUTLIER_ORDER_REGISTER_2)
	REGDEF_BIT(ife_ord_dark_weight_lut0,		4)
	REGDEF_BIT(ife_ord_dark_weight_lut1,		4)
	REGDEF_BIT(ife_ord_dark_weight_lut2,		4)
	REGDEF_BIT(ife_ord_dark_weight_lut3,		4)
	REGDEF_BIT(ife_ord_dark_weight_lut4,		4)
	REGDEF_BIT(ife_ord_dark_weight_lut5,		4)
	REGDEF_BIT(ife_ord_dark_weight_lut6,		4)
	REGDEF_BIT(ife_ord_dark_weight_lut7,		4)
	REGDEF_END(OUTLIER_ORDER_REGISTER_2)
	
	
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
	
	
	/*
		ife_gbal_edge_w_max:	[0x0, 0xff],			bits : 7_0
		ife_gbal_edge_w_min:	[0x0, 0xff],			bits : 15_8
	*/
#define GBAL_REGISTER_2_OFS 0x01bc
	REGDEF_BEGIN(GBAL_REGISTER_2)
	REGDEF_BIT(ife_gbal_edge_w_max, 	   8)
	REGDEF_BIT(ife_gbal_edge_w_min, 	   8)
	REGDEF_END(GBAL_REGISTER_2)
	
	
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
		ife_rth_nlm_c0_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c0_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_33_OFS 0x01c4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_33)
	REGDEF_BIT(ife_rth_nlm_c0_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_lut_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_33)
	
	
	/*
		ife_rth_nlm_c0_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c0_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_34_OFS 0x01c8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_34)
	REGDEF_BIT(ife_rth_nlm_c0_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_lut_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_34)
	
	
	/*
		ife_rth_nlm_c0_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c0_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_35_OFS 0x01cc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_35)
	REGDEF_BIT(ife_rth_nlm_c0_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_lut_7,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_35)
	
	
	/*
		ife_rth_nlm_c0_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c0_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_36_OFS 0x01d0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_36)
	REGDEF_BIT(ife_rth_nlm_c0_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_lut_9,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_36)
	
	
	/*
		ife_rth_nlm_c0_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c0_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_37_OFS 0x01d4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_37)
	REGDEF_BIT(ife_rth_nlm_c0_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_lut_11,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_37)
	
	
	/*
		ife_rth_nlm_c0_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c0_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_38_OFS 0x01d8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_38)
	REGDEF_BIT(ife_rth_nlm_c0_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_lut_13,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_38)
	
	
	/*
		ife_rth_nlm_c0_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c0_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_39_OFS 0x01dc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_39)
	REGDEF_BIT(ife_rth_nlm_c0_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c0_lut_15,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_39)
	
	
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
		ife_rth_nlm_c1_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c1_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_42_OFS 0x01e8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_42)
	REGDEF_BIT(ife_rth_nlm_c1_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_lut_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_42)
	
	
	/*
		ife_rth_nlm_c1_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c1_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_43_OFS 0x01ec
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_43)
	REGDEF_BIT(ife_rth_nlm_c1_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_lut_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_43)
	
	
	/*
		ife_rth_nlm_c1_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c1_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_44_OFS 0x01f0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_44)
	REGDEF_BIT(ife_rth_nlm_c1_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_lut_7,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_44)
	
	
	/*
		ife_rth_nlm_c1_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c1_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_45_OFS 0x01f4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_45)
	REGDEF_BIT(ife_rth_nlm_c1_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_lut_9,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_45)
	
	
	/*
		ife_rth_nlm_c1_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c1_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_46_OFS 0x01f8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_46)
	REGDEF_BIT(ife_rth_nlm_c1_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_lut_11,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_46)
	
	
	/*
		ife_rth_nlm_c1_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c1_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_47_OFS 0x01fc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_47)
	REGDEF_BIT(ife_rth_nlm_c1_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_lut_13,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_47)
	
	
	/*
		ife_rth_nlm_c1_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c1_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_48_OFS 0x0200
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_48)
	REGDEF_BIT(ife_rth_nlm_c1_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c1_lut_15,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_48)
	
	
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
		ife_rth_nlm_c2_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c2_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_51_OFS 0x020c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_51)
	REGDEF_BIT(ife_rth_nlm_c2_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_lut_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_51)
	
	
	/*
		ife_rth_nlm_c2_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c2_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_52_OFS 0x0210
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_52)
	REGDEF_BIT(ife_rth_nlm_c2_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_lut_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_52)
	
	
	/*
		ife_rth_nlm_c2_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c2_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_53_OFS 0x0214
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_53)
	REGDEF_BIT(ife_rth_nlm_c2_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_lut_7,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_53)
	
	
	/*
		ife_rth_nlm_c2_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c2_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_54_OFS 0x0218
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_54)
	REGDEF_BIT(ife_rth_nlm_c2_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_lut_9,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_54)
	
	
	/*
		ife_rth_nlm_c2_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c2_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_55_OFS 0x021c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_55)
	REGDEF_BIT(ife_rth_nlm_c2_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_lut_11,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_55)
	
	
	/*
		ife_rth_nlm_c2_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c2_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_56_OFS 0x0220
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_56)
	REGDEF_BIT(ife_rth_nlm_c2_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_lut_13,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_56)
	
	
	/*
		ife_rth_nlm_c2_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c2_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_57_OFS 0x0224
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_57)
	REGDEF_BIT(ife_rth_nlm_c2_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c2_lut_15,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_57)
	
	
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
		ife_rth_nlm_c3_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c3_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_60_OFS 0x0230
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_60)
	REGDEF_BIT(ife_rth_nlm_c3_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_lut_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_60)
	
	
	/*
		ife_rth_nlm_c3_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c3_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_61_OFS 0x0234
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_61)
	REGDEF_BIT(ife_rth_nlm_c3_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_lut_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_61)
	
	
	/*
		ife_rth_nlm_c3_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c3_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_62_OFS 0x0238
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_62)
	REGDEF_BIT(ife_rth_nlm_c3_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_lut_7,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_62)
	
	
	/*
		ife_rth_nlm_c3_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		ife_rth_nlm_c3_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_63_OFS 0x023c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_63)
	REGDEF_BIT(ife_rth_nlm_c3_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_lut_9,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_63)
	
	
	/*
		ife_rth_nlm_c3_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c3_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_64_OFS 0x0240
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_64)
	REGDEF_BIT(ife_rth_nlm_c3_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_lut_11,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_64)
	
	
	/*
		ife_rth_nlm_c3_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c3_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_65_OFS 0x0244
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_65)
	REGDEF_BIT(ife_rth_nlm_c3_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_lut_13,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_65)
	
	
	/*
		ife_rth_nlm_c3_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		ife_rth_nlm_c3_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_66_OFS 0x0248
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_66)
	REGDEF_BIT(ife_rth_nlm_c3_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_nlm_c3_lut_15,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_66)
	
	
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
		ife_rth_bilat_c0_lut_2:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_lut_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_69_OFS 0x0254
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_69)
	REGDEF_BIT(ife_rth_bilat_c0_lut_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_lut_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_69)
	
	
	/*
		ife_rth_bilat_c0_lut_4:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_lut_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_70_OFS 0x0258
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_70)
	REGDEF_BIT(ife_rth_bilat_c0_lut_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_lut_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_70)
	
	
	/*
		ife_rth_bilat_c0_lut_6:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_lut_7:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_71_OFS 0x025c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_71)
	REGDEF_BIT(ife_rth_bilat_c0_lut_6,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_lut_7,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_71)
	
	
	/*
		ife_rth_bilat_c0_lut_8:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_lut_9:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_72_OFS 0x0260
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_72)
	REGDEF_BIT(ife_rth_bilat_c0_lut_8,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_lut_9,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_72)
	
	
	/*
		ife_rth_bilat_c0_lut_10:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_lut_11:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_73_OFS 0x0264
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_73)
	REGDEF_BIT(ife_rth_bilat_c0_lut_10, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_lut_11, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_73)
	
	
	/*
		ife_rth_bilat_c0_lut_12:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_lut_13:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_74_OFS 0x0268
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_74)
	REGDEF_BIT(ife_rth_bilat_c0_lut_12, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_lut_13, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_74)
	
	
	/*
		ife_rth_bilat_c0_lut_14:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c0_lut_15:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_75_OFS 0x026c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_75)
	REGDEF_BIT(ife_rth_bilat_c0_lut_14, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c0_lut_15, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_75)
	
	
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
		ife_rth_bilat_c1_lut_2:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_lut_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_78_OFS 0x0278
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_78)
	REGDEF_BIT(ife_rth_bilat_c1_lut_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_lut_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_78)
	
	
	/*
		ife_rth_bilat_c1_lut_4:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_lut_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_79_OFS 0x027c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_79)
	REGDEF_BIT(ife_rth_bilat_c1_lut_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_lut_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_79)
	
	
	/*
		ife_rth_bilat_c1_lut_6:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_lut_7:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_80_OFS 0x0280
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_80)
	REGDEF_BIT(ife_rth_bilat_c1_lut_6,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_lut_7,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_80)
	
	
	/*
		ife_rth_bilat_c1_lut_8:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_lut_9:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_81_OFS 0x0284
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_81)
	REGDEF_BIT(ife_rth_bilat_c1_lut_8,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_lut_9,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_81)
	
	
	/*
		ife_rth_bilat_c1_lut_10:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_lut_11:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_82_OFS 0x0288
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_82)
	REGDEF_BIT(ife_rth_bilat_c1_lut_10, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_lut_11, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_82)
	
	
	/*
		ife_rth_bilat_c1_lut_12:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_lut_13:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_83_OFS 0x028c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_83)
	REGDEF_BIT(ife_rth_bilat_c1_lut_12, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_lut_13, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_83)
	
	
	/*
		ife_rth_bilat_c1_lut_14:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c1_lut_15:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_84_OFS 0x0290
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_84)
	REGDEF_BIT(ife_rth_bilat_c1_lut_14, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c1_lut_15, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_84)
	
	
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
		ife_rth_bilat_c2_lut_2:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_lut_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_87_OFS 0x029c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_87)
	REGDEF_BIT(ife_rth_bilat_c2_lut_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_lut_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_87)
	
	
	/*
		ife_rth_bilat_c2_lut_4:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_lut_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_88_OFS 0x02a0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_88)
	REGDEF_BIT(ife_rth_bilat_c2_lut_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_lut_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_88)
	
	
	/*
		ife_rth_bilat_c2_lut_6:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_lut_7:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_89_OFS 0x02a4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_89)
	REGDEF_BIT(ife_rth_bilat_c2_lut_6,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_lut_7,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_89)
	
	
	/*
		ife_rth_bilat_c2_lut_8:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_lut_9:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_90_OFS 0x02a8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_90)
	REGDEF_BIT(ife_rth_bilat_c2_lut_8,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_lut_9,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_90)
	
	
	/*
		ife_rth_bilat_c2_lut_10:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_lut_11:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_91_OFS 0x02ac
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_91)
	REGDEF_BIT(ife_rth_bilat_c2_lut_10, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_lut_11, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_91)
	
	
	/*
		ife_rth_bilat_c2_lut_12:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_lut_13:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_92_OFS 0x02b0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_92)
	REGDEF_BIT(ife_rth_bilat_c2_lut_12, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_lut_13, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_92)
	
	
	/*
		ife_rth_bilat_c2_lut_14:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c2_lut_15:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_93_OFS 0x02b4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_93)
	REGDEF_BIT(ife_rth_bilat_c2_lut_14, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c2_lut_15, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_93)
	
	
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
		ife_rth_bilat_c3_lut_2:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_lut_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_96_OFS 0x02c0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_96)
	REGDEF_BIT(ife_rth_bilat_c3_lut_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_lut_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_96)
	
	
	/*
		ife_rth_bilat_c3_lut_4:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_lut_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_97_OFS 0x02c4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_97)
	REGDEF_BIT(ife_rth_bilat_c3_lut_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_lut_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_97)
	
	
	/*
		ife_rth_bilat_c3_lut_6:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_lut_7:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_98_OFS 0x02c8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_98)
	REGDEF_BIT(ife_rth_bilat_c3_lut_6,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_lut_7,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_98)
	
	
	/*
		ife_rth_bilat_c3_lut_8:    [0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_lut_9:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_99_OFS 0x02cc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_99)
	REGDEF_BIT(ife_rth_bilat_c3_lut_8,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_lut_9,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_99)
	
	
	/*
		ife_rth_bilat_c3_lut_10:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_lut_11:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_100_OFS 0x02d0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_100)
	REGDEF_BIT(ife_rth_bilat_c3_lut_10, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_lut_11, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_100)
	
	
	/*
		ife_rth_bilat_c3_lut_12:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_lut_13:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_101_OFS 0x02d4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_101)
	REGDEF_BIT(ife_rth_bilat_c3_lut_12, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_lut_13, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_101)
	
	
	/*
		ife_rth_bilat_c3_lut_14:	[0x0, 0x3ff],			bits : 9_0
		ife_rth_bilat_c3_lut_15:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_102_OFS 0x02d8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_102)
	REGDEF_BIT(ife_rth_bilat_c3_lut_14, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_rth_bilat_c3_lut_15, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_102)
	
	
	/*
		ife_rth_bilat_c3_lut_16:	[0x0, 0x3ff],			bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_103_OFS 0x02dc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_103)
	REGDEF_BIT(ife_rth_bilat_c3_lut_16, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_103)
	
	
	/*
		ife_distvgtx_c0:	[0x0, 0xffff],			bits : 15_0
		ife_distvgty_c0:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_0_OFS 0x02e0
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_0)
		REGDEF_BIT(ife_distvgtx_c0, 	   14)
		REGDEF_BIT(               ,        2)
		REGDEF_BIT(ife_distvgty_c0, 	   14)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_0)
	
	
	/*
		ife_distvgtx_c1:	[0x0, 0xffff],			bits : 15_0
		ife_distvgty_c1:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_1_OFS 0x02e4
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_1)
		REGDEF_BIT(ife_distvgtx_c1, 	   14)
		REGDEF_BIT(               ,        2)
		REGDEF_BIT(ife_distvgty_c1, 	   14)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_1)
	
	
	/*
		ife_distvgtx_c2:	[0x0, 0xffff],			bits : 15_0
		ife_distvgty_c2:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_2_OFS 0x02e8
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_2)
		REGDEF_BIT(ife_distvgtx_c2, 	   14)
		REGDEF_BIT(               ,        2)
		REGDEF_BIT(ife_distvgty_c2, 	   14)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_2)
	
	
	/*
		ife_distvgtx_c3:	[0x0, 0xffff],			bits : 15_0
		ife_distvgty_c3:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_3_OFS 0x02ec
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_3)
		REGDEF_BIT(ife_distvgtx_c3, 	   14)
		REGDEF_BIT(               ,        2)
		REGDEF_BIT(ife_distvgty_c3, 	   14)
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
		distdthr_en :	 [0x0, 0x1],			bits : 0
		distdthr_rst:	 [0x0, 0x1],			bits : 4
		distthr 	:	 [0x0, 0x3ff],			bits : 17_8
	*/
#define VIGNETTE_SETTING_REGISTER_5_OFS 0x02f4
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_5)
		REGDEF_BIT(distdthr_en ,		1)
		REGDEF_BIT( 		   ,		3)
		REGDEF_BIT(distdthr_rst,		1)
		REGDEF_BIT( 		   ,		3)
		REGDEF_BIT(distthr	   ,		10)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_5)
	
	
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
		ife_vig_c0_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c0_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_1_OFS 0x02fc
	REGDEF_BEGIN(VIGNETTE_REGISTER_1)
	REGDEF_BIT(ife_vig_c0_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c0_lut_3,		10)
	REGDEF_END(VIGNETTE_REGISTER_1)
	
	
	/*
		ife_vig_c0_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c0_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_2_OFS 0x0300
	REGDEF_BEGIN(VIGNETTE_REGISTER_2)
	REGDEF_BIT(ife_vig_c0_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c0_lut_5,		10)
	REGDEF_END(VIGNETTE_REGISTER_2)
	
	
	/*
		ife_vig_c0_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c0_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_3_OFS 0x0304
	REGDEF_BEGIN(VIGNETTE_REGISTER_3)
	REGDEF_BIT(ife_vig_c0_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c0_lut_7,		10)
	REGDEF_END(VIGNETTE_REGISTER_3)
	
	
	/*
		ife_vig_c0_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c0_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_4_OFS 0x0308
	REGDEF_BEGIN(VIGNETTE_REGISTER_4)
	REGDEF_BIT(ife_vig_c0_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c0_lut_9,		10)
	REGDEF_END(VIGNETTE_REGISTER_4)
	
	
	/*
		ife_vig_c0_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c0_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_5_OFS 0x030c
	REGDEF_BEGIN(VIGNETTE_REGISTER_5)
	REGDEF_BIT(ife_vig_c0_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c0_lut_11,		 10)
	REGDEF_END(VIGNETTE_REGISTER_5)
	
	
	/*
		ife_vig_c0_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c0_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_6_OFS 0x0310
	REGDEF_BEGIN(VIGNETTE_REGISTER_6)
	REGDEF_BIT(ife_vig_c0_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c0_lut_13,		 10)
	REGDEF_END(VIGNETTE_REGISTER_6)
	
	
	/*
		ife_vig_c0_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c0_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_7_OFS 0x0314
	REGDEF_BEGIN(VIGNETTE_REGISTER_7)
	REGDEF_BIT(ife_vig_c0_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c0_lut_15,		 10)
	REGDEF_END(VIGNETTE_REGISTER_7)
	
	
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
		ife_vig_c1_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c1_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_10_OFS 0x0320
	REGDEF_BEGIN(VIGNETTE_REGISTER_10)
	REGDEF_BIT(ife_vig_c1_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c1_lut_3,		10)
	REGDEF_END(VIGNETTE_REGISTER_10)
	
	
	/*
		ife_vig_c1_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c1_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_11_OFS 0x0324
	REGDEF_BEGIN(VIGNETTE_REGISTER_11)
	REGDEF_BIT(ife_vig_c1_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c1_lut_5,		10)
	REGDEF_END(VIGNETTE_REGISTER_11)
	
	
	/*
		ife_vig_c1_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c1_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_12_OFS 0x0328
	REGDEF_BEGIN(VIGNETTE_REGISTER_12)
	REGDEF_BIT(ife_vig_c1_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c1_lut_7,		10)
	REGDEF_END(VIGNETTE_REGISTER_12)
	
	
	/*
		ife_vig_c1_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c1_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_13_OFS 0x032c
	REGDEF_BEGIN(VIGNETTE_REGISTER_13)
	REGDEF_BIT(ife_vig_c1_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c1_lut_9,		10)
	REGDEF_END(VIGNETTE_REGISTER_13)
	
	
	/*
		ife_vig_c1_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c1_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_14_OFS 0x0330
	REGDEF_BEGIN(VIGNETTE_REGISTER_14)
	REGDEF_BIT(ife_vig_c1_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c1_lut_11,		 10)
	REGDEF_END(VIGNETTE_REGISTER_14)
	
	
	/*
		ife_vig_c1_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c1_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_15_OFS 0x0334
	REGDEF_BEGIN(VIGNETTE_REGISTER_15)
	REGDEF_BIT(ife_vig_c1_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c1_lut_13,		 10)
	REGDEF_END(VIGNETTE_REGISTER_15)
	
	
	/*
		ife_vig_c1_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c1_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_16_OFS 0x0338
	REGDEF_BEGIN(VIGNETTE_REGISTER_16)
	REGDEF_BIT(ife_vig_c1_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c1_lut_15,		 10)
	REGDEF_END(VIGNETTE_REGISTER_16)
	
	
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
		ife_vig_c2_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c2_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_19_OFS 0x0344
	REGDEF_BEGIN(VIGNETTE_REGISTER_19)
	REGDEF_BIT(ife_vig_c2_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c2_lut_3,		10)
	REGDEF_END(VIGNETTE_REGISTER_19)
	
	
	/*
		ife_vig_c2_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c2_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_20_OFS 0x0348
	REGDEF_BEGIN(VIGNETTE_REGISTER_20)
	REGDEF_BIT(ife_vig_c2_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c2_lut_5,		10)
	REGDEF_END(VIGNETTE_REGISTER_20)
	
	
	/*
		ife_vig_c2_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c2_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_21_OFS 0x034c
	REGDEF_BEGIN(VIGNETTE_REGISTER_21)
	REGDEF_BIT(ife_vig_c2_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c2_lut_7,		10)
	REGDEF_END(VIGNETTE_REGISTER_21)
	
	
	/*
		ife_vig_c2_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c2_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_22_OFS 0x0350
	REGDEF_BEGIN(VIGNETTE_REGISTER_22)
	REGDEF_BIT(ife_vig_c2_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c2_lut_9,		10)
	REGDEF_END(VIGNETTE_REGISTER_22)
	
	
	/*
		ife_vig_c2_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c2_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_23_OFS 0x0354
	REGDEF_BEGIN(VIGNETTE_REGISTER_23)
	REGDEF_BIT(ife_vig_c2_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c2_lut_11,		 10)
	REGDEF_END(VIGNETTE_REGISTER_23)
	
	
	/*
		ife_vig_c2_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c2_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_24_OFS 0x0358
	REGDEF_BEGIN(VIGNETTE_REGISTER_24)
	REGDEF_BIT(ife_vig_c2_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c2_lut_13,		 10)
	REGDEF_END(VIGNETTE_REGISTER_24)
	
	
	/*
		ife_vig_c2_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c2_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_25_OFS 0x035c
	REGDEF_BEGIN(VIGNETTE_REGISTER_25)
	REGDEF_BIT(ife_vig_c2_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c2_lut_15,		 10)
	REGDEF_END(VIGNETTE_REGISTER_25)
	
	
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
		ife_vig_c3_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c3_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_28_OFS 0x0368
	REGDEF_BEGIN(VIGNETTE_REGISTER_28)
	REGDEF_BIT(ife_vig_c3_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c3_lut_3,		10)
	REGDEF_END(VIGNETTE_REGISTER_28)
	
	
	/*
		ife_vig_c3_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c3_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_29_OFS 0x036c
	REGDEF_BEGIN(VIGNETTE_REGISTER_29)
	REGDEF_BIT(ife_vig_c3_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c3_lut_5,		10)
	REGDEF_END(VIGNETTE_REGISTER_29)
	
	
	/*
		ife_vig_c3_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c3_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_30_OFS 0x0370
	REGDEF_BEGIN(VIGNETTE_REGISTER_30)
	REGDEF_BIT(ife_vig_c3_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c3_lut_7,		10)
	REGDEF_END(VIGNETTE_REGISTER_30)
	
	
	/*
		ife_vig_c3_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		ife_vig_c3_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_31_OFS 0x0374
	REGDEF_BEGIN(VIGNETTE_REGISTER_31)
	REGDEF_BIT(ife_vig_c3_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c3_lut_9,		10)
	REGDEF_END(VIGNETTE_REGISTER_31)
	
	
	/*
		ife_vig_c3_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c3_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_32_OFS 0x0378
	REGDEF_BEGIN(VIGNETTE_REGISTER_32)
	REGDEF_BIT(ife_vig_c3_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c3_lut_11,		 10)
	REGDEF_END(VIGNETTE_REGISTER_32)
	
	
	/*
		ife_vig_c3_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c3_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_33_OFS 0x037c
	REGDEF_BEGIN(VIGNETTE_REGISTER_33)
	REGDEF_BIT(ife_vig_c3_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c3_lut_13,		 10)
	REGDEF_END(VIGNETTE_REGISTER_33)
	
	
	/*
		ife_vig_c3_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		ife_vig_c3_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_34_OFS 0x0380
	REGDEF_BEGIN(VIGNETTE_REGISTER_34)
	REGDEF_BIT(ife_vig_c3_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_vig_c3_lut_15,		 10)
	REGDEF_END(VIGNETTE_REGISTER_34)
	
	
	/*
		ife_vig_c3_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define VIGNETTE_REGISTER_35_OFS 0x0384
	REGDEF_BEGIN(VIGNETTE_REGISTER_35)
	REGDEF_BIT(ife_vig_c3_lut_16,		 10)
	REGDEF_END(VIGNETTE_REGISTER_35)
	
	
	/*
		ife_gbal_ofs_lut_00:	[0x0, 0x3f],			bits : 5_0
		ife_gbal_ofs_lut_01:	[0x0, 0x3f],			bits : 13_8
		ife_gbal_ofs_lut_02:	[0x0, 0x3f],			bits : 21_16
		ife_gbal_ofs_lut_03:	[0x0, 0x3f],			bits : 29_24
	*/
#define GBAL_REGISTER_3_OFS 0x0388
	REGDEF_BEGIN(GBAL_REGISTER_3)
	REGDEF_BIT(ife_gbal_ofs_lut_00, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_01, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_02, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_03, 	   6)
	REGDEF_END(GBAL_REGISTER_3)
	
	
	/*
		ife_gbal_ofs_lut_04:	[0x0, 0x3f],			bits : 5_0
		ife_gbal_ofs_lut_05:	[0x0, 0x3f],			bits : 13_8
		ife_gbal_ofs_lut_06:	[0x0, 0x3f],			bits : 21_16
		ife_gbal_ofs_lut_07:	[0x0, 0x3f],			bits : 29_24
	*/
#define GBAL_REGISTER_4_OFS 0x038c
	REGDEF_BEGIN(GBAL_REGISTER_4)
	REGDEF_BIT(ife_gbal_ofs_lut_04, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_05, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_06, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_07, 	   6)
	REGDEF_END(GBAL_REGISTER_4)
	
	
	/*
		ife_gbal_ofs_lut_08:	[0x0, 0x3f],			bits : 5_0
		ife_gbal_ofs_lut_09:	[0x0, 0x3f],			bits : 13_8
		ife_gbal_ofs_lut_10:	[0x0, 0x3f],			bits : 21_16
		ife_gbal_ofs_lut_11:	[0x0, 0x3f],			bits : 29_24
	*/
#define GBAL_REGISTER_5_OFS 0x0390
	REGDEF_BEGIN(GBAL_REGISTER_5)
	REGDEF_BIT(ife_gbal_ofs_lut_08, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_09, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_10, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_11, 	   6)
	REGDEF_END(GBAL_REGISTER_5)
	
	
	/*
		ife_gbal_ofs_lut_12:	[0x0, 0x3f],			bits : 5_0
		ife_gbal_ofs_lut_13:	[0x0, 0x3f],			bits : 13_8
		ife_gbal_ofs_lut_14:	[0x0, 0x3f],			bits : 21_16
		ife_gbal_ofs_lut_15:	[0x0, 0x3f],			bits : 29_24
	*/
#define GBAL_REGISTER_6_OFS 0x0394
	REGDEF_BEGIN(GBAL_REGISTER_6)
	REGDEF_BIT(ife_gbal_ofs_lut_12, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_13, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_14, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(ife_gbal_ofs_lut_15, 	   6)
	REGDEF_END(GBAL_REGISTER_6)
	
	
	/*
		ife_gbal_ofs_lut_16:	[0x0, 0x3f],			bits : 5_0
	*/
#define GBAL_REGISTER_7_OFS 0x0398
	REGDEF_BEGIN(GBAL_REGISTER_7)
	REGDEF_BIT(ife_gbal_ofs_lut_16, 	   6)
	REGDEF_END(GBAL_REGISTER_7)
	
	
	/*
		ife_rbluma00:	 [0x0, 0x1f],			bits : 4_0
		ife_rbluma01:	 [0x0, 0x1f],			bits : 12_8
		ife_rbluma02:	 [0x0, 0x1f],			bits : 20_16
		ife_rbluma03:	 [0x0, 0x1f],			bits : 28_24
	*/
#define RB_LUMINANCE_REGISTER_0_OFS 0x039c
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_0)
	REGDEF_BIT(ife_rbluma00,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma01,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma02,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma03,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_0)
	
	
	/*
		ife_rbluma04:	 [0x0, 0x1f],			bits : 4_0
		ife_rbluma05:	 [0x0, 0x1f],			bits : 12_8
		ife_rbluma06:	 [0x0, 0x1f],			bits : 20_16
		ife_rbluma07:	 [0x0, 0x1f],			bits : 28_24
	*/
#define RB_LUMINANCE_REGISTER_1_OFS 0x03a0
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_1)
	REGDEF_BIT(ife_rbluma04,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma05,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma06,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma07,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_1)
	
	
	/*
		ife_rbluma08:	 [0x0, 0x1f],			bits : 4_0
		ife_rbluma09:	 [0x0, 0x1f],			bits : 12_8
		ife_rbluma10:	 [0x0, 0x1f],			bits : 20_16
		ife_rbluma11:	 [0x0, 0x1f],			bits : 28_24
	*/
#define RB_LUMINANCE_REGISTER_2_OFS 0x03a4
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_2)
	REGDEF_BIT(ife_rbluma08,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma09,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma10,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma11,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_2)
	
	
	/*
		ife_rbluma12:	 [0x0, 0x1f],			bits : 4_0
		ife_rbluma13:	 [0x0, 0x1f],			bits : 12_8
		ife_rbluma14:	 [0x0, 0x1f],			bits : 20_16
		ife_rbluma15:	 [0x0, 0x1f],			bits : 28_24
	*/
#define RB_LUMINANCE_REGISTER_3_OFS 0x03a8
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_3)
	REGDEF_BIT(ife_rbluma12,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma13,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma14,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbluma15,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_3)
	
	
	/*
		ife_rbluma16:	 [0x0, 0x1f],			bits : 4_0
	*/
#define RB_LUMINANCE_REGISTER_4_OFS 0x03ac
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_4)
	REGDEF_BIT(ife_rbluma16,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_4)
	
	/*
		ife_rbratio00:	  [0x0, 0x1f],			bits : 4_0
		ife_rbratio01:	  [0x0, 0x1f],			bits : 12_8
		ife_rbratio02:	  [0x0, 0x1f],			bits : 20_16
		ife_rbratio03:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_0_OFS 0x03c0
	REGDEF_BEGIN(RB_RATIO_REGISTER_0)
	REGDEF_BIT(ife_rbratio00,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio01,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio02,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio03,		 5)
	REGDEF_END(RB_RATIO_REGISTER_0)
	
	
	/*
		ife_rbratio04:	  [0x0, 0x1f],			bits : 4_0
		ife_rbratio05:	  [0x0, 0x1f],			bits : 12_8
		ife_rbratio06:	  [0x0, 0x1f],			bits : 20_16
		ife_rbratio07:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_1_OFS 0x03c4
	REGDEF_BEGIN(RB_RATIO_REGISTER_1)
	REGDEF_BIT(ife_rbratio04,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio05,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio06,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio07,		 5)
	REGDEF_END(RB_RATIO_REGISTER_1)
	
	
	/*
		ife_rbratio08:	  [0x0, 0x1f],			bits : 4_0
		ife_rbratio09:	  [0x0, 0x1f],			bits : 12_8
		ife_rbratio10:	  [0x0, 0x1f],			bits : 20_16
		ife_rbratio11:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_2_OFS 0x03c8
	REGDEF_BEGIN(RB_RATIO_REGISTER_2)
	REGDEF_BIT(ife_rbratio08,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio09,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio10,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio11,		 5)
	REGDEF_END(RB_RATIO_REGISTER_2)
	
	
	/*
		ife_rbratio12:	  [0x0, 0x1f],			bits : 4_0
		ife_rbratio13:	  [0x0, 0x1f],			bits : 12_8
		ife_rbratio14:	  [0x0, 0x1f],			bits : 20_16
		ife_rbratio15:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_3_OFS 0x03cc
	REGDEF_BEGIN(RB_RATIO_REGISTER_3)
	REGDEF_BIT(ife_rbratio12,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio13,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio14,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio15,		 5)
	REGDEF_END(RB_RATIO_REGISTER_3)
	
	
	/*
		ife_rbratio16:	  [0x0, 0x1f],			bits : 4_0
		ife_rbratio17:	  [0x0, 0x1f],			bits : 12_8
		ife_rbratio18:	  [0x0, 0x1f],			bits : 20_16
		ife_rbratio19:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_4_OFS 0x03d0
	REGDEF_BEGIN(RB_RATIO_REGISTER_4)
	REGDEF_BIT(ife_rbratio16,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio17,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio18,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio19,		 5)
	REGDEF_END(RB_RATIO_REGISTER_4)
	
	
	/*
		ife_rbratio20:	  [0x0, 0x1f],			bits : 4_0
		ife_rbratio21:	  [0x0, 0x1f],			bits : 12_8
		ife_rbratio22:	  [0x0, 0x1f],			bits : 20_16
		ife_rbratio23:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_5_OFS 0x03d4
	REGDEF_BEGIN(RB_RATIO_REGISTER_5)
	REGDEF_BIT(ife_rbratio20,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio21,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio22,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio23,		 5)
	REGDEF_END(RB_RATIO_REGISTER_5)
	
	
	/*
		ife_rbratio24:	  [0x0, 0x1f],			bits : 4_0
		ife_rbratio25:	  [0x0, 0x1f],			bits : 12_8
		ife_rbratio26:	  [0x0, 0x1f],			bits : 20_16
		ife_rbratio27:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_6_OFS 0x03d8
	REGDEF_BEGIN(RB_RATIO_REGISTER_6)
	REGDEF_BIT(ife_rbratio24,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio25,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio26,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio27,		 5)
	REGDEF_END(RB_RATIO_REGISTER_6)
	
	
	/*
		ife_rbratio28	  :    [0x0, 0x1f], 		bits : 4_0
		ife_rbratio29	  :    [0x0, 0x1f], 		bits : 12_8
		ife_rbratio30	  :    [0x0, 0x1f], 		bits : 20_16
		ife_rbratio31	  :    [0x0, 0x1f], 		bits : 28_24
		rbratio_mode (0~2):    [0x0, 0x3],			bits : 30_29
	*/
#define RB_RATIO_REGISTER_7_OFS 0x03dc
	REGDEF_BEGIN(RB_RATIO_REGISTER_7)
	REGDEF_BIT(ife_rbratio28,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio29,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio30,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(ife_rbratio31,		 5)
	REGDEF_BIT(rbratio_mode,		2)
	REGDEF_END(RB_RATIO_REGISTER_7)
	
	/*
		ife_axi_disable   :    [0x0, 0x1],			bits : 0
		ife_axi_ch_idle   :    [0x0, 0x1],			bits : 1
		ife_axi_channel_en:    [0x0, 0x1ff],			bits : 16_8
		ife_axi_lock_dis  :    [0x0, 0x1ff],			bits : 25_17
	*/
#define IFE_AXI_REGISTER_0_OFS 0x03f0
	REGDEF_BEGIN(IFE_AXI_REGISTER_0)
		REGDEF_BIT(ife_axi_disable	 ,		  1)
		REGDEF_BIT(ife_axi_ch_idle	 ,		  1)
		REGDEF_BIT( 				 ,		  6)
		REGDEF_BIT(ife_axi_channel_en,		  9)
		REGDEF_BIT(ife_axi_lock_dis  ,		  9)
	REGDEF_END(IFE_AXI_REGISTER_0)
	
	
	/*
		ife_r_ostd_num:    [0x0, 0xff], 		bits : 7_0
		ife_w_ostd_num:    [0x0, 0xff], 		bits : 15_8
	*/
#define IFE_AXI_REGISTER_2_OFS 0x03f8
	REGDEF_BEGIN(IFE_AXI_REGISTER_2)
		REGDEF_BIT(ife_r_ostd_num,		  8)
		REGDEF_BIT(ife_w_ostd_num,		  8)
	REGDEF_END(IFE_AXI_REGISTER_2)
	
	
	
	
	/*
		ife_f_nrs0_en:	  [0x0, 0x1],			bits : 0
	*/
#define NRS_CONTROL_REGISTER_OFS 0x0500
	REGDEF_BEGIN(NRS_REGISTER)
	REGDEF_BIT(ife_f_nrs0_en,		 1)
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
		IFE_F_NRS0_STR2 :	 [0x0, 0x3FF],			  bits : 9_0
		IFE_F_NRS0_STR3 :	 [0x0, 0x3FF],			  bits : 25_16
	*/
#define NRS_REGISTER1_OFS 0x0508
	REGDEF_BEGIN(NRS_REGISTER1)
	REGDEF_BIT(ife_f_nrs0_str2, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_nrs0_str3, 	   10)
	REGDEF_END(NRS_REGISTER1)
	
	
	
	/*
		IFE_F_NRS0_STR4 :	 [0x0, 0x3FF],			  bits : 9_0
		IFE_F_NRS0_STR5 :	 [0x0, 0x3FF],			  bits : 25_16
	*/
#define NRS_REGISTER2_OFS 0x050C
	REGDEF_BEGIN(NRS_REGISTER2)
	REGDEF_BIT(ife_f_nrs0_str4, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(ife_f_nrs0_str5, 	   10)
	REGDEF_END(NRS_REGISTER2)
	
	
	
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
		IFE_F_FU_EVRATIO:	 [0x0, 0x1ff],			bits : 8_0
	*/
#define RHE_EVRATIO_REGISTER_OFS 0x053c
	REGDEF_BEGIN(RHE_EVRATIO_REGISTER)
	REGDEF_BIT(ife_f_fusion_evratio,		9)
	REGDEF_END(RHE_EVRATIO_REGISTER)
	
	
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
		ife_f_fusion_mc_lut_diff_weight6 (0~16) :	 [0x0, 0x1f],		   bits : 4_0
		ife_f_fusion_mc_lut_diff_weight7 (0~16) :	 [0x0, 0x1f],		   bits : 9_5
		ife_f_fusion_mc_lut_diff_weight8 (0~16) :	 [0x0, 0x1f],		   bits : 14_10
		ife_f_fusion_mc_lut_diff_weight9 (0~16) :	 [0x0, 0x1f],		   bits : 19_15
		ife_f_fusion_mc_lut_diff_weight10 (0~16):	 [0x0, 0x1f],		   bits : 24_20
		ife_f_fusion_mc_lut_diff_weight11 (0~16):	 [0x0, 0x1f],		   bits : 29_25
	*/
#define RHE_FUSION_MOTION_COMPENSATION_LUT_1_OFS 0x0548
	REGDEF_BEGIN(RHE_FUSION_MOTION_COMPENSATION_LUT_1)
	REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight6,		5)
	REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight7,		5)
	REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight8,		5)
	REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight9,		5)
	REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight10,		 5)
	REGDEF_BIT(ife_f_fusion_mc_lut_diff_weight11,		 5)
	REGDEF_END(RHE_FUSION_MOTION_COMPENSATION_LUT_1)
	
	
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
	
	
	/*
		IFE_F_DS0_TH  :    [0x0, 0xfff],			bits : 11_0
		IFE_F_DS0_STEP:    [0x0, 0xff], 		bits : 23_16
		IFE_F_DS0_LB  :    [0x0, 0xff], 		bits : 31_24
	*/
#define RHE_FUSION_PATH0_DARK_SATURATION_REDUCTION_OFS 0x0550
	REGDEF_BEGIN(RHE_FUSION_PATH0_DSR)
	REGDEF_BIT(ife_f_dark_sat_reduction0_th,		12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(ife_f_dark_sat_reduction0_step,		  8)
	REGDEF_BIT(ife_f_dark_sat_reduction0_low_bound, 	   8)
	REGDEF_END(RHE_FUSION_PATH0_DARK_SATURATION_REDUCTION)
	
	
	/*
		IFE_F_DS1_TH  :    [0x0, 0xfff],			bits : 11_0
		IFE_F_DS1_STEP:    [0x0, 0xff], 		bits : 23_16
		IFE_F_DS1_LB  :    [0x0, 0xff], 		bits : 31_24
	*/
#define RHE_FUSION_PATH1_DARK_SATURATION_REDUCTION_OFS 0x0554
	REGDEF_BEGIN(RHE_FUSION_PATH1_DSR)
	REGDEF_BIT(ife_f_dark_sat_reduction1_th,		12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(ife_f_dark_sat_reduction1_step,		  8)
	REGDEF_BIT(ife_f_dark_sat_reduction1_low_bound, 	   8)
	REGDEF_END(RHE_FUSION_PATH1_DARK_SATURATION_REDUCTION)
	
	
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
		ife_f_fcurve_yweight_lut4:	  [0x0, 0xff],			bits : 7_0
		ife_f_fcurve_yweight_lut5:	  [0x0, 0xff],			bits : 15_8
		ife_f_fcurve_yweight_lut6:	  [0x0, 0xff],			bits : 23_16
		ife_f_fcurve_yweight_lut7:	  [0x0, 0xff],			bits : 31_24
	*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER1_OFS 0x0568
	REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER1)
	REGDEF_BIT(ife_f_fcurve_yweight_lut4,		 8)
	REGDEF_BIT(ife_f_fcurve_yweight_lut5,		 8)
	REGDEF_BIT(ife_f_fcurve_yweight_lut6,		 8)
	REGDEF_BIT(ife_f_fcurve_yweight_lut7,		 8)
	REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER1)
	
	
	/*
		ife_f_fcurve_yweight_lut8 :    [0x0, 0xff], 		bits : 7_0
		ife_f_fcurve_yweight_lut9 :    [0x0, 0xff], 		bits : 15_8
		ife_f_fcurve_yweight_lut10:    [0x0, 0xff], 		bits : 23_16
		ife_f_fcurve_yweight_lut11:    [0x0, 0xff], 		bits : 31_24
	*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER2_OFS 0x056c
	REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER2)
	REGDEF_BIT(ife_f_fcurve_yweight_lut8,		 8)
	REGDEF_BIT(ife_f_fcurve_yweight_lut9,		 8)
	REGDEF_BIT(ife_f_fcurve_yweight_lut10,		  8)
	REGDEF_BIT(ife_f_fcurve_yweight_lut11,		  8)
	REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER2)
	
	
	/*
		ife_f_fcurve_yweight_lut12:    [0x0, 0xff], 		bits : 7_0
		ife_f_fcurve_yweight_lut13:    [0x0, 0xff], 		bits : 15_8
		ife_f_fcurve_yweight_lut14:    [0x0, 0xff], 		bits : 23_16
		ife_f_fcurve_yweight_lut15:    [0x0, 0xff], 		bits : 31_24
	*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER3_OFS 0x0570
	REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER3)
	REGDEF_BIT(ife_f_fcurve_yweight_lut12,		  8)
	REGDEF_BIT(ife_f_fcurve_yweight_lut13,		  8)
	REGDEF_BIT(ife_f_fcurve_yweight_lut14,		  8)
	REGDEF_BIT(ife_f_fcurve_yweight_lut15,		  8)
	REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER3)
	
	
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
		ife_f_fcurve_l_2:	 [0x0, 0xffff], 		bits : 15_0
		ife_f_fcurve_l_3:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_1_OFS 0x057c
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_1)
		REGDEF_BIT(ife_f_fcurve_l_2,		16)
		REGDEF_BIT(ife_f_fcurve_l_3,		16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_1)
	
	
	/*
		ife_f_fcurve_l_4:	 [0x0, 0xffff], 		bits : 15_0
		ife_f_fcurve_l_5:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_2_OFS 0x0580
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_2)
		REGDEF_BIT(ife_f_fcurve_l_4,		16)
		REGDEF_BIT(ife_f_fcurve_l_5,		16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_2)
	
	
	/*
		ife_f_fcurve_l_6:	 [0x0, 0xffff], 		bits : 15_0
		ife_f_fcurve_l_7:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_3_OFS 0x0584
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_3)
		REGDEF_BIT(ife_f_fcurve_l_6,		16)
		REGDEF_BIT(ife_f_fcurve_l_7,		16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_3)
	
	
	/*
		ife_f_fcurve_l_8:	 [0x0, 0xffff], 		bits : 15_0
		ife_f_fcurve_l_9:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_4_OFS 0x0588
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_4)
		REGDEF_BIT(ife_f_fcurve_l_8,		16)
		REGDEF_BIT(ife_f_fcurve_l_9,		16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_4)
	
	
	/*
		ife_f_fcurve_l_10:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_11:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_5_OFS 0x058c
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_5)
		REGDEF_BIT(ife_f_fcurve_l_10,		 16)
		REGDEF_BIT(ife_f_fcurve_l_11,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_5)
	
	
	/*
		ife_f_fcurve_l_12:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_13:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_6_OFS 0x0590
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_6)
		REGDEF_BIT(ife_f_fcurve_l_12,		 16)
		REGDEF_BIT(ife_f_fcurve_l_13,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_6)
	
	
	/*
		ife_f_fcurve_l_14:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_15:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_7_OFS 0x0594
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_7)
		REGDEF_BIT(ife_f_fcurve_l_14,		 16)
		REGDEF_BIT(ife_f_fcurve_l_15,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_7)
	
	
	/*
		ife_f_fcurve_l_16:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_17:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_8_OFS 0x0598
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_8)
		REGDEF_BIT(ife_f_fcurve_l_16,		 16)
		REGDEF_BIT(ife_f_fcurve_l_17,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_8)
	
	
	/*
		ife_f_fcurve_l_18:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_19:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_9_OFS 0x059c
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_9)
		REGDEF_BIT(ife_f_fcurve_l_18,		 16)
		REGDEF_BIT(ife_f_fcurve_l_19,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_9)
	
	
	/*
		ife_f_fcurve_l_20:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_21:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_10_OFS 0x05a0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_10)
		REGDEF_BIT(ife_f_fcurve_l_20,		 16)
		REGDEF_BIT(ife_f_fcurve_l_21,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_10)
	
	
	/*
		ife_f_fcurve_l_22:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_23:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_11_OFS 0x05a4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_11)
		REGDEF_BIT(ife_f_fcurve_l_22,		 16)
		REGDEF_BIT(ife_f_fcurve_l_23,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_11)
	
	
	/*
		ife_f_fcurve_l_24:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_25:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_12_OFS 0x05a8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_12)
		REGDEF_BIT(ife_f_fcurve_l_24,		 16)
		REGDEF_BIT(ife_f_fcurve_l_25,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_12)
	
	
	/*
		ife_f_fcurve_l_26:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_27:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_13_OFS 0x05ac
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_13)
		REGDEF_BIT(ife_f_fcurve_l_26,		 16)
		REGDEF_BIT(ife_f_fcurve_l_27,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_13)
	
	
	/*
		ife_f_fcurve_l_28:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_29:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_14_OFS 0x05b0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_14)
		REGDEF_BIT(ife_f_fcurve_l_28,		 16)
		REGDEF_BIT(ife_f_fcurve_l_29,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_14)
	
	
	/*
		ife_f_fcurve_l_30:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_31:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_15_OFS 0x05b4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_15)
		REGDEF_BIT(ife_f_fcurve_l_30,		 16)
		REGDEF_BIT(ife_f_fcurve_l_31,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_15)
	
	
	/*
		ife_f_fcurve_l_32:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_33:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_16_OFS 0x05b8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_16)
		REGDEF_BIT(ife_f_fcurve_l_32,		 16)
		REGDEF_BIT(ife_f_fcurve_l_33,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_16)
	
	
	/*
		ife_f_fcurve_l_34:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_35:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_17_OFS 0x05bc
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_17)
		REGDEF_BIT(ife_f_fcurve_l_34,		 16)
		REGDEF_BIT(ife_f_fcurve_l_35,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_17)
	
	
	/*
		ife_f_fcurve_l_36:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_37:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_18_OFS 0x05c0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_18)
		REGDEF_BIT(ife_f_fcurve_l_36,		 16)
		REGDEF_BIT(ife_f_fcurve_l_37,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_18)
	
	
	/*
		ife_f_fcurve_l_38:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_39:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_19_OFS 0x05c4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_19)
		REGDEF_BIT(ife_f_fcurve_l_38,		 16)
		REGDEF_BIT(ife_f_fcurve_l_39,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_19)
	
	
	/*
		ife_f_fcurve_l_40:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_41:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_20_OFS 0x05c8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_20)
		REGDEF_BIT(ife_f_fcurve_l_40,		 16)
		REGDEF_BIT(ife_f_fcurve_l_41,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_20)
	
	
	/*
		ife_f_fcurve_l_42:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_43:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_21_OFS 0x05cc
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_21)
		REGDEF_BIT(ife_f_fcurve_l_42,		 16)
		REGDEF_BIT(ife_f_fcurve_l_43,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_21)
	
	
	/*
		ife_f_fcurve_l_44:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_45:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_22_OFS 0x05d0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_22)
		REGDEF_BIT(ife_f_fcurve_l_44,		 16)
		REGDEF_BIT(ife_f_fcurve_l_45,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_22)
	
	
	/*
		ife_f_fcurve_l_46:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_47:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_23_OFS 0x05d4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_23)
		REGDEF_BIT(ife_f_fcurve_l_46,		 16)
		REGDEF_BIT(ife_f_fcurve_l_47,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_23)
	
	
	/*
		ife_f_fcurve_l_48:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_49:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_24_OFS 0x05d8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_24)
		REGDEF_BIT(ife_f_fcurve_l_48,		 16)
		REGDEF_BIT(ife_f_fcurve_l_49,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_24)
	
	
	/*
		ife_f_fcurve_l_50:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_51:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_25_OFS 0x05dc
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_25)
		REGDEF_BIT(ife_f_fcurve_l_50,		 16)
		REGDEF_BIT(ife_f_fcurve_l_51,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_25)
	
	
	/*
		ife_f_fcurve_l_52:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_53:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_26_OFS 0x05e0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_26)
		REGDEF_BIT(ife_f_fcurve_l_52,		 16)
		REGDEF_BIT(ife_f_fcurve_l_53,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_26)
	
	
	/*
		ife_f_fcurve_l_54:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_55:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_27_OFS 0x05e4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_27)
		REGDEF_BIT(ife_f_fcurve_l_54,		 16)
		REGDEF_BIT(ife_f_fcurve_l_55,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_27)
	
	
	/*
		ife_f_fcurve_l_56:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_57:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_28_OFS 0x05e8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_28)
		REGDEF_BIT(ife_f_fcurve_l_56,		 16)
		REGDEF_BIT(ife_f_fcurve_l_57,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_28)
	
	
	/*
		ife_f_fcurve_l_58:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_59:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_29_OFS 0x05ec
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_29)
		REGDEF_BIT(ife_f_fcurve_l_58,		 16)
		REGDEF_BIT(ife_f_fcurve_l_59,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_29)
	
	
	/*
		ife_f_fcurve_l_60:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_61:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_30_OFS 0x05f0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_30)
		REGDEF_BIT(ife_f_fcurve_l_60,		 16)
		REGDEF_BIT(ife_f_fcurve_l_61,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_30)
	
	
	/*
		ife_f_fcurve_l_62:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_l_63:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_31_OFS 0x05f4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_31)
		REGDEF_BIT(ife_f_fcurve_l_62,		 16)
		REGDEF_BIT(ife_f_fcurve_l_63,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_31)
	
	
	/*
		ife_f_fcurve_l_64:	  [0x0, 0xffff],			bits : 15_0
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_32_OFS 0x05f8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_32)
		REGDEF_BIT(ife_f_fcurve_l_64,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_32)

	/*
		ife_f_fcurve_r_0:	 [0x0, 0xffff], 		bits : 15_0
		ife_f_fcurve_r_1:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_0_OFS 0x067c
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_0)
		REGDEF_BIT(ife_f_fcurve_r_0,		16)
		REGDEF_BIT(ife_f_fcurve_r_1,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_0)
	
	
	/*
		ife_f_fcurve_r_2:	 [0x0, 0xffff], 		bits : 15_0
		ife_f_fcurve_r_3:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_1_OFS 0x0680
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_1)
		REGDEF_BIT(ife_f_fcurve_r_2,		16)
		REGDEF_BIT(ife_f_fcurve_r_3,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_1)
	
	
	/*
		ife_f_fcurve_r_4:	 [0x0, 0xffff], 		bits : 15_0
		ife_f_fcurve_r_5:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_2_OFS 0x0684
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_2)
		REGDEF_BIT(ife_f_fcurve_r_4,		16)
		REGDEF_BIT(ife_f_fcurve_r_5,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_2)
	
	
	/*
		ife_f_fcurve_r_6:	 [0x0, 0xffff], 		bits : 15_0
		ife_f_fcurve_r_7:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_3_OFS 0x0688
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_3)
		REGDEF_BIT(ife_f_fcurve_r_6,		16)
		REGDEF_BIT(ife_f_fcurve_r_7,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_3)
	
	
	/*
		ife_f_fcurve_r_8:	 [0x0, 0xffff], 		bits : 15_0
		ife_f_fcurve_r_9:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_4_OFS 0x068c
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_4)
		REGDEF_BIT(ife_f_fcurve_r_8,		16)
		REGDEF_BIT(ife_f_fcurve_r_9,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_4)
	
	
	/*
		ife_f_fcurve_r_10:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_r_11:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_5_OFS 0x0690
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_5)
		REGDEF_BIT(ife_f_fcurve_r_10,		 16)
		REGDEF_BIT(ife_f_fcurve_r_11,		 16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_5)
	
	
	/*
		ife_f_fcurve_r_12:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_r_13:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_6_OFS 0x0694
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_6)
		REGDEF_BIT(ife_f_fcurve_r_12,		 16)
		REGDEF_BIT(ife_f_fcurve_r_13,		 16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_6)
	
	
	/*
		ife_f_fcurve_r_14:	  [0x0, 0xffff],			bits : 15_0
		ife_f_fcurve_r_15:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_7_OFS 0x0698
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_7)
		REGDEF_BIT(ife_f_fcurve_r_14,		 16)
		REGDEF_BIT(ife_f_fcurve_r_15,		 16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_7)
	
	
	/*
		ife_f_fcurve_r_16		 :	  [0x0, 0xffff],			bits : 15_0
		IFE_F_FCURVE_EV_FMT (0~4):	  [0x0, 0x7],			bits : 26_24
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_8_OFS 0x069c
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_8)
		REGDEF_BIT(ife_f_fcurve_r_16		,		 16)
		REGDEF_BIT( 						,		 8)
		REGDEF_BIT(ife_f_fcurve_ev_fmt      ,		 3)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_8)



	/*
		ife_fpn_en		   :	[0x0, 0x1], 		bits : 0
		ife_fpn_cgain_range:	[0x0, 0x1], 		bits : 1
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER_OFS 0x070c
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER)
		REGDEF_BIT(ife_fpn_en		  , 	   1)
		REGDEF_BIT(ife_fpn_cgain_range, 	   1)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER)
	
	
	/*
		ife_fpn_p0_cgain_r :	[0x0, 0x3ff],			bits : 9_0
		ife_fpn_p0_cgain_gr:	[0x0, 0x3ff],			bits : 25_16
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER1_OFS 0x0710
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER1)
		REGDEF_BIT(ife_fpn_p0_cgain_r , 	   10)
		REGDEF_BIT( 				  , 	   6)
		REGDEF_BIT(ife_fpn_p0_cgain_gr, 	   10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER1)
	
	
	/*
		ife_fpn_p0_cgain_gb:	[0x0, 0x3ff],			bits : 9_0
		ife_fpn_p0_cgain_b :	[0x0, 0x3ff],			bits : 25_16
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER2_OFS 0x0714
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER2)
		REGDEF_BIT(ife_fpn_p0_cgain_gb, 	   10)
		REGDEF_BIT( 				  , 	   6)
		REGDEF_BIT(ife_fpn_p0_cgain_b , 	   10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER2)
	
	
	/*
		ife_fpn_p0_cgain_ir:	[0x0, 0x3ff],			bits : 9_0
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER3_OFS 0x0718
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER3)
		REGDEF_BIT(ife_fpn_p0_cgain_ir, 	   10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER3)
	
	
	/*
		ife_fpn_p0_cofs_r :    [0x0, 0x3ff],			bits : 9_0
		ife_fpn_p0_cofs_gr:    [0x0, 0x3ff],			bits : 25_16
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER7_OFS 0x0728
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER7)
		REGDEF_BIT(ife_fpn_p0_cofs_r ,		  10)
		REGDEF_BIT( 				 ,		  6)
		REGDEF_BIT(ife_fpn_p0_cofs_gr,		  10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER7)
	
	
	/*
		ife_fpn_p0_cofs_gb:    [0x0, 0x3ff],			bits : 9_0
		ife_fpn_p0_cofs_b :    [0x0, 0x3ff],			bits : 25_16
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER8_OFS 0x072c
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER8)
		REGDEF_BIT(ife_fpn_p0_cofs_gb,		  10)
		REGDEF_BIT( 				 ,		  6)
		REGDEF_BIT(ife_fpn_p0_cofs_b ,		  10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER8)
	
	
	/*
		ife_fpn_p0_cofs_ir:    [0x0, 0x3ff],			bits : 9_0
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER9_OFS 0x0730
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER9)
		REGDEF_BIT(ife_fpn_p0_cofs_ir,		  10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER9)
	
	
	/*
		ife_ll_cmd_start_addr_info:    [0x0, 0xffffffff],			bits : 31_0
	*/
#define LINKED_LIST_DEBUG_REGISTER0_OFS 0x07f8
	REGDEF_BEGIN(LINKED_LIST_DEBUG_REGISTER0)
		REGDEF_BIT(ife_ll_cmd_start_addr_info,		  32)
	REGDEF_END(LINKED_LIST_DEBUG_REGISTER0)
	
	
	/*
		ife_ll_cmdprs_cnt:	  [0x0, 0xffffffff],			bits : 31_0
	*/
#define LINKED_LIST_DEBUG_REGISTER1_OFS 0x07fc
	REGDEF_BEGIN(LINKED_LIST_DEBUG_REGISTER1)
		REGDEF_BIT(ife_ll_cmdprs_cnt,		 32)
	REGDEF_END(LINKED_LIST_DEBUG_REGISTER1)
	
	
	/*
		ife_r_degamma_en:	 [0x0, 0x1],			bits : 0
		ife_r_decode_en :	 [0x0, 0x1],			bits : 1
		ife_r_decode2_en:	 [0x0, 0x1],			bits : 2
		ife_r_dith_en	:	 [0x0, 0x1],			bits : 4
		ife_r_segbitno	:	 [0x0, 0x3],			bits : 9_8
		ife_r_dith_rst	:	 [0x0, 0x1],			bits : 12
	*/
#define RDE_CONTROL_OFS 0x0800
	REGDEF_BEGIN(RDE_CONTROL)
		REGDEF_BIT(ife_r_degamma_en,		1)
		REGDEF_BIT(ife_r_decode_en ,		1)
		REGDEF_BIT(ife_r_decode2_en,		1)
		REGDEF_BIT( 			   ,		1)
		REGDEF_BIT(ife_r_dith_en   ,		1)
		REGDEF_BIT( 			   ,		3)
		REGDEF_BIT(ife_r_segbitno  ,		2)
		REGDEF_BIT( 			   ,		2)
		REGDEF_BIT(ife_r_dith_rst  ,		1)
	REGDEF_END(RDE_CONTROL)
	
	
	/*
		ife_r_dct_qtbl0_idx:	[0x0, 0x1f],			bits : 4_0
		ife_r_dct_qtbl1_idx:	[0x0, 0x1f],			bits : 12_8
		ife_r_dct_qtbl2_idx:	[0x0, 0x1f],			bits : 20_16
		ife_r_dct_qtbl3_idx:	[0x0, 0x1f],			bits : 28_24
	*/
#define DCT_QTBL_REGISTER_0_OFS 0x0804
	REGDEF_BEGIN(DCT_QTBL_REGISTER_0)
		REGDEF_BIT(ife_r_dct_qtbl0_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(ife_r_dct_qtbl1_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(ife_r_dct_qtbl2_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(ife_r_dct_qtbl3_idx, 	   5)
	REGDEF_END(DCT_QTBL_REGISTER_0)
	
	
	/*
		ife_r_dct_qtbl4_idx:	[0x0, 0x1f],			bits : 4_0
		ife_r_dct_qtbl5_idx:	[0x0, 0x1f],			bits : 12_8
		ife_r_dct_qtbl6_idx:	[0x0, 0x1f],			bits : 20_16
		ife_r_dct_qtbl7_idx:	[0x0, 0x1f],			bits : 28_24
	*/
#define DCT_QTBL_REGISTER_1_OFS 0x0808
	REGDEF_BEGIN(DCT_QTBL_REGISTER_1)
		REGDEF_BIT(ife_r_dct_qtbl4_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(ife_r_dct_qtbl5_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(ife_r_dct_qtbl6_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(ife_r_dct_qtbl7_idx, 	   5)
	REGDEF_END(DCT_QTBL_REGISTER_1)
	
	
	/*
		ife_r_out_rand1_init1:	  [0x0, 0xf],			bits : 3_0
		ife_r_out_rand1_init2:	  [0x0, 0x7fff],			bits : 18_4
	*/
#define DITHERING_INITIAL_REGISTER_0_OFS 0x080c
	REGDEF_BEGIN(DITHERING_INITIAL_REGISTER_0)
		REGDEF_BIT(ife_r_out_rand1_init1,		 4)
		REGDEF_BIT(ife_r_out_rand1_init2,		 15)
	REGDEF_END(DITHERING_INITIAL_REGISTER_0)
	
	
	/*
		ife_r_out_rand2_init1:	  [0x0, 0xf],			bits : 3_0
		ife_r_out_rand2_init2:	  [0x0, 0x7fff],			bits : 18_4
	*/
#define DITHERING_INITIAL_REGISTER_1_OFS 0x0810
	REGDEF_BEGIN(DITHERING_INITIAL_REGISTER_1)
		REGDEF_BIT(ife_r_out_rand2_init1,		 4)
		REGDEF_BIT(ife_r_out_rand2_init2,		 15)
	REGDEF_END(DITHERING_INITIAL_REGISTER_1)
	
	/*
		ife_stripe_size0:	 [0x0, 0x7ff],			bits : 10_0
		ife_stripe_num	:	 [0x0, 0xf],			bits : 15_12
		ife_stripe_size1:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define IFE_MULTIPLE_STRIPE_SETTING_REGISTER0_OFS 0x0838
	REGDEF_BEGIN(IFE_MULTIPLE_STRIPE_SETTING_REGISTER0)
		REGDEF_BIT(ife_stripe_size0,		11)
		REGDEF_BIT( 			   ,		1)
		REGDEF_BIT(ife_stripe_num  ,		4)
		REGDEF_BIT(ife_stripe_size1,		11)
	REGDEF_END(IFE_MULTIPLE_STRIPE_SETTING_REGISTER0)
	
	
	/*
		ife_stripe_size2:	 [0x0, 0x7ff],			bits : 10_0
		ife_stripe_size3:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define IFE_MULTIPLE_STRIPE_SETTING_REGISTER1_OFS 0x083c
	REGDEF_BEGIN(IFE_MULTIPLE_STRIPE_SETTING_REGISTER1)
		REGDEF_BIT(ife_stripe_size2,		11)
		REGDEF_BIT( 			   ,		5)
		REGDEF_BIT(ife_stripe_size3,		11)
	REGDEF_END(IFE_MULTIPLE_STRIPE_SETTING_REGISTER1)
	
	
	/*
		ife_stripe_size4:	 [0x0, 0x7ff],			bits : 10_0
		ife_stripe_size5:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define IFE_MULTIPLE_STRIPE_SETTING_REGISTER2_OFS 0x0840
	REGDEF_BEGIN(IFE_MULTIPLE_STRIPE_SETTING_REGISTER2)
		REGDEF_BIT(ife_stripe_size4,		11)
		REGDEF_BIT( 			   ,		5)
		REGDEF_BIT(ife_stripe_size5,		11)
	REGDEF_END(IFE_MULTIPLE_STRIPE_SETTING_REGISTER2)
	
	
	/*
		ife_stripe_size6:	 [0x0, 0x7ff],			bits : 10_0
		ife_stripe_size7:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define IFE_MULTIPLE_STRIPE_SETTING_REGISTER3_OFS 0x0844
	REGDEF_BEGIN(IFE_MULTIPLE_STRIPE_SETTING_REGISTER3)
		REGDEF_BIT(ife_stripe_size6,		11)
		REGDEF_BIT( 			   ,		5)
		REGDEF_BIT(ife_stripe_size7,		11)
	REGDEF_END(IFE_MULTIPLE_STRIPE_SETTING_REGISTER3)
	
	
	/*
		ife_stripe_overlap:    [0x0, 0x3ff],			bits : 9_3
	*/
#define IFE_MULTIPLE_STRIPE_SETTING_REGISTER4_OFS 0x0848
	REGDEF_BEGIN(IFE_MULTIPLE_STRIPE_SETTING_REGISTER4)
		REGDEF_BIT( 			   ,		3)
		REGDEF_BIT(ife_stripe_overlap,		7)
	REGDEF_END(IFE_MULTIPLE_STRIPE_SETTING_REGISTER4)
	
	
	/*
		ife_sub_ipp_cst_en	   :	[0x0, 0x1], 		bits : 0
		ife_sub_ipp_gamma_en   :	[0x0, 0x1], 		bits : 1
		ife_sub_ipp_cfa_en	   :	[0x0, 0x1], 		bits : 2
		ife_sub_ipp_out_yuv_fmt:	[0x0, 0x1], 		bits : 3
	*/
#define SUB_IPP_CONTROL_REGISTER_OFS 0x0860
	REGDEF_BEGIN(SUB_IPP_CONTROL_REGISTER)
		REGDEF_BIT(ife_sub_ipp_cst_en	  , 	   1)
		REGDEF_BIT(ife_sub_ipp_gamma_en   , 	   1)
		REGDEF_BIT(ife_sub_ipp_cfa_en	  , 	   1)
		REGDEF_BIT(ife_sub_ipp_out_yuv_fmt, 	   1)
	REGDEF_END(SUB_IPP_CONTROL_REGISTER)
	
	
	/*
		ife_sub_ipp_coef_rr:	[0x0, 0xfff],			bits : 11_0
		ife_sub_ipp_coef_rg:	[0x0, 0xfff],			bits : 27_16
	*/
#define COLOR_CORRECTION_REGISTER0_OFS 0x0864
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER0)
		REGDEF_BIT(ife_sub_ipp_coef_rr, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(ife_sub_ipp_coef_rg, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER0)
	
	
	/*
		ife_sub_ipp_coef_rb:	[0x0, 0xfff],			bits : 11_0
		ife_sub_ipp_coef_gr:	[0x0, 0xfff],			bits : 27_16
	*/
#define COLOR_CORRECTION_REGISTER1_OFS 0x0868
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER1)
		REGDEF_BIT(ife_sub_ipp_coef_rb, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(ife_sub_ipp_coef_gr, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER1)
	
	
	/*
		ife_sub_ipp_coef_gg:	[0x0, 0xfff],			bits : 11_0
		ife_sub_ipp_coef_gb:	[0x0, 0xfff],			bits : 27_16
	*/
#define COLOR_CORRECTION_REGISTER2_OFS 0x086c
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER2)
		REGDEF_BIT(ife_sub_ipp_coef_gg, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(ife_sub_ipp_coef_gb, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER2)
	
	
	/*
		ife_sub_ipp_coef_br:	[0x0, 0xfff],			bits : 11_0
		ife_sub_ipp_coef_bg:	[0x0, 0xfff],			bits : 27_16
	*/
#define COLOR_CORRECTION_REGISTER3_OFS 0x0870
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER3)
		REGDEF_BIT(ife_sub_ipp_coef_br, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(ife_sub_ipp_coef_bg, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER3)
	
	
	/*
		ife_sub_ipp_coef_bb:	[0x0, 0xfff],			bits : 11_0
	*/
#define COLOR_CORRECTION_REGISTER4_OFS 0x0874
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER4)
		REGDEF_BIT(ife_sub_ipp_coef_bb, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER4)
	
	
	/*
		ife_sub_ipp_smpl_x_rate:	[0x0, 0x1f],			bits : 4_0
		ife_sub_ipp_smpl_y_rate:	[0x0, 0x1f],			bits : 12_8
	*/
#define SCALE_REGISTER_OFS 0x0878
	REGDEF_BEGIN(SCALE_REGISTER)
		REGDEF_BIT(ife_sub_ipp_smpl_x_rate, 	   5)
		REGDEF_BIT( 					  , 	   3)
		REGDEF_BIT(ife_sub_ipp_smpl_y_rate, 	   5)
	REGDEF_END(SCALE_REGISTER)
	
	
	/*
		ife_sub_ipp_gamma_lut_0:	[0x0, 0xff],			bits : 7_0
		ife_sub_ipp_gamma_lut_1:	[0x0, 0xff],			bits : 15_8
		ife_sub_ipp_gamma_lut_2:	[0x0, 0xff],			bits : 23_16
		ife_sub_ipp_gamma_lut_3:	[0x0, 0xff],			bits : 31_24
	*/
#define RGB_GAMMA_LUT_REGISTER0_OFS 0x087c
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER0)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_0, 	   8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_1, 	   8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_2, 	   8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_3, 	   8)
	REGDEF_END(RGB_GAMMA_LUT_REGISTER0)
	
	
	/*
		ife_sub_ipp_gamma_lut_4:	[0x0, 0xff],			bits : 7_0
		ife_sub_ipp_gamma_lut_5:	[0x0, 0xff],			bits : 15_8
		ife_sub_ipp_gamma_lut_6:	[0x0, 0xff],			bits : 23_16
		ife_sub_ipp_gamma_lut_7:	[0x0, 0xff],			bits : 31_24
	*/
#define RGB_GAMMA_LUT_REGISTER1_OFS 0x0880
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER1)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_4, 	   8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_5, 	   8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_6, 	   8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_7, 	   8)
	REGDEF_END(RGB_GAMMA_LUT_REGISTER1)
	
	
	/*
		ife_sub_ipp_gamma_lut_8 :	 [0x0, 0xff],			bits : 7_0
		ife_sub_ipp_gamma_lut_9 :	 [0x0, 0xff],			bits : 15_8
		ife_sub_ipp_gamma_lut_10:	 [0x0, 0xff],			bits : 23_16
		ife_sub_ipp_gamma_lut_11:	 [0x0, 0xff],			bits : 31_24
	*/
#define RGB_GAMMA_LUT_REGISTER2_OFS 0x0884
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER2)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_8 ,		8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_9 ,		8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_10,		8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_11,		8)
	REGDEF_END(RGB_GAMMA_LUT_REGISTER2)
	
	
	/*
		ife_sub_ipp_gamma_lut_12:	 [0x0, 0xff],			bits : 7_0
		ife_sub_ipp_gamma_lut_13:	 [0x0, 0xff],			bits : 15_8
		ife_sub_ipp_gamma_lut_14:	 [0x0, 0xff],			bits : 23_16
		ife_sub_ipp_gamma_lut_15:	 [0x0, 0xff],			bits : 31_24
	*/
#define RGB_GAMMA_LUT_REGISTER3_OFS 0x0888
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER3)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_12,		8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_13,		8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_14,		8)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_15,		8)
	REGDEF_END(RGB_GAMMA_LUT_REGISTER3)
	
	
	/*
		ife_sub_ipp_gamma_lut_16:	 [0x0, 0xff],			bits : 7_0
	*/
#define RGB_GAMMA_LUT_REGISTER4_OFS 0x088c
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER4)
		REGDEF_BIT(ife_sub_ipp_gamma_lut_16,		8)
	REGDEF_END(RGB_GAMMA_LUT_REGISTER4)
	
	
	
	/*
		sub_ipp_crop_width :	[0x0, 0xffff],			bits : 15_0
		sub_ipp_crop_height:	[0x0, 0xffff],			bits : 31_16
	*/
#define SUB_IPP_CROP_SIZE_OFS 0x0894
	REGDEF_BEGIN(SUB_IPP_CROP_SIZE)
		REGDEF_BIT(sub_ipp_crop_width , 	   16)
		REGDEF_BIT(sub_ipp_crop_height, 	   16)
	REGDEF_END(SUB_IPP_CROP_SIZE)
	
	
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
		ife_wdr_subimg_sai_msb:	[0x0, 0xf], 		bits : 27_24
	*/
#define IFE_WDR_SUBIMAGE_REGISTER_2_OFS 0x090c
	REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_2)
		REGDEF_BIT( 					  , 	   2)
		REGDEF_BIT(ife_wdr_subimg_ofsi	  , 	   14)
		REGDEF_BIT( 					  , 	   8)
		REGDEF_BIT(ife_wdr_subimg_sai_msb, 	   4)
	REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_2)
	
	
	/*
		ife_wdr_subimg_sao:    [0x0, 0x3fffffff],			bits : 31_2
	*/
#define IFE_WDR_SUBIMAGE_REGISTER_3_OFS 0x0910
	REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_3)
		REGDEF_BIT( 				 ,		  2)
		REGDEF_BIT(ife_wdr_subimg_sao,		  30)
	REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_3)
	
	
	/*
		ife_wdr_subimg_ofso    :	[0x0, 0x3fff],			bits : 15_2
		ife_wdr_subimg_sao_msb:	[0x0, 0xf], 		bits : 27_24
	*/
#define IFE_WDR_SUBIMAGE_REGISTER_4_OFS 0x0914
	REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_4)
		REGDEF_BIT( 					  , 	   2)
		REGDEF_BIT(ife_wdr_subimg_ofso	  , 	   14)
		REGDEF_BIT( 					  , 	   8)
		REGDEF_BIT(ife_wdr_subimg_sao_msb, 	   4)
	REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_4)
	
	
	/*
		ife_wdr_subimg_ratio_hori:	  [0x0, 0xffff],			bits : 15_0
		ife_wdr_subimg_ratio_vert:	  [0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_SUBIMAGE_REGISTER_5_OFS 0x0918
	REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_5)
		REGDEF_BIT(ife_wdr_subimg_ratio_hori,		 16)
		REGDEF_BIT(ife_wdr_subimg_ratio_vert,		 16)
	REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_5)
	
	
	/*
		ife_wdr_subimg_ratio_hori_div:	  [0x0, 0x3fffff],			bits : 21_0
	*/
#define IFE_WDR_SUBIMAGE_REGISTER_6_OFS 0x091c
	REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_6)
		REGDEF_BIT(ife_wdr_subimg_ratio_hori_div,		 22)
	REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_6)
	
	
	/*
		ife_wdr_subimg_ratio_vert_div:	  [0x0, 0x3fffff],			bits : 21_0
	*/
#define IFE_WDR_SUBIMAGE_REGISTER_7_OFS 0x0920
	REGDEF_BEGIN(IFE_WDR_SUBIMAGE_REGISTER_7)
		REGDEF_BIT(ife_wdr_subimg_ratio_vert_div,		 22)
	REGDEF_END(IFE_WDR_SUBIMAGE_REGISTER_7)
	
	
	/*
		ife_wdr_gain_prot_str_(0~256):	  [0x0, 0x1ff], 		bits : 8_0
	*/
#define IFE_WDR_GAIN_STRENGTH_REGISTER_OFS 0x0924
	REGDEF_BEGIN(IFE_WDR_GAIN_STRENGTH_REGISTER)
		REGDEF_BIT(ife_wdr_gain_prot_str,		 9)
	REGDEF_END(IFE_WDR_GAIN_STRENGTH_REGISTER)
	
	
	/*
		ife_wdr_input_bldrto0:	  [0x0, 0xff],			bits : 7_0
		ife_wdr_input_bldrto1:	  [0x0, 0xff],			bits : 15_8
		ife_wdr_input_bldrto2:	  [0x0, 0xff],			bits : 23_16
		ife_wdr_input_bldrto3:	  [0x0, 0xff],			bits : 31_24
	*/
#define IFE_WDR_INPUT_BLENDING_REGISTER0_OFS 0x0928
	REGDEF_BEGIN(IFE_WDR_INPUT_BLENDING_REGISTER0)
		REGDEF_BIT(ife_wdr_input_bldrto0,		 8)
		REGDEF_BIT(ife_wdr_input_bldrto1,		 8)
		REGDEF_BIT(ife_wdr_input_bldrto2,		 8)
		REGDEF_BIT(ife_wdr_input_bldrto3,		 8)
	REGDEF_END(IFE_WDR_INPUT_BLENDING_REGISTER0)
	
	
	/*
		ife_wdr_input_bldrto4:	  [0x0, 0xff],			bits : 7_0
		ife_wdr_input_bldrto5:	  [0x0, 0xff],			bits : 15_8
		ife_wdr_input_bldrto6:	  [0x0, 0xff],			bits : 23_16
		ife_wdr_input_bldrto7:	  [0x0, 0xff],			bits : 31_24
	*/
#define IFE_WDR_INPUT_BLENDING_REGISTER1_OFS 0x092c
	REGDEF_BEGIN(IFE_WDR_INPUT_BLENDING_REGISTER1)
		REGDEF_BIT(ife_wdr_input_bldrto4,		 8)
		REGDEF_BIT(ife_wdr_input_bldrto5,		 8)
		REGDEF_BIT(ife_wdr_input_bldrto6,		 8)
		REGDEF_BIT(ife_wdr_input_bldrto7,		 8)
	REGDEF_END(IFE_WDR_INPUT_BLENDING_REGISTER1)
	
	
	/*
		ife_wdr_input_bldrto8 :    [0x0, 0xff], 		bits : 7_0
		ife_wdr_input_bldrto9 :    [0x0, 0xff], 		bits : 15_8
		ife_wdr_input_bldrto10:    [0x0, 0xff], 		bits : 23_16
		ife_wdr_input_bldrto11:    [0x0, 0xff], 		bits : 31_24
	*/
#define IFE_WDR_INPUT_BLENDING_REGISTER2_OFS 0x0930
	REGDEF_BEGIN(IFE_WDR_INPUT_BLENDING_REGISTER2)
		REGDEF_BIT(ife_wdr_input_bldrto8 ,		  8)
		REGDEF_BIT(ife_wdr_input_bldrto9 ,		  8)
		REGDEF_BIT(ife_wdr_input_bldrto10,		  8)
		REGDEF_BIT(ife_wdr_input_bldrto11,		  8)
	REGDEF_END(IFE_WDR_INPUT_BLENDING_REGISTER2)
	
	
	/*
		ife_wdr_input_bldrto12:    [0x0, 0xff], 		bits : 7_0
		ife_wdr_input_bldrto13:    [0x0, 0xff], 		bits : 15_8
		ife_wdr_input_bldrto14:    [0x0, 0xff], 		bits : 23_16
		ife_wdr_input_bldrto15:    [0x0, 0xff], 		bits : 31_24
	*/
#define IFE_WDR_INPUT_BLENDING_REGISTER3_OFS 0x0934
	REGDEF_BEGIN(IFE_WDR_INPUT_BLENDING_REGISTER3)
		REGDEF_BIT(ife_wdr_input_bldrto12,		  8)
		REGDEF_BIT(ife_wdr_input_bldrto13,		  8)
		REGDEF_BIT(ife_wdr_input_bldrto14,		  8)
		REGDEF_BIT(ife_wdr_input_bldrto15,		  8)
	REGDEF_END(IFE_WDR_INPUT_BLENDING_REGISTER3)
	
	
	/*
		ife_wdr_input_bldrto16		  :    [0x0, 0xff], 		bits : 7_0
		ife_wdr_input_bldsrc_sel_(0~2):    [0x0, 0x3],			bits : 27_26
	*/
#define IFE_WDR_INPUT_BLENDING_REGISTER4_OFS 0x0938
	REGDEF_BEGIN(IFE_WDR_INPUT_BLENDING_REGISTER4)
		REGDEF_BIT(ife_wdr_input_bldrto16		 ,		  8)
		REGDEF_BIT( 							 ,		  18)
		REGDEF_BIT(ife_wdr_input_bldsrc_sel 	 ,		  2)
	REGDEF_END(IFE_WDR_INPUT_BLENDING_REGISTER4)
	
	
	/*
		ife_wdr_lpf_c0:    [0x0, 0x7],			bits : 2_0
		ife_wdr_lpf_c1:    [0x0, 0x7],			bits : 5_3
		ife_wdr_lpf_c2:    [0x0, 0x7],			bits : 8_6
	*/
#define IFE_WDR_SUB_IMG_LPF_REGISTER_OFS 0x093c
	REGDEF_BEGIN(IFE_WDR_SUB_IMG_LPF_REGISTER)
		REGDEF_BIT(ife_wdr_lpf_c0,		  3)
		REGDEF_BIT(ife_wdr_lpf_c1,		  3)
		REGDEF_BIT(ife_wdr_lpf_c2,		  3)
	REGDEF_END(IFE_WDR_SUB_IMG_LPF_REGISTER)
	
	
	/*
		ife_wdr_coeff1:    [0x0, 0x1fff],			bits : 12_0
		ife_wdr_coeff2:    [0x0, 0x1fff],			bits : 28_16
	*/
#define IFE_WDR_PARAMETER_REGISTER0_OFS 0x0940
	REGDEF_BEGIN(IFE_WDR_PARAMETER_REGISTER0)
		REGDEF_BIT(ife_wdr_coeff1,		  13)
		REGDEF_BIT( 			 ,		  3)
		REGDEF_BIT(ife_wdr_coeff2,		  13)
	REGDEF_END(IFE_WDR_PARAMETER_REGISTER0)
	
	
	/*
		ife_wdr_coeff3:    [0x0, 0x1fff],			bits : 12_0
		ife_wdr_coeff4:    [0x0, 0x1fff],			bits : 28_16
	*/
#define IFE_WDR_PARAMETER_REGISTER1_OFS 0x0944
	REGDEF_BEGIN(IFE_WDR_PARAMETER_REGISTER1)
		REGDEF_BIT(ife_wdr_coeff3,		  13)
		REGDEF_BIT( 			 ,		  3)
		REGDEF_BIT(ife_wdr_coeff4,		  13)
	REGDEF_END(IFE_WDR_PARAMETER_REGISTER1)
	
	
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
		ife_wdr_tcurve_val_lut_l2:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_l3:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_1_OFS 0x0950
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_1)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l2,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l3,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_1)
	
	
	/*
		ife_wdr_tcurve_val_lut_l4:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_l5:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_2_OFS 0x0954
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_2)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l4,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l5,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_2)
	
	
	/*
		ife_wdr_tcurve_val_lut_l6:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_l7:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_3_OFS 0x0958
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_3)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l6,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l7,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_3)
	
	
	/*
		ife_wdr_tcurve_val_lut_l8:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_l9:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_4_OFS 0x095c
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l8,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l9,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_4)
	
	
	/*
		ife_wdr_tcurve_val_lut_l10:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l11:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_5_OFS 0x0960
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_5)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l10,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l11,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_5)
	
	
	/*
		ife_wdr_tcurve_val_lut_l12:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l13:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_6_OFS 0x0964
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_6)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l12,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l13,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_6)
	
	
	/*
		ife_wdr_tcurve_val_lut_l14:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l15:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_7_OFS 0x0968
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_7)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l14,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l15,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_7)
	
	
	/*
		ife_wdr_tcurve_val_lut_l16:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l17:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_8_OFS 0x096c
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_8)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l16,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l17,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_8)
	
	
	/*
		ife_wdr_tcurve_val_lut_l18:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l19:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_9_OFS 0x0970
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_9)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l18,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l19,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_9)
	
	
	/*
		ife_wdr_tcurve_val_lut_l20:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l21:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_10_OFS 0x0974
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_10)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l20,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l21,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_10)
	
	
	/*
		ife_wdr_tcurve_val_lut_l22:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l23:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_11_OFS 0x0978
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_11)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l22,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l23,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_11)
	
	
	/*
		ife_wdr_tcurve_val_lut_l24:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l25:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_12_OFS 0x097c
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_12)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l24,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l25,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_12)
	
	
	/*
		ife_wdr_tcurve_val_lut_l26:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l27:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_13_OFS 0x0980
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_13)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l26,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l27,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_13)
	
	
	/*
		ife_wdr_tcurve_val_lut_l28:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l29:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_14_OFS 0x0984
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_14)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l28,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l29,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_14)
	
	
	/*
		ife_wdr_tcurve_val_lut_l30:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l31:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_15_OFS 0x0988
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_15)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l30,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l31,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_15)
	
	
	/*
		ife_wdr_tcurve_val_lut_l32:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l33:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_16_OFS 0x098c
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_16)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l32,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l33,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_16)
	
	
	/*
		ife_wdr_tcurve_val_lut_l34:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l35:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_17_OFS 0x0990
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_17)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l34,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l35,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_17)
	
	
	/*
		ife_wdr_tcurve_val_lut_l36:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l37:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_18_OFS 0x0994
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_18)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l36,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l37,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_18)
	
	
	/*
		ife_wdr_tcurve_val_lut_l38:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l39:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_19_OFS 0x0998
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_19)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l38,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l39,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_19)
	
	
	/*
		ife_wdr_tcurve_val_lut_l40:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l41:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_20_OFS 0x099c
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_20)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l40,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l41,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_20)
	
	
	/*
		ife_wdr_tcurve_val_lut_l42:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l43:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_21_OFS 0x09a0
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_21)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l42,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l43,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_21)
	
	
	/*
		ife_wdr_tcurve_val_lut_l44:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l45:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_22_OFS 0x09a4
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_22)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l44,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l45,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_22)
	
	
	/*
		ife_wdr_tcurve_val_lut_l46:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l47:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_23_OFS 0x09a8
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_23)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l46,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l47,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_23)
	
	
	/*
		ife_wdr_tcurve_val_lut_l48:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l49:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_24_OFS 0x09ac
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_24)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l48,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l49,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_24)
	
	
	/*
		ife_wdr_tcurve_val_lut_l50:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l51:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_25_OFS 0x09b0
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_25)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l50,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l51,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_25)
	
	
	/*
		ife_wdr_tcurve_val_lut_l52:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l53:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_26_OFS 0x09b4
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_26)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l52,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l53,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_26)
	
	
	/*
		ife_wdr_tcurve_val_lut_l54:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l55:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_27_OFS 0x09b8
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_27)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l54,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l55,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_27)
	
	
	/*
		ife_wdr_tcurve_val_lut_l56:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l57:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_28_OFS 0x09bc
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_28)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l56,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l57,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_28)
	
	
	/*
		ife_wdr_tcurve_val_lut_l58:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l59:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_29_OFS 0x09c0
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_29)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l58,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l59,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_29)
	
	
	/*
		ife_wdr_tcurve_val_lut_l60:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l61:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_30_OFS 0x09c4
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_30)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l60,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l61,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_30)
	
	
	/*
		ife_wdr_tcurve_val_lut_l62:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_l63:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_LEFT_REGISTER_31_OFS 0x09c8
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_LEFT_REGISTER_31)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l62,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_l63,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_LEFT_REGISTER_31)
	
	
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
		ife_wdr_tcurve_val_lut_r2:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_r3:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_1_OFS 0x09d4
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_1)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r2,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r3,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_1)
	
	
	/*
		ife_wdr_tcurve_val_lut_r4:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_r5:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_2_OFS 0x09d8
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_2)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r4,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r5,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_2)
	
	
	/*
		ife_wdr_tcurve_val_lut_r6:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_r7:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_3_OFS 0x09dc
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_3)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r6,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r7,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_3)
	
	
	/*
		ife_wdr_tcurve_val_lut_r8:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_tcurve_val_lut_r9:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_4_OFS 0x09e0
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r8,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r9,		 12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_4)
	
	
	/*
		ife_wdr_tcurve_val_lut_r10:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_r11:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_5_OFS 0x09e4
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_5)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r10,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r11,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_5)
	
	
	/*
		ife_wdr_tcurve_val_lut_r12:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_r13:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_6_OFS 0x09e8
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_6)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r12,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r13,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_6)
	
	
	/*
		ife_wdr_tcurve_val_lut_r14:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_tcurve_val_lut_r15:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_7_OFS 0x09ec
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_7)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r14,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r15,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_7)
	
	
	/*
		ife_wdr_tcurve_val_lut_r16:    [0x0, 0xfff],			bits : 11_0
	*/
#define IFE_WDR_TONE_CURVE_RIGHT_REGISTER_8_OFS 0x09f0
	REGDEF_BEGIN(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_8)
		REGDEF_BIT(ife_wdr_tcurve_val_lut_r16,		  12)
	REGDEF_END(IFE_WDR_TONE_CURVE_RIGHT_REGISTER_8)
	
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
		ife_wdr_outbld_val_lut_l2:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_l3:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_1_OFS 0x09fc
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_1)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l2,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l3,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_1)
	
	
	/*
		ife_wdr_outbld_val_lut_l4:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_l5:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_2_OFS 0x0a00
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_2)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l4,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l5,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_2)
	
	
	/*
		ife_wdr_outbld_val_lut_l6:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_l7:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_3_OFS 0x0a04
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_3)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l6,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l7,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_3)
	
	
	/*
		ife_wdr_outbld_val_lut_l8:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_l9:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_4_OFS 0x0a08
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l8,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l9,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_4)
	
	
	/*
		ife_wdr_outbld_val_lut_l10:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l11:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_5_OFS 0x0a0c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_5)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l10,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l11,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_5)
	
	
	/*
		ife_wdr_outbld_val_lut_l12:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l13:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_6_OFS 0x0a10
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_6)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l12,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l13,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_6)
	
	
	/*
		ife_wdr_outbld_val_lut_l14:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l15:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_7_OFS 0x0a14
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_7)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l14,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l15,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_7)
	
	
	/*
		ife_wdr_outbld_val_lut_l16:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l17:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_8_OFS 0x0a18
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_8)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l16,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l17,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_8)
	
	
	/*
		ife_wdr_outbld_val_lut_l18:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l19:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_9_OFS 0x0a1c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_9)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l18,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l19,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_9)
	
	
	/*
		ife_wdr_outbld_val_lut_l20:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l21:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_10_OFS 0x0a20
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_10)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l20,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l21,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_10)
	
	
	/*
		ife_wdr_outbld_val_lut_l22:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l23:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_11_OFS 0x0a24
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_11)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l22,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l23,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_11)
	
	
	/*
		ife_wdr_outbld_val_lut_l24:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l25:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_12_OFS 0x0a28
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_12)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l24,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l25,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_12)
	
	
	/*
		ife_wdr_outbld_val_lut_l26:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l27:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_13_OFS 0x0a2c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_13)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l26,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l27,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_13)
	
	
	/*
		ife_wdr_outbld_val_lut_l28:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l29:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_14_OFS 0x0a30
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_14)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l28,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l29,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_14)
	
	
	/*
		ife_wdr_outbld_val_lut_l30:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l31:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_15_OFS 0x0a34
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_15)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l30,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l31,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_15)
	
	
	/*
		ife_wdr_outbld_val_lut_l32:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l33:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_16_OFS 0x0a38
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_16)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l32,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l33,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_16)
	
	
	/*
		ife_wdr_outbld_val_lut_l34:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l35:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_17_OFS 0x0a3c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_17)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l34,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l35,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_17)
	
	
	/*
		ife_wdr_outbld_val_lut_l36:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l37:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_18_OFS 0x0a40
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_18)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l36,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l37,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_18)
	
	
	/*
		ife_wdr_outbld_val_lut_l38:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l39:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_19_OFS 0x0a44
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_19)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l38,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l39,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_19)
	
	
	/*
		ife_wdr_outbld_val_lut_l40:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l41:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_20_OFS 0x0a48
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_20)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l40,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l41,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_20)
	
	
	/*
		ife_wdr_outbld_val_lut_l42:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l43:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_21_OFS 0x0a4c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_21)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l42,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l43,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_21)
	
	
	/*
		ife_wdr_outbld_val_lut_l44:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l45:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_22_OFS 0x0a50
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_22)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l44,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l45,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_22)
	
	
	/*
		ife_wdr_outbld_val_lut_l46:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l47:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_23_OFS 0x0a54
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_23)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l46,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l47,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_23)
	
	
	/*
		ife_wdr_outbld_val_lut_l48:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l49:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_24_OFS 0x0a58
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_24)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l48,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l49,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_24)
	
	
	/*
		ife_wdr_outbld_val_lut_l50:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l51:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_25_OFS 0x0a5c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_25)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l50,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l51,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_25)
	
	
	/*
		ife_wdr_outbld_val_lut_l52:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l53:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_26_OFS 0x0a60
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_26)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l52,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l53,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_26)
	
	
	/*
		ife_wdr_outbld_val_lut_l54:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l55:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_27_OFS 0x0a64
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_27)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l54,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l55,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_27)
	
	
	/*
		ife_wdr_outbld_val_lut_l56:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l57:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_28_OFS 0x0a68
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_28)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l56,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l57,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_28)
	
	
	/*
		ife_wdr_outbld_val_lut_l58:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l59:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_29_OFS 0x0a6c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_29)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l58,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l59,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_29)
	
	
	/*
		ife_wdr_outbld_val_lut_l60:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l61:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_30_OFS 0x0a70
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_30)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l60,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l61,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_30)
	
	
	/*
		ife_wdr_outbld_val_lut_l62:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_l63:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_31_OFS 0x0a74
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_31)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l62,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_l63,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_31)
	
	
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
		ife_wdr_outbld_val_lut_r2:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_r3:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_1_OFS 0x0a80
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_1)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r2,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r3,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_1)
	
	
	/*
		ife_wdr_outbld_val_lut_r4:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_r5:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_2_OFS 0x0a84
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_2)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r4,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r5,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_2)
	
	
	/*
		ife_wdr_outbld_val_lut_r6:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_r7:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_3_OFS 0x0a88
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_3)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r6,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r7,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_3)
	
	
	/*
		ife_wdr_outbld_val_lut_r8:	  [0x0, 0xfff], 		bits : 11_0
		ife_wdr_outbld_val_lut_r9:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_4_OFS 0x0a8c
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r8,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r9,		 12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_4)
	
	
	/*
		ife_wdr_outbld_val_lut_r10:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_r11:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_5_OFS 0x0a90
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_5)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r10,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r11,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_5)
	
	
	/*
		ife_wdr_outbld_val_lut_r12:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_r13:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_6_OFS 0x0a94
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_6)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r12,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r13,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_6)
	
	
	/*
		ife_wdr_outbld_val_lut_r14:    [0x0, 0xfff],			bits : 11_0
		ife_wdr_outbld_val_lut_r15:    [0x0, 0xfff],			bits : 27_16
	*/
#define IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_7_OFS 0x0a98
	REGDEF_BEGIN(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_7)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r14,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(ife_wdr_outbld_val_lut_r15,		  12)
	REGDEF_END(IFE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_7)
	
	
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
		ife_wdr_sat_th_    :	[0x0, 0xfff],			bits : 11_0
		ife_wdr_sat_wt_low_:	[0x0, 0xff],			bits : 23_16
		ife_wdr_sat_delta_ :	[0x0, 0xff],			bits : 31_24
	*/
#define IFE_WDR_SATURATION_REDUCTION_REGISTER_OFS 0x0aa8
	REGDEF_BEGIN(IFE_WDR_SATURATION_REDUCTION_REGISTER)
		REGDEF_BIT(ife_wdr_sat_th	  , 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(ife_wdr_sat_wt_low , 	   8)
		REGDEF_BIT(ife_wdr_sat_delta  , 	   8)
	REGDEF_END(IFE_WDR_SATURATION_REDUCTION_REGISTER)
	
	
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
		ife_wdr_b2p_var:	[0x0, 0xfff],			bits : 11_0
	*/
#define IFE_WDR_TONE_MAPPING_REGISTER_1_OFS 0x0ab0
	REGDEF_BEGIN(IFE_WDR_TONE_MAPPING_REGISTER_1)
		REGDEF_BIT(ife_wdr_b2p_var, 	   12)
	REGDEF_END(IFE_WDR_TONE_MAPPING_REGISTER_1)
	
	
	/*
		ife_wdr_histogram_h_step_:	  [0x0, 0x1f],			bits : 4_0
		ife_wdr_histogram_v_step_:	  [0x0, 0x1f],			bits : 20_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER0_OFS 0x0ab4
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER0)
		REGDEF_BIT(ife_wdr_histogram_h_step ,		 5)
		REGDEF_BIT( 						,		 11)
		REGDEF_BIT(ife_wdr_histogram_v_step ,		 5)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER0)
	
	
	/*
		ife_wdr_histogram_bin0:    [0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin1:    [0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_0_OFS 0x0ab8
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_0)
		REGDEF_BIT(ife_wdr_histogram_bin0,		  16)
		REGDEF_BIT(ife_wdr_histogram_bin1,		  16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_0)
	
	
	/*
		ife_wdr_histogram_bin2:    [0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin3:    [0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_1_OFS 0x0abc
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_1)
		REGDEF_BIT(ife_wdr_histogram_bin2,		  16)
		REGDEF_BIT(ife_wdr_histogram_bin3,		  16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_1)
	
	
	/*
		ife_wdr_histogram_bin4:    [0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin5:    [0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_2_OFS 0x0ac0
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_2)
		REGDEF_BIT(ife_wdr_histogram_bin4,		  16)
		REGDEF_BIT(ife_wdr_histogram_bin5,		  16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_2)
	
	
	/*
		ife_wdr_histogram_bin6:    [0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin7:    [0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_3_OFS 0x0ac4
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_3)
		REGDEF_BIT(ife_wdr_histogram_bin6,		  16)
		REGDEF_BIT(ife_wdr_histogram_bin7,		  16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_3)
	
	
	/*
		ife_wdr_histogram_bin8:    [0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin9:    [0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_4_OFS 0x0ac8
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_4)
		REGDEF_BIT(ife_wdr_histogram_bin8,		  16)
		REGDEF_BIT(ife_wdr_histogram_bin9,		  16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_4)
	
	
	/*
		ife_wdr_histogram_bin10:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin11:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_5_OFS 0x0acc
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_5)
		REGDEF_BIT(ife_wdr_histogram_bin10, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin11, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_5)
	
	
	/*
		ife_wdr_histogram_bin12:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin13:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_6_OFS 0x0ad0
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_6)
		REGDEF_BIT(ife_wdr_histogram_bin12, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin13, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_6)
	
	
	/*
		ife_wdr_histogram_bin14:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin15:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_7_OFS 0x0ad4
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_7)
		REGDEF_BIT(ife_wdr_histogram_bin14, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin15, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_7)
	
	
	/*
		ife_wdr_histogram_bin16:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin17:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_8_OFS 0x0ad8
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_8)
		REGDEF_BIT(ife_wdr_histogram_bin16, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin17, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_8)
	
	
	/*
		ife_wdr_histogram_bin18:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin19:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_9_OFS 0x0adc
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_9)
		REGDEF_BIT(ife_wdr_histogram_bin18, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin19, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_9)
	
	
	/*
		ife_wdr_histogram_bin20:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin21:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_10_OFS 0x0ae0
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_10)
		REGDEF_BIT(ife_wdr_histogram_bin20, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin21, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_10)
	
	
	/*
		ife_wdr_histogram_bin22:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin23:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_11_OFS 0x0ae4
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_11)
		REGDEF_BIT(ife_wdr_histogram_bin22, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin23, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_11)
	
	
	/*
		ife_wdr_histogram_bin24:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin25:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_12_OFS 0x0ae8
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_12)
		REGDEF_BIT(ife_wdr_histogram_bin24, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin25, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_12)
	
	
	/*
		ife_wdr_histogram_bin26:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin27:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_13_OFS 0x0aec
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_13)
		REGDEF_BIT(ife_wdr_histogram_bin26, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin27, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_13)
	
	
	/*
		ife_wdr_histogram_bin28:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin29:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_14_OFS 0x0af0
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_14)
		REGDEF_BIT(ife_wdr_histogram_bin28, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin29, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_14)
	
	
	/*
		ife_wdr_histogram_bin30:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin31:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_15_OFS 0x0af4
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_15)
		REGDEF_BIT(ife_wdr_histogram_bin30, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin31, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_15)
	
	
	/*
		ife_wdr_histogram_bin32:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin33:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_16_OFS 0x0af8
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_16)
		REGDEF_BIT(ife_wdr_histogram_bin32, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin33, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_16)
	
	
	/*
		ife_wdr_histogram_bin34:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin35:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_17_OFS 0x0afc
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_17)
		REGDEF_BIT(ife_wdr_histogram_bin34, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin35, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_17)
	
	
	/*
		ife_wdr_histogram_bin36:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin37:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_18_OFS 0x0b00
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_18)
		REGDEF_BIT(ife_wdr_histogram_bin36, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin37, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_18)
	
	
	/*
		ife_wdr_histogram_bin38:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin39:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_19_OFS 0x0b04
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_19)
		REGDEF_BIT(ife_wdr_histogram_bin38, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin39, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_19)
	
	
	/*
		ife_wdr_histogram_bin40:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin41:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_20_OFS 0x0b08
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_20)
		REGDEF_BIT(ife_wdr_histogram_bin40, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin41, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_20)
	
	
	/*
		ife_wdr_histogram_bin42:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin43:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_21_OFS 0x0b0c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_21)
		REGDEF_BIT(ife_wdr_histogram_bin42, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin43, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_21)
	
	
	/*
		ife_wdr_histogram_bin44:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin45:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_22_OFS 0x0b10
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_22)
		REGDEF_BIT(ife_wdr_histogram_bin44, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin45, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_22)
	
	
	/*
		ife_wdr_histogram_bin46:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin47:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_23_OFS 0x0b14
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_23)
		REGDEF_BIT(ife_wdr_histogram_bin46, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin47, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_23)
	
	
	/*
		ife_wdr_histogram_bin48:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin49:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_24_OFS 0x0b18
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_24)
		REGDEF_BIT(ife_wdr_histogram_bin48, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin49, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_24)
	
	
	/*
		ife_wdr_histogram_bin50:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin51:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_25_OFS 0x0b1c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_25)
		REGDEF_BIT(ife_wdr_histogram_bin50, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin51, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_25)
	
	
	/*
		ife_wdr_histogram_bin52:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin53:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_26_OFS 0x0b20
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_26)
		REGDEF_BIT(ife_wdr_histogram_bin52, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin53, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_26)
	
	
	/*
		ife_wdr_histogram_bin54:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin55:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_27_OFS 0x0b24
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_27)
		REGDEF_BIT(ife_wdr_histogram_bin54, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin55, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_27)
	
	
	/*
		ife_wdr_histogram_bin56:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin57:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_28_OFS 0x0b28
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_28)
		REGDEF_BIT(ife_wdr_histogram_bin56, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin57, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_28)
	
	
	/*
		ife_wdr_histogram_bin58:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin59:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_29_OFS 0x0b2c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_29)
		REGDEF_BIT(ife_wdr_histogram_bin58, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin59, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_29)
	
	
	/*
		ife_wdr_histogram_bin60:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin61:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_30_OFS 0x0b30
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_30)
		REGDEF_BIT(ife_wdr_histogram_bin60, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin61, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_30)
	
	
	/*
		ife_wdr_histogram_bin62:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin63:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_31_OFS 0x0b34
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_31)
		REGDEF_BIT(ife_wdr_histogram_bin62, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin63, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_31)
	
	
	/*
		ife_wdr_histogram_bin64:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin65:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_32_OFS 0x0b38
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_32)
		REGDEF_BIT(ife_wdr_histogram_bin64, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin65, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_32)
	
	
	/*
		ife_wdr_histogram_bin66:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin67:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_33_OFS 0x0b3c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_33)
		REGDEF_BIT(ife_wdr_histogram_bin66, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin67, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_33)
	
	
	/*
		ife_wdr_histogram_bin68:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin69:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_34_OFS 0x0b40
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_34)
		REGDEF_BIT(ife_wdr_histogram_bin68, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin69, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_34)
	
	
	/*
		ife_wdr_histogram_bin70:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin71:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_35_OFS 0x0b44
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_35)
		REGDEF_BIT(ife_wdr_histogram_bin70, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin71, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_35)
	
	
	/*
		ife_wdr_histogram_bin72:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin73:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_36_OFS 0x0b48
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_36)
		REGDEF_BIT(ife_wdr_histogram_bin72, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin73, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_36)
	
	
	/*
		ife_wdr_histogram_bin74:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin75:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_37_OFS 0x0b4c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_37)
		REGDEF_BIT(ife_wdr_histogram_bin74, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin75, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_37)
	
	
	/*
		ife_wdr_histogram_bin76:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin77:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_38_OFS 0x0b50
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_38)
		REGDEF_BIT(ife_wdr_histogram_bin76, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin77, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_38)
	
	
	/*
		ife_wdr_histogram_bin78:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin79:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_39_OFS 0x0b54
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_39)
		REGDEF_BIT(ife_wdr_histogram_bin78, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin79, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_39)
	
	
	/*
		ife_wdr_histogram_bin80:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin81:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_40_OFS 0x0b58
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_40)
		REGDEF_BIT(ife_wdr_histogram_bin80, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin81, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_40)
	
	
	/*
		ife_wdr_histogram_bin82:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin83:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_41_OFS 0x0b5c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_41)
		REGDEF_BIT(ife_wdr_histogram_bin82, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin83, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_41)
	
	
	/*
		ife_wdr_histogram_bin84:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin85:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_42_OFS 0x0b60
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_42)
		REGDEF_BIT(ife_wdr_histogram_bin84, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin85, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_42)
	
	
	/*
		ife_wdr_histogram_bin86:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin87:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_43_OFS 0x0b64
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_43)
		REGDEF_BIT(ife_wdr_histogram_bin86, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin87, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_43)
	
	
	/*
		ife_wdr_histogram_bin88:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin89:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_44_OFS 0x0b68
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_44)
		REGDEF_BIT(ife_wdr_histogram_bin88, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin89, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_44)
	
	
	/*
		ife_wdr_histogram_bin90:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin91:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_45_OFS 0x0b6c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_45)
		REGDEF_BIT(ife_wdr_histogram_bin90, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin91, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_45)
	
	
	/*
		ife_wdr_histogram_bin92:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin93:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_46_OFS 0x0b70
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_46)
		REGDEF_BIT(ife_wdr_histogram_bin92, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin93, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_46)
	
	
	/*
		ife_wdr_histogram_bin94:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin95:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_47_OFS 0x0b74
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_47)
		REGDEF_BIT(ife_wdr_histogram_bin94, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin95, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_47)
	
	
	/*
		ife_wdr_histogram_bin96:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin97:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_48_OFS 0x0b78
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_48)
		REGDEF_BIT(ife_wdr_histogram_bin96, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin97, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_48)
	
	
	/*
		ife_wdr_histogram_bin98:	[0x0, 0xffff],			bits : 15_0
		ife_wdr_histogram_bin99:	[0x0, 0xffff],			bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_49_OFS 0x0b7c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_49)
		REGDEF_BIT(ife_wdr_histogram_bin98, 	   16)
		REGDEF_BIT(ife_wdr_histogram_bin99, 	   16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_49)
	
	
	/*
		ife_wdr_histogram_bin100:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin101:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_50_OFS 0x0b80
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_50)
		REGDEF_BIT(ife_wdr_histogram_bin100,		16)
		REGDEF_BIT(ife_wdr_histogram_bin101,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_50)
	
	
	/*
		ife_wdr_histogram_bin102:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin103:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_51_OFS 0x0b84
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_51)
		REGDEF_BIT(ife_wdr_histogram_bin102,		16)
		REGDEF_BIT(ife_wdr_histogram_bin103,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_51)
	
	
	/*
		ife_wdr_histogram_bin104:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin105:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_52_OFS 0x0b88
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_52)
		REGDEF_BIT(ife_wdr_histogram_bin104,		16)
		REGDEF_BIT(ife_wdr_histogram_bin105,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_52)
	
	
	/*
		ife_wdr_histogram_bin106:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin107:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_53_OFS 0x0b8c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_53)
		REGDEF_BIT(ife_wdr_histogram_bin106,		16)
		REGDEF_BIT(ife_wdr_histogram_bin107,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_53)
	
	
	/*
		ife_wdr_histogram_bin108:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin109:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_54_OFS 0x0b90
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_54)
		REGDEF_BIT(ife_wdr_histogram_bin108,		16)
		REGDEF_BIT(ife_wdr_histogram_bin109,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_54)
	
	
	/*
		ife_wdr_histogram_bin110:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin111:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_55_OFS 0x0b94
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_55)
		REGDEF_BIT(ife_wdr_histogram_bin110,		16)
		REGDEF_BIT(ife_wdr_histogram_bin111,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_55)
	
	
	/*
		ife_wdr_histogram_bin112:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin113:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_56_OFS 0x0b98
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_56)
		REGDEF_BIT(ife_wdr_histogram_bin112,		16)
		REGDEF_BIT(ife_wdr_histogram_bin113,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_56)
	
	
	/*
		ife_wdr_histogram_bin114:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin115:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_57_OFS 0x0b9c
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_57)
		REGDEF_BIT(ife_wdr_histogram_bin114,		16)
		REGDEF_BIT(ife_wdr_histogram_bin115,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_57)
	
	
	/*
		ife_wdr_histogram_bin116:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin117:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_58_OFS 0x0ba0
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_58)
		REGDEF_BIT(ife_wdr_histogram_bin116,		16)
		REGDEF_BIT(ife_wdr_histogram_bin117,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_58)
	
	
	/*
		ife_wdr_histogram_bin118:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin119:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_59_OFS 0x0ba4
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_59)
		REGDEF_BIT(ife_wdr_histogram_bin118,		16)
		REGDEF_BIT(ife_wdr_histogram_bin119,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_59)
	
	
	/*
		ife_wdr_histogram_bin120:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin121:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_60_OFS 0x0ba8
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_60)
		REGDEF_BIT(ife_wdr_histogram_bin120,		16)
		REGDEF_BIT(ife_wdr_histogram_bin121,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_60)
	
	
	/*
		ife_wdr_histogram_bin122:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin123:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_61_OFS 0x0bac
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_61)
		REGDEF_BIT(ife_wdr_histogram_bin122,		16)
		REGDEF_BIT(ife_wdr_histogram_bin123,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_61)
	
	
	/*
		ife_wdr_histogram_bin124:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin125:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_62_OFS 0x0bb0
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_62)
		REGDEF_BIT(ife_wdr_histogram_bin124,		16)
		REGDEF_BIT(ife_wdr_histogram_bin125,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_62)
	
	
	/*
		ife_wdr_histogram_bin126:	 [0x0, 0xffff], 		bits : 15_0
		ife_wdr_histogram_bin127:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define IFE_WDR_HISTOGRAM_REGISTER_63_OFS 0x0bb4
	REGDEF_BEGIN(IFE_WDR_HISTOGRAM_REGISTER_63)
		REGDEF_BIT(ife_wdr_histogram_bin126,		16)
		REGDEF_BIT(ife_wdr_histogram_bin127,		16)
	REGDEF_END(IFE_WDR_HISTOGRAM_REGISTER_63)
	
	
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
	
	
	/*
		ife_wdr_fbc_th1:	[0x0, 0x1fff],			bits : 12_0
		ife_wdr_fbc_th2:	[0x0, 0x1fff],			bits : 28_16
	*/
#define IFE_WDR_FBC_REGISTER_1_OFS 0x0bbc
	REGDEF_BEGIN(IFE_WDR_FBC_REGISTER_1)
		REGDEF_BIT(ife_wdr_fbc_th1, 	   13)
		REGDEF_BIT( 			  , 	   3)
		REGDEF_BIT(ife_wdr_fbc_th2, 	   13)
	REGDEF_END(IFE_WDR_FBC_REGISTER_1)
	
	
	/*
		ife_wdr_fbc_th3:	[0x0, 0x1fff],			bits : 12_0
		ife_wdr_fbc_th4:	[0x0, 0x1fff],			bits : 28_16
	*/
#define IFE_WDR_FBC_REGISTER_2_OFS 0x0bc0
	REGDEF_BEGIN(IFE_WDR_FBC_REGISTER_2)
		REGDEF_BIT(ife_wdr_fbc_th3, 	   13)
		REGDEF_BIT( 			  , 	   3)
		REGDEF_BIT(ife_wdr_fbc_th4, 	   13)
	REGDEF_END(IFE_WDR_FBC_REGISTER_2)
	
	
	/*
		ife_wdr_fbc_th5:	[0x0, 0x1fff],			bits : 12_0
	*/
#define IFE_WDR_FBC_REGISTER_3_OFS 0x0bc4
	REGDEF_BEGIN(IFE_WDR_FBC_REGISTER_3)
		REGDEF_BIT(ife_wdr_fbc_th5, 	   13)
	REGDEF_END(IFE_WDR_FBC_REGISTER_3)
	
	
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
		ife_wdr_input_yv_bld_lut4:	  [0x0, 0x3f],			bits : 5_0
		ife_wdr_input_yv_bld_lut5:	  [0x0, 0x3f],			bits : 13_8
		ife_wdr_input_yv_bld_lut6:	  [0x0, 0x3f],			bits : 21_16
		ife_wdr_input_yv_bld_lut7:	  [0x0, 0x3f],			bits : 29_24
	*/
#define IFE_WDR_YV_BLD_REGISTER_1_OFS 0x0bcc
	REGDEF_BEGIN(IFE_WDR_YV_BLD_REGISTER_1)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut4,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut5,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut6,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut7,		 6)
	REGDEF_END(IFE_WDR_YV_BLD_REGISTER_1)
	
	
	/*
		ife_wdr_input_yv_bld_lut8:	  [0x0, 0x3f],			bits : 5_0
	*/
#define IFE_WDR_YV_BLD_REGISTER_2_OFS 0x0bd0
	REGDEF_BEGIN(IFE_WDR_YV_BLD_REGISTER_2)
		REGDEF_BIT(ife_wdr_input_yv_bld_lut8,		 6)
	REGDEF_END(IFE_WDR_YV_BLD_REGISTER_2)
	
	
	
	/*
		vdetgh2_iir2_a:    [0x0, 0x3ff],			bits : 9_0
		vdetgh2_iir2_b:    [0x0, 0x3ff],			bits : 19_10
		vdetgh2_iir2_e:    [0x0, 0x3ff],			bits : 29_20
	*/
#define VA_REGISTER81_OFS 0x0bf4
	REGDEF_BEGIN(VA_REGISTER81)
		REGDEF_BIT(vdetgh2_iir2_a,		  10)
		REGDEF_BIT(vdetgh2_iir2_b,		  10)
		REGDEF_BIT(vdetgh2_iir2_e,		  10)
	REGDEF_END(VA_REGISTER81)
	
	
	/*
		vdetgh2_iir2_f:    [0x0, 0x3ff],			bits : 9_0
		vdetgh2_iir3_a:    [0x0, 0x3ff],			bits : 19_10
		vdetgh2_iir3_b:    [0x0, 0x3ff],			bits : 29_20
	*/
#define VA_REGISTER82_OFS 0x0bf8
	REGDEF_BEGIN(VA_REGISTER82)
		REGDEF_BIT(vdetgh2_iir2_f,		  10)
		REGDEF_BIT(vdetgh2_iir3_a,		  10)
		REGDEF_BIT(vdetgh2_iir3_b,		  10)
	REGDEF_END(VA_REGISTER82)
	
	
	/*
		vdetgh2_iir3_e:    [0x0, 0x3ff],			bits : 9_0
		vdetgh2_iir3_f:    [0x0, 0x3ff],			bits : 19_10
	*/
#define VA_REGISTER83_OFS 0x0bfc
	REGDEF_BEGIN(VA_REGISTER83)
		REGDEF_BIT(vdetgh2_iir3_e,		  10)
		REGDEF_BIT(vdetgh2_iir3_f,		  10)
	REGDEF_END(VA_REGISTER83)
	
	
	/*
		vacc_en 				:	 [0x0, 0x1],			bits : 0
		win0_vaen				:	 [0x0, 0x1],			bits : 1
		win1_vaen				:	 [0x0, 0x1],			bits : 2
		win2_vaen				:	 [0x0, 0x1],			bits : 3
		win3_vaen				:	 [0x0, 0x1],			bits : 4
		win4_vaen				:	 [0x0, 0x1],			bits : 5
		va_pre_filter_mode_(0~2):	 [0x0, 0x3],			bits : 7_6
		va_gamma_sel			:	 [0x0, 0x1],			bits : 8
		va_win_ldg_en			:	 [0x0, 0x1],			bits : 9
	*/
#define VA_REGISTER1_OFS 0x0c00
	REGDEF_BEGIN(VA_REGISTER1)
		REGDEF_BIT(vacc_en				   ,		1)
		REGDEF_BIT(win0_vaen			   ,		1)
		REGDEF_BIT(win1_vaen			   ,		1)
		REGDEF_BIT(win2_vaen			   ,		1)
		REGDEF_BIT(win3_vaen			   ,		1)
		REGDEF_BIT(win4_vaen			   ,		1)
		REGDEF_BIT(va_pre_filter_mode	   ,		2)
		REGDEF_BIT(va_gamma_sel 		   ,		1)
		REGDEF_BIT(va_win_ldg_en		   ,		1)
	REGDEF_END(VA_REGISTER1)
	
	
	/*
		dram_sao_va:	[0x0, 0x3fffffff],			bits : 31_2
	*/
#define VA_REGISTER2_OFS 0x0c04
	REGDEF_BEGIN(VA_REGISTER2)
		REGDEF_BIT( 		  , 	   2)
		REGDEF_BIT(dram_sao_va, 	   30)
	REGDEF_END(VA_REGISTER2)
	
	
	/*
		dram_sao_va_msb:	[0x0, 0xf], 		bits : 3_0
	*/
#define VA_REGISTER3_OFS 0x0c08
	REGDEF_BEGIN(VA_REGISTER3)
		REGDEF_BIT(dram_sao_va_msb, 	   4)
	REGDEF_END(VA_REGISTER3)
	
	
	/*
		dram_ofso_va:	 [0x0, 0x3fff], 		bits : 15_2
	*/
#define VA_REGISTER4_OFS 0x0c0c
	REGDEF_BEGIN(VA_REGISTER4)
		REGDEF_BIT( 		   ,		2)
		REGDEF_BIT(dram_ofso_va,		14)
	REGDEF_END(VA_REGISTER4)
	
	
	/*
		va_win_cnt_out_sel :	[0x0, 0x1], 		bits : 0
		va_win_high_luma_th:	[0x0, 0xff],			bits : 11_4
		va_energy_w_(0~16) :	[0x0, 0x1f],			bits : 16_12
	*/
#define VA_REGISTER5_OFS 0x0c10
	REGDEF_BEGIN(VA_REGISTER5)
		REGDEF_BIT(va_win_cnt_out_sel , 	   1)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(va_win_high_luma_th, 	   8)
		REGDEF_BIT(va_energy_w		  , 	   5)
	REGDEF_END(VA_REGISTER5)
	
	
	/*
		vdetgh1_filter_select		:	 [0x0, 0x1],			bits : 0
		vdetgh1_iir_input_sel       :    [0x0, 0x1],			bits : 1
		vdetgh1_iir_shift_bit_(0~15):	 [0x0, 0xf],			bits : 6_3
		vdetgh1_bcd_op				:	 [0x0, 0x1],			bits : 7
		vdetgh1_iir2_shift_bit		:	 [0x0, 0xf],			bits : 11_8
		vdetgh1_iir2_bcd_op 		:	 [0x0, 0x1],			bits : 12
		vdetgh1_iir3_shift_bit		:	 [0x0, 0xf],			bits : 16_13
		vdetgh1_iir3_bcd_op 		:	 [0x0, 0x1],			bits : 17
		vdetgh1_iir2_en 			:	 [0x0, 0x1],			bits : 18
		vdetgh1_iir3_en 			:	 [0x0, 0x1],			bits : 19
		vdetgh1_fsize				:	 [0x0, 0x3],			bits : 25_24
		vdetgh1_div 				:	 [0x0, 0xf],			bits : 31_28
	*/
#define VA_REGISTER6_OFS 0x0c14
	REGDEF_BEGIN(VA_REGISTER6)
		REGDEF_BIT(vdetgh1_filter_select	   ,		1)
		REGDEF_BIT(vdetgh1_iir_input_sel       ,        1)
		REGDEF_BIT( 						   ,		1)
		REGDEF_BIT(vdetgh1_iir_shift_bit	   ,		4)
		REGDEF_BIT(vdetgh1_bcd_op			   ,		1)
		REGDEF_BIT(vdetgh1_iir2_shift_bit	   ,		4)
		REGDEF_BIT(vdetgh1_iir2_bcd_op		   ,		1)
		REGDEF_BIT(vdetgh1_iir3_shift_bit	   ,		4)
		REGDEF_BIT(vdetgh1_iir3_bcd_op		   ,		1)
		REGDEF_BIT(vdetgh1_iir2_en			   ,		1)
		REGDEF_BIT(vdetgh1_iir3_en			   ,		1)
		REGDEF_BIT( 						   ,		4)
		REGDEF_BIT(vdetgh1_fsize			   ,		2)
		REGDEF_BIT( 						   ,		2)
		REGDEF_BIT(vdetgh1_div				   ,		4)
	REGDEF_END(VA_REGISTER6)
	
	
	/*
		vdetgv1a_(0~31)  :	  [0x0, 0x1f],			bits : 4_0
		vdetgv1_bcd_op	 :	  [0x0, 0x1],			bits : 7
		vdetgv1b_(-16~15):	  [0x0, 0x1f],			bits : 12_8
		vdetgv1c_(-8~7)  :	  [0x0, 0xf],			bits : 19_16
		vdetgv1d_(-8~7)  :	  [0x0, 0xf],			bits : 23_20
		vdetgv1_fsize	 :	  [0x0, 0x3],			bits : 25_24
		vdetgv1_div 	 :	  [0x0, 0xf],			bits : 31_28
	*/
#define VA_REGISTER7_OFS 0x0c18
	REGDEF_BEGIN(VA_REGISTER7)
		REGDEF_BIT(vdetgv1a 		,		 5)
		REGDEF_BIT( 				,		 2)
		REGDEF_BIT(vdetgv1_bcd_op	,		 1)
		REGDEF_BIT(vdetgv1b 		,		 5)
		REGDEF_BIT( 				,		 3)
		REGDEF_BIT(vdetgv1c 		,		 4)
		REGDEF_BIT(vdetgv1d 		,		 4)
		REGDEF_BIT(vdetgv1_fsize	,		 2)
		REGDEF_BIT( 				,		 2)
		REGDEF_BIT(vdetgv1_div		,		 4)
	REGDEF_END(VA_REGISTER7)
	
	
	/*
		vdetgh2_filter_select		:	 [0x0, 0x1],			bits : 0
		vdetgh2_iir_input_sel       :	 [0x0, 0x1],			bits : 1
		vdetgh2_iir_shift_bit		:	 [0x0, 0xf],			bits : 6_3
		vdetgh2_bcd_op				:	 [0x0, 0x1],			bits : 7
		vdetgh2_iir2_shift_bit		:	 [0x0, 0xf],			bits : 11_8
		vdetgh2_iir2_bcd_op 		:	 [0x0, 0x1],			bits : 12
		vdetgh2_iir3_shift_bit		:	 [0x0, 0xf],			bits : 16_13
		vdetgh2_iir3_bcd_op 		:	 [0x0, 0x1],			bits : 17
		vdetgh2_iir2_en 			:	 [0x0, 0x1],			bits : 18
		vdetgh2_iir3_en 			:	 [0x0, 0x1],			bits : 19
		vdetgh2_fsize				:	 [0x0, 0x3],			bits : 25_24
		vdetgh2_div 				:	 [0x0, 0xf],			bits : 31_28
	*/
#define VA_REGISTER8_OFS 0x0c1c
	REGDEF_BEGIN(VA_REGISTER8)
		REGDEF_BIT(vdetgh2_filter_select	   ,		1)
		REGDEF_BIT(vdetgh2_iir_input_sel	   ,		1)
		REGDEF_BIT( 						   ,		1)
		REGDEF_BIT(vdetgh2_iir_shift_bit	   ,		4)
		REGDEF_BIT(vdetgh2_bcd_op			   ,		1)
		REGDEF_BIT(vdetgh2_iir2_shift_bit	   ,		4)
		REGDEF_BIT(vdetgh2_iir2_bcd_op		   ,		1)
		REGDEF_BIT(vdetgh2_iir3_shift_bit	   ,		4)
		REGDEF_BIT(vdetgh2_iir3_bcd_op		   ,		1)
		REGDEF_BIT(vdetgh2_iir2_en			   ,		1)
		REGDEF_BIT(vdetgh2_iir3_en			   ,		1)
		REGDEF_BIT( 						   ,		4)
		REGDEF_BIT(vdetgh2_fsize			   ,		2)
		REGDEF_BIT( 						   ,		2)
		REGDEF_BIT(vdetgh2_div				   ,		4)
	REGDEF_END(VA_REGISTER8)
	
	
	/*
		vdetgv2a_(0~31)  :	  [0x0, 0x1f],			bits : 4_0
		vdetgv2_bcd_op	 :	  [0x0, 0x1],			bits : 7
		vdetgv2b_(-16~15):	  [0x0, 0x1f],			bits : 12_8
		vdetgv2c_(-8~7)  :	  [0x0, 0xf],			bits : 19_16
		vdetgv2d_(-8~7)  :	  [0x0, 0xf],			bits : 23_20
		vdetgv2_fsize	 :	  [0x0, 0x3],			bits : 25_24
		vdetgv2_div 	 :	  [0x0, 0xf],			bits : 31_28
	*/
#define VA_REGISTER9_OFS 0x0c20
	REGDEF_BEGIN(VA_REGISTER9)
		REGDEF_BIT(vdetgv2a 		,		 5)
		REGDEF_BIT( 				,		 2)
		REGDEF_BIT(vdetgv2_bcd_op	,		 1)
		REGDEF_BIT(vdetgv2b 		,		 5)
		REGDEF_BIT( 				,		 3)
		REGDEF_BIT(vdetgv2c 		,		 4)
		REGDEF_BIT(vdetgv2d 		,		 4)
		REGDEF_BIT(vdetgv2_fsize	,		 2)
		REGDEF_BIT( 				,		 2)
		REGDEF_BIT(vdetgv2_div		,		 4)
	REGDEF_END(VA_REGISTER9)
	
	
	/*
		vacc_outsel:	[0x0, 0x1], 		bits : 0
		va_stx	   :	[0x0, 0x3fff],			bits : 16_3
		va_sty	   :	[0x0, 0x3fff],			bits : 30_17
	*/
#define VA_REGISTER10_OFS 0x0c24
	REGDEF_BEGIN(VA_REGISTER10)
		REGDEF_BIT(vacc_outsel, 	   1)
		REGDEF_BIT( 		  , 	   2)
		REGDEF_BIT(va_stx	  , 	   14)
		REGDEF_BIT(va_sty	  , 	   14)
	REGDEF_END(VA_REGISTER10)
	
	
	/*
		va_g1hthl:	  [0x0, 0xff],			bits : 11_0
		va_g1hthh:	  [0x0, 0xff],			bits : 27_16
	*/
#define VA_REGISTER11_OFS 0x0c28
	REGDEF_BEGIN(VA_REGISTER11)
		REGDEF_BIT(va_g1hthl,		 12)
		REGDEF_BIT(,		 3)
		REGDEF_BIT(va_g1hthh,		 12)
	REGDEF_END(VA_REGISTER11)
	
	
	/*
		va_g2hthl:	  [0x0, 0xff],			bits : 11_0
		va_g2hthh:	  [0x0, 0xff],			bits : 27_16
	*/
#define VA_REGISTER12_OFS 0x0c2c
	REGDEF_BEGIN(VA_REGISTER12)
		REGDEF_BIT(va_g2hthl,		 12)
		REGDEF_BIT(,		 3)
		REGDEF_BIT(va_g2hthh,		 12)
	REGDEF_END(VA_REGISTER12)
	
	
	/*
		va_win_szx	:	 [0x0, 0x3ff],			bits : 10_0
		va_win_szy	:	 [0x0, 0x3ff],			bits : 22_12
		va_g1_cnt_en:	 [0x0, 0x1],			bits : 30
		va_g2_cnt_en:	 [0x0, 0x1],			bits : 31
	*/
#define VA_REGISTER13_OFS 0x0c30
	REGDEF_BEGIN(VA_REGISTER13)
		REGDEF_BIT(va_win_szx  ,		11)
		REGDEF_BIT( 		   ,		1)
		REGDEF_BIT(va_win_szy  ,		11)
		REGDEF_BIT( 		   ,		7)
		REGDEF_BIT(va_g1_cnt_en,		1)
		REGDEF_BIT(va_g2_cnt_en,		1)
	REGDEF_END(VA_REGISTER13)
	
	
	/*
		va_win_numx :	 [0x0, 0x7],			bits : 2_0
		va_win_numy :	 [0x0, 0x7],			bits : 6_4
		va_win_skipx:	 [0x0, 0x3f],			bits : 21_16
		va_win_skipy:	 [0x0, 0x3f],			bits : 29_24
	*/
#define VA_REGISTER14_OFS 0x0c34
	REGDEF_BEGIN(VA_REGISTER14)
		REGDEF_BIT(va_win_numx ,		3)
		REGDEF_BIT(,		1)		
		REGDEF_BIT(va_win_numy ,		3)
		REGDEF_BIT( 		   ,		9)
		REGDEF_BIT(va_win_skipx,		6)
		REGDEF_BIT( 		   ,		2)
		REGDEF_BIT(va_win_skipy,		6)
	REGDEF_END(VA_REGISTER14)
	
	
	/*
		win0_stx:	 [0x0, 0x3fff], 		bits : 12_0
		win0_sty:	 [0x0, 0x3fff], 		bits : 28_16
	*/
#define VA_REGISTER15_OFS 0x0c38
	REGDEF_BEGIN(VA_REGISTER15)
		REGDEF_BIT(win0_stx,		13)
		REGDEF_BIT( 	   ,		3)
		REGDEF_BIT(win0_sty,		13)
	REGDEF_END(VA_REGISTER15)
	
	
	/*
		win0_hsz:	 [0x0, 0x3ff],			bits : 10_0
		win0_vsz:	 [0x0, 0x3ff],			bits : 26_16
	*/
#define VA_REGISTER16_OFS 0x0c3c
	REGDEF_BEGIN(VA_REGISTER16)
		REGDEF_BIT(win0_hsz,		11)
		REGDEF_BIT( 	   ,		5)
		REGDEF_BIT(win0_vsz,		11)
	REGDEF_END(VA_REGISTER16)
	
	
	/*
		win1_stx:	 [0x0, 0x1fff], 		bits : 12_0
		win1_sty:	 [0x0, 0x1fff], 		bits : 28_16
	*/
#define VA_REGISTER17_OFS 0x0c40
	REGDEF_BEGIN(VA_REGISTER17)
		REGDEF_BIT(win1_stx,		13)
		REGDEF_BIT( 	   ,		3)
		REGDEF_BIT(win1_sty,		13)
	REGDEF_END(VA_REGISTER17)
	
	
	/*
		win1_hsz:	 [0x0, 0x7ff],			bits : 10_0
		win1_vsz:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define VA_REGISTER18_OFS 0x0c44
	REGDEF_BEGIN(VA_REGISTER18)
		REGDEF_BIT(win1_hsz,		11)
		REGDEF_BIT( 	   ,		5)
		REGDEF_BIT(win1_vsz,		11)
	REGDEF_END(VA_REGISTER18)
	
	
	/*
		win2_stx:	 [0x0, 0x1fff], 		bits : 13_0
		win2_sty:	 [0x0, 0x1fff], 		bits : 29_16
	*/
#define VA_REGISTER19_OFS 0x0c48
	REGDEF_BEGIN(VA_REGISTER19)
		REGDEF_BIT(win2_stx,		13)
		REGDEF_BIT( 	   ,		3)
		REGDEF_BIT(win2_sty,		13)
	REGDEF_END(VA_REGISTER19)
	
	
	/*
		win2_hsz:	 [0x0, 0x7ff],			bits : 10_0
		win2_vsz:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define VA_REGISTER20_OFS 0x0c4c
	REGDEF_BEGIN(VA_REGISTER20)
		REGDEF_BIT(win2_hsz,		11)
		REGDEF_BIT( 	   ,		5)
		REGDEF_BIT(win2_vsz,		11)
	REGDEF_END(VA_REGISTER20)
	
	
	/*
		win3_stx:	 [0x0, 0x1fff], 		bits : 12_0
		win3_sty:	 [0x0, 0x1fff], 		bits : 28_16
	*/
#define VA_REGISTER21_OFS 0x0c50
	REGDEF_BEGIN(VA_REGISTER21)
		REGDEF_BIT(win3_stx,		13)
		REGDEF_BIT( 	   ,		3)
		REGDEF_BIT(win3_sty,		13)
	REGDEF_END(VA_REGISTER21)
	
	
	/*
		win3_hsz:	 [0x0, 0x7ff],			bits : 10_0
		win3_vsz:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define VA_REGISTER22_OFS 0x0c54
	REGDEF_BEGIN(VA_REGISTER22)
		REGDEF_BIT(win3_hsz,		11)
		REGDEF_BIT( 	   ,		5)
		REGDEF_BIT(win3_vsz,		11)
	REGDEF_END(VA_REGISTER22)
	
	
	/*
		win4_stx:	 [0x0, 0x1fff], 		bits : 12_0
		win4_sty:	 [0x0, 0x1fff], 		bits : 28_16
	*/
#define VA_REGISTER23_OFS 0x0c58
	REGDEF_BEGIN(VA_REGISTER23)
		REGDEF_BIT(win4_stx,		13)
		REGDEF_BIT( 	   ,		3)
		REGDEF_BIT(win4_sty,		13)
	REGDEF_END(VA_REGISTER23)
	
	
	/*
		win4_hsz:	 [0x0, 0x3ff],			bits : 10_0
		win4_vsz:	 [0x0, 0x3ff],			bits : 26_16
	*/
#define VA_REGISTER24_OFS 0x0c5c
	REGDEF_BEGIN(VA_REGISTER24)
		REGDEF_BIT(win4_hsz,		11)
		REGDEF_BIT( 	   ,		5)
		REGDEF_BIT(win4_vsz,		11)
	REGDEF_END(VA_REGISTER24)
	
	
	/*
		va_win0g1h_vacc:	[0x0, 0xffffffff],			bits : 31_0
	*/
#define VA_REGISTER25_OFS 0x0c60
	REGDEF_BEGIN(VA_REGISTER25)
		REGDEF_BIT(va_win0_lumacc,		  32)
	REGDEF_END(VA_REGISTER25)
	
	
	/*
		va_win0G1h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win0G1h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER26_OFS 0x0c64
	REGDEF_BEGIN(VA_REGISTER26)
		REGDEF_BIT(va_win0g1h_vacc ,		16)
		REGDEF_BIT(va_win0g1h_vacnt,		16)
	REGDEF_END(VA_REGISTER26)
	
	
	/*
		va_win0G1v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win0G1v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER27_OFS 0x0c68
	REGDEF_BEGIN(VA_REGISTER27)
		REGDEF_BIT(va_win0g1v_vacc ,		16)
		REGDEF_BIT(va_win0g1v_vacnt,		16)
	REGDEF_END(VA_REGISTER27)
	
	
	/*
		va_win0G2h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win0G2h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER28_OFS 0x0c6c
	REGDEF_BEGIN(VA_REGISTER28)
		REGDEF_BIT(va_win0g2h_vacc ,		16)
		REGDEF_BIT(va_win0g2h_vacnt,		16)
	REGDEF_END(VA_REGISTER28)
	
	
	/*
		va_win0G2v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win0G2v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER29_OFS 0x0c70
	REGDEF_BEGIN(VA_REGISTER29)
		REGDEF_BIT(va_win0g2v_vacc ,		16)
		REGDEF_BIT(va_win0g2v_vacnt,		16)
	REGDEF_END(VA_REGISTER29)
	
	
	/*
		va_win1_lumacc:    [0x0, 0xffffffff],			bits : 31_0
	*/
#define VA_REGISTER30_OFS 0x0c74
	REGDEF_BEGIN(VA_REGISTER30)
		REGDEF_BIT(va_win1_lumacc,		  32)
	REGDEF_END(VA_REGISTER30)
	
	
	/*
		va_win1G1h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win1G1h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER31_OFS 0x0c78
	REGDEF_BEGIN(VA_REGISTER31)
		REGDEF_BIT(va_win1g1h_vacc ,		16)
		REGDEF_BIT(va_win1g1h_vacnt,		16)
	REGDEF_END(VA_REGISTER31)
	
	
	/*
		va_win1G1v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win1G1v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER32_OFS 0x0c7c
	REGDEF_BEGIN(VA_REGISTER32)
		REGDEF_BIT(va_win1g1v_vacc ,		16)
		REGDEF_BIT(va_win1g1v_vacnt,		16)
	REGDEF_END(VA_REGISTER32)
	
	
	/*
		va_win1G2h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win1G2h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER33_OFS 0x0c80
	REGDEF_BEGIN(VA_REGISTER33)
		REGDEF_BIT(va_win1G2h_vacc ,		16)
		REGDEF_BIT(va_win1G2h_vacnt,		16)
	REGDEF_END(VA_REGISTER33)
	
	
	/*
		va_win1G2v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win1G2v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER34_OFS 0x0c84
	REGDEF_BEGIN(VA_REGISTER34)
		REGDEF_BIT(va_win1g2v_vacc ,		16)
		REGDEF_BIT(va_win1g2v_vacnt,		16)
	REGDEF_END(VA_REGISTER34)
	
	
	/*
		va_win2_lumacc:    [0x0, 0xffffffff],			bits : 31_0
	*/
#define VA_REGISTER35_OFS 0x0c88
	REGDEF_BEGIN(VA_REGISTER35)
		REGDEF_BIT(va_win2_lumacc,		  32)
	REGDEF_END(VA_REGISTER35)
	
	
	/*
		va_win2G1h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win2G1h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER36_OFS 0x0c8c
	REGDEF_BEGIN(VA_REGISTER36)
		REGDEF_BIT(va_win2g1h_vacc ,		16)
		REGDEF_BIT(va_win2g1h_vacnt,		16)
	REGDEF_END(VA_REGISTER36)
	
	
	/*
		va_win2G1v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win2G1v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER37_OFS 0x0c90
	REGDEF_BEGIN(VA_REGISTER37)
		REGDEF_BIT(va_win2g1v_vacc ,		16)
		REGDEF_BIT(va_win2g1v_vacnt,		16)
	REGDEF_END(VA_REGISTER37)
	
	
	/*
		va_win2G2h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win2G2h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER38_OFS 0x0c94
	REGDEF_BEGIN(VA_REGISTER38)
		REGDEF_BIT(va_win2g2h_vacc ,		16)
		REGDEF_BIT(va_win2g2h_vacnt,		16)
	REGDEF_END(VA_REGISTER38)
	
	
	/*
		va_win2G2v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win2G2v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER39_OFS 0x0c98
	REGDEF_BEGIN(VA_REGISTER39)
		REGDEF_BIT(va_win2g2v_vacc ,		16)
		REGDEF_BIT(va_win2g2v_vacnt,		16)
	REGDEF_END(VA_REGISTER39)
	
	
	/*
		va_win3_lumacc:    [0x0, 0xffffffff],			bits : 31_0
	*/
#define VA_REGISTER40_OFS 0x0c9c
	REGDEF_BEGIN(VA_REGISTER40)
		REGDEF_BIT(va_win3_lumacc,		  32)
	REGDEF_END(VA_REGISTER40)
	
	
	/*
		va_win3G1h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win3G1h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER41_OFS 0x0ca0
	REGDEF_BEGIN(VA_REGISTER41)
		REGDEF_BIT(va_win3g1h_vacc ,		16)
		REGDEF_BIT(va_win3g1h_vacnt,		16)
	REGDEF_END(VA_REGISTER41)
	
	
	/*
		va_win3G1v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win3G1v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER42_OFS 0x0ca4
	REGDEF_BEGIN(VA_REGISTER42)
		REGDEF_BIT(va_win3g1v_vacc ,		16)
		REGDEF_BIT(va_win3g1v_vacnt,		16)
	REGDEF_END(VA_REGISTER42)
	
	
	/*
		va_win3G2h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win3G2h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER43_OFS 0x0ca8
	REGDEF_BEGIN(VA_REGISTER43)
		REGDEF_BIT(va_win3g2h_vacc ,		16)
		REGDEF_BIT(va_win3g2h_vacnt,		16)
	REGDEF_END(VA_REGISTER43)
	
	
	/*
		va_win3G2v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win3G2v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER44_OFS 0x0cac
	REGDEF_BEGIN(VA_REGISTER44)
		REGDEF_BIT(va_win3g2v_vacc ,		16)
		REGDEF_BIT(va_win3g2v_vacnt,		16)
	REGDEF_END(VA_REGISTER44)
	
	
	/*
		va_win4_lumacc:    [0x0, 0xffffffff],			bits : 31_0
	*/
#define VA_REGISTER45_OFS 0x0cb0
	REGDEF_BEGIN(VA_REGISTER45)
		REGDEF_BIT(va_win4_lumacc,		  32)
	REGDEF_END(VA_REGISTER45)
	
	
	/*
		va_win4G1h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win4G1h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER46_OFS 0x0cb4
	REGDEF_BEGIN(VA_REGISTER46)
		REGDEF_BIT(va_win4g1h_vacc ,		16)
		REGDEF_BIT(va_win4g1h_vacnt,		16)
	REGDEF_END(VA_REGISTER46)
	
	
	/*
		va_win4G1v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win4G1v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER47_OFS 0x0cb8
	REGDEF_BEGIN(VA_REGISTER47)
		REGDEF_BIT(va_win4g1v_vacc ,		16)
		REGDEF_BIT(va_win4g1v_vacnt,		16)
	REGDEF_END(VA_REGISTER47)
	
	
	/*
		va_win4G2h_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win4G2h_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER48_OFS 0x0cbc
	REGDEF_BEGIN(VA_REGISTER48)
		REGDEF_BIT(va_win4g2h_vacc ,		16)
		REGDEF_BIT(va_win4g2h_vacnt,		16)
	REGDEF_END(VA_REGISTER48)
	
	
	/*
		va_win4G2v_vacc :	 [0x0, 0xffff], 		bits : 15_0
		va_win4G2v_vacnt:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define VA_REGISTER49_OFS 0x0cc0
	REGDEF_BEGIN(VA_REGISTER49)
		REGDEF_BIT(va_win4g2v_vacc ,		16)
		REGDEF_BIT(va_win4g2v_vacnt,		16)
	REGDEF_END(VA_REGISTER49)
	
	
	/*
		va_g2hthl:	  [0x0, 0xfff], 		bits : 11_0
		va_g2hthh:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define VA_REGISTER50_OFS 0x0cc4
	REGDEF_BEGIN(VA_REGISTER50)
		REGDEF_BIT(va_g2hthl,		 12)
		REGDEF_BIT( 		,		 4)
		REGDEF_BIT(va_g2hthh,		 12)
	REGDEF_END(VA_REGISTER50)
	
	
	/*
		va_g2vthl:	  [0x0, 0xfff], 		bits : 11_0
		va_g2vthh:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define VA_REGISTER51_OFS 0x0cc8
	REGDEF_BEGIN(VA_REGISTER51)
		REGDEF_BIT(va_g2vthl,		 12)
		REGDEF_BIT( 		,		 4)
		REGDEF_BIT(va_g2vthh,		 12)
	REGDEF_END(VA_REGISTER51)
	
	
	/*
		va_win_ldg_low_slope :	  [0x0, 0xf],			bits : 7_4
		va_win_ldg_high_slope:	  [0x0, 0xf],			bits : 11_8
	*/
#define VA_REGISTER61_OFS 0x0cf0
	REGDEF_BEGIN(VA_REGISTER61)
		REGDEF_BIT( 					,		 4)
		REGDEF_BIT(va_win_ldg_low_slope ,		 4)
		REGDEF_BIT(va_win_ldg_high_slope,		 4)
	REGDEF_END(VA_REGISTER61)
	
	
	/*
		va_win_ldg_low_th	:	 [0x0, 0xff],			bits : 7_0
		va_win_ldg_high_th	:	 [0x0, 0xff],			bits : 15_8
		va_win_ldg_low_gain :	 [0x0, 0xff],			bits : 23_16
		va_win_ldg_high_gain:	 [0x0, 0xff],			bits : 31_24
	*/
#define VA_REGISTER62_OFS 0x0cf4
	REGDEF_BEGIN(VA_REGISTER62)
		REGDEF_BIT(va_win_ldg_low_th   ,		8)
		REGDEF_BIT(va_win_ldg_high_th  ,		8)
		REGDEF_BIT(va_win_ldg_low_gain ,		8)
		REGDEF_BIT(va_win_ldg_high_gain,		8)
	REGDEF_END(VA_REGISTER62)
	
	
	/*
		va_gamma_0 :	[0x0, 0xfff],			bits : 11_0
		va_gamma_1 :	[0x0, 0xfff],			bits : 27_16
	*/
#define VA_REGISTER63_OFS 0x0cf8
	REGDEF_BEGIN(VA_REGISTER63)
		REGDEF_BIT(va_gamma_0 , 	   12)
		REGDEF_BIT( 		  , 	   4)
		REGDEF_BIT(va_gamma_1 , 	   12)
	REGDEF_END(VA_REGISTER63)
	
	
	/*
		va_gamma_2:    [0x0, 0xfff],			bits : 11_0
		va_gamma_3:    [0x0, 0xfff],			bits : 27_16
	*/
#define VA_REGISTER64_OFS 0x0cfc
	REGDEF_BEGIN(VA_REGISTER64)
		REGDEF_BIT(va_gamma_2,		  12)
		REGDEF_BIT( 		 ,		  4)
		REGDEF_BIT(va_gamma_3,		  12)
	REGDEF_END(VA_REGISTER64)
	
	
	/*
		va_gamma_4:    [0x0, 0xfff],			bits : 11_0
		va_gamma_5:    [0x0, 0xfff],			bits : 27_16
	*/
#define VA_REGISTER65_OFS 0x0d00
	REGDEF_BEGIN(VA_REGISTER65)
		REGDEF_BIT(va_gamma_4,		  12)
		REGDEF_BIT( 		 ,		  4)
		REGDEF_BIT(va_gamma_5,		  12)
	REGDEF_END(VA_REGISTER65)
	
	
	/*
		va_gamma_6:    [0x0, 0xfff],			bits : 11_0
		va_gamma_7:    [0x0, 0xfff],			bits : 27_16
	*/
#define VA_REGISTER66_OFS 0x0d04
	REGDEF_BEGIN(VA_REGISTER66)
		REGDEF_BIT(va_gamma_6,		  12)
		REGDEF_BIT( 		 ,		  4)
		REGDEF_BIT(va_gamma_7,		  12)
	REGDEF_END(VA_REGISTER66)
	
	
	/*
		va_gamma_8:    [0x0, 0xfff],			bits : 11_0
		va_gamma_9:    [0x0, 0xfff],			bits : 27_16
	*/
#define VA_REGISTER67_OFS 0x0d08
	REGDEF_BEGIN(VA_REGISTER67)
		REGDEF_BIT(va_gamma_8,		  12)
		REGDEF_BIT( 		 ,		  4)
		REGDEF_BIT(va_gamma_9,		  12)
	REGDEF_END(VA_REGISTER67)
	
	
	/*
		va_gamma_10:	[0x0, 0xfff],			bits : 11_0
		va_gamma_11:	[0x0, 0xfff],			bits : 27_16
	*/
#define VA_REGISTER68_OFS 0x0d0c
	REGDEF_BEGIN(VA_REGISTER68)
		REGDEF_BIT(va_gamma_10, 	   12)
		REGDEF_BIT( 		  , 	   4)
		REGDEF_BIT(va_gamma_11, 	   12)
	REGDEF_END(VA_REGISTER68)
	
	
	/*
		va_gamma_12:	[0x0, 0xfff],			bits : 11_0
		va_gamma_13:	[0x0, 0xfff],			bits : 27_16
	*/
#define VA_REGISTER69_OFS 0x0d10
	REGDEF_BEGIN(VA_REGISTER69)
		REGDEF_BIT(va_gamma_12, 	   12)
		REGDEF_BIT( 		  , 	   4)
		REGDEF_BIT(va_gamma_13, 	   12)
	REGDEF_END(VA_REGISTER69)
	
	
	/*
		va_gamma_14:	[0x0, 0xfff],			bits : 11_0
		va_gamma_15:	[0x0, 0xfff],			bits : 27_16
	*/
#define VA_REGISTER70_OFS 0x0d14
	REGDEF_BEGIN(VA_REGISTER70)
		REGDEF_BIT(va_gamma_14, 	   12)
		REGDEF_BIT( 		  , 	   4)
		REGDEF_BIT(va_gamma_15, 	   12)
	REGDEF_END(VA_REGISTER70)
	
	
	/*
		va_gamma_16:	[0x0, 0xfff],			bits : 11_0
	*/
#define VA_REGISTER71_OFS 0x0d18
	REGDEF_BEGIN(VA_REGISTER71)
		REGDEF_BIT(va_gamma_16, 	   12)
	REGDEF_END(VA_REGISTER71)
	
	
	/*
		vdetgh1a:	 [0x0, 0x3ff],			bits : 9_0
		vdetgh1b:	 [0x0, 0x3ff],			bits : 19_10
		vdetgh1c:	 [0x0, 0x3ff],			bits : 29_20
	*/
#define VA_REGISTER72_OFS 0x0d1c
	REGDEF_BEGIN(VA_REGISTER72)
		REGDEF_BIT(vdetgh1a,		10)
		REGDEF_BIT(vdetgh1b,		10)
		REGDEF_BIT(vdetgh1c,		10)
	REGDEF_END(VA_REGISTER72)
	
	
	/*
		vdetgh1d	  :    [0x0, 0x3ff],			bits : 9_0
		vdetgh1_iir1_e:    [0x0, 0x3ff],			bits : 19_10
		vdetgh1_iir1_f:    [0x0, 0x3ff],			bits : 29_20
	*/
#define VA_REGISTER73_OFS 0x0d20
	REGDEF_BEGIN(VA_REGISTER73)
		REGDEF_BIT(vdetgh1d 	 ,		  10)
		REGDEF_BIT(vdetgh1_iir1_e,		  10)
		REGDEF_BIT(vdetgh1_iir1_f,		  10)
	REGDEF_END(VA_REGISTER73)
	
	
	
	/*
		vdetgh2a:	 [0x0, 0x3ff],			bits : 9_0
		vdetgh2b:	 [0x0, 0x3ff],			bits : 19_10
		vdetgh2c:	 [0x0, 0x3ff],			bits : 29_20
	*/
#define VA_REGISTER75_OFS 0x0d28
	REGDEF_BEGIN(VA_REGISTER75)
		REGDEF_BIT(vdetgh2a,		10)
		REGDEF_BIT(vdetgh2b,		10)
		REGDEF_BIT(vdetgh2c,		10)
	REGDEF_END(VA_REGISTER75)
	
	
	/*
		vdetgh2d	  :    [0x0, 0x3ff],			bits : 9_0
		vdetgh2_iir1_e:    [0x0, 0x3ff],			bits : 19_10
		vdetgh2_iir1_f:    [0x0, 0x3ff],			bits : 29_20
	*/
#define VA_REGISTER76_OFS 0x0d2c
	REGDEF_BEGIN(VA_REGISTER76)
		REGDEF_BIT(vdetgh2d 	 ,		  10)
		REGDEF_BIT(vdetgh2_iir1_e,		  10)
		REGDEF_BIT(vdetgh2_iir1_f,		  10)
	REGDEF_END(VA_REGISTER76)
	
	
	
	/*
		vdetgh1_iir2_a:    [0x0, 0x3ff],			bits : 9_0
		vdetgh1_iir2_b:    [0x0, 0x3ff],			bits : 19_10
		vdetgh1_iir2_e:    [0x0, 0x3ff],			bits : 29_20
	*/
#define VA_REGISTER78_OFS 0x0d34
	REGDEF_BEGIN(VA_REGISTER78)
		REGDEF_BIT(vdetgh1_iir2_a,		  10)
		REGDEF_BIT(vdetgh1_iir2_b,		  10)
		REGDEF_BIT(vdetgh1_iir2_e,		  10)
	REGDEF_END(VA_REGISTER78)
	
	
	/*
		vdetgh1_iir2_f:    [0x0, 0x3ff],			bits : 9_0
		vdetgh1_iir3_a:    [0x0, 0x3ff],			bits : 19_10
		vdetgh1_iir3_b:    [0x0, 0x3ff],			bits : 29_20
	*/
#define VA_REGISTER79_OFS 0x0d38
	REGDEF_BEGIN(VA_REGISTER79)
		REGDEF_BIT(vdetgh1_iir2_f,		  10)
		REGDEF_BIT(vdetgh1_iir3_a,		  10)
		REGDEF_BIT(vdetgh1_iir3_b,		  10)
	REGDEF_END(VA_REGISTER79)
	
	
	/*
		vdetgh1_iir3_e:    [0x0, 0x3ff],			bits : 9_0
		vdetgh1_iir3_f:    [0x0, 0x3ff],			bits : 19_10
	*/
#define VA_REGISTER80_OFS 0x0d3c
	REGDEF_BEGIN(VA_REGISTER80)
		REGDEF_BIT(vdetgh1_iir3_e,		  10)
		REGDEF_BIT(vdetgh1_iir3_f,		  10)
	REGDEF_END(VA_REGISTER80)
	
	
	
	
	
	
#define NN_ISP_REGISTER0_OFS 0x0D40
	REGDEF_BEGIN(NN_ISP_REGISTER0)
	REGDEF_BIT(ife_nn_isp_p0_en, 1) 							// bits , 0
	REGDEF_BIT(ife_nn_isp_p1_en, 1) 							// bits , 1
	REGDEF_BIT(ife_nn_isp_p0_handshake_mode, 1) 				// bits , 2
	REGDEF_BIT(ife_nn_isp_p1_handshake_mode, 1) 				// bits , 3
	//REGDEF_BIT(ife_nn_isp_p0_ringbuf_slice_clear	  , 1)		  // bits , 4
	//REGDEF_BIT(ife_nn_isp_p0_outbuf_slice_ready	  , 1)		  // bits , 5
	//REGDEF_BIT(ife_nn_isp_p1_ringbuf_slice_clear	  , 1)		  // bits , 6
	//REGDEF_BIT(ife_nn_isp_p1_outbuf_slice_ready	  , 1)		  // bits , 7
	REGDEF_BIT(, 4)
	REGDEF_BIT(ife_nn_isp_p0_input_burst_mode, 1)				// bits , 8
	REGDEF_BIT(ife_nn_isp_p0_output_burst_mode, 1)				// bits , 9
	REGDEF_BIT(ife_nn_isp_p1_input_burst_mode, 1)				// bits , 10
	REGDEF_BIT(ife_nn_isp_p1_output_burst_mode, 1)				// bits , 11
	REGDEF_END(NN_ISP_REGISTER0)
	
	
#define NN_ISP_REGISTER1_OFS 0x0D44
	REGDEF_BEGIN(NN_ISP_REGISTER1)
	REGDEF_BIT(, 1)
	REGDEF_BIT(nn_isp_p0_slice_height, 15)				// bits , 15_1
	REGDEF_BIT(nn_isp_p0_ringbuf_height, 16)			// bits , 31_16
	REGDEF_END(NN_ISP_REGISTER1)
	
	
#define NN_ISP_REGISTER2_OFS 0x0D48
	REGDEF_BEGIN(NN_ISP_REGISTER2)
	REGDEF_BIT(nn_isp_p0_slice_ovlp, 10)			  // bits , 9_0
	REGDEF_BIT(, 6)
	REGDEF_BIT(nn_isp_p0_outbuf_height, 16) 			// bits , 31_16
	REGDEF_END(NN_ISP_REGISTER2)
	
	
#define NN_ISP_REGISTER3_OFS 0x0D4C
	REGDEF_BEGIN(NN_ISP_REGISTER3)
	REGDEF_BIT(ife_nn_isp_p0_set_slice_ready_status, 1) 			  // bits , 0
	REGDEF_BIT(ife_nn_isp_p0_get_slice_clear_status, 1) 				// bits , 1
	REGDEF_BIT(ife_nn_isp_p1_set_slice_ready_status, 1) 				// bits , 2
	REGDEF_BIT(ife_nn_isp_p1_get_slice_clear_status, 1) 				// bits , 3
	REGDEF_END(NN_ISP_REGISTER3)
	
	
#define NN_ISP_REGISTER4_OFS 0x0D50
	REGDEF_BEGIN(NN_ISP_REGISTER4)
	REGDEF_BIT(nn_isp_p0_sai, 32)				  // bits , 0
	REGDEF_END(NN_ISP_REGISTER4)
	
	
#define NN_ISP_REGISTER5_OFS 0x0D54
	REGDEF_BEGIN(NN_ISP_REGISTER5)
	REGDEF_BIT(, 2)
	REGDEF_BIT(nn_isp_p0_ofsi, 14)				// bits , 15_2
	REGDEF_BIT(, 8)
	REGDEF_BIT(nn_isp_p0_sai_msb, 4)			// bits , 15_2
	REGDEF_END(NN_ISP_REGISTER5)
	
	
#define NN_ISP_REGISTER6_OFS 0x0D58
	REGDEF_BEGIN(NN_ISP_REGISTER6)
	REGDEF_BIT(nn_isp_p0_sao, 32)			  // bits , 31_0
	REGDEF_END(NN_ISP_REGISTER6)
	
	
#define NN_ISP_REGISTER7_OFS 0x0D5C
	REGDEF_BEGIN(NN_ISP_REGISTER7)
	REGDEF_BIT(, 2)
	REGDEF_BIT(nn_isp_p0_ofso, 14)				// bits , 15_2
	REGDEF_BIT(, 8)
	REGDEF_BIT(nn_isp_p0_sao_msb, 4)			// bits , 27_24
	REGDEF_END(NN_ISP_REGISTER7)
	
	
#define NN_ISP_REGISTER8_OFS 0x0D60
	REGDEF_BEGIN(NN_ISP_REGISTER8)
	REGDEF_BIT(, 1)
	REGDEF_BIT(nn_isp_p1_slice_height, 15)				// bits , 15_1
	REGDEF_BIT(nn_isp_p1_ringbuf_height, 16)			// bits , 31_16
	REGDEF_END(NN_ISP_REGISTER8)
	
	
#define NN_ISP_REGISTER9_OFS 0x0D64
	REGDEF_BEGIN(NN_ISP_REGISTER9)
	REGDEF_BIT(nn_isp_p1_slice_ovlp, 10)			// bits , 9_0
	REGDEF_BIT(, 6)
	REGDEF_BIT(nn_isp_p1_outbuf_height, 16) 		// bits , 31_16
	REGDEF_END(NN_ISP_REGISTER9)
	
	
#define NN_ISP_REGISTER10_OFS 0x0D68
	REGDEF_BEGIN(NN_ISP_REGISTER10)
	REGDEF_BIT(, 32)
	REGDEF_END(NN_ISP_REGISTER10)
	
	
#define NN_ISP_REGISTER11_OFS 0x0D6C
	REGDEF_BEGIN(NN_ISP_REGISTER11)
	REGDEF_BIT(nn_isp_p1_sai, 32)			  // bits , 31_0
	REGDEF_END(NN_ISP_REGISTER11)
	
	
#define NN_ISP_REGISTER12_OFS 0x0D70
	REGDEF_BEGIN(NN_ISP_REGISTER12)
	REGDEF_BIT(, 2)
	REGDEF_BIT(nn_isp_p1_ofsi, 14)				// bits , 15_2
	REGDEF_BIT(, 8)
	REGDEF_BIT(nn_isp_p1_sai_msb, 4)			// bits , 27_24
	REGDEF_END(NN_ISP_REGISTER12)
	
	
#define NN_ISP_REGISTER13_OFS 0x0D74
	REGDEF_BEGIN(NN_ISP_REGISTER13)
	REGDEF_BIT(nn_isp_p1_sao, 32)							  // bits , 31_0
	REGDEF_END(NN_ISP_REGISTER13)
	
	
#define NN_ISP_REGISTER14_OFS 0x0D78
	REGDEF_BEGIN(NN_ISP_REGISTER14)
	REGDEF_BIT(, 2)
	REGDEF_BIT(nn_isp_p1_ofso, 14)					// bits , 15_2
	REGDEF_BIT(, 8)
	REGDEF_BIT(nn_isp_p1_sao_msb, 4)				// bits , 27_24
	REGDEF_END(NN_ISP_REGISTER14)
	
#define NN_ISP_REGISTER15_OFS 0x0D7C
	REGDEF_BEGIN(NN_ISP_REGISTER15)
	REGDEF_BIT(, 31)
	REGDEF_BIT(ife_nn_isp_p0_ringbuf_slice_clear, 1)			   // bits , 27_24
	REGDEF_END(NN_ISP_REGISTER15)
	
#define NN_ISP_REGISTER16_OFS 0x0D80
	REGDEF_BEGIN(NN_ISP_REGISTER16)
	REGDEF_BIT(, 31)
	REGDEF_BIT(ife_nn_isp_p0_outbuf_slice_ready, 1) 			  // bits , 27_24
	REGDEF_END(NN_ISP_REGISTER16)
	
#define NN_ISP_REGISTER17_OFS 0x0D84
	REGDEF_BEGIN(NN_ISP_REGISTER17)
	REGDEF_BIT(, 31)
	REGDEF_BIT(ife_nn_isp_p1_ringbuf_slice_clear, 1)			   // bits , 27_24
	REGDEF_END(NN_ISP_REGISTER17)
	
#define NN_ISP_REGISTER18_OFS 0x0D88
	REGDEF_BEGIN(NN_ISP_REGISTER18)
	REGDEF_BIT(, 31)
	REGDEF_BIT(ife_nn_isp_p1_outbuf_slice_ready, 1) 			  // bits , 27_24
	REGDEF_END(NN_ISP_REGISTER18)




#endif

