/**
 * @file ise_drv_uti.c
 *  ise drv utility
 *
 *
 */
#if defined(__LINUX)
#include <linux/module.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <plat/nvt_jiffies.h>
#endif
#include "ise_drv_dbg_int.h"
#include "ise_drv_util_int.h"
#include "comm/hwclock.h"

struct val_str_map_info {
	unsigned int val;
	char *str;
};

const static struct val_str_map_info sts_str_tab[ISE_DRV_STS_MAX] = {
	{ISE_DRV_STS_QUEUE, "QUEUE"},
	{ISE_DRV_STS_PROC, "PROC"},
	{ISE_DRV_STS_PROC_END, "PEND"},
	{ISE_DRV_STS_DONE, "DONE"},
	{ISE_DRV_STS_FLUSH, "FLUSH"},
	{ISE_DRV_STS_ERROR, "ERR"},
	{ISE_DRV_STS_IDLE, "IDLE"},
	{ISE_DRV_STS_OK, "OK"},
	{ISE_DRV_STS_QFULL, "QFULL"},
};

const char* ise_drv_uti_get_sts_str(unsigned int val)
{
	int i;

	for (i = 0; i < ISE_DRV_STS_MAX; i ++) {
		if (sts_str_tab[i].val == val) {
			return sts_str_tab[i].str;
		}
	}
	return 0;
}

#define ISE_DRV_FMT_NUM ISE_DRV_FMT_MAX
const static struct val_str_map_info fmt_str_tab[ISE_DRV_FMT_NUM] = {
	{ISE_DRV_Y8_ONLY, "Y8"},
	{ISE_DRV_Y4_ONLY, "Y4"},
	{ISE_DRV_Y1_ONLY, "Y1"},
	{ISE_DRV_UVP, "UVP"},
	{ISE_DRV_RGB565, "RGB565"},
	{ISE_DRV_ARGB8888, "ARGB8888"},
	{ISE_DRV_ARGB1555, "ARGB1555"},
	{ISE_DRV_ARGB4444, "ARGB4444"},
	{ISE_DRV_YUVP, "YUVP"},
};

const char* ise_drv_uti_get_fmt_str(unsigned int val)
{
	int i;

	for (i = 0; i < ISE_DRV_FMT_NUM; i ++) {
		if (fmt_str_tab[i].val == val) {
			return fmt_str_tab[i].str;
		}
	}
	return 0;
}

const static struct val_str_map_info scale_method_str_tab[ISE_DRV_SCALE_METHOD_MAX] = {
	{ISE_DRV_SCALE_METHOD_AUTO, "auto"},
	{ISE_DRV_SCALE_METHOD_BILINEAR, "bilinear"},
	{ISE_DRV_SCALE_METHOD_NEAREST, "nearest"},
	{ISE_DRV_SCALE_METHOD_ISD, "isd"},
};

const char* ise_drv_uti_get_scale_method_str(unsigned int val)
{
	int i;

	for (i = 0; i < ISE_DRV_SCALE_METHOD_MAX; i ++) {
		if (scale_method_str_tab[i].val == val) {
			return scale_method_str_tab[i].str;
		}
	}
	return 0;
}

ISE_ENG_SCALE_METHOD ise_drv_uti_conv2_scale_method(enum ise_drv_scale_method_sel method)
{

	const ISE_ENG_SCALE_METHOD method_tab[ISE_DRV_SCALE_METHOD_MAX] = {
		0xffffffff,
		ISE_ENG_SCALE_BILINEAR,
		ISE_ENG_SCALE_NEAREST,
		ISE_ENG_SCALE_INTEGRATION,
	};

	if (method == ISE_DRV_SCALE_METHOD_AUTO) {
		ise_drv_err("auto method err\n");
		return method_tab[0];
	}

	if (method >= ISE_DRV_SCALE_METHOD_MAX) {
		ise_drv_err("method(%d > %d) overflow\n", method, ISE_DRV_SCALE_METHOD_MAX);
		return method_tab[0];
	}

	if (method_tab[method] == 0xffffffff) {
		ise_drv_err("not support method(0x%.8x) \r\n", method);
		return method_tab[1];
	}

	return method_tab[method];
}

ISE_ENG_IO_FMT ise_drv_uti_conv2_fmt(enum ise_drv_fmt fmt)
{
	const ISE_ENG_IO_FMT fmt_tab[ISE_DRV_FMT_MAX] = {

		ISE_ENG_IOFMT_Y8,		//ISE_DRV_Y8_ONLY
		ISE_ENG_IOFMT_Y4,		//ISE_DRV_Y4_ONLY
		ISE_ENG_IOFMT_Y1,		//ISE_DRV_Y1_ONLY
		ISE_ENG_IOFMT_UVP,		//ISE_DRV_UVP
		ISE_ENG_IOFMT_RGB565,	//ISE_DRV_RGB565
		ISE_ENG_IOFMT_ARGB8888,	//ISE_DRV_ARGB8888
		ISE_ENG_IOFMT_ARGB1555,	//ISE_DRV_ARGB1555
		ISE_ENG_IOFMT_ARGB4444,	//ISE_DRV_ARGB4444
		ISE_ENG_IOFMT_YUVP,		//ISE_DRV_YUVP
	};

	if (fmt >= ISE_DRV_FMT_MAX) {
		ise_drv_err("fmt(%d > %d) overflow\n", fmt, ISE_DRV_FMT_MAX);
		return fmt_tab[0];
	}
	return fmt_tab[fmt];
}

