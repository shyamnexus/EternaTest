/*
    Interrupt handler (1 & 8 vector mode)

*/

#include "interrupt.h"
#include "cache.h"
//#include "interrupt_int.h"
//#include "interrupt_reg.h"
#include <io_address.h>
#include "cache_protected.h"

#if !defined(__aarch64__)

/*
 * SCTLR_EL1/SCTLR_EL2/SCTLR_EL3 bits definitions
 */
#define CR_M            (1 << 0)        /* MMU enable                   */
#define CR_A            (1 << 1)        /* Alignment abort enable       */
#define CR_C            (1 << 2)        /* Dcache enable                */
#define CR_SA           (1 << 3)        /* Stack Alignment Check Enable */
#define CR_Z            (1 << 11)       /* Branch prediction            */
#define CR_I            (1 << 12)       /* Icache enable                */
#define CR_WXN          (1 << 19)       /* Write Permision Imply XN     */
#define CR_EE           (1 << 25)       /* Exception (Big) Endian       */



static inline unsigned int get_cr(void)
{
	unsigned int val;

	asm volatile("mrc p15, 0, %0, c1, c0, 0 @ get CR" : "=r"(val)
		:
		: "cc");
	return val;
}

static inline void set_cr(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 0 @ set CR" :
		: "r"(val)
		: "cc");
	asm volatile("isb");
}

extern char _load_change_dma_clock_base[];
void dispatch_interrupt(void);
void clock_do_power_down(void);
void uart_put_char(UINT8 ch);

extern void v7_outer_cache_disable(void);

#define _GICC_                          0x2000
#define GICC_IAR                        _GICC_ + 0xC
#define GICC_EOIR                       _GICC_ + 0x10
#define CKG_SLEEP_MODE_REG_OFS          0xB0
#define WAIT                            (10000)
#define UART_LSR                        (IOADDR_UART_REG_BASE + 0x14)
#define UART_THR                        (IOADDR_UART_REG_BASE + 0x00)
#define UART_LSR_THR_EMPTY              (0x01 << 5)
#define UART_TRANSMITTER_EMPTY          (0x01 << 6)

#define INREG8(x)                       (*((volatile UINT8*)(x)))
#define OUTREG8(x, y)                   (*((volatile UINT8*)(x)) = (y))
#define SETREG8(x, y)                   OUTREG8((x), INREG8(x) | (y))
#define CLRREG8(x, y)                   OUTREG8((x), INREG8(x) & ~(y))

// below functions may cause more read/write times
#define INREG32(x)                      (*((volatile UINT32*)(x)))
#define OUTREG32(x, y)                  (*((volatile UINT32*)(x)) = (y))
#define SETREG32(x, y)                  OUTREG32((x), INREG32(x) | (y))
#define CLRREG32(x, y)                  OUTREG32((x), INREG32(x) & ~(y))

#define INW                             INREG32

#define AUTO_REFRESH_CTRL               0x20
#define SELF_REFRESH_STATE              0x10
#define SELF_REFRESH_STS                0x80


#define POWER_DOWN_ONLY                 0
#define POWER_DOWN_AND_SELF_REFRESH     1
#define SELF_REFRESH_ONLY               2

#define SELF_REFRESH_FLOW               POWER_DOWN_AND_SELF_REFRESH


#define IOADDR_DRAM_REG_BASE            IOADDR_DDR_ARB_REG_BASE
#define IOADDR_DRAM2_REG_BASE           IOADDR_DDR_ARB2_REG_BASE

#define K_L2_REG1_CONTROL_EN_ON     1
#define K_L2_REG1_CONTROL_EN_OFF    0
#define S_L2_REG1_CONTROL_EN        (0)

#define L2_MEM_BASE             (0xffe00000)

#define L2_REG1_BASE            (L2_MEM_BASE + 0x100)   /* Control */

#define L2_REG1_CONTROL         (*((volatile unsigned long *)(L2_REG1_BASE + 0x00)))
/*
static int checkbit(unsigned long addr,unsigned long mask,unsigned long value) {
    int i = 0;  for(i=0;i<WAIT;i++) {
        if(((*(volatile unsigned long*) addr) & mask) == value)
        return 0;
    }
    return -1;
}*/

