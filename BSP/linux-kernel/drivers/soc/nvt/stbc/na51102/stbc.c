/*
    PMC controller

    PMC controller

    @file       stbc.c
    @ingroup    mIDrvIO_stbc
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#include <plat/stbc.h>
#include "../stbc_platform.h"
#include <linux/delay.h>

//Linux kernel
#ifndef EFUSE_DDRP_LDO_TRIM_DATA
#define EFUSE_DDRP_LDO_TRIM_DATA    EFUSE_DDRP_ZQ_TRIM_DATA

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
static CHAR    cAutoTestVersion[] = "NT98530_STBC#"MT_REVISION_STR;
#endif
#endif

#define LABEL_NAME(_name_) _name_
extern char LABEL_NAME(_section_01_addr)[];

#define CKG_REG_ADDR(ofs)           (STBC_IOADDR_CG_REG_BASE+(ofs))
#define CKG_GETREG(ofs)             INW(CKG_REG_ADDR(ofs))
#define CKG_SETREG(ofs,value)       OUTW(CKG_REG_ADDR(ofs), (value))

#define CKG_SETREG32(x, y)          CKG_SETREG((x), CKG_GETREG(x) | (y))
#define CKG_CLRREG32(x, y)          CKG_SETREG((x), CKG_GETREG(x) & ~(y))

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

#define PD_MODE_CASE_1                  0
#define PD_MODE_CASE_2                  1               //STBC
#define PD_MODE                         PD_MODE_CASE_2
#define CG_BASE                         STBC_IOADDR_CG_REG_BASE
#define SCG_BASE                        STBC_IOADDR_STBC_CG_REG_BASE
#define PMC_BASE                        STBC_IOADDR_STBC_REG_BASE
#define PMC_ENABLE                      (1<<0)
#define PMC_DISABLE                     (0<<0)

#define RO_PHY_CLKSEL_LPRO              (0x0<<0)
#define RO_PHY_CLKSEL_LNRO              (0x1<<0)
#define RO_PHY_CLKSEL_PRO               (0x2<<0)
#define RO_PHY_CLKSEL_NRO               (0x3<<0)

#define RO_PHY_FREQ_HIGH_FREQ           (0x0<<4)
#define RO_PHY_FREQ_LOW_FREQ            (0x1<<4)

#define PMC_CLK_EN                      (1<<0)
#define TRNG_RO_CLK_EN                  (1<<1)
#define RO_32K_CLK_EN                   (1<<2)

#define bit_set(m, n)                   SETREG32(m, (1<<n))
#define sw(x,y,z)                       OUTREG32(x, y)

#define pmc_en                          0x1
#define pmc_wakeup_sel                  0x3
#define pmc_wakeup_oen                  0x3
#define pmc_wakeup_from                 0x1
#define pmc_wakeup_dbg                  0x1

#define pmc_debug_probe1                0x1
#define pmc_debug_probe2                0x2
#define pmc_debug_always_on             0x0
#define pmc_debug_always_off            0x0

//#define PMC_SHARE_MEMORY_REG0           (STBC_IOADDR_STBC_REG_BASE + 0xA0)
#define PMC_SHARE_MEMORY_REG0           (STBC_IOADDR_CC_REG_BASE + 0x100)

#define PMC_DEBUG_HALT_REG_OFS          0x00000000
//bit[0] = 0 -> stbc case1
//bit[0] = 1 -> stbc case2
//bit[31..28]-> debug probe
#define PMC_DEBUG_CONFIG_REG_OFS        0x00000004
#define PMC_CFG_DEF_0_OFS               0x00000008
#define PMC_CFG_DEF_1_OFS               0x0000000C

#define PMC_ISO_STAGE_DEF_0_OFS         0x00000010      //stage0~7
#define PMC_ISO_STAGE_DEF_1_OFS         0x00000014      //stage8~15
#define PMC_PAD_OUT_STAGE_DEF_0_OFS     0x00000018      //stage0~7
#define PMC_PAD_OUT_STAGE_DEF_1_OFS     0x0000001C      //stage8~15

#define PMC_PAD_OEN_STAGE_DEF_0_OFS     0x00000020      //stage0~7
#define PMC_PAD_OEN_STAGE_DEF_1_OFS     0x00000024      //stage8~15
#define PMC_USB_SHARE_REGISTER_OFS      0x00000028      //SUSPEND reg
#define STBC_RINGOSC_CLK_CTRL_REG_OFS   0x0000002C      //STBC ring cfg


#define PMC_DEBUG_HALT_REG              (PMC_SHARE_MEMORY_REG0 + PMC_DEBUG_HALT_REG_OFS)
//bit[0] = 0 -> stbc case1
//bit[0] = 1 -> stbc case2
//bit[31..28]-> debug probe
#define PMC_DEBUG_CONFIG_REG            (PMC_SHARE_MEMORY_REG0 + PMC_DEBUG_CONFIG_REG_OFS)
#define PMC_CFG_DEF_0                   (PMC_SHARE_MEMORY_REG0 + PMC_CFG_DEF_0_OFS)
#define PMC_CFG_DEF_1                   (PMC_SHARE_MEMORY_REG0 + PMC_CFG_DEF_1_OFS)

#define PMC_ISO_STAGE_DEF_0             (PMC_SHARE_MEMORY_REG0 + PMC_ISO_STAGE_DEF_0_OFS)       //stage0~7
#define PMC_ISO_STAGE_DEF_1             (PMC_SHARE_MEMORY_REG0 + PMC_ISO_STAGE_DEF_1_OFS)       //stage8~15
#define PMC_PAD_OUT_STAGE_DEF_0         (PMC_SHARE_MEMORY_REG0 + PMC_PAD_OUT_STAGE_DEF_0_OFS)   //stage0~7
#define PMC_PAD_OUT_STAGE_DEF_1         (PMC_SHARE_MEMORY_REG0 + PMC_PAD_OUT_STAGE_DEF_1_OFS)   //stage8~15

#define PMC_PAD_OEN_STAGE_DEF_0         (PMC_SHARE_MEMORY_REG0 + PMC_PAD_OEN_STAGE_DEF_0_OFS)   //stage0~7
#define PMC_PAD_OEN_STAGE_DEF_1         (PMC_SHARE_MEMORY_REG0 + PMC_PAD_OEN_STAGE_DEF_1_OFS)   //stage8~15
#define PMC_USB_SHARE_REGISTER          (PMC_SHARE_MEMORY_REG0 + PMC_USB_SHARE_REGISTER_OFS)    //SUSPEND reg
#define STBC_RINGOSC_CLK_CTRL_REG       (PMC_SHARE_MEMORY_REG0 + STBC_RINGOSC_CLK_CTRL_REG_OFS) //STBC ring cfg

#define IS_CASE_2()                     ((INREG32(PMC_DEBUG_CONFIG_REG) & 0x1))
#define SET_CASE_1()                    CLRREG32(PMC_DEBUG_CONFIG_REG, 0x1)
#define SET_CASE_2()                    SETREG32(PMC_DEBUG_CONFIG_REG, 0x1)

#define SET_OUT_MSG_ON()                SETREG32(PMC_DEBUG_CONFIG_REG, 0x2)
#define SET_OUT_MSG_OFF()               CLRREG32(PMC_DEBUG_CONFIG_REG, 0x2)


#define SET_ISO_STAGE_0(m)              OUTREG32(PMC_ISO_STAGE_DEF_0, m)
#define SET_ISO_STAGE_1(m)              OUTREG32(PMC_ISO_STAGE_DEF_1, m)

#define SET_PAD_OUT_STAGE_0(m)          OUTREG32(PMC_PAD_OUT_STAGE_DEF_0, m)
#define SET_PAD_OUT_STAGE_1(m)          OUTREG32(PMC_PAD_OUT_STAGE_DEF_1, m)

#define SET_PAD_OEN_STAGE_0(m)          OUTREG32(PMC_PAD_OEN_STAGE_DEF_0, m)
#define SET_PAD_OEN_STAGE_1(m)          OUTREG32(PMC_PAD_OEN_STAGE_DEF_1, m)

#define SET_PMC_CFG_0(m)                OUTREG32(PMC_CFG_DEF_0, m)
#define SET_PMC_CFG_1(m)                OUTREG32(PMC_CFG_DEF_1, m)

#define SET_STBC_RING_OSC_CTRL_REG(m)   OUTREG32(STBC_RINGOSC_CLK_CTRL_REG, m)

#define DEBUG_PROBE_SEL()               (INREG32(PMC_DEBUG_CONFIG_REG) & 0xF0000000)
#define IS_OUTPUT_MSG()                 (INREG32(PMC_DEBUG_CONFIG_REG) & 0x2)

#define GET_PMC_CFG_0()                 INREG32(PMC_CFG_DEF_0)
#define GET_PMC_CFG_1()                 INREG32(PMC_CFG_DEF_1)


#define STBC_MSG(m)                     (IS_OUTPUT_MSG() == TRUE)?uart_putSystemUARTStr(m):uart_putSystemUARTStr("")

#define AXI_CH_MON_R_REG_0              0x1170
#define AXI_CH_MON_R_REG_1              0x1174
#define AXI_CH_MON_R_REG_2              0x1178

#define AXI_CH_MON_W_REG_0              0x1180
#define AXI_CH_MON_W_REG_1              0x1184
#define AXI_CH_MON_W_REG_2              0x1188

#define AXI_CH_STATUS_REG               0x1190
#define AXI_CH_RECORD_REG               0x1194

#define AXI_CH_CHREC_OSEL               (0x1<<8)

#define AXI_MON_SWITCH_CH_STATUS(m)     (m == 0)?CLRREG32(TOP_CTRL_REG_BASE_ADDR + AXI_CH_RECORD_REG, AXI_CH_CHREC_OSEL):\
	SETREG32(TOP_CTRL_REG_BASE_ADDR + AXI_CH_RECORD_REG, AXI_CH_CHREC_OSEL)

#define AXI_MON_SET_CHREC_SEL(m)        CLRREG32(TOP_CTRL_REG_BASE_ADDR + AXI_CH_RECORD_REG, 0xFF);\
	SETREG32(TOP_CTRL_REG_BASE_ADDR + AXI_CH_RECORD_REG, (m&0xFF))

#define AXI_MON_R_CH_IDLE               (1<<2)
#define AXI_MON_W_CH_IDLE               (1<<3)

#define AXI_CH_DISABLE_MSG              DISABLE

#define POLLING_IDLE_TIMOUT             100

#define PMC_BASE                        STBC_IOADDR_STBC_REG_BASE
void toggle_pmc_reset(void)
{
	UINT32  rdata;
	rdata = INREG32(PMC_BASE + 0x00);

	if ((rdata & PMC_ENABLE) == PMC_DISABLE) {
		// ------------------------------------------------
		// ---  Setupt CG
		// ------------------------------------------------
		// --- enable ro
		// scaling up  RO clock
		//SETREG32(SCG_BASE + 0x110, RO_PHY_CLKSEL_NRO | RO_PHY_FREQ_LOW_FREQ);
		//CLRREG32(SCG_BASE + 0x110, 0xFFF0);
		SETREG32(SCG_BASE + 0x110, INREG32(STBC_RINGOSC_CLK_CTRL_REG));
		if (IS_OUTPUT_MSG()) {
			DBG_DUMP("  RingOSC clock ctrl(0x110)=0x%08x\r\n", INREG32(SCG_BASE + 0x110));
		}

		//PMC_CLKEN
		bit_set(SCG_BASE + 0x140, 1);                   // enable ro macro

		//TRNG_RO_CLKEN
		bit_set(SCG_BASE + 0x140, 2);                   // enable ro_32k

		// --- select ro
		//RTC_32K_CLKSEL
		//Power down mode 3 source clock selection.
		//0: RTC
		//1: RingOSC
		//switch to RingOSC
		bit_set(SCG_BASE + 0x130, 24);

		// --- select LP_CLK
		// PMC clock sel to RTC CLK
		bit_set(SCG_BASE + 0x130, 0);

		// --- enable PMC clock
		bit_set(SCG_BASE + 0x140, 0);


		// select PD3
		//bit_set (SCG_BASE + 0xb0, 0);
		//bit_set (SCG_BASE + 0xb0, 1);

		// enable sleep
		//bit_set (SCG_BASE + 0xb0, 2);
		//SETREG32(PMC_BASE + 0x0, (pmc_debug_probe1<<28));
		SETREG32(PMC_BASE + 0x0, DEBUG_PROBE_SEL());
		mdelay(5);
		if (IS_OUTPUT_MSG()) {
			DBG_DUMP("   RingOSC system clock(0x130)=0x%08x\r\n", INREG32(SCG_BASE + 0x130));
			DBG_DUMP("RingOSC module clock en(0x140)=0x%08x\r\n", INREG32(SCG_BASE + 0x140));
			DBG_DUMP("	->Setup ring osc\r\n");
			DBG_DUMP("	->Setup  PMC clock\r\n");
		}
	}
}

void toggle_pmc_start(void)
{
	UINT32 pmc_config;
	//ISO stage definition 0x10~0x14
	//sw (PMC_BASE + 0x10, 0x00111110, HSIZE_WORD);
	//sw (PMC_BASE + 0x14, 0x00000000, HSIZE_WORD);
	sw(PMC_BASE + 0x10, INREG32(PMC_ISO_STAGE_DEF_0), HSIZE_WORD);
	sw(PMC_BASE + 0x14, INREG32(PMC_ISO_STAGE_DEF_1), HSIZE_WORD);


	//PWR PAD out stage 0x20~0x24
	//sw (PMC_BASE + 0x20, 0x07770077, HSIZE_WORD);
	//sw (PMC_BASE + 0x24, 0x00000000, HSIZE_WORD);
	sw(PMC_BASE + 0x20, INREG32(PMC_PAD_OUT_STAGE_DEF_0), HSIZE_WORD);
	sw(PMC_BASE + 0x24, INREG32(PMC_PAD_OUT_STAGE_DEF_1), HSIZE_WORD);

	//PWR PAD oen stage 0x30~0x34
	//sw (PMC_BASE + 0x30, 0x08800888, HSIZE_WORD);
	//sw (PMC_BASE + 0x34, 0x00000000, HSIZE_WORD);
	sw(PMC_BASE + 0x30, INREG32(PMC_PAD_OEN_STAGE_DEF_0), HSIZE_WORD);
	sw(PMC_BASE + 0x34, INREG32(PMC_PAD_OEN_STAGE_DEF_1), HSIZE_WORD);

	// --- config pmc
#if (0)
	pmc_config =    pmc_en              << 0  |
					pmc_off_stage       << 8  |
					pmc_on_stage        << 12 |
					pmc_wakeup_sel      << 16 |
					pmc_wakeup_oen      << 20 |
					pmc_wakeup_from     << 24 |
					pmc_wakeup_dbg      << 25 |
					pmc_debug_probe1    << 28 |
					//pmc_debug_always_on << 31 |
					0;
#else
	pmc_config = GET_PMC_CFG_0();
#endif

	if (IS_OUTPUT_MSG()) {

		DBG_DUMP("\r\n");
		DBG_DUMP("[PMC cfg0]=0x%08x\r\n", pmc_config);

		DBG_DUMP("[0x20]=0x%08x\r\n", INREG32(PMC_BASE + 0x20));
		DBG_DUMP("[0x24]=0x%08x\r\n", INREG32(PMC_BASE + 0x24));
		DBG_DUMP("[0x30]=0x%08x\r\n", INREG32(PMC_BASE + 0x30));
		DBG_DUMP("[0x34]=0x%08x\r\n", INREG32(PMC_BASE + 0x34));
	}
	sw(PMC_BASE + 0x00, pmc_config, HSIZE_WORD);
	mdelay(2);

	// ---  switch ISO & IO to PMC
#if (0)
	pmc_config =    pmc_en              << 0  |
					pmc_iso_en          << 2  |
					pmc_io_en           << 4  |
					pmc_off_stage       << 8  |
					pmc_on_stage        << 12 |
					pmc_wakeup_sel      << 16 |
					pmc_wakeup_oen      << 20 |
					pmc_wakeup_from     << 24 |
					pmc_wakeup_dbg      << 25 |
					pmc_debug_probe1    << 28 |
					//pmc_debug_always_on << 31 |
					0;
#else
	pmc_config = GET_PMC_CFG_1();
	if (IS_OUTPUT_MSG()) {
		DBG_DUMP("[PMC cfg1]=0x%08x\r\n", pmc_config);
	}
#endif
//

	sw(PMC_BASE + 0x00, pmc_config, HSIZE_WORD);
	asm volatile("isb");
	//asm volatile("dsm");

#if 0
	if (!IS_CASE_2()) {
		// enable iso
		//if(IS_OUTPUT_MSG()) {
		//  uart_putSystemUARTStr(" =>Enable iso\r\n");
		//}
		//OUTREG32 (PMC_BASE + 0x08, 0x00000002);

		// disable power
		if (IS_OUTPUT_MSG()) {
			uart_putSystemUARTStr("	=>Disable power\r\n");
		}
		//OUTREG32 (PMC_BASE + 0x04, 0x00000068);
		// enable CG interrupt
		SETREG32(0xFFF01108, (1 << 26));
	} else {
		// enable CG interrupt
		SETREG32(0xFFF01188, (1 << 26));
	}
	if (INREG32(PMC_USB_SHARE_REGISTER) == 0) {
		uart_putSystemUARTStr("0xF05F0040=0\r\n");
	} else {
		sw(0xF05F0040, INREG32(PMC_USB_SHARE_REGISTER), HSIZE_WORD);
	}
	asm volatile("isb");
	asm volatile("dsb");
#endif
	mdelay(2);
	if (IS_OUTPUT_MSG()) {
		DBG_DUMP("=======================\r\n");
		DBG_DUMP(" PMC[0x4]=0x%08x\r\n", INREG32(PMC_BASE + 0x04));
		DBG_DUMP("  usb int=0x%08x\r\n", INREG32(PMC_BASE + 0x0C));
		DBG_DUMP("=======================\r\n");
	}
	mdelay(2);
	while (INREG32(PMC_DEBUG_HALT_REG)) {
		//while(test){};
		DBG_DUMP(">>0x%08x\r\n", INREG32(PMC_DEBUG_HALT_REG));
	}
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
	UINT32  reg;
	UINT32  debug_en;
	UINT32  pmc_config;
	UINT32  pmc_iso_en;
	UINT32  pmc_io_en;
	UINT32  pmc_off_stage;
	UINT32  pmc_on_stage;
	unsigned long  spin_flags;
	UINT32  idx;

	debug_en = dbg_en;

	spin_flags = stbc_platform_spin_lock();

	for (idx = 0; idx < 12; idx++) {
		OUTREG32(PMC_SHARE_MEMORY_REG0 + (idx << 2), 0x0);
	}

	reg = INW(STBC_IOADDR_CORE_REG_BASE + 0x10);
	DBG_DUMP("MPIDR_EL1[%02d]->", (int)get_cpuid());
	reg &= ~0x8000000F;
	//reg |= 0x1;//(1 << get_cpuid());
	reg |= (1 << get_cpuid());
	DBG_DUMP("MPIDR_EL1[0x%08x]\r\n", reg);
	OUTW(STBC_IOADDR_CORE_REG_BASE + 0x10, reg);

	SETREG32(PMC_DEBUG_CONFIG_REG, 0x1);
	SETREG32(PMC_DEBUG_CONFIG_REG, 0x2);

	//SET_CASE_2();

	SET_ISO_STAGE_0(0x11111110);
	SET_ISO_STAGE_1(0x00000011);

	SET_PAD_OUT_STAGE_0(0x31013777);
	SET_PAD_OUT_STAGE_1(0x00007777);

	SET_PAD_OEN_STAGE_0(0x00000088);
	SET_PAD_OEN_STAGE_1(0x00008880);
	pmc_iso_en  = 0x3;
	pmc_io_en   = 0xf;
	pmc_off_stage = 0x4;
	pmc_on_stage = 0xa;

	pmc_config =    pmc_en              << 0  |
					pmc_off_stage       << 8  |
					pmc_on_stage        << 12 |
					pmc_wakeup_sel      << 16 |
					pmc_wakeup_oen      << 20 |
					pmc_wakeup_from     << 24 |
					debug_en            << 25 |
					pmc_debug_probe2    << 28 |
					pmc_debug_always_off << 31 |
					0;
	SET_PMC_CFG_0(pmc_config);

	pmc_config =    pmc_en              << 0  |
					pmc_iso_en          << 2  |
					pmc_io_en           << 4  |
					pmc_off_stage       << 8  |
					pmc_on_stage        << 12 |
					pmc_wakeup_sel      << 16 |
					pmc_wakeup_oen      << 20 |
					pmc_wakeup_from     << 24 |
					debug_en            << 25 |
					pmc_debug_probe2    << 28 |
					pmc_debug_always_off << 31 |
					0;


	SET_PMC_CFG_1(pmc_config);
//	SETREG32(0xF0090100, (0x1));

	//clock div bit[15..4] = 0

	//[1]. Setup ring osc
	SET_STBC_RING_OSC_CTRL_REG(RO_PHY_CLKSEL_NRO | RO_PHY_FREQ_LOW_FREQ);

	OUTREG32(PMC_BASE + 0x40, 0x2FF);
	OUTREG32(PMC_BASE + 0x44, 0x2FF);
	OUTREG32(PMC_BASE + 0x48, 0x2FF);
	OUTREG32(PMC_BASE + 0x4C, 0x2FF);
	OUTREG32(PMC_BASE + 0x50, 0x2FF);
	OUTREG32(PMC_BASE + 0x54, 0x2FFFD);
	//OUTREG32(PMC_BASE+0x54, 0x800);

	OUTREG32(PMC_BASE + 0x58, 0x2FF);
	OUTREG32(PMC_BASE + 0x5C, 0x2FF);
	OUTREG32(PMC_BASE + 0x60, 0x2FF);
	OUTREG32(PMC_BASE + 0x64, 0x2FF);
	OUTREG32(PMC_BASE + 0x68, 0x2FF);
	OUTREG32(PMC_BASE + 0x6C, 0x2FF);
	OUTREG32(PMC_BASE + 0x70, 0x2FF);
	OUTREG32(PMC_BASE + 0x74, 0x2FF);
	OUTREG32(PMC_BASE + 0x78, 0x2FF);
	OUTREG32(PMC_BASE + 0x7C, 0x2FF);

	DBG_DUMP("stbc_pmc_start\r\n");
	toggle_pmc_reset();
	toggle_pmc_start();
	OUTREG32(STBC_IOADDR_STBC_CG_REG_BASE + 0xB0, 0x17);
	asm volatile("wfi");
	stbc_platform_spin_unlock(spin_flags);
}
#endif


#if defined __KERNEL__
EXPORT_SYMBOL(stbc_version);
EXPORT_SYMBOL(stbc_pmc_start);
#endif

