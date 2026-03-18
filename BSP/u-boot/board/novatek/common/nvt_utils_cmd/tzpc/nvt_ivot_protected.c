
#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <cpu_func.h>
#include <part.h>
#include <asm/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <stdlib.h>
#include <linux/arm-smccc.h>
#include "nvt_ivot_tzpc_utils.h"
#include "nvt_ivot_protected.h"

//UINT32 dma_getDramBaseAddr(DMA_ID id);
//UINT32 dma_getDramCapacity(DMA_ID id);
#define INREG32(x)                      readl(x)
#define OUTREG32(x, y)                  writel((y), (x))

#define SETREG32(x, y)                  OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)                  OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register

#define _Y_LOG(fmt, args...)            printf(DBG_COLOR_YELLOW fmt DBG_COLOR_END, ##args)
#define _R_LOG(fmt, args...)            printf(DBG_COLOR_RED fmt DBG_COLOR_END, ##args)
#define _M_LOG(fmt, args...)            printf(DBG_COLOR_MAGENTA fmt DBG_COLOR_END, ##args)
#define _G_LOG(fmt, args...)            printf(DBG_COLOR_GREEN fmt DBG_COLOR_END, ##args)
#define _W_LOG(fmt, args...)            printf(DBG_COLOR_WHITE fmt DBG_COLOR_END, ##args)
#define _X_LOG(fmt, args...)            printf(DBG_COLOR_HI_GRAY fmt DBG_COLOR_END, ##args)


static UINT32 chip_id = 0x0;

#ifdef CONFIG_TARGET_NS02302_A64

#define PROT_START_OFS(ch)				(MAU_PROTECT0_START_ADDR + ch * 8)
#define PROT_END_OFS(ch)				(MAU_PROTECT0_END_ADDR + ch * 8)
#define PROT_EN0_OFS(ch)				(MAU_IN_RANGE_PROTECT0_ENABLE_OFS0 + ch * 8)
#define PROT_EN1_OFS(ch)				(MAU_IN_RANGE_PROTECT0_ENABLE_OFS1 + ch * 8)

#define IOADDR_TZASC_REG_BASE         	0x2F01C0000

#define ARB_SETREG(ofs,value)           OUTREG32(IOADDR_TZASC_REG_BASE+(ofs),(value))
#define ARB_GETREG(ofs)                 INREG32(IOADDR_TZASC_REG_BASE+(ofs))


static void debug_dump_addr(UINT64 addr, UINT32 size)
{
	UINT32 i;
	UINT64 j;

	for (j = addr; j < (addr + size); j += 0x10) {
		printf("0x%lx:", j);
		for (i = 0; i < 4; i++) {
			printf("%08x", readl(j + 4 * i));
			printf("  ");
		}
		printf("\r\n");
	}
}

static UINT32 arb_get_reg(DDR_ARB id, UINT32 offset)
{
	return ARB_GETREG(offset);
}

static void arb_set_reg(DDR_ARB id, UINT32 offset, REGVALUE value)
{
	ARB_SETREG(offset, value);
}