int ise_drv_uti_is_ycc_mode(enum ise_drv_fmt fmt)
{
	return 0;
}

int ise_drv_uti_get_ch_num(enum ise_drv_fmt fmt)
{
	return 1;
}

struct ise_drv_roi ise_drv_uti_get_min_lofs(struct ise_drv_roi roi, enum ise_drv_fmt fmt)
{
#define RATIO_BASE 1000

	int i, ch_num;
	struct ise_drv_roi rt_roi;
	int ratio_tab[ISE_DRV_FMT_MAX][2] =
	{
		{1000, -1},	//ISE_DRV_Y8_ONLY = 0,
		{500,  -1},	//ISE_DRV_Y4_ONLY,
		{125 , -1},	//ISE_DRV_Y1_ONLY,
		{2000, -1},	//ISE_DRV_UVP,
		{2000, -1},	//ISE_DRV_RGB565,
		{4000, -1},	//ISE_DRV_ARGB8888,
		{2000, -1},	//ISE_DRV_ARGB1555,
		{2000, -1},	//ISE_DRV_ARGB4444,
		{3000, -1},	//ISE_DRV_YUVP,
	};

	rt_roi = roi;
	if (fmt >= ISE_DRV_FMT_MAX) {
		ise_drv_err("fmt(%d > %d) overflow\n", fmt, ISE_DRV_FMT_MAX);
		return rt_roi;
	}

	ch_num = ise_drv_uti_get_ch_num(fmt);
	for (i = 0; i < ch_num; i ++) {
		rt_roi.lofs[i] = rt_roi.w * ratio_tab[fmt][i] / RATIO_BASE;
	}

	return rt_roi;
#undef RATIO_BASE
}

struct ise_drv_uti_buf_info ise_drv_uti_get_buf_size(struct ise_drv_roi roi, enum ise_drv_fmt fmt)
{
#define RATIO_BASE 100

	int i, ch_num;
	struct ise_drv_uti_buf_info info;
	int ratio_tab[ISE_DRV_FMT_MAX][2] =
	{
		{100, -1},	//ISE_DRV_Y8_ONLY = 0,
		{100, -1},	//ISE_DRV_Y4_ONLY,
		{100, -1},	//ISE_DRV_Y1_ONLY,
		{100, -1},	//ISE_DRV_UVP,
		{100, -1},	//ISE_DRV_RGB565,
		{100, -1},	//ISE_DRV_ARGB8888,
		{100, -1},	//ISE_DRV_ARGB1555,
		{100, -1},	//ISE_DRV_ARGB4444,
		{100, -1},	//ISE_DRV_YUVP,
	};

	info.size[0] = 0;
	info.size[1] = 0;

	if (fmt >= ISE_DRV_FMT_MAX) {
		ise_drv_err("fmt(%d > %d) overflow\n", fmt, ISE_DRV_FMT_MAX);
		return info;
	}

	ch_num = ise_drv_uti_get_ch_num(fmt);
	for (i = 0; i < ch_num; i ++) {
		info.size[i] = roi.h * roi.lofs[i] * ratio_tab[fmt][i] / RATIO_BASE;
	}
	return info;

#undef RATIO_BASE
}

unsigned long ise_drv_uti_get_ms(void)
{
	return (hwclock_get_counter() / 1000);
}

unsigned long ise_drv_uti_get_us(void)
{
	return hwclock_get_counter();
}

#if defined(__LINUX)
int ise_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt)
{
#define PARAM_LENGTH 100

	int var_sum, n, ofs, hex_flag;
	char buf[PARAM_LENGTH], *scan_pos, *end_pos;

	if ((count > PARAM_LENGTH) || (count < 2)) {
		ise_drv_err("cmd length(%zd) must <%d and >2\n", count, PARAM_LENGTH);
		return -1;
	}

	memset(buf, 0, PARAM_LENGTH);
	if (copy_from_user(buf, buffer, PARAM_LENGTH)) {
		ise_drv_err("convert fail1\n");
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
			ise_drv_err("convert fail2\n");
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

void *ise_drv_util_malloc(size_t size)
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

void ise_drv_util_mfree(void *addr)
{
#if defined(__LINUX)
	kfree(addr);
#else
	free(addr);
#endif
}

unsigned int ise_drv_util_read_reg(void *vbase, unsigned long ofs)
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
