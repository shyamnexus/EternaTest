/**
 * @file graph_drv_uti.c
 *  graph drv utility
 *
 *
 */
#if defined(__LINUX)
#include <linux/module.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <plat/nvt_jiffies.h>
#endif
#include "grph_drv_dbg_int.h"
#include "grph_drv_util_int.h"
#include "comm/hwclock.h"
#include <kwrap/perf.h>

#define ENG_MAP(eng0, eng1, eng2) (eng0 | (eng1 << 1) | (eng2 << 2))

struct cmd_supt_map_info {
	GRPH_CMD cmd;
	unsigned int eng; 	//[0]:eng0 [1]:eng1 [2]:eng2....
};

struct val_str_map_info {
	unsigned int val;
	char *str;
};

const static struct val_str_map_info sts_str_tab[GRAPH_DRV_STS_MAX] = {
	{GRAPH_DRV_STS_QUEUE, "QUEUE"},
	{GRAPH_DRV_STS_PROC, "PROC"},
	{GRAPH_DRV_STS_PROC_END, "PEND"},
	{GRAPH_DRV_STS_DONE, "DONE"},
	{GRAPH_DRV_STS_FLUSH, "FLUSH"},
	{GRAPH_DRV_STS_ERROR, "ERR"},
	{GRAPH_DRV_STS_IDLE, "IDLE"},
	{GRAPH_DRV_STS_OK, "OK"},
	{GRAPH_DRV_STS_QFULL, "QFULL"},

};
const char* graph_drv_uti_get_sts_str(unsigned int val)
{
	int i;

	for (i = 0; i < GRAPH_DRV_STS_MAX; i ++) {
		if (sts_str_tab[i].val == val) {
			return sts_str_tab[i].str;
		}
	}
	return 0;
}

const static struct val_str_map_info cmd_str_tab[] = {
	{GRPH_CMD_ROT_90, "rot90"},
	{GRPH_CMD_ROT_270, "rot270"},
	{GRPH_CMD_ROT_180, "rot180"},
	{GRPH_CMD_HRZ_FLIP, "Hflip"},
	{GRPH_CMD_VTC_FLIP, "Vflip"},
	{GRPH_CMD_HRZ_FLIP_ROT_90, "Hflip_rot90"},
	{GRPH_CMD_HRZ_FLIP_ROT_270, "Hflip_rot270"},
	{GRPH_CMD_ROT_0, "rot0"},
	{GRPH_CMD_VCOV, "vcov"},
	{GRPH_CMD_Draw_line, "dline"},
	{GRPH_CMD_A_COPY, "a_cpy"},
	{GRPH_CMD_PLUS_SHF, "plus_shf"},
	{GRPH_CMD_MINUS_SHF, "minus_shf"},
	{GRPH_CMD_COLOR_EQ, "clr_eq"},
	{GRPH_CMD_COLOR_LE, "clr_le"},
	{GRPH_CMD_A_AND_B, "a_and_b"},
	{GRPH_CMD_A_OR_B, "a_or_b"},
	{GRPH_CMD_A_XOR_B, "a_xor_b"},
	{GRPH_CMD_TEXT_COPY, "txt_cpy"},
	{GRPH_CMD_TEXT_AND_A, "txt_and_a"},
	{GRPH_CMD_TEXT_OR_A, "txt_or_a"},
	{GRPH_CMD_TEXT_XOR_A, "txt_xor_a"},
	{GRPH_CMD_TEXT_AND_AB, "txt_and_ab"},
	{GRPH_CMD_BLENDING, "blend"},
	{GRPH_CMD_ACC, "acc"},
	{GRPH_CMD_MULTIPLY_DIV, "mult_div"},
	{GRPH_CMD_PACKING, "pack"},
	{GRPH_CMD_DEPACKING, "depack"},
	{GRPH_CMD_TEXT_MUL, "txt_mul"},
	{GRPH_CMD_PLANE_BLENDING, "p_blend"},
	{GRPH_CMD_1D_LUT, "1dlut"},
	{GRPH_CMD_2D_LUT, "2dlut"},
	{GRPH_CMD_RGBYUV_BLEND, "rgbyuv_blend"},
	{GRPH_CMD_RGBYUV_COLORKEY, "rgbyuv_clrkey"},
	{GRPH_CMD_RGB_INVERT, "rgb_inv"},
	{GRPH_CMD_YCC_ENC, "ycc_enc"},
	{GRPH_CMD_YCC_DEC, "ycc_dec"},
	{GRPH_CMD_MINUS_SHF_ABS, "minus_shf_abs"},
	{GRPH_CMD_COLOR_MR, "clr_mr"},
	{GRPH_CMD_COLOR_FILTER, "clr_filter"},
};
const char* graph_drv_uti_get_cmd_str(unsigned int val)
{
	int i;
	int total_num;

	total_num = sizeof(cmd_str_tab)/sizeof(struct val_str_map_info);
	for (i = 0; i < total_num; i ++) {
		if (cmd_str_tab[i].val == val) {
			return cmd_str_tab[i].str;
		}
	}
	return 0;
}

