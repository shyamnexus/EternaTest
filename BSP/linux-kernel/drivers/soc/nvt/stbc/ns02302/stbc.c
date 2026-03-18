/*
    PMC controller

    PMC controller

    @file       stbc.c
    @ingroup    mIDrvIO_stbc
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/
#include <plat/stbc.h>
#include "../stbc_platform.h"
#include <linux/delay.h>

//Linux kernel

#ifndef CYGBLD_ATTRIB_SECTION
#define CYGBLD_ATTRIB_SECTION(__sect__) __attribute__((section (__sect__)))

#define __string(_x) #_x
#define __xstring(_x) __string(_x)

#define MT_MAJOR            00
#define MT_MINOR            001
#define MT_REVISION_STR     "1.00.001"

#ifndef VERSION_INFO_ID4
#define VERSION_INFO_ID4(_name, _major, _minor, _bugfix, _ext) \
	CYGBLD_ATTRIB_SECTION(".moduleTest.version." __xstring(_major) "_" __xstring(_minor) "_" __xstring(_bugfix) "." __xstring(_name))
#endif
//static CHAR    cAutoTestVersion[] CYGBLD_ATTRIB_SECTION(".otp.version." __xstring(1) "." __xstring(MT_MAJOR)"."__xstring(MT_MINOR)) = "NT9853X_STBC#"MT_REVISION_STR;
static CHAR    cAutoTestVersion[] = "NT98538_STBC#"MT_REVISION_STR;
#endif

#define SETREG32(x, y)              OUTW((x), INW(x) | (y))
#define CLRREG32(x, y)              OUTW((x), INW(x) & ~(y))

#define OUTREG32                    OUTW
#define INREG32                     INW
#define bit_set(m, n)               SETREG32(m, (1<<n))
#define sw(x,y,z)                   OUTREG32(x, y)

#if defined __KERNEL__
/*
     otp_version

     otp_version (56bits)
*/
void stbc_version(void)
{
	DBG_DUMP("%s\r\n", cAutoTestVersion);
	return;
}

#define STBC_CG_PD_REG_OFS		(0xB0)

#define PD_MODE                         3
#define PD_SLEEP_EN_MSK			(1<<2)
#define PD_INTEN_MSK			(1<<4)

#define PMC_BASE                        STBC_IOADDR_STBC_REG_BASE
#define PMC_ENABLE                      (1<<0)
#define PMC_DISABLE                     (0<<0)

#define MCU_OFS_DATA_PORT0		(0x20)	// command port

// software defined control on data port 0
#define MCU_CMD_APB_READ		(1<<0)
#define MCU_CMD_APB_WRITE		(2<<0)
#define MCU_CMD_PWR_SET			(3<<0)
#define MCU_CMD_PWR_GET			(4<<0)
#define MCU_CMD_SFR_READ		(5<<0)
#define MCU_CMD_SFR_WRITE		(6<<0)
#define MCU_CMD_SLEEP			(7<<0)
#define MCU_CMD_ISOLATION		(8<<0)
#define MCU_CMD_TIMER_START		(9<<0)
#define MCU_CMD_TIMER_STOP		(10<<0)
#define MCU_CMD_TIMER_READ		(11<<0)
#define MCU_CMD_TIMER_CHECK		(12<<0)
#define MCU_CMD_TIMER_AUTO_TEST		(13<<0)
#define MCU_CMD_SUSPEND_USB		(14<<0)
#define MCU_CMD_MSK			(0x7FFFFFFF)
#define MCU_CMD_TRIG_MSK		(1<<31)

#define PMC_BASE                        STBC_IOADDR_STBC_REG_BASE

static void stbc_suspened_en(void)
{
	OUTW(PMC_BASE + MCU_OFS_DATA_PORT0, MCU_CMD_TRIG_MSK|MCU_CMD_SUSPEND_USB);
	while (1) {
		if ((INW(PMC_BASE + MCU_OFS_DATA_PORT0)&MCU_CMD_TRIG_MSK) == 0) break;
	}

	return;
}

static void toggle_pmc_reset(void)
{
	UINT32  rdata;

	rdata = INREG32(PMC_BASE + 0x00);

	if ((rdata & PMC_ENABLE) == PMC_DISABLE) {
		OUTREG32(PMC_BASE + 0x00, rdata | PMC_ENABLE);
	}
}

static void toggle_pmc_start(void)
{
	stbc_suspened_en();
}

static inline unsigned int get_cpuid(void)
{
	unsigned int val;
	/*
	 * core 0 is EL1, but core 1 ~ 3 is EL3
	 * cor 1 ~ core 3 need to change to EL1
	 */

	asm volatile("mrs %0, MPIDR_EL1" : "=r"(val)
				 :
				 : "cc");
	return (val & 0xFF);
}

void stbc_pmc_start(UINT32 dbg_en)
{
	unsigned long  spin_flags;

	stbc_platform_clk_standby();

	spin_flags = stbc_platform_spin_lock();

	DBG_DUMP("stbc_pmc_start\r\n");
	toggle_pmc_reset();
	toggle_pmc_start();
	OUTREG32(STBC_IOADDR_STBC_CG_REG_BASE + STBC_CG_PD_REG_OFS, PD_INTEN_MSK|PD_SLEEP_EN_MSK|PD_MODE);
	asm volatile("wfi");
	stbc_platform_spin_unlock(spin_flags);
}
#endif


#if defined __KERNEL__
EXPORT_SYMBOL(stbc_version);
EXPORT_SYMBOL(stbc_pmc_start);
#endif


