/**
 * @file vpe_drv_uti.c
 *  vpe drv utility
 *
 *
 */
#if defined(__LINUX)
#include <linux/module.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <plat/nvt_jiffies.h>
#endif
#include "vpe_drv_dbg_int.h"
#include "vpe_drv_util_int.h"
#include "comm/hwclock.h"

struct val_str_map_info {
	unsigned int val;
	char *str;
};

const static struct val_str_map_info sts_str_tab[VPE_DRV_STS_MAX] = {
	{VPE_DRV_STS_QUEUE, "QUEUE"},
	{VPE_DRV_STS_PROC, "PROC"},
	{VPE_DRV_STS_PROC_END, "PEND"},
	{VPE_DRV_STS_DONE, "DONE"},
	{VPE_DRV_STS_FLUSH, "FLUSH"},
	{VPE_DRV_STS_ERROR, "ERR"},
	{VPE_DRV_STS_IDLE, "IDLE"},
	{VPE_DRV_STS_OK, "OK"},
	{VPE_DRV_STS_QFULL, "QFULL"},
};

const char* vpe_drv_uti_get_sts_str(unsigned int val)
{
	int i;

	for (i = 0; i < VPE_DRV_STS_MAX; i ++) {
		if (sts_str_tab[i].val == val) {
			return sts_str_tab[i].str;
		}
	}
	return 0;
}

#define VPE_DRV_FMT_NUM VPE_DRV_FMT_MAX
const static struct val_str_map_info fmt_str_tab[VPE_DRV_FMT_NUM] = {
	{VPE_DRV_YUV420_SP, "420SP"},
	{VPE_DRV_YUV420_YCC, "420YCC"},
	{VPE_DRV_YUV422_YUYV, "422YUYV"},
	{VPE_DRV_YUV422_YVYU, "422YVYU"},
	{VPE_DRV_YUV422_UYVY, "422UYVY"},
	{VPE_DRV_YUV422_VYUY, "422VYUV"},
	{VPE_DRV_UNKNOW_FMT, "unknown"},
};

const char* vpe_drv_uti_get_fmt_str(unsigned int val)
{
	int i;

	for (i = 0; i < VPE_DRV_FMT_NUM; i ++) {
		if (fmt_str_tab[i].val == val) {
			return fmt_str_tab[i].str;
		}
	}
	return 0;
}

const static struct val_str_map_info lut2d_rot_str_tab[VPE_DRV_DCE_ROT_MAX] = {
	{VPE_DRV_DCE_ROT_NONE, "rot_dis"},
	{VPE_DRV_DCE_ROT_90, "rot_90"},
	{VPE_DRV_DCE_ROT_180, "rot_180"},
	{VPE_DRV_DCE_ROT_270, "rot_270"},
	{VPE_DRV_DCE_ROT_0_H_FLIP, "rot_0H"},
	{VPE_DRV_DCE_ROT_90_H_FLIP, "rot_90H"},
	{VPE_DRV_DCE_ROT_180_H_FLIP, "rot_180H"},
	{VPE_DRV_DCE_ROT_270_H_FLIP, "rot_270H"},
	{VPE_DRV_DCE_ROT_MANUAL, "manual"},
};

const char* vpe_drv_uti_get_lut2d_rot_str(unsigned int val)
{
	int i;

	for (i = 0; i < VPE_DRV_DCE_ROT_MAX; i ++) {
		if (lut2d_rot_str_tab[i].val == val) {
			return lut2d_rot_str_tab[i].str;
		}
	}
	return 0;
}

const static VPE_ENG_DRT drt_tab[VPE_DRV_DRT_MAX] = {
	VPE_ENG_DRT_BYPASS,
	VPE_ENG_DRT_PC2TV,
	VPE_ENG_DRT_TV2PC
};
VPE_ENG_DRT vpe_drv_uti_conv2_drt(enum vpe_drv_drt drt)
{
	if (drt >= VPE_DRV_DRT_MAX) {
		vpe_drv_err("drt(%d > %d) overflow\n", drt, VPE_DRV_DRT_MAX);
		return drt_tab[0];
	}
	return drt_tab[drt];
}

unsigned long vpe_drv_uti_get_ms(void)
{
	return (hwclock_get_counter() / 1000);
}

unsigned long vpe_drv_uti_get_us(void)
{
	return hwclock_get_counter();
}

VPE_ENG_SRC_FMT vpe_drv_uti_fmt_conv2_src_fmt(enum vpe_drv_fmt fmt)
{
	int idx;

	unsigned int fmt_map_tab[VPE_DRV_FMT_MAX] = {
	    VPE_ENG_SRC_YUV420_SP,
	    VPE_ENG_SRC_YCC_YUV420,
		0xffffffff,
		0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
	};

	idx = fmt;
	if (idx >= VPE_DRV_FMT_MAX) {
		vpe_drv_err("oveflow fmt(0x%.8x) (%d >= %d)\r\n", fmt, idx, VPE_DRV_FMT_MAX);
		return 0;
	}
	if (fmt_map_tab[idx] == 0xffffffff) {
		vpe_drv_err("not support fmt(0x%.8x) \r\n", fmt);
		return 0;
	}
	return fmt_map_tab[idx];
}