const static struct val_str_map_info fmt_str_tab[] = {
	{GRPH_FORMAT_1BIT, "1bit"},
	{GRPH_FORMAT_4BITS, "4bit"},
	{GRPH_FORMAT_8BITS, "8bit"},
	{GRPH_FORMAT_16BITS, "16bit"},
	{GRPH_FORMAT_16BITS_UVPACK, "uvpack"},
	{GRPH_FORMAT_16BITS_UVPACK_U, "uvpack_u"},
	{GRPH_FORMAT_16BITS_UVPACK_V, "uvpack_v"},
	{GRPH_FORMAT_16BITS_RGB565, "RGB565"},
	{GRPH_FORMAT_32BITS, "32bit"},
	{GRPH_FORMAT_32BITS_ARGB8888_RGB, "ARGB8888_RGB"},
	{GRPH_FORMAT_32BITS_ARGB8888_A, "ARGB8888_A"},
	{GRPH_FORMAT_16BITS_ARGB1555_RGB, "ARGB1555_RGB"},
	{GRPH_FORMAT_16BITS_ARGB1555_A, "ARGB1555_A"},
	{GRPH_FORMAT_16BITS_ARGB4444_RGB, "ARGB4444_RGB"},
	{GRPH_FORMAT_16BITS_ARGB4444_A, "ARGB1555_A"},
	{GRPH_FORMAT_PALETTE_1BIT, "pal1"},
	{GRPH_FORMAT_PALETTE_2BITS, "pal1"},
	{GRPH_FORMAT_PALETTE_4BITS, "pal4"},
};
const char* graph_drv_uti_get_fmt_str(unsigned int val)
{
	int i;
	int total_num;

	total_num = sizeof(fmt_str_tab)/sizeof(struct val_str_map_info);
	for (i = 0; i < total_num; i ++) {
		if (fmt_str_tab[i].val == val) {
			return fmt_str_tab[i].str;
		}
	}
	return 0;
}

const static struct val_str_map_info img_id_str_tab[] = {
	{GRPH_IMG_ID_A, "imgA"},
	{GRPH_IMG_ID_B, "imgB"},
	{GRPH_IMG_ID_C, "imgC"},
	{GRPH_IMG_ID_D, "imgD"},
};
const char* graph_drv_uti_get_img_id_str(unsigned int val)
{
	int i;
	int total_num;

	total_num = sizeof(img_id_str_tab)/sizeof(struct val_str_map_info);
	for (i = 0; i < total_num; i ++) {
		if (img_id_str_tab[i].val == val) {
			return img_id_str_tab[i].str;
		}
	}
	return 0;
}

const static struct val_str_map_info io_id_str_tab[] = {
	{GRPH_INOUT_ID_IN_A, "inA"},
	{GRPH_INOUT_ID_IN_A_U, "inAU"},
	{GRPH_INOUT_ID_IN_A_V, "inAV"},
	{GRPH_INOUT_ID_IN_B, "inB"},
	{GRPH_INOUT_ID_IN_B_U, "inBU"},
	{GRPH_INOUT_ID_IN_B_V, "inBV"},
	{GRPH_INOUT_ID_OUT_C, "outC"},
	{GRPH_INOUT_ID_OUT_C_U, "outCU"},
	{GRPH_INOUT_ID_OUT_C_V, "outCV"},

};
const char* graph_drv_uti_get_io_id_str(unsigned int val)
{
	int i;
	int total_num;

	total_num = sizeof(io_id_str_tab)/sizeof(struct val_str_map_info);
	for (i = 0; i < total_num; i ++) {
		if (io_id_str_tab[i].val == val) {
			return io_id_str_tab[i].str;
		}
	}
	return 0;
}

