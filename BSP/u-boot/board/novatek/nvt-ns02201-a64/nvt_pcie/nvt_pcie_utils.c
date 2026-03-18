#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <stdlib.h>
#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/nvt_pcie.h>
#include <linux/bitfield.h>
#include <pci.h>

#define PCIE_BUS_BASE			0xC0000000
#define PCIE_MAPPING_BASE		0x800000000
#define PCIE_SRAM_BASE			(PCIE_MAPPING_BASE + 0x2D40000)
#define PCIE_MAP_SIZE			0x40000000
#define PCIE_ABP_SIZE			0x1000000
#define PCIE_ABP_OFFSET			0x2000000
#define NVT_ABP_BASE			0x2F0000000

#define PCIE_SRAM_BASE			(PCIE_MAPPING_BASE + 0x2D40000)
#define EP_IFE1_SRAM_CLK_ADDR_NS02201	(PCIE_MAPPING_BASE + 0x2020070)
#define IFE1_CLK_EN_NS02201		BIT(26)
#define EP_IFE2_SRAM_CLK_ADDR_NS02201	(PCIE_MAPPING_BASE + 0x2020074)
#define IFE2_CLK_EN_NS02201		BIT(4)
#define EP_DDR1_RESET_ADDR_NS02201	(PCIE_MAPPING_BASE + 0x2020090)
#define EP_DDR1_RESET_VALUE_NS02201	~FIELD_PREP(GENMASK(9,8), 0x3)
#define EP_DDR2_RESET_ADDR_NS02201	(PCIE_MAPPING_BASE + 0x20200A0)
#define EP_DDR2_RESET_VALUE_NS02201	~FIELD_PREP(GENMASK(26,25), 0x3)

#define EP_LOADER_SIZE			0x10000
#define OUTBOUND_SIZE			0x2000000

#define CC_RC_EP_BOOT_COMM_REG		(PCIE_MAPPING_BASE + 0x2110120)


#define PCIE_STATUS_IDLE		0x544F4F42
#define PCIE_STATUS_DRAM_DONE		0x5944524C


static int nvt_get_ep_atu_offset(u64 *bus_start)
{
	u8 header_type;
	u32 base_low, base_high;
	u32 size_low, size_high;
	u64 base, size;
	u32 reg_addr;
	struct udevice *dev;
	int mem_type;
	int ret = 0;

	pci_dev_t bdf = PCI_BDF(1, 0, 0);

	ret = dm_pci_bus_find_bdf(bdf, &dev);
	if (ret) {
		printf("%s: dm_pci_bus_find_bdf fail\n", __func__);
		return ret;
	}

	reg_addr = PCI_BASE_ADDRESS_4;

	dm_pci_read_config32(dev, reg_addr, &base_low);
	dm_pci_write_config32(dev, reg_addr, 0xffffffff);
	dm_pci_read_config32(dev, reg_addr, &size_low);
	dm_pci_write_config32(dev, reg_addr, base_low);
	reg_addr += 4;

	base = base_low & ~0xf;
	size = size_low & ~0xf;
	base_high = 0x0;
	size_high = 0xffffffff;
	mem_type = base_low & PCI_BASE_ADDRESS_MEM_TYPE_MASK;

	if (mem_type == PCI_BASE_ADDRESS_MEM_TYPE_64) {
		dm_pci_read_config32(dev, reg_addr, &base_high);
		dm_pci_write_config32(dev, reg_addr, 0xffffffff);
		dm_pci_read_config32(dev, reg_addr, &size_high);
		dm_pci_write_config32(dev, reg_addr, base_high);
	}

	base = base | ((u64)base_high << 32);
	*bus_start = base;

	return ret;
}

