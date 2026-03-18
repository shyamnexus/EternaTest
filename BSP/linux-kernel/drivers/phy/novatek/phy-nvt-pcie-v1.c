/*
 * Copyright (C) 2021 Novatek Microelectronics Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/bitfield.h>

#include <plat/hardware.h>

#define DRV_VERSION             "1.00.006"

/* TOP registers */
#define TOP_BOOT_CTRL_REG       0x00
#define PCIE_SATA_COMBO_MASK    GENMASK(9, 9)
#define PCIE_SATA_COMBO_SATA    0
#define PCIE_SATA_COMBO_PCIE    1

/* PHY registers */
#define PHY_RXODT_REG           0x0000
#define PHY_RXODT_CAL_MSK       GENMASK(23, 16)
#define PHY_RXODT_CAL_LONGER    0xFF

#define PHY_TXODT_REG           0x000C
#define PHY_TXODT_CAL_MSK       GENMASK(23, 22)
#define PHY_TXODT_CAL_LONGER    2
#define PHY_NODT_INI_MSK        GENMASK(21, 21)
#define PHY_NODT_INI_MID        1
#define PHY_PODT_INI_MSK        GENMASK(20, 20)
#define PHY_PODT_INI_MID        1

#define PHY_DFE_CTRL_REG        0x00A0
#define PHY_VREF_REL_MSK        GENMASK(30, 30)
#define PHY_VREF_REL_AUTO       1
#define PHY_VREF_INITIAL_MSK    GENMASK(29, 24)
#define PHY_VREF_INIT_VAL       0x3F
#define PHY_AGC_ACC_TH_MSK	GENMASK(18, 10)
#define PHY_AGC_ACC_TH_VAL	0x100

#define PHY_DFE_TH1_REG		0x00A4
#define PHY_DFE_TH1_MSK		GENMASK(21, 12)
#define PHY_DFE_TH1_VAL		0x100

#define PHY_DFE_TH2_REG		0x00A8
#define PHY_DFE_TH2_MSK		GENMASK(21, 12)
#define PHY_DFE_TH2_VAL		0x100

#define PHY_CTLE_ACC_REG        0x00B8
#define PHY_CTLE_ACC_MSK        GENMASK(10, 0)
#define PHY_CTLE_ACC_VAL        0x400

#define PHY_RXQEC_REG           0x0108
#define PHY_CLKQ_DLY_MSK        GENMASK(19, 17)
#define PHY_CLKQ_DLY_SETTING    1
#define PHY_CLKQ_DLY_ENA_MSK    GENMASK(9, 9)
#define PHY_CLKQ_DLY_EN         1

#define PHY_0218_REG            0x0218
#define PHY_T2R_PATH_MSK	GENMASK(5, 5)
#define PHY_T2R_PATH_EN		1

#define PHY_LOS_REG             0x0834
#define PHY_LOS_THSHLD_MSK      GENMASK(15, 12)
#define PHY_LOS_THSHLD_PCIE     0x04

#define PHY_085C_REG            0x085C
#define PHY_RX_MUX_SEL_MSK	GENMASK(30, 30)
#define PHY_TX_CLK_DEGLITCH_MSK	GENMASK(26, 26)

#define PHY_0918_REG            0x0918
#define PHY_RX_DEMUX_MSK	GENMASK(22, 22)
#define PHY_RX_DEMUX_ECO	1

#define PHY_1068_REG            0x1068
#define PHY_1068_B21_MSK        GENMASK(21, 21)
#define PHY_TX_ELEC_PH_MSK	GENMASK(20, 20)
#define PHY_TX_ELEC_PH_EN	0

#define PHY_1070_REG            0x1070
#define PHY_PTEN_RST_CTRL_MSK   GENMASK(8, 8)
#define PHY_PTEN_RST_BYPASS     1

#define PHY_AEQ_AGC_REG		0x1080
#define PHY_AEQ_TIME_MSK	GENMASK(13, 0)
#define PHY_AGC_TIME_MSK	GENMASK(29, 16)

#define PHY_PIPE_CTRL_REG       0x3010
#define PHY_EIOS_CHK_HDR_MSK    GENMASK(21, 21)
#define PHY_EIOS_CHK_HDR_NEW	1
#define PHY_PIPE_MODE_MSK       GENMASK(14, 14)
#define PHY_PIPE_MODE_CHG       1
#define PHY_EIOS_DET_MSK        GENMASK(3, 3)
#define PHY_EIOS_DET_DIS        1
#define PHY_EIOS_DET_EN         0