const static struct val_str_map_info pty_str_tab[] = {
	{GRPH_PROPERTY_ID_NORMAL, "norm"},
	{GRPH_PROPERTY_ID_U, "u"},
	{GRPH_PROPERTY_ID_V, "v"},
	{GRPH_PROPERTY_ID_R, "r"},
	{GRPH_PROPERTY_ID_G, "g"},
	{GRPH_PROPERTY_ID_B, "b"},
	{GRPH_PROPERTY_ID_A, "a"},
	{GRPH_PROPERTY_ID_ACC_SKIPCTRL, "acc_skip"},
	{GRPH_PROPERTY_ID_ACC_FULL_FLAG, "acc_full"},
	{GRPH_PROPERTY_ID_PIXEL_CNT, "pix_cnt"},
	{GRPH_PROPERTY_ID_VALID_PIXEL_CNT, "valid_pix_cnt"},
	{GRPH_PROPERTY_ID_ACC_RESULT, "acc_rlt"},
	{GRPH_PROPERTY_ID_ACC_RESULT2, "acc_rlt2"},
	{GRPH_PROPERTY_ID_LUT_BUF, "lut_buf"},
	{GRPH_PROPERTY_ID_YUVFMT, "yuvfmt"},
	{GRPH_PROPERTY_ID_ALPHA0_INDEX, "alpha0"},
	{GRPH_PROPERTY_ID_ALPHA1_INDEX, "alpha1"},
	{GRPH_PROPERTY_ID_INVRGB, "inv_rgb"},
	{GRPH_PROPERTY_ID_PAL_BUF, "pal_buf"},
	{GRPH_PROPERTY_ID_QUAD_PTR, "quad_o_ptr"},
	{GRPH_PROPERTY_ID_QUAD_INNER_PTR, "quad_i_ptr"},
	{GRPH_PROPERTY_ID_MOSAIC_SRC_FMT, "mosaic_src_fmt"},
	{GRPH_PROPERTY_ID_UV_SUBSAMPLE, "uv_subsample"},
	{GRPH_PROPERTY_ID_GOP_DIR, "gop_dir"},
	{GRPH_PROPERTY_ID_YCC_EN, "ycc_en"},
	{GRPH_PROPERTY_ID_MB_EN, "mb_en"},
};
const char* graph_drv_uti_get_pty_str(unsigned int val)
{
	int i;
	int total_num;

	total_num = sizeof(pty_str_tab)/sizeof(struct val_str_map_info);
	for (i = 0; i < total_num; i ++) {
		if (pty_str_tab[i].val == val) {
			return pty_str_tab[i].str;
		}
	}
	return 0;
}

