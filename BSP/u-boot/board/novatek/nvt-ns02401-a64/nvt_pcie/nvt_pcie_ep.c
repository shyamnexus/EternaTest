#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/io.h>
#include <stdlib.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/nvt_pcie.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/efuse_protected.h>
#include <pci.h>
#include <generic-phy.h>
#include <dm.h>
//#include <dm/uclass.h>
#include <linux/delay.h>

//#include "nvt_pcie_fdt.h"

#define DRV_VER	"1.00"

enum nvt_pcie_phy_id {
	NVT_PHY_PCIE0 = 0,
	NVT_PHY_PCIE1 = 1,
	NVT_PHY_PCIE2 = 2,
	NVT_PHY_PCIE3 = 3,
	MAX_NUM_PHYS,
};

enum nvt_pcie_phy_mode {
	NVT_PHY_ONE_x4 = 0,
	NVT_PHY_TWO_x2 = 1,
	NVT_PHY_FOUR_x1 = 2,
	NVT_PHY_ONE_x2_TWO_x1 = 3,
	MAX_NUM_PHY_MODE,
};

#define CC_RC_EP_BOOT_COMM_REG      	0x4F0110120

#define CLOCK_GEN_ENABLE_REG3		(IOADDR_CG_REG_BASE + 0x7C)
#define CLOCK_GEN_RESET_REG6		(IOADDR_CG_REG_BASE + 0xA8)