void arb_enable_wp(DDR_ARB id, DMA_WRITEPROT_SET set,
		DMA_WRITEPROT_ATTR *p_attr)
{
#if 1
	UINT32 v_mask[DMA_CH_GROUP_CNT];
	union T_MAU_INT_ENABLE_REG intctrl_reg = {0};
	union T_MAU_INT_STATUS_REG sts_reg;
	union T_MAU_PROTECT_MODE_REG mode;

	if (id > DDR_ARB_1) {
		printf("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > OUT_WP) {
		printf("invalid protect ch %d\r\n", set);
		return;
	}

	if (p_attr->starting_addr & 0x7f) {
		printf("starting addr not 32 word aligned 0x%08x\r\n", (unsigned int)p_attr->starting_addr);
		return;
	}

	if (p_attr->size & 0x7f) {
		printf("size not 32 word aligned 0x%x\r\n", (unsigned int)p_attr->size);
		return;
	}

	mode.Reg = arb_get_reg(id, MAU_PROTECT_MODE_OFS);
	memcpy(v_mask, &p_attr->mask, sizeof(DMA_CH_MSK));


	// disable wp int
	intctrl_reg.Reg = arb_get_reg(id, MAU_INT_ENABLE_OFS);
	intctrl_reg.Reg &= ~0xffff;
	arb_set_reg(id, MAU_INT_ENABLE_OFS, intctrl_reg.Reg);

	// clr status
	sts_reg.Reg = 0xffff;
	arb_set_reg(id, MAU_INT_STATUS_OFS, sts_reg.Reg);

	// set addr
	if (set > WPSET_4) {
		arb_set_reg(id, MAU_OUT_PROTECT_START_ADDR, p_attr->starting_addr);
		arb_set_reg(id, MAU_OUT_PROTECT_END_ADDR, p_attr->starting_addr + p_attr->size - 1);
	} else {
		arb_set_reg(id, PROT_START_OFS(set), p_attr->starting_addr);
		arb_set_reg(id, PROT_END_OFS(set), p_attr->starting_addr + p_attr->size - 1);
	}

	// set mode
	if (set == WPSET_0) {
		mode.Bit.WP0_MODE = p_attr->level;
	} else if (set == WPSET_1) {
		mode.Bit.WP1_MODE = p_attr->level;
	} else if (set == WPSET_2) {
		mode.Bit.WP2_MODE = p_attr->level;
	} else if (set == WPSET_3) {
		mode.Bit.WP3_MODE = p_attr->level;
	} else if (set == WPSET_4) {
		mode.Bit.WP4_MODE = p_attr->level;
	} else {
		mode.Bit.OUT_WP_MODE = p_attr->level;
	}
	arb_set_reg(id, MAU_PROTECT_MODE_OFS, mode.Reg);

	// set en
	arb_set_reg(id, PROT_EN0_OFS(set), v_mask[0]);
	arb_set_reg(id, PROT_EN1_OFS(set), v_mask[1]);
#endif
}

void arb_disable_wp(DDR_ARB id, DMA_WRITEPROT_SET set)
{
#if 1
	union T_MAU_INT_ENABLE_REG intctrl_reg = {0};
	union T_MAU_INT_STATUS_REG sts_reg;


	if (id > DDR_ARB_1) {
                printf("invalid arb ch %d\r\n", id);
                return;
        }

        if (set > OUT_WP) {
                printf("invalid protect ch %d\r\n", set);
                return;
        }


	// disable wp
	arb_set_reg(id, PROT_EN0_OFS(set), 0);
	arb_set_reg(id, PROT_EN1_OFS(set), 0);

    // disable inten
    intctrl_reg.Reg = arb_get_reg(id, MAU_INT_ENABLE_OFS);
    intctrl_reg.Reg &= ~0xffff;
    arb_set_reg(id, MAU_INT_ENABLE_OFS, intctrl_reg.Reg);

    // ensure int sts is cleared
    sts_reg.Reg = 0xffff;
    arb_set_reg(id, MAU_INT_STATUS_OFS, sts_reg.Reg);

#endif
}
#elif defined(CONFIG_TARGET_NS02301)

#define IOADDR_TZASC_REG_BASE         	0xF0FE0000

#define ARB_SETREG(ofs,value)	OUTW(IOADDR_DRAM_REG_BASE+(ofs),(value))
#define ARB_GETREG(ofs)		INW(IOADDR_DRAM_REG_BASE+(ofs))

#define PROT_SETREG(ofs,value)	OUTW(IOADDR_TZASC_REG_BASE+(ofs),(value))
#define PROT_GETREG(ofs)		INW(IOADDR_TZASC_REG_BASE+(ofs))

#define PROTECT_START_ADDR_OFS(ch)      (DMA_PROTECT_STARTADDR0_REG0_OFS+(ch)*8)
#define PROTECT_END_ADDR_OFS(ch)        (DMA_PROTECT_STOPADDR0_REG0_OFS+(ch)*8)
#define PROTECT_CH_MSK0_OFS(ch)         (DMA_PROTECT_RANGE0_MSK0_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK1_OFS(ch)         (DMA_PROTECT_RANGE0_MSK1_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK2_OFS(ch)         (DMA_PROTECT_RANGE0_MSK2_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK3_OFS(ch)         (DMA_PROTECT_RANGE0_MSK3_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK4_OFS(ch)         (DMA_PROTECT_RANGE0_MSK4_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK5_OFS(ch)         (DMA_PROTECT_RANGE0_MSK5_REG_OFS+(ch)*32)

static void debug_dump_addr(UINT32 addr, UINT32 size)
{
	UINT32 i;
	UINT32 j;

	for (j = addr; j < (addr + size); j += 0x10) {
		printf("0x%lx:", j);
		for (i = 0; i < 4; i++) {
			printf("%08x", readl(j + 4 * i));
			printf("  ");
		}
		printf("\r\n");
	}
}

static UINT32 arb_get_reg(DDR_ARB id, UINT32 offset)
{
	if (id == DDR_ARB_1) {
		return ARB_GETREG(offset);
	} else {
		return 0;//ARB2_GETREG(offset);
	}
}

static void arb_set_reg(DDR_ARB id, UINT32 offset, REGVALUE value)
{
	if (id == DDR_ARB_1) {
		ARB_SETREG(offset, value);
	} else {
		;//ARB2_SETREG(offset, value);
	}
}

static UINT32 prot_get_reg(DDR_ARB id, UINT32 offset)
{
	if (id == DDR_ARB_1) {
		return PROT_GETREG(offset);
	} else {
		return 0;//PROT2_GETREG(offset);
	}
}

static void prot_set_reg(DDR_ARB id, UINT32 offset, REGVALUE value)
{
	if (id == DDR_ARB_1) {
		PROT_SETREG(offset, value);
	} else {
		;//PROT2_SETREG(offset, value);
	}
}

void arb_enable_wp(DDR_ARB id, DMA_WRITEPROT_SET set,
		DMA_WRITEPROT_ATTR *p_attr)
{
	UINT32 i = 0;
	//UINT32 lock;
	UINT32 v_mask[DMA_CH_GROUP_CNT];
	union T_DMA_PROTECT_INTSTS_REG sts_reg = {0};
	//T_DMA_PROTECT_INTCTRL_REG intctrl_reg = {0};
	union T_DMA_PROTECT_CTRL_REG wp_ctrl;
	union T_DMA_PROTECT_REGION_EN_REG0 region_en0 = {0};
	union T_DMA_PROTECT_REGION_EN_REG1 region_en1 = {0};

	if (id > DDR_ARB_1) {
		printf("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_5) {
		printf("invalid protect ch %d\r\n", set);
		return;
	}

	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			if (((UINT32)p_attr->protect_rgn_attr[i].starting_addr) & 0xF) {
				printf("start addr 0x%x NOT 4 word align\r\n",
					(unsigned int)p_attr->protect_rgn_attr[i].starting_addr);
			}
			if (((UINT32)p_attr->protect_rgn_attr[i].size) & 0xF) {
				printf("size 0x%x NOT 4 word align\r\n",
					(unsigned int)p_attr->protect_rgn_attr[i].size);
			}
		}
	}

	memcpy(v_mask, &p_attr->mask, sizeof(DMA_CH_MSK));

	//lock = ddr_arb_platform_spin_lock();

	// disable wp before setting
	wp_ctrl.Reg = prot_get_reg(id, DMA_PROTECT_CTRL_REG_OFS);
	wp_ctrl.Reg &= ~(1<<set);
	prot_set_reg(id, DMA_PROTECT_CTRL_REG_OFS, wp_ctrl.Reg);

	// disable ch inten before setting
	/*intctrl_reg.reg = arb_get_reg(id, DMA_PROTECT_INTCTRL_REG_OFS);
	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			intctrl_reg.reg &= ~(1<<(set * 4 + i));
		}
	}
	arb_set_reg(id, DMA_PROTECT_INTCTRL_REG_OFS, intctrl_reg.reg);*/

	// ensure int sts is cleared
	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			sts_reg.Reg |= 1<<(set * 4 + i);
		}
	}
	arb_set_reg(id, DMA_PROTECT_INTSTS_REG_OFS, sts_reg.Reg);

	// setup protected channels
	prot_set_reg(id, PROTECT_CH_MSK0_OFS(set), v_mask[DMA_CH_GROUP0]);
	prot_set_reg(id, PROTECT_CH_MSK1_OFS(set), v_mask[DMA_CH_GROUP1]);
	prot_set_reg(id, PROTECT_CH_MSK2_OFS(set), v_mask[DMA_CH_GROUP2]);
	prot_set_reg(id, PROTECT_CH_MSK3_OFS(set), v_mask[DMA_CH_GROUP3]);
	prot_set_reg(id, PROTECT_CH_MSK4_OFS(set), v_mask[DMA_CH_GROUP4]);
	prot_set_reg(id, PROTECT_CH_MSK5_OFS(set), v_mask[DMA_CH_GROUP5]);

	// setup range
	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			prot_set_reg(id, PROTECT_START_ADDR_OFS(set * 4 + i), p_attr->protect_rgn_attr[i].starting_addr);
			prot_set_reg(id, PROTECT_END_ADDR_OFS(set * 4 + i),
			p_attr->protect_rgn_attr[i].starting_addr + p_attr->protect_rgn_attr[i].size - 1);
		}
	}

	// enable write protect
	wp_ctrl.Reg |= 1 << set;

	// set mode
	wp_ctrl.Reg &= ~(1 << (set + 8));
	wp_ctrl.Reg |= (p_attr->protect_mode << (set + 8));

	// set level
	wp_ctrl.Reg &= ~(0x3 << (set * 2 + 16));
	wp_ctrl.Reg |= (p_attr->level << (set * 2 + 16));

	prot_set_reg(id, DMA_PROTECT_CTRL_REG_OFS, wp_ctrl.Reg);

	// enable region
	if (set > WPSET_3) {
		region_en1.Reg = prot_get_reg(id, DMA_PROTECT_REGION_EN_REG1_OFS);
		for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
			if (p_attr->protect_rgn_attr[i].en) {
				region_en1.Reg |= (1 << ((set - 4) * 8 + i));
			}
		}
		prot_set_reg(id, DMA_PROTECT_REGION_EN_REG1_OFS, region_en1.Reg);
	} else {
		region_en0.Reg = prot_get_reg(id, DMA_PROTECT_REGION_EN_REG0_OFS);
		for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
			if (p_attr->protect_rgn_attr[i].en) {
				region_en0.Reg |= 1 << (set * 8 + i);
			}
		}
		prot_set_reg(id, DMA_PROTECT_REGION_EN_REG0_OFS, region_en0.Reg);
	}

	/*// enable interrupt
	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			intctrl_reg.reg |= 1 << (set * 4 + i);
		}
	}
	arb_set_reg(id, DMA_PROTECT_INTCTRL_REG_OFS, intctrl_reg.reg);*/

	//ddr_arb_platform_spin_unlock(lock);
}

