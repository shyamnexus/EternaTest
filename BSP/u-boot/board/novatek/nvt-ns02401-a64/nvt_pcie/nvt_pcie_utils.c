#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <stdlib.h>
#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/nvt_pcie.h>
#include <linux/bitfield.h>

#define PCIE_MAPPING_BASE		0x800000000
#define PCIE_SRAM_BASE			0x802D40000

#define PCIE_SRAM_BASE			0x802D40000
#define EP_IFE1_SRAM_CLK_ADDR_NS02401	0x802020070
#define IFE1_CLK_EN_NS02401		BIT(26)
#define EP_IFE2_SRAM_CLK_ADDR_NS02401	0x802020074
#define IFE2_CLK_EN_NS02401		BIT(4)
#define EP_DDR1_RESET_ADDR_NS02401	0x802020090
#define EP_DDR1_RESET_VALUE_NS02401	~FIELD_PREP(GENMASK(9,8), 0x3)
#define EP_DDR2_RESET_ADDR_NS02401	0x8020200A0
#define EP_DDR2_RESET_VALUE_NS02401	~FIELD_PREP(GENMASK(26,25), 0x3)

#define EP_LOADER_SIZE			0x10000
#define OUTBOUND_SIZE			0x2000000

#define CC_RC_EP_BOOT_COMM_REG		0x802110120


#define PCIE_STATUS_IDLE		0x544F4F42
#define PCIE_STATUS_DRAM_DONE		0x5944524C

static int nvt_change_ep_inbound(unsigned long dest, unsigned long size)
{
	if (size > OUTBOUND_SIZE) {
		nvt_dbg(MSG, "Size is over %x\n", OUTBOUND_SIZE);
		return -1;
	}

	writel(0x0, 0x803000104);
	writel(0x2000, 0x803000100);
	writel(0xC0000000, 0x803000108);
	writel(0x0, 0x80300010C);
	writel(0xC1FFFFFF, 0x803000110);
	writel(dest & GENMASK(31,0), 0x803000114);
	writel(dest >> 32, 0x803000118);
	writel(0x0, 0x80300011C);
	writel(0x0, 0x803000120);
	writel(0x80000000, 0x803000104);
	return 0;
}

static int do_nvt_pcie_copy_sram(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int eploader_size = EP_LOADER_SIZE;
	unsigned int *p_src = (unsigned int *)simple_strtoul(argv[1], NULL, 16);
	unsigned int *p_dst = (unsigned int *)(PCIE_SRAM_BASE);
	unsigned int reg;
	int i;

	reg = readl(EP_IFE1_SRAM_CLK_ADDR_NS02401);
	reg |= IFE1_CLK_EN_NS02401;
	writel(reg, EP_IFE1_SRAM_CLK_ADDR_NS02401);

	reg = readl(EP_IFE2_SRAM_CLK_ADDR_NS02401);
	reg |= IFE2_CLK_EN_NS02401;
	writel(reg, EP_IFE2_SRAM_CLK_ADDR_NS02401);

	memcpy(p_dst, p_src, eploader_size);
	return 0;
}

U_BOOT_CMD(
	nvt_pcie_copy_sram, 2,    1,      do_nvt_pcie_copy_sram,
	"nvt_pcie_copy_sram [eploader addre]",
	""
);

static int do_nvt_pcie_boot(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	writel(EP_DDR1_RESET_VALUE_NS02401, EP_DDR1_RESET_ADDR_NS02401);
	writel(EP_DDR2_RESET_VALUE_NS02401, EP_DDR2_RESET_ADDR_NS02401);
	return 0;
}

U_BOOT_CMD(
	nvt_pcie_boot, 1,    1,      do_nvt_pcie_boot,
	"trigger pcie CPU",
	""
);

static char epstatus_help_text[] =
	"nvt_pcie_epstatus get - get current ep status\n"
	"nvt_pcie_epstatus set idle - set communication register to idle\n"
	"nvt_pcie_epstatus set start [epfdt address] - set ep fdt address to communication register\n"
	"";