#if 0 //don't check eng spec
const static struct cmd_supt_map_info cmd_eng_supt_tab[] = {
	{GRPH_CMD_ROT_90, 			ENG_MAP(0, 0, 1)},
	{GRPH_CMD_ROT_270, 			ENG_MAP(0, 0, 1)},
	{GRPH_CMD_ROT_180, 			ENG_MAP(0, 0, 1)},
	{GRPH_CMD_HRZ_FLIP,			ENG_MAP(0, 0, 1)},
	{GRPH_CMD_VTC_FLIP, 		ENG_MAP(0, 0, 1)},
	{GRPH_CMD_HRZ_FLIP_ROT_90, 	ENG_MAP(0, 0, 1)},
	{GRPH_CMD_HRZ_FLIP_ROT_270, ENG_MAP(0, 0, 1)},
	{GRPH_CMD_ROT_0, 			ENG_MAP(0, 0, 0)},
	{GRPH_CMD_VCOV, 			ENG_MAP(0, 1, 0)},
	{GRPH_CMD_Draw_line, 		ENG_MAP(0, 1, 0)},
	//GRPH_CMD_GOPMAX
	{GRPH_CMD_A_COPY, 			ENG_MAP(1, 1, 1)},
	{GRPH_CMD_PLUS_SHF, 		ENG_MAP(1, 0, 0)},
	{GRPH_CMD_MINUS_SHF, 		ENG_MAP(1, 0, 0)},
	{GRPH_CMD_COLOR_EQ, 		ENG_MAP(1, 1, 0)},
	{GRPH_CMD_COLOR_LE, 		ENG_MAP(1, 1, 0)},
	{GRPH_CMD_A_AND_B, 			ENG_MAP(1, 0, 0)},
	{GRPH_CMD_A_OR_B, 			ENG_MAP(1, 0, 0)},
	{GRPH_CMD_A_XOR_B, 			ENG_MAP(1, 0, 0)},
	{GRPH_CMD_TEXT_COPY, 		ENG_MAP(1, 1, 1)},
	{GRPH_CMD_TEXT_AND_A, 		ENG_MAP(1, 0, 0)},
	{GRPH_CMD_TEXT_OR_A, 		ENG_MAP(1, 0, 0)},
	{GRPH_CMD_TEXT_XOR_A, 		ENG_MAP(1, 0, 0)},
	{GRPH_CMD_TEXT_AND_AB, 		ENG_MAP(1, 0, 0)},
	{GRPH_CMD_BLENDING, 		ENG_MAP(1, 1, 0)},
	{GRPH_CMD_ACC, 				ENG_MAP(1, 0, 0)},
	{GRPH_CMD_MULTIPLY_DIV, 	ENG_MAP(1, 0, 0)},
	{GRPH_CMD_PACKING, 			ENG_MAP(1, 0, 0)},
	{GRPH_CMD_DEPACKING, 		ENG_MAP(1, 0, 0)},
	{GRPH_CMD_TEXT_MUL, 		ENG_MAP(1, 0, 0)},
	{GRPH_CMD_PLANE_BLENDING, 	ENG_MAP(1, 0, 0)},
	{GRPH_CMD_1D_LUT, 			ENG_MAP(1, 0, 0)},
	{GRPH_CMD_2D_LUT, 			ENG_MAP(1, 0, 0)},
	{GRPH_CMD_RGBYUV_BLEND, 	ENG_MAP(1, 1, 0)},
	{GRPH_CMD_RGBYUV_COLORKEY, 	ENG_MAP(1, 0, 0)},
	{GRPH_CMD_RGB_INVERT, 		ENG_MAP(1, 0, 0)},
	{GRPH_CMD_YCC_ENC, 			ENG_MAP(1, 0, 0)},
	{GRPH_CMD_YCC_DEC, 			ENG_MAP(1, 0, 0)},
	//GRPH_CMD_AOPMAX
	{GRPH_CMD_MINUS_SHF_ABS, 	ENG_MAP(1, 0, 0)},
	{GRPH_CMD_COLOR_MR, 		ENG_MAP(1, 1, 0)},
	{GRPH_CMD_COLOR_FILTER, 	ENG_MAP(1, 0, 0)},
};
#endif

int graph_drv_uti_get_supt_eng(unsigned int cmd)
{
	return ENG_MAP(1, 1, 1);
#if 0 //don't check eng spec
	int i;
	int total_num;

	total_num = sizeof(cmd_eng_supt_tab)/sizeof(struct cmd_supt_map_info);
	for (i = 0; i < total_num; i ++) {
		if (cmd_eng_supt_tab[i].cmd == cmd) {
			return (int)cmd_eng_supt_tab[i].eng;
		}
	}
	graph_drv_err("not support cmd(0x%x)\n", cmd);
	return -1;
#endif
}