void arb_disable_wp(DDR_ARB id, DMA_WRITEPROT_SET set)
{
	//UINT32 lock;
	union T_DMA_PROTECT_INTSTS_REG sts_reg = {0};
	//T_DMA_PROTECT_INTCTRL_REG intctrl_reg = {0};
	union T_DMA_PROTECT_CTRL_REG wp_ctrl;

	if (id > DDR_ARB_1) {
                printf("invalid arb ch %d\r\n", id);
                return;
        }

        if (set > WPSET_5) {
                printf("invalid protect ch %d\r\n", set);
                return;
        }

	//lock = ddr_arb_platform_spin_lock();

	// disable wp
        wp_ctrl.Reg = prot_get_reg(id, DMA_PROTECT_CTRL_REG_OFS);
        wp_ctrl.Reg &= ~(1<<set);
        prot_set_reg(id, DMA_PROTECT_CTRL_REG_OFS, wp_ctrl.Reg);

        /*// disable ch inten
        intctrl_reg.reg = arb_get_reg(id, DMA_PROTECT_INTCTRL_REG_OFS);
        intctrl_reg.reg &= ~(0xf<<(set * 4));
        arb_set_reg(id, DMA_PROTECT_INTCTRL_REG_OFS, intctrl_reg.reg);*/

        // ensure int sts is cleared
        sts_reg.Reg = 0xf<<(set * 4);
        arb_set_reg(id, DMA_PROTECT_INTSTS_REG_OFS, sts_reg.Reg);

	//ddr_arb_platform_spin_unlock(lock);
}

#else

#define HEAVY_LOAD_CTRL_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_CTRL_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_ADDR_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_START_ADDR_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_SIZE_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_DMA_SIZE_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_WAIT_CYCLE_OFS(ch)   (DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE_OFS + ((ch) * 0x10))

#define PROTECT_START_ADDR_OFS(ch)      (DMA_PROTECT_START_ADDR_REG_OFS+(ch)*0x100)
#define PROTECT_END_ADDR_OFS(ch)        (DMA_PROTECT_END_ADDR_REG_OFS+(ch)*0x100)
#define PROTECT_CH_W_MSK0_OFS(ch)         (DMA_PROTECT_CH0_W_REG_OFS+(ch)*0x100)
#define PROTECT_CH_W_MSK1_OFS(ch)         (DMA_PROTECT_CH1_W_REG_OFS+(ch)*0x100)
#define PROTECT_CH_W_MSK2_OFS(ch)         (DMA_PROTECT_CH2_W_REG_OFS+(ch)*0x100)
#define PROTECT_CH_W_MSK3_OFS(ch)         (DMA_PROTECT_CH3_W_REG_OFS+(ch)*0x100)
#define PROTECT_CH_W_MSK4_OFS(ch)         (DMA_PROTECT_CH4_W_REG_OFS+(ch)*0x100)
#define PROTECT_CH_W_MSK5_OFS(ch)         (DMA_PROTECT_CH5_W_REG_OFS+(ch)*0x100)
#define PROTECT_CH_W_MSK6_OFS(ch)         (DMA_PROTECT_CH6_W_REG_OFS+(ch)*0x100)
#define PROTECT_CH_W_MSK7_OFS(ch)         (DMA_PROTECT_CH7_W_REG_OFS+(ch)*0x100)

#define PROTECT_CH_R_MSK0_OFS(ch)         (DMA_PROTECT_CH0_R_REG_OFS+(ch)*0x100)
#define PROTECT_CH_R_MSK1_OFS(ch)         (DMA_PROTECT_CH1_R_REG_OFS+(ch)*0x100)
#define PROTECT_CH_R_MSK2_OFS(ch)         (DMA_PROTECT_CH2_R_REG_OFS+(ch)*0x100)
#define PROTECT_CH_R_MSK3_OFS(ch)         (DMA_PROTECT_CH3_R_REG_OFS+(ch)*0x100)
#define PROTECT_CH_R_MSK4_OFS(ch)         (DMA_PROTECT_CH4_R_REG_OFS+(ch)*0x100)
#define PROTECT_CH_R_MSK5_OFS(ch)         (DMA_PROTECT_CH5_R_REG_OFS+(ch)*0x100)
#define PROTECT_CH_R_MSK6_OFS(ch)         (DMA_PROTECT_CH6_R_REG_OFS+(ch)*0x100)
#define PROTECT_CH_R_MSK7_OFS(ch)         (DMA_PROTECT_CH7_R_REG_OFS+(ch)*0x100)

