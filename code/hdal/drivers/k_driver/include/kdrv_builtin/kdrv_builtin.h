/**
 * @file kdrv_builtin.h
 * @brief type definition of KDRV API.
 */

#ifndef __KDRV_BUILTIN_H__
#define __KDRV_BUILTIN_H__

/*!
 * @fn int kdrv_builtin_is_fastboot(void)
 * @brief indicate start linux with fastboot mode
 * @return return 0: start linux by uboot, 1: start linux by rtos with fastboot
 */
#if defined(__KERNEL__) && !defined(CONFIG_NVT_AMP)
	#define FASTBOOT_FUNC_EN  0
#else
	// RTOS
	#define FASTBOOT_FUNC_EN  1
#endif

/*
#if defined(__KERNEL__)
	#ifdef CONFIG_NVT_AMP
	#define FASTBOOT_FUNC_EN  1
	#else
	#define FASTBOOT_FUNC_EN  0
	#endif
#else
	// RTOS
	#define FASTBOOT_FUNC_EN  1
#endif
*/

extern int kdrv_builtin_is_fastboot(void);

extern int kdrv_builtin_is_fastboot_yuvque_en(void);
extern int kdrv_builtin_get_fastboot_yuvque_num(void);
extern void kdrv_builtin_parse_dts_yuv(void);

#endif
