#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>

#include "kdrv_ipp_builtin.h"
#include "kdrv_ipp_builtin_int.h"

/**
 * 0: uboot boot linux
 * 1: rtos boot linux without fastboot_en
 * 2: rtos boot linux with fastboot_en
 */
static int m_fastboot = 0;

static int m_fastboot_yuvque_en = 0;
static int m_fastboot_yuvque_num = 0;

#if !defined(MODULE)
static int __init early_param_rtos_boot(char *p)
{
	if (p == NULL) {
		return 0;
	}
	if (strncmp(p, "1", 2) == 0) {
		m_fastboot = 0;
	}
	if (strncmp(p, "2", 2) == 0) {
		m_fastboot = 1;
	}
	return 0;
}

early_param("rtos_boot", early_param_rtos_boot);
#endif

int kdrv_builtin_is_fastboot(void)
{
	return m_fastboot;
}

void kdrv_builtin_parse_dts_yuv(void)
{
	//CHAR fb_node[64] = "/fastboot";
	kdrv_ipp_builtin_plat_read_dtsi_array("/fastboot", "yuvque_en", (int *)&m_fastboot_yuvque_en, 1);
	kdrv_ipp_builtin_plat_read_dtsi_array("/fastboot", "queue_num", (int *)&m_fastboot_yuvque_num, 1);
	printk("fastboot yuvque_en = %d, num = %d\r\n", m_fastboot_yuvque_en, m_fastboot_yuvque_num);
}

int kdrv_builtin_is_fastboot_yuvque_en(void)
{
	return m_fastboot_yuvque_en;
}

int kdrv_builtin_get_fastboot_yuvque_num(void)
{
	return m_fastboot_yuvque_num;
}

EXPORT_SYMBOL(kdrv_builtin_is_fastboot);

EXPORT_SYMBOL(kdrv_builtin_is_fastboot_yuvque_en);
EXPORT_SYMBOL(kdrv_builtin_get_fastboot_yuvque_num);
EXPORT_SYMBOL(kdrv_builtin_parse_dts_yuv);

#else
#endif