#define DETECT_CH_W_MSK0_OFS(ch)         (DMA_DETECT_CH0_W_REG_OFS+(ch)*0x100)
#define DETECT_CH_W_MSK1_OFS(ch)         (DMA_DETECT_CH1_W_REG_OFS+(ch)*0x100)
#define DETECT_CH_W_MSK2_OFS(ch)         (DMA_DETECT_CH2_W_REG_OFS+(ch)*0x100)
#define DETECT_CH_W_MSK3_OFS(ch)         (DMA_DETECT_CH3_W_REG_OFS+(ch)*0x100)
#define DETECT_CH_W_MSK4_OFS(ch)         (DMA_DETECT_CH4_W_REG_OFS+(ch)*0x100)
#define DETECT_CH_W_MSK5_OFS(ch)         (DMA_DETECT_CH5_W_REG_OFS+(ch)*0x100)
#define DETECT_CH_W_MSK6_OFS(ch)         (DMA_DETECT_CH6_W_REG_OFS+(ch)*0x100)
#define DETECT_CH_W_MSK7_OFS(ch)         (DMA_DETECT_CH7_W_REG_OFS+(ch)*0x100)

#define DETECT_CH_R_MSK0_OFS(ch)         (DMA_DETECT_CH0_R_REG_OFS+(ch)*0x100)
#define DETECT_CH_R_MSK1_OFS(ch)         (DMA_DETECT_CH1_R_REG_OFS+(ch)*0x100)
#define DETECT_CH_R_MSK2_OFS(ch)         (DMA_DETECT_CH2_R_REG_OFS+(ch)*0x100)
#define DETECT_CH_R_MSK3_OFS(ch)         (DMA_DETECT_CH3_R_REG_OFS+(ch)*0x100)
#define DETECT_CH_R_MSK4_OFS(ch)         (DMA_DETECT_CH4_R_REG_OFS+(ch)*0x100)
#define DETECT_CH_R_MSK5_OFS(ch)         (DMA_DETECT_CH5_R_REG_OFS+(ch)*0x100)
#define DETECT_CH_R_MSK6_OFS(ch)         (DMA_DETECT_CH6_R_REG_OFS+(ch)*0x100)
#define DETECT_CH_R_MSK7_OFS(ch)         (DMA_DETECT_CH7_R_REG_OFS+(ch)*0x100)



#define IOADDR_DDR_ARB_REG_BASE         IOADDR_DRAM_REG_BASE
#define IOADDR_DDR_ARB2_REG_BASE        IOADDR_DRAM2_REG_BASE

#define ARB_SETREG(ofs,value)           OUTREG32(IOADDR_DDR_ARB_REG_BASE+(ofs),(value))
#define ARB_GETREG(ofs)                 INREG32(IOADDR_DDR_ARB_REG_BASE+(ofs))

#define ARB2_SETREG(ofs,value)          OUTREG32(IOADDR_DDR_ARB2_REG_BASE+(ofs),(value))
#define ARB2_GETREG(ofs)                INREG32(IOADDR_DDR_ARB2_REG_BASE+(ofs))

#define PROT_SETREG(ofs,value)          OUTREG32(IOADDR_DDR_ARB_REG_BASE+0x4300+(ofs),(value))
#define PROT_GETREG(ofs)                INREG32(IOADDR_DDR_ARB_REG_BASE+0x4300+(ofs))

#define PROT2_SETREG(ofs,value)         OUTREG32(IOADDR_DDR_ARB2_REG_BASE+0x4300+(ofs),(value))
#define PROT2_GETREG(ofs)               INREG32(IOADDR_DDR_ARB2_REG_BASE+0x4300+(ofs))

#define REG_LOG                         0

static void debug_dump_addr(UINT64 addr, UINT32 size)
{
	UINT32 i;
	UINT64 j;

	for (j = addr; j < (addr + size); j += 0x10) {
		printf("0x%lx:", j);
		for (i = 0; i < 4; i++) {
			printf("%08x", readl(j + 4 * i));
			printf("  ");
		}
		printf("\r\n");
	}
}


#if 0
static UINT32 arb_get_reg(DDR_ARB id, UINT32 offset)
{
	if (id == DDR_ARB_2) {
		return ARB_GETREG(offset);
	} else {
		return 0;//ARB2_GETREG(offset);
	}
}

static void arb_set_reg(DDR_ARB id, UINT32 offset, REGVALUE value)
{
	if (id == DDR_ARB_2) {
		ARB_SETREG(offset, value);
	} else {
		;//ARB2_SETREG(offset, value);
	}
}
#endif

static UINT32 prot_get_reg(DDR_ARB arb_id, int id, UINT32 offset)
{
	if (id < 6) {
		if (arb_id == DDR_ARB_1) {
#if (REG_LOG)
			_Y_LOG("[GET][0x%0lx]=[0x%08x]\r\n", IOADDR_DDR_ARB_REG_BASE + 0x4300 + offset + id * 0x100, PROT_GETREG(offset + id * 0x100));
#endif
			return PROT_GETREG(offset + id * 0x100);
		} else {
#if (REG_LOG)
			_Y_LOG("[GET][0x%0lx]=[0x%08x]\r\n", IOADDR_DDR_ARB2_REG_BASE + 0x4300 + offset + id * 0x100, PROT_GETREG(offset + id * 0x100));
#endif
			return PROT2_GETREG(offset + id * 0x100);
		}
	} else {
		printf("invalid protect set %d\r\n", id);
		return 0;
	}
}

static void prot_set_reg(DDR_ARB arb_id, int id, UINT32 offset, REGVALUE value)
{
	if (id < 6) {
		if (arb_id == DDR_ARB_1) {
			PROT_SETREG(offset + id * 0x100, value);
#if (REG_LOG)
			_Y_LOG("[SET][0x%lx]=[0x%08x]\r\n", IOADDR_DDR_ARB_REG_BASE + 0x4300 + offset + id * 0x100, PROT_GETREG(offset + id * 0x100));
#endif
		} else {
			PROT2_SETREG(offset + id * 0x100, value);
#if (REG_LOG)
			_Y_LOG("[SET][0x%lx]=[0x%08x]\r\n", IOADDR_DDR_ARB2_REG_BASE + 0x4300 + offset + id * 0x100, PROT_GETREG(offset + id * 0x100));
#endif

		}

	} else {
		printf("invalid protect set %d\r\n", id);
	}
}