int graph_drv_uti_is_supt_req_fmt(GRPH_CMD cmd, GRPH_FORMAT fmt)
{
	return 0;
#if 0 //don't check eng spec
	int rt;

	rt = 1;
	if (cmd < GRPH_CMD_GOPMAX) {
		rt = 0;
		if ((fmt == GRPH_FORMAT_1BIT) || (fmt == GRPH_FORMAT_8BITS) ||
			(fmt == GRPH_FORMAT_16BITS) || (fmt == GRPH_FORMAT_32BITS)) {
			rt = 1;
		}
	} else if (cmd == GRPH_CMD_A_COPY) {
		rt = 0;
		if ((fmt == GRPH_FORMAT_8BITS) || (fmt == GRPH_FORMAT_32BITS_ARGB8888_RGB) ||
			(fmt == GRPH_FORMAT_32BITS_ARGB8888_A) || (fmt == GRPH_FORMAT_16BITS_ARGB1555_RGB) ||
			(fmt == GRPH_FORMAT_16BITS_ARGB1555_A) || (fmt == GRPH_FORMAT_16BITS_ARGB4444_RGB) ||
			(fmt == GRPH_FORMAT_16BITS_ARGB4444_A)) {
			rt = 1;
		}
	} else if ((cmd == GRPH_CMD_A_AND_B) || (cmd == GRPH_CMD_A_OR_B) || (cmd == GRPH_CMD_A_XOR_B)) {
		if (fmt != GRPH_FORMAT_8BITS) {
			rt = 0;
		}
	} else if ((cmd == GRPH_CMD_YCC_ENC) || (cmd == GRPH_CMD_YCC_DEC)) {
		rt = 0;
		if ((fmt == GRPH_FORMAT_8BITS) || (fmt == GRPH_FORMAT_16BITS_UVPACK)) {
			rt = 1;
		}
	}
	return rt;
#endif
}

unsigned long graph_drv_uti_get_ms(void)
{
	VOS_TICK tick;

	vos_perf_mark(&tick);

	return (tick / 1000);
}

unsigned long graph_drv_uti_get_us(void)
{
	VOS_TICK tick;

	vos_perf_mark(&tick);

	return tick;
}

#if defined(__LINUX)
int graph_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt)
{
#define PARAM_LENGTH 100

	int var_sum, n, ofs, hex_flag;
	char buf[PARAM_LENGTH], *scan_pos, *end_pos;

	if ((count > PARAM_LENGTH) || (count < 2)) {
		graph_drv_err("cmd length(%zd) must <%d and >2\n", count, PARAM_LENGTH);
		return -1;
	}

	memset(buf, 0, PARAM_LENGTH);
	if (copy_from_user(buf, buffer, PARAM_LENGTH)) {
		graph_drv_err("convert fail1\n");
		return -1;
	}

	scan_pos = &buf[0];
	end_pos = &buf[(count - 1)];
	ofs = 0;
	var_sum = 0;
	while (scan_pos < end_pos)
	{
		//skip space
		scan_pos += ofs;
		while (scan_pos < end_pos) {
			if (*scan_pos != ' ') {
				break;
			}
			scan_pos += 1;
		}

		if (scan_pos >= end_pos) {
			return var_sum;
		}


		hex_flag = 0;
		if ((int)(end_pos - scan_pos) > 2) {
			if (((*scan_pos == '0') && (*(scan_pos + 1) == 'x')) ||
				((*scan_pos == '0') && (*(scan_pos + 1) == 'X'))) {
				hex_flag = 1;
			}
		}

		if (hex_flag) {
			n = sscanf(scan_pos, "%x%n", &dst[var_sum], &ofs);
		} else {
			n = sscanf(scan_pos, "%d%n", &dst[var_sum], &ofs);
		}

		//EOF
		if (n == -1) {
			return var_sum;
		}

		//scan fail
		if (n == 0) {
			graph_drv_err("convert fail2\n");
			return -1;
		}

		//dst buffer full
		var_sum += 1;
		if (var_sum >= dst_cnt) {
			break;
		}
	}
	return var_sum;
}
#endif

void *graph_drv_util_malloc(size_t size)
{
	void *ret;

#if defined(__LINUX)
	if (in_interrupt() || in_atomic() || irqs_disabled()) {
		ret = kzalloc(size, GFP_ATOMIC);
	} else {
		ret = kzalloc(size, GFP_KERNEL);
	}
#else
	ret = malloc(size);
#endif

	return ret;
}

void graph_drv_util_mfree(void *addr)
{
#if defined(__LINUX)
	kfree(addr);
#else
	free(addr);
#endif
}