#define PHY_3014_REG            0x3014
#define PHY_3014_B7_MSK         GENMASK(7, 7)
#define PHY_RX_MUX_RST_MSK	GENMASK(3, 3)
#define PHY_RX_MUX_RST_EN	1

#define PHY_CMM_REG             0x4060
#define PHY_CMLR_PDB_MSK        GENMASK(3, 3)
#define PHY_CMLR_PDB_OFF        0
#define PHY_CMLR_PDB_ON         1

#define PHY_PHASE_CALIBRATION_REG  0x1210
#define PHY_PHASE_CALIBRATION_MSK  GENMASK(4, 4)
#define PHY_PHASE_CALIBRATION_VAL  0x1

#define PHY_PHASE_CALIBRATION_FOM_REG  0x1210
#define PHY_PHASE_CALIBRATION_FOM_MSK  GENMASK(19, 19)

#define PHY_FOM_CTRL_REG  0x1028
#define PHY_FOM_CTRL_MSK  GENMASK(11, 11)
#define PHY_FOM_CTRL_VAL  0x1

#define PHY_FOM_PARAM_REG  0x102C
#define PHY_FOM_PARAM_MSK  GENMASK(16, 16)
#define PHY_FOM_PARAM_VAL  0x1

#define PHY_PRESET_REG  0x1304
#define PHY_PRESET_MSK  GENMASK(25, 22)
#define PHY_PRESET_VAL  0x7
#define PHY_PRESET_MANUAL_MSK  GENMASK(21, 21)
#define PHY_PRESET_MANUAL_VAL  0x1


#define PHY_AEQ_REG  0x091C
#define PHY_AEQ_MSK  GENMASK(16, 16)

#define PHY_TUNE_R_REG  0x00AC
#define PHY_TUNE_R_MSK  GENMASK(22, 22)

#define PHY_MANUAL_C_REG  0x00AC
#define PHY_MANUAL_C_MSK  GENMASK(23, 23)
#define PHY_MANUAL_C_VAL  0x1

#define PHY_FIX_C_REG  0x00A8
#define PHY_FIX_C_MSK  GENMASK(27, 25)
#define PHY_FIX_C_VAL  0x4

#define PHY_40DC_REG            0x40DC


enum nvt_pcie_phy_id {
	NVT_PHY_PCIE0 = 0,
	MAX_NUM_PHYS,
};

struct nvt_pcie_phy_core;

/**
 * struct nvt_pcie_phy - Novatek PCIe PHY device
 * @core: pointer to the Novatek PCIe PHY core control
 * @id: internal ID to identify the Novatek PCIe PHY
 * @phy: pointer to the kernel PHY device
 */
struct nvt_pcie_phy {
	struct nvt_pcie_phy_core *core;
	enum nvt_pcie_phy_id id;
	struct phy *phy;
};

/**
 * struct nvt_pcie_phy_core - Novatek PCIe PHY core control
 * @dev: pointer to device
 * @base: base register
 * @lock: mutex to protect access to individual PHYs
 * @phys: pointer to Novatek PHY device
 */
struct nvt_pcie_phy_core {
	struct device *dev;
	void __iomem *base;
	int en_l1_detect;
	struct mutex lock;
	struct nvt_pcie_phy phys[MAX_NUM_PHYS];
};