void arb_enable_wp(DDR_ARB id, DMA_WRITEPROT_SET set,
				   DMA_WRITEPROT_ATTR *p_attr)
{
	u32 v_mask[DMA_CH_GROUP_CNT];
	union T_DMA_RANGE_SEL_REG range_sel = {0};

	if (id > DDR_ARB_2) {
		printf("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_5) {
		printf("invalid protect ch %d\r\n", set);
		return;
	}

	//printf("arb_enable_wp %d\r\n", id);

	memcpy(v_mask, &p_attr->mask, sizeof(DMA_CH_MSK));

	//debug_dump_addr((UINT32)&p_attr->mask, sizeof(DMA_CH_MSK));

	// disable wp before setting
	range_sel.Reg = prot_get_reg(id, (int)set, DMA_RANGE_SEL_REG_OFS);
	range_sel.Reg = 0;
	prot_set_reg(id, (int)set, DMA_RANGE_SEL_REG_OFS, range_sel.Reg);
	// setup w protected channels
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK0_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK1_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK2_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK3_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK4_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK5_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK6_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK7_OFS(0), 0x0);
	// setup w detected channels
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK0_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK1_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK2_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK3_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK4_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK5_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK6_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK7_OFS(0), 0);

	// setup r protect channels
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK0_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK1_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK2_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK3_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK4_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK5_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK6_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK7_OFS(0), 0x0);
	// setup r detect channels
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK0_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK1_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK2_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK3_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK4_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK5_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK6_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK7_OFS(0), 0);

	printf("arb_enable_wp level %d\r\n", p_attr->level);
	//debug_dump_addr((UINT32)&p_attr->mask, sizeof(DMA_CH_MSK));
	if (p_attr->level == DMA_WPLEL_UNWRITE) {
		// setup w protected channels
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK0_OFS(0), v_mask[DMA_CH_GROUP0]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK1_OFS(0), v_mask[DMA_CH_GROUP1]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK2_OFS(0), v_mask[DMA_CH_GROUP2]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK3_OFS(0), v_mask[DMA_CH_GROUP3]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK4_OFS(0), v_mask[DMA_CH_GROUP4]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK5_OFS(0), v_mask[DMA_CH_GROUP5]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK6_OFS(0), v_mask[DMA_CH_GROUP6]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK7_OFS(0), v_mask[DMA_CH_GROUP7]);

		// setup w detected channels
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK0_OFS(0), v_mask[DMA_CH_GROUP0]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK1_OFS(0), v_mask[DMA_CH_GROUP1]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK2_OFS(0), v_mask[DMA_CH_GROUP2]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK3_OFS(0), v_mask[DMA_CH_GROUP3]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK4_OFS(0), v_mask[DMA_CH_GROUP4]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK5_OFS(0), v_mask[DMA_CH_GROUP5]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK6_OFS(0), v_mask[DMA_CH_GROUP6]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK7_OFS(0), v_mask[DMA_CH_GROUP7]);

		if (p_attr->protect_mode == DMA_PROT_IN) {
			range_sel.Bit.range_sel_w = 0x2;
		} else {
			range_sel.Bit.range_sel_w = 0x5;
		}
	} else if (p_attr->level == DMA_WPLEL_DETECT) {
		// setup w detected channels
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK0_OFS(0), v_mask[DMA_CH_GROUP0]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK1_OFS(0), v_mask[DMA_CH_GROUP1]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK2_OFS(0), v_mask[DMA_CH_GROUP2]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK3_OFS(0), v_mask[DMA_CH_GROUP3]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK4_OFS(0), v_mask[DMA_CH_GROUP4]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK5_OFS(0), v_mask[DMA_CH_GROUP5]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK6_OFS(0), v_mask[DMA_CH_GROUP6]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK7_OFS(0), v_mask[DMA_CH_GROUP7]);

		if (p_attr->protect_mode == DMA_PROT_IN) {
			range_sel.Bit.range_sel_w = 0x2;
		} else {
			range_sel.Bit.range_sel_w = 0x5;
		}

	} else if (p_attr->level == DMA_RPLEL_UNREAD) {
		// setup r protect channels
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK0_OFS(0), v_mask[DMA_CH_GROUP0]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK1_OFS(0), v_mask[DMA_CH_GROUP1]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK2_OFS(0), v_mask[DMA_CH_GROUP2]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK3_OFS(0), v_mask[DMA_CH_GROUP3]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK4_OFS(0), v_mask[DMA_CH_GROUP4]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK5_OFS(0), v_mask[DMA_CH_GROUP5]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK6_OFS(0), v_mask[DMA_CH_GROUP6]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK7_OFS(0), v_mask[DMA_CH_GROUP7]);

		// setup r detect channels
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK0_OFS(0), v_mask[DMA_CH_GROUP0]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK1_OFS(0), v_mask[DMA_CH_GROUP1]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK2_OFS(0), v_mask[DMA_CH_GROUP2]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK3_OFS(0), v_mask[DMA_CH_GROUP3]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK4_OFS(0), v_mask[DMA_CH_GROUP4]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK5_OFS(0), v_mask[DMA_CH_GROUP5]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK6_OFS(0), v_mask[DMA_CH_GROUP6]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK7_OFS(0), v_mask[DMA_CH_GROUP7]);

		if (p_attr->protect_mode == DMA_PROT_IN) {
			range_sel.Bit.range_sel_r = 0x2;
		} else {
			range_sel.Bit.range_sel_r = 0x5;
		}

	} else {
		// setup w protected channels
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK0_OFS(0), v_mask[DMA_CH_GROUP0]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK1_OFS(0), v_mask[DMA_CH_GROUP1]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK2_OFS(0), v_mask[DMA_CH_GROUP2]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK3_OFS(0), v_mask[DMA_CH_GROUP3]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK4_OFS(0), v_mask[DMA_CH_GROUP4]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK5_OFS(0), v_mask[DMA_CH_GROUP5]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK6_OFS(0), v_mask[DMA_CH_GROUP6]);
		prot_set_reg(id, (int)set, PROTECT_CH_W_MSK7_OFS(0), v_mask[DMA_CH_GROUP7]);

		// setup w detected channels
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK0_OFS(0), v_mask[DMA_CH_GROUP0]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK1_OFS(0), v_mask[DMA_CH_GROUP1]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK2_OFS(0), v_mask[DMA_CH_GROUP2]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK3_OFS(0), v_mask[DMA_CH_GROUP3]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK4_OFS(0), v_mask[DMA_CH_GROUP4]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK5_OFS(0), v_mask[DMA_CH_GROUP5]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK6_OFS(0), v_mask[DMA_CH_GROUP6]);
		prot_set_reg(id, (int)set, DETECT_CH_W_MSK7_OFS(0), v_mask[DMA_CH_GROUP7]);

		// setup r protect channels
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK0_OFS(0), v_mask[DMA_CH_GROUP0]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK1_OFS(0), v_mask[DMA_CH_GROUP1]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK2_OFS(0), v_mask[DMA_CH_GROUP2]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK3_OFS(0), v_mask[DMA_CH_GROUP3]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK4_OFS(0), v_mask[DMA_CH_GROUP4]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK5_OFS(0), v_mask[DMA_CH_GROUP5]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK6_OFS(0), v_mask[DMA_CH_GROUP6]);
		prot_set_reg(id, (int)set, PROTECT_CH_R_MSK7_OFS(0), v_mask[DMA_CH_GROUP7]);

		// setup r detect channels
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK0_OFS(0), v_mask[DMA_CH_GROUP0]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK1_OFS(0), v_mask[DMA_CH_GROUP1]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK2_OFS(0), v_mask[DMA_CH_GROUP2]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK3_OFS(0), v_mask[DMA_CH_GROUP3]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK4_OFS(0), v_mask[DMA_CH_GROUP4]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK5_OFS(0), v_mask[DMA_CH_GROUP5]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK6_OFS(0), v_mask[DMA_CH_GROUP6]);
		prot_set_reg(id, (int)set, DETECT_CH_R_MSK7_OFS(0), v_mask[DMA_CH_GROUP7]);


		if (p_attr->protect_mode == DMA_PROT_IN) {
			range_sel.Bit.range_sel_w = 0x2;
			range_sel.Bit.range_sel_r = 0x2;
		} else {
			range_sel.Bit.range_sel_w = 0x5;
			range_sel.Bit.range_sel_r = 0x5;
		}
	}
	// set range
	prot_set_reg(id, (int)set, DMA_RANGE_SEL_REG_OFS, range_sel.Reg);
	//DBG_ERR("range sel %x\r\n", prot_get_reg((int)set, DMA_RANGE_SEL_REG_OFS));

	// set address
	prot_set_reg(id, (int)set, DMA_PROTECT_START_ADDR_REG_OFS, (UINT32)(p_attr->protect_rgn_attr.starting_addr - 1));
	prot_set_reg(id, (int)set, DMA_PROTECT_END_ADDR_REG_OFS, p_attr->protect_rgn_attr.starting_addr + p_attr->protect_rgn_attr.size - 1);

	//debug_dump_addr(IOADDR_DRAM_REG_BASE + 0x4300, 0x890);
}