static int do_nvt_pcie_epstatus(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = -1;
	u32 value;
	u32 status;
	enum {
		IDLE = 0,
		DRAM_DONE,
	};

	if (argc == 2) {
		status = readl(CC_RC_EP_BOOT_COMM_REG);
		switch (status) {
		case PCIE_STATUS_IDLE :
			nvt_dbg(MSG, "EP Idle \n");
			ret = IDLE;
			break;
		case PCIE_STATUS_DRAM_DONE :
			nvt_dbg(MSG, "EP Dram config done \n");
			ret = DRAM_DONE;
			break;
		default:
			nvt_dbg(MSG, "EP on unknow status\n");
			ret = -1;
			break;
		}
	} else if (argc == 3) {
		if (strcmp(argv[2], "idle") == 0) {
			writel(PCIE_STATUS_IDLE, CC_RC_EP_BOOT_COMM_REG);
			ret = 0;
		}
	} else if (argc == 4) {
		if (strcmp(argv[2], "start") == 0) {
			value = (u32)simple_strtoul(argv[3], NULL, 16);
			nvt_dbg(MSG, "write start value 0x%lx\n", value);
			writel(value, CC_RC_EP_BOOT_COMM_REG);
			ret = 0;
		}
	}

	return ret;
}

U_BOOT_CMD(
	nvt_pcie_epstatus, 4,    1,      do_nvt_pcie_epstatus,
	"get or set epstatus on boot ep flow",
	epstatus_help_text
);

static int do_nvt_pcie_copy(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long *src = (unsigned long *)simple_strtoul(argv[1], NULL, 16);
	unsigned long *ep_dest = (unsigned long *)(simple_strtoul(argv[2], NULL, 16) + PCIE_MAPPING_BASE);
	unsigned long dest = (unsigned long )(simple_strtoul(argv[2], NULL, 16));
	unsigned long size = (unsigned long)simple_strtoul(argv[3], NULL, 16);
	u32 status;


	status = readl(CC_RC_EP_BOOT_COMM_REG);
	switch (status) {
	case PCIE_STATUS_DRAM_DONE :
		nvt_dbg(MSG, "EP Dram config done \n");
		break;
	default:
		nvt_dbg(MSG, "EP Dram is not ready\n");
		return -1;
	}

	if (nvt_change_ep_inbound(dest, size))
		return -1;

	nvt_dbg(MSG, "src : 0x%lx, dest : 0x%lx, size : 0x%lx\n", src, ep_dest, size);
	memcpy((void *)PCIE_MAPPING_BASE, src, size);
	flush_cache(round_down((unsigned long)PCIE_MAPPING_BASE, CONFIG_SYS_CACHELINE_SIZE), round_up(size, CONFIG_SYS_CACHELINE_SIZE));
	return 0;
}

U_BOOT_CMD(
	nvt_pcie_copy, 4,    1,      do_nvt_pcie_copy,
	"nvt_pcie_copy [source data] [ep address] [size]",
	""
);

static int do_nvt_pcie_init(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	writel(0x0, 0x2f2300004);
	writel(0x0, 0x2f2300000);
	writel(0x0, 0x2f2300008);
	writel(0x8, 0x2f230000C);
	writel(0x3FFFFFFF, 0x2f2300010);
	writel(0xC0000000, 0x2f2300014);
	writel(0x0, 0x2f2300018);
	writel(0x0, 0x2f230001C);
	writel(0x8, 0x2f2300020);
	writel(0x80000000, 0x2f2300004);

	writel(0x0, 0x803000F04);
	writel(0x2000, 0x803000F00);
	writel(0xC2000000, 0x803000F08);
	writel(0x0, 0x803000F0C);
	writel(0xC2FFFFFF, 0x803000F10);
	writel(0xF0000000, 0x803000F14);
	writel(0x2, 0x803000F18);
	writel(0x0, 0x803000F1C);
	writel(0x0, 0x803000F20);
	writel(0x80000000, 0x803000F04);

	return 0;
}

U_BOOT_CMD(
	nvt_pcie_init, 1,    1,      do_nvt_pcie_init,
	"nvt_pcie_init",
	""
);