void uart_put_char(UINT8 ch)
{
	while (!(INREG32(UART_LSR) & UART_LSR_THR_EMPTY));

	OUTREG8(UART_THR, ch);
}
void dispatch_interrupt(void)
{
	UINT32  IntID;
	UINT32  pdn_status;
	uart_put_char('D');

	IntID = INW(IOADDR_GIC_REG_BASE + (GICC_IAR)); ///Read IAR

	if (IntID == 1023) {
		uart_put_char('D');
		uart_put_char('m');
		uart_put_char('y');
		uart_put_char('I');
		uart_put_char('n');
		uart_put_char('t');
		uart_put_char('\n');
		uart_put_char('\r');
		return;
	} else if (IntID == 90) {
		uart_put_char('C');
		uart_put_char('K');
		uart_put_char('G');
		uart_put_char('_');
		uart_put_char('I');
		uart_put_char('n');
		uart_put_char('t');
		uart_put_char('\n');
		uart_put_char('\r');
		pdn_status = INREG32(IOADDR_STBC_CG_REG_BASE + CKG_SLEEP_MODE_REG_OFS);
		if ((pdn_status & 0x8) == 0x8) {
			uart_put_char('M');
			pdn_status &= ~0x4;
			OUTREG32(IOADDR_STBC_CG_REG_BASE + CKG_SLEEP_MODE_REG_OFS, pdn_status);
			while (1) {
				if (!(INREG32(IOADDR_STBC_CG_REG_BASE + CKG_SLEEP_MODE_REG_OFS) & (1 << 3))) {
					break;
				}
				uart_put_char('x');
			}
			uart_put_char('T');
		} else {
			uart_put_char('N');
		}
	}
	OUTREG32(IOADDR_GIC_REG_BASE + GICC_EOIR, IntID);  //Set EOIR
	INREG32(IOADDR_GIC_REG_BASE + GICC_EOIR);       // dummy read
	return;
}
/*
// power down related phy registers
static UINT32 __attribute__ ((section (".chgdma_data")))phy_reg_09;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy_reg_85;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy_reg_86;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy_reg_87;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy_reg_88;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy_reg_B0;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy_reg_D4;

static UINT32 __attribute__ ((section (".chgdma_data"))) phy2_reg_09;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy2_reg_85;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy2_reg_86;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy2_reg_87;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy2_reg_88;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy2_reg_B0;
static UINT32 __attribute__ ((section (".chgdma_data"))) phy2_reg_D4;
*/

static UINT32 __attribute__((section(".chgdma_data"))) temp_irq_sp_irq;
static UINT32 __attribute__((section(".chgdma_data"))) temp_irq_fp_irq;
static UINT32 __attribute__((section(".chgdma_data"))) temp_irq_sp_fiq;
static UINT32 __attribute__((section(".chgdma_data"))) temp_irq_fp_fiq;
static UINT32 __attribute__((section(".chgdma_data"))) temp_irq_sp_abt;
static UINT32 __attribute__((section(".chgdma_data"))) temp_irq_fp_abt;
//static UINT32 __attribute__ ((section (".chgdma_data"))) temp_irq_sp_udf;
//static UINT32 __attribute__ ((section (".chgdma_data"))) temp_irq_fp_udf;