static int nvt_pcie_phy_init(struct phy *p)
{
	u32 val;
	struct nvt_pcie_phy *phy = phy_get_drvdata(p);
	struct nvt_pcie_phy_core *core = phy->core;

	// DFE vref
	val = readl(core->base + PHY_DFE_CTRL_REG);
	val &= ~PHY_VREF_INITIAL_MSK;
	val |= FIELD_PREP(PHY_VREF_INITIAL_MSK, PHY_VREF_INIT_VAL);
	val |= FIELD_PREP(PHY_VREF_REL_MSK, PHY_VREF_REL_AUTO);
	writel(val, core->base + PHY_DFE_CTRL_REG);

	// txodt
	val = readl(core->base + PHY_TXODT_REG);
	val &= ~PHY_TXODT_CAL_MSK;
	val |= FIELD_PREP(PHY_TXODT_CAL_MSK, PHY_TXODT_CAL_LONGER);
	val |= FIELD_PREP(PHY_NODT_INI_MSK, PHY_NODT_INI_MID);
	val |= FIELD_PREP(PHY_PODT_INI_MSK, PHY_PODT_INI_MID);
	writel(val, core->base + PHY_TXODT_REG);

	// rxodt
	val = readl(core->base + PHY_RXODT_REG);
	val &= ~PHY_RXODT_CAL_MSK;
	val |= FIELD_PREP(PHY_RXODT_CAL_MSK, PHY_RXODT_CAL_LONGER);
	writel(val, core->base + PHY_RXODT_REG);

	// rxqec
	val = readl(core->base + PHY_RXQEC_REG);
	val &= ~PHY_CLKQ_DLY_MSK;
	val |= FIELD_PREP(PHY_CLKQ_DLY_MSK, PHY_CLKQ_DLY_SETTING);
	val |= FIELD_PREP(PHY_CLKQ_DLY_ENA_MSK, PHY_CLKQ_DLY_EN);
	writel(val, core->base + PHY_RXQEC_REG);

	// bypass PTEN RESET
	val = readl(core->base + PHY_1070_REG);
	val |= FIELD_PREP(PHY_PTEN_RST_CTRL_MSK, PHY_PTEN_RST_BYPASS);
	writel(val, core->base + PHY_1070_REG);

	// PIPE mode change
	val = readl(core->base + PHY_PIPE_CTRL_REG);
	val |= FIELD_PREP(PHY_PIPE_MODE_MSK, PHY_PIPE_MODE_CHG);
	writel(val, core->base + PHY_PIPE_CTRL_REG);

	// pclk deglitch
	val = readl(core->base + PHY_1068_REG);
	val |= FIELD_PREP(PHY_1068_B21_MSK, 1);
	writel(val, core->base + PHY_1068_REG);
	val = readl(core->base + PHY_3014_REG);
	val |= FIELD_PREP(PHY_3014_B7_MSK, 1);
	writel(val, core->base + PHY_3014_REG);

	// CMM
	val = readl(core->base + PHY_CMM_REG);
	val &= ~PHY_CMLR_PDB_MSK;
	writel(val, core->base + PHY_CMM_REG);

	// LOS comparator
	val = readl(core->base + PHY_LOS_REG);
	val &= ~PHY_LOS_THSHLD_MSK;
	val |= FIELD_PREP(PHY_LOS_THSHLD_MSK, PHY_LOS_THSHLD_PCIE);
	writel(val, core->base + PHY_LOS_REG);

	// disable EIOS detect
	val = readl(core->base + PHY_PIPE_CTRL_REG);
	if (core->en_l1_detect == 0) {
		val |= FIELD_PREP(PHY_EIOS_DET_MSK, PHY_EIOS_DET_DIS);
	} else {
		val &= ~PHY_EIOS_DET_MSK;
		val |= FIELD_PREP(PHY_EIOS_CHK_HDR_MSK, PHY_EIOS_CHK_HDR_NEW);
	}
	writel(val, core->base + PHY_PIPE_CTRL_REG);

	// switch rx div2 demux
	if (core->en_l1_detect == 1) {
		val = readl(core->base + PHY_0918_REG);
		val |= FIELD_PREP(PHY_RX_DEMUX_MSK, PHY_RX_DEMUX_ECO);
		writel(val, core->base + PHY_0918_REG);
	}

	// T2R idle
	val = readl(core->base + PHY_0218_REG);
	val |= FIELD_PREP(PHY_T2R_PATH_MSK, PHY_T2R_PATH_EN);
	writel(val, core->base + PHY_0218_REG);

	val = readl(core->base + PHY_1068_REG);
	val &= ~PHY_TX_ELEC_PH_MSK;
	writel(val, core->base + PHY_1068_REG);

	// TXCLK deglitch
	val = readl(core->base + PHY_085C_REG);
	val |= FIELD_PREP(PHY_TX_CLK_DEGLITCH_MSK, 1);
	writel(val, core->base + PHY_085C_REG);

	// AEQ paremeter
	val = readl(core->base + PHY_CTLE_ACC_REG);	// ctle accu th
	val &= ~PHY_CTLE_ACC_MSK;
	val |= FIELD_PREP(PHY_CTLE_ACC_MSK, PHY_CTLE_ACC_VAL);
	writel(val, core->base + PHY_CTLE_ACC_REG);

	val = readl(core->base + PHY_AEQ_AGC_REG);	// aeq time period
	val |= PHY_AEQ_TIME_MSK;
	writel(val, core->base + PHY_AEQ_AGC_REG);

	// AGC/VTH parameter
	val = readl(core->base + PHY_DFE_CTRL_REG);	// agc acc th
	val &= ~PHY_AGC_ACC_TH_MSK;
	val |= FIELD_PREP(PHY_AGC_ACC_TH_MSK, PHY_AGC_ACC_TH_VAL);
	writel(val, core->base + PHY_DFE_CTRL_REG);

	val = readl(core->base + PHY_AEQ_AGC_REG);	// agc time period/dfe time period
	val |= PHY_AGC_TIME_MSK;
	writel(val, core->base + PHY_AEQ_AGC_REG);

	// DFE parameter
	val = readl(core->base + PHY_DFE_TH1_REG);	// H1 threshold
	val &= ~PHY_DFE_TH1_MSK;
	val |= FIELD_PREP(PHY_DFE_TH1_MSK, PHY_DFE_TH1_VAL);
	writel(val, core->base + PHY_DFE_TH1_REG);

	val = readl(core->base + PHY_DFE_TH2_REG);	// H2 threshold
	val &= ~PHY_DFE_TH2_MSK;
	val |= FIELD_PREP(PHY_DFE_TH2_MSK, PHY_DFE_TH2_VAL);
	writel(val, core->base + PHY_DFE_TH2_REG);

	val = readl(core->base + PHY_AEQ_AGC_REG);	// agc time period/dfe time period
	val |= PHY_AGC_TIME_MSK;
	writel(val, core->base + PHY_AEQ_AGC_REG);

	//set pcie link eq prototype
	val = readl(core->base + PHY_PHASE_CALIBRATION_REG);	// bypass phase calibration
	val |= FIELD_PREP(PHY_PHASE_CALIBRATION_MSK, PHY_PHASE_CALIBRATION_VAL);
	writel(val, core->base + PHY_PHASE_CALIBRATION_REG);

	val = readl(core->base + PHY_PHASE_CALIBRATION_FOM_REG);	// bypass phase calibration
	val &= ~PHY_PHASE_CALIBRATION_FOM_MSK;
	writel(val, core->base + PHY_PHASE_CALIBRATION_FOM_REG);

	val = readl(core->base + PHY_FOM_CTRL_REG);	// enable FOM sw ctrl
	val |= FIELD_PREP(PHY_FOM_CTRL_MSK, PHY_FOM_CTRL_VAL);
	writel(val, core->base + PHY_FOM_CTRL_REG);

	val = readl(core->base + PHY_FOM_PARAM_REG);	// enable FOM para sw
	val |= FIELD_PREP(PHY_FOM_PARAM_MSK, PHY_FOM_PARAM_VAL);
	writel(val, core->base + PHY_FOM_PARAM_REG);

	val = readl(core->base + PHY_PRESET_REG);	// total 5 preset
	val &= ~PHY_PRESET_MSK;
	val |= FIELD_PREP(PHY_PRESET_MSK, PHY_PRESET_VAL);
	val |= FIELD_PREP(PHY_PRESET_MANUAL_MSK, PHY_PRESET_MANUAL_VAL);
	writel(val, core->base + PHY_PRESET_REG);

	// suppress pll noise
	writel(0x54, core->base + PHY_40DC_REG);

	return 0;
}

