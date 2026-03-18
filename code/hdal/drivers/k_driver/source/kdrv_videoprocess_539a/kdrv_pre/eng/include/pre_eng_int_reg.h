#ifndef _PRE_INT_REGISTER_H_
#define _PRE_INT_REGISTER_H_

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
			unsigned pre_sw_rst             :  1;            //bits : 0
			unsigned pre_start              :  1;            //bits : 1
			unsigned pre_load_start         :  1;            //bits : 2
			unsigned pre_load_fd            :  1;            //bits : 3
			unsigned pre_load_frmstart      :  1;            //bits : 4
			unsigned pre_global_load_en     :  1;            //bits : 5
			unsigned                        : 22;
			unsigned pre_ll_fire            :  1;            //bits : 28
		} bit;
		UINT32 word;
	} pre_register_0; // 0x00;

	union {
		struct {
			unsigned pre_mode               :  2;            //bits : 1..0
			unsigned            :  1;
			unsigned              :  1;
			unsigned inbit_16_fmt_sel       :  1;            //bits : 4
			unsigned inbit_depth            :  2;            //bits : 6..5
			unsigned outbit_depth           :  2;            //bits : 8..7
			unsigned cfapat                 :  3;            //bits : 11..9
			unsigned                        :  1;            //bits : 12
			unsigned outl_en            :  1;            //bits : 13
			unsigned           :  1;            //bits : 14
			unsigned            :  1;            //bits : 15
			unsigned              :  1;            //bits : 16
			unsigned             :  1;            //bits : 17
			unsigned             :  3;            //bits : 20..18
			unsigned bayer_format           :  1;            //bits : 21
			unsigned     :  1;            //bits : 22
			unsigned         :  1;            //bits : 23
			unsigned            :  1;            //bits : 24
			unsigned f_cg_en            :  1;            //bits : 25
			unsigned f_fusion_en        :  1;            //bits : 26
			unsigned f_fusion_fnum      :  2;            //bits : 28..27
			unsigned f_fc_en            :  1;            //bits : 29
			unsigned mirror_en          :  1;            //bits : 30
            unsigned thermal_mode     : 1;		// bits : 31
		} bit;
		UINT32 word;
	} pre_register_1; // 0x04;

	union {
		struct {
			unsigned inte_frmend            : 1;        // bits : 0
			unsigned inte_pre_r_dec1_err    : 1;        // bits : 1
			unsigned inte_pre_r_dec2_err    : 1;        // bits : 2
			unsigned inte_llend             : 1;        // bits : 3
			unsigned inte_llerror           : 1;        // bits : 4
			unsigned inte_llerror2          : 1;        // bits : 5
			unsigned inte_lljobend          : 1;        // bits : 6
			unsigned inte_pre_bufovfl       : 1;        // bits : 7
			unsigned inte_pre_ringbuf_err   : 1;        // bits : 8
			unsigned inte_pre_frame_error   : 1;        // bits : 9
			unsigned                        : 1;        // bits : 10
			unsigned                        : 1;
			unsigned inte_sie_frame_start   : 1;        // bits : 12
			unsigned inte_sie2_frame_start  : 1;        // bits : 13
			unsigned inte_pre_frame_start   : 1;        // bits : 14
			unsigned                        : 1;
			unsigned inte_pre_bnr_enc_ovfl        : 1;		// bits : 20
			unsigned inte_pre_bnr_dec_err         : 1;		// bits : 21
		} bit;
		UINT32 word;
	} pre_register_2; // 0x08;

	union {
		struct {
			unsigned int_frmend             : 1;        // bits : 0
			unsigned int_pre_r_dec1_err     : 1;        // bits : 1
			unsigned int_pre_r_dec2_err     : 1;        // bits : 2
			unsigned int_llend              : 1;        // bits : 3
			unsigned int_llerror            : 1;        // bits : 4
			unsigned int_llerror2           : 1;        // bits : 5
			unsigned int_lljobend           : 1;        // bits : 6
			unsigned int_pre_bufovfl        : 1;        // bits : 7
			unsigned int_pre_ringbuf_err    : 1;        // bits : 8
			unsigned int_pre_frame_error    : 1;        // bits : 9
			unsigned                        : 1;        // bits : 10
			unsigned                        : 1;
			unsigned int_sie_frame_start    : 1;        // bits : 12
			unsigned int_sie2_frame_start   : 1;        // bits : 13
			unsigned int_pre_frame_start    : 1;        // bits : 14
			unsigned                        : 1;
			unsigned int_pre_bnr_enc_ovfl        : 1;		// bits : 20
			unsigned int_pre_bnr_dec_err         : 1;		// bits : 21
		} bit;
		UINT32 word;
	} pre_register_3; // 0x0c;

	union {
		struct {
			unsigned pre_busy               :  1;       //bits : 0
			unsigned                        : 19;
			unsigned pre_frmstr_rst         :  1;       // bits : 20
			unsigned pre_checksum_en        :  1;       // bits : 21
		} bit;
		UINT32 word;
	} pre_register_4; // 0x10;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_saill        : 30;        // bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_5; // 0x14;

	union {
		struct {
			unsigned cfapat_2                         : 3;		// bits : 2_0
			unsigned                                  : 5;
			unsigned pre_hdr_sram_shutdown_en         : 1;		// bits : 8
			unsigned pre_va_sram_shutdown_en          : 1;		// bits : 9
			unsigned pre_bnr_sram_shutdown_en         : 1;		// bits : 10
			unsigned pre_outl_sram_shutdown_en        : 1;		// bits : 11
			unsigned                                  : 12;
			unsigned dram_saill_msb         : 4;        // bits : 27_24
			unsigned flip0_en               : 1;        // bits : 28
			unsigned flip1_en               : 1;        // bits : 29
			unsigned pre_output_sel         : 1;        // bits : 30
		} bit;
		UINT32 word;
	} pre_register_6; // 0x18;

	union {
		struct {
			unsigned pre_dmach_idle                       : 1;             //bits : 0
			unsigned pre_dmach_dis                        : 1;             //bits : 1
			unsigned pre_dma_output_en                    : 1;
			unsigned                                      : 5;
			unsigned pre_dich_line_buf_ctrl               : 2;             //bits : 9..8
			unsigned                                      : 6;
			unsigned pre_dma_sync_dich_line_buf_disable   : 1;             //bits : 16
			unsigned pre_dma1_wait_sie2_start_disable     : 1;             //bits : 17
		} bit;
		UINT32 word;
	} pre_register_7; // 0x1c;

	union {
		struct {
			unsigned                        :  2;
			unsigned width                  : 14;            //bits : 15..2
			unsigned                        :  1;
			unsigned height                 : 15;            //bits : 31..17
		} bit;
		UINT32 word;
	} pre_register_8; // 0x20;

	union {
		struct {
			unsigned                        :  2;
			unsigned crop_width             : 14;            //bits : 15..2
			unsigned                        :  1;
			unsigned crop_height            : 15;            //bits : 31..17
		} bit;
		UINT32 word;
	} pre_register_9; // 0x24;

	union {
		struct {
			unsigned crop_hpos              : 16;            //bits : 15..0
			unsigned crop_vpos              : 16;            //bits : 31..16
		} bit;
		UINT32 word;
	} pre_register_10; // 0x28;

	union {
		struct {
			unsigned pre_ll_terminate        : 1;       // bits : 0
		} bit;
		UINT32 word;
	} pre_register_11; // 0x2c;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_sai0              : 30;            //bits : 31..2
		} bit;
		UINT32 word;
	} pre_register_12; // 0x30;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_ofsi0             : 14;            //bits : 15..2
			unsigned                        :  8;
			unsigned dram_sai0_msb          :  4;
		} bit;
		UINT32 word;
	} pre_register_13; // 0x34;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_sai1              : 30;            //bits : 31..2
		} bit;
		UINT32 word;
	} pre_register_14; // 0x38;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_ofsi1             : 14;            //bits : 15..2
			unsigned                        :  8;
			unsigned dram_sai1_msb         :  4;
		} bit;
		UINT32 word;
	} pre_register_15; // 0x3c;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_16; // 0x40;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_17; // 0x44;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_sao               : 30;            //bits : 31..2
		} bit;
		UINT32 word;
	} pre_register_18; // 0x48;

	union {
		struct {
			unsigned                        :  2;
			unsigned dram_ofso              : 14;            //bits : 15..2
			unsigned                        :  8;
			unsigned dram_sao_msb          :  4;
		} bit;
		UINT32 word;
	} pre_register_19; // 0x4c;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_20; // 0x50;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_21; // 0x54;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_22; // 0x58;

	union {
		struct {
			unsigned input_burst_mode       :  1;            //bits : 0
			unsigned                        :  3;
			unsigned output_burst_mode      :  1;            //bits : 4
			unsigned                        :  7;
			unsigned dmaloop_line           : 12;            //bits : 23..12
			unsigned dmaloop_en             :  1;            //bits : 24
			unsigned dmaloop_ctrl           :  1;            //bits : 25
			unsigned                        :  2;
			unsigned sie2_line_count_align_to_height :  1;   //bits : 28
		} bit;
		UINT32 word;
	} pre_register_23; // 0x5c;

	union {
		struct {
			unsigned pre_out0_single_en                  : 1;		// bits : 0
			unsigned                                     : 1;
			unsigned pre_out2_single_en                  : 1;		// bits : 2
			unsigned                                     : 2;
			unsigned pre_bnr_ref_out_single_en           : 1;		// bits : 5
			unsigned pre_bnr_sigma_out_single_en         : 1;		// bits : 6
			unsigned pre_bnr_sta_out_single_en           : 1;		// bits : 7
			unsigned pre_bnr_gamma_out_single_en         : 1;		// bits : 8
			unsigned pre_fusion_wat_out_single_en        : 1;		// bits : 9
			unsigned                                     : 21;
			unsigned pre_dram_out_mode        : 1;         //bits : 31
		} bit;
		UINT32 word;
	} pre_register_24; // 0x60;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_25; // 0x64;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_26; // 0x68;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_27; // 0x6c;

	union {
		struct {
			unsigned                          : 3;
			unsigned pre_f_cgain_range      :  1;            //bits : 3
		} bit;
		UINT32 word;
	} pre_register_28; // 0x70;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_29; // 0x74;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_30; // 0x78;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_31; // 0x7c;

	union {
		struct {
			unsigned pre_f_p0_cgain_r         : 16;		// bits : 15_0
			unsigned pre_f_p0_cgain_gr        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_32; // 0x80;

	union {
		struct {
			unsigned pre_f_p0_cgain_gb        : 16;		// bits : 15_0
			unsigned pre_f_p0_cgain_b         : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_33; // 0x84;

	union {
		struct {
			unsigned pre_f_p0_cgain_ir        : 16;		// bits : 15_0
		} bit;
		UINT32 word;
	} pre_register_34; // 0x88;

	union {
		struct {
			unsigned pre_f_p1_cgain_r         : 16;		// bits : 15_0
			unsigned pre_f_p1_cgain_gr        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_35; // 0x8c;

	union {
		struct {
			unsigned pre_f_p1_cgain_gb        : 16;		// bits : 15_0
			unsigned pre_f_p1_cgain_b         : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_36; // 0x90;

	union {
		struct {
			unsigned pre_f_p1_cgain_ir        : 16;		// bits : 15_0
		} bit;
		UINT32 word;
	} pre_register_37; // 0x94;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_38; // 0x98;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_39; // 0x9c;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_40; // 0xa0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_41; // 0xa4;

	union {
		struct {
			unsigned pre_dma1_line_count        : 16;		// bits : 15_0
			unsigned sie2_line_count            : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_42; // 0xa8;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_43; // 0xac;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_44; // 0xb0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_45; // 0xb4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_46; // 0xb8;

	union {
		struct {
			unsigned pre_f_p0_cofs_r        : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned pre_f_p0_cofs_gr       : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} pre_register_47; // 0xbc;

	union {
		struct {
			unsigned pre_f_p0_cofs_gb       : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned pre_f_p0_cofs_b        : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} pre_register_48; // 0xc0;

	union {
		struct {
			unsigned pre_f_p0_cofs_ir       : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} pre_register_49; // 0xc4;

	union {
		struct {
			unsigned pre_f_p1_cofs_r        : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned pre_f_p1_cofs_gr       : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} pre_register_50; // 0xc8;

	union {
		struct {
			unsigned pre_f_p1_cofs_gb       : 10;            //bits : 9..0
			unsigned                        :  6;
			unsigned pre_f_p1_cofs_b        : 10;            //bits : 25..16
		} bit;
		UINT32 word;
	} pre_register_51; // 0xcc;

	union {
		struct {
			unsigned pre_f_p1_cofs_ir       : 10;            //bits : 9..0
		} bit;
		UINT32 word;
	} pre_register_52; // 0xd0;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_53; // 0xd4;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_54; // 0xd8;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_55; // 0xdc;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_56; // 0xe0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_57; // 0xe4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_58; // 0xe8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_59; // 0xec;

	union {
		struct {
			unsigned pre_outl_rgbir_rb_w        : 8;        // bits : 7_0
			unsigned pre_ord_rgbir_rb_w         : 8;        // bits : 15_8
		} bit;
		UINT32 word;
	} pre_register_60; // 0xf0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_61; // 0xf4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_62; // 0xf8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_63; // 0xfc;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_64; // 0x100;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_65; // 0x104;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_66; // 0x108;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_67; // 0x10c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_68; // 0x110;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_69; // 0x114;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_70; // 0x118;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_71; // 0x11c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_72; // 0x120;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_73; // 0x124;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_74; // 0x128;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_75; // 0x12c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_76; // 0x130;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_77; // 0x134;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_78; // 0x138;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_79; // 0x13c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_80; // 0x140;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_81; // 0x144;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_82; // 0x148;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_83; // 0x14c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_84; // 0x150;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_85; // 0x154;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_86; // 0x158;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_87; // 0x15c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_88; // 0x160;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_89; // 0x164;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_90; // 0x168;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_91; // 0x16c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_92; // 0x170;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_93; // 0x174;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_94; // 0x178;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_95; // 0x17c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_96; // 0x180;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_97; // 0x184;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_98; // 0x188;

	union {
		struct {
			unsigned pre_outl_bright_ofs    : 12;            //bits : 11..0
			unsigned pre_outl_dark_ofs      : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} pre_register_99; // 0x18c;

	union {
		struct {
			unsigned pre_outlth_bri0        : 12;            //bits : 11..0
			unsigned pre_outlth_dark0       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} pre_register_100; // 0x190;

	union {
		struct {
			unsigned pre_outlth_bri1        : 12;            //bits : 11..0
			unsigned pre_outlth_dark1       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} pre_register_101; // 0x194;

	union {
		struct {
			unsigned pre_outlth_bri2        : 12;            //bits : 11..0
			unsigned pre_outlth_dark2       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} pre_register_102; // 0x198;

	union {
		struct {
			unsigned pre_outlth_bri3        : 12;            //bits : 11..0
			unsigned pre_outlth_dark3       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} pre_register_103; // 0x19c;

	union {
		struct {
			unsigned pre_outlth_bri4        : 12;            //bits : 11..0
			unsigned pre_outlth_dark4       : 12;            //bits : 23..12
		} bit;
		UINT32 word;
	} pre_register_104; // 0x1a0;

	union {
		struct {
			unsigned pre_outl_compensate_mode :  1;            //bits : 0
			unsigned                          :  7;
			unsigned pre_outl_weight          :  8;
			unsigned pre_outl_cnt1          :  5;            //bits : 20..16
			unsigned                        :  3;
			unsigned pre_outl_cnt2          :  5;            //bits : 28..24
		} bit;
		UINT32 word;
	} pre_register_105; // 0x1a4;

	union {
		struct {
			unsigned pre_ord_range_bri      :  3;            //bits : 2..0
			unsigned                        :  1;
			unsigned pre_ord_range_dark     :  3;            //bits : 6..4
			unsigned                        :  1;
			unsigned pre_ord_protect_th     : 10;            //bits : 17..8
			unsigned                        :  2;
			unsigned pre_ord_blend_w        :  8;            //bits : 27..20
		} bit;
		UINT32 word;
	} pre_register_106; // 0x1a8;

	union {
		struct {
			unsigned pre_ord_bri_wlut0      :  4;            //bits : 3..0
			unsigned pre_ord_bri_wlut1      :  4;            //bits : 7..4
			unsigned pre_ord_bri_wlut2      :  4;            //bits : 11..8
			unsigned pre_ord_bri_wlut3      :  4;            //bits : 15..12
			unsigned pre_ord_bri_wlut4      :  4;            //bits : 19..16
			unsigned pre_ord_bri_wlut5      :  4;            //bits : 23..20
			unsigned pre_ord_bri_wlut6      :  4;            //bits : 27..24
			unsigned pre_ord_bri_wlut7      :  4;            //bits : 31..28
		} bit;
		UINT32 word;
	} pre_register_107; // 0x1ac;

	union {
		struct {
			unsigned pre_ord_dark_wlut0     :  4;            //bits : 3..0
			unsigned pre_ord_dark_wlut1     :  4;            //bits : 7..4
			unsigned pre_ord_dark_wlut2     :  4;            //bits : 11..8
			unsigned pre_ord_dark_wlut3     :  4;            //bits : 15..12
			unsigned pre_ord_dark_wlut4     :  4;            //bits : 19..16
			unsigned pre_ord_dark_wlut5     :  4;            //bits : 23..20
			unsigned pre_ord_dark_wlut6     :  4;            //bits : 27..24
			unsigned pre_ord_dark_wlut7     :  4;            //bits : 31..28
		} bit;
		UINT32 word;
	} pre_register_108; // 0x1b0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_109; // 0x1b4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_110; // 0x1b8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_111; // 0x1bc;

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_112; // 0x01c0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_113; // 0x01c4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_114; // 0x01c8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_115; // 0x01cc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_116; // 0x01d0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_117; // 0x01d4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_118; // 0x01d8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_119; // 0x01dc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_120; // 0x01e0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_121; // 0x01e4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_122; // 0x01e8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_123; // 0x01ec

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_124; // 0x01f0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_125; // 0x01f4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_126; // 0x01f8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_127; // 0x01fc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_128; // 0x0200

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_129; // 0x0204

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_130; // 0x0208

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_131; // 0x020c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_132; // 0x0210

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_133; // 0x0214

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_134; // 0x0218

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_135; // 0x021c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_136; // 0x0220

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_137; // 0x0224

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_138; // 0x0228

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_139; // 0x022c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_140; // 0x0230

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_141; // 0x0234

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_142; // 0x0238

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_143; // 0x023c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_144; // 0x0240

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_145; // 0x0244

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_146; // 0x0248

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_147; // 0x024c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_148; // 0x0250

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_149; // 0x0254

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_150; // 0x0258

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_151; // 0x025c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_152; // 0x0260

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_153; // 0x0264

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_154; // 0x0268

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_155; // 0x026c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_156; // 0x0270

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_157; // 0x0274

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_158; // 0x0278

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_159; // 0x027c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_160; // 0x0280

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_161; // 0x0284

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_162; // 0x0288

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_163; // 0x028c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_164; // 0x0290

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_165; // 0x0294

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_166; // 0x0298

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_167; // 0x029c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_168; // 0x02a0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_169; // 0x02a4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_170; // 0x02a8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_171; // 0x02ac

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_172; // 0x02b0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_173; // 0x02b4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_174; // 0x02b8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_175; // 0x02bc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_176; // 0x02c0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_177; // 0x02c4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_178; // 0x02c8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_179; // 0x02cc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_180; // 0x02d0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_181; // 0x02d4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_182; // 0x02d8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_183; // 0x02dc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_184; // 0x02e0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_185; // 0x02e4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_186; // 0x02e8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_187; // 0x02ec

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_188; // 0x02f0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_189; // 0x02f4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_190; // 0x02f8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_191; // 0x02fc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_192; // 0x0300

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_193; // 0x0304

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_194; // 0x0308

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_195; // 0x030c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_196; // 0x0310

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_197; // 0x0314

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_198; // 0x0318

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_199; // 0x031c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_200; // 0x0320

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_201; // 0x0324

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_202; // 0x0328

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_203; // 0x032c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_204; // 0x0330

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_205; // 0x0334

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_206; // 0x0338

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_207; // 0x033c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_208; // 0x0340

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_209; // 0x0344

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_210; // 0x0348

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_211; // 0x034c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_212; // 0x0350

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_213; // 0x0354

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_214; // 0x0358

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_215; // 0x035c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_216; // 0x0360

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_217; // 0x0364

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_218; // 0x0368

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_219; // 0x036c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_220; // 0x0370

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_221; // 0x0374

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_222; // 0x0378

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_223; // 0x037c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_224; // 0x0380

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_225; // 0x0384

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_226; // 0x0388

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_227; // 0x038c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_228; // 0x0390

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_229; // 0x0394

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_230; // 0x0398

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_231; // 0x039c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_232; // 0x03a0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_233; // 0x03a4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_234; // 0x03a8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_235; // 0x03ac

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_236; // 0x03b0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_237; // 0x03b4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_238; // 0x03b8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_239; // 0x03bc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_240; // 0x03c0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_241; // 0x03c4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_242; // 0x03c8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_243; // 0x03cc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_244; // 0x03d0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_245; // 0x03d4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_246; // 0x03d8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_247; // 0x03dc

	union {
		struct {
			unsigned pre_ll_table_index0        : 8;        // bits : 7_0
			unsigned pre_ll_table_index1        : 8;        // bits : 15_8
			unsigned pre_ll_table_index2        : 8;        // bits : 23_16
			unsigned pre_ll_table_index3        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} pre_register_248; // 0x3e0;

	union {
		struct {
			unsigned pre_ll_table_index4        : 8;        // bits : 7_0
			unsigned pre_ll_table_index5        : 8;        // bits : 15_8
			unsigned pre_ll_table_index6        : 8;        // bits : 23_16
			unsigned pre_ll_table_index7        : 8;        // bits : 31_24
		} bit;
		UINT32 word;
	} pre_register_249; // 0x3e4;

	union {
		struct {
			unsigned pre_ll_table_index8         : 8;       // bits : 7_0
			unsigned pre_ll_table_index9         : 8;       // bits : 15_8
			unsigned pre_ll_table_index10        : 8;       // bits : 23_16
			unsigned pre_ll_table_index11        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} pre_register_250; // 0x3e8;

	union {
		struct {
			unsigned pre_ll_table_index12        : 8;       // bits : 7_0
			unsigned pre_ll_table_index13        : 8;       // bits : 15_8
			unsigned pre_ll_table_index14        : 8;       // bits : 23_16
			unsigned pre_ll_table_index15        : 8;       // bits : 31_24
		} bit;
		UINT32 word;
	} pre_register_251; // 0x3ec;

	union {
		struct {
			unsigned pre_axi_disable           : 1;     // bits : 0
			unsigned pre_axi_ch_idle           : 1;     // bits : 1
			unsigned pre_axi_channel_en        : 15;    // bits : 16_2
			unsigned pre_axi_lock_dis          : 15;    // bits : 31_17
		} bit;
		UINT32 word;
	} pre_register_252; // 0x3f0;

	union {
		struct {
			unsigned pre_axi_ch_sta        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} pre_register_253; // 0x3f4;

	union {
		struct {
			unsigned pre_r_ostd_num            : 8;     // bits : 7_0
			unsigned pre_w_ostd_num            : 8;     // bits : 15_8
		} bit;
		UINT32 word;
	} pre_register_254; // 0x3f8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_255; // 0x3fc;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_256; // 0x400;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_257; // 0x404;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_258; // 0x408;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_259; // 0x40c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_260; // 0x410;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_261; // 0x414;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_262; // 0x418;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_263; // 0x41c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_264; // 0x420;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_265; // 0x424;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_266; // 0x428;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_267; // 0x42c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_268; // 0x430;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_269; // 0x434;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_270; // 0x438;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_271; // 0x43c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_272; // 0x440;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_273; // 0x444;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_274; // 0x448;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_275; // 0x44c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_276; // 0x450;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_277; // 0x454;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_278; // 0x458;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_279; // 0x45c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_280; // 0x460;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_281; // 0x464;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_282; // 0x468;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_283; // 0x46c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_284; // 0x470;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_285; // 0x474;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_286; // 0x478;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_287; // 0x47c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_288; // 0x480;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_289; // 0x484;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_290; // 0x488;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_291; // 0x48c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_292; // 0x490;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_293; // 0x494;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_294; // 0x498;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_295; // 0x49c;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_296; // 0x4a0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_297; // 0x4a4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_298; // 0x4a8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_299; // 0x4ac;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_300; // 0x4b0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_301; // 0x4b4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_302; // 0x4b8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_303; // 0x4bc;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_304; // 0x4c0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_305; // 0x4c4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_306; // 0x4c8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_307; // 0x4cc;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_308; // 0x4d0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_309; // 0x4d4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_310; // 0x4d8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_311; // 0x4dc;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_312; // 0x4e0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_313; // 0x4e4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_314; // 0x4e8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_315; // 0x4ec;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_316; // 0x4f0;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_317; // 0x4f4;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_318; // 0x4f8;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_319; // 0x4fc;

	union {
		struct {
			unsigned pre_f_nrs0_en        : 1;      // bits : 0
		} bit;
		UINT32 word;
	} pre_register_320; // 0x500;

	union {
		struct {
			unsigned pre_f_nrs0_str0        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned pre_f_nrs0_str1        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_321; // 0x504;

	union {
		struct {
			unsigned pre_f_nrs0_str2        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned pre_f_nrs0_str3        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_322; // 0x508;

	union {
		struct {
			unsigned pre_f_nrs0_str4        : 10;       // bits : 9_0
			unsigned                        : 6;
			unsigned pre_f_nrs0_str5        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_323; // 0x50c;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_324; // 0x510;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_325; // 0x514;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_326; // 0x518;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_327; // 0x51c;

	union {
		struct {
			unsigned pre_f_fu_ym_sel               : 2;     // bits : 1_0
			unsigned                               : 2;
			unsigned pre_f_fu_bcn_sel              : 2;     // bits : 5_4
			unsigned                               : 2;
			unsigned pre_f_fu_bcd_sel              : 2;     // bits : 9_8
			unsigned                               : 2;
			unsigned pre_f_fu_mode                 : 2;     // bits : 13_12
			unsigned                               : 14;
			unsigned pre_f_fu_dbg_mode             : 4;     // bits : 31_28
		} bit;
		UINT32 word;
	} pre_register_328; // 0x520;

	union {
		struct {
			unsigned pre_f_fu_bcnl_p0                     : 12;     // bits : 11_0
			unsigned                                      : 4;
			unsigned pre_f_fu_bcnl_range                  : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_329; // 0x524;

	union {
		struct {
			unsigned pre_f_fu_bcns_p0                     : 12;     // bits : 11_0
			unsigned                                      : 4;
			unsigned pre_f_fu_bcns_range                  : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_330; // 0x528;

	union {
		struct {
			unsigned pre_f_fu_bcdl_p0                     : 12;     // bits : 11_0
			unsigned                                      : 4;
			unsigned pre_f_fu_bcdl_range                  : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_331; // 0x52c;

	union {
		struct {
			unsigned pre_f_fu_bcds_p0                     : 12;     // bits : 11_0
			unsigned                                      : 4;
			unsigned pre_f_fu_bcds_range                  : 12;     // bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_332; // 0x530;

	union {
		struct {
			unsigned pre_f_fu_bcnl_slope        : 16;       // bits : 15_0
			unsigned pre_f_fu_bcns_slope        : 16;       // bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_333; // 0x534;

	union {
		struct {
			unsigned pre_f_fu_bcdl_slope        : 16;       // bits : 15_0
			unsigned pre_f_fu_bcds_slope        : 16;       // bits : 31_16

		} bit;
		UINT32 word;
	} pre_register_334; // 0x538;

	union {
		struct {
			unsigned pre_f_fu_evratio        : 13;       // bits : 12_0
		} bit;
		UINT32 word;
	} pre_register_335; // 0x53c;

	union {
		struct {
			unsigned pre_f_fu_mc_lumthr     :  8;            //bits : 7..0
			unsigned                        :  4;            //bits : 11..8
			unsigned pre_f_fu_mc_diff_ratio :  2;            //bits : 13..12
		} bit;
		UINT32 word;
	} pre_register_336; // 0x540;

	union {
		struct {
			unsigned pre_f_fu_mc_lut_dwp0   :  5;            //bits : 4..0
			unsigned pre_f_fu_mc_lut_dwp1   :  5;            //bits : 9..5
			unsigned pre_f_fu_mc_lut_dwp2   :  5;            //bits : 14..10
			unsigned pre_f_fu_mc_lut_dwp3   :  5;            //bits : 19..15
			unsigned pre_f_fu_mc_lut_dwp4   :  5;            //bits : 24..20
			unsigned pre_f_fu_mc_lut_dwp5   :  5;            //bits : 29..25
		} bit;
		UINT32 word;
	} pre_register_337; // 0x544;

	union {
		struct {
			unsigned pre_f_fu_mc_lut_dwp6   :  5;            //bits : 4..0
			unsigned pre_f_fu_mc_lut_dwp7   :  5;            //bits : 9..5
			unsigned pre_f_fu_mc_lut_dwp8   :  5;            //bits : 14..10
			unsigned pre_f_fu_mc_lut_dwp9   :  5;            //bits : 19..15
			unsigned pre_f_fu_mc_lut_dwp10  :  5;            //bits : 24..20
			unsigned pre_f_fu_mc_lut_dwp11  :  5;            //bits : 29..25
		} bit;
		UINT32 word;
	} pre_register_338; // 0x548;

	union {
		struct {
			unsigned pre_f_fu_mc_lut_dwp12  :  5;            //bits : 4..0
			unsigned pre_f_fu_mc_lut_dwp13  :  5;            //bits : 9..5
			unsigned pre_f_fu_mc_lut_dwp14  :  5;            //bits : 14..10
			unsigned pre_f_fu_mc_lut_dwp15  :  5;            //bits : 19..15
			unsigned pre_f_fu_mc_lut_dwd    :  5;            //bits : 24..20
		} bit;
		UINT32 word;
	} pre_register_339; // 0x54c;

	union {
		struct {
			unsigned pre_f_ds0_th           : 12;            //bits : 11..0
			unsigned                        :  4;
			unsigned pre_f_ds0_step         :  8;            //bits : 23..16
			unsigned pre_f_ds0_lb           :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} pre_register_340; // 0x550;

	union {
		struct {
			unsigned pre_f_ds1_th           : 12;            //bits : 11..0
			unsigned                        :  4;
			unsigned pre_f_ds1_step         :  8;            //bits : 23..16
			unsigned pre_f_ds1_lb           :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} pre_register_341; // 0x554;

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_342; // 0x558;

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_343; // 0x55c;

	union {
		struct {
			unsigned pre_f_fc_ym_sel        :  2;            //bits : 1..0
			unsigned pre_f_fc_yvweight      :  4;            //bits : 5..2
		} bit;
		UINT32 word;
	} pre_register_344; // 0x560;

	union {
		struct {
			unsigned pre_f_fc_yweight_lut0  :  8;            //bits : 7..0
			unsigned pre_f_fc_yweight_lut1  :  8;            //bits : 15..8
			unsigned pre_f_fc_yweight_lut2  :  8;            //bits : 23..16
			unsigned pre_f_fc_yweight_lut3  :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} pre_register_345; // 0x564;

	union {
		struct {
			unsigned pre_f_fc_yweight_lut4  :  8;            //bits : 7..0
			unsigned pre_f_fc_yweight_lut5  :  8;            //bits : 15..8
			unsigned pre_f_fc_yweight_lut6  :  8;            //bits : 23..16
			unsigned pre_f_fc_yweight_lut7  :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} pre_register_346; // 0x568;

	union {
		struct {
			unsigned pre_f_fc_yweight_lut8  :  8;            //bits : 7..0
			unsigned pre_f_fc_yweight_lut9  :  8;            //bits : 15..8
			unsigned pre_f_fc_yweight_lut10 :  8;            //bits : 23..16
			unsigned pre_f_fc_yweight_lut11 :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} pre_register_347; // 0x56c;

	union {
		struct {
			unsigned pre_f_fc_yweight_lut12 :  8;            //bits : 7..0
			unsigned pre_f_fc_yweight_lut13 :  8;            //bits : 15..8
			unsigned pre_f_fc_yweight_lut14 :  8;            //bits : 23..16
			unsigned pre_f_fc_yweight_lut15 :  8;            //bits : 31..24
		} bit;
		UINT32 word;
	} pre_register_348; // 0x570;

	union {
		struct {
			unsigned pre_f_fc_yweight_lut16 :  8;            //bits : 7..0
		} bit;
		UINT32 word;
	} pre_register_349; // 0x574;

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_0        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_350; // 0x0578

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_1        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_351; // 0x057c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_2        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_352; // 0x0580

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_3        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_353; // 0x0584

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_4        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_354; // 0x0588

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_5        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_355; // 0x058c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_6        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_356; // 0x0590

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_7        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_357; // 0x0594

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_8        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_358; // 0x0598

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_9        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_359; // 0x059c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_10        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_360; // 0x05a0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_11        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_361; // 0x05a4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_12        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_362; // 0x05a8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_13        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_363; // 0x05ac

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_14        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_364; // 0x05b0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_15        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_365; // 0x05b4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_16        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_366; // 0x05b8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_17        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_367; // 0x05bc

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_18        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_368; // 0x05c0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_19        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_369; // 0x05c4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_20        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_370; // 0x05c8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_21        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_371; // 0x05cc

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_22        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_372; // 0x05d0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_23        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_373; // 0x05d4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_24        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_374; // 0x05d8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_25        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_375; // 0x05dc

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_26        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_376; // 0x05e0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_27        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_377; // 0x05e4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_28        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_378; // 0x05e8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_29        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_379; // 0x05ec

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_30        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_380; // 0x05f0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_31        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_381; // 0x05f4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_32        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_382; // 0x05f8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_33        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_383; // 0x05fc

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_34        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_384; // 0x0600

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_35        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_385; // 0x0604

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_36        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_386; // 0x0608

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_37        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_387; // 0x060c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_38        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_388; // 0x0610

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_39        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_389; // 0x0614

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_40        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_390; // 0x0618

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_41        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_391; // 0x061c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_42        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_392; // 0x0620

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_43        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_393; // 0x0624

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_44        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_394; // 0x0628

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_45        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_395; // 0x062c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_46        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_396; // 0x0630

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_47        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_397; // 0x0634

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_48        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_398; // 0x0638

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_49        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_399; // 0x063c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_50        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_400; // 0x0640

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_51        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_401; // 0x0644

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_52        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_402; // 0x0648

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_53        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_403; // 0x064c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_54        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_404; // 0x0650

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_55        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_405; // 0x0654

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_56        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_406; // 0x0658

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_57        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_407; // 0x065c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_58        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_408; // 0x0660

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_59        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_409; // 0x0664

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_60        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_410; // 0x0668

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_61        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_411; // 0x066c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_62        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_412; // 0x0670

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_63        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_413; // 0x0674

	union
	{
		struct
		{
			unsigned pre_f_fcurve_l_64        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_414; // 0x0678

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_0        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_415; // 0x067c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_1        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_416; // 0x0680

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_2        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_417; // 0x0684

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_3        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_418; // 0x0688

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_4        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_419; // 0x068c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_5        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_420; // 0x0690

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_6        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_421; // 0x0694

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_7        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_422; // 0x0698

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_8        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_423; // 0x069c

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_9        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_424; // 0x06a0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_10        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_425; // 0x06a4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_11        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_426; // 0x06a8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_12        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_427; // 0x06ac

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_13        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_428; // 0x06b0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_14        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_429; // 0x06b4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_15        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_430; // 0x06b8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_r_16        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_431; // 0x06bc

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_0        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_432; // 0x06c0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_1        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_433; // 0x06c4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_2        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_434; // 0x06c8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_3        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_435; // 0x06cc

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_4        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_436; // 0x06d0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_5        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_437; // 0x06d4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_6        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_438; // 0x06d8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_7        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_439; // 0x06dc

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_8        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_440; // 0x06e0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_9        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_441; // 0x06e4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_10        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_442; // 0x06e8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_11        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_443; // 0x06ec

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_12        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_444; // 0x06f0

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_13        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_445; // 0x06f4

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_14        : 20;		// bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_446; // 0x06f8

	union
	{
		struct
		{
			unsigned pre_f_fcurve_end_15        : 20;		// bits : 19_0
			unsigned                            : 4;		//
			unsigned pre_f_fcurve_ev_fmt        : 3;		// bits : 26_24
		} bit;
		UINT32 word;
	} pre_register_447; // 0x06fc


	/*  union
	  {
	      struct
	      {
	          unsigned pre_r_degamma_en       :  1;            //bits : 0
	          unsigned                        :  3;
	          unsigned pre_r_dith_en          :  1;            //bits : 4
	          unsigned                        :  3;
	          unsigned pre_r_segbitno         :  2;            //bits : 9..8
	          unsigned                        :  2;
	          unsigned pre_r_dith_rst         :  1;            //bits : 12
	      } bit;
	      UINT32 word;
	  } pre_register_448; // 0x700;

	  union
	  {
	      struct
	      {
	          unsigned pre_r_dct_qtbl0_idx    :  5;            //bits : 4..0
	          unsigned                        :  3;
	          unsigned pre_r_dct_qtbl1_idx    :  5;            //bits : 12..8
	          unsigned                        :  3;
	          unsigned pre_r_dct_qtbl2_idx    :  5;            //bits : 20..16
	          unsigned                        :  3;
	          unsigned pre_r_dct_qtbl3_idx    :  5;            //bits : 28..24
	      } bit;
	      UINT32 word;
	  } pre_register_449; // 0x704;

	  union
	  {
	      struct
	      {
	          unsigned pre_r_dct_qtbl4_idx    :  5;            //bits : 4..0
	          unsigned                        :  3;
	          unsigned pre_r_dct_qtbl5_idx    :  5;            //bits : 12..8
	          unsigned                        :  3;
	          unsigned pre_r_dct_qtbl6_idx    :  5;            //bits : 20..16
	          unsigned                        :  3;
	          unsigned pre_r_dct_qtbl7_idx    :  5;            //bits : 28..24
	      } bit;
	      UINT32 word;
	  } pre_register_450; // 0x708;

	  union
	  {
	      struct
	      {
	          unsigned pre_r_out_rand1_init1        : 4;      // bits : 3_0
	          unsigned pre_r_out_rand1_init2        : 15;     // bits : 18_4
	      } bit;
	      UINT32 word;
	  } pre_register_451; // 0x70c;

	  union
	  {
	      struct
	      {
	          unsigned pre_r_out_rand2_init1        : 4;      // bits : 3_0
	          unsigned pre_r_out_rand2_init2        : 15;     // bits : 18_4
	      } bit;
	      UINT32 word;
	  } pre_register_452; // 0x710;

	  union
	  {
	      struct
	      {
	          unsigned pre_r_out_rand3_init1        : 4;      // bits : 3_0
	          unsigned pre_r_out_rand3_init2        : 15;     // bits : 18_4
	      } bit;
	      UINT32 word;
	  } pre_register_453; // 0x714;*/


	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_448; // 0x0700

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_449; // 0x0704

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_450; // 0x0708

	union {
		struct {
			unsigned pre_fpn_en                 : 1;        // bits : 0
			unsigned pre_fpn_cgain_range        : 1;        // bits : 1
		} bit;
		UINT32 word;
	} pre_register_451; // 0x070c

	union {
		struct {
			unsigned pre_fpn_p0_cgain_r         : 10;       // bits : 9_0
			unsigned                            : 6;
			unsigned pre_fpn_p0_cgain_gr        : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_452; // 0x0710

	union {
		struct {
			unsigned pre_fpn_p0_cgain_gb        : 10;       // bits : 9_0
			unsigned                            : 6;
			unsigned pre_fpn_p0_cgain_b         : 10;       // bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_453; // 0x0714

	union {
		struct {
			unsigned pre_fpn_p0_cgain_ir        : 10;       // bits : 9_0
		} bit;
		UINT32 word;
	} pre_register_454; // 0x0718

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_455; // 0x071c

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_456; // 0x0720

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_457; // 0x0724

	union {
		struct {
			unsigned pre_fpn_p0_cofs_r         : 10;        // bits : 9_0
			unsigned                           : 6;
			unsigned pre_fpn_p0_cofs_gr        : 10;        // bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_458; // 0x0728

	union {
		struct {
			unsigned pre_fpn_p0_cofs_gb        : 10;        // bits : 9_0
			unsigned                           : 6;
			unsigned pre_fpn_p0_cofs_b         : 10;        // bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_459; // 0x072c

	union {
		struct {
			unsigned pre_fpn_p0_cofs_ir        : 10;        // bits : 9_0
		} bit;
		UINT32 word;
	} pre_register_460; // 0x0730

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_461; // 0x0734

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_462; // 0x0738

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_463; // 0x073c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_464; // 0x0740

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_465; // 0x0744

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_466; // 0x0748

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_467; // 0x074c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_468; // 0x0750

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_469; // 0x0754

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_470; // 0x0758

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_471; // 0x075c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_472; // 0x0760

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_473; // 0x0764

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_474; // 0x0768

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_475; // 0x076c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_476; // 0x0770

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_477; // 0x0774

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_478; // 0x0778

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_479; // 0x077c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_480; // 0x0780

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_481; // 0x0784

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_482; // 0x0788

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_483; // 0x078c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_484; // 0x0790

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_485; // 0x0794

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_486; // 0x0798

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_487; // 0x079c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_488; // 0x07a0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_489; // 0x07a4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_490; // 0x07a8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_491; // 0x07ac

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_492; // 0x07b0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_493; // 0x07b4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_494; // 0x07b8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_495; // 0x07bc

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_496; // 0x07c0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_497; // 0x07c4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_498; // 0x07c8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_499; // 0x07cc

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_500; // 0x07d0

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_501; // 0x07d4

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_502; // 0x07d8

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_503; // 0x07dc

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_504; // 0x07e0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_505; // 0x07e4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_506; // 0x07e8

	union {
		struct {

			unsigned pre_fusion_wat_out_en             : 1;		// bits : 0
			unsigned pre_fusion_wat_out_wait_en        : 1;		// bits : 1
		} bit;
		UINT32 word;
	} pre_register_507; // 0x07ec

	union {
		struct {

			unsigned                           : 2;		// bits : 1_0
			unsigned pre_fusion_wat_sao        : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_508; // 0x07f0

	union {
		struct {

			unsigned                               : 2;		// bits : 1_0
			unsigned pre_fusion_wat_ofso           : 14;		// bits : 15_2
			unsigned                               : 8;
			unsigned pre_fusion_wat_sao_msb        : 4;		// bits : 27_24
		} bit;
		UINT32 word;
	} pre_register_509; // 0x07f4

	union {
		struct {
			unsigned pre_ll_cmd_start_addr_info        : 32;        // bits : 31_0
		} bit;
		UINT32 word;
	} pre_register_510; // 0x07f8

	union {
		struct {
			unsigned pre_ll_cmdprs_cnt        : 20;     // bits : 19_0
		} bit;
		UINT32 word;
	} pre_register_511; // 0x07fc

	union {
		struct {
			unsigned pre_r_decode_en         : 1;       // bits : 0
			unsigned pre_r_decode2_en        : 1;       // bits : 1
			unsigned pre_r_degamma_en        : 1;       // bits : 2
			unsigned                         : 1;
			unsigned pre_r_dith_en           : 1;       // bits : 4
			unsigned                         : 3;
			unsigned pre_r_segbitno          : 2;       // bits : 8
			unsigned                         : 2;
			unsigned pre_r_dith_rst          : 1;       // bits : 12
		} bit;
		UINT32 word;
	} pre_register_512; // 0x0800

	union {
		struct {
			unsigned pre_r_dct_qtbl0_idx        : 5;        // bits : 4_0
			unsigned                            : 3;
			unsigned pre_r_dct_qtbl1_idx        : 5;        // bits : 12_8
			unsigned                            : 3;
			unsigned pre_r_dct_qtbl2_idx        : 5;        // bits : 20_16
			unsigned                            : 3;
			unsigned pre_r_dct_qtbl3_idx        : 5;        // bits : 28_24
		} bit;
		UINT32 word;
	} pre_register_513; // 0x0804

	union {
		struct {
			unsigned pre_r_dct_qtbl4_idx        : 5;        // bits : 4_0
			unsigned                            : 3;
			unsigned pre_r_dct_qtbl5_idx        : 5;        // bits : 12_8
			unsigned                            : 3;
			unsigned pre_r_dct_qtbl6_idx        : 5;        // bits : 20_16
			unsigned                            : 3;
			unsigned pre_r_dct_qtbl7_idx        : 5;        // bits : 28_24
		} bit;
		UINT32 word;
	} pre_register_514; // 0x0808

	union {
		struct {
			unsigned pre_r_out_rand1_init1        : 4;      // bits : 3_0
			unsigned pre_r_out_rand1_init2        : 15;     // bits : 18_4
		} bit;
		UINT32 word;
	} pre_register_515; // 0x080c

	union {
		struct {
			unsigned pre_r_out_rand2_init1        : 4;      // bits : 3_0
			unsigned pre_r_out_rand2_init2        : 15;     // bits : 18_4
		} bit;
		UINT32 word;
	} pre_register_516; // 0x0810

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_517; // 0x0814

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_518; // 0x0818

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_519; // 0x081c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_520; // 0x0820

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_521; // 0x0824

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_522; // 0x0828

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_523; // 0x082c

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_524; // 0x0830

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_525; // 0x0834

	union {
		struct {
			unsigned pre_stripe_size0        : 11;      // bits : 10_0
			unsigned                         : 1;
			unsigned pre_stripe_num          : 4;       // bits : 15_12
			unsigned pre_stripe_size1        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} pre_register_526; // 0x0838

	union {
		struct {
			unsigned pre_stripe_size2        : 11;      // bits : 10_0
			unsigned                         : 5;
			unsigned pre_stripe_size3        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} pre_register_527; // 0x083c

	union {
		struct {
			unsigned pre_stripe_size4        : 11;      // bits : 10_0
			unsigned                         : 5;
			unsigned pre_stripe_size5        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} pre_register_528; // 0x0840

	union {
		struct {
			unsigned pre_stripe_size6        : 11;      // bits : 10_0
			unsigned                         : 5;
			unsigned pre_stripe_size7        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} pre_register_529; // 0x0844

	union {
		struct {
			unsigned                         : 3;
			unsigned pre_stripe_overlap      : 7;      // bits : 9_3
		} bit;
		UINT32 word;
	} pre_register_530; // 0x0848

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_531; // 0x084c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_532; // 0x0850

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_533; // 0x0854

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_534; // 0x0858

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_535; // 0x085c

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_536; // 0x0860

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_537; // 0x0864

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_538; // 0x0868

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_539; // 0x086c

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_540; // 0x0870

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_541; // 0x0874

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_542; // 0x0878

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_543; // 0x087c

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_544; // 0x0880

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_545; // 0x0884

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_546; // 0x0888

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_547; // 0x088c

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_548; // 0x0890

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_549; // 0x0894

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_550; // 0x0898

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_551; // 0x089c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_552; // 0x08a0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_553; // 0x08a4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_554; // 0x08a8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_555; // 0x08ac

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_556; // 0x08b0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_557; // 0x08b4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_558; // 0x08b8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_559; // 0x08bc

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_560; // 0x08c0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_561; // 0x08c4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_562; // 0x08c8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_563; // 0x08cc

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_564; // 0x08d0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_565; // 0x08d4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_566; // 0x08d8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_567; // 0x08dc

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_568; // 0x08e0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_569; // 0x08e4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_570; // 0x08e8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_571; // 0x08ec

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_572; // 0x08f0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_573; // 0x08f4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_574; // 0x08f8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_575; // 0x08fc


	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_576; // 0x0900

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_577; // 0x0904

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_578; // 0x0908

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_579; // 0x090c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_580; // 0x0910

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_581; // 0x0914

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_582; // 0x0918

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_583; // 0x091c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_584; // 0x0920

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_585; // 0x0924

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_586; // 0x0928

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_587; // 0x092c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_588; // 0x0930

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_589; // 0x0934

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_590; // 0x0938

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_591; // 0x093c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_592; // 0x0940

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_593; // 0x0944

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_594; // 0x0948

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_595; // 0x094c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_596; // 0x0950

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_597; // 0x0954

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_598; // 0x0958

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_599; // 0x095c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_600; // 0x0960

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_601; // 0x0964

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_602; // 0x0968

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_603; // 0x096c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_604; // 0x0970

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_605; // 0x0974

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_606; // 0x0978

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_607; // 0x097c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_608; // 0x0980

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_609; // 0x0984

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_610; // 0x0988

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_611; // 0x098c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_612; // 0x0990

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_613; // 0x0994

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_614; // 0x0998

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_615; // 0x099c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_616; // 0x09a0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_617; // 0x09a4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_618; // 0x09a8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_619; // 0x09ac

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_620; // 0x09b0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_621; // 0x09b4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_622; // 0x09b8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_623; // 0x09bc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_624; // 0x09c0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_625; // 0x09c4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_626; // 0x09c8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_627; // 0x09cc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_628; // 0x09d0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_629; // 0x09d4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_630; // 0x09d8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_631; // 0x09dc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_632; // 0x09e0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_633; // 0x09e4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_634; // 0x09e8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_635; // 0x09ec

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_636; // 0x09f0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_637; // 0x09f4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_638; // 0x09f8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_639; // 0x09fc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_640; // 0x0a00

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_641; // 0x0a04

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_642; // 0x0a08

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_643; // 0x0a0c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_644; // 0x0a10

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_645; // 0x0a14

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_646; // 0x0a18

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_647; // 0x0a1c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_648; // 0x0a20

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_649; // 0x0a24

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_650; // 0x0a28

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_651; // 0x0a2c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_652; // 0x0a30

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_653; // 0x0a34

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_654; // 0x0a38

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_655; // 0x0a3c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_656; // 0x0a40

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_657; // 0x0a44

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_658; // 0x0a48

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_659; // 0x0a4c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_660; // 0x0a50

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_661; // 0x0a54

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_662; // 0x0a58

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_663; // 0x0a5c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_664; // 0x0a60

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_665; // 0x0a64

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_666; // 0x0a68

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_667; // 0x0a6c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_668; // 0x0a70

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_669; // 0x0a74

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_670; // 0x0a78

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_671; // 0x0a7c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_672; // 0x0a80

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_673; // 0x0a84

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_674; // 0x0a88

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_675; // 0x0a8c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_676; // 0x0a90

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_677; // 0x0a94

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_678; // 0x0a98

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_679; // 0x0a9c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_680; // 0x0aa0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_681; // 0x0aa4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_682; // 0x0aa8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_683; // 0x0aac

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_684; // 0x0ab0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_685; // 0x0ab4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_686; // 0x0ab8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_687; // 0x0abc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_688; // 0x0ac0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_689; // 0x0ac4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_690; // 0x0ac8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_691; // 0x0acc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_692; // 0x0ad0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_693; // 0x0ad4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_694; // 0x0ad8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_695; // 0x0adc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_696; // 0x0ae0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_697; // 0x0ae4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_698; // 0x0ae8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_699; // 0x0aec

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_700; // 0x0af0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_701; // 0x0af4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_702; // 0x0af8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_703; // 0x0afc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_704; // 0x0b00

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_705; // 0x0b04

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_706; // 0x0b08

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_707; // 0x0b0c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_708; // 0x0b10

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_709; // 0x0b14

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_710; // 0x0b18

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_711; // 0x0b1c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_712; // 0x0b20

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_713; // 0x0b24

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_714; // 0x0b28

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_715; // 0x0b2c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_716; // 0x0b30

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_717; // 0x0b34

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_718; // 0x0b38

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_719; // 0x0b3c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_720; // 0x0b40

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_721; // 0x0b44

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_722; // 0x0b48

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_723; // 0x0b4c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_724; // 0x0b50

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_725; // 0x0b54

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_726; // 0x0b58

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_727; // 0x0b5c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_728; // 0x0b60

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_729; // 0x0b64

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_730; // 0x0b68

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_731; // 0x0b6c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_732; // 0x0b70

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_733; // 0x0b74

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_734; // 0x0b78

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_735; // 0x0b7c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_736; // 0x0b80

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_737; // 0x0b84

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_738; // 0x0b88

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_739; // 0x0b8c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_740; // 0x0b90

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_741; // 0x0b94

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_742; // 0x0b98

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_743; // 0x0b9c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_744; // 0x0ba0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_745; // 0x0ba4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_746; // 0x0ba8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_747; // 0x0bac

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_748; // 0x0bb0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_749; // 0x0bb4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_750; // 0x0bb8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_751; // 0x0bbc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_752; // 0x0bc0

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_753; // 0x0bc4

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_754; // 0x0bc8

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_755; // 0x0bcc

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_756; // 0x0bd0

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_757; // 0x0bd4

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_758; // 0x0bd8

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_759; // 0x0bdc

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_760; // 0x0be0

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_761; // 0x0be4

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_762; // 0x0be8

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_763; // 0x0bec

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_764; // 0x0bf0

	union {
		struct {
			unsigned vdetgh2_iir2_a   : 10;       // bits : 9_0
			unsigned vdetgh2_iir2_b   : 10;       // bits : 19_10
			unsigned vdetgh2_iir2_e   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} pre_register_765; // 0x0bf4

	union {
		struct {
			unsigned vdetgh2_iir2_f   : 10;       // bits : 9_0
			unsigned vdetgh2_iir3_a   : 10;       // bits : 19_10
			unsigned vdetgh2_iir3_b   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} pre_register_766; // 0x0bf8

	union {
		struct {
			unsigned vdetgh2_iir3_e   : 10;       // bits : 9_0
			unsigned vdetgh2_iir3_f   : 10;       // bits : 19_10
		} bit;
		UINT32 word;
	} pre_register_767; // 0x0bfc

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
			unsigned va_in_sel                 : 2;		// bits : 11_10
			unsigned va_vertical_fir_en        : 1;		// bits : 12
		} bit;
		UINT32 word;
	} pre_register_768; // 0x0c00

	union {
		struct {
			unsigned                    : 2;
			unsigned dram_sao_va        : 30;       // bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_769; // 0x0c04

	union {
		struct {
			unsigned dram_sao_va_msb        : 4;        // bits : 3_0
		} bit;
		UINT32 word;
	} pre_register_770; // 0x0c08

	union {
		struct {
			unsigned                         : 2;
			unsigned dram_ofso_va            : 14;      // bits : 15_2
		} bit;
		UINT32 word;
	} pre_register_771; // 0x0c0c

	union {
		struct {
			unsigned va_win_cnt_out_sel         : 1;        // bits : 0
			unsigned                            : 3;
			unsigned va_win_high_luma_th        : 8;        // bits : 11_4
			unsigned va_energy_w                : 5;        // bits : 16_12
		} bit;
		UINT32 word;
	} pre_register_772; // 0x0c10

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
	} pre_register_773; // 0x0c14

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
	} pre_register_774; // 0x0c18

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
	} pre_register_775; // 0x0c1c

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
	} pre_register_776; // 0x0c20

	union {
		struct {
			unsigned vacc_outsel        : 1;        // bits : 0
			unsigned                    : 2;
			unsigned va_stx             : 13;	// bits : 15_3
			unsigned va_sty             : 13;	// bits : 28_16
		} bit;
		UINT32 word;
	} pre_register_777; // 0x0c24

	union {
		struct {
			unsigned va_g1hthl          : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_g1hthh          : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_778; // 0x0c28

	union {
		struct {
			unsigned va_g1vthl          : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_g1vthh          : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_779; // 0x0c2c

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
	} pre_register_780; // 0x0c30

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
	} pre_register_781; // 0x0c34

	union {
		struct {
			unsigned win0_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win0_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} pre_register_782; // 0x0c38

	union {
		struct {
			unsigned win0_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win0_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} pre_register_783; // 0x0c3c

	union {
		struct {
			unsigned win1_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win1_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} pre_register_784; // 0x0c40

	union {
		struct {
			unsigned win1_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win1_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} pre_register_785; // 0x0c44

	union {
		struct {
			unsigned win2_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win2_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} pre_register_786; // 0x0c48

	union {
		struct {
			unsigned win2_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win2_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} pre_register_787; // 0x0c4c

	union {
		struct {
			unsigned win3_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win3_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} pre_register_788; // 0x0c50

	union {
		struct {
			unsigned win3_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win3_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} pre_register_789; // 0x0c54

	union {
		struct {
			unsigned win4_stx        : 13;		// bits : 12_0
			unsigned                 : 3;
			unsigned win4_sty        : 13;		// bits : 28_16
		} bit;
		UINT32 word;
	} pre_register_790; // 0x0c58

	union {
		struct {
			unsigned win4_hsz        : 11;      // bits : 10_0
			unsigned                 : 5;
			unsigned win4_vsz        : 11;      // bits : 26_16
		} bit;
		UINT32 word;
	} pre_register_791; // 0x0c5c

	union {
		struct {
			unsigned va_win0_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} pre_register_792; // 0x0c60

	union {
		struct {
			unsigned va_win0g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win0g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_793; // 0x0c64

	union {
		struct {
			unsigned va_win0g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win0g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_794; // 0x0c68

	union {
		struct {
			unsigned va_win0g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win0g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_795; // 0x0c6c

	union {
		struct {
			unsigned va_win0g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win0g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_796; // 0x0c70

	union {
		struct {
			unsigned va_win1_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} pre_register_797; // 0x0c74

	union {
		struct {
			unsigned va_win1g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win1g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_798; // 0x0c78

	union {
		struct {
			unsigned va_win1g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win1g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_799; // 0x0c7c

	union {
		struct {
			unsigned va_win1g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win1g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_800; // 0x0c80

	union {
		struct {
			unsigned va_win1g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win1g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_801; // 0x0c84

	union {
		struct {
			unsigned va_win2_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} pre_register_802; // 0x0c88

	union {
		struct {
			unsigned va_win2g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win2g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_803; // 0x0c8c

	union {
		struct {
			unsigned va_win2g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win2g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_804; // 0x0c90

	union {
		struct {
			unsigned va_win2g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win2g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_805; // 0x0c94

	union {
		struct {
			unsigned va_win2g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win2g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_806; // 0x0c98

	union {
		struct {
			unsigned va_win3_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} pre_register_807; // 0x0c9c

	union {
		struct {
			unsigned va_win3g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win3g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_808; // 0x0ca0

	union {
		struct {
			unsigned va_win3g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win3g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_809; // 0x0ca4

	union {
		struct {
			unsigned va_win3g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win3g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_810; // 0x0ca8

	union {
		struct {
			unsigned va_win3g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win3g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_811; // 0x0cac

	union {
		struct {
			unsigned va_win4_lumacc         : 32;		// bits : 31_0
		} bit;
		UINT32 word;
	} pre_register_812; // 0x0cb0

	union {
		struct {
			unsigned va_win4g1h_vacc        : 16;		// bits : 15_0
			unsigned va_win4g1h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_813; // 0x0cb4

	union {
		struct {
			unsigned va_win4g1v_vacc        : 16;		// bits : 15_0
			unsigned va_win4g1v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_814; // 0x0cb8

	union {
		struct {
			unsigned va_win4g2h_vacc        : 16;		// bits : 15_0
			unsigned va_win4g2h_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_815; // 0x0cbc

	union {
		struct {
			unsigned va_win4g2v_vacc        : 16;		// bits : 15_0
			unsigned va_win4g2v_vacnt       : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_816; // 0x0cc0

	union {
		struct {
			unsigned va_g2hthl        : 12;		// bits : 11_0
			unsigned                  :  4;
			unsigned va_g2hthh        : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_817; // 0x0cc4

	union {
		struct {
			unsigned va_g2vthl        : 12;		// bits : 11_0
			unsigned                  :  4;
			unsigned va_g2vthh        : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_818; // 0x0cc8

	union {
		struct {
		} bit;
		UINT32 word;
	} pre_register_819; // 0x0ccc

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_820; // 0x0cd0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_821; // 0x0cd4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_822; // 0x0cd8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_823; // 0x0cdc

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_824; // 0x0ce0

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_825; // 0x0ce4

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_826; // 0x0ce8

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_827; // 0x0cec

	union {
		struct {
			unsigned                              : 4;
			unsigned va_win_ldg_low_slope         : 4;		// bits : 7_4
			unsigned va_win_ldg_high_slope        : 4;		// bits : 11_8
		} bit;
		UINT32 word;
	} pre_register_828; // 0x0cf0

	union {
		struct {
			unsigned va_win_ldg_low_th           : 8;		// bits : 7_0
			unsigned va_win_ldg_high_th          : 8;		// bits : 15_8
			unsigned va_win_ldg_low_gain         : 8;		// bits : 23_16
			unsigned va_win_ldg_high_gain        : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} pre_register_829; // 0x0cf4

	union {
		struct {
			unsigned va_gamma_0         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_1         : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_830; // 0x0cf8

	union {
		struct {
			unsigned va_gamma_2         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_3         : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_831; // 0x0cfc

	union {
		struct {
			unsigned va_gamma_4         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_5         : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_832; // 0x0d00

	union {
		struct {
			unsigned va_gamma_6         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_7         : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_833; // 0x0d04

	union {
		struct {
			unsigned va_gamma_8         : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_9         : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_834; // 0x0d08

	union {
		struct {
			unsigned va_gamma_10        : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_11        : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_835; // 0x0d0c

	union {
		struct {
			unsigned va_gamma_12        : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_13        : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_836; // 0x0d10

	union {
		struct {
			unsigned va_gamma_14        : 12;		// bits : 11_0
			unsigned                    : 4;
			unsigned va_gamma_15        : 12;		// bits : 27_16
		} bit;
		UINT32 word;
	} pre_register_837; // 0x0d14

	union {
		struct {
			unsigned va_gamma_16        : 12;		// bits : 11_0
		} bit;
		UINT32 word;
	} pre_register_838; // 0x0d18

	union {
		struct {
			unsigned vdetgh1a         : 10;       // bits : 9_0
			unsigned vdetgh1b         : 10;       // bits : 19_10
			unsigned vdetgh1c         : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} pre_register_839; // 0x0d1c

	union {
		struct {
			unsigned vdetgh1d         : 10;       // bits : 9_0
			unsigned vdetgh1_iir1_e   : 10;       // bits : 19_10
			unsigned vdetgh1_iir1_f   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} pre_register_840; // 0x0d20

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_841; // 0x0d24

	union {
		struct {
			unsigned vdetgh2a         : 10;       // bits : 9_0
			unsigned vdetgh2b         : 10;       // bits : 19_10
			unsigned vdetgh2c         : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} pre_register_842; // 0x0d28

	union {
		struct {
			unsigned vdetgh2d         : 10;       // bits : 9_0
			unsigned vdetgh2_iir1_e   : 10;       // bits : 19_10
			unsigned vdetgh2_iir1_f   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} pre_register_843; // 0x0d2c

	union {
		struct {

		} bit;
		UINT32 word;
	} pre_register_844; // 0x0d30

	union {
		struct {
			unsigned vdetgh1_iir2_a   : 10;       // bits : 9_0
			unsigned vdetgh1_iir2_b   : 10;       // bits : 19_10
			unsigned vdetgh1_iir2_e   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} pre_register_845; // 0x0d34

	union {
		struct {
			unsigned vdetgh1_iir2_f   : 10;       // bits : 9_0
			unsigned vdetgh1_iir3_a   : 10;       // bits : 19_10
			unsigned vdetgh1_iir3_b   : 10;       // bits : 29_20
		} bit;
		UINT32 word;
	} pre_register_846; // 0x0d38

	union {
		struct {
			unsigned vdetgh1_iir3_e   : 10;       // bits : 9_0
			unsigned vdetgh1_iir3_f   : 10;       // bits : 19_10
		} bit;
		UINT32 word;
	} pre_register_847; // 0x0d3c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_848; // 0x0d40

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_849; // 0x0d44

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_850; // 0x0d48

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_851; // 0x0d4c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_852; // 0x0d50

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_853; // 0x0d54

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_854; // 0x0d58

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_855; // 0x0d5c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_856; // 0x0d60

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_857; // 0x0d64

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_858; // 0x0d68

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_859; // 0x0d6c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_860; // 0x0d70

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_861; // 0x0d74

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_862; // 0x0d78

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_863; // 0x0d7c

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_864; // 0x0d80

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_865; // 0x0d84

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_866; // 0x0d88

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_867; // 0x0d8c

	union
	{
		struct
		{
			unsigned pre_cgain_r            : 10;		// bits : 9_0
			unsigned                        : 6;
			unsigned pre_cgain_gr           : 10;		// bits : 25_16
			unsigned                        : 5;
			unsigned pre_cgain_range        : 1;		// bits : 31
		} bit;
		UINT32 word;
	} pre_register_868; // 0x0d90

	union
	{
		struct
		{
			unsigned pre_cgain_gb        : 10;		// bits : 9_0
			unsigned                     : 6;
			unsigned pre_cgain_b         : 10;		// bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_869; // 0x0d94

	union
	{
		struct
		{
			unsigned pre_cofs_r         : 10;		// bits : 9_0
			unsigned                    : 6;
			unsigned pre_cofs_gr        : 10;		// bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_870; // 0x0d98

	union
	{
		struct
		{
			unsigned pre_cofs_gb        : 10;		// bits : 9_0
			unsigned                    : 6;
			unsigned pre_cofs_b         : 10;		// bits : 25_16
		} bit;
		UINT32 word;
	} pre_register_871; // 0x0d9c

	union
	{
		struct
		{
			unsigned pre_bnr_en                         : 1;		// bits : 0
			unsigned pre_bnr_ref_out_en                 : 1;		// bits : 1
			unsigned pre_bnr_sta_out_en                 : 1;		// bits : 2
			unsigned pre_bnr_sg_en                      : 1;		// bits : 3
			unsigned pre_bnr_gamma_map_out_en           : 1;		// bits : 4
			unsigned pre_bnr_err_comp_en                : 1;		// bits : 5
			unsigned pre_bnr_ref_enc_en                 : 1;		// bits : 6
			unsigned pre_bnr_ref_dec_en                 : 1;		// bits : 7
			unsigned pre_bnr_enc_gamma_en               : 1;		// bits : 8
			unsigned pre_bnr_dec_degamma_en             : 1;		// bits : 9
			unsigned pre_bnr_dec_dith_en                : 1;		// bits : 10
			unsigned pre_bnr_debug_mode                 : 1;		// bits : 11
			unsigned pre_bnr_sg_opt                     : 1;		// bits : 12
			unsigned pre_bnr_gamma_out_wait_en          : 1;		// bits : 13
			unsigned pre_bnr_aided_map_direct_en        : 1;		// bits : 14
			unsigned                                    : 5;
			unsigned pre_bnr_residue_th                 : 4;		// bits : 23_20
			unsigned pre_bnr_pf_str                     : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} pre_register_872; // 0x0da0

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_l_0        : 14;		// bits : 13_0
			unsigned                          : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_l_1        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_873; // 0x0da4

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_l_2        : 14;		// bits : 13_0
			unsigned                          : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_l_3        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_874; // 0x0da8

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_l_4        : 14;		// bits : 13_0
			unsigned                          : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_l_5        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_875; // 0x0dac

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_l_6        : 14;		// bits : 13_0
			unsigned                          : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_l_7        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_876; // 0x0db0

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_l_8        : 14;		// bits : 13_0
			unsigned                          : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_l_9        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_877; // 0x0db4

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_l_10        : 14;		// bits : 13_0
			unsigned                           : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_l_11        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_878; // 0x0db8

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_l_12        : 14;		// bits : 13_0
			unsigned                           : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_l_13        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_879; // 0x0dbc

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_l_14        : 14;		// bits : 13_0
			unsigned                           : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_l_15        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_880; // 0x0dc0

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_r_0        : 14;		// bits : 13_0
			unsigned                          : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_r_1        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_881; // 0x0dc4

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_r_2        : 14;		// bits : 13_0
			unsigned                          : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_r_3        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_882; // 0x0dc8

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_r_4        : 14;		// bits : 13_0
			unsigned                          : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_r_5        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_883; // 0x0dcc

	union
	{
		struct
		{
			unsigned pre_bnr_md_th_r_6        : 14;		// bits : 13_0
			unsigned                          : 2;		// bits : 15_14
			unsigned pre_bnr_md_th_r_7        : 14;		// bits : 29_16
		} bit;
		UINT32 word;
	} pre_register_884; // 0x0dd0

	union
	{
		struct
		{
			unsigned pre_bnr_md_base        : 14;		// bits : 13_0
			unsigned                        : 2;		// bits : 15_14
			unsigned pre_bnr_md_k1          : 8;		// bits : 23_16
			unsigned pre_bnr_md_k2          : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} pre_register_885; // 0x0dd4

	union
	{
		struct
		{
			unsigned pre_bnr_fusion_str_s        : 8;		// bits : 7_0
			unsigned pre_bnr_fusion_str_t        : 8;		// bits : 15_8
			unsigned pre_bnr_fusion_str_m        : 8;		// bits : 23_16
		} bit;
		UINT32 word;
	} pre_register_886; // 0x0dd8

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_0        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_1        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_887; // 0x0ddc

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_2        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_3        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_888; // 0x0de0

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_4        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_5        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_889; // 0x0de4

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_6        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_7        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_890; // 0x0de8

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_8        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_9        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_891; // 0x0dec

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_10        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_11        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_892; // 0x0df0

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_12        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_13        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_893; // 0x0df4

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_14        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_15        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_894; // 0x0df8

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_16        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_17        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_895; // 0x0dfc

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_18        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_19        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_896; // 0x0e00

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_20        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_21        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_897; // 0x0e04

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_22        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_23        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_898; // 0x0e08

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_24        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_25        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_899; // 0x0e0c

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_26        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_27        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_900; // 0x0e10

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_28        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_29        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_901; // 0x0e14

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_30        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_31        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_902; // 0x0e18

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_32        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_33        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_903; // 0x0e1c

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_34        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_35        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_904; // 0x0e20

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_36        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_37        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_905; // 0x0e24

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_38        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_39        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_906; // 0x0e28

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_40        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_41        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_907; // 0x0e2c

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_42        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_43        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_908; // 0x0e30

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_44        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_45        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_909; // 0x0e34

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_46        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_47        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_910; // 0x0e38

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_48        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_49        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_911; // 0x0e3c

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_50        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_51        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_912; // 0x0e40

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_52        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_53        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_913; // 0x0e44

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_54        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_55        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_914; // 0x0e48

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_56        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_57        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_915; // 0x0e4c

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_58        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_59        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_916; // 0x0e50

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_60        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_61        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_917; // 0x0e54

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_62        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_l_63        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_918; // 0x0e58

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_l_64        : 16;		// bits : 15_0
		} bit;
		UINT32 word;
	} pre_register_919; // 0x0e5c

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_r_0        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_r_1        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_920; // 0x0e60

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_r_2        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_r_3        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_921; // 0x0e64

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_r_4        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_r_5        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_922; // 0x0e68

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_r_6        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_r_7        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_923; // 0x0e6c

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_r_8        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_r_9        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_924; // 0x0e70

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_r_10        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_r_11        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_925; // 0x0e74

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_r_12        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_r_13        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_926; // 0x0e78

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_r_14        : 16;		// bits : 15_0
			unsigned pre_bnr_err_comp_r_15        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_927; // 0x0e7c

	union
	{
		struct
		{
			unsigned pre_bnr_err_comp_r_16        : 16;		// bits : 15_0
			unsigned pre_bnr_err_sft             : 2;		// bits : 17_16
		} bit;
		UINT32 word;
	} pre_register_928; // 0x0e80

	union
	{
		struct
		{
			unsigned pre_bnr_codec_rate             : 2;		// bits : 1_0
			unsigned pre_bnr_dec_dith_rst           : 1;		// bits : 2
			unsigned                                : 1;
			unsigned pre_bnr_dec_rand1_init1        : 4;		// bits : 7_4
			unsigned pre_bnr_dec_rand1_init2        : 15;		// bits : 22_8
		} bit;
		UINT32 word;
	} pre_register_929; // 0x0e84

	union
	{
		struct
		{
			unsigned pre_bnr_codec_dct_qtbl0_idx        : 5;		// bits : 4_0
			unsigned                                    : 3;
			unsigned pre_bnr_codec_dct_qtbl1_idx        : 5;		// bits : 12_8
			unsigned                                    : 3;
			unsigned pre_bnr_codec_dct_qtbl2_idx        : 5;		// bits : 20_16
			unsigned                                    : 3;
			unsigned pre_bnr_codec_dct_qtbl3_idx        : 5;		// bits : 28_24
		} bit;
		UINT32 word;
	} pre_register_930; // 0x0e88

	union
	{
		struct
		{
			unsigned pre_bnr_codec_dct_qtbl4_idx        : 5;		// bits : 4_0
			unsigned                                    : 3;
			unsigned pre_bnr_codec_dct_qtbl5_idx        : 5;		// bits : 12_8
			unsigned                                    : 3;
			unsigned pre_bnr_codec_dct_qtbl6_idx        : 5;		// bits : 20_16
			unsigned                                    : 3;
			unsigned pre_bnr_codec_dct_qtbl7_idx        : 5;		// bits : 28_24
		} bit;
		UINT32 word;
	} pre_register_931; // 0x0e8c

	union
	{
		struct
		{
			unsigned pre_bnr_enc_dct_level_th0        : 8;		// bits : 7_0
			unsigned pre_bnr_enc_dct_level_th1        : 8;		// bits : 15_8
			unsigned pre_bnr_enc_dct_level_th2        : 8;		// bits : 23_16
			unsigned pre_bnr_enc_dct_level_th3        : 8;		// bits : 31_24
		} bit;
		UINT32 word;
	} pre_register_932; // 0x0e90

	union
	{
		struct
		{
			unsigned pre_bnr_np_coef_a        : 16;		// bits : 15_0
			unsigned pre_bnr_np_coef_b        : 16;		// bits : 31_16
		} bit;
		UINT32 word;
	} pre_register_933; // 0x0e94

	union
	{
		struct
		{
			unsigned pre_bnr_ps_ds_th         : 5;		// bits : 4_0
			unsigned                          : 3;
			unsigned pre_bnr_ps_ds_th2        : 5;		// bits : 12_8
		} bit;
		UINT32 word;
	} pre_register_934; // 0x0e98

	union
	{
		struct
		{

		} bit;
		UINT32 word;
	} pre_register_935; // 0x0e9c

	union
	{
		struct
		{
			unsigned                        : 2;
			unsigned pre_bnr_ref_sai        : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_936; // 0x0ea0

	union
	{
		struct
		{
			unsigned                            : 2;
			unsigned pre_bnr_ref_ofsi           : 14;		// bits : 15_2
			unsigned                            : 8;
			unsigned pre_bnr_ref_sai_msb        : 4;		// bits : 27_24
		} bit;
		UINT32 word;
	} pre_register_937; // 0x0ea4

	union
	{
		struct
		{
			unsigned                               : 2;
			unsigned pre_bnr_motion_bit_sai        : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_938; // 0x0ea8

	union
	{
		struct
		{
			unsigned                                   : 2;
			unsigned pre_bnr_motion_bit_ofsi           : 14;		// bits : 15_2
			unsigned                                   : 8;
			unsigned pre_bnr_motion_bit_sai_msb        : 4;		// bits : 27_24
		} bit;
		UINT32 word;
	} pre_register_939; // 0x0eac

	union
	{
		struct
		{
			unsigned                          : 2;
			unsigned pre_bnr_sigma_sai        : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_940; // 0x0eb0

	union
	{
		struct
		{
			unsigned                              : 2;
			unsigned pre_bnr_sigma_ofsi           : 14;		// bits : 15_2
			unsigned                              : 8;
			unsigned pre_bnr_sigma_sai_msb        : 4;		// bits : 27_24
		} bit;
		UINT32 word;
	} pre_register_941; // 0x0eb4

	union
	{
		struct
		{
			unsigned                        : 2;
			unsigned pre_bnr_ref_sao        : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_942; // 0x0eb8

	union
	{
		struct
		{
			unsigned                            : 2;
			unsigned pre_bnr_ref_ofso           : 14;		// bits : 15_2
			unsigned                            : 8;
			unsigned pre_bnr_ref_sao_msb        : 4;		// bits : 27_24
		} bit;
		UINT32 word;
	} pre_register_943; // 0x0ebc

	union
	{
		struct
		{
			unsigned                               : 2;
			unsigned pre_bnr_motion_bit_sao        : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_944; // 0x0ec0

	union
	{
		struct
		{
			unsigned                                   : 2;
			unsigned pre_bnr_motion_bit_ofso           : 14;		// bits : 15_2
			unsigned                                   : 8;
			unsigned pre_bnr_motion_bit_sao_msb        : 4;		// bits : 27_24
		} bit;
		UINT32 word;
	} pre_register_945; // 0x0ec4

	union
	{
		struct
		{
			unsigned                          : 2;
			unsigned pre_bnr_sigma_sao        : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_946; // 0x0ec8

	union
	{
		struct
		{
			unsigned                              : 2;
			unsigned pre_bnr_sigma_ofso           : 14;		// bits : 15_2
			unsigned                              : 8;
			unsigned pre_bnr_sigma_sao_msb        : 4;		// bits : 27_24
		} bit;
		UINT32 word;
	} pre_register_947; // 0x0ecc

	union
	{
		struct
		{
			unsigned                        : 2;
			unsigned pre_bnr_sta_sao        : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_948; // 0x0ed0

	union
	{
		struct
		{
			unsigned                            : 2;
			unsigned pre_bnr_sta_ofso           : 14;		// bits : 15_2
			unsigned                            : 8;
			unsigned pre_bnr_sta_sao_msb        : 4;		// bits : 27_24
		} bit;
		UINT32 word;
	} pre_register_949; // 0x0ed4

	union
	{
		struct
		{
			unsigned                          : 2;
			unsigned pre_bnr_gamma_sao        : 30;		// bits : 31_2
		} bit;
		UINT32 word;
	} pre_register_950; // 0x0ed8

	union
	{
		struct
		{
			unsigned                           : 2;
			unsigned pre_bnr_gamma_ofso        : 14;	// bits : 15_2
			unsigned                           : 8;
			unsigned pre_bnr_gamma_sao_msb     : 4;		// bits : 27_24
		} bit;
		UINT32 word;
	} pre_register_951; // 0x0edc



} NT98539A_PRE_REGISTER_STRUCT;



/*
#define PRE_REG_ADDR(ofs)        (_PRE_REG_BASE_ADDR+(ofs))
#define PRE_SETREG(ofs, value)   OUTW((_PRE_REG_BASE_ADDR + ofs), value)
#define PRE_GETREG(ofs)          INW(_PRE_REG_BASE_ADDR + ofs)
*/
	/*
		pre_sw_rst		 :	  [0x0, 0x1],			bits : 0
		pre_start		 :	  [0x0, 0x1],			bits : 1
		pre_load_start	 :	  [0x0, 0x1],			bits : 2
		pre_load_fd 	 :	  [0x0, 0x1],			bits : 3
		pre_load_frmstart:	  [0x0, 0x1],			bits : 4
		pre_ll_fire 	 :	  [0x0, 0x1],			bits : 28
		pre_ll_terminate :	  [0x0, 0x1],			bits : 29
	*/
#define PRE_OPERATION_CONTROL_REGISTER_OFS 0x0000
	REGDEF_BEGIN(PRE_OPERATION_CONTROL_REGISTER)
	REGDEF_BIT(pre_sw_rst,		  1)
	REGDEF_BIT(pre_start,		 1)
	REGDEF_BIT(pre_load_start,		  1)
	REGDEF_BIT(pre_load_fd, 	   1)
	REGDEF_BIT(pre_load_frmstart,		 1)
	REGDEF_BIT(pre_global_load_en,		  1)
	REGDEF_BIT(,		22)
	REGDEF_BIT(pre_ll_fire, 	   1)
	REGDEF_END(PRE_OPERATION_CONTROL_REGISTER)

#define PRE_DMA_DISABLE_REGISTER_OFS 0x001c
	REGDEF_BEGIN(PRE_DMA_DISABLE_REGISTER)
	REGDEF_BIT(pre_dmach_idle,		  1)
	REGDEF_BIT(pre_dmach_dis,		  1)
	REGDEF_BIT(pre_dma_output_en,		 1)
	REGDEF_BIT(,		  5)
	REGDEF_BIT(pre_dich_line_buf_ctrl,		  2)
	REGDEF_BIT(,		  6)
	REGDEF_BIT(pre_dma_sync_dich_line_buf_disable,		  1)
	REGDEF_BIT(pre_dma1_wait_sie2_start_disable,		  1)
	REGDEF_END(PRE_DMA_DISABLE_REGISTER)

#define PRE_INTERRUPT_ENABLE_REGISTER_OFS 0x0008
	REGDEF_BEGIN(FILTER_INTERRUPT_ENABLE_REGISTER)
	REGDEF_BIT(inte_frmend, 			1)
	REGDEF_BIT(inte_pre_r_dec_err1,    1)
	REGDEF_BIT(inte_pre_r_dec_err2,    1)
	REGDEF_BIT(inte_llend,				1)
	REGDEF_BIT(inte_llerror,			1)
	REGDEF_BIT(inte_llerror2,			1)
	REGDEF_BIT(inte_lljobend,			1)
	REGDEF_BIT(inte_pre_bufovfl,		1)
	REGDEF_BIT(inte_pre_ringbuf_err1,		 1)
	REGDEF_BIT(inte_pre_frame_err,	 1)
	REGDEF_BIT(                  ,      2)
	REGDEF_BIT(inte_sie_frame_start,	1)
	REGDEF_BIT(inte_sie2_frame_start,   1)
	REGDEF_BIT(inte_pre_frame_start,	1)
	REGDEF_BIT(                                  ,        5)
	REGDEF_BIT(inte_pre_bnr_enc_ovfl,        1)
	REGDEF_BIT(inte_pre_bnr_dec_err,        1)
	REGDEF_END(PRE_INTERRUPT_ENABLE_REGISTER)

#define PRE_INTERRUPT_STATUS_REGISTER_OFS 0x000c
	REGDEF_BEGIN(FILTER_INTERRUPT_STATUS_REGISTER)
	REGDEF_BIT(int_frmend,			1)
	REGDEF_BIT(int_pre_r_dec_err1, 1)
	REGDEF_BIT(int_pre_r_dec_err2, 1)
	REGDEF_BIT(int_llend,				1)
	REGDEF_BIT(int_llerror, 		1)
	REGDEF_BIT(int_llerror2,			1)
	REGDEF_BIT(int_lljobend,			1)
	REGDEF_BIT(int_pre_bufovfl, 	1)
	REGDEF_BIT(int_pre_ringbuf_err1,		1)
	REGDEF_BIT(int_pre_frame_err,	1)
	REGDEF_BIT(                    ,    2)
	REGDEF_BIT(int_sie_frame_start, 1)
	REGDEF_BIT(int_sie2_frame_start,   1)
	REGDEF_BIT(int_pre_frame_start, 1)
	REGDEF_BIT(                                  ,        5)
	REGDEF_BIT(int_pre_bnr_enc_ovfl,        1)
	REGDEF_BIT(int_pre_bnr_dec_err,        1)
	REGDEF_END(PRE_INTERRUPT_STATUS_REGISTER)

#define PRE_DRAM_SETTINGS_OFS 0x005c
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
	REGDEF_END(PRE_DRAM_SETTINGS)

#define PRE_AXI_REGISTER_0_OFS 0x03f0
	REGDEF_BEGIN(PRE_AXI_REGISTER_0)
		REGDEF_BIT(pre_axi_disable	 ,		  1)
		REGDEF_BIT(pre_axi_ch_idle	 ,		  1)
		REGDEF_BIT(pre_axi_channel_en,		  15)
		REGDEF_BIT(pre_axi_lock_dis  ,		  15)
	REGDEF_END(PRE_AXI_REGISTER_0)

#define PRE_SETTING_REGISTER_OFS 0x0018
	REGDEF_BEGIN(PRE_SETTING_REGISTER)
	REGDEF_BIT(cfapat_2      ,        3)
	REGDEF_BIT(              ,        5)
	REGDEF_BIT(pre_hdr_sram_shutdown_en ,      1)
	REGDEF_BIT(pre_va_sram_shutdown_en  ,      1)
	REGDEF_BIT(pre_bnr_sram_shutdown_en ,      1)
	REGDEF_BIT(pre_outl_sram_shutdown_en,      1)
	REGDEF_BIT(              ,        12)
	REGDEF_BIT(dram_saill_msb,		  4)
	REGDEF_BIT(pre_flip0_en,		  1)
	REGDEF_BIT(pre_flip1_en,		  1)
	REGDEF_END(PRE_SETTING_REGISTER)

#define PRE_SOURCE_ADDRESS_REGISTER_3_OFS 0x0014
	REGDEF_BEGIN(PRE_SOURCE_ADDRESS_REGISTER_3)
	REGDEF_BIT(,		2)
	REGDEF_BIT(dram_saill,		  30)
	REGDEF_END(PRE_SOURCE_ADDRESS_REGISTER_3)





#if 1
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
		REGDEF_BIT(va_in_sel		       ,		2)
		REGDEF_BIT(va_vertical_fir_en      ,		1)
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


#endif

#if 0
	/*
		PRE_MODE		   :	[0x0, 0x3], 		bits : 1_0
		INBIT_16_FMT_SEL   :	[0x0, 0x1], 		bits : 4
		INBIT_DEPTH 	   :	[0x0, 0x3], 		bits : 6_5
		OUTBIT_DEPTH	   :	[0x0, 0x3], 		bits : 8_7
		CFAPAT			   :	[0x0, 0x7], 		bits : 11_9
		PRE_OUTL_EN 	   :	[0x0, 0x1], 		bits : 13
		PRE_FILTER_EN	   :	[0x0, 0x1], 		bits : 14
		PRE_CGAIN_EN	   :	[0x0, 0x1], 		bits : 15
		PRE_VIG_EN		   :	[0x0, 0x1], 		bits : 16
		PRE_GBAL_EN 	   :	[0x0, 0x1], 		bits : 17
		PRE_BINNING 	   :	[0x0, 0x7], 		bits : 20_18
		BAYER_FORMAT	   :	[0x0, 0x1], 		bits : 21
		PRE_RGBIR_RB_NRFILL:	[0x0, 0x1], 		bits : 22
		PRE_BILAT_TH_EN    :	[0x0, 0x1], 		bits : 23
		PRE_DGAIN_EN	   :	[0x0, 0x1], 		bits : 24
		PRE_F_CG_EN 	   :	[0x0, 0x1], 		bits : 25
		PRE_F_FUSION_EN    :	[0x0, 0x1], 		bits : 26
		PRE_F_FUSION_FNUM  :	[0x0, 0x3], 		bits : 28_27
		PRE_F_FC_EN 	   :	[0x0, 0x1], 		bits : 29
		MIRROR_EN		   :	[0x0, 0x1], 		bits : 30
		PRE_R_DECODE_EN    :	[0x0, 0x1], 		bits : 31
	*/
#define CONTROL_REGISTER_OFS 0x0004
	REGDEF_BEGIN(CONTROL_REGISTER)
	REGDEF_BIT(pre_mode,		2)
	REGDEF_BIT(pre_input_format   , 	   1)
	REGDEF_BIT(pre_subout_sel	  , 	   1)
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
	REGDEF_BIT(pre_thermal_mode, 	   1)
	REGDEF_END(CONTROL_REGISTER)


	/*
		inte_frmend 		 :	  [0x0, 0x1],			bits : 0
		inte_pre_r_dec_err1  :	  [0x0, 0x1],			bits : 1
		inte_pre_r_dec_err2  :	  [0x0, 0x1],			bits : 2
		inte_llend			 :	  [0x0, 0x1],			bits : 3
		inte_llerror		 :	  [0x0, 0x1],			bits : 4
		inte_llerror2		 :	  [0x0, 0x1],			bits : 5
		inte_lljobend		 :	  [0x0, 0x1],			bits : 6
		inte_pre_bufovfl	 :	  [0x0, 0x1],			bits : 7
		inte_pre_ringbuf_err1:	  [0x0, 0x1],			bits : 8
		inte_pre_frame_err	 :	  [0x0, 0x1],			bits : 9
		inte_sie_frame_start :	  [0x0, 0x1],			bits : 12
		inte_sie2_frame_start:    [0x0, 0x1],			bits : 13
		inte_pre_frame_start :	  [0x0, 0x1],			bits : 14
	    inte_pre_nn_isp_p0_set_slice_ready:    [0x0, 0x1],			bits : 16
	    inte_pre_nn_isp_p0_get_slice_clear:    [0x0, 0x1],			bits : 17
	    inte_pre_nn_isp_p1_set_slice_ready:    [0x0, 0x1],			bits : 18
	    inte_pre_nn_isp_p1_get_slice_clear:    [0x0, 0x1],			bits : 19
	*/



	/*
		int_frmend			:	 [0x0, 0x1],			bits : 0
		int_pre_r_dec_err1	:	 [0x0, 0x1],			bits : 1
		int_pre_r_dec_err2	:	 [0x0, 0x1],			bits : 2
		int_llend			:	 [0x0, 0x1],			bits : 3
		int_llerror 		:	 [0x0, 0x1],			bits : 4
		int_llerror2		:	 [0x0, 0x1],			bits : 5
		int_lljobend		:	 [0x0, 0x1],			bits : 6
		int_pre_bufovfl 	:	 [0x0, 0x1],			bits : 7
		int_pre_ringbuf_err1:	 [0x0, 0x1],			bits : 8
		int_pre_frame_err	:	 [0x0, 0x1],			bits : 9
		int_pre_r_dec_err3	:	 [0x0, 0x1],			bits : 10
		int_pre_ringbuf_err2:	 [0x0, 0x1],			bits : 11
		int_sie_frame_start:	 [0x0, 0x1],			bits : 12
		int_sie2_frame_start:    [0x0, 0x1],			bits : 13
		int_pre_frame_start:	 [0x0, 0x1],			bits : 14
	    int_pre_nn_isp_p0_set_slice_ready:    [0x0, 0x1],			bits : 16
	    int_pre_nn_isp_p0_get_slice_clear:    [0x0, 0x1],			bits : 17
	    int_pre_nn_isp_p1_set_slice_ready:    [0x0, 0x1],			bits : 18
	    int_pre_nn_isp_p1_get_slice_clear:    [0x0, 0x1],			bits : 19
	*/



	/*
		pre_busy	   :	[0x0, 0x1], 		bits : 0
		reserved	   :	[0x0, 0xf], 		bits : 7_4
		pre_checksum_en:	[0x0, 0x1], 		bits : 21
		sram_ls_en	   :	[0x0, 0x3ff],		bits : 31_22
	*/
#define DEBUG_REGISTER_OFS 0x0010
	REGDEF_BEGIN(DEBUG_REGISTER)
	REGDEF_BIT(pre_busy,		1)
	REGDEF_BIT(,	   19)
	REGDEF_BIT(pre_frmstr_rst,	1)
	REGDEF_BIT(pre_checksum_en, 	   1)
	REGDEF_BIT(sram_ls_en,		  10)
	REGDEF_END(DEBUG_REGISTER)


	/*
		dram_saill:    [0x0, 0x3fffffff],			bits : 31_2
	*/


	/*
		cfapat_2       :    [0x0, 0x7],			bits : 2_0
		dram_saill_msb:    [0x0, 0x3fffffff],			bits : 27_24
	*/



	/*
		pre_dmach_idle :	[0x0, 0x1], 		 bits : 0
		pre_dmach_dis :    [0x0, 0x1],			 bits : 1
	*/


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
		pre_ll_terminate:	 [0x0, 0x1],			bits : 0
	*/
#define LINK_LIST_CONTROL_REGISTER_OFS 0x002C
	REGDEF_BEGIN(LINK_LIST_CONTROL_REGISTER)
	REGDEF_BIT(pre_ll_terminate,		1)
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
		pre_yuv_in_fmt_(0~2) :	  [0x0, 0x3],			bits : 1_0
		pre_yuv2rgb_en		 :	  [0x0, 0x1],			bits : 4
		pre_yuv2rgb_fmt_(0~2):	  [0x0, 0x3],			bits : 6_5
	*/
#define YUV_CONTROL_REGISTER_OFS 0x0058
	REGDEF_BEGIN(YUV_CONTROL_REGISTER)
		REGDEF_BIT(pre_yuv_in_fmt , 	   2)
		REGDEF_BIT( 					,		 2)
		REGDEF_BIT(pre_yuv2rgb_en		,		 1)
		REGDEF_BIT(pre_yuv2rgb_fmt, 	   2)
	REGDEF_END(YUV_CONTROL_REGISTER)


	/*
		input_burst_mode :	  [0x0, 0x1],			bits : 0
		output_burst_mode:	  [0x0, 0x1],			bits : 4
		dmaloop_line	 :	  [0x0, 0x7ff], 		bits : 22_12
		dmaloop_en		 :	  [0x0, 0x1],			bits : 24
		dmaloop_ctrl	 :	  [0x0, 0x1],			bits : 25
	*/



	/*
		pre_dram_out0_single_en:	[0x0, 0x1], 		bits : 0
		pre_dram_out1_single_en:	[0x0, 0x1], 		bits : 1
		pre_dram_out2_single_en:	[0x0, 0x1], 		bits : 2
    	pre_hist_out_single_en    :    [0x0, 0x1],			bits : 3
    	pre_indep_va_out_single_en:    [0x0, 0x1],			bits : 4
		pre_dram_out_mode	   :	[0x0, 0x1], 		bits : 31
	*/
#define PRE_DMA_OUTPUT_CHANNEL_ENABLE_REGISTER_OFS 0x0060
	REGDEF_BEGIN(PRE_DMA_OUTPUT_CHANNEL_ENABLE_REGISTER)
		REGDEF_BIT(pre_dram_out0_single_en, 	   1)
		REGDEF_BIT(pre_dram_out1_single_en, 	   1)
		REGDEF_BIT(pre_dram_out2_single_en, 	   1)
		REGDEF_BIT(pre_hist_out_single_en    ,     1)
		REGDEF_BIT(pre_indep_va_out_single_en,     1)
		REGDEF_BIT(                          ,    26)
		REGDEF_BIT(pre_dram_out_mode	  , 	   1)
	REGDEF_END(PRE_DMA_OUTPUT_CHANNEL_ENABLE_REGISTER)


	/*
		pre_dgain:	  [0x0, 0xffff],			bits : 15_0
	*/
#define DIGITAL_GAIN_REGISTER_OFS 0x006c
	REGDEF_BEGIN(DIGITAL_GAIN_REGISTER)
	REGDEF_BIT(pre_dgain,		 16)
	REGDEF_END(DIGITAL_GAIN_REGISTER)

	/*
		cgain_inv		 :	  [0x0, 0x1],			bits : 0
		cgain_hinv		 :	  [0x0, 0x1],			bits : 1
		cgain_range 	 :	  [0x0, 0x1],			bits : 2
		pre_f_cgain_range:	  [0x0, 0x1],			bits : 3
		cgain_mask		 :	  [0x0, 0xfff], 		bits : 19_8
	*/
#define PRE_COLOR_GAIN_REGISTER_0_OFS 0x0070
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_0)
	REGDEF_BIT(cgain_inv,		 1)
	REGDEF_BIT(cgain_hinv,		  1)
	REGDEF_BIT(cgain_range, 	   1)
	REGDEF_BIT(pre_f_cgain_range,		 1)
	REGDEF_BIT(,		4)
	REGDEF_BIT(cgain_mask,		  12)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_0)


	/*
		pre_cgain_r :	 [0x0, 0x3ff],			bits : 9_0
		pre_cgain_gr:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define PRE_COLOR_GAIN_REGISTER_1_OFS 0x0074
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_1)
	REGDEF_BIT(pre_cgain_r, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_cgain_gr,		10)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_1)


	/*
		pre_cgain_gb:	 [0x0, 0x3ff],			bits : 9_0
		pre_cgain_b :	 [0x0, 0x3ff],			bits : 25_16
	*/
#define PRE_COLOR_GAIN_REGISTER_2_OFS 0x0078
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_2)
	REGDEF_BIT(pre_cgain_gb,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_cgain_b, 	   10)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_2)


	/*
		pre_cgain_ir:	 [0x0, 0x3ff],			bits : 9_0
	*/
#define PRE_COLOR_GAIN_REGISTER_3_OFS 0x007c
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_3)
	REGDEF_BIT(pre_cgain_ir,		10)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_3)


	/*
		pre_f_p0_cgain_r :	  [0x0, 0xffff], 		bits : 15_0
		pre_f_p0_cgain_gr:	  [0x0, 0xffff], 		bits : 25_16
	*/
#define PRE_COLOR_GAIN_REGISTER_4_OFS 0x0080
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_4)
	REGDEF_BIT(pre_f_p0_cgain_r,		16)
	REGDEF_BIT(pre_f_p0_cgain_gr,		 16)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_4)


	/*
		pre_f_p0_cgain_gb:	  [0x0, 0xffff], 		bits : 15_0
		pre_f_p0_cgain_b :	  [0x0, 0xffff], 		bits : 25_16
	*/
#define PRE_COLOR_GAIN_REGISTER_5_OFS 0x0084
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_5)
	REGDEF_BIT(pre_f_p0_cgain_gb,		 16)
	REGDEF_BIT(pre_f_p0_cgain_b,		16)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_5)


	/*
		pre_f_p0_cgain_ir:	  [0x0, 0xffff], 		bits : 15_0
	*/
#define PRE_COLOR_GAIN_REGISTER_6_OFS 0x0088
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_6)
	REGDEF_BIT(pre_f_p0_cgain_ir,		 16)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_6)


	/*
		pre_f_p1_cgain_r :	  [0x0, 0xffff], 		bits : 15_0
		pre_f_p1_cgain_gr:	  [0x0, 0xffff], 		bits : 25_16
	*/
#define PRE_COLOR_GAIN_REGISTER_7_OFS 0x008c
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_7)
	REGDEF_BIT(pre_f_p1_cgain_r,		16)
	REGDEF_BIT(pre_f_p1_cgain_gr,		 16)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_7)


	/*
		pre_f_p1_cgain_gb:	  [0x0, 0xffff], 		bits : 15_0
		pre_f_p1_cgain_b :	  [0x0, 0xffff], 		bits : 25_16
	*/
#define PRE_COLOR_GAIN_REGISTER_8_OFS 0x0090
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_8)
	REGDEF_BIT(pre_f_p1_cgain_gb,		 16)
	REGDEF_BIT(pre_f_p1_cgain_b,		16)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_8)


	/*
		pre_f_p1_cgain_ir:	  [0x0, 0xffff], 		bits : 15_0
	*/
#define PRE_COLOR_GAIN_REGISTER_9_OFS 0x0094
	REGDEF_BEGIN(PRE_COLOR_GAIN_REGISTER_9)
	REGDEF_BIT(pre_f_p1_cgain_ir,		 16)
	REGDEF_END(PRE_COLOR_GAIN_REGISTER_9)


	/*
		pre_line_count :	[0x0, 0xffff],			  bits : 15_0
		sie2_line_count:	[0x0, 0xffff],			  bits : 31_16
	*/
#define DMALOOP_ERROR_LINE_COUNT_REGISTER_OFS 0x00A8
	REGDEF_BEGIN(DMALOOP_ERROR_LINE_COUNT_REGISTER)
	REGDEF_BIT(pre_line_count,		  16)
	REGDEF_BIT(sie2_line_count, 	  16)
	REGDEF_END(DMALOOP_ERROR_LINE_COUNT_REGISTER)


	/*
		pre_cofs_r :	[0x0, 0x3ff],			bits : 9_0
		pre_cofs_gr:	[0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_0_OFS 0x00b0
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_0)
	REGDEF_BIT(pre_cofs_r,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_cofs_gr, 	   10)
	REGDEF_END(COLOR_OFFSET_REGISTER_0)


	/*
		pre_cofs_gb:	[0x0, 0x3ff],			bits : 9_0
		pre_cofs_b :	[0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_1_OFS 0x00b4
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_1)
	REGDEF_BIT(pre_cofs_gb, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_cofs_b,		  10)
	REGDEF_END(COLOR_OFFSET_REGISTER_1)


	/*
		pre_cofs_ir:	[0x0, 0x3ff],			bits : 9_0
	*/
#define COLOR_OFFSET_REGISTER_2_OFS 0x00b8
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_2)
	REGDEF_BIT(pre_cofs_ir, 	   10)
	REGDEF_END(COLOR_OFFSET_REGISTER_2)


	/*
		pre_f_p0_cofs_r :	 [0x0, 0x3ff],			bits : 9_0
		pre_f_p0_cofs_gr:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_3_OFS 0x00bc
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_3)
	REGDEF_BIT(pre_f_p0_cofs_r, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_f_p0_cofs_gr,		10)
	REGDEF_END(COLOR_OFFSET_REGISTER_3)


	/*
		pre_f_p0_cofs_gb:	 [0x0, 0x3ff],			bits : 9_0
		pre_f_p0_cofs_b :	 [0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_4_OFS 0x00c0
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_4)
	REGDEF_BIT(pre_f_p0_cofs_gb,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_f_p0_cofs_b, 	   10)
	REGDEF_END(COLOR_OFFSET_REGISTER_4)


	/*
		pre_f_p0_cofs_ir:	 [0x0, 0x3ff],			bits : 9_0
	*/
#define COLOR_OFFSET_REGISTER_5_OFS 0x00c4
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_5)
	REGDEF_BIT(pre_f_p0_cofs_ir,		10)
	REGDEF_END(COLOR_OFFSET_REGISTER_5)


	/*
		pre_f_p1_cofs_r :	 [0x0, 0x3ff],			bits : 9_0
		pre_f_p1_cofs_gr:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_6_OFS 0x00c8
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_6)
	REGDEF_BIT(pre_f_p1_cofs_r, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_f_p1_cofs_gr,		10)
	REGDEF_END(COLOR_OFFSET_REGISTER_6)


	/*
		pre_f_p1_cofs_gb:	 [0x0, 0x3ff],			bits : 9_0
		pre_f_p1_cofs_b :	 [0x0, 0x3ff],			bits : 25_16
	*/
#define COLOR_OFFSET_REGISTER_7_OFS 0x00cc
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_7)
	REGDEF_BIT(pre_f_p1_cofs_gb,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_f_p1_cofs_b, 	   10)
	REGDEF_END(COLOR_OFFSET_REGISTER_7)


	/*
		pre_f_p1_cofs_ir:	 [0x0, 0x3ff],			bits : 9_0
	*/
#define COLOR_OFFSET_REGISTER_8_OFS 0x00d0
	REGDEF_BEGIN(COLOR_OFFSET_REGISTER_8)
	REGDEF_BIT(pre_f_p1_cofs_ir,		10)
	REGDEF_END(COLOR_OFFSET_REGISTER_8)


	/*
		pre_outl_rgbir_rb_w:	[0x0, 0xff],			bits : 7_0
		pre_ord_rgbir_rb_w :	[0x0, 0xff],			bits : 15_8
	*/
#define OUTLIER_REGISTER_OFS 0x00f0
	REGDEF_BEGIN(OUTLIER_REGISTER)
	REGDEF_BIT(pre_outl_rgbir_rb_w, 	   8)
	REGDEF_BIT(pre_ord_rgbir_rb_w,		  8)
	REGDEF_END(OUTLIER_REGISTER)


	/*
		pre_gbal_str_luma_low_bnd :    [0x0, 0xfff],			bits : 11_0
		pre_gbal_edge_luma_low_bnd:    [0x0, 0xfff],			bits : 27_16
	*/
#define GBALANCE_REGISTER_OFS 0x00f4
	REGDEF_BEGIN(GBALANCE_REGISTER)
	REGDEF_BIT(pre_gbal_str_luma_low_bnd,		 12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(pre_gbal_edge_luma_low_bnd,		  12)
	REGDEF_END(GBALANCE_REGISTER)



	/*
		pre_clamp_th :	  [0x0, 0xfff], 		bits : 11_0
		pre_clamp_mul:	  [0x0, 0xff],			bits : 19_12
		pre_clamp_dlt:	  [0x0, 0xfff], 		bits : 31_20
	*/
#define OUTPUT_FILTER_REGISTER_OFS 0x0100
	REGDEF_BEGIN(OUTPUT_FILTER_REGISTER)
	REGDEF_BIT(pre_clamp_th,		12)
	REGDEF_BIT(pre_clamp_mul,		 8)
	REGDEF_BIT(pre_clamp_dlt,		 12)
	REGDEF_END(OUTPUT_FILTER_REGISTER)


	/*
		pre_bilat_w  :	  [0x0, 0xf],			bits : 3_0
		pre_rth_w	 :	  [0x0, 0xf],			bits : 7_4
		pre_bilat_th1:	  [0x0, 0x3ff], 		bits : 17_8
		pre_bilat_th2:	  [0x0, 0x3ff], 		bits : 27_18
		pre_bilat_cen_sel:[0x0, 0x1],			bits : 28
	*/
#define RANGE_FILTER_REGISTER_0_OFS 0x0104
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_0)
	REGDEF_BIT(pre_bilat_w, 	   4)
	REGDEF_BIT(pre_rth_w,		 4)
	REGDEF_BIT(pre_bilat_th1,		 10)
	REGDEF_BIT(pre_bilat_th2,		 10)
	REGDEF_BIT(pre_bilat_cen_sel,	 1)
	REGDEF_END(RANGE_FILTER_REGISTER_0)


	/*
		PRE_S_WEIGHT0:	  [0x0, 0x1f],			bits : 4_0
		PRE_S_WEIGHT1:	  [0x0, 0x1f],			bits : 12_8
		PRE_S_WEIGHT2:	  [0x0, 0x1f],			bits : 20_16
		PRE_S_WEIGHT3:	  [0x0, 0x1f],			bits : 28_24
	*/
#define SPATIAL_FILTER_REGISTER_1_OFS 0x0108
	REGDEF_BEGIN(SPATIAL_FILTER_REGISTER_1)
	REGDEF_BIT(pre_spatial_weight0, 	   5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_spatial_weight1, 	   5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_spatial_weight2, 	   5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_spatial_weight3, 	   5)
	REGDEF_END(SPATIAL_FILTER_REGISTER_1)


	/*
		PRE_S_WEIGHT4:	  [0x0, 0x1f],			bits : 4_0
		PRE_S_WEIGHT5:	  [0x0, 0x1f],			bits : 12_8
	*/
#define SPATIAL_FILTER_REGISTER_2_OFS 0x010c
	REGDEF_BEGIN(SPATIAL_FILTER_REGISTER_2)
	REGDEF_BIT(pre_spatial_weight4, 	   5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_spatial_weight5, 	   5)
	REGDEF_END(SPATIAL_FILTER_REGISTER_2)


	/*
		pre_rth_nlm_c0_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c0_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_1_OFS 0x0110
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_1)
	REGDEF_BIT(pre_rth_nlm_c0_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_1)


	/*
		pre_rth_nlm_c0_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c0_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_2_OFS 0x0114
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_2)
	REGDEF_BIT(pre_rth_nlm_c0_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_2)


	/*
		pre_rth_nlm_c0_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c0_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_3_OFS 0x0118
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_3)
	REGDEF_BIT(pre_rth_nlm_c0_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_3)


	/*
		pre_rth_nlm_c1_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c1_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_5_OFS 0x0120
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_5)
	REGDEF_BIT(pre_rth_nlm_c1_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_5)


	/*
		pre_rth_nlm_c1_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c1_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_6_OFS 0x0124
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_6)
	REGDEF_BIT(pre_rth_nlm_c1_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_6)


	/*
		pre_rth_nlm_c1_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c1_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_7_OFS 0x0128
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_7)
	REGDEF_BIT(pre_rth_nlm_c1_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_7)


	/*
		pre_rth_nlm_c2_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c2_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_9_OFS 0x0130
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_9)
	REGDEF_BIT(pre_rth_nlm_c2_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_9)


	/*
		pre_rth_nlm_c2_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c2_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_10_OFS 0x0134
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_10)
	REGDEF_BIT(pre_rth_nlm_c2_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_10)


	/*
		pre_rth_nlm_c2_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c2_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_11_OFS 0x0138
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_11)
	REGDEF_BIT(pre_rth_nlm_c2_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_11)

	/*
		pre_rth_nlm_c3_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c3_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_13_OFS 0x0140
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_13)
	REGDEF_BIT(pre_rth_nlm_c3_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_13)


	/*
		pre_rth_nlm_c3_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c3_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_14_OFS 0x0144
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_14)
	REGDEF_BIT(pre_rth_nlm_c3_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_14)


	/*
		pre_rth_nlm_c3_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c3_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_15_OFS 0x0148
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_15)
	REGDEF_BIT(pre_rth_nlm_c3_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_15)


	/*
		reserved:	 [0x0, 0xffffffff], 		bits : 31_0
	*/
#define RANGE_FILTER_REGISTER_16_OFS 0x014c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_16)
	REGDEF_BIT(reserved,		32)
	REGDEF_END(RANGE_FILTER_REGISTER_16)


	/*
		pre_rth_bilat_c0_0:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_17_OFS 0x0150
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_17)
	REGDEF_BIT(pre_rth_bilat_c0_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_17)


	/*
		pre_rth_bilat_c0_2:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_18_OFS 0x0154
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_18)
	REGDEF_BIT(pre_rth_bilat_c0_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_18)


	/*
		pre_rth_bilat_c0_4:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_19_OFS 0x0158
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_19)
	REGDEF_BIT(pre_rth_bilat_c0_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_19)


	/*
		reserved:	 [0x0, 0xffffffff], 		bits : 31_0
	*/
#define RANGE_FILTER_REGISTER_20_OFS 0x015c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_20)
	REGDEF_BIT(reserved,		32)
	REGDEF_END(RANGE_FILTER_REGISTER_20)


	/*
		pre_rth_bilat_c1_0:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_21_OFS 0x0160
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_21)
	REGDEF_BIT(pre_rth_bilat_c1_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_21)


	/*
		pre_rth_bilat_c1_2:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_22_OFS 0x0164
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_22)
	REGDEF_BIT(pre_rth_bilat_c1_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_22)


	/*
		pre_rth_bilat_c1_4:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_23_OFS 0x0168
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_23)
	REGDEF_BIT(pre_rth_bilat_c1_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_23)


	/*
		pre_rth_bilat_c2_0:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_25_OFS 0x0170
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_25)
	REGDEF_BIT(pre_rth_bilat_c2_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_25)


	/*
		pre_rth_bilat_c2_2:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_26_OFS 0x0174
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_26)
	REGDEF_BIT(pre_rth_bilat_c2_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_26)


	/*
		pre_rth_bilat_c2_4:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_27_OFS 0x0178
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_27)
	REGDEF_BIT(pre_rth_bilat_c2_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_27)


	/*
		pre_rth_bilat_c3_0:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_29_OFS 0x0180
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_29)
	REGDEF_BIT(pre_rth_bilat_c3_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_29)


	/*
		pre_rth_bilat_c3_2:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_30_OFS 0x0184
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_30)
	REGDEF_BIT(pre_rth_bilat_c3_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_30)


	/*
		pre_rth_bilat_c3_4:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_31_OFS 0x0188
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_31)
	REGDEF_BIT(pre_rth_bilat_c3_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_31)


	/*
		pre_outl_bright_ofs:	[0x0, 0xfff],			bits : 11_0
		pre_outl_dark_ofs  :	[0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_0_OFS 0x018c
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_0)
	REGDEF_BIT(pre_outl_bright_ofs, 	   12)
	REGDEF_BIT(pre_outl_dark_ofs,		 12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_0)


	/*
		pre_outlth_bri0 :	 [0x0, 0xfff],			bits : 11_0
		pre_outlth_dark0:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_1_OFS 0x0190
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_1)
	REGDEF_BIT(pre_outlth_bri0, 	   12)
	REGDEF_BIT(pre_outlth_dark0,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_1)


	/*
		pre_outlth_bri1 :	 [0x0, 0xfff],			bits : 11_0
		pre_outlth_dark1:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_2_OFS 0x0194
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_2)
	REGDEF_BIT(pre_outlth_bri1, 	   12)
	REGDEF_BIT(pre_outlth_dark1,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_2)


	/*
		pre_outlth_bri2 :	 [0x0, 0xfff],			bits : 11_0
		pre_outlth_dark2:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_3_OFS 0x0198
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_3)
	REGDEF_BIT(pre_outlth_bri2, 	   12)
	REGDEF_BIT(pre_outlth_dark2,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_3)


	/*
		pre_outlth_bri3 :	 [0x0, 0xfff],			bits : 11_0
		pre_outlth_dark3:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_4_OFS 0x019c
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_4)
	REGDEF_BIT(pre_outlth_bri3, 	   12)
	REGDEF_BIT(pre_outlth_dark3,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_4)


	/*
		pre_outlth_bri4 :	 [0x0, 0xfff],			bits : 11_0
		pre_outlth_dark4:	 [0x0, 0xfff],			bits : 23_12
	*/
#define OUTLIER_THRESHOLD_REGISTER_5_OFS 0x01a0
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_5)
	REGDEF_BIT(pre_outlth_bri4, 	   12)
	REGDEF_BIT(pre_outlth_dark4,		12)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_5)


	/*
		pre_outl_compensate_mode :	  [0x0, 0x3],			bits : 1_0
		pre_outl_weight  :	  [0x0, 0xff],			bits : 15_8
		pre_outl_cnt1	 :	  [0x0, 0x1f],			bits : 20_16
		pre_outl_cnt2	 :	  [0x0, 0x1f],			bits : 28_24
	*/
#define OUTLIER_THRESHOLD_REGISTER_6_OFS 0x01a4
	REGDEF_BEGIN(OUTLIER_THRESHOLD_REGISTER_6)
	REGDEF_BIT(pre_outl_compensate_mode ,		 2)
	REGDEF_BIT( 						,		 7)
	REGDEF_BIT(pre_outl_weight, 	   8)
	REGDEF_BIT(pre_outl_cnt1,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_outl_cnt2,		 5)
	REGDEF_END(OUTLIER_THRESHOLD_REGISTER_6)


	/*
		PRE_ORD_RANGE_BRI :    [0x0, 0x7],			bits : 2_0
		pre_ord_range_dark:    [0x0, 0x7],			bits : 6_4
		pre_ord_protect_th:    [0x0, 0x3ff],			bits : 17_8
		PRE_ORD_BLEND_W   :    [0x0, 0xff], 		bits : 27_20
	*/
#define OUTLIER_ORDER_REGISTER_0_OFS 0x01a8
	REGDEF_BEGIN(OUTLIER_ORDER_REGISTER_0)
	REGDEF_BIT(pre_ord_range_bright,	 3)
	REGDEF_BIT(,		1)
	REGDEF_BIT(pre_ord_range_dark,		  3)
	REGDEF_BIT(,		1)
	REGDEF_BIT(pre_ord_protect_th,		  10)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_ord_blend_weight,	  8)
	REGDEF_END(OUTLIER_ORDER_REGISTER_0)


	/*
		pre_ord_bright_weight_lut0 (0~8):	 [0x0, 0xf],			bits : 3_0
		pre_ord_bright_weight_lut1 (0~8):	 [0x0, 0xf],			bits : 7_4
		pre_ord_bright_weight_lut2 (0~8):	 [0x0, 0xf],			bits : 11_8
		pre_ord_bright_weight_lut3 (0~8):	 [0x0, 0xf],			bits : 15_12
		pre_ord_bright_weight_lut4 (0~8):	 [0x0, 0xf],			bits : 19_16
		pre_ord_bright_weight_lut5 (0~8):	 [0x0, 0xf],			bits : 23_20
		pre_ord_bright_weight_lut6 (0~8):	 [0x0, 0xf],			bits : 27_24
		pre_ord_bright_weight_lut7 (0~8):	 [0x0, 0xf],			bits : 31_28
	*/
#define OUTLIER_ORDER_REGISTER_1_OFS 0x01ac
	REGDEF_BEGIN(OUTLIER_ORDER_REGISTER_1)
	REGDEF_BIT(pre_ord_bright_weight_lut0,		  4)
	REGDEF_BIT(pre_ord_bright_weight_lut1,		  4)
	REGDEF_BIT(pre_ord_bright_weight_lut2,		  4)
	REGDEF_BIT(pre_ord_bright_weight_lut3,		  4)
	REGDEF_BIT(pre_ord_bright_weight_lut4,		  4)
	REGDEF_BIT(pre_ord_bright_weight_lut5,		  4)
	REGDEF_BIT(pre_ord_bright_weight_lut6,		  4)
	REGDEF_BIT(pre_ord_bright_weight_lut7,		  4)
	REGDEF_END(OUTLIER_ORDER_REGISTER_1)


	/*
		pre_ord_dark_weight_lut0 (0~8):    [0x0, 0xf],			bits : 3_0
		pre_ord_dark_weight_lut1 (0~8):    [0x0, 0xf],			bits : 7_4
		pre_ord_dark_weight_lut2 (0~8):    [0x0, 0xf],			bits : 11_8
		pre_ord_dark_weight_lut3 (0~8):    [0x0, 0xf],			bits : 15_12
		pre_ord_dark_weight_lut4 (0~8):    [0x0, 0xf],			bits : 19_16
		pre_ord_dark_weight_lut5 (0~8):    [0x0, 0xf],			bits : 23_20
		pre_ord_dark_weight_lut6 (0~8):    [0x0, 0xf],			bits : 27_24
		pre_ord_dark_weight_lut7 (0~8):    [0x0, 0xf],			bits : 31_28
	*/
#define OUTLIER_ORDER_REGISTER_2_OFS 0x01b0
	REGDEF_BEGIN(OUTLIER_ORDER_REGISTER_2)
	REGDEF_BIT(pre_ord_dark_weight_lut0,		4)
	REGDEF_BIT(pre_ord_dark_weight_lut1,		4)
	REGDEF_BIT(pre_ord_dark_weight_lut2,		4)
	REGDEF_BIT(pre_ord_dark_weight_lut3,		4)
	REGDEF_BIT(pre_ord_dark_weight_lut4,		4)
	REGDEF_BIT(pre_ord_dark_weight_lut5,		4)
	REGDEF_BIT(pre_ord_dark_weight_lut6,		4)
	REGDEF_BIT(pre_ord_dark_weight_lut7,		4)
	REGDEF_END(OUTLIER_ORDER_REGISTER_2)


	/*
		pre_gbal_edge_protect_en:	 [0x0, 0x1],			bits : 0
		pre_gbal_diff_thr_str	:	 [0x0, 0xfff],			bits : 19_8
		pre_gbal_diff_w_max 	:	 [0x0, 0xf],			bits : 23_20
	*/
#define GBAL_REGISTER_0_OFS 0x01b4
	REGDEF_BEGIN(GBAL_REGISTER_0)
	REGDEF_BIT(pre_gbal_edge_protect_en,		1)
	REGDEF_BIT(,		7)
	REGDEF_BIT(pre_gbal_diff_thr_str,		 12)
	REGDEF_BIT(pre_gbal_diff_w_max, 	   4)
	REGDEF_END(GBAL_REGISTER_0)


	/*
		pre_gbal_edge_thr_1:	[0x0, 0xfff],			bits : 11_0
		pre_gbal_edge_thr_0:	[0x0, 0xfff],			bits : 27_16
	*/
#define GBAL_REGISTER_1_OFS 0x01b8
	REGDEF_BEGIN(GBAL_REGISTER_1)
		REGDEF_BIT(pre_gbal_edge_thr_1, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(pre_gbal_edge_thr_0, 	   12)
	REGDEF_END(GBAL_REGISTER_1)


	/*
		pre_gbal_edge_w_max:	[0x0, 0xff],			bits : 7_0
		pre_gbal_edge_w_min:	[0x0, 0xff],			bits : 15_8
	*/
#define GBAL_REGISTER_2_OFS 0x01bc
	REGDEF_BEGIN(GBAL_REGISTER_2)
	REGDEF_BIT(pre_gbal_edge_w_max, 	   8)
	REGDEF_BIT(pre_gbal_edge_w_min, 	   8)
	REGDEF_END(GBAL_REGISTER_2)


	/*
		pre_rth_nlm_c0_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c0_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_32_OFS 0x01c0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_32)
	REGDEF_BIT(pre_rth_nlm_c0_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_lut_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_32)


	/*
		pre_rth_nlm_c0_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c0_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_33_OFS 0x01c4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_33)
	REGDEF_BIT(pre_rth_nlm_c0_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_lut_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_33)


	/*
		pre_rth_nlm_c0_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c0_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_34_OFS 0x01c8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_34)
	REGDEF_BIT(pre_rth_nlm_c0_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_lut_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_34)


	/*
		pre_rth_nlm_c0_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c0_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_35_OFS 0x01cc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_35)
	REGDEF_BIT(pre_rth_nlm_c0_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_lut_7,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_35)


	/*
		pre_rth_nlm_c0_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c0_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_36_OFS 0x01d0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_36)
	REGDEF_BIT(pre_rth_nlm_c0_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_lut_9,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_36)


	/*
		pre_rth_nlm_c0_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c0_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_37_OFS 0x01d4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_37)
	REGDEF_BIT(pre_rth_nlm_c0_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_lut_11,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_37)


	/*
		pre_rth_nlm_c0_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c0_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_38_OFS 0x01d8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_38)
	REGDEF_BIT(pre_rth_nlm_c0_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_lut_13,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_38)


	/*
		pre_rth_nlm_c0_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c0_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_39_OFS 0x01dc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_39)
	REGDEF_BIT(pre_rth_nlm_c0_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c0_lut_15,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_39)


	/*
		pre_rth_nlm_c0_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_40_OFS 0x01e0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_40)
	REGDEF_BIT(pre_rth_nlm_c0_lut_16,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_40)


	/*
		pre_rth_nlm_c1_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c1_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_41_OFS 0x01e4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_41)
	REGDEF_BIT(pre_rth_nlm_c1_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_lut_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_41)


	/*
		pre_rth_nlm_c1_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c1_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_42_OFS 0x01e8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_42)
	REGDEF_BIT(pre_rth_nlm_c1_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_lut_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_42)


	/*
		pre_rth_nlm_c1_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c1_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_43_OFS 0x01ec
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_43)
	REGDEF_BIT(pre_rth_nlm_c1_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_lut_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_43)


	/*
		pre_rth_nlm_c1_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c1_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_44_OFS 0x01f0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_44)
	REGDEF_BIT(pre_rth_nlm_c1_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_lut_7,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_44)


	/*
		pre_rth_nlm_c1_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c1_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_45_OFS 0x01f4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_45)
	REGDEF_BIT(pre_rth_nlm_c1_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_lut_9,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_45)


	/*
		pre_rth_nlm_c1_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c1_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_46_OFS 0x01f8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_46)
	REGDEF_BIT(pre_rth_nlm_c1_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_lut_11,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_46)


	/*
		pre_rth_nlm_c1_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c1_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_47_OFS 0x01fc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_47)
	REGDEF_BIT(pre_rth_nlm_c1_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_lut_13,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_47)


	/*
		pre_rth_nlm_c1_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c1_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_48_OFS 0x0200
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_48)
	REGDEF_BIT(pre_rth_nlm_c1_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c1_lut_15,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_48)


	/*
		pre_rth_nlm_c1_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_49_OFS 0x0204
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_49)
	REGDEF_BIT(pre_rth_nlm_c1_lut_16,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_49)


	/*
		pre_rth_nlm_c2_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c2_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_50_OFS 0x0208
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_50)
	REGDEF_BIT(pre_rth_nlm_c2_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_lut_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_50)


	/*
		pre_rth_nlm_c2_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c2_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_51_OFS 0x020c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_51)
	REGDEF_BIT(pre_rth_nlm_c2_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_lut_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_51)


	/*
		pre_rth_nlm_c2_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c2_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_52_OFS 0x0210
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_52)
	REGDEF_BIT(pre_rth_nlm_c2_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_lut_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_52)


	/*
		pre_rth_nlm_c2_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c2_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_53_OFS 0x0214
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_53)
	REGDEF_BIT(pre_rth_nlm_c2_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_lut_7,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_53)


	/*
		pre_rth_nlm_c2_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c2_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_54_OFS 0x0218
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_54)
	REGDEF_BIT(pre_rth_nlm_c2_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_lut_9,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_54)


	/*
		pre_rth_nlm_c2_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c2_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_55_OFS 0x021c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_55)
	REGDEF_BIT(pre_rth_nlm_c2_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_lut_11,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_55)


	/*
		pre_rth_nlm_c2_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c2_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_56_OFS 0x0220
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_56)
	REGDEF_BIT(pre_rth_nlm_c2_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_lut_13,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_56)


	/*
		pre_rth_nlm_c2_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c2_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_57_OFS 0x0224
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_57)
	REGDEF_BIT(pre_rth_nlm_c2_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c2_lut_15,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_57)


	/*
		pre_rth_nlm_c2_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_58_OFS 0x0228
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_58)
	REGDEF_BIT(pre_rth_nlm_c2_lut_16,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_58)


	/*
		pre_rth_nlm_c3_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c3_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_59_OFS 0x022c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_59)
	REGDEF_BIT(pre_rth_nlm_c3_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_lut_1,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_59)


	/*
		pre_rth_nlm_c3_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c3_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_60_OFS 0x0230
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_60)
	REGDEF_BIT(pre_rth_nlm_c3_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_lut_3,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_60)


	/*
		pre_rth_nlm_c3_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c3_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_61_OFS 0x0234
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_61)
	REGDEF_BIT(pre_rth_nlm_c3_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_lut_5,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_61)


	/*
		pre_rth_nlm_c3_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c3_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_62_OFS 0x0238
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_62)
	REGDEF_BIT(pre_rth_nlm_c3_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_lut_7,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_62)


	/*
		pre_rth_nlm_c3_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		pre_rth_nlm_c3_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_63_OFS 0x023c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_63)
	REGDEF_BIT(pre_rth_nlm_c3_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_lut_9,		10)
	REGDEF_END(RANGE_FILTER_REGISTER_63)


	/*
		pre_rth_nlm_c3_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c3_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_64_OFS 0x0240
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_64)
	REGDEF_BIT(pre_rth_nlm_c3_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_lut_11,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_64)


	/*
		pre_rth_nlm_c3_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c3_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_65_OFS 0x0244
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_65)
	REGDEF_BIT(pre_rth_nlm_c3_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_lut_13,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_65)


	/*
		pre_rth_nlm_c3_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		pre_rth_nlm_c3_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_66_OFS 0x0248
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_66)
	REGDEF_BIT(pre_rth_nlm_c3_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_nlm_c3_lut_15,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_66)


	/*
		pre_rth_nlm_c3_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_67_OFS 0x024c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_67)
	REGDEF_BIT(pre_rth_nlm_c3_lut_16,		 10)
	REGDEF_END(RANGE_FILTER_REGISTER_67)


	/*
		pre_rth_bilat_c0_lut_0:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_lut_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_68_OFS 0x0250
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_68)
	REGDEF_BIT(pre_rth_bilat_c0_lut_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_lut_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_68)


	/*
		pre_rth_bilat_c0_lut_2:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_lut_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_69_OFS 0x0254
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_69)
	REGDEF_BIT(pre_rth_bilat_c0_lut_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_lut_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_69)


	/*
		pre_rth_bilat_c0_lut_4:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_lut_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_70_OFS 0x0258
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_70)
	REGDEF_BIT(pre_rth_bilat_c0_lut_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_lut_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_70)


	/*
		pre_rth_bilat_c0_lut_6:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_lut_7:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_71_OFS 0x025c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_71)
	REGDEF_BIT(pre_rth_bilat_c0_lut_6,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_lut_7,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_71)


	/*
		pre_rth_bilat_c0_lut_8:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_lut_9:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_72_OFS 0x0260
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_72)
	REGDEF_BIT(pre_rth_bilat_c0_lut_8,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_lut_9,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_72)


	/*
		pre_rth_bilat_c0_lut_10:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_lut_11:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_73_OFS 0x0264
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_73)
	REGDEF_BIT(pre_rth_bilat_c0_lut_10, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_lut_11, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_73)


	/*
		pre_rth_bilat_c0_lut_12:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_lut_13:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_74_OFS 0x0268
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_74)
	REGDEF_BIT(pre_rth_bilat_c0_lut_12, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_lut_13, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_74)


	/*
		pre_rth_bilat_c0_lut_14:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c0_lut_15:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_75_OFS 0x026c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_75)
	REGDEF_BIT(pre_rth_bilat_c0_lut_14, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c0_lut_15, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_75)


	/*
		pre_rth_bilat_c0_lut_16:	[0x0, 0x3ff],			bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_76_OFS 0x0270
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_76)
	REGDEF_BIT(pre_rth_bilat_c0_lut_16, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_76)


	/*
		pre_rth_bilat_c1_lut_0:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_lut_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_77_OFS 0x0274
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_77)
	REGDEF_BIT(pre_rth_bilat_c1_lut_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_lut_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_77)


	/*
		pre_rth_bilat_c1_lut_2:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_lut_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_78_OFS 0x0278
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_78)
	REGDEF_BIT(pre_rth_bilat_c1_lut_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_lut_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_78)


	/*
		pre_rth_bilat_c1_lut_4:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_lut_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_79_OFS 0x027c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_79)
	REGDEF_BIT(pre_rth_bilat_c1_lut_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_lut_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_79)


	/*
		pre_rth_bilat_c1_lut_6:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_lut_7:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_80_OFS 0x0280
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_80)
	REGDEF_BIT(pre_rth_bilat_c1_lut_6,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_lut_7,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_80)


	/*
		pre_rth_bilat_c1_lut_8:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_lut_9:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_81_OFS 0x0284
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_81)
	REGDEF_BIT(pre_rth_bilat_c1_lut_8,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_lut_9,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_81)


	/*
		pre_rth_bilat_c1_lut_10:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_lut_11:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_82_OFS 0x0288
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_82)
	REGDEF_BIT(pre_rth_bilat_c1_lut_10, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_lut_11, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_82)


	/*
		pre_rth_bilat_c1_lut_12:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_lut_13:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_83_OFS 0x028c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_83)
	REGDEF_BIT(pre_rth_bilat_c1_lut_12, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_lut_13, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_83)


	/*
		pre_rth_bilat_c1_lut_14:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c1_lut_15:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_84_OFS 0x0290
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_84)
	REGDEF_BIT(pre_rth_bilat_c1_lut_14, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c1_lut_15, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_84)


	/*
		pre_rth_bilat_c1_lut_16:	[0x0, 0x3ff],			bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_85_OFS 0x0294
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_85)
	REGDEF_BIT(pre_rth_bilat_c1_lut_16, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_85)


	/*
		pre_rth_bilat_c2_lut_0:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_lut_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_86_OFS 0x0298
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_86)
	REGDEF_BIT(pre_rth_bilat_c2_lut_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_lut_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_86)


	/*
		pre_rth_bilat_c2_lut_2:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_lut_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_87_OFS 0x029c
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_87)
	REGDEF_BIT(pre_rth_bilat_c2_lut_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_lut_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_87)


	/*
		pre_rth_bilat_c2_lut_4:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_lut_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_88_OFS 0x02a0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_88)
	REGDEF_BIT(pre_rth_bilat_c2_lut_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_lut_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_88)


	/*
		pre_rth_bilat_c2_lut_6:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_lut_7:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_89_OFS 0x02a4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_89)
	REGDEF_BIT(pre_rth_bilat_c2_lut_6,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_lut_7,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_89)


	/*
		pre_rth_bilat_c2_lut_8:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_lut_9:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_90_OFS 0x02a8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_90)
	REGDEF_BIT(pre_rth_bilat_c2_lut_8,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_lut_9,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_90)


	/*
		pre_rth_bilat_c2_lut_10:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_lut_11:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_91_OFS 0x02ac
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_91)
	REGDEF_BIT(pre_rth_bilat_c2_lut_10, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_lut_11, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_91)


	/*
		pre_rth_bilat_c2_lut_12:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_lut_13:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_92_OFS 0x02b0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_92)
	REGDEF_BIT(pre_rth_bilat_c2_lut_12, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_lut_13, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_92)


	/*
		pre_rth_bilat_c2_lut_14:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c2_lut_15:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_93_OFS 0x02b4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_93)
	REGDEF_BIT(pre_rth_bilat_c2_lut_14, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c2_lut_15, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_93)


	/*
		pre_rth_bilat_c2_lut_16:	[0x0, 0x3ff],			bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_94_OFS 0x02b8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_94)
	REGDEF_BIT(pre_rth_bilat_c2_lut_16, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_94)


	/*
		pre_rth_bilat_c3_lut_0:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_lut_1:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_95_OFS 0x02bc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_95)
	REGDEF_BIT(pre_rth_bilat_c3_lut_0,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_lut_1,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_95)


	/*
		pre_rth_bilat_c3_lut_2:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_lut_3:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_96_OFS 0x02c0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_96)
	REGDEF_BIT(pre_rth_bilat_c3_lut_2,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_lut_3,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_96)


	/*
		pre_rth_bilat_c3_lut_4:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_lut_5:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_97_OFS 0x02c4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_97)
	REGDEF_BIT(pre_rth_bilat_c3_lut_4,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_lut_5,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_97)


	/*
		pre_rth_bilat_c3_lut_6:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_lut_7:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_98_OFS 0x02c8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_98)
	REGDEF_BIT(pre_rth_bilat_c3_lut_6,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_lut_7,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_98)


	/*
		pre_rth_bilat_c3_lut_8:    [0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_lut_9:    [0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_99_OFS 0x02cc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_99)
	REGDEF_BIT(pre_rth_bilat_c3_lut_8,		  10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_lut_9,		  10)
	REGDEF_END(RANGE_FILTER_REGISTER_99)


	/*
		pre_rth_bilat_c3_lut_10:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_lut_11:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_100_OFS 0x02d0
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_100)
	REGDEF_BIT(pre_rth_bilat_c3_lut_10, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_lut_11, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_100)


	/*
		pre_rth_bilat_c3_lut_12:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_lut_13:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_101_OFS 0x02d4
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_101)
	REGDEF_BIT(pre_rth_bilat_c3_lut_12, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_lut_13, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_101)


	/*
		pre_rth_bilat_c3_lut_14:	[0x0, 0x3ff],			bits : 9_0
		pre_rth_bilat_c3_lut_15:	[0x0, 0x3ff],			bits : 25_16
	*/
#define RANGE_FILTER_REGISTER_102_OFS 0x02d8
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_102)
	REGDEF_BIT(pre_rth_bilat_c3_lut_14, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_rth_bilat_c3_lut_15, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_102)


	/*
		pre_rth_bilat_c3_lut_16:	[0x0, 0x3ff],			bits : 9_0
	*/
#define RANGE_FILTER_REGISTER_103_OFS 0x02dc
	REGDEF_BEGIN(RANGE_FILTER_REGISTER_103)
	REGDEF_BIT(pre_rth_bilat_c3_lut_16, 	   10)
	REGDEF_END(RANGE_FILTER_REGISTER_103)


	/*
		pre_distvgtx_c0:	[0x0, 0xffff],			bits : 15_0
		pre_distvgty_c0:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_0_OFS 0x02e0
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_0)
		REGDEF_BIT(pre_distvgtx_c0, 	   14)
		REGDEF_BIT(               ,        2)
		REGDEF_BIT(pre_distvgty_c0, 	   14)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_0)


	/*
		pre_distvgtx_c1:	[0x0, 0xffff],			bits : 15_0
		pre_distvgty_c1:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_1_OFS 0x02e4
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_1)
		REGDEF_BIT(pre_distvgtx_c1, 	   14)
		REGDEF_BIT(               ,        2)
		REGDEF_BIT(pre_distvgty_c1, 	   14)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_1)


	/*
		pre_distvgtx_c2:	[0x0, 0xffff],			bits : 15_0
		pre_distvgty_c2:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_2_OFS 0x02e8
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_2)
		REGDEF_BIT(pre_distvgtx_c2, 	   14)
		REGDEF_BIT(               ,        2)
		REGDEF_BIT(pre_distvgty_c2, 	   14)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_2)


	/*
		pre_distvgtx_c3:	[0x0, 0xffff],			bits : 15_0
		pre_distvgty_c3:	[0x0, 0xffff],			bits : 31_16
	*/
#define VIGNETTE_SETTING_REGISTER_3_OFS 0x02ec
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_3)
		REGDEF_BIT(pre_distvgtx_c3, 	   14)
		REGDEF_BIT(               ,        2)
		REGDEF_BIT(pre_distvgty_c3, 	   14)
	REGDEF_END(VIGNETTE_SETTING_REGISTER_3)


	/*
		pre_distvgxdiv:    [0x0, 0xfff],			bits : 11_0
		pre_distvgydiv:    [0x0, 0xfff],			bits : 23_12
		pre_distgain  :    [0x0, 0x3],			bits : 29_28
	*/
#define VIGNETTE_SETTING_REGISTER_4_OFS 0x02f0
	REGDEF_BEGIN(VIGNETTE_SETTING_REGISTER_4)
		REGDEF_BIT(pre_distvgxdiv,		  12)
		REGDEF_BIT(pre_distvgydiv,		  12)
		REGDEF_BIT( 			 ,		  4)
		REGDEF_BIT(pre_distgain  ,		  2)
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
		pre_vig_c0_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c0_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_0_OFS 0x02f8
	REGDEF_BEGIN(VIGNETTE_REGISTER_0)
	REGDEF_BIT(pre_vig_c0_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c0_lut_1,		10)
	REGDEF_END(VIGNETTE_REGISTER_0)


	/*
		pre_vig_c0_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c0_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_1_OFS 0x02fc
	REGDEF_BEGIN(VIGNETTE_REGISTER_1)
	REGDEF_BIT(pre_vig_c0_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c0_lut_3,		10)
	REGDEF_END(VIGNETTE_REGISTER_1)


	/*
		pre_vig_c0_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c0_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_2_OFS 0x0300
	REGDEF_BEGIN(VIGNETTE_REGISTER_2)
	REGDEF_BIT(pre_vig_c0_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c0_lut_5,		10)
	REGDEF_END(VIGNETTE_REGISTER_2)


	/*
		pre_vig_c0_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c0_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_3_OFS 0x0304
	REGDEF_BEGIN(VIGNETTE_REGISTER_3)
	REGDEF_BIT(pre_vig_c0_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c0_lut_7,		10)
	REGDEF_END(VIGNETTE_REGISTER_3)


	/*
		pre_vig_c0_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c0_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_4_OFS 0x0308
	REGDEF_BEGIN(VIGNETTE_REGISTER_4)
	REGDEF_BIT(pre_vig_c0_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c0_lut_9,		10)
	REGDEF_END(VIGNETTE_REGISTER_4)


	/*
		pre_vig_c0_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c0_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_5_OFS 0x030c
	REGDEF_BEGIN(VIGNETTE_REGISTER_5)
	REGDEF_BIT(pre_vig_c0_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c0_lut_11,		 10)
	REGDEF_END(VIGNETTE_REGISTER_5)


	/*
		pre_vig_c0_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c0_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_6_OFS 0x0310
	REGDEF_BEGIN(VIGNETTE_REGISTER_6)
	REGDEF_BIT(pre_vig_c0_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c0_lut_13,		 10)
	REGDEF_END(VIGNETTE_REGISTER_6)


	/*
		pre_vig_c0_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c0_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_7_OFS 0x0314
	REGDEF_BEGIN(VIGNETTE_REGISTER_7)
	REGDEF_BIT(pre_vig_c0_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c0_lut_15,		 10)
	REGDEF_END(VIGNETTE_REGISTER_7)


	/*
		pre_vig_c0_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define VIGNETTE_REGISTER_8_OFS 0x0318
	REGDEF_BEGIN(VIGNETTE_REGISTER_8)
	REGDEF_BIT(pre_vig_c0_lut_16,		 10)
	REGDEF_END(VIGNETTE_REGISTER_8)


	/*
		pre_vig_c1_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c1_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_9_OFS 0x031c
	REGDEF_BEGIN(VIGNETTE_REGISTER_9)
	REGDEF_BIT(pre_vig_c1_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c1_lut_1,		10)
	REGDEF_END(VIGNETTE_REGISTER_9)


	/*
		pre_vig_c1_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c1_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_10_OFS 0x0320
	REGDEF_BEGIN(VIGNETTE_REGISTER_10)
	REGDEF_BIT(pre_vig_c1_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c1_lut_3,		10)
	REGDEF_END(VIGNETTE_REGISTER_10)


	/*
		pre_vig_c1_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c1_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_11_OFS 0x0324
	REGDEF_BEGIN(VIGNETTE_REGISTER_11)
	REGDEF_BIT(pre_vig_c1_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c1_lut_5,		10)
	REGDEF_END(VIGNETTE_REGISTER_11)


	/*
		pre_vig_c1_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c1_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_12_OFS 0x0328
	REGDEF_BEGIN(VIGNETTE_REGISTER_12)
	REGDEF_BIT(pre_vig_c1_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c1_lut_7,		10)
	REGDEF_END(VIGNETTE_REGISTER_12)


	/*
		pre_vig_c1_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c1_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_13_OFS 0x032c
	REGDEF_BEGIN(VIGNETTE_REGISTER_13)
	REGDEF_BIT(pre_vig_c1_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c1_lut_9,		10)
	REGDEF_END(VIGNETTE_REGISTER_13)


	/*
		pre_vig_c1_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c1_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_14_OFS 0x0330
	REGDEF_BEGIN(VIGNETTE_REGISTER_14)
	REGDEF_BIT(pre_vig_c1_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c1_lut_11,		 10)
	REGDEF_END(VIGNETTE_REGISTER_14)


	/*
		pre_vig_c1_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c1_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_15_OFS 0x0334
	REGDEF_BEGIN(VIGNETTE_REGISTER_15)
	REGDEF_BIT(pre_vig_c1_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c1_lut_13,		 10)
	REGDEF_END(VIGNETTE_REGISTER_15)


	/*
		pre_vig_c1_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c1_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_16_OFS 0x0338
	REGDEF_BEGIN(VIGNETTE_REGISTER_16)
	REGDEF_BIT(pre_vig_c1_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c1_lut_15,		 10)
	REGDEF_END(VIGNETTE_REGISTER_16)


	/*
		pre_vig_c1_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define VIGNETTE_REGISTER_17_OFS 0x033c
	REGDEF_BEGIN(VIGNETTE_REGISTER_17)
	REGDEF_BIT(pre_vig_c1_lut_16,		 10)
	REGDEF_END(VIGNETTE_REGISTER_17)


	/*
		pre_vig_c2_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c2_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_18_OFS 0x0340
	REGDEF_BEGIN(VIGNETTE_REGISTER_18)
	REGDEF_BIT(pre_vig_c2_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c2_lut_1,		10)
	REGDEF_END(VIGNETTE_REGISTER_18)


	/*
		pre_vig_c2_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c2_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_19_OFS 0x0344
	REGDEF_BEGIN(VIGNETTE_REGISTER_19)
	REGDEF_BIT(pre_vig_c2_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c2_lut_3,		10)
	REGDEF_END(VIGNETTE_REGISTER_19)


	/*
		pre_vig_c2_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c2_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_20_OFS 0x0348
	REGDEF_BEGIN(VIGNETTE_REGISTER_20)
	REGDEF_BIT(pre_vig_c2_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c2_lut_5,		10)
	REGDEF_END(VIGNETTE_REGISTER_20)


	/*
		pre_vig_c2_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c2_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_21_OFS 0x034c
	REGDEF_BEGIN(VIGNETTE_REGISTER_21)
	REGDEF_BIT(pre_vig_c2_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c2_lut_7,		10)
	REGDEF_END(VIGNETTE_REGISTER_21)


	/*
		pre_vig_c2_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c2_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_22_OFS 0x0350
	REGDEF_BEGIN(VIGNETTE_REGISTER_22)
	REGDEF_BIT(pre_vig_c2_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c2_lut_9,		10)
	REGDEF_END(VIGNETTE_REGISTER_22)


	/*
		pre_vig_c2_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c2_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_23_OFS 0x0354
	REGDEF_BEGIN(VIGNETTE_REGISTER_23)
	REGDEF_BIT(pre_vig_c2_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c2_lut_11,		 10)
	REGDEF_END(VIGNETTE_REGISTER_23)


	/*
		pre_vig_c2_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c2_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_24_OFS 0x0358
	REGDEF_BEGIN(VIGNETTE_REGISTER_24)
	REGDEF_BIT(pre_vig_c2_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c2_lut_13,		 10)
	REGDEF_END(VIGNETTE_REGISTER_24)


	/*
		pre_vig_c2_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c2_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_25_OFS 0x035c
	REGDEF_BEGIN(VIGNETTE_REGISTER_25)
	REGDEF_BIT(pre_vig_c2_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c2_lut_15,		 10)
	REGDEF_END(VIGNETTE_REGISTER_25)


	/*
		pre_vig_c2_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define VIGNETTE_REGISTER_26_OFS 0x0360
	REGDEF_BEGIN(VIGNETTE_REGISTER_26)
	REGDEF_BIT(pre_vig_c2_lut_16,		 10)
	REGDEF_END(VIGNETTE_REGISTER_26)


	/*
		pre_vig_c3_lut_0:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c3_lut_1:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_27_OFS 0x0364
	REGDEF_BEGIN(VIGNETTE_REGISTER_27)
	REGDEF_BIT(pre_vig_c3_lut_0,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c3_lut_1,		10)
	REGDEF_END(VIGNETTE_REGISTER_27)


	/*
		pre_vig_c3_lut_2:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c3_lut_3:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_28_OFS 0x0368
	REGDEF_BEGIN(VIGNETTE_REGISTER_28)
	REGDEF_BIT(pre_vig_c3_lut_2,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c3_lut_3,		10)
	REGDEF_END(VIGNETTE_REGISTER_28)


	/*
		pre_vig_c3_lut_4:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c3_lut_5:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_29_OFS 0x036c
	REGDEF_BEGIN(VIGNETTE_REGISTER_29)
	REGDEF_BIT(pre_vig_c3_lut_4,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c3_lut_5,		10)
	REGDEF_END(VIGNETTE_REGISTER_29)


	/*
		pre_vig_c3_lut_6:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c3_lut_7:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_30_OFS 0x0370
	REGDEF_BEGIN(VIGNETTE_REGISTER_30)
	REGDEF_BIT(pre_vig_c3_lut_6,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c3_lut_7,		10)
	REGDEF_END(VIGNETTE_REGISTER_30)


	/*
		pre_vig_c3_lut_8:	 [0x0, 0x3ff],			bits : 9_0
		pre_vig_c3_lut_9:	 [0x0, 0x3ff],			bits : 25_16
	*/
#define VIGNETTE_REGISTER_31_OFS 0x0374
	REGDEF_BEGIN(VIGNETTE_REGISTER_31)
	REGDEF_BIT(pre_vig_c3_lut_8,		10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c3_lut_9,		10)
	REGDEF_END(VIGNETTE_REGISTER_31)


	/*
		pre_vig_c3_lut_10:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c3_lut_11:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_32_OFS 0x0378
	REGDEF_BEGIN(VIGNETTE_REGISTER_32)
	REGDEF_BIT(pre_vig_c3_lut_10,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c3_lut_11,		 10)
	REGDEF_END(VIGNETTE_REGISTER_32)


	/*
		pre_vig_c3_lut_12:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c3_lut_13:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_33_OFS 0x037c
	REGDEF_BEGIN(VIGNETTE_REGISTER_33)
	REGDEF_BIT(pre_vig_c3_lut_12,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c3_lut_13,		 10)
	REGDEF_END(VIGNETTE_REGISTER_33)


	/*
		pre_vig_c3_lut_14:	  [0x0, 0x3ff], 		bits : 9_0
		pre_vig_c3_lut_15:	  [0x0, 0x3ff], 		bits : 25_16
	*/
#define VIGNETTE_REGISTER_34_OFS 0x0380
	REGDEF_BEGIN(VIGNETTE_REGISTER_34)
	REGDEF_BIT(pre_vig_c3_lut_14,		 10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_vig_c3_lut_15,		 10)
	REGDEF_END(VIGNETTE_REGISTER_34)


	/*
		pre_vig_c3_lut_16:	  [0x0, 0x3ff], 		bits : 9_0
	*/
#define VIGNETTE_REGISTER_35_OFS 0x0384
	REGDEF_BEGIN(VIGNETTE_REGISTER_35)
	REGDEF_BIT(pre_vig_c3_lut_16,		 10)
	REGDEF_END(VIGNETTE_REGISTER_35)


	/*
		pre_gbal_ofs_lut_00:	[0x0, 0x3f],			bits : 5_0
		pre_gbal_ofs_lut_01:	[0x0, 0x3f],			bits : 13_8
		pre_gbal_ofs_lut_02:	[0x0, 0x3f],			bits : 21_16
		pre_gbal_ofs_lut_03:	[0x0, 0x3f],			bits : 29_24
	*/
#define GBAL_REGISTER_3_OFS 0x0388
	REGDEF_BEGIN(GBAL_REGISTER_3)
	REGDEF_BIT(pre_gbal_ofs_lut_00, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_01, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_02, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_03, 	   6)
	REGDEF_END(GBAL_REGISTER_3)


	/*
		pre_gbal_ofs_lut_04:	[0x0, 0x3f],			bits : 5_0
		pre_gbal_ofs_lut_05:	[0x0, 0x3f],			bits : 13_8
		pre_gbal_ofs_lut_06:	[0x0, 0x3f],			bits : 21_16
		pre_gbal_ofs_lut_07:	[0x0, 0x3f],			bits : 29_24
	*/
#define GBAL_REGISTER_4_OFS 0x038c
	REGDEF_BEGIN(GBAL_REGISTER_4)
	REGDEF_BIT(pre_gbal_ofs_lut_04, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_05, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_06, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_07, 	   6)
	REGDEF_END(GBAL_REGISTER_4)


	/*
		pre_gbal_ofs_lut_08:	[0x0, 0x3f],			bits : 5_0
		pre_gbal_ofs_lut_09:	[0x0, 0x3f],			bits : 13_8
		pre_gbal_ofs_lut_10:	[0x0, 0x3f],			bits : 21_16
		pre_gbal_ofs_lut_11:	[0x0, 0x3f],			bits : 29_24
	*/
#define GBAL_REGISTER_5_OFS 0x0390
	REGDEF_BEGIN(GBAL_REGISTER_5)
	REGDEF_BIT(pre_gbal_ofs_lut_08, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_09, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_10, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_11, 	   6)
	REGDEF_END(GBAL_REGISTER_5)


	/*
		pre_gbal_ofs_lut_12:	[0x0, 0x3f],			bits : 5_0
		pre_gbal_ofs_lut_13:	[0x0, 0x3f],			bits : 13_8
		pre_gbal_ofs_lut_14:	[0x0, 0x3f],			bits : 21_16
		pre_gbal_ofs_lut_15:	[0x0, 0x3f],			bits : 29_24
	*/
#define GBAL_REGISTER_6_OFS 0x0394
	REGDEF_BEGIN(GBAL_REGISTER_6)
	REGDEF_BIT(pre_gbal_ofs_lut_12, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_13, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_14, 	   6)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_gbal_ofs_lut_15, 	   6)
	REGDEF_END(GBAL_REGISTER_6)


	/*
		pre_gbal_ofs_lut_16:	[0x0, 0x3f],			bits : 5_0
	*/
#define GBAL_REGISTER_7_OFS 0x0398
	REGDEF_BEGIN(GBAL_REGISTER_7)
	REGDEF_BIT(pre_gbal_ofs_lut_16, 	   6)
	REGDEF_END(GBAL_REGISTER_7)


	/*
		pre_rbluma00:	 [0x0, 0x1f],			bits : 4_0
		pre_rbluma01:	 [0x0, 0x1f],			bits : 12_8
		pre_rbluma02:	 [0x0, 0x1f],			bits : 20_16
		pre_rbluma03:	 [0x0, 0x1f],			bits : 28_24
	*/
#define RB_LUMINANCE_REGISTER_0_OFS 0x039c
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_0)
	REGDEF_BIT(pre_rbluma00,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma01,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma02,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma03,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_0)


	/*
		pre_rbluma04:	 [0x0, 0x1f],			bits : 4_0
		pre_rbluma05:	 [0x0, 0x1f],			bits : 12_8
		pre_rbluma06:	 [0x0, 0x1f],			bits : 20_16
		pre_rbluma07:	 [0x0, 0x1f],			bits : 28_24
	*/
#define RB_LUMINANCE_REGISTER_1_OFS 0x03a0
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_1)
	REGDEF_BIT(pre_rbluma04,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma05,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma06,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma07,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_1)


	/*
		pre_rbluma08:	 [0x0, 0x1f],			bits : 4_0
		pre_rbluma09:	 [0x0, 0x1f],			bits : 12_8
		pre_rbluma10:	 [0x0, 0x1f],			bits : 20_16
		pre_rbluma11:	 [0x0, 0x1f],			bits : 28_24
	*/
#define RB_LUMINANCE_REGISTER_2_OFS 0x03a4
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_2)
	REGDEF_BIT(pre_rbluma08,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma09,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma10,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma11,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_2)


	/*
		pre_rbluma12:	 [0x0, 0x1f],			bits : 4_0
		pre_rbluma13:	 [0x0, 0x1f],			bits : 12_8
		pre_rbluma14:	 [0x0, 0x1f],			bits : 20_16
		pre_rbluma15:	 [0x0, 0x1f],			bits : 28_24
	*/
#define RB_LUMINANCE_REGISTER_3_OFS 0x03a8
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_3)
	REGDEF_BIT(pre_rbluma12,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma13,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma14,		5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbluma15,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_3)


	/*
		pre_rbluma16:	 [0x0, 0x1f],			bits : 4_0
	*/
#define RB_LUMINANCE_REGISTER_4_OFS 0x03ac
	REGDEF_BEGIN(RB_LUMINANCE_REGISTER_4)
	REGDEF_BIT(pre_rbluma16,		5)
	REGDEF_END(RB_LUMINANCE_REGISTER_4)

	/*
		pre_rbratio00:	  [0x0, 0x1f],			bits : 4_0
		pre_rbratio01:	  [0x0, 0x1f],			bits : 12_8
		pre_rbratio02:	  [0x0, 0x1f],			bits : 20_16
		pre_rbratio03:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_0_OFS 0x03c0
	REGDEF_BEGIN(RB_RATIO_REGISTER_0)
	REGDEF_BIT(pre_rbratio00,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio01,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio02,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio03,		 5)
	REGDEF_END(RB_RATIO_REGISTER_0)


	/*
		pre_rbratio04:	  [0x0, 0x1f],			bits : 4_0
		pre_rbratio05:	  [0x0, 0x1f],			bits : 12_8
		pre_rbratio06:	  [0x0, 0x1f],			bits : 20_16
		pre_rbratio07:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_1_OFS 0x03c4
	REGDEF_BEGIN(RB_RATIO_REGISTER_1)
	REGDEF_BIT(pre_rbratio04,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio05,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio06,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio07,		 5)
	REGDEF_END(RB_RATIO_REGISTER_1)


	/*
		pre_rbratio08:	  [0x0, 0x1f],			bits : 4_0
		pre_rbratio09:	  [0x0, 0x1f],			bits : 12_8
		pre_rbratio10:	  [0x0, 0x1f],			bits : 20_16
		pre_rbratio11:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_2_OFS 0x03c8
	REGDEF_BEGIN(RB_RATIO_REGISTER_2)
	REGDEF_BIT(pre_rbratio08,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio09,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio10,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio11,		 5)
	REGDEF_END(RB_RATIO_REGISTER_2)


	/*
		pre_rbratio12:	  [0x0, 0x1f],			bits : 4_0
		pre_rbratio13:	  [0x0, 0x1f],			bits : 12_8
		pre_rbratio14:	  [0x0, 0x1f],			bits : 20_16
		pre_rbratio15:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_3_OFS 0x03cc
	REGDEF_BEGIN(RB_RATIO_REGISTER_3)
	REGDEF_BIT(pre_rbratio12,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio13,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio14,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio15,		 5)
	REGDEF_END(RB_RATIO_REGISTER_3)


	/*
		pre_rbratio16:	  [0x0, 0x1f],			bits : 4_0
		pre_rbratio17:	  [0x0, 0x1f],			bits : 12_8
		pre_rbratio18:	  [0x0, 0x1f],			bits : 20_16
		pre_rbratio19:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_4_OFS 0x03d0
	REGDEF_BEGIN(RB_RATIO_REGISTER_4)
	REGDEF_BIT(pre_rbratio16,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio17,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio18,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio19,		 5)
	REGDEF_END(RB_RATIO_REGISTER_4)


	/*
		pre_rbratio20:	  [0x0, 0x1f],			bits : 4_0
		pre_rbratio21:	  [0x0, 0x1f],			bits : 12_8
		pre_rbratio22:	  [0x0, 0x1f],			bits : 20_16
		pre_rbratio23:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_5_OFS 0x03d4
	REGDEF_BEGIN(RB_RATIO_REGISTER_5)
	REGDEF_BIT(pre_rbratio20,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio21,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio22,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio23,		 5)
	REGDEF_END(RB_RATIO_REGISTER_5)


	/*
		pre_rbratio24:	  [0x0, 0x1f],			bits : 4_0
		pre_rbratio25:	  [0x0, 0x1f],			bits : 12_8
		pre_rbratio26:	  [0x0, 0x1f],			bits : 20_16
		pre_rbratio27:	  [0x0, 0x1f],			bits : 28_24
	*/
#define RB_RATIO_REGISTER_6_OFS 0x03d8
	REGDEF_BEGIN(RB_RATIO_REGISTER_6)
	REGDEF_BIT(pre_rbratio24,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio25,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio26,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio27,		 5)
	REGDEF_END(RB_RATIO_REGISTER_6)


	/*
		pre_rbratio28	  :    [0x0, 0x1f], 		bits : 4_0
		pre_rbratio29	  :    [0x0, 0x1f], 		bits : 12_8
		pre_rbratio30	  :    [0x0, 0x1f], 		bits : 20_16
		pre_rbratio31	  :    [0x0, 0x1f], 		bits : 28_24
		rbratio_mode (0~2):    [0x0, 0x3],			bits : 30_29
	*/
#define RB_RATIO_REGISTER_7_OFS 0x03dc
	REGDEF_BEGIN(RB_RATIO_REGISTER_7)
	REGDEF_BIT(pre_rbratio28,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio29,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio30,		 5)
	REGDEF_BIT(,		3)
	REGDEF_BIT(pre_rbratio31,		 5)
	REGDEF_BIT(rbratio_mode,		2)
	REGDEF_END(RB_RATIO_REGISTER_7)

	/*
		pre_axi_disable   :    [0x0, 0x1],			bits : 0
		pre_axi_ch_idle   :    [0x0, 0x1],			bits : 1
		pre_axi_channel_en:    [0x0, 0x1ff],			bits : 16_8
		pre_axi_lock_dis  :    [0x0, 0x1ff],			bits : 25_17
	*/



	/*
		pre_r_ostd_num:    [0x0, 0xff], 		bits : 7_0
		pre_w_ostd_num:    [0x0, 0xff], 		bits : 15_8
	*/
#define PRE_AXI_REGISTER_2_OFS 0x03f8
	REGDEF_BEGIN(PRE_AXI_REGISTER_2)
		REGDEF_BIT(pre_r_ostd_num,		  8)
		REGDEF_BIT(pre_w_ostd_num,		  8)
	REGDEF_END(PRE_AXI_REGISTER_2)




	/*
		pre_f_nrs0_en:	  [0x0, 0x1],			bits : 0
	*/
#define NRS_CONTROL_REGISTER_OFS 0x0500
	REGDEF_BEGIN(NRS_REGISTER)
	REGDEF_BIT(pre_f_nrs0_en,		 1)
	REGDEF_END(NRS_CONTROL_REGISTER)


	/*
		PRE_F_NRS0_STR0 :	 [0x0, 0x3FF],			  bits : 9_0
		PRE_F_NRS0_STR1 :	 [0x0, 0x3FF],			  bits : 25_16
	*/
#define NRS_REGISTER0_OFS 0x0504
	REGDEF_BEGIN(NRS_REGISTER0)
	REGDEF_BIT(pre_f_nrs0_str0, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_f_nrs0_str1, 	   10)
	REGDEF_END(NRS_REGISTER0)


	/*
		PRE_F_NRS0_STR2 :	 [0x0, 0x3FF],			  bits : 9_0
		PRE_F_NRS0_STR3 :	 [0x0, 0x3FF],			  bits : 25_16
	*/
#define NRS_REGISTER1_OFS 0x0508
	REGDEF_BEGIN(NRS_REGISTER1)
	REGDEF_BIT(pre_f_nrs0_str2, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_f_nrs0_str3, 	   10)
	REGDEF_END(NRS_REGISTER1)



	/*
		PRE_F_NRS0_STR4 :	 [0x0, 0x3FF],			  bits : 9_0
		PRE_F_NRS0_STR5 :	 [0x0, 0x3FF],			  bits : 25_16
	*/
#define NRS_REGISTER2_OFS 0x050C
	REGDEF_BEGIN(NRS_REGISTER2)
	REGDEF_BIT(pre_f_nrs0_str4, 	   10)
	REGDEF_BIT(,		6)
	REGDEF_BIT(pre_f_nrs0_str5, 	   10)
	REGDEF_END(NRS_REGISTER2)



	/*
		PRE_F_FUSION_YM_SEL 	  :    [0x0, 0x3],			bits : 1_0
		PRE_F_FUSION_BCN_SEL (0~2):    [0x0, 0x3],			bits : 5_4
		PRE_F_FUSION_BCD_SEL (0~2):    [0x0, 0x3],			bits : 9_8
		pre_f_fusion_mode		  :    [0x0, 0x3],			bits : 13_12
		pre_f_fusion_debug_mode   :    [0x0, 0xff], 		bits : 31_28
	*/
#define RHE_FUSION_REGISTER_OFS 0x0520
	REGDEF_BEGIN(RHE_FUSION_REGISTER)
	REGDEF_BIT(pre_f_fusion_ymean_sel,		  2)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_f_fusion_normal_blend_curve_sel, 	   2)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_f_fusion_diff_blend_curve_sel,		 2)
	REGDEF_BIT(,		2)
	REGDEF_BIT(pre_f_fusion_mode,		 2)
	REGDEF_BIT(,		14)
	REGDEF_BIT(pre_f_fusion_dbg_mode,		 4)
	REGDEF_END(RHE_FUSION_REGISTER)


	/*
		PRE_F_FUSION_BCNL_P0			 :	  [0x0, 0xfff], 		   bits : 11_0
		PRE_F_FUSION_BCNL_RANGE (16~4095):	  [0x0, 0xfff], 		   bits : 27_16
	*/
#define RHE_FUSION_LONG_EXP_NORMAL_BLEND_CURVE_OFS 0x0524
	REGDEF_BEGIN(RHE_FUSION_LONG_EXPOSURE_NORMAL_BLENDING_CURVE)
	REGDEF_BIT(pre_f_fusion_long_exp_normal_blend_curve_knee_point0,		12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(pre_f_fusion_long_exp_normal_blend_curve_range,		  12)
	REGDEF_END(RHE_FUSION_LONG_EXP_NORMAL_BLEND_CURVE)


	/*
		PRE_F_FUSION_BCNS_P0		  :    [0x0, 0xfff],			bits : 11_0
		PRE_F_FUSION_BCNS_RANGE 	  :    [0x0, 0xfff],			bits : 27_16
	*/
#define RHE_FUSION_SHORT_EXP_NORMAL_BLEND_CURVE_OFS 0x0528
	REGDEF_BEGIN(RHE_FUSION_SHORT_EXP_NORMAL_BLEND_CURVE)
	REGDEF_BIT(pre_f_fusion_short_exp_normal_blend_curve_knee_point0,		 12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(pre_f_fusion_short_exp_normal_blend_curve_range, 	   12)
	REGDEF_END(RHE_FUSION_SHORT_EXP_NORMAL_BLEND_CURVE)


	/*
		PRE_F_FUSION_BCDL_P0		  :    [0x0, 0xfff],			bits : 11_0
		PRE_F_FUSION_BCDL_RANGE 	  :    [0x0, 0xfff],			bits : 27_16
	*/
#define RHE_FUSION_LONG_EXP_DIFF_BLEND_CURVE_OFS 0x052c
	REGDEF_BEGIN(RHE_FUSION_LONG_EXP_DIFF_BLEND_CURVE)
	REGDEF_BIT(pre_f_fusion_long_exp_diff_blend_curve_knee_point0,		  12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(pre_f_fusion_long_exp_diff_blend_curve_range,		12)
	REGDEF_END(RHE_FUSION_LONG_EXP_DIFF_BLEND_CURVE)


	/*
		PRE_F_FUSION_BCDS_P0		  :    [0x0, 0xfff],			bits : 11_0
		PRE_F_FUSION_BCDS_RANGE (0~12):    [0x0, 0xf],			bits : 15_12
		PRE_F_FUSION_BCDS_P1		  :    [0x0, 0xfff],			bits : 27_16
		PRE_F_FUSION_BCDS_WEDGE 	  :    [0x0, 0x1],			bits : 31
	*/
#define RHE_FUSION_SHORT_EXP_DIFF_BLEND_CURVE_OFS 0x0530
	REGDEF_BEGIN(RHE_FUSION_SHORT_EXP_DIFF_BLEND_CURVE)
	REGDEF_BIT(pre_f_fusion_short_exp_diff_blend_curve_knee_point0, 	   12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(pre_f_fusion_short_exp_diff_blend_curve_range,		 12)
	REGDEF_END(RHE_FUSION_SHORT_EXP_DIFF_BLEND_CURVE)


	/*
		PRE_F_FU_BCNL_SLPOE:	[0x0, 0xffff],			bits : 15_0
		PRE_F_FU_BCNS_SLPOE:	[0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FUSION_SLOPE_0_OFS 0x0534
	REGDEF_BEGIN(RHE_FUSION_SLOPE_0)
	REGDEF_BIT(pre_f_fusion_bcnl_slope, 	   16)
	REGDEF_BIT(pre_f_fusion_bcns_slope, 	   16)
	REGDEF_END(RHE_FUSION_SLOPE_0)


	/*
		PRE_F_FU_BCDL_SLPOE:	[0x0, 0xffff],			bits : 15_0
		PRE_F_FU_BCDS_SLPOE:	[0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FUSION_SLOPE_1_OFS 0x0538
	REGDEF_BEGIN(RHE_FUSION_SLOPE_1)
	REGDEF_BIT(pre_f_fusion_bcdl_slope, 	   16)
	REGDEF_BIT(pre_f_fusion_bcds_slope, 	   16)
	REGDEF_END(RHE_FUSION_SLOPE_1)


	/*
		PRE_F_FU_EVRATIO:	 [0x0, 0x1ff],			bits : 8_0
	*/
#define RHE_EVRATIO_REGISTER_OFS 0x053c
	REGDEF_BEGIN(RHE_EVRATIO_REGISTER)
	REGDEF_BIT(pre_f_fusion_evratio,		9)
	REGDEF_END(RHE_EVRATIO_REGISTER)


	/*
		pre_f_fusion_mc_lumthr	  :    [0x0, 0xfff],			bits : 11_0
		pre_f_fusion_mc_diff_ratio:    [0x0, 0x3],			bits : 13_12
	*/
#define RHE_FUSION_MOTION_COMPENSATION_OFS 0x0540
	REGDEF_BEGIN(RHE_FUSION_MOTION_COMPENSATION)
	REGDEF_BIT(pre_f_fusion_mc_lumthr,		  12)
	REGDEF_BIT(pre_f_fusion_mc_diff_ratio,		  2)
	REGDEF_END(RHE_FUSION_MOTION_COMPENSATION)


	/*
		pre_f_fusion_mc_lut_diff_weight0 (0~16):	[0x0, 0x1f],		   bits : 4_0
		pre_f_fusion_mc_lut_diff_weight1 (0~16):	[0x0, 0x1f],		   bits : 9_5
		pre_f_fusion_mc_lut_diff_weight2 (0~16):	[0x0, 0x1f],		   bits : 14_10
		pre_f_fusion_mc_lut_diff_weight3 (0~16):	[0x0, 0x1f],		   bits : 19_15
		pre_f_fusion_mc_lut_diff_weight4 (0~16):	[0x0, 0x1f],		   bits : 24_20
		pre_f_fusion_mc_lut_diff_weight5 (0~16):	[0x0, 0x1f],		   bits : 29_25
	*/
#define RHE_FUSION_MOTION_COMPENSATION_LUT_0_OFS 0x0544
	REGDEF_BEGIN(RHE_FUSION_MOTION_COMPENSATION_LUT_0)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight0,		5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight1,		5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight2,		5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight3,		5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight4,		5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight5,		5)
	REGDEF_END(RHE_FUSION_MOTION_COMPENSATION_LUT_0)


	/*
		pre_f_fusion_mc_lut_diff_weight6 (0~16) :	 [0x0, 0x1f],		   bits : 4_0
		pre_f_fusion_mc_lut_diff_weight7 (0~16) :	 [0x0, 0x1f],		   bits : 9_5
		pre_f_fusion_mc_lut_diff_weight8 (0~16) :	 [0x0, 0x1f],		   bits : 14_10
		pre_f_fusion_mc_lut_diff_weight9 (0~16) :	 [0x0, 0x1f],		   bits : 19_15
		pre_f_fusion_mc_lut_diff_weight10 (0~16):	 [0x0, 0x1f],		   bits : 24_20
		pre_f_fusion_mc_lut_diff_weight11 (0~16):	 [0x0, 0x1f],		   bits : 29_25
	*/
#define RHE_FUSION_MOTION_COMPENSATION_LUT_1_OFS 0x0548
	REGDEF_BEGIN(RHE_FUSION_MOTION_COMPENSATION_LUT_1)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight6,		5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight7,		5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight8,		5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight9,		5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight10,		 5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight11,		 5)
	REGDEF_END(RHE_FUSION_MOTION_COMPENSATION_LUT_1)


	/*
		pre_f_fusion_mc_lut_diff_weight12 (0~16):	 [0x0, 0x1f],		   bits : 4_0
		pre_f_fusion_mc_lut_diff_weight13 (0~16):	 [0x0, 0x1f],		   bits : 9_5
		pre_f_fusion_mc_lut_diff_weight14 (0~16):	 [0x0, 0x1f],		   bits : 14_10
		pre_f_fusion_mc_lut_diff_weight15 (0~16):	 [0x0, 0x1f],		   bits : 19_15
		PRE_F_FUSION_MC_LUT_DWD (0~16)	:	 [0x0, 0x1f],			bits : 24_20
	*/
#define RHE_FUSION_MOTION_COMPENSATION_LUT_2_OFS 0x054c
	REGDEF_BEGIN(RHE_FUSION_MOTION_COMPENSATION_LUT_2)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight12,		 5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight13,		 5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight14,		 5)
	REGDEF_BIT(pre_f_fusion_mc_lut_diff_weight15,		 5)
	REGDEF_BIT(pre_f_fusion_mc_lut_difflumth_diff_weight,		 5)
	REGDEF_END(RHE_FUSION_MOTION_COMPENSATION_LUT_2)


	/*
		PRE_F_DS0_TH  :    [0x0, 0xfff],			bits : 11_0
		PRE_F_DS0_STEP:    [0x0, 0xff], 		bits : 23_16
		PRE_F_DS0_LB  :    [0x0, 0xff], 		bits : 31_24
	*/
#define RHE_FUSION_PATH0_DARK_SATURATION_REDUCTION_OFS 0x0550
	REGDEF_BEGIN(RHE_FUSION_PATH0_DSR)
	REGDEF_BIT(pre_f_dark_sat_reduction0_th,		12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(pre_f_dark_sat_reduction0_step,		  8)
	REGDEF_BIT(pre_f_dark_sat_reduction0_low_bound, 	   8)
	REGDEF_END(RHE_FUSION_PATH0_DARK_SATURATION_REDUCTION)


	/*
		PRE_F_DS1_TH  :    [0x0, 0xfff],			bits : 11_0
		PRE_F_DS1_STEP:    [0x0, 0xff], 		bits : 23_16
		PRE_F_DS1_LB  :    [0x0, 0xff], 		bits : 31_24
	*/
#define RHE_FUSION_PATH1_DARK_SATURATION_REDUCTION_OFS 0x0554
	REGDEF_BEGIN(RHE_FUSION_PATH1_DSR)
	REGDEF_BIT(pre_f_dark_sat_reduction1_th,		12)
	REGDEF_BIT(,		4)
	REGDEF_BIT(pre_f_dark_sat_reduction1_step,		  8)
	REGDEF_BIT(pre_f_dark_sat_reduction1_low_bound, 	   8)
	REGDEF_END(RHE_FUSION_PATH1_DARK_SATURATION_REDUCTION)


	/*
		PRE_F_FCURVE_YM_SEL (0~2)  :	[0x0, 0x3], 		bits : 1_0
		pre_f_fcurve_yvweight (0~8):	[0x0, 0xf], 		bits : 5_2
	*/
#define RHE_FCURVE_CTRL_OFS 0x0560
	REGDEF_BEGIN(RHE_FCURVE_CTRL)
	REGDEF_BIT(pre_f_fcurve_ymean_select,		 2)
	REGDEF_BIT(pre_f_fcurve_yvweight,		 4)
	REGDEF_END(RHE_FCURVE_CTRL)


	/*
		pre_f_fcurve_yweight_lut0:	  [0x0, 0xff],			bits : 7_0
		pre_f_fcurve_yweight_lut1:	  [0x0, 0xff],			bits : 15_8
		pre_f_fcurve_yweight_lut2:	  [0x0, 0xff],			bits : 23_16
		pre_f_fcurve_yweight_lut3:	  [0x0, 0xff],			bits : 31_24
	*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER0_OFS 0x0564
	REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER0)
	REGDEF_BIT(pre_f_fcurve_yweight_lut0,		 8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut1,		 8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut2,		 8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut3,		 8)
	REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER0)


	/*
		pre_f_fcurve_yweight_lut4:	  [0x0, 0xff],			bits : 7_0
		pre_f_fcurve_yweight_lut5:	  [0x0, 0xff],			bits : 15_8
		pre_f_fcurve_yweight_lut6:	  [0x0, 0xff],			bits : 23_16
		pre_f_fcurve_yweight_lut7:	  [0x0, 0xff],			bits : 31_24
	*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER1_OFS 0x0568
	REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER1)
	REGDEF_BIT(pre_f_fcurve_yweight_lut4,		 8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut5,		 8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut6,		 8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut7,		 8)
	REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER1)


	/*
		pre_f_fcurve_yweight_lut8 :    [0x0, 0xff], 		bits : 7_0
		pre_f_fcurve_yweight_lut9 :    [0x0, 0xff], 		bits : 15_8
		pre_f_fcurve_yweight_lut10:    [0x0, 0xff], 		bits : 23_16
		pre_f_fcurve_yweight_lut11:    [0x0, 0xff], 		bits : 31_24
	*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER2_OFS 0x056c
	REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER2)
	REGDEF_BIT(pre_f_fcurve_yweight_lut8,		 8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut9,		 8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut10,		  8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut11,		  8)
	REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER2)


	/*
		pre_f_fcurve_yweight_lut12:    [0x0, 0xff], 		bits : 7_0
		pre_f_fcurve_yweight_lut13:    [0x0, 0xff], 		bits : 15_8
		pre_f_fcurve_yweight_lut14:    [0x0, 0xff], 		bits : 23_16
		pre_f_fcurve_yweight_lut15:    [0x0, 0xff], 		bits : 31_24
	*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER3_OFS 0x0570
	REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER3)
	REGDEF_BIT(pre_f_fcurve_yweight_lut12,		  8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut13,		  8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut14,		  8)
	REGDEF_BIT(pre_f_fcurve_yweight_lut15,		  8)
	REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER3)


	/*
		pre_f_fcurve_yweight_lut16:    [0x0, 0xff], 		bits : 7_0
	*/
#define RHE_FCURVE_Y_WEIGHT_REGISTER4_OFS 0x0574
	REGDEF_BEGIN(RHE_FCURVE_Y_WEIGHT_REGISTER4)
	REGDEF_BIT(pre_f_fcurve_yweight_lut16,		  8)
	REGDEF_END(RHE_FCURVE_Y_WEIGHT_REGISTER4)


	/*
		pre_f_fcurve_l_0:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_l_1:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_0_OFS 0x0578
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_0)
		REGDEF_BIT(pre_f_fcurve_l_0,		16)
		REGDEF_BIT(pre_f_fcurve_l_1,		16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_0)


	/*
		pre_f_fcurve_l_2:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_l_3:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_1_OFS 0x057c
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_1)
		REGDEF_BIT(pre_f_fcurve_l_2,		16)
		REGDEF_BIT(pre_f_fcurve_l_3,		16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_1)


	/*
		pre_f_fcurve_l_4:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_l_5:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_2_OFS 0x0580
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_2)
		REGDEF_BIT(pre_f_fcurve_l_4,		16)
		REGDEF_BIT(pre_f_fcurve_l_5,		16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_2)


	/*
		pre_f_fcurve_l_6:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_l_7:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_3_OFS 0x0584
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_3)
		REGDEF_BIT(pre_f_fcurve_l_6,		16)
		REGDEF_BIT(pre_f_fcurve_l_7,		16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_3)


	/*
		pre_f_fcurve_l_8:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_l_9:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_4_OFS 0x0588
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_4)
		REGDEF_BIT(pre_f_fcurve_l_8,		16)
		REGDEF_BIT(pre_f_fcurve_l_9,		16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_4)


	/*
		pre_f_fcurve_l_10:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_11:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_5_OFS 0x058c
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_5)
		REGDEF_BIT(pre_f_fcurve_l_10,		 16)
		REGDEF_BIT(pre_f_fcurve_l_11,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_5)


	/*
		pre_f_fcurve_l_12:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_13:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_6_OFS 0x0590
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_6)
		REGDEF_BIT(pre_f_fcurve_l_12,		 16)
		REGDEF_BIT(pre_f_fcurve_l_13,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_6)


	/*
		pre_f_fcurve_l_14:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_15:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_7_OFS 0x0594
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_7)
		REGDEF_BIT(pre_f_fcurve_l_14,		 16)
		REGDEF_BIT(pre_f_fcurve_l_15,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_7)


	/*
		pre_f_fcurve_l_16:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_17:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_8_OFS 0x0598
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_8)
		REGDEF_BIT(pre_f_fcurve_l_16,		 16)
		REGDEF_BIT(pre_f_fcurve_l_17,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_8)


	/*
		pre_f_fcurve_l_18:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_19:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_9_OFS 0x059c
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_9)
		REGDEF_BIT(pre_f_fcurve_l_18,		 16)
		REGDEF_BIT(pre_f_fcurve_l_19,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_9)


	/*
		pre_f_fcurve_l_20:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_21:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_10_OFS 0x05a0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_10)
		REGDEF_BIT(pre_f_fcurve_l_20,		 16)
		REGDEF_BIT(pre_f_fcurve_l_21,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_10)


	/*
		pre_f_fcurve_l_22:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_23:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_11_OFS 0x05a4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_11)
		REGDEF_BIT(pre_f_fcurve_l_22,		 16)
		REGDEF_BIT(pre_f_fcurve_l_23,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_11)


	/*
		pre_f_fcurve_l_24:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_25:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_12_OFS 0x05a8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_12)
		REGDEF_BIT(pre_f_fcurve_l_24,		 16)
		REGDEF_BIT(pre_f_fcurve_l_25,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_12)


	/*
		pre_f_fcurve_l_26:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_27:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_13_OFS 0x05ac
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_13)
		REGDEF_BIT(pre_f_fcurve_l_26,		 16)
		REGDEF_BIT(pre_f_fcurve_l_27,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_13)


	/*
		pre_f_fcurve_l_28:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_29:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_14_OFS 0x05b0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_14)
		REGDEF_BIT(pre_f_fcurve_l_28,		 16)
		REGDEF_BIT(pre_f_fcurve_l_29,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_14)


	/*
		pre_f_fcurve_l_30:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_31:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_15_OFS 0x05b4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_15)
		REGDEF_BIT(pre_f_fcurve_l_30,		 16)
		REGDEF_BIT(pre_f_fcurve_l_31,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_15)


	/*
		pre_f_fcurve_l_32:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_33:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_16_OFS 0x05b8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_16)
		REGDEF_BIT(pre_f_fcurve_l_32,		 16)
		REGDEF_BIT(pre_f_fcurve_l_33,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_16)


	/*
		pre_f_fcurve_l_34:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_35:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_17_OFS 0x05bc
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_17)
		REGDEF_BIT(pre_f_fcurve_l_34,		 16)
		REGDEF_BIT(pre_f_fcurve_l_35,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_17)


	/*
		pre_f_fcurve_l_36:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_37:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_18_OFS 0x05c0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_18)
		REGDEF_BIT(pre_f_fcurve_l_36,		 16)
		REGDEF_BIT(pre_f_fcurve_l_37,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_18)


	/*
		pre_f_fcurve_l_38:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_39:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_19_OFS 0x05c4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_19)
		REGDEF_BIT(pre_f_fcurve_l_38,		 16)
		REGDEF_BIT(pre_f_fcurve_l_39,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_19)


	/*
		pre_f_fcurve_l_40:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_41:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_20_OFS 0x05c8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_20)
		REGDEF_BIT(pre_f_fcurve_l_40,		 16)
		REGDEF_BIT(pre_f_fcurve_l_41,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_20)


	/*
		pre_f_fcurve_l_42:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_43:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_21_OFS 0x05cc
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_21)
		REGDEF_BIT(pre_f_fcurve_l_42,		 16)
		REGDEF_BIT(pre_f_fcurve_l_43,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_21)


	/*
		pre_f_fcurve_l_44:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_45:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_22_OFS 0x05d0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_22)
		REGDEF_BIT(pre_f_fcurve_l_44,		 16)
		REGDEF_BIT(pre_f_fcurve_l_45,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_22)


	/*
		pre_f_fcurve_l_46:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_47:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_23_OFS 0x05d4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_23)
		REGDEF_BIT(pre_f_fcurve_l_46,		 16)
		REGDEF_BIT(pre_f_fcurve_l_47,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_23)


	/*
		pre_f_fcurve_l_48:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_49:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_24_OFS 0x05d8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_24)
		REGDEF_BIT(pre_f_fcurve_l_48,		 16)
		REGDEF_BIT(pre_f_fcurve_l_49,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_24)


	/*
		pre_f_fcurve_l_50:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_51:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_25_OFS 0x05dc
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_25)
		REGDEF_BIT(pre_f_fcurve_l_50,		 16)
		REGDEF_BIT(pre_f_fcurve_l_51,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_25)


	/*
		pre_f_fcurve_l_52:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_53:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_26_OFS 0x05e0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_26)
		REGDEF_BIT(pre_f_fcurve_l_52,		 16)
		REGDEF_BIT(pre_f_fcurve_l_53,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_26)


	/*
		pre_f_fcurve_l_54:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_55:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_27_OFS 0x05e4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_27)
		REGDEF_BIT(pre_f_fcurve_l_54,		 16)
		REGDEF_BIT(pre_f_fcurve_l_55,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_27)


	/*
		pre_f_fcurve_l_56:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_57:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_28_OFS 0x05e8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_28)
		REGDEF_BIT(pre_f_fcurve_l_56,		 16)
		REGDEF_BIT(pre_f_fcurve_l_57,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_28)


	/*
		pre_f_fcurve_l_58:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_59:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_29_OFS 0x05ec
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_29)
		REGDEF_BIT(pre_f_fcurve_l_58,		 16)
		REGDEF_BIT(pre_f_fcurve_l_59,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_29)


	/*
		pre_f_fcurve_l_60:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_61:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_30_OFS 0x05f0
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_30)
		REGDEF_BIT(pre_f_fcurve_l_60,		 16)
		REGDEF_BIT(pre_f_fcurve_l_61,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_30)


	/*
		pre_f_fcurve_l_62:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_l_63:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_31_OFS 0x05f4
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_31)
		REGDEF_BIT(pre_f_fcurve_l_62,		 16)
		REGDEF_BIT(pre_f_fcurve_l_63,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_31)


	/*
		pre_f_fcurve_l_64:	  [0x0, 0xffff],			bits : 15_0
	*/
#define RHE_FCURVE_L_INDEX_REGISTER_32_OFS 0x05f8
	REGDEF_BEGIN(RHE_FCURVE_L_INDEX_REGISTER_32)
		REGDEF_BIT(pre_f_fcurve_l_64,		 16)
	REGDEF_END(RHE_FCURVE_L_INDEX_REGISTER_32)

	/*
		pre_f_fcurve_r_0:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_r_1:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_0_OFS 0x067c
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_0)
		REGDEF_BIT(pre_f_fcurve_r_0,		16)
		REGDEF_BIT(pre_f_fcurve_r_1,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_0)


	/*
		pre_f_fcurve_r_2:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_r_3:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_1_OFS 0x0680
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_1)
		REGDEF_BIT(pre_f_fcurve_r_2,		16)
		REGDEF_BIT(pre_f_fcurve_r_3,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_1)


	/*
		pre_f_fcurve_r_4:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_r_5:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_2_OFS 0x0684
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_2)
		REGDEF_BIT(pre_f_fcurve_r_4,		16)
		REGDEF_BIT(pre_f_fcurve_r_5,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_2)


	/*
		pre_f_fcurve_r_6:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_r_7:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_3_OFS 0x0688
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_3)
		REGDEF_BIT(pre_f_fcurve_r_6,		16)
		REGDEF_BIT(pre_f_fcurve_r_7,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_3)


	/*
		pre_f_fcurve_r_8:	 [0x0, 0xffff], 		bits : 15_0
		pre_f_fcurve_r_9:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_4_OFS 0x068c
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_4)
		REGDEF_BIT(pre_f_fcurve_r_8,		16)
		REGDEF_BIT(pre_f_fcurve_r_9,		16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_4)


	/*
		pre_f_fcurve_r_10:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_r_11:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_5_OFS 0x0690
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_5)
		REGDEF_BIT(pre_f_fcurve_r_10,		 16)
		REGDEF_BIT(pre_f_fcurve_r_11,		 16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_5)


	/*
		pre_f_fcurve_r_12:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_r_13:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_6_OFS 0x0694
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_6)
		REGDEF_BIT(pre_f_fcurve_r_12,		 16)
		REGDEF_BIT(pre_f_fcurve_r_13,		 16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_6)


	/*
		pre_f_fcurve_r_14:	  [0x0, 0xffff],			bits : 15_0
		pre_f_fcurve_r_15:	  [0x0, 0xffff],			bits : 31_16
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_7_OFS 0x0698
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_7)
		REGDEF_BIT(pre_f_fcurve_r_14,		 16)
		REGDEF_BIT(pre_f_fcurve_r_15,		 16)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_7)


	/*
		pre_f_fcurve_r_16		 :	  [0x0, 0xffff],			bits : 15_0
		PRE_F_FCURVE_EV_FMT (0~4):	  [0x0, 0x7],			bits : 26_24
	*/
#define RHE_FCURVE_R_INDEX_REGISTER_8_OFS 0x069c
	REGDEF_BEGIN(RHE_FCURVE_R_INDEX_REGISTER_8)
		REGDEF_BIT(pre_f_fcurve_r_16		,		 16)
		REGDEF_BIT( 						,		 8)
		REGDEF_BIT(pre_f_fcurve_ev_fmt      ,		 3)
	REGDEF_END(RHE_FCURVE_R_INDEX_REGISTER_8)



	/*
		pre_fpn_en		   :	[0x0, 0x1], 		bits : 0
		pre_fpn_cgain_range:	[0x0, 0x1], 		bits : 1
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER_OFS 0x070c
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER)
		REGDEF_BIT(pre_fpn_en		  , 	   1)
		REGDEF_BIT(pre_fpn_cgain_range, 	   1)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER)


	/*
		pre_fpn_p0_cgain_r :	[0x0, 0x3ff],			bits : 9_0
		pre_fpn_p0_cgain_gr:	[0x0, 0x3ff],			bits : 25_16
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER1_OFS 0x0710
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER1)
		REGDEF_BIT(pre_fpn_p0_cgain_r , 	   10)
		REGDEF_BIT( 				  , 	   6)
		REGDEF_BIT(pre_fpn_p0_cgain_gr, 	   10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER1)


	/*
		pre_fpn_p0_cgain_gb:	[0x0, 0x3ff],			bits : 9_0
		pre_fpn_p0_cgain_b :	[0x0, 0x3ff],			bits : 25_16
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER2_OFS 0x0714
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER2)
		REGDEF_BIT(pre_fpn_p0_cgain_gb, 	   10)
		REGDEF_BIT( 				  , 	   6)
		REGDEF_BIT(pre_fpn_p0_cgain_b , 	   10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER2)


	/*
		pre_fpn_p0_cgain_ir:	[0x0, 0x3ff],			bits : 9_0
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER3_OFS 0x0718
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER3)
		REGDEF_BIT(pre_fpn_p0_cgain_ir, 	   10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER3)


	/*
		pre_fpn_p0_cofs_r :    [0x0, 0x3ff],			bits : 9_0
		pre_fpn_p0_cofs_gr:    [0x0, 0x3ff],			bits : 25_16
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER7_OFS 0x0728
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER7)
		REGDEF_BIT(pre_fpn_p0_cofs_r ,		  10)
		REGDEF_BIT( 				 ,		  6)
		REGDEF_BIT(pre_fpn_p0_cofs_gr,		  10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER7)


	/*
		pre_fpn_p0_cofs_gb:    [0x0, 0x3ff],			bits : 9_0
		pre_fpn_p0_cofs_b :    [0x0, 0x3ff],			bits : 25_16
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER8_OFS 0x072c
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER8)
		REGDEF_BIT(pre_fpn_p0_cofs_gb,		  10)
		REGDEF_BIT( 				 ,		  6)
		REGDEF_BIT(pre_fpn_p0_cofs_b ,		  10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER8)


	/*
		pre_fpn_p0_cofs_ir:    [0x0, 0x3ff],			bits : 9_0
	*/
#define DARK_FRAME_SUBTRACTION_REGISTER9_OFS 0x0730
	REGDEF_BEGIN(DARK_FRAME_SUBTRACTION_REGISTER9)
		REGDEF_BIT(pre_fpn_p0_cofs_ir,		  10)
	REGDEF_END(DARK_FRAME_SUBTRACTION_REGISTER9)


	/*
		pre_ll_cmd_start_addr_info:    [0x0, 0xffffffff],			bits : 31_0
	*/
#define LINKED_LIST_DEBUG_REGISTER0_OFS 0x07f8
	REGDEF_BEGIN(LINKED_LIST_DEBUG_REGISTER0)
		REGDEF_BIT(pre_ll_cmd_start_addr_info,		  32)
	REGDEF_END(LINKED_LIST_DEBUG_REGISTER0)


	/*
		pre_ll_cmdprs_cnt:	  [0x0, 0xffffffff],			bits : 31_0
	*/
#define LINKED_LIST_DEBUG_REGISTER1_OFS 0x07fc
	REGDEF_BEGIN(LINKED_LIST_DEBUG_REGISTER1)
		REGDEF_BIT(pre_ll_cmdprs_cnt,		 32)
	REGDEF_END(LINKED_LIST_DEBUG_REGISTER1)


	/*
		pre_r_degamma_en:	 [0x0, 0x1],			bits : 0
		pre_r_decode_en :	 [0x0, 0x1],			bits : 1
		pre_r_decode2_en:	 [0x0, 0x1],			bits : 2
		pre_r_dith_en	:	 [0x0, 0x1],			bits : 4
		pre_r_segbitno	:	 [0x0, 0x3],			bits : 9_8
		pre_r_dith_rst	:	 [0x0, 0x1],			bits : 12
	*/
#define RDE_CONTROL_OFS 0x0800
	REGDEF_BEGIN(RDE_CONTROL)
		REGDEF_BIT(pre_r_degamma_en,		1)
		REGDEF_BIT(pre_r_decode_en ,		1)
		REGDEF_BIT(pre_r_decode2_en,		1)
		REGDEF_BIT( 			   ,		1)
		REGDEF_BIT(pre_r_dith_en   ,		1)
		REGDEF_BIT( 			   ,		3)
		REGDEF_BIT(pre_r_segbitno  ,		2)
		REGDEF_BIT( 			   ,		2)
		REGDEF_BIT(pre_r_dith_rst  ,		1)
	REGDEF_END(RDE_CONTROL)


	/*
		pre_r_dct_qtbl0_idx:	[0x0, 0x1f],			bits : 4_0
		pre_r_dct_qtbl1_idx:	[0x0, 0x1f],			bits : 12_8
		pre_r_dct_qtbl2_idx:	[0x0, 0x1f],			bits : 20_16
		pre_r_dct_qtbl3_idx:	[0x0, 0x1f],			bits : 28_24
	*/
#define DCT_QTBL_REGISTER_0_OFS 0x0804
	REGDEF_BEGIN(DCT_QTBL_REGISTER_0)
		REGDEF_BIT(pre_r_dct_qtbl0_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(pre_r_dct_qtbl1_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(pre_r_dct_qtbl2_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(pre_r_dct_qtbl3_idx, 	   5)
	REGDEF_END(DCT_QTBL_REGISTER_0)


	/*
		pre_r_dct_qtbl4_idx:	[0x0, 0x1f],			bits : 4_0
		pre_r_dct_qtbl5_idx:	[0x0, 0x1f],			bits : 12_8
		pre_r_dct_qtbl6_idx:	[0x0, 0x1f],			bits : 20_16
		pre_r_dct_qtbl7_idx:	[0x0, 0x1f],			bits : 28_24
	*/
#define DCT_QTBL_REGISTER_1_OFS 0x0808
	REGDEF_BEGIN(DCT_QTBL_REGISTER_1)
		REGDEF_BIT(pre_r_dct_qtbl4_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(pre_r_dct_qtbl5_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(pre_r_dct_qtbl6_idx, 	   5)
		REGDEF_BIT( 				  , 	   3)
		REGDEF_BIT(pre_r_dct_qtbl7_idx, 	   5)
	REGDEF_END(DCT_QTBL_REGISTER_1)


	/*
		pre_r_out_rand1_init1:	  [0x0, 0xf],			bits : 3_0
		pre_r_out_rand1_init2:	  [0x0, 0x7fff],			bits : 18_4
	*/
#define DITHERING_INITIAL_REGISTER_0_OFS 0x080c
	REGDEF_BEGIN(DITHERING_INITIAL_REGISTER_0)
		REGDEF_BIT(pre_r_out_rand1_init1,		 4)
		REGDEF_BIT(pre_r_out_rand1_init2,		 15)
	REGDEF_END(DITHERING_INITIAL_REGISTER_0)


	/*
		pre_r_out_rand2_init1:	  [0x0, 0xf],			bits : 3_0
		pre_r_out_rand2_init2:	  [0x0, 0x7fff],			bits : 18_4
	*/
#define DITHERING_INITIAL_REGISTER_1_OFS 0x0810
	REGDEF_BEGIN(DITHERING_INITIAL_REGISTER_1)
		REGDEF_BIT(pre_r_out_rand2_init1,		 4)
		REGDEF_BIT(pre_r_out_rand2_init2,		 15)
	REGDEF_END(DITHERING_INITIAL_REGISTER_1)

	/*
		pre_stripe_size0:	 [0x0, 0x7ff],			bits : 10_0
		pre_stripe_num	:	 [0x0, 0xf],			bits : 15_12
		pre_stripe_size1:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define PRE_MULTIPLE_STRIPE_SETTING_REGISTER0_OFS 0x0838
	REGDEF_BEGIN(PRE_MULTIPLE_STRIPE_SETTING_REGISTER0)
		REGDEF_BIT(pre_stripe_size0,		11)
		REGDEF_BIT( 			   ,		1)
		REGDEF_BIT(pre_stripe_num  ,		4)
		REGDEF_BIT(pre_stripe_size1,		11)
	REGDEF_END(PRE_MULTIPLE_STRIPE_SETTING_REGISTER0)


	/*
		pre_stripe_size2:	 [0x0, 0x7ff],			bits : 10_0
		pre_stripe_size3:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define PRE_MULTIPLE_STRIPE_SETTING_REGISTER1_OFS 0x083c
	REGDEF_BEGIN(PRE_MULTIPLE_STRIPE_SETTING_REGISTER1)
		REGDEF_BIT(pre_stripe_size2,		11)
		REGDEF_BIT( 			   ,		5)
		REGDEF_BIT(pre_stripe_size3,		11)
	REGDEF_END(PRE_MULTIPLE_STRIPE_SETTING_REGISTER1)


	/*
		pre_stripe_size4:	 [0x0, 0x7ff],			bits : 10_0
		pre_stripe_size5:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define PRE_MULTIPLE_STRIPE_SETTING_REGISTER2_OFS 0x0840
	REGDEF_BEGIN(PRE_MULTIPLE_STRIPE_SETTING_REGISTER2)
		REGDEF_BIT(pre_stripe_size4,		11)
		REGDEF_BIT( 			   ,		5)
		REGDEF_BIT(pre_stripe_size5,		11)
	REGDEF_END(PRE_MULTIPLE_STRIPE_SETTING_REGISTER2)


	/*
		pre_stripe_size6:	 [0x0, 0x7ff],			bits : 10_0
		pre_stripe_size7:	 [0x0, 0x7ff],			bits : 26_16
	*/
#define PRE_MULTIPLE_STRIPE_SETTING_REGISTER3_OFS 0x0844
	REGDEF_BEGIN(PRE_MULTIPLE_STRIPE_SETTING_REGISTER3)
		REGDEF_BIT(pre_stripe_size6,		11)
		REGDEF_BIT( 			   ,		5)
		REGDEF_BIT(pre_stripe_size7,		11)
	REGDEF_END(PRE_MULTIPLE_STRIPE_SETTING_REGISTER3)


	/*
		pre_stripe_overlap:    [0x0, 0x3ff],			bits : 9_3
	*/
#define PRE_MULTIPLE_STRIPE_SETTING_REGISTER4_OFS 0x0848
	REGDEF_BEGIN(PRE_MULTIPLE_STRIPE_SETTING_REGISTER4)
		REGDEF_BIT( 			   ,		3)
		REGDEF_BIT(pre_stripe_overlap,		7)
	REGDEF_END(PRE_MULTIPLE_STRIPE_SETTING_REGISTER4)


	/*
		pre_sub_ipp_cst_en	   :	[0x0, 0x1], 		bits : 0
		pre_sub_ipp_gamma_en   :	[0x0, 0x1], 		bits : 1
		pre_sub_ipp_cfa_en	   :	[0x0, 0x1], 		bits : 2
		pre_sub_ipp_out_yuv_fmt:	[0x0, 0x1], 		bits : 3
	*/
#define SUB_IPP_CONTROL_REGISTER_OFS 0x0860
	REGDEF_BEGIN(SUB_IPP_CONTROL_REGISTER)
		REGDEF_BIT(pre_sub_ipp_cst_en	  , 	   1)
		REGDEF_BIT(pre_sub_ipp_gamma_en   , 	   1)
		REGDEF_BIT(pre_sub_ipp_cfa_en	  , 	   1)
		REGDEF_BIT(pre_sub_ipp_out_yuv_fmt, 	   1)
	REGDEF_END(SUB_IPP_CONTROL_REGISTER)


	/*
		pre_sub_ipp_coef_rr:	[0x0, 0xfff],			bits : 11_0
		pre_sub_ipp_coef_rg:	[0x0, 0xfff],			bits : 27_16
	*/
#define COLOR_CORRECTION_REGISTER0_OFS 0x0864
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER0)
		REGDEF_BIT(pre_sub_ipp_coef_rr, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(pre_sub_ipp_coef_rg, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER0)


	/*
		pre_sub_ipp_coef_rb:	[0x0, 0xfff],			bits : 11_0
		pre_sub_ipp_coef_gr:	[0x0, 0xfff],			bits : 27_16
	*/
#define COLOR_CORRECTION_REGISTER1_OFS 0x0868
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER1)
		REGDEF_BIT(pre_sub_ipp_coef_rb, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(pre_sub_ipp_coef_gr, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER1)


	/*
		pre_sub_ipp_coef_gg:	[0x0, 0xfff],			bits : 11_0
		pre_sub_ipp_coef_gb:	[0x0, 0xfff],			bits : 27_16
	*/
#define COLOR_CORRECTION_REGISTER2_OFS 0x086c
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER2)
		REGDEF_BIT(pre_sub_ipp_coef_gg, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(pre_sub_ipp_coef_gb, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER2)


	/*
		pre_sub_ipp_coef_br:	[0x0, 0xfff],			bits : 11_0
		pre_sub_ipp_coef_bg:	[0x0, 0xfff],			bits : 27_16
	*/
#define COLOR_CORRECTION_REGISTER3_OFS 0x0870
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER3)
		REGDEF_BIT(pre_sub_ipp_coef_br, 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(pre_sub_ipp_coef_bg, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER3)


	/*
		pre_sub_ipp_coef_bb:	[0x0, 0xfff],			bits : 11_0
	*/
#define COLOR_CORRECTION_REGISTER4_OFS 0x0874
	REGDEF_BEGIN(COLOR_CORRECTION_REGISTER4)
		REGDEF_BIT(pre_sub_ipp_coef_bb, 	   12)
	REGDEF_END(COLOR_CORRECTION_REGISTER4)


	/*
		pre_sub_ipp_smpl_x_rate:	[0x0, 0x1f],			bits : 4_0
		pre_sub_ipp_smpl_y_rate:	[0x0, 0x1f],			bits : 12_8
	*/
#define SCALE_REGISTER_OFS 0x0878
	REGDEF_BEGIN(SCALE_REGISTER)
		REGDEF_BIT(pre_sub_ipp_smpl_x_rate, 	   5)
		REGDEF_BIT( 					  , 	   3)
		REGDEF_BIT(pre_sub_ipp_smpl_y_rate, 	   5)
	REGDEF_END(SCALE_REGISTER)


	/*
		pre_sub_ipp_gamma_lut_0:	[0x0, 0xff],			bits : 7_0
		pre_sub_ipp_gamma_lut_1:	[0x0, 0xff],			bits : 15_8
		pre_sub_ipp_gamma_lut_2:	[0x0, 0xff],			bits : 23_16
		pre_sub_ipp_gamma_lut_3:	[0x0, 0xff],			bits : 31_24
	*/
#define RGB_GAMMA_LUT_REGISTER0_OFS 0x087c
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER0)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_0, 	   8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_1, 	   8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_2, 	   8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_3, 	   8)
	REGDEF_END(RGB_GAMMA_LUT_REGISTER0)


	/*
		pre_sub_ipp_gamma_lut_4:	[0x0, 0xff],			bits : 7_0
		pre_sub_ipp_gamma_lut_5:	[0x0, 0xff],			bits : 15_8
		pre_sub_ipp_gamma_lut_6:	[0x0, 0xff],			bits : 23_16
		pre_sub_ipp_gamma_lut_7:	[0x0, 0xff],			bits : 31_24
	*/
#define RGB_GAMMA_LUT_REGISTER1_OFS 0x0880
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER1)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_4, 	   8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_5, 	   8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_6, 	   8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_7, 	   8)
	REGDEF_END(RGB_GAMMA_LUT_REGISTER1)


	/*
		pre_sub_ipp_gamma_lut_8 :	 [0x0, 0xff],			bits : 7_0
		pre_sub_ipp_gamma_lut_9 :	 [0x0, 0xff],			bits : 15_8
		pre_sub_ipp_gamma_lut_10:	 [0x0, 0xff],			bits : 23_16
		pre_sub_ipp_gamma_lut_11:	 [0x0, 0xff],			bits : 31_24
	*/
#define RGB_GAMMA_LUT_REGISTER2_OFS 0x0884
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER2)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_8 ,		8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_9 ,		8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_10,		8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_11,		8)
	REGDEF_END(RGB_GAMMA_LUT_REGISTER2)


	/*
		pre_sub_ipp_gamma_lut_12:	 [0x0, 0xff],			bits : 7_0
		pre_sub_ipp_gamma_lut_13:	 [0x0, 0xff],			bits : 15_8
		pre_sub_ipp_gamma_lut_14:	 [0x0, 0xff],			bits : 23_16
		pre_sub_ipp_gamma_lut_15:	 [0x0, 0xff],			bits : 31_24
	*/
#define RGB_GAMMA_LUT_REGISTER3_OFS 0x0888
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER3)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_12,		8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_13,		8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_14,		8)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_15,		8)
	REGDEF_END(RGB_GAMMA_LUT_REGISTER3)


	/*
		pre_sub_ipp_gamma_lut_16:	 [0x0, 0xff],			bits : 7_0
	*/
#define RGB_GAMMA_LUT_REGISTER4_OFS 0x088c
	REGDEF_BEGIN(RGB_GAMMA_LUT_REGISTER4)
		REGDEF_BIT(pre_sub_ipp_gamma_lut_16,		8)
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
		pre_wdr_en			 :	  [0x0, 0x1],			bits : 0
		pre_wdr_subimg_out_en:	  [0x0, 0x1],			bits : 1
		pre_wdr_histogram_en :	  [0x0, 0x1],			bits : 2
		pre_wdr_histogram_sel:	  [0x0, 0x1],			bits : 3
		pre_wdr_tcurve_en	 :	  [0x0, 0x1],			bits : 4
		pre_wdr_d2d_rand	 :	  [0x0, 0x1],			bits : 5
		pre_wdr_d2d_rand_rst :	  [0x0, 0x1],			bits : 6
	*/
#define WDR_CONTROL_REGISTER0_OFS 0x0900
	REGDEF_BEGIN(WDR_CONTROL_REGISTER0)
		REGDEF_BIT(pre_wdr_en			,		 1)
		REGDEF_BIT(pre_wdr_subimg_out_en,		 1)
		REGDEF_BIT(pre_wdr_histogram_en ,		 1)
		REGDEF_BIT(pre_wdr_histogram_sel,		 1)
		REGDEF_BIT(pre_wdr_tcurve_en	,		 1)
		REGDEF_BIT(pre_wdr_d2d_rand 	,		 1)
		REGDEF_BIT(pre_wdr_d2d_rand_rst ,		 1)
	REGDEF_END(WDR_CONTROL_REGISTER0)


	/*
		pre_wdr_subimg_width  :    [0x0, 0x3f], 		bits : 5_0
		pre_wdr_subimg_height :    [0x0, 0x3f], 		bits : 13_8
		pre_wdr_dithering_en  :    [0x0, 0x1],			bits : 16
		pre_wdr_rand_reset	  :    [0x0, 0x1],			bits : 17
		pre_wdr_rand_sel_(0~2):    [0x0, 0x3],			bits : 19_18
	*/
#define PRE_WDR_SUBIMAGE_REGISTER_0_OFS 0x0904
	REGDEF_BEGIN(PRE_WDR_SUBIMAGE_REGISTER_0)
		REGDEF_BIT(pre_wdr_subimg_width  ,		  6)
		REGDEF_BIT( 					 ,		  2)
		REGDEF_BIT(pre_wdr_subimg_height ,		  6)
		REGDEF_BIT( 					 ,		  2)
		REGDEF_BIT(pre_wdr_dithering_en  ,		  1)
		REGDEF_BIT(pre_wdr_rand_reset	 ,		  1)
		REGDEF_BIT(pre_wdr_rand_sel 	 ,		  2)
	REGDEF_END(PRE_WDR_SUBIMAGE_REGISTER_0)


	/*
		pre_wdr_subimg_sai:    [0x0, 0x3fffffff],			bits : 31_2
	*/
#define PRE_WDR_SUBIMAGE_REGISTER_1_OFS 0x0908
	REGDEF_BEGIN(PRE_WDR_SUBIMAGE_REGISTER_1)
		REGDEF_BIT( 				 ,		  2)
		REGDEF_BIT(pre_wdr_subimg_sai,		  30)
	REGDEF_END(PRE_WDR_SUBIMAGE_REGISTER_1)


	/*
		pre_wdr_subimg_ofsi    :	[0x0, 0x3fff],			bits : 15_2
		pre_wdr_subimg_sai_msb:	[0x0, 0xf], 		bits : 27_24
	*/
#define PRE_WDR_SUBIMAGE_REGISTER_2_OFS 0x090c
	REGDEF_BEGIN(PRE_WDR_SUBIMAGE_REGISTER_2)
		REGDEF_BIT( 					  , 	   2)
		REGDEF_BIT(pre_wdr_subimg_ofsi	  , 	   14)
		REGDEF_BIT( 					  , 	   8)
		REGDEF_BIT(pre_wdr_subimg_sai_msb, 	   4)
	REGDEF_END(PRE_WDR_SUBIMAGE_REGISTER_2)


	/*
		pre_wdr_subimg_sao:    [0x0, 0x3fffffff],			bits : 31_2
	*/
#define PRE_WDR_SUBIMAGE_REGISTER_3_OFS 0x0910
	REGDEF_BEGIN(PRE_WDR_SUBIMAGE_REGISTER_3)
		REGDEF_BIT( 				 ,		  2)
		REGDEF_BIT(pre_wdr_subimg_sao,		  30)
	REGDEF_END(PRE_WDR_SUBIMAGE_REGISTER_3)


	/*
		pre_wdr_subimg_ofso    :	[0x0, 0x3fff],			bits : 15_2
		pre_wdr_subimg_sao_msb:	[0x0, 0xf], 		bits : 27_24
	*/
#define PRE_WDR_SUBIMAGE_REGISTER_4_OFS 0x0914
	REGDEF_BEGIN(PRE_WDR_SUBIMAGE_REGISTER_4)
		REGDEF_BIT( 					  , 	   2)
		REGDEF_BIT(pre_wdr_subimg_ofso	  , 	   14)
		REGDEF_BIT( 					  , 	   8)
		REGDEF_BIT(pre_wdr_subimg_sao_msb, 	   4)
	REGDEF_END(PRE_WDR_SUBIMAGE_REGISTER_4)


	/*
		pre_wdr_subimg_ratio_hori:	  [0x0, 0xffff],			bits : 15_0
		pre_wdr_subimg_ratio_vert:	  [0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_SUBIMAGE_REGISTER_5_OFS 0x0918
	REGDEF_BEGIN(PRE_WDR_SUBIMAGE_REGISTER_5)
		REGDEF_BIT(pre_wdr_subimg_ratio_hori,		 16)
		REGDEF_BIT(pre_wdr_subimg_ratio_vert,		 16)
	REGDEF_END(PRE_WDR_SUBIMAGE_REGISTER_5)


	/*
		pre_wdr_subimg_ratio_hori_div:	  [0x0, 0x3fffff],			bits : 21_0
	*/
#define PRE_WDR_SUBIMAGE_REGISTER_6_OFS 0x091c
	REGDEF_BEGIN(PRE_WDR_SUBIMAGE_REGISTER_6)
		REGDEF_BIT(pre_wdr_subimg_ratio_hori_div,		 22)
	REGDEF_END(PRE_WDR_SUBIMAGE_REGISTER_6)


	/*
		pre_wdr_subimg_ratio_vert_div:	  [0x0, 0x3fffff],			bits : 21_0
	*/
#define PRE_WDR_SUBIMAGE_REGISTER_7_OFS 0x0920
	REGDEF_BEGIN(PRE_WDR_SUBIMAGE_REGISTER_7)
		REGDEF_BIT(pre_wdr_subimg_ratio_vert_div,		 22)
	REGDEF_END(PRE_WDR_SUBIMAGE_REGISTER_7)


	/*
		pre_wdr_gain_prot_str_(0~256):	  [0x0, 0x1ff], 		bits : 8_0
	*/
#define PRE_WDR_GAIN_STRENGTH_REGISTER_OFS 0x0924
	REGDEF_BEGIN(PRE_WDR_GAIN_STRENGTH_REGISTER)
		REGDEF_BIT(pre_wdr_gain_prot_str,		 9)
	REGDEF_END(PRE_WDR_GAIN_STRENGTH_REGISTER)


	/*
		pre_wdr_input_bldrto0:	  [0x0, 0xff],			bits : 7_0
		pre_wdr_input_bldrto1:	  [0x0, 0xff],			bits : 15_8
		pre_wdr_input_bldrto2:	  [0x0, 0xff],			bits : 23_16
		pre_wdr_input_bldrto3:	  [0x0, 0xff],			bits : 31_24
	*/
#define PRE_WDR_INPUT_BLENDING_REGISTER0_OFS 0x0928
	REGDEF_BEGIN(PRE_WDR_INPUT_BLENDING_REGISTER0)
		REGDEF_BIT(pre_wdr_input_bldrto0,		 8)
		REGDEF_BIT(pre_wdr_input_bldrto1,		 8)
		REGDEF_BIT(pre_wdr_input_bldrto2,		 8)
		REGDEF_BIT(pre_wdr_input_bldrto3,		 8)
	REGDEF_END(PRE_WDR_INPUT_BLENDING_REGISTER0)


	/*
		pre_wdr_input_bldrto4:	  [0x0, 0xff],			bits : 7_0
		pre_wdr_input_bldrto5:	  [0x0, 0xff],			bits : 15_8
		pre_wdr_input_bldrto6:	  [0x0, 0xff],			bits : 23_16
		pre_wdr_input_bldrto7:	  [0x0, 0xff],			bits : 31_24
	*/
#define PRE_WDR_INPUT_BLENDING_REGISTER1_OFS 0x092c
	REGDEF_BEGIN(PRE_WDR_INPUT_BLENDING_REGISTER1)
		REGDEF_BIT(pre_wdr_input_bldrto4,		 8)
		REGDEF_BIT(pre_wdr_input_bldrto5,		 8)
		REGDEF_BIT(pre_wdr_input_bldrto6,		 8)
		REGDEF_BIT(pre_wdr_input_bldrto7,		 8)
	REGDEF_END(PRE_WDR_INPUT_BLENDING_REGISTER1)


	/*
		pre_wdr_input_bldrto8 :    [0x0, 0xff], 		bits : 7_0
		pre_wdr_input_bldrto9 :    [0x0, 0xff], 		bits : 15_8
		pre_wdr_input_bldrto10:    [0x0, 0xff], 		bits : 23_16
		pre_wdr_input_bldrto11:    [0x0, 0xff], 		bits : 31_24
	*/
#define PRE_WDR_INPUT_BLENDING_REGISTER2_OFS 0x0930
	REGDEF_BEGIN(PRE_WDR_INPUT_BLENDING_REGISTER2)
		REGDEF_BIT(pre_wdr_input_bldrto8 ,		  8)
		REGDEF_BIT(pre_wdr_input_bldrto9 ,		  8)
		REGDEF_BIT(pre_wdr_input_bldrto10,		  8)
		REGDEF_BIT(pre_wdr_input_bldrto11,		  8)
	REGDEF_END(PRE_WDR_INPUT_BLENDING_REGISTER2)


	/*
		pre_wdr_input_bldrto12:    [0x0, 0xff], 		bits : 7_0
		pre_wdr_input_bldrto13:    [0x0, 0xff], 		bits : 15_8
		pre_wdr_input_bldrto14:    [0x0, 0xff], 		bits : 23_16
		pre_wdr_input_bldrto15:    [0x0, 0xff], 		bits : 31_24
	*/
#define PRE_WDR_INPUT_BLENDING_REGISTER3_OFS 0x0934
	REGDEF_BEGIN(PRE_WDR_INPUT_BLENDING_REGISTER3)
		REGDEF_BIT(pre_wdr_input_bldrto12,		  8)
		REGDEF_BIT(pre_wdr_input_bldrto13,		  8)
		REGDEF_BIT(pre_wdr_input_bldrto14,		  8)
		REGDEF_BIT(pre_wdr_input_bldrto15,		  8)
	REGDEF_END(PRE_WDR_INPUT_BLENDING_REGISTER3)


	/*
		pre_wdr_input_bldrto16		  :    [0x0, 0xff], 		bits : 7_0
		pre_wdr_input_bldsrc_sel_(0~2):    [0x0, 0x3],			bits : 27_26
	*/
#define PRE_WDR_INPUT_BLENDING_REGISTER4_OFS 0x0938
	REGDEF_BEGIN(PRE_WDR_INPUT_BLENDING_REGISTER4)
		REGDEF_BIT(pre_wdr_input_bldrto16		 ,		  8)
		REGDEF_BIT( 							 ,		  18)
		REGDEF_BIT(pre_wdr_input_bldsrc_sel 	 ,		  2)
	REGDEF_END(PRE_WDR_INPUT_BLENDING_REGISTER4)


	/*
		pre_wdr_lpf_c0:    [0x0, 0x7],			bits : 2_0
		pre_wdr_lpf_c1:    [0x0, 0x7],			bits : 5_3
		pre_wdr_lpf_c2:    [0x0, 0x7],			bits : 8_6
	*/
#define PRE_WDR_SUB_IMG_LPF_REGISTER_OFS 0x093c
	REGDEF_BEGIN(PRE_WDR_SUB_IMG_LPF_REGISTER)
		REGDEF_BIT(pre_wdr_lpf_c0,		  3)
		REGDEF_BIT(pre_wdr_lpf_c1,		  3)
		REGDEF_BIT(pre_wdr_lpf_c2,		  3)
	REGDEF_END(PRE_WDR_SUB_IMG_LPF_REGISTER)


	/*
		pre_wdr_coeff1:    [0x0, 0x1fff],			bits : 12_0
		pre_wdr_coeff2:    [0x0, 0x1fff],			bits : 28_16
	*/
#define PRE_WDR_PARAMETER_REGISTER0_OFS 0x0940
	REGDEF_BEGIN(PRE_WDR_PARAMETER_REGISTER0)
		REGDEF_BIT(pre_wdr_coeff1,		  13)
		REGDEF_BIT( 			 ,		  3)
		REGDEF_BIT(pre_wdr_coeff2,		  13)
	REGDEF_END(PRE_WDR_PARAMETER_REGISTER0)


	/*
		pre_wdr_coeff3:    [0x0, 0x1fff],			bits : 12_0
		pre_wdr_coeff4:    [0x0, 0x1fff],			bits : 28_16
	*/
#define PRE_WDR_PARAMETER_REGISTER1_OFS 0x0944
	REGDEF_BEGIN(PRE_WDR_PARAMETER_REGISTER1)
		REGDEF_BIT(pre_wdr_coeff3,		  13)
		REGDEF_BIT( 			 ,		  3)
		REGDEF_BIT(pre_wdr_coeff4,		  13)
	REGDEF_END(PRE_WDR_PARAMETER_REGISTER1)


	/*
		pre_wdr_outbld_table_en:	[0x0, 0x1], 		bits : 0
		pre_wdr_gainctrl_en    :	[0x0, 0x1], 		bits : 1
		pre_wdr_maxgain 	   :	[0x0, 0xff],			bits : 23_16
		pre_wdr_mingain 	   :	[0x0, 0xff],			bits : 31_24
	*/
#define PRE_WDR_CONTROL_REGISTER1_OFS 0x0948
	REGDEF_BEGIN(PRE_WDR_CONTROL_REGISTER1)
		REGDEF_BIT(pre_wdr_outbld_table_en, 	   1)
		REGDEF_BIT(pre_wdr_gainctrl_en	  , 	   1)
		REGDEF_BIT( 					  , 	   14)
		REGDEF_BIT(pre_wdr_maxgain		  , 	   8)
		REGDEF_BIT(pre_wdr_mingain		  , 	   8)
	REGDEF_END(PRE_WDR_CONTROL_REGISTER1)


	/*
		pre_wdr_tcurve_val_lut_l0:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_l1:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_0_OFS 0x094c
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_0)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l0,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l1,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_0)


	/*
		pre_wdr_tcurve_val_lut_l2:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_l3:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_1_OFS 0x0950
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_1)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l2,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l3,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_1)


	/*
		pre_wdr_tcurve_val_lut_l4:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_l5:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_2_OFS 0x0954
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_2)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l4,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l5,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_2)


	/*
		pre_wdr_tcurve_val_lut_l6:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_l7:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_3_OFS 0x0958
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_3)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l6,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l7,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_3)


	/*
		pre_wdr_tcurve_val_lut_l8:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_l9:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_4_OFS 0x095c
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l8,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l9,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_4)


	/*
		pre_wdr_tcurve_val_lut_l10:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l11:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_5_OFS 0x0960
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_5)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l10,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l11,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_5)


	/*
		pre_wdr_tcurve_val_lut_l12:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l13:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_6_OFS 0x0964
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_6)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l12,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l13,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_6)


	/*
		pre_wdr_tcurve_val_lut_l14:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l15:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_7_OFS 0x0968
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_7)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l14,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l15,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_7)


	/*
		pre_wdr_tcurve_val_lut_l16:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l17:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_8_OFS 0x096c
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_8)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l16,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l17,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_8)


	/*
		pre_wdr_tcurve_val_lut_l18:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l19:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_9_OFS 0x0970
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_9)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l18,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l19,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_9)


	/*
		pre_wdr_tcurve_val_lut_l20:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l21:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_10_OFS 0x0974
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_10)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l20,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l21,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_10)


	/*
		pre_wdr_tcurve_val_lut_l22:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l23:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_11_OFS 0x0978
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_11)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l22,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l23,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_11)


	/*
		pre_wdr_tcurve_val_lut_l24:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l25:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_12_OFS 0x097c
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_12)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l24,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l25,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_12)


	/*
		pre_wdr_tcurve_val_lut_l26:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l27:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_13_OFS 0x0980
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_13)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l26,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l27,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_13)


	/*
		pre_wdr_tcurve_val_lut_l28:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l29:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_14_OFS 0x0984
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_14)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l28,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l29,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_14)


	/*
		pre_wdr_tcurve_val_lut_l30:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l31:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_15_OFS 0x0988
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_15)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l30,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l31,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_15)


	/*
		pre_wdr_tcurve_val_lut_l32:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l33:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_16_OFS 0x098c
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_16)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l32,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l33,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_16)


	/*
		pre_wdr_tcurve_val_lut_l34:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l35:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_17_OFS 0x0990
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_17)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l34,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l35,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_17)


	/*
		pre_wdr_tcurve_val_lut_l36:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l37:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_18_OFS 0x0994
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_18)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l36,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l37,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_18)


	/*
		pre_wdr_tcurve_val_lut_l38:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l39:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_19_OFS 0x0998
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_19)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l38,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l39,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_19)


	/*
		pre_wdr_tcurve_val_lut_l40:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l41:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_20_OFS 0x099c
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_20)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l40,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l41,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_20)


	/*
		pre_wdr_tcurve_val_lut_l42:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l43:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_21_OFS 0x09a0
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_21)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l42,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l43,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_21)


	/*
		pre_wdr_tcurve_val_lut_l44:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l45:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_22_OFS 0x09a4
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_22)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l44,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l45,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_22)


	/*
		pre_wdr_tcurve_val_lut_l46:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l47:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_23_OFS 0x09a8
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_23)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l46,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l47,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_23)


	/*
		pre_wdr_tcurve_val_lut_l48:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l49:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_24_OFS 0x09ac
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_24)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l48,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l49,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_24)


	/*
		pre_wdr_tcurve_val_lut_l50:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l51:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_25_OFS 0x09b0
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_25)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l50,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l51,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_25)


	/*
		pre_wdr_tcurve_val_lut_l52:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l53:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_26_OFS 0x09b4
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_26)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l52,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l53,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_26)


	/*
		pre_wdr_tcurve_val_lut_l54:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l55:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_27_OFS 0x09b8
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_27)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l54,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l55,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_27)


	/*
		pre_wdr_tcurve_val_lut_l56:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l57:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_28_OFS 0x09bc
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_28)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l56,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l57,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_28)


	/*
		pre_wdr_tcurve_val_lut_l58:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l59:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_29_OFS 0x09c0
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_29)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l58,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l59,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_29)


	/*
		pre_wdr_tcurve_val_lut_l60:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l61:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_30_OFS 0x09c4
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_30)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l60,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l61,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_30)


	/*
		pre_wdr_tcurve_val_lut_l62:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_l63:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_31_OFS 0x09c8
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_31)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l62,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l63,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_31)


	/*
		pre_wdr_tcurve_val_lut_l64:    [0x0, 0xfff],			bits : 11_0
	*/
#define PRE_WDR_TONE_CURVE_LEFT_REGISTER_32_OFS 0x09cc
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_LEFT_REGISTER_32)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_l64,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_LEFT_REGISTER_32)


	/*
		pre_wdr_tcurve_val_lut_r0:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_r1:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_RIGHT_REGISTER_0_OFS 0x09d0
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_0)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r0,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r1,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_0)


	/*
		pre_wdr_tcurve_val_lut_r2:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_r3:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_RIGHT_REGISTER_1_OFS 0x09d4
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_1)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r2,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r3,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_1)


	/*
		pre_wdr_tcurve_val_lut_r4:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_r5:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_RIGHT_REGISTER_2_OFS 0x09d8
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_2)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r4,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r5,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_2)


	/*
		pre_wdr_tcurve_val_lut_r6:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_r7:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_RIGHT_REGISTER_3_OFS 0x09dc
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_3)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r6,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r7,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_3)


	/*
		pre_wdr_tcurve_val_lut_r8:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_tcurve_val_lut_r9:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_RIGHT_REGISTER_4_OFS 0x09e0
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r8,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r9,		 12)
	REGDEF_END(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_4)


	/*
		pre_wdr_tcurve_val_lut_r10:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_r11:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_RIGHT_REGISTER_5_OFS 0x09e4
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_5)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r10,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r11,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_5)


	/*
		pre_wdr_tcurve_val_lut_r12:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_r13:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_RIGHT_REGISTER_6_OFS 0x09e8
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_6)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r12,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r13,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_6)


	/*
		pre_wdr_tcurve_val_lut_r14:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_tcurve_val_lut_r15:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_TONE_CURVE_RIGHT_REGISTER_7_OFS 0x09ec
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_7)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r14,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r15,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_7)


	/*
		pre_wdr_tcurve_val_lut_r16:    [0x0, 0xfff],			bits : 11_0
	*/
#define PRE_WDR_TONE_CURVE_RIGHT_REGISTER_8_OFS 0x09f0
	REGDEF_BEGIN(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_8)
		REGDEF_BIT(pre_wdr_tcurve_val_lut_r16,		  12)
	REGDEF_END(PRE_WDR_TONE_CURVE_RIGHT_REGISTER_8)

	/*
		pre_wdr_outbld_val_lut_l0:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_l1:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_0_OFS 0x09f8
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_0)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l0,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l1,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_0)


	/*
		pre_wdr_outbld_val_lut_l2:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_l3:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_1_OFS 0x09fc
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_1)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l2,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l3,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_1)


	/*
		pre_wdr_outbld_val_lut_l4:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_l5:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_2_OFS 0x0a00
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_2)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l4,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l5,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_2)


	/*
		pre_wdr_outbld_val_lut_l6:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_l7:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_3_OFS 0x0a04
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_3)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l6,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l7,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_3)


	/*
		pre_wdr_outbld_val_lut_l8:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_l9:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_4_OFS 0x0a08
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l8,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l9,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_4)


	/*
		pre_wdr_outbld_val_lut_l10:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l11:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_5_OFS 0x0a0c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_5)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l10,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l11,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_5)


	/*
		pre_wdr_outbld_val_lut_l12:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l13:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_6_OFS 0x0a10
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_6)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l12,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l13,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_6)


	/*
		pre_wdr_outbld_val_lut_l14:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l15:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_7_OFS 0x0a14
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_7)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l14,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l15,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_7)


	/*
		pre_wdr_outbld_val_lut_l16:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l17:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_8_OFS 0x0a18
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_8)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l16,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l17,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_8)


	/*
		pre_wdr_outbld_val_lut_l18:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l19:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_9_OFS 0x0a1c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_9)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l18,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l19,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_9)


	/*
		pre_wdr_outbld_val_lut_l20:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l21:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_10_OFS 0x0a20
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_10)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l20,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l21,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_10)


	/*
		pre_wdr_outbld_val_lut_l22:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l23:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_11_OFS 0x0a24
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_11)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l22,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l23,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_11)


	/*
		pre_wdr_outbld_val_lut_l24:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l25:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_12_OFS 0x0a28
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_12)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l24,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l25,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_12)


	/*
		pre_wdr_outbld_val_lut_l26:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l27:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_13_OFS 0x0a2c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_13)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l26,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l27,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_13)


	/*
		pre_wdr_outbld_val_lut_l28:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l29:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_14_OFS 0x0a30
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_14)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l28,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l29,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_14)


	/*
		pre_wdr_outbld_val_lut_l30:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l31:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_15_OFS 0x0a34
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_15)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l30,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l31,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_15)


	/*
		pre_wdr_outbld_val_lut_l32:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l33:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_16_OFS 0x0a38
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_16)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l32,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l33,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_16)


	/*
		pre_wdr_outbld_val_lut_l34:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l35:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_17_OFS 0x0a3c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_17)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l34,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l35,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_17)


	/*
		pre_wdr_outbld_val_lut_l36:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l37:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_18_OFS 0x0a40
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_18)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l36,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l37,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_18)


	/*
		pre_wdr_outbld_val_lut_l38:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l39:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_19_OFS 0x0a44
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_19)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l38,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l39,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_19)


	/*
		pre_wdr_outbld_val_lut_l40:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l41:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_20_OFS 0x0a48
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_20)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l40,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l41,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_20)


	/*
		pre_wdr_outbld_val_lut_l42:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l43:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_21_OFS 0x0a4c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_21)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l42,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l43,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_21)


	/*
		pre_wdr_outbld_val_lut_l44:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l45:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_22_OFS 0x0a50
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_22)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l44,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l45,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_22)


	/*
		pre_wdr_outbld_val_lut_l46:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l47:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_23_OFS 0x0a54
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_23)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l46,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l47,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_23)


	/*
		pre_wdr_outbld_val_lut_l48:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l49:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_24_OFS 0x0a58
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_24)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l48,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l49,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_24)


	/*
		pre_wdr_outbld_val_lut_l50:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l51:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_25_OFS 0x0a5c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_25)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l50,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l51,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_25)


	/*
		pre_wdr_outbld_val_lut_l52:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l53:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_26_OFS 0x0a60
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_26)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l52,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l53,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_26)


	/*
		pre_wdr_outbld_val_lut_l54:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l55:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_27_OFS 0x0a64
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_27)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l54,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l55,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_27)


	/*
		pre_wdr_outbld_val_lut_l56:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l57:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_28_OFS 0x0a68
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_28)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l56,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l57,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_28)


	/*
		pre_wdr_outbld_val_lut_l58:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l59:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_29_OFS 0x0a6c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_29)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l58,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l59,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_29)


	/*
		pre_wdr_outbld_val_lut_l60:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l61:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_30_OFS 0x0a70
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_30)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l60,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l61,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_30)


	/*
		pre_wdr_outbld_val_lut_l62:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_l63:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_31_OFS 0x0a74
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_31)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l62,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l63,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_31)


	/*
		pre_wdr_outbld_val_lut_l64:    [0x0, 0xfff],			bits : 11_0
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_32_OFS 0x0a78
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_32)
		REGDEF_BIT(pre_wdr_outbld_val_lut_l64,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_LEFT_REGISTER_32)


	/*
		pre_wdr_outbld_val_lut_r0:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_r1:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_0_OFS 0x0a7c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_0)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r0,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r1,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_0)


	/*
		pre_wdr_outbld_val_lut_r2:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_r3:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_1_OFS 0x0a80
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_1)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r2,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r3,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_1)


	/*
		pre_wdr_outbld_val_lut_r4:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_r5:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_2_OFS 0x0a84
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_2)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r4,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r5,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_2)


	/*
		pre_wdr_outbld_val_lut_r6:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_r7:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_3_OFS 0x0a88
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_3)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r6,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r7,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_3)


	/*
		pre_wdr_outbld_val_lut_r8:	  [0x0, 0xfff], 		bits : 11_0
		pre_wdr_outbld_val_lut_r9:	  [0x0, 0xfff], 		bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_4_OFS 0x0a8c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r8,		 12)
		REGDEF_BIT( 						,		 4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r9,		 12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_4)


	/*
		pre_wdr_outbld_val_lut_r10:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_r11:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_5_OFS 0x0a90
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_5)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r10,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r11,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_5)


	/*
		pre_wdr_outbld_val_lut_r12:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_r13:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_6_OFS 0x0a94
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_6)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r12,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r13,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_6)


	/*
		pre_wdr_outbld_val_lut_r14:    [0x0, 0xfff],			bits : 11_0
		pre_wdr_outbld_val_lut_r15:    [0x0, 0xfff],			bits : 27_16
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_7_OFS 0x0a98
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_7)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r14,		  12)
		REGDEF_BIT( 						 ,		  4)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r15,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_7)


	/*
		pre_wdr_outbld_val_lut_r16:    [0x0, 0xfff],			bits : 11_0
	*/
#define PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_8_OFS 0x0a9c
	REGDEF_BEGIN(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_8)
		REGDEF_BIT(pre_wdr_outbld_val_lut_r16,		  12)
	REGDEF_END(PRE_WDR_OUTPUT_BLENDING_CURVE_RIGHT_REGISTER_8)

	/*
		pre_wdr_strength_:	  [0x0, 0xff],			bits : 7_0
	*/
#define PRE_WDR_OUPUT_BLENDING_REGISTER_OFS 0x0aa4
	REGDEF_BEGIN(PRE_WDR_OUPUT_BLENDING_REGISTER)
		REGDEF_BIT(pre_wdr_strength,		8)
	REGDEF_END(PRE_WDR_OUPUT_BLENDING_REGISTER)


	/*
		pre_wdr_sat_th_    :	[0x0, 0xfff],			bits : 11_0
		pre_wdr_sat_wt_low_:	[0x0, 0xff],			bits : 23_16
		pre_wdr_sat_delta_ :	[0x0, 0xff],			bits : 31_24
	*/
#define PRE_WDR_SATURATION_REDUCTION_REGISTER_OFS 0x0aa8
	REGDEF_BEGIN(PRE_WDR_SATURATION_REDUCTION_REGISTER)
		REGDEF_BIT(pre_wdr_sat_th	  , 	   12)
		REGDEF_BIT( 				  , 	   4)
		REGDEF_BIT(pre_wdr_sat_wt_low , 	   8)
		REGDEF_BIT(pre_wdr_sat_delta  , 	   8)
	REGDEF_END(PRE_WDR_SATURATION_REDUCTION_REGISTER)


	/*
		pre_wdr_mode		 :	  [0x0, 0x1],			bits : 0
		pre_wdr_anti_halo_opt:	  [0x0, 0x1],			bits : 1
		pre_wdr_halo_ratio	 :	  [0x0, 0xff],			bits : 15_8
		pre_wdr_halo_slope	 :	  [0x0, 0xff],			bits : 23_16
	*/
#define PRE_WDR_TONE_MAPPING_REGISTER_0_OFS 0x0aac
	REGDEF_BEGIN(PRE_WDR_TONE_MAPPING_REGISTER_0)
		REGDEF_BIT(pre_wdr_mode 		,		 1)
		REGDEF_BIT(pre_wdr_anti_halo_opt,		 1)
		REGDEF_BIT( 					,		 6)
		REGDEF_BIT(pre_wdr_halo_ratio	,		 8)
		REGDEF_BIT(pre_wdr_halo_slope	,		 8)
	REGDEF_END(PRE_WDR_TONE_MAPPING_REGISTER_0)


	/*
		pre_wdr_b2p_var:	[0x0, 0xfff],			bits : 11_0
	*/
#define PRE_WDR_TONE_MAPPING_REGISTER_1_OFS 0x0ab0
	REGDEF_BEGIN(PRE_WDR_TONE_MAPPING_REGISTER_1)
		REGDEF_BIT(pre_wdr_b2p_var, 	   12)
	REGDEF_END(PRE_WDR_TONE_MAPPING_REGISTER_1)


	/*
		pre_wdr_histogram_h_step_:	  [0x0, 0x1f],			bits : 4_0
		pre_wdr_histogram_v_step_:	  [0x0, 0x1f],			bits : 20_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER0_OFS 0x0ab4
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER0)
		REGDEF_BIT(pre_wdr_histogram_h_step ,		 5)
		REGDEF_BIT( 						,		 11)
		REGDEF_BIT(pre_wdr_histogram_v_step ,		 5)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER0)


	/*
		pre_wdr_histogram_bin0:    [0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin1:    [0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_0_OFS 0x0ab8
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_0)
		REGDEF_BIT(pre_wdr_histogram_bin0,		  16)
		REGDEF_BIT(pre_wdr_histogram_bin1,		  16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_0)


	/*
		pre_wdr_histogram_bin2:    [0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin3:    [0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_1_OFS 0x0abc
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_1)
		REGDEF_BIT(pre_wdr_histogram_bin2,		  16)
		REGDEF_BIT(pre_wdr_histogram_bin3,		  16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_1)


	/*
		pre_wdr_histogram_bin4:    [0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin5:    [0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_2_OFS 0x0ac0
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_2)
		REGDEF_BIT(pre_wdr_histogram_bin4,		  16)
		REGDEF_BIT(pre_wdr_histogram_bin5,		  16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_2)


	/*
		pre_wdr_histogram_bin6:    [0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin7:    [0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_3_OFS 0x0ac4
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_3)
		REGDEF_BIT(pre_wdr_histogram_bin6,		  16)
		REGDEF_BIT(pre_wdr_histogram_bin7,		  16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_3)


	/*
		pre_wdr_histogram_bin8:    [0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin9:    [0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_4_OFS 0x0ac8
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_4)
		REGDEF_BIT(pre_wdr_histogram_bin8,		  16)
		REGDEF_BIT(pre_wdr_histogram_bin9,		  16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_4)


	/*
		pre_wdr_histogram_bin10:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin11:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_5_OFS 0x0acc
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_5)
		REGDEF_BIT(pre_wdr_histogram_bin10, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin11, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_5)


	/*
		pre_wdr_histogram_bin12:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin13:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_6_OFS 0x0ad0
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_6)
		REGDEF_BIT(pre_wdr_histogram_bin12, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin13, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_6)


	/*
		pre_wdr_histogram_bin14:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin15:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_7_OFS 0x0ad4
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_7)
		REGDEF_BIT(pre_wdr_histogram_bin14, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin15, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_7)


	/*
		pre_wdr_histogram_bin16:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin17:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_8_OFS 0x0ad8
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_8)
		REGDEF_BIT(pre_wdr_histogram_bin16, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin17, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_8)


	/*
		pre_wdr_histogram_bin18:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin19:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_9_OFS 0x0adc
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_9)
		REGDEF_BIT(pre_wdr_histogram_bin18, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin19, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_9)


	/*
		pre_wdr_histogram_bin20:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin21:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_10_OFS 0x0ae0
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_10)
		REGDEF_BIT(pre_wdr_histogram_bin20, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin21, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_10)


	/*
		pre_wdr_histogram_bin22:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin23:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_11_OFS 0x0ae4
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_11)
		REGDEF_BIT(pre_wdr_histogram_bin22, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin23, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_11)


	/*
		pre_wdr_histogram_bin24:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin25:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_12_OFS 0x0ae8
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_12)
		REGDEF_BIT(pre_wdr_histogram_bin24, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin25, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_12)


	/*
		pre_wdr_histogram_bin26:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin27:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_13_OFS 0x0aec
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_13)
		REGDEF_BIT(pre_wdr_histogram_bin26, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin27, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_13)


	/*
		pre_wdr_histogram_bin28:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin29:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_14_OFS 0x0af0
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_14)
		REGDEF_BIT(pre_wdr_histogram_bin28, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin29, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_14)


	/*
		pre_wdr_histogram_bin30:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin31:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_15_OFS 0x0af4
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_15)
		REGDEF_BIT(pre_wdr_histogram_bin30, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin31, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_15)


	/*
		pre_wdr_histogram_bin32:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin33:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_16_OFS 0x0af8
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_16)
		REGDEF_BIT(pre_wdr_histogram_bin32, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin33, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_16)


	/*
		pre_wdr_histogram_bin34:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin35:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_17_OFS 0x0afc
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_17)
		REGDEF_BIT(pre_wdr_histogram_bin34, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin35, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_17)


	/*
		pre_wdr_histogram_bin36:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin37:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_18_OFS 0x0b00
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_18)
		REGDEF_BIT(pre_wdr_histogram_bin36, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin37, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_18)


	/*
		pre_wdr_histogram_bin38:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin39:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_19_OFS 0x0b04
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_19)
		REGDEF_BIT(pre_wdr_histogram_bin38, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin39, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_19)


	/*
		pre_wdr_histogram_bin40:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin41:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_20_OFS 0x0b08
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_20)
		REGDEF_BIT(pre_wdr_histogram_bin40, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin41, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_20)


	/*
		pre_wdr_histogram_bin42:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin43:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_21_OFS 0x0b0c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_21)
		REGDEF_BIT(pre_wdr_histogram_bin42, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin43, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_21)


	/*
		pre_wdr_histogram_bin44:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin45:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_22_OFS 0x0b10
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_22)
		REGDEF_BIT(pre_wdr_histogram_bin44, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin45, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_22)


	/*
		pre_wdr_histogram_bin46:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin47:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_23_OFS 0x0b14
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_23)
		REGDEF_BIT(pre_wdr_histogram_bin46, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin47, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_23)


	/*
		pre_wdr_histogram_bin48:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin49:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_24_OFS 0x0b18
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_24)
		REGDEF_BIT(pre_wdr_histogram_bin48, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin49, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_24)


	/*
		pre_wdr_histogram_bin50:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin51:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_25_OFS 0x0b1c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_25)
		REGDEF_BIT(pre_wdr_histogram_bin50, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin51, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_25)


	/*
		pre_wdr_histogram_bin52:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin53:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_26_OFS 0x0b20
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_26)
		REGDEF_BIT(pre_wdr_histogram_bin52, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin53, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_26)


	/*
		pre_wdr_histogram_bin54:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin55:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_27_OFS 0x0b24
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_27)
		REGDEF_BIT(pre_wdr_histogram_bin54, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin55, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_27)


	/*
		pre_wdr_histogram_bin56:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin57:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_28_OFS 0x0b28
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_28)
		REGDEF_BIT(pre_wdr_histogram_bin56, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin57, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_28)


	/*
		pre_wdr_histogram_bin58:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin59:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_29_OFS 0x0b2c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_29)
		REGDEF_BIT(pre_wdr_histogram_bin58, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin59, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_29)


	/*
		pre_wdr_histogram_bin60:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin61:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_30_OFS 0x0b30
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_30)
		REGDEF_BIT(pre_wdr_histogram_bin60, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin61, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_30)


	/*
		pre_wdr_histogram_bin62:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin63:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_31_OFS 0x0b34
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_31)
		REGDEF_BIT(pre_wdr_histogram_bin62, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin63, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_31)


	/*
		pre_wdr_histogram_bin64:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin65:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_32_OFS 0x0b38
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_32)
		REGDEF_BIT(pre_wdr_histogram_bin64, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin65, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_32)


	/*
		pre_wdr_histogram_bin66:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin67:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_33_OFS 0x0b3c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_33)
		REGDEF_BIT(pre_wdr_histogram_bin66, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin67, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_33)


	/*
		pre_wdr_histogram_bin68:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin69:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_34_OFS 0x0b40
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_34)
		REGDEF_BIT(pre_wdr_histogram_bin68, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin69, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_34)


	/*
		pre_wdr_histogram_bin70:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin71:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_35_OFS 0x0b44
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_35)
		REGDEF_BIT(pre_wdr_histogram_bin70, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin71, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_35)


	/*
		pre_wdr_histogram_bin72:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin73:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_36_OFS 0x0b48
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_36)
		REGDEF_BIT(pre_wdr_histogram_bin72, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin73, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_36)


	/*
		pre_wdr_histogram_bin74:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin75:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_37_OFS 0x0b4c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_37)
		REGDEF_BIT(pre_wdr_histogram_bin74, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin75, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_37)


	/*
		pre_wdr_histogram_bin76:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin77:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_38_OFS 0x0b50
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_38)
		REGDEF_BIT(pre_wdr_histogram_bin76, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin77, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_38)


	/*
		pre_wdr_histogram_bin78:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin79:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_39_OFS 0x0b54
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_39)
		REGDEF_BIT(pre_wdr_histogram_bin78, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin79, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_39)


	/*
		pre_wdr_histogram_bin80:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin81:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_40_OFS 0x0b58
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_40)
		REGDEF_BIT(pre_wdr_histogram_bin80, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin81, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_40)


	/*
		pre_wdr_histogram_bin82:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin83:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_41_OFS 0x0b5c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_41)
		REGDEF_BIT(pre_wdr_histogram_bin82, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin83, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_41)


	/*
		pre_wdr_histogram_bin84:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin85:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_42_OFS 0x0b60
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_42)
		REGDEF_BIT(pre_wdr_histogram_bin84, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin85, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_42)


	/*
		pre_wdr_histogram_bin86:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin87:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_43_OFS 0x0b64
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_43)
		REGDEF_BIT(pre_wdr_histogram_bin86, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin87, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_43)


	/*
		pre_wdr_histogram_bin88:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin89:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_44_OFS 0x0b68
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_44)
		REGDEF_BIT(pre_wdr_histogram_bin88, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin89, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_44)


	/*
		pre_wdr_histogram_bin90:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin91:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_45_OFS 0x0b6c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_45)
		REGDEF_BIT(pre_wdr_histogram_bin90, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin91, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_45)


	/*
		pre_wdr_histogram_bin92:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin93:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_46_OFS 0x0b70
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_46)
		REGDEF_BIT(pre_wdr_histogram_bin92, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin93, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_46)


	/*
		pre_wdr_histogram_bin94:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin95:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_47_OFS 0x0b74
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_47)
		REGDEF_BIT(pre_wdr_histogram_bin94, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin95, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_47)


	/*
		pre_wdr_histogram_bin96:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin97:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_48_OFS 0x0b78
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_48)
		REGDEF_BIT(pre_wdr_histogram_bin96, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin97, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_48)


	/*
		pre_wdr_histogram_bin98:	[0x0, 0xffff],			bits : 15_0
		pre_wdr_histogram_bin99:	[0x0, 0xffff],			bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_49_OFS 0x0b7c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_49)
		REGDEF_BIT(pre_wdr_histogram_bin98, 	   16)
		REGDEF_BIT(pre_wdr_histogram_bin99, 	   16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_49)


	/*
		pre_wdr_histogram_bin100:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin101:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_50_OFS 0x0b80
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_50)
		REGDEF_BIT(pre_wdr_histogram_bin100,		16)
		REGDEF_BIT(pre_wdr_histogram_bin101,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_50)


	/*
		pre_wdr_histogram_bin102:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin103:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_51_OFS 0x0b84
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_51)
		REGDEF_BIT(pre_wdr_histogram_bin102,		16)
		REGDEF_BIT(pre_wdr_histogram_bin103,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_51)


	/*
		pre_wdr_histogram_bin104:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin105:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_52_OFS 0x0b88
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_52)
		REGDEF_BIT(pre_wdr_histogram_bin104,		16)
		REGDEF_BIT(pre_wdr_histogram_bin105,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_52)


	/*
		pre_wdr_histogram_bin106:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin107:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_53_OFS 0x0b8c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_53)
		REGDEF_BIT(pre_wdr_histogram_bin106,		16)
		REGDEF_BIT(pre_wdr_histogram_bin107,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_53)


	/*
		pre_wdr_histogram_bin108:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin109:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_54_OFS 0x0b90
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_54)
		REGDEF_BIT(pre_wdr_histogram_bin108,		16)
		REGDEF_BIT(pre_wdr_histogram_bin109,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_54)


	/*
		pre_wdr_histogram_bin110:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin111:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_55_OFS 0x0b94
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_55)
		REGDEF_BIT(pre_wdr_histogram_bin110,		16)
		REGDEF_BIT(pre_wdr_histogram_bin111,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_55)


	/*
		pre_wdr_histogram_bin112:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin113:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_56_OFS 0x0b98
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_56)
		REGDEF_BIT(pre_wdr_histogram_bin112,		16)
		REGDEF_BIT(pre_wdr_histogram_bin113,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_56)


	/*
		pre_wdr_histogram_bin114:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin115:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_57_OFS 0x0b9c
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_57)
		REGDEF_BIT(pre_wdr_histogram_bin114,		16)
		REGDEF_BIT(pre_wdr_histogram_bin115,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_57)


	/*
		pre_wdr_histogram_bin116:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin117:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_58_OFS 0x0ba0
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_58)
		REGDEF_BIT(pre_wdr_histogram_bin116,		16)
		REGDEF_BIT(pre_wdr_histogram_bin117,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_58)


	/*
		pre_wdr_histogram_bin118:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin119:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_59_OFS 0x0ba4
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_59)
		REGDEF_BIT(pre_wdr_histogram_bin118,		16)
		REGDEF_BIT(pre_wdr_histogram_bin119,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_59)


	/*
		pre_wdr_histogram_bin120:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin121:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_60_OFS 0x0ba8
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_60)
		REGDEF_BIT(pre_wdr_histogram_bin120,		16)
		REGDEF_BIT(pre_wdr_histogram_bin121,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_60)


	/*
		pre_wdr_histogram_bin122:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin123:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_61_OFS 0x0bac
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_61)
		REGDEF_BIT(pre_wdr_histogram_bin122,		16)
		REGDEF_BIT(pre_wdr_histogram_bin123,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_61)


	/*
		pre_wdr_histogram_bin124:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin125:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_62_OFS 0x0bb0
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_62)
		REGDEF_BIT(pre_wdr_histogram_bin124,		16)
		REGDEF_BIT(pre_wdr_histogram_bin125,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_62)


	/*
		pre_wdr_histogram_bin126:	 [0x0, 0xffff], 		bits : 15_0
		pre_wdr_histogram_bin127:	 [0x0, 0xffff], 		bits : 31_16
	*/
#define PRE_WDR_HISTOGRAM_REGISTER_63_OFS 0x0bb4
	REGDEF_BEGIN(PRE_WDR_HISTOGRAM_REGISTER_63)
		REGDEF_BIT(pre_wdr_histogram_bin126,		16)
		REGDEF_BIT(pre_wdr_histogram_bin127,		16)
	REGDEF_END(PRE_WDR_HISTOGRAM_REGISTER_63)


	/*
		pre_wdr_fbc_en :	[0x0, 0x1], 		bits : 0
		pre_wdr_fbc_rto:	[0x0, 0xff],			bits : 15_8
		pre_wdr_fbc_th0:	[0x0, 0x1fff],			bits : 28_16
	*/
#define PRE_WDR_FBC_REGISTER_0_OFS 0x0bb8
	REGDEF_BEGIN(PRE_WDR_FBC_REGISTER_0)
		REGDEF_BIT(pre_wdr_fbc_en , 	   1)
		REGDEF_BIT( 			  , 	   7)
		REGDEF_BIT(pre_wdr_fbc_rto, 	   8)
		REGDEF_BIT(pre_wdr_fbc_th0, 	   13)
	REGDEF_END(PRE_WDR_FBC_REGISTER_0)


	/*
		pre_wdr_fbc_th1:	[0x0, 0x1fff],			bits : 12_0
		pre_wdr_fbc_th2:	[0x0, 0x1fff],			bits : 28_16
	*/
#define PRE_WDR_FBC_REGISTER_1_OFS 0x0bbc
	REGDEF_BEGIN(PRE_WDR_FBC_REGISTER_1)
		REGDEF_BIT(pre_wdr_fbc_th1, 	   13)
		REGDEF_BIT( 			  , 	   3)
		REGDEF_BIT(pre_wdr_fbc_th2, 	   13)
	REGDEF_END(PRE_WDR_FBC_REGISTER_1)


	/*
		pre_wdr_fbc_th3:	[0x0, 0x1fff],			bits : 12_0
		pre_wdr_fbc_th4:	[0x0, 0x1fff],			bits : 28_16
	*/
#define PRE_WDR_FBC_REGISTER_2_OFS 0x0bc0
	REGDEF_BEGIN(PRE_WDR_FBC_REGISTER_2)
		REGDEF_BIT(pre_wdr_fbc_th3, 	   13)
		REGDEF_BIT( 			  , 	   3)
		REGDEF_BIT(pre_wdr_fbc_th4, 	   13)
	REGDEF_END(PRE_WDR_FBC_REGISTER_2)


	/*
		pre_wdr_fbc_th5:	[0x0, 0x1fff],			bits : 12_0
	*/
#define PRE_WDR_FBC_REGISTER_3_OFS 0x0bc4
	REGDEF_BEGIN(PRE_WDR_FBC_REGISTER_3)
		REGDEF_BIT(pre_wdr_fbc_th5, 	   13)
	REGDEF_END(PRE_WDR_FBC_REGISTER_3)


	/*
		pre_wdr_input_yv_bld_lut0:	  [0x0, 0x3f],			bits : 5_0
		pre_wdr_input_yv_bld_lut1:	  [0x0, 0x3f],			bits : 13_8
		pre_wdr_input_yv_bld_lut2:	  [0x0, 0x3f],			bits : 21_16
		pre_wdr_input_yv_bld_lut3:	  [0x0, 0x3f],			bits : 29_24
	*/
#define PRE_WDR_YV_BLD_REGISTER_0_OFS 0x0bc8
	REGDEF_BEGIN(PRE_WDR_YV_BLD_REGISTER_0)
		REGDEF_BIT(pre_wdr_input_yv_bld_lut0,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(pre_wdr_input_yv_bld_lut1,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(pre_wdr_input_yv_bld_lut2,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(pre_wdr_input_yv_bld_lut3,		 6)
	REGDEF_END(PRE_WDR_YV_BLD_REGISTER_0)


	/*
		pre_wdr_input_yv_bld_lut4:	  [0x0, 0x3f],			bits : 5_0
		pre_wdr_input_yv_bld_lut5:	  [0x0, 0x3f],			bits : 13_8
		pre_wdr_input_yv_bld_lut6:	  [0x0, 0x3f],			bits : 21_16
		pre_wdr_input_yv_bld_lut7:	  [0x0, 0x3f],			bits : 29_24
	*/
#define PRE_WDR_YV_BLD_REGISTER_1_OFS 0x0bcc
	REGDEF_BEGIN(PRE_WDR_YV_BLD_REGISTER_1)
		REGDEF_BIT(pre_wdr_input_yv_bld_lut4,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(pre_wdr_input_yv_bld_lut5,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(pre_wdr_input_yv_bld_lut6,		 6)
		REGDEF_BIT( 						,		 2)
		REGDEF_BIT(pre_wdr_input_yv_bld_lut7,		 6)
	REGDEF_END(PRE_WDR_YV_BLD_REGISTER_1)


	/*
		pre_wdr_input_yv_bld_lut8:	  [0x0, 0x3f],			bits : 5_0
	*/
#define PRE_WDR_YV_BLD_REGISTER_2_OFS 0x0bd0
	REGDEF_BEGIN(PRE_WDR_YV_BLD_REGISTER_2)
		REGDEF_BIT(pre_wdr_input_yv_bld_lut8,		 6)
	REGDEF_END(PRE_WDR_YV_BLD_REGISTER_2)










#define NN_ISP_REGISTER0_OFS 0x0D40
	REGDEF_BEGIN(NN_ISP_REGISTER0)
	REGDEF_BIT(pre_nn_isp_p0_en, 1) 							// bits , 0
	REGDEF_BIT(pre_nn_isp_p1_en, 1) 							// bits , 1
	REGDEF_BIT(pre_nn_isp_p0_handshake_mode, 1) 				// bits , 2
	REGDEF_BIT(pre_nn_isp_p1_handshake_mode, 1) 				// bits , 3
	//REGDEF_BIT(pre_nn_isp_p0_ringbuf_slice_clear	  , 1)		  // bits , 4
	//REGDEF_BIT(pre_nn_isp_p0_outbuf_slice_ready	  , 1)		  // bits , 5
	//REGDEF_BIT(pre_nn_isp_p1_ringbuf_slice_clear	  , 1)		  // bits , 6
	//REGDEF_BIT(pre_nn_isp_p1_outbuf_slice_ready	  , 1)		  // bits , 7
	REGDEF_BIT(, 4)
	REGDEF_BIT(pre_nn_isp_p0_input_burst_mode, 1)				// bits , 8
	REGDEF_BIT(pre_nn_isp_p0_output_burst_mode, 1)				// bits , 9
	REGDEF_BIT(pre_nn_isp_p1_input_burst_mode, 1)				// bits , 10
	REGDEF_BIT(pre_nn_isp_p1_output_burst_mode, 1)				// bits , 11
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
	REGDEF_BIT(pre_nn_isp_p0_set_slice_ready_status, 1) 			  // bits , 0
	REGDEF_BIT(pre_nn_isp_p0_get_slice_clear_status, 1) 				// bits , 1
	REGDEF_BIT(pre_nn_isp_p1_set_slice_ready_status, 1) 				// bits , 2
	REGDEF_BIT(pre_nn_isp_p1_get_slice_clear_status, 1) 				// bits , 3
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
	REGDEF_BIT(pre_nn_isp_p0_ringbuf_slice_clear, 1)			   // bits , 27_24
	REGDEF_END(NN_ISP_REGISTER15)

#define NN_ISP_REGISTER16_OFS 0x0D80
	REGDEF_BEGIN(NN_ISP_REGISTER16)
	REGDEF_BIT(, 31)
	REGDEF_BIT(pre_nn_isp_p0_outbuf_slice_ready, 1) 			  // bits , 27_24
	REGDEF_END(NN_ISP_REGISTER16)

#define NN_ISP_REGISTER17_OFS 0x0D84
	REGDEF_BEGIN(NN_ISP_REGISTER17)
	REGDEF_BIT(, 31)
	REGDEF_BIT(pre_nn_isp_p1_ringbuf_slice_clear, 1)			   // bits , 27_24
	REGDEF_END(NN_ISP_REGISTER17)

#define NN_ISP_REGISTER18_OFS 0x0D88
	REGDEF_BEGIN(NN_ISP_REGISTER18)
	REGDEF_BIT(, 31)
	REGDEF_BIT(pre_nn_isp_p1_outbuf_slice_ready, 1) 			  // bits , 27_24
	REGDEF_END(NN_ISP_REGISTER18)
#endif



#endif