void clock_do_power_down(void)
{
	UINT32          i;

#if 1
	// setup IRQ stack
	asm volatile("cps #0x12");    //@ Disable I/F bit and change to IRQ mode
	asm volatile("mov %0, sp" :"=r"(temp_irq_sp_irq));
	asm volatile("mov %0, fp" :"=r"(temp_irq_fp_irq));
	asm volatile("mov sp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x2000));
	asm volatile("mov fp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x2400));
	// setup FIQ stack
	asm volatile("cps #0x11");    //@ Disable I/F bit and change to FIQ mode
	asm volatile("mov %0, sp" :"=r"(temp_irq_sp_fiq));
	asm volatile("mov %0, fp" :"=r"(temp_irq_fp_fiq));
	asm volatile("mov sp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x2800));
	asm volatile("mov fp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x2C00));
	// setup Abort stack
	/*
	asm volatile("cps #0x17");    //@ Disable I/F bit and change to abort mode
	asm volatile("mov %0, sp" :"=r"(temp_irq_sp_abt));
	asm volatile("mov %0, fp" :"=r"(temp_irq_fp_abt));
	asm volatile("mov sp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x3000));
	asm volatile("mov fp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x3400));
	// setup UDF stack
	asm volatile("cps #0x1B");    //@ Disable I/F bit and change to UDF mode
	asm volatile("mov %0, sp" :"=r"(temp_irq_sp_udf));
	asm volatile("mov %0, fp" :"=r"(temp_irq_fp_udf));
	asm volatile("mov sp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x3800));
	asm volatile("mov fp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x3C00));
	*/
	asm volatile("cps #0x13");    //@ Disable I/F bit and change to svc mode
	asm volatile("mov %0, sp" :"=r"(temp_irq_sp_abt));
	asm volatile("mov %0, fp" :"=r"(temp_irq_fp_abt));
	asm volatile("mov sp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x3000));
	asm volatile("mov fp, %0" ::"r"((UINT32)&_load_change_dma_clock_base + 0x3400));
#endif

//	while(test){};
#if 1
	// disable MMU
	{
		\
		__asm__ __volatile__(\
			"mrc p15, 0, r0, c1, c0, 0\n\t" \
			"bic r0, r0, #0x5\n\t" \
			"bic r0, r0, #0x1000\n\t" \
			"mcr p15, 0, r0, c1, c0, 0\n\t" \
			: \
			: \
			: "r0" \
		);
		\
	}
	asm volatile("dsb");
	asm volatile("isb");
#endif

#if (0)                         // power off DMA2 fist
//while (1);
	if (INREG32(IOADDR_DRAM2_REG_BASE + 0x04) & AUTO_REFRESH_CTRL) {
		// Disable whole DMA channel
		CLRREG32(IOADDR_DRAM2_REG_BASE + 0x8000, 0x01);
		while (1) {
			if (INREG32(IOADDR_DRAM2_REG_BASE + 0x8000) & 0x02) {
				break;
			}
		}

		//Clear auto refresh
		CLRREG32(IOADDR_DRAM2_REG_BASE + 0x4, AUTO_REFRESH_CTRL);

		// Write PHY registers
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x33 * 4, 0x72);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x35 * 4, 0x72);
		CLRREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x51 * 4, 0x01);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x55 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x56 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x5B * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x60 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x85 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x86 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x87 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x88 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x97 * 4, 0x62);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x9E * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0xD4 * 4, 0x10);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x09 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x15 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x18 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x93 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0xB0 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0xC3 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0x98 * 4, 0x00);
		OUTREG32(IOADDR_DRAM2_REG_BASE + 0x1000 + 0xA8 * 4, 0x00);

		// Assert async reset
		CLRREG32(IOADDR_CG_REG_BASE + 0xB0, 1 << 7);
		CLRREG32(IOADDR_CG_REG_BASE + 0xA0, 1 << 1);
	}
#endif
#if (0)
	if (INREG32(IOADDR_DRAM2_REG_BASE + 0x4) & AUTO_REFRESH_CTRL) {
		uart_put_char('D');
		uart_put_char('R');
		uart_put_char('2');
		uart_put_char('O');
		uart_put_char('N');
		uart_put_char('\n');
		uart_put_char('\r');
		b_dram2_valid = 1;
	} else {
		uart_put_char('D');
		uart_put_char('R');
		uart_put_char('2');
		uart_put_char('O');
		uart_put_char('F');
		uart_put_char('F');
		uart_put_char('\n');
		uart_put_char('\r');
		b_dram2_valid = 0;
	}
#endif
	//b_dram2_valid = 0;

	uart_put_char('P');
	uart_put_char('D');
	uart_put_char('N');
	uart_put_char('\n');
	uart_put_char('\r');

	for (i = 0; i < 1000; i++) {
		if (INREG32(UART_LSR) & UART_TRANSMITTER_EMPTY) {
			break;
		}
	}
	//__asm__ __volatile__("isb\n\t");
	//__asm__ __volatile__("wfi\n\t");
	asm volatile("nop");
	asm volatile("nop");

	uart_put_char('w');
	uart_put_char('a');
	uart_put_char('k');
	uart_put_char('\n');
	uart_put_char('\r');

	__asm__ __volatile__("isb\n\t");