static void pcie_set_inbound_region(int bar_id, u64 src_addr, u64 src_upper_addr, u64 target_addr, int region_id, int en)
{
	const uintptr_t BASE = PCIE_DBI_REG_BASE + PF0_ATU_CAP_DBIBaseAddress;

	writel(lower_32_bits(src_addr), BASE+IATU_LWR_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(upper_32_bits(src_addr), BASE+IATU_UPPER_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(lower_32_bits(src_upper_addr), BASE+IATU_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(upper_32_bits(src_upper_addr), BASE+IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(lower_32_bits(target_addr), BASE+IATU_LWR_TARGET_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(upper_32_bits(target_addr), BASE+IATU_UPPER_TARGET_ADDR_OFF_INBOUND_0 + region_id*0x200);
	writel(0x00000000, BASE+IATU_REGION_CTRL_1_OFF_INBOUND_0 + region_id*0x200); //[4:0]00000 Memory
	writel((en<<31)|(1<<30)|(bar_id<<8), BASE+IATU_REGION_CTRL_2_OFF_INBOUND_0 + region_id*0x200); //[31]REGION_EN [30] MATCH_MODE [28]CFG_SHIFT_MODE [10..8] BAR_NUM
}

static void pcie_set_outbound_region(u64 src_addr, u64 src_upper_addr, u64 target_addr, int region_id, int atu_type, int en) {
	const uintptr_t BASE = PCIE_DBI_REG_BASE + PF0_ATU_CAP_DBIBaseAddress;

	writel(lower_32_bits(src_addr), BASE+IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(upper_32_bits(src_addr), BASE+IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(lower_32_bits(src_upper_addr), BASE+IATU_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(upper_32_bits(src_upper_addr), BASE+IATU_UPPER_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(lower_32_bits(target_addr), BASE+IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(upper_32_bits(target_addr), BASE+IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(atu_type, BASE+IATU_REGION_CTRL_1_OFF_OUTBOUND_0 + region_id*0x200); //[4:0]00100 CFG Type0

	writel((en<<31), BASE+IATU_REGION_CTRL_2_OFF_OUTBOUND_0 + region_id*0x200); //[31]REGION_EN [28]CFG_SHIFT_MODE
}

static void pcie_set_atu(void)
{
	// EP inbound regions
	pcie_set_inbound_region(0, 0x0, 0x10000000, 0x000000000ULL, 0, TRUE);	// BAR0 (MAU): map to internal addr 0
	pcie_set_inbound_region(2, 0x0, 0x10000000, 0x2F0000000ULL, 1, TRUE);	// BAR2 (APB): map to internal addr 0x2_F000_0000

	// EP outbound regions
	pcie_set_outbound_region(0xC00000000ULL, 0xDFFFFFFFFULL, 0xC00000000, 0, PCIE_ATU_TYPE_MEM, TRUE);
	pcie_set_outbound_region(0xE00000000ULL, 0xE0FFFFFFFULL, 0xE00000000, 1, PCIE_ATU_TYPE_MEM, TRUE);
}


#define DETECT_QUIET	0x00
#define RCVRY_LOCK	0x0D
#define RECVRY_EQ0	0x20
#define RECVRY_EQ1	0x21

#define FOURCC_BOOT 0x544F4F42
static int utl_pcie_ep_load(void)
{
	u16	calc_sum = 0;
	u32	secure_boot;
	u32	last_ltssm = DETECT_QUIET;
	u32	state_start_us = 0;	// unit: us
	int	is_reset_done = 0;

	printf("%s: curr tick 0x%x\r\n", __func__, get_nvt_timer0_cnt());

	flush_dcache_all();
	// Enable link training (Start LTSSM)
	writel(0x35, PCIE_TOP_REG_BASE+0x304);

	// Never break loop
	while (1) {
		u32 reg;
		u32 curr_ltssm;
		volatile unsigned int *p_comm_reg = (volatile unsigned int *)CC_RC_EP_BOOT_COMM_REG;

		if (*p_comm_reg == FOURCC_BOOT) break;

		reg = readl(PCIE_TOP_REG_BASE+0x08);
		curr_ltssm = (reg>>4) & 0x3F;
		if (last_ltssm != curr_ltssm) {
			if (last_ltssm == RECVRY_EQ0) {
//				printf("EQ0 elps %d\r\n", timer_getSysTick()-state_start_us);
			}
			state_start_us = get_nvt_timer0_cnt();
			is_reset_done = 0;
		}

		switch (curr_ltssm) {
		case RECVRY_EQ0:
			break;
		default:
			break;
		}

		last_ltssm = curr_ltssm;
	}

	printf("%s: get RC continue command\r\n", __func__);

	return 0;
}

static int phy_drv_init(struct udevice **devp)
{
	struct udevice *bus;

	/*
	 * Enumerate all known controller devices. Enumeration has the side-
	 * effect of probing them, so PCIe devices will be enumerated too.
	 */
	for (uclass_first_device_check(UCLASS_PHY, &bus);
	     bus;
	     uclass_next_device_check(&bus)) {
		printf("%s: uclass name %s\r\n", __func__, bus->name);
		if (strncmp(bus->name, "phy@4,f03b0000", strlen("phy@4,f03b0000")) == 0) {
			*devp = bus;
			printf("%s: return ptr 0x%p\r\n", __func__, bus);
			return 0;
		}
		if (strncmp(bus->name, "phy@4,f03c0000", strlen("phy@4,f03c0000")) == 0) {
			*devp = bus;
			printf("%s: return ptr 0x%p\r\n", __func__, bus);
			return 0;
		}
	}

	return -1;
}


static int config_pcie_ep(void) {
	int width;
	int ret;
	u32 val;
	UINT32 reg;
	UINT32 bootstrap;
	UINT32 lane_mode;
	UINT32 preset;
	u32 PCIE_VENDOR_ID = 0x1E09;	// novatek
	u32 PCIE_DEV_ID = 0x0007;	// NT98635
	u32 BAR0_SIZE = 5;		// 2^5 MB
	enum nvt_pcie_phy_mode mode;
	struct phy phy0 = {0};
	struct udevice *dev = NULL;

	phy_drv_init(&dev);
	phy0.dev = dev;

	bootstrap = readl(IOADDR_TOP_REG_BASE);
	lane_mode = (bootstrap&BOOT_SOURCE_PCIE_LANE_MSK) >> BOOT_SOURCE_PCIE_LANE_SHF;
	switch (lane_mode) {
	case NVT_PHY_ONE_x4:
		printf("%s: bootstrap: one x4\r\n", __func__);
		mode = NVT_PHY_ONE_x4;
		width = 4;
		break;
	case NVT_PHY_TWO_x2:
		printf("%s: bootstrap: two x2\r\n", __func__);
		mode = NVT_PHY_TWO_x2;
		width = 2;
		break;
	case NVT_PHY_FOUR_x1:
		printf("%s: bootstrap: four x2\r\n", __func__);
		mode = NVT_PHY_FOUR_x1;
		width = 1;
		break;
	case NVT_PHY_ONE_x2_TWO_x1:
	default:
		printf("%s: bootstrap: one x2 and two x1\r\n", __func__);
		mode = NVT_PHY_ONE_x2_TWO_x1;
		width = 2;
		break;
	}
	generic_phy_configure(&phy0, &mode);

	reg = readl(IOADDR_CG_REG_BASE+0x00);
	writel(reg|(1<<2), IOADDR_CG_REG_BASE+0x00);
	reg = readl(IOADDR_CG_REG_BASE+0x00);
	writel(reg|(1<<13), IOADDR_CG_REG_BASE+0x00);

	reg = readl(IOADDR_TOP_REG_BASE + 0x60);
	reg |= 1<<0;	// EP mode
	writel(reg, IOADDR_TOP_REG_BASE + 0x60);

	// enable PCIe clock
	reg = readl(CLOCK_GEN_ENABLE_REG3);
	writel(reg|(0x01 << 24), CLOCK_GEN_ENABLE_REG3);

	// Apply best setting before PORn
	generic_phy_reset(&phy0);
	generic_phy_init(&phy0);
	generic_phy_power_on(&phy0);

	// release phy PORn
	reg = readl(CLOCK_GEN_RESET_REG6);
	writel(reg|(1<<20), CLOCK_GEN_RESET_REG6);

	// 1. stop LTSSM
	writel(0x30, PCIE_TOP_REG_BASE+0x304);

	// 2. release EP register reset (for access DBI)
	val = readl(PCIE_TOP_REG_BASE+0x300);
	val = 1<<3;
	writel(val, PCIE_TOP_REG_BASE+0x300);
	udelay(2);			// wait stable
	val = readl(PCIE_TOP_REG_BASE+0x300);
	val |= 0x1F;
	writel(val, PCIE_TOP_REG_BASE+0x300);
	udelay(2);			// wait stable

	// 3. update DBI default value
	val = readl(PCIE_DBI_REG_BASE+0x8BC);	// DBI overwrite
	val |= 1<<0;
	writel(val, PCIE_DBI_REG_BASE+0x8BC);

#if defined(CONFIG_NVT_PCIE_EP_BAR0_SIZE)
	BAR0_SIZE = CONFIG_NVT_PCIE_EP_BAR0_SIZE;
	printf("%s: define BAR0 %d\r\n", __func__, BAR0_SIZE);
	if (BAR0_SIZE > 13) BAR0_SIZE = 5;	// default 32 MB
#endif
	val = readl(PCIE_DBI_REG_BASE+0x2F4);	// RESBAR_CTRL_REG_0_REG
	val &= ~(0x3F<<8);
	val |= (BAR0_SIZE<<8);		// 2^8 MB => 32MB
	writel(val, PCIE_DBI_REG_BASE+0x2F4);
	writel(0xFF0, PCIE_DBI_REG_BASE+0x2F0);

#if defined(CONFIG_NVT_PCIE_EP_VENDOR_ID)
	PCIE_VENDOR_ID = CONFIG_NVT_PCIE_EP_VENDOR_ID;
	printf("%s: defined vendor id = 0x%x\r\n", __func__, PCIE_VENDOR_ID);
#endif
#if defined(CONFIG_NVT_PCIE_EP_DEV_ID)
	PCIE_DEV_ID = CONFIG_NVT_PCIE_EP_DEV_ID;
	printf("%s: defined dev id = 0x%x\r\n", __func__, PCIE_DEV_ID);
#endif
	val = PCIE_VENDOR_ID & 0xFFFF;
	val |= (PCIE_DEV_ID&0xFFFF)<<16;
	writel(val, PCIE_DBI_REG_BASE);

#if (!IS_ENABLED(CONFIG_NVT_PCIE_EP_BAR2_EN))
	printf("Disable BAR2\r\n");
	writel(0, PCIE_DBI_REG_BASE+0x100018);
	writel(0, PCIE_DBI_REG_BASE+0x10001C);
#else
	printf("BAR2 enabled\r\n");
#endif

	// modify BAR2 to prefetchable
	reg = readl(PCIE_DBI_REG_BASE + 0x18);
	reg |= 1<<3;
	writel(reg, PCIE_DBI_REG_BASE + 0x18);
	// modify BAR4 to prefetchable
	reg = readl(PCIE_DBI_REG_BASE + 0x20);
	reg |= 1<<3;
	writel(reg, PCIE_DBI_REG_BASE + 0x20);

	// fix preset
	preset = 4;
	printf("Fix Preset %d\r\n", preset);
	reg = readl(PCIE_DBI_REG_BASE+0x08A8);   // force EQ preset 4
	reg &= ~(1<<24);
	reg &= ~(0xFFFF<<8);
	reg |= ((1<<preset))<<8;
	writel(reg, PCIE_DBI_REG_BASE+0x08A8);

	// ONLY support ASPM L1
	reg = readl(PCIE_DBI_REG_BASE+0x007C);
	reg &= ~(0x03<<10);
	reg |= 0x2<<10;
	writel(reg, PCIE_DBI_REG_BASE+0x007C);

	reg = readl(PCIE_DBI_REG_BASE + 0x708);	// PORT_FORCE_OFF
	reg |= 1<<22;		// auto solve broken lane: go to config if any lane receives TS1
	writel(reg, PCIE_DBI_REG_BASE + 0x708);

	switch (lane_mode) {
	case NVT_PHY_ONE_x4:
		reg = readl(PCIE_DBI_REG_BASE + 0x80C);	// GEN2_CTRL_OFF
		reg &= ~(0x1F<<8);
		reg |= 4<<8;		// NUM_OF_LANES
		writel(reg, PCIE_DBI_REG_BASE + 0x80C);

		reg = readl(PCIE_DBI_REG_BASE + 0x710);	// PORT_LINK_CTRL_OFF
		reg &= ~(0x3F<<16);
		reg |= 0x7<<16;		// x4
		writel(reg, PCIE_DBI_REG_BASE + 0x710);
		break;
	case NVT_PHY_TWO_x2:
	case NVT_PHY_ONE_x2_TWO_x1:
		reg = readl(PCIE_DBI_REG_BASE + 0x80C);	// GEN2_CTRL_OFF
		reg &= ~(0x1F<<8);
		reg |= 2<<8;		// NUM_OF_LANES
		writel(reg, PCIE_DBI_REG_BASE + 0x80C);

		reg = readl(PCIE_DBI_REG_BASE + 0x710);	// PORT_LINK_CTRL_OFF
		reg &= ~(0x3F<<16);
		reg |= 0x3<<16;		// x2
		writel(reg, PCIE_DBI_REG_BASE + 0x710);
		break;
	case NVT_PHY_FOUR_x1:
	default:
		reg = readl(PCIE_DBI_REG_BASE + 0x80C);	// GEN2_CTRL_OFF
		reg &= ~(0x1F<<8);
		reg |= 1<<8;		// NUM_OF_LANES
		writel(reg, PCIE_DBI_REG_BASE + 0x80C);

		reg = readl(PCIE_DBI_REG_BASE + 0x710);	// PORT_LINK_CTRL_OFF
		reg &= ~(0x3F<<16);
		reg |= 0x1<<16;		// x1
		writel(reg, PCIE_DBI_REG_BASE + 0x710);
		break;
	}
	printf("DBI 0x274 = 0x%x\r\n", readl(PCIE_DBI_REG_BASE + 0x274));

	// 4. setup iATU
	pcie_set_atu();

	// 5. phy settings
//	pcie_phy_settings();

	utl_pcie_ep_load();
	printf("%s: end\r\n", __func__);
}


/* this function is portable for the customer */
int do_pcie_ep (struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	printk("-------------- PCIe EP Version: %s -------------- \n", DRV_VER);

	config_pcie_ep();

	return 0;
}


U_BOOT_CMD(
	nvt_pcie_ep_init,	2,	1,	do_pcie_ep,
	"setup pcie ep mode",
	"command: nvt_pcie_ep_init"
);