static int nvt_pcie_phy_power_on(struct phy *p)
{
//	struct nvt_pcie_phy *phy = phy_get_drvdata(p);

	return 0;
}

static int nvt_pcie_phy_power_off(struct phy *p)
{
//	struct nvt_pcie_phy *phy = phy_get_drvdata(p);

	return 0;
}

static int nvt_pcie_set_mode(struct phy *p, enum phy_mode mode, int submode)
{
	u32 val;
	struct nvt_pcie_phy *phy = phy_get_drvdata(p);
	struct nvt_pcie_phy_core *core = phy->core;

	if (mode == PHY_MODE_INVALID) {

		// swap mux
		val = readl(core->base + PHY_085C_REG);
		val &= ~PHY_RX_MUX_SEL_MSK;
		writel(val, core->base + PHY_085C_REG);

		val = readl(core->base + PHY_3014_REG);
		val &= ~PHY_RX_MUX_RST_MSK;
		writel(val, core->base + PHY_3014_REG);
		val |= FIELD_PREP(PHY_RX_MUX_RST_MSK, PHY_RX_MUX_RST_EN);
		printk("%s: RX MUX RST value = 0x%lx\r\n", __func__, FIELD_PREP(PHY_RX_MUX_RST_MSK, PHY_RX_MUX_RST_EN));
		writel(val, core->base + PHY_3014_REG);
		val &= ~PHY_RX_MUX_RST_MSK;
		writel(val, core->base + PHY_3014_REG);

	}

	return 0;
}