//	goto done;
	// Disable whole DMA channel
	CLRREG32(0xf00d0490, 0x1);
	CLRREG32(0xf00d0540, 0x1);
	CLRREG32(0xf00d05f0, 0x1);
	CLRREG32(0xf00d06a0, 0x1);
	CLRREG32(0xf00d0750, 0x1);
	CLRREG32(0xf00d0800, 0x1);
	CLRREG32(0xf00d08b0, 0x1);
	CLRREG32(0xf00d0960, 0x1);

	uart_put_char('a');
	uart_put_char('x');
	uart_put_char('i');

	while (1) {
		if (INREG32(0xf00d03fc) == 0x0) {
			break;
		}
	}
	uart_put_char('-');
	uart_put_char('i');
	uart_put_char('d');
	uart_put_char('l');
	uart_put_char('e');
	uart_put_char('\n');
	uart_put_char('\r');

	SETREG32(0xf00d0304, 0x2);

	//wait 0xf00d0308[26..25] = 0x3
	while (1) {
		if (((INREG32(0xf00d0308) >> 25) & 0x3) == 0x3) {
			break;
		}
	}

	//disable DFI
	//0xf00d01c4[0] = 0
	CLRREG32(0xf00d01c4, 0x1);

	//7. trig self refresh
	//0xf00d0030[5] = 1
	SETREG32(0xf00d0030, (1 << 5));

	uart_put_char('s');
	uart_put_char('l');
	uart_put_char('f');
	uart_put_char('-');
	uart_put_char('b');
	uart_put_char('\n');
	uart_put_char('\r');

	while (1) {
		if ((INREG32(0xf00d0004) & 0x3) == 0x3) {
			break;
		}
	}


#if 1//(SELF_REFRESH_FLOW != SELF_REFRESH_ONLY)
	for (i = 0; i < 1000; i++) {
		if (INREG32(UART_LSR) & UART_TRANSMITTER_EMPTY) {
			break;
		}
	}
	__asm__ __volatile__("isb\n\t");
	__asm__ __volatile__("wfi\n\t");
	asm volatile("nop");
	asm volatile("nop");
#endif

#if (SELF_REFRESH_FLOW == SELF_REFRESH_ONLY)
	while ((INREG32(0xF0070000 + 0x4) & 0x2) != 0x2) {
		uart_put_char('.');
	}
#endif


#if (SELF_REFRESH_FLOW != POWER_DOWN_ONLY)
	// Restore PHY registers
	//CLRREG32(IOADDR_CG_REG_BASE + 0x88, 1 << 28);
	//SETREG32(IOADDR_CG_REG_BASE + 0x88, 1 << 28);

	//Clear self refresh
	//7. trig self refresh
	//0xf00d0030[5] = 1
	CLRREG32(0xf00d0030, (1 << 5));

	while (1) {
		if ((INREG32(0xf00d0004) & 0x3) != 0x3) {
			break;
		}
	}

	CLRREG32(0xf00d0304, 0x2);

	//enable DFI
	//0xf00d01c4[0] = 0
	SETREG32(0xf00d01c4, 0x1);

	// Enable whole DMA channel
	SETREG32(0xf00d0490, 0x1);
	SETREG32(0xf00d0540, 0x1);
	SETREG32(0xf00d05f0, 0x1);
	SETREG32(0xf00d06a0, 0x1);
	SETREG32(0xf00d0750, 0x1);
	SETREG32(0xf00d0800, 0x1);
	SETREG32(0xf00d08b0, 0x1);
	SETREG32(0xf00d0960, 0x1);

	asm volatile("dmb");
	asm volatile("dsb");
#endif

//done:
	uart_put_char('D');
	uart_put_char('o');
	uart_put_char('n');
	uart_put_char('e');
	uart_put_char('\n');
	uart_put_char('\r');


