/*
 *  include/asm/arch-nvt-xxxx/hardware.h
 *
 *  Author:	Howard Chang
 *  Created:	Jan 21, 2016
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __ARCH_HARDWARE_H__
#define __ARCH_HARDWARE_H__

#define __REG(x) 	(*((volatile u32 *) (x)))

enum CHIP_ID {
	CHIP_NA51055 = 0x4821,
	CHIP_NA51084 = 0x5021,
	CHIP_NA51089 = 0x7021,
	CHIP_NA51090 = 0xBC21,
	CHIP_NA51102 = 0x5221,
	CHIP_NA51103 = 0x8B20,
	CHIP_NS02201 = 0xF221,
	CHIP_NS02301 = 0x7721,
	CHIP_NS02302 = 0x5A21,
	CHIP_NS02401 = 0xBB21,
	CHIP_NS02402 = 0x5B21,
};

enum CHIP_VER {
	CHIPVER_A,
	CHIPVER_C
};

extern u32 nvt_get_chip_id(void);
extern u32 nvt_get_chip_ver(void);

#define NVT_TIMER0_CNT		0xF0040108

#endif /* __ARCH_HARDWARE_H__ */