static int nvt_pcie_phy_reset(struct phy *p)
{
	u32 val;
	struct nvt_pcie_phy *phy = phy_get_drvdata(p);
	struct nvt_pcie_phy_core *core = phy->core;

	// set mux to default value
	val = readl(core->base + PHY_085C_REG);
	val |= FIELD_PREP(PHY_RX_MUX_SEL_MSK, 1);
	writel(val, core->base + PHY_085C_REG);

	val = readl(core->base + PHY_3014_REG);
	val &= ~PHY_RX_MUX_RST_MSK;
	writel(val, core->base + PHY_3014_REG);
	val |= FIELD_PREP(PHY_RX_MUX_RST_MSK, PHY_RX_MUX_RST_EN);
	writel(val, core->base + PHY_3014_REG);
	val &= ~PHY_RX_MUX_RST_MSK;
	writel(val, core->base + PHY_3014_REG);

	return 0;
}

static const struct phy_ops nvt_pcie_phy_ops = {
	.init = nvt_pcie_phy_init,
	.power_on = nvt_pcie_phy_power_on,
	.power_off = nvt_pcie_phy_power_off,
	.set_mode = nvt_pcie_set_mode,
	.reset = nvt_pcie_phy_reset,
	.owner = THIS_MODULE,
};

static int nvt_pcie_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node, *child;
	struct nvt_pcie_phy_core *core;
	struct phy_provider *provider;
	struct resource *res;
	void __iomem *ptr_top;
	unsigned cnt = 0;
#if (!IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NS02201))
	int en_l1_detect = 0;
#endif
	int ret;
	u32 val;

	// Check bootstrap correctness
	ptr_top = ioremap(NVT_TOP_BASE_PHYS, 256);
	val = readl(ptr_top + TOP_BOOT_CTRL_REG);
	iounmap(ptr_top);
	if (FIELD_GET(PCIE_SATA_COMBO_MASK, val) != PCIE_SATA_COMBO_PCIE) {
		dev_err(dev, "Bootstrap[9] does NOT configure combo phy to pcie mode\n");
		return -EPERM;
	}

	if (of_get_child_count(node) == 0) {
		dev_err(dev, "PHY no child node\n");
		return -ENODEV;
	}

	core = devm_kzalloc(dev, sizeof(*core), GFP_KERNEL);
	if (!core)
		return -ENOMEM;

	core->dev = dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	core->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(core->base))
		return PTR_ERR(core->base);

#if (!IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NS02201))
	if (nvt_get_chip_ver(NVT_PERIPHERAL_PHYS_BASE) != CHIPVER_A) {
		if (!of_property_read_u32(pdev->dev.of_node, "en-l1-det", &en_l1_detect)) {
			printk("%s: find en-l1-det %d\r\n", __func__, en_l1_detect);
			core->en_l1_detect = en_l1_detect;
		}
	}
#else
	core->en_l1_detect = 1;
#endif

	mutex_init(&core->lock);

	for_each_available_child_of_node(node, child) {
		unsigned int id;
		struct nvt_pcie_phy *p;

		if (id >= MAX_NUM_PHYS) {
			dev_err(dev, "invalid PHY id: %u\n", id);
			ret = -EINVAL;
			goto put_child;
		}

		if (core->phys[id].phy) {
			dev_err(dev, "duplicated PHY id: %u\n", id);
			ret = -EINVAL;
			goto put_child;
		}

		p = &core->phys[id];
		p->phy = devm_phy_create(dev, child, &nvt_pcie_phy_ops);
		if (IS_ERR(p->phy)) {
			dev_err(dev, "failed to create PHY\n");
			ret = PTR_ERR(p->phy);
			goto put_child;
		}

		p->core = core;
		p->id = id;
		phy_set_drvdata(p->phy, p);
		cnt++;
	}

	dev_set_drvdata(dev, core);

	provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	if (IS_ERR(provider)) {
		dev_err(dev, "failed to register PHY provider\n");
		return PTR_ERR(provider);
	}

	dev_dbg(dev, "registered %u PCIe PHY(s)\n", cnt);

	dev_info(dev, "PCIe PHY probe done\r\n");

	return 0;
put_child:
	of_node_put(child);
	return ret;
}

static const struct of_device_id nvt_pcie_phy_match_table[] = {
	{ .compatible = "nvt,pcie-v1-phy" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, nvt_pcie_phy_match_table);

static struct platform_driver nvt_pcie_phy_driver = {
	.driver = {
		.name = "nvt-pcie-phy",
		.of_match_table = nvt_pcie_phy_match_table,
	},
	.probe = nvt_pcie_phy_probe,
};
module_platform_driver(nvt_pcie_phy_driver);

MODULE_DESCRIPTION("Novatek PCIe-v1 PHY driver");
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL v2");