#if 1
	uart_put_char('1');
	// enable MMU
	{
		\
		__asm__ __volatile__(\
			"mrc p15, 0, r0, c1, c0, 0\n\t" \
			"orr r0, r0, #0x5\n\t" \
			"orr r0, r0, #0x1000\n\t" \
			"mcr p15, 0, r0, c1, c0, 0\n\t" \
			: \
			: \
			: "r0" \
		);
		\
	}
	uart_put_char('2');
	asm volatile("isb");
	asm volatile("dsb");
	uart_put_char('3');
	cpu_invalidateICacheAll();
	uart_put_char('4');

#endif

#if 1
//while(test) {};
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");

	// restore IRQ stack
	asm volatile("cps #0x12");    //@ Disable I/F bit and change to IRQ mode
	asm volatile("mov sp, %0" ::"r"(temp_irq_sp_irq));
	asm volatile("mov fp, %0" ::"r"(temp_irq_fp_irq));
	uart_put_char('5');
	// restore FIQ stack
	asm volatile("cps #0x11");    //@ Disable I/F bit and change to FIQ mode
	asm volatile("mov sp, %0" ::"r"(temp_irq_sp_fiq));
	asm volatile("mov fp, %0" ::"r"(temp_irq_fp_fiq));
	uart_put_char('6');

	// restore Abort stack
//	asm volatile("cps #0x17");    //@ Disable I/F bit and change to abort mode
//	asm volatile("mov sp, %0" ::"r"(temp_irq_sp_abt));
//	asm volatile("mov fp, %0" ::"r"(temp_irq_fp_abt));
//	uart_put_char('7');
	// restore UDF stack
//	asm volatile("cps #0x1B");    //@ Disable I/F bit and change to UDF mode
//	asm volatile("mov sp, %0" ::"r"(temp_irq_sp_udf));
//	asm volatile("mov fp, %0" ::"r"(temp_irq_fp_udf));
	// restore Abort stack
	asm volatile("cps #0x13");    //@ Disable I/F bit and change to abort mode
	asm volatile("mov sp, %0" ::"r"(temp_irq_sp_abt));
	asm volatile("mov fp, %0" ::"r"(temp_irq_fp_abt));

	uart_put_char('8');
	asm volatile("cps #0x1F");    //@ Disable I/F bit and change to svc mode

#endif

	uart_put_char('9');
	asm volatile("isb");
	asm volatile("dsb");

	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
}
#if 0
static void clk_uart_put_char(UINT8 ch)
{
	while (!(INREG8(UART_LSR) & UART_LSR_THR_EMPTY));

	OUTREG8(UART_THR, ch);
}
#endif

typedef void (*LDR_GENERIC_CB)(void);

void clock_do_sleep_dram_off(void)
{

#if 0
	UINT32          reg;
	int is_icache_en = 0, is_dcache_en = 0, is_mmu_en = 0;
	void (*image_entry)(void) = NULL;
	//clk_uart_put_char('O');
	//clk_uart_put_char('F');
	//clk_uart_put_char('F');
	//clk_uart_put_char('\n');
	//clk_uart_put_char('\r');
//	__asm__ __volatile__("isb\n\t");
#if 0//(SELF_REFRESH_FLOW != POWER_DOWN_ONLY)
//while (1);
	// Disable whole DMA channel
	CLRREG32(IOADDR_DRAM_REG_BASE + 0x8000, 0x01);
	while (1) {
		if (INREG32(IOADDR_DRAM_REG_BASE + 0x8000) & 0x02) {
			break;
		}
	}
//while (1);

	//Clear auto refresh
	CLRREG32(IOADDR_DRAM_REG_BASE + 0x4, AUTO_REFRESH_CTRL);

	// Delay > 50 DMA clock
	for (i = 50; i; i--) {
		INREG32(IOADDR_DRAM_REG_BASE + 0x4);
	}

	//Issue self refresh
	SETREG32(IOADDR_DRAM_REG_BASE + 0x4, SELF_REFRESH_STATE);

	//Bit[7] Wait until become 1
	while ((INREG32(IOADDR_DRAM_REG_BASE + 0x4) & SELF_REFRESH_STS) != SELF_REFRESH_STS) {
		clk_uart_put_char('p');
	}
	clk_uart_put_char('s');
	clk_uart_put_char('l');
	clk_uart_put_char('f');
	clk_uart_put_char('-');
	clk_uart_put_char('b');
	clk_uart_put_char('\n');
	clk_uart_put_char('\r');

	// Enter DC test mode
	SETREG32(IOADDR_DRAM_REG_BASE + 0x34, 1 << 7);
	SETREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x51 * 4, (1 << 1));
	SETREG32(IOADDR_DRAM_REG_BASE + 0x30, 1 << 5);