VPE_ENG_DES_FMT vpe_drv_uti_fmt_conv2_dst_fmt(enum vpe_drv_fmt fmt)
{
	int idx;

	unsigned int fmt_map_tab[VPE_DRV_FMT_MAX] = {
	    VPE_ENG_DES_YUV420_SP,
	    VPE_ENG_DES_YCC_YUV420,
		VPE_ENG_DES_YUV422_YUYV, //VPE_DRV_YUV422_YUYV
		VPE_ENG_DES_YUV422_YVYU, //VPE_DRV_YUV422_YVYU
        VPE_ENG_DES_YUV422_UYVY, //VPE_DRV_YUV422_UYVY
        VPE_ENG_DES_YUV422_VYUY, //VPE_DRV_YUV422_VYUY
        0xffffffff,
	};

	idx = fmt;
	if (idx >= VPE_DRV_FMT_MAX) {
		vpe_drv_err("oveflow fmt(0x%.8x) (%d >= %d)\r\n", fmt, idx, VPE_DRV_FMT_MAX);
		return 0;
	}

	if (fmt_map_tab[idx]  == 0xffffffff) {
		vpe_drv_err("not support fmt(0x%.8x) \r\n", fmt);
		return 0;
	}
	return fmt_map_tab[idx];
}

int vpe_drv_uti_get_y_buf_sz(unsigned int w, unsigned int h, unsigned int type)
{
    switch (type)
    {
        case VPE_DRV_YUV420_SP:
            return VPE_DRV_UTI_GET_420_Y_SZ(w, h);

        case VPE_DRV_YUV420_YCC:
            return VPE_DRV_UTI_GET_420YCC_Y_SZ(w, h);

		case VPE_DRV_YUV422_YUYV:
		case VPE_DRV_YUV422_YVYU:
		case VPE_DRV_YUV422_UYVY:
		case VPE_DRV_YUV422_VYUY:
			return 0;

        default:
			vpe_drv_err("not support format 0x%.8x\n", type);
            return -1;
    }
}

int vpe_drv_uti_get_y_line_offset(unsigned int w, unsigned int type)
{
    switch (type)
    {
        case VPE_DRV_YUV420_SP:
            return VPE_DRV_UTI_GET_420_Y_LOFS(w);

        case VPE_DRV_YUV420_YCC:
            return VPE_DRV_UTI_GET_420YCC_Y_LOFS(w);

		case VPE_DRV_YUV422_YUYV:
		case VPE_DRV_YUV422_YVYU:
		case VPE_DRV_YUV422_UYVY:
		case VPE_DRV_YUV422_VYUY:
            return VPE_DRV_UTI_GET_422_Y_LOFS(w);

        default:
			vpe_drv_err("not support format 0x%.8x\n", type);
            return -1;
    }
}

int vpe_drv_uti_get_uv_line_offset(unsigned int w, unsigned int type)
{
    switch (type)
    {
        case VPE_DRV_YUV420_SP:
            return VPE_DRV_UTI_GET_420_UV_LOFS(w);

        case VPE_DRV_YUV420_YCC:
            return VPE_DRV_UTI_GET_420YCC_UV_LOFS(w);

		case VPE_DRV_YUV422_YUYV:
		case VPE_DRV_YUV422_YVYU:
		case VPE_DRV_YUV422_UYVY:
		case VPE_DRV_YUV422_VYUY:
			return 0;

        default:
			vpe_drv_err("not support format 0x%.8x\n", type);
            return -1;
    }
}

unsigned long vpe_drv_uti_get_uv_addr(unsigned long addr, unsigned int w, unsigned int h, unsigned int type)
{
	int size;

	size = vpe_drv_uti_get_y_buf_sz(w, h, type);
	if (size > 0) {
		return (addr + size);
	}

	return 0;
}

#if defined(__LINUX)
int vpe_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt)
{
#define PARAM_LENGTH 100

	int var_sum, n, ofs, hex_flag;
	char buf[PARAM_LENGTH], *scan_pos, *end_pos;

	if ((count > PARAM_LENGTH) || (count < 2)) {
		vpe_drv_err("cmd length(%zd) must <%d and >2\n", count, PARAM_LENGTH);
		return -1;
	}

	memset(buf, 0, PARAM_LENGTH);
	if (copy_from_user(buf, buffer, PARAM_LENGTH)) {
		vpe_drv_err("convert fail1\n");
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
			vpe_drv_err("convert fail2\n");
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

void *vpe_drv_util_malloc(size_t size)
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

void vpe_drv_util_mfree(void *addr)
{
#if defined(__LINUX)
	kfree(addr);
#else
	free(addr);
#endif
}

unsigned int vpe_drv_util_read_reg(void *vbase, unsigned long ofs)
{
	unsigned long addr;

	addr = (unsigned long)vbase;
	addr = addr + ofs;
#if defined (__LINUX)
	return ioread32((void*)addr);
#else
	return INW(addr);
#endif
}

unsigned int vpe_drv_util_cal_bit_nums(unsigned int val)
{
    unsigned int rt = 0;

    while (val)
    {
        rt += 1;
        val = (val - 1) & val;
    }

    return rt;
}
