/**
 * @file dre_drv_uti.c
 *  dre drv utility
 *
 *
 */
#if defined(__LINUX)
#include <linux/module.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <plat/nvt_jiffies.h>
#endif
#include "dre_drv_dbg_int.h"
#include "dre_drv_util_int.h"
#include "comm/hwclock.h"
#include <plat/top.h>

struct val_str_map_info {
	unsigned int val;
	char *str;
};

const static struct val_str_map_info sts_str_tab[DRE_DRV_STS_MAX] = {
	{DRE_DRV_STS_QUEUE, "QUEUE"},
	{DRE_DRV_STS_PROC, "PROC"},
	{DRE_DRV_STS_PROC_END, "PEND"},
	{DRE_DRV_STS_DONE, "DONE"},
	{DRE_DRV_STS_FLUSH, "FLUSH"},
	{DRE_DRV_STS_ERROR, "ERR"},
	{DRE_DRV_STS_IDLE, "IDLE"},
	{DRE_DRV_STS_OK, "OK"},
	{DRE_DRV_STS_QFULL, "QFULL"},
	{DRE_DRV_STS_SW_PROC, "SWPROC"},
	{DRE_DRV_STS_SW_PROC_END, "SWPEND"},
};

const char* dre_drv_uti_get_sts_str(unsigned int val)
{
	int i;

	for (i = 0; i < DRE_DRV_STS_MAX; i ++) {
		if (sts_str_tab[i].val == val) {
			return sts_str_tab[i].str;
		}
	}
	return 0;
}

unsigned long dre_drv_uti_get_ms(void)
{
	return (hwclock_get_counter() / 1000);
}

unsigned long dre_drv_uti_get_us(void)
{
	return hwclock_get_counter();
}

#if defined(__LINUX)
int dre_drv_uti_proc_buffer_to_int(const char __user *buffer, size_t count, int *dst, int dst_cnt)
{
#define PARAM_LENGTH 100

	int var_sum, n, ofs, hex_flag;
	char buf[PARAM_LENGTH], *scan_pos, *end_pos;

	if ((count > PARAM_LENGTH) || (count < 2)) {
		dre_drv_err("cmd length(%zd) must <%d and >2\n", count, PARAM_LENGTH);
		return -1;
	}

	memset(buf, 0, PARAM_LENGTH);
	if (copy_from_user(buf, buffer, PARAM_LENGTH)) {
		dre_drv_err("convert fail1\n");
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
			dre_drv_err("convert fail2\n");
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

void *dre_drv_util_malloc(size_t size)
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

void dre_drv_util_mfree(void *addr)
{
#if defined(__LINUX)
	kfree(addr);
#else
	free(addr);
#endif
}

unsigned int dre_drv_util_read_reg(void *vbase, unsigned long ofs)
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

BOOL dre_drv_util_is_539a(void)
{
    if (nvt_get_chip_id() == CHIP_NS02402)
        return TRUE;
    else
        return FALSE;
}