//    while (1);

	// Disable DMA clk
//    OUTREG32(IOADDR_DRAM_REG_BASE + 0x1008, 0x0);
	CLRREG32(IOADDR_CG_REG_BASE + 0x70, 0x02);

	// Write PHY registers
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x33 * 4, 0x72);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x35 * 4, 0x72);
	CLRREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x51 * 4, 0x81);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x55 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x56 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x5B * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x60 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x85 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x86 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x87 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x88 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x97 * 4, 0x64);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x9E * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xD4 * 4, 0x10);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x09 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x15 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x18 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x93 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xB0 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xC3 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x98 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xA8 * 4, 0x00);

	// Assert async reset
	CLRREG32(IOADDR_CG_REG_BASE + 0x88, 1 << 28);   // assert DDR phy
//	CLRREG32(IOADDR_CG_REG_BASE + 0x80, 1 << 1); // assert DDR mau
#if 0
	// Write PHY registers
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x33 * 4, 0x72);
//    OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x19*4, 0x00); // test B
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x35 * 4, 0x72);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x55 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x56 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x5B * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x60 * 4, 0x20);
	phy_reg_85 = INREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x85 * 4);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x85 * 4, 0x00);
	phy_reg_86 = INREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x86 * 4);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x86 * 4, 0x00);
	phy_reg_87 = INREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x87 * 4);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x87 * 4, 0x00);
	phy_reg_88 = INREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x88 * 4);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x88 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x9E * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xC3 * 4, 0x00);
	phy_reg_D4 = INREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xD4 * 4);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xD4 * 4, 0x10);
	phy_reg_09 = INREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x09 * 4);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x09 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x15 * 4, 0x03);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x93 * 4, 0x00);
	phy_reg_B0 = INREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xB0 * 4);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xB0 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x98 * 4, 0x00);
	OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xA8 * 4, 0x00);
//    OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0x98*4, 0x10); // test A
//    OUTREG32(IOADDR_DRAM_REG_BASE + 0x1000 + 0xA8*4, 0xE4); // test A
#endif

//    i = INREG32(IOADDR_TIMER_REG_BASE + 0x108);
//    while (1) {     // wait 200us for BG/LDO wake-up and stable
//        if ((INREG32(IOADDR_TIMER_REG_BASE + 0x108) - i) > 200) break;
//    }
#endif

	//for (i=0; i<1000; i++) {
	//    if (INREG32(UART_LSR) & UART_TRANSMITTER_EMPTY) break;
	//}

	is_dcache_en = dcache_status();
	if (is_dcache_en) {
//		printf("detect L1 D cache enabled\r\n");
		dcache_disable();
	}
	if (L2_REG1_CONTROL & (K_L2_REG1_CONTROL_EN_ON << S_L2_REG1_CONTROL_EN)) {
//		printf("detect L2 enabled: 0x%x\r\n", L2_REG1_CONTROL);
		v7_outer_cache_disable();
	}
	cpu_invalidateDCacheAll();
	is_icache_en = icache_status();
	if (is_icache_en) {
//		printf("detect I cached enabled\r\n");
		icache_disable();
		cpu_invalidateICacheAll();
	}
	if (is_mmu_en) {
		set_cr(reg & ~CR_M);
	}

	//DBG_DUMP("Jump into sram\n");

	// jump to special loader
	image_entry = (LDR_GENERIC_CB)(*((unsigned long *)0xF07C0000));
//	cpu_invalidateICacheAll();
//	asm volatile("isb");
	image_entry();

	clk_uart_put_char('O');
#endif
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
}

#endif