void arb_disable_wp(DDR_ARB id, DMA_WRITEPROT_SET set)
{
	union T_DMA_RANGE_SEL_REG range_sel = {0};

	if (id > DDR_ARB_2) {
		printf("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_5) {
		printf("invalid protect ch %d\r\n", set);
		return;
	}


	// disable wp
	range_sel.Reg = prot_get_reg(id, (int)set, DMA_RANGE_SEL_REG_OFS);
	range_sel.Reg = 0;
	prot_set_reg(id, (int)set, DMA_RANGE_SEL_REG_OFS, range_sel.Reg);

	// setup w protected channels
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK0_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK1_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK2_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK3_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK4_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK5_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK6_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_W_MSK7_OFS(0), 0x0);
	// setup w detected channels
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK0_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK1_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK2_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK3_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK4_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK5_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK6_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_W_MSK7_OFS(0), 0);

	// setup r protect channels
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK0_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK1_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK2_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK3_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK4_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK5_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK6_OFS(0), 0x0);
	prot_set_reg(id, (int)set, PROTECT_CH_R_MSK7_OFS(0), 0x0);
	// setup r detect channels
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK0_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK1_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK2_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK3_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK4_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK5_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK6_OFS(0), 0);
	prot_set_reg(id, (int)set, DETECT_CH_R_MSK7_OFS(0), 0);
}
#endif
#define TEST_SIZE  0x3000
static int do_tzasc(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	UINT32  selectionBit = 0;
	UINT32  element;
	UINT32  address, address_ofs, address_bit, tzpc_address;
	UINT32  reg;

	chip_id = INREG32(0xF00100F0);
	printf("do_tzpc argc = %d\r\n", argc);
	if (argc < 2) {
		printf("argc %d error\n", argc);
		return -1;
	}

	printf("cmd = %s\r\n", argv[1]);

	printf("chip id = 0x%08x\r\n", chip_id);

	if (strncmp(argv[1], "mem_prot_in_1", 13) == 0) {
		DMA_WRITEPROT_ATTR  protect_attr = {0};
#if defined(CONFIG_TARGET_NS02301)
		u32                 start_addr[6];
#else
		u64                 start_addr[6];
#endif
		BOOL                bResult;
		UINT32              uiSet;
		UINT32              i;
		UINT32              protect_mode;
		UINT32              uiTestSize;
		UINT32              uiTestAddrStart;
		UINT32              test_size = 0x3000;


		uiTestSize = TEST_SIZE * 6;

		uiTestAddrStart = (UINT32)(unsigned long)malloc(uiTestSize);

		if (uiTestAddrStart % 16) {
			uiTestAddrStart = (uiTestAddrStart + 15) & 0xFFFFFFF0;
		}


		_M_LOG(" Memory address = 0x%08x size = 0x%08x\r\n", (int)uiTestAddrStart, (int)uiTestSize);

		start_addr[0] = uiTestAddrStart;
		start_addr[1] = start_addr[0]  + TEST_SIZE;
		start_addr[2] = start_addr[1]  + TEST_SIZE;
		start_addr[3] = start_addr[2]  + TEST_SIZE;
		start_addr[4] = start_addr[3]  + TEST_SIZE;
		start_addr[5] = start_addr[4]  + TEST_SIZE;



		_Y_LOG("addr 0x%lx\n", start_addr[0]);
		_Y_LOG("addr 0x%lx\n", start_addr[1]);
		_Y_LOG("addr 0x%lx\n", start_addr[2]);
		_Y_LOG("addr 0x%lx\n", start_addr[3]);
		_Y_LOG("addr 0x%lx\n", start_addr[4]);
		_Y_LOG("addr 0x%lx\n", start_addr[5]);

#ifdef CONFIG_TARGET_NS02302_A64
		//printf("====================  mode%d, %s ===================\r\n", (int)protect_mode, is_out ? "out range" : "in range");
		protect_attr.mask.CPU_NS = TRUE;
//		protect_attr.mask.CPU_S = TRUE;
		protect_attr.level = DMA_WPLEL_UNWRITE;
		protect_attr.size = TEST_SIZE;
		protect_attr.starting_addr = start_addr[0];
#elif defined(CONFIG_TARGET_NS02301)
		protect_attr.mask.CPU_NS = TRUE;
//		protect_attr.mask.CPU = TRUE;
		protect_attr.level = DMA_WPLEL_UNWRITE;
		protect_attr.protect_mode = 0;
		protect_attr.protect_rgn_attr[0].en = ENABLE;
		protect_attr.protect_rgn_attr[0].size = TEST_SIZE;
		protect_attr.protect_rgn_attr[0].starting_addr = start_addr[0];
#else
		//printf("====================  mode%d, %s ===================\r\n", (int)protect_mode, is_out ? "out range" : "in range");
		protect_attr.mask.CPU_NS = TRUE;
//		protect_attr.mask.CPU_S = TRUE;
		protect_attr.level = DMA_WPLEL_UNWRITE;
		protect_attr.protect_mode = 0;
		protect_attr.protect_rgn_attr.en = ENABLE;
		protect_attr.protect_rgn_attr.size = TEST_SIZE;
		protect_attr.protect_rgn_attr.starting_addr = start_addr[0];
#endif
		//memset((void *)(start_addr[0]-0x3000), 0x96, 0x3000);
		memset((void *)start_addr[0], 0x96, TEST_SIZE);
		memset((void *)start_addr[1], 0x96, TEST_SIZE);
		memset((void *)start_addr[2], 0x96, TEST_SIZE);
		memset((void *)start_addr[3], 0x96, TEST_SIZE);
		memset((void *)start_addr[4], 0x96, TEST_SIZE);
		memset((void *)start_addr[5], 0x96, TEST_SIZE);

		flush_dcache_range((uintptr_t)uiTestAddrStart, (uintptr_t)uiTestAddrStart + roundup(uiTestSize, uiTestSize));

		_G_LOG("==========[addr 0]================\n");
		debug_dump_addr(start_addr[0], 0x80);
		_G_LOG("==========[addr 1]================\n");
		debug_dump_addr(start_addr[1], 0x80);
		_G_LOG("==========[addr 2]================\n");
		debug_dump_addr(start_addr[2], 0x80);
		_G_LOG("==========[addr 3]================\n");
		debug_dump_addr(start_addr[3], 0x80);
		_G_LOG("==========[addr 4]================\n");
		debug_dump_addr(start_addr[4], 0x80);
		_G_LOG("==========[addr 5]================\n");
		debug_dump_addr(start_addr[5], 0x80);

#ifdef CONFIG_TARGET_NS02302_A64
		arb_enable_wp(DDR_ARB_1, 0, &protect_attr);
		protect_attr.starting_addr = (start_addr[1]);
		arb_enable_wp(DDR_ARB_1, 1, &protect_attr);
		protect_attr.starting_addr = (start_addr[2]);
		arb_enable_wp(DDR_ARB_1, 2, &protect_attr);
		protect_attr.starting_addr = (start_addr[3]);
		arb_enable_wp(DDR_ARB_1, 3, &protect_attr);
		protect_attr.starting_addr = (start_addr[4]);
		arb_enable_wp(DDR_ARB_1, 4, &protect_attr);
		protect_attr.starting_addr = (start_addr[5]);
		arb_enable_wp(DDR_ARB_1, 5, &protect_attr);
#elif defined(CONFIG_TARGET_NS02301)
		arb_enable_wp(DDR_ARB_1, 0, &protect_attr);
		protect_attr.protect_rgn_attr[0].starting_addr = (start_addr[1]);
		arb_enable_wp(DDR_ARB_1, 1, &protect_attr);
		protect_attr.protect_rgn_attr[0].starting_addr = (start_addr[2]);
		arb_enable_wp(DDR_ARB_1, 2, &protect_attr);
		protect_attr.protect_rgn_attr[0].starting_addr = (start_addr[3]);
		arb_enable_wp(DDR_ARB_1, 3, &protect_attr);
		protect_attr.protect_rgn_attr[0].starting_addr = (start_addr[4]);
		arb_enable_wp(DDR_ARB_1, 4, &protect_attr);
		protect_attr.protect_rgn_attr[0].starting_addr = (start_addr[5]);
		arb_enable_wp(DDR_ARB_1, 5, &protect_attr);
#else
		arb_enable_wp(DDR_ARB_1, 0, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[1]);
		arb_enable_wp(DDR_ARB_1, 1, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[2]);
		arb_enable_wp(DDR_ARB_1, 2, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[3]);
		arb_enable_wp(DDR_ARB_1, 3, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[4]);
		arb_enable_wp(DDR_ARB_1, 4, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[5]);
		//arb_enable_wp(DDR_ARB_1, 5, &protect_attr);
#endif
		memset((void *)start_addr[0], 0x5a, TEST_SIZE);

		memset((void *)start_addr[1], 0xa5, TEST_SIZE);
		memset((void *)start_addr[2], 0xaa, TEST_SIZE);
		memset((void *)start_addr[3], 0xbb, TEST_SIZE);
		memset((void *)start_addr[4], 0xcc, TEST_SIZE);
		memset((void *)start_addr[5], 0xdd, TEST_SIZE);

		flush_dcache_range((uintptr_t)uiTestAddrStart, (uintptr_t)(uiTestAddrStart + roundup(uiTestSize, uiTestSize)));

		_Y_LOG("==========[addr 0]================\n");
		debug_dump_addr(start_addr[0], 0x80);
		_Y_LOG("==========[addr 1]================\n");
		debug_dump_addr(start_addr[1], 0x80);
		_Y_LOG("==========[addr 2]================\n");
		debug_dump_addr(start_addr[2], 0x80);
		_Y_LOG("==========[addr 3]================\n");
		debug_dump_addr(start_addr[3], 0x80);
		_Y_LOG("==========[addr 4]================\n");
		debug_dump_addr(start_addr[4], 0x80);
		_Y_LOG("==========[addr 5]================\n");
		debug_dump_addr(start_addr[5], 0x80);

		arb_disable_wp(DDR_ARB_1, 0);
		arb_disable_wp(DDR_ARB_1, 1);
		arb_disable_wp(DDR_ARB_1, 2);
		arb_disable_wp(DDR_ARB_1, 3);
		arb_disable_wp(DDR_ARB_1, 4);
		arb_disable_wp(DDR_ARB_1, 5);
	} else if (strncmp(argv[1], "mem_prot_in_2", 13) == 0) {
#ifdef CONFIG_TARGET_NS02302_A64
#elif defined(CONFIG_TARGET_NS02301)
#else
		DMA_WRITEPROT_ATTR  protect_attr = {0};
		u64                 start_addr[6];
		BOOL                bResult;
		UINT32              uiSet;
		UINT32              i;
		UINT32              protect_mode;
		UINT32              uiTestSize;
		UINT32              uiTestAddrStart;
		UINT32              test_size = 0x3000;
		uintptr_t           dram2_offset = 0x0;




		uiTestSize = TEST_SIZE * 6;
		uiTestAddrStart = 0x80001000;
		//uiTestAddrStart = (UINT32)(unsigned long)malloc(uiTestSize);
		if (((INREG32(IOADDR_TOP_REG_BASE + 0x20) >> 4) & 0xF) == 0x0) {
			dram2_offset = 0x100000000;
		} else if (((INREG32(IOADDR_TOP_REG_BASE + 0x20) >> 4) & 0xF) == 0x1) {
			dram2_offset = 0x10000000;
		} else if (((INREG32(IOADDR_TOP_REG_BASE + 0x20) >> 4) & 0xF) == 0x2) {
			dram2_offset = 0x20000000;
		} else if (((INREG32(IOADDR_TOP_REG_BASE + 0x20) >> 4) & 0xF) == 0x4) {
			dram2_offset = 0x40000000;
		} else if (((INREG32(IOADDR_TOP_REG_BASE + 0x20) >> 4) & 0xF) == 0x8) {
			dram2_offset = 0x80000000;
		} else {
			_Y_LOG("TOP error 0x%lx\n", INREG32(IOADDR_TOP_REG_BASE + 0x20));
		}
		if (uiTestAddrStart % 16) {
			uiTestAddrStart = (uiTestAddrStart + 15) & 0xFFFFFFF0;
		}


		_M_LOG(" Memory address = 0x%08x size = 0x%08x\r\n", (int)uiTestAddrStart, (int)uiTestSize);

		start_addr[0] = uiTestAddrStart - dram2_offset;
		start_addr[1] = start_addr[0]  + TEST_SIZE;
		start_addr[2] = start_addr[1]  + TEST_SIZE;
		start_addr[3] = start_addr[2]  + TEST_SIZE;
		start_addr[4] = start_addr[3]  + TEST_SIZE;
		start_addr[5] = start_addr[4]  + TEST_SIZE;



		_Y_LOG("addr 0x%lx\n", start_addr[0]);
		_Y_LOG("addr 0x%lx\n", start_addr[1]);
		_Y_LOG("addr 0x%lx\n", start_addr[2]);
		_Y_LOG("addr 0x%lx\n", start_addr[3]);
		_Y_LOG("addr 0x%lx\n", start_addr[4]);
		_Y_LOG("addr 0x%lx\n", start_addr[5]);

		//printf("====================  mode%d, %s ===================\r\n", (int)protect_mode, is_out ? "out range" : "in range");
		protect_attr.mask.CPU_NS = TRUE;
//		protect_attr.mask.CPU_S = TRUE;
		protect_attr.level = DMA_WPLEL_UNWRITE;
		protect_attr.protect_mode = 0;
		protect_attr.protect_rgn_attr.en = ENABLE;
		protect_attr.protect_rgn_attr.size = TEST_SIZE;
		protect_attr.protect_rgn_attr.starting_addr = start_addr[0];

		//memset((void *)(start_addr[0]-0x3000), 0x96, 0x3000);
		memset((void *)start_addr[0] + dram2_offset, 0x96, TEST_SIZE);
		memset((void *)start_addr[1] + dram2_offset, 0x96, TEST_SIZE);
		memset((void *)start_addr[2] + dram2_offset, 0x96, TEST_SIZE);
		memset((void *)start_addr[3] + dram2_offset, 0x96, TEST_SIZE);
		memset((void *)start_addr[4] + dram2_offset, 0x96, TEST_SIZE);
		memset((void *)start_addr[5] + dram2_offset, 0x96, TEST_SIZE);

		flush_dcache_range((uintptr_t)uiTestAddrStart, (uintptr_t)uiTestAddrStart + roundup(uiTestSize, uiTestSize));

		_G_LOG("==========[addr 0]================\n");
		debug_dump_addr(start_addr[0] + dram2_offset, 0x80);
		_G_LOG("==========[addr 1]================\n");
		debug_dump_addr(start_addr[1] + dram2_offset, 0x80);
		_G_LOG("==========[addr 2]================\n");
		debug_dump_addr(start_addr[2] + dram2_offset, 0x80);
		_G_LOG("==========[addr 3]================\n");
		debug_dump_addr(start_addr[3] + dram2_offset, 0x80);
		_G_LOG("==========[addr 4]================\n");
		debug_dump_addr(start_addr[4] + dram2_offset, 0x80);
		_G_LOG("==========[addr 5]================\n");
		debug_dump_addr(start_addr[5] + dram2_offset, 0x80);

		arb_enable_wp(DDR_ARB_2, 0, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[1]);
		arb_enable_wp(DDR_ARB_2, 1, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[2]);
		arb_enable_wp(DDR_ARB_2, 2, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[3]);
		arb_enable_wp(DDR_ARB_2, 3, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[4]);
		arb_enable_wp(DDR_ARB_2, 4, &protect_attr);
		protect_attr.protect_rgn_attr.starting_addr = (start_addr[5]);
		arb_enable_wp(DDR_ARB_2, 5, &protect_attr);
		memset((void *)start_addr[0] + dram2_offset, 0x5a, TEST_SIZE);

		memset((void *)start_addr[1] + dram2_offset, 0xa5, TEST_SIZE);
		memset((void *)start_addr[2] + dram2_offset, 0xaa, TEST_SIZE);
		memset((void *)start_addr[3] + dram2_offset, 0xbb, TEST_SIZE);
		memset((void *)start_addr[4] + dram2_offset, 0xcc, TEST_SIZE);
		memset((void *)start_addr[5] + dram2_offset, 0xdd, TEST_SIZE);

		flush_dcache_range((uintptr_t)uiTestAddrStart, (uintptr_t)(uiTestAddrStart + roundup(uiTestSize, uiTestSize)));

		_Y_LOG("==========[addr 0]================\n");
		debug_dump_addr(start_addr[0] + dram2_offset, 0x80);
		_Y_LOG("==========[addr 1]================\n");
		debug_dump_addr(start_addr[1] + dram2_offset, 0x80);
		_Y_LOG("==========[addr 2]================\n");
		debug_dump_addr(start_addr[2] + dram2_offset, 0x80);
		_Y_LOG("==========[addr 3]================\n");
		debug_dump_addr(start_addr[3] + dram2_offset, 0x80);
		_Y_LOG("==========[addr 4]================\n");
		debug_dump_addr(start_addr[4] + dram2_offset, 0x80);
		_Y_LOG("==========[addr 5]================\n");
		debug_dump_addr(start_addr[5] + dram2_offset, 0x80);

		arb_disable_wp(DDR_ARB_2, 0);
		arb_disable_wp(DDR_ARB_2, 1);
		arb_disable_wp(DDR_ARB_2, 2);
		arb_disable_wp(DDR_ARB_2, 3);
		arb_disable_wp(DDR_ARB_2, 4);
		arb_disable_wp(DDR_ARB_2, 5);
#endif
	}
	return 0;
}

U_BOOT_CMD(nvt_tzasc, 3, 0, do_tzasc,
		   "tzasc emulation cmd:",
		   "[Option] \n"
		   "       [mem_prot_in_1] :  mem protect in @ dram1 @ non secure world\n"
		   "       [mem_prot_in_2] :  mem protect in @ dram2 @ non secure world\n"
		  );