static void pcie_set_ep_inbound_region(int bar_id, u64 src_addr, u64 src_upper_addr, u64 target_addr, int region_id, int en)
{
	u64 bus_start;
	u64 BASE;
	if (nvt_get_ep_atu_offset(&bus_start))
		return;

       	BASE = PCIE_MAPPING_BASE + (bus_start - PCIE_BUS_BASE);

	writel(lower_32_bits(src_addr), BASE+IATU_LWR_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(upper_32_bits(src_addr), BASE+IATU_UPPER_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(lower_32_bits(src_upper_addr), BASE+IATU_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(upper_32_bits(src_upper_addr), BASE+IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(lower_32_bits(target_addr), BASE+IATU_LWR_TARGET_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(upper_32_bits(target_addr), BASE+IATU_UPPER_TARGET_ADDR_OFF_INBOUND_0 + region_id*0x200);
	writel(0x00000000, BASE+IATU_REGION_CTRL_1_OFF_INBOUND_0 + region_id*0x200);
	writel((en<<31), BASE+IATU_REGION_CTRL_2_OFF_INBOUND_0 + region_id*0x200);
}

static void pcie_set_outbound_region(u64 src_addr, u64 src_upper_addr, u64 target_addr, int region_id, int atu_type, int en) {
	const uintptr_t BASE = PCIE_DBI_REG_BASE + PF0_ATU_CAP_DBIBaseAddress;

	writel(lower_32_bits(src_addr), BASE+IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(upper_32_bits(src_addr), BASE+IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(lower_32_bits(src_upper_addr), BASE+IATU_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(upper_32_bits(src_upper_addr), BASE+IATU_UPPER_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(lower_32_bits(target_addr), BASE+IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(upper_32_bits(target_addr), BASE+IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(atu_type, BASE+IATU_REGION_CTRL_1_OFF_OUTBOUND_0 + region_id*0x200);

	writel((en<<31), BASE+IATU_REGION_CTRL_2_OFF_OUTBOUND_0 + region_id*0x200);
}

static int nvt_change_ep_inbound(unsigned long dest, unsigned long size)
{
	if (size > OUTBOUND_SIZE) {
		nvt_dbg(MSG, "Size is over %x\n", OUTBOUND_SIZE);
		return -1;
	}

	pcie_set_ep_inbound_region(0, PCIE_BUS_BASE, PCIE_BUS_BASE + OUTBOUND_SIZE - 1, dest, 0, 1);
	return 0;
}

static int do_nvt_pcie_copy_sram(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int eploader_size = EP_LOADER_SIZE;
	unsigned int *p_src = (unsigned int *)simple_strtoul(argv[1], NULL, 16);
	unsigned int *p_dst = (unsigned int *)(PCIE_SRAM_BASE);
	unsigned int reg;
	int i;

	reg = readl(EP_IFE1_SRAM_CLK_ADDR_NS02201);
	reg |= IFE1_CLK_EN_NS02201;
	writel(reg, EP_IFE1_SRAM_CLK_ADDR_NS02201);

	reg = readl(EP_IFE2_SRAM_CLK_ADDR_NS02201);
	reg |= IFE2_CLK_EN_NS02201;
	writel(reg, EP_IFE2_SRAM_CLK_ADDR_NS02201);

	memcpy_toio(p_dst, p_src, eploader_size);
	return 0;
}

U_BOOT_CMD(
	nvt_pcie_copy_sram, 2,    1,      do_nvt_pcie_copy_sram,
	"nvt_pcie_copy_sram [eploader addre]",
	""
);

static int do_nvt_pcie_boot(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	writel(EP_DDR1_RESET_VALUE_NS02201, EP_DDR1_RESET_ADDR_NS02201);
	writel(EP_DDR2_RESET_VALUE_NS02201, EP_DDR2_RESET_ADDR_NS02201);
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
	memcpy_toio((void *)PCIE_MAPPING_BASE, src, size);
	flush_cache(round_down((unsigned long)PCIE_MAPPING_BASE, CONFIG_SYS_CACHELINE_SIZE), round_up(OUTBOUND_SIZE, CONFIG_SYS_CACHELINE_SIZE));
	return 0;
}

U_BOOT_CMD(
	nvt_pcie_copy, 4,    1,      do_nvt_pcie_copy,
	"nvt_pcie_copy [source data] [ep address] [size]",
	""
);

static int do_nvt_pcie_init(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{

	pcie_set_outbound_region(PCIE_MAPPING_BASE, PCIE_MAPPING_BASE + PCIE_MAP_SIZE - 1, PCIE_BUS_BASE, 0, PCIE_ATU_TYPE_MEM, 1);

	pcie_set_ep_inbound_region(0, PCIE_BUS_BASE + PCIE_ABP_OFFSET, PCIE_BUS_BASE + PCIE_ABP_OFFSET + PCIE_ABP_SIZE -1, NVT_ABP_BASE, 7, 1);

	return 0;
}

U_BOOT_CMD(
	nvt_pcie_init, 1,    1,      do_nvt_pcie_init,
	"nvt_pcie_init",
	""
);
