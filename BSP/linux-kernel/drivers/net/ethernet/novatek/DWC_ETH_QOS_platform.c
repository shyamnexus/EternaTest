/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 */

#include "DWC_ETH_QOS_yheader.h"

struct NVT_PLAT_INFO {
	phys_addr_t     base_pa;        // base physical address
	char    *rmii_refclk_i_name;    // name of rmii_refclk_i
	char    *rxclk_i_name;
	char    *txclk_i_name;
	char    *ext_phy_clk_name;
	char    *ptp_clk_name;

	UINT    pinmux_rgmii;
	UINT    pinmux_rgmii_2;
	UINT    pinmux_rmii;
	UINT    pinmux_rmii_2;
	UINT    pinmux_emb;

	UINT    pinmux_ext_phy;

	UINT    gpio_reset;
	UINT    gpio_2_reset;

	UINT    pad_phy_clk;        // 25MHz clock for phy osc
	UINT    pad_txclk;      // TXCLK (RGMII)
	UINT    pad_txctl;      // TXCTL (RGMII) / TXEN (RMII)
	UINT    pad_rmii_refclk;    // REFCLK (RMII)
	UINT    pad_tx_d0;      // TXD[0]
	UINT    pad_tx_d1;      // TXD[1]
	UINT    pad_tx_d2;      // TXD[2]
	UINT    pad_tx_d3;      // TXD[3]

	UINT    pad_phy_clk_2;      // 25MHz clock for phy osc
	UINT    pad_txclk_2;        // TXCLK (RGMII)
	UINT    pad_txctl_2;        // TXCTL (RGMII) / TXEN (RMII)
	UINT    pad_rmii_refclk_2;  // REFCLK (RMII)
	UINT    pad_tx_d0_2;        // TXD[0]
	UINT    pad_tx_d1_2;        // TXD[1]
	UINT    pad_tx_d2_2;        // TXD[2]
	UINT    pad_tx_d3_2;        // TXD[3]
};

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
/* ---------------------NA51102-------------------------- */
static const struct NVT_PLAT_INFO v_nvt_plat_info[2] = {
	// MAC0 (0xFCC0_0000)
	{
		NVT_ETH_BASE_PHYS,
		"rmii0_refclk_i",
		"eth0_rxclk_i",
		"eth0_txclk_i",
		"eth0_extphy_clk",
		"eth0_ptp_clk",

		PIN_ETH_CFG_ETH_RGMII_1,//PIN_ETH_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_ETH_RMII_1,//PIN_ETH_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_ETH_EXTPHYCLK,//PIN_ETH_CFG_REFCLK_PINMUX,

		D_GPIO(8),//D_GPIO(8),          // gpio reset
		D_GPIO(8),//NOT SUP,            // gpio reset

		PAD_DS_DGPIO7,//PAD_DS_DGPIO7,  // PHYCLK PIN_1
		PAD_DS_LGPIO24,//PAD_DS_LGPIO24,    // TXCLK PIN_1
		PAD_DS_LGPIO19,//PAD_DS_LGPIO19,    // TXCTL PIN_1
		PAD_DS_LGPIO25,//PAD_DS_LGPIO25,    // REFCLK (RMII) PIN_1
		PAD_DS_LGPIO20,//PAD_DS_LGPIO20,    // TXD[0] PIN_1
		PAD_DS_LGPIO21,//PAD_DS_LGPIO21,    // TXD[1] PIN_1
		PAD_DS_LGPIO22,//PAD_DS_LGPIO22,    // TXD[2] PIN_1
		PAD_DS_LGPIO23,//PAD_DS_LGPIO23,    // TXD[3] PIN_1

		PAD_DS_DGPIO7,//NOT SUP,    // PHYCLK PIN_2
		PAD_DS_LGPIO24,//NOT SUP,   // TXCLK PIN_2
		PAD_DS_LGPIO19,//NOT SUP,   // TXCTL PIN_2
		PAD_DS_LGPIO25,//NOT SUP,   // REFCLK (RMII) PIN_2
		PAD_DS_PGPIO20,//NOT SUP,   // TXD[0] PIN_2
		PAD_DS_LGPIO21,//NOT SUP,   // TXD[1] PIN_2
		PAD_DS_LGPIO22,//NOT SUP,   // TXD[2] PIN_2
		PAD_DS_LGPIO23,//NOT SUP,   // TXD[3] PIN_2
	},
	// MAC1 (0xFCD0_0000)
	{
		NVT_ETH2_BASE_PHYS,
		"rmii1_refclk_i",
		"eth1_rxclk_i",
		"eth1_txclk_i",
		"eth1_extphy_clk",
		"eth1_ptp_clk",

		PIN_ETH_CFG_ETH2_RGMII_1,//PIN_ETH2_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_ETH2_RMII_1,//PIN_ETH2_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_ETH2_EXTPHYCLK,//PIN_ETH2_CFG_REFCLK_PINMUX,

		L_GPIO(18),//L_GPIO(18),            // gpio reset
		L_GPIO(18),//NOT SUP,           // gpio reset

		PAD_DS_LGPIO17,//PAD_DS_LGPIO17,    // PHYCLK
		PAD_DS_LGPIO8,//PAD_DS_LGPIO8,  // TXCLK
		PAD_DS_LGPIO3,//PAD_DS_LGPIO3,  // TXCTL
		PAD_DS_LGPIO9,//PAD_DS_LGPIO9,  // REFCLK (RMII)
		PAD_DS_LGPIO4,//PAD_DS_LGPIO4,  // TXD[0]
		PAD_DS_LGPIO5,//PAD_DS_LGPIO5,  // TXD[1]
		PAD_DS_LGPIO6,//PAD_DS_LGPIO6,  // TXD[2]
		PAD_DS_LGPIO7,//PAD_DS_LGPIO7,  // TXD[3]

		PAD_DS_LGPIO17,//NOT SUP,   // PHYCLK PIN_2
		PAD_DS_LGPIO8,//NOT SUP,    // TXCLK PIN_2
		PAD_DS_LGPIO3,//NOT SUP,    // TXCTL PIN_2
		PAD_DS_LGPIO9,//NOT SUP,    // REFCLK (RMII) PIN_2
		PAD_DS_LGPIO4,//NOT SUP,    // TXD[0] PIN_2
		PAD_DS_LGPIO5,//NOT SUP,    // TXD[1] PIN_2
		PAD_DS_LGPIO6,//NOT SUP,    // TXD[2] PIN_2
		PAD_DS_LGPIO7,//NOT SUP,    // TXD[3] PIN_2
	},
};


static void nvt_eth_phy_poweron(void __iomem *base_addr)
{
	unsigned long reg;

	reg = ioread32((void *)(base_addr + 0x3800 + 0xF8));
	iowrite32(reg | (1 << 7), (void *)(base_addr + 0x3800 + 0xF8));
	udelay(20);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(200);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 1)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(250);
	reg = ioread32((void *)(base_addr + 0x3800 + 0x2E8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x2E8));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xCC));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xCC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xDC));
	iowrite32(reg | (1 << 0), (void *)(base_addr + 0x3800 + 0xDC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0x9C));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x9C));
}

static int set_pad_driving(UINT interface, const struct NVT_PLAT_INFO *p_info)
{

	if ((interface == PIN_ETH_CFG_ETH_RMII_1) || (interface == PIN_ETH_CFG_ETH2_RMII_1)) {
		pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_rmii_refclk, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_10);
	} else if ((interface == PIN_ETH_CFG_ETH_RGMII_1) || (interface == PIN_ETH_CFG_ETH2_RGMII_1)) {

		pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_txclk, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d2, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d3, PAD_DRIVINGSINK_LEVEL_10);
	}

	return 0;
}

int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base, int *embd_en)
{
	int ret = 0;
	struct resource *res;
	void __iomem *base;
	PIN_GROUP_CONFIG pinmux_config[1];
	UINT ETH_QOS_INDEX; // index to MAC (0: MAC0, 1: MAC1);
#if IS_ENABLED(CONFIG_TIMESTAMP)
	struct clk *ptpclk;
#endif
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;
	int spclk_en = 0;
	int refclk_out = 0;
#endif

//	ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No memory resources\n");
		return -1;
	}
	printk("%s: IO MEM res start 0x%llx\r\n", __func__, res->start);

	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to find IORESOURCE_MEM\n");
		return -1;
//		ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	} else {
		*p_base = base;
//		ETH_QOS_REG_BASE = base;
	}
	if (res->start == v_nvt_plat_info[0].base_pa) {
		ETH_QOS_INDEX = 0;
	} else {
		ETH_QOS_INDEX = 1;
	}
//	ETH_QOS_INDEX = 1;   // temp force to MAC1 (for embedded phy)
	printk("%s: get IO MEM 0x%px\r\n", __func__, base);

	pinmux_config[0].pin_function = PIN_FUNC_ETH;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		dev_err(&pdev->dev, "get pinmux config failed\n");

	printk("%s: get pinmux 0x%x\r\n", __func__, pinmux_config[0].config);

#if IS_ENABLED(CONFIG_TIMESTAMP)
	ptpclk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
	if (IS_ERR(ptpclk)) {
		dev_err(&pdev->dev, "can't find ptp_clock %s\n",
				v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
		ptpclk = NULL;
	} else {
		clk_prepare(ptpclk);
		clk_enable(ptpclk);
	}
	clk_put(ptpclk);
#endif

#if IS_ENABLED(CONFIG_MULTI_IRQ)

	reg = ioread32((void *)(base + 0x3004));
	iowrite32(reg | (1 << 0), (void *)(base + 0x3004));

	reg = ioread32((void *)(base + 0x3028));
	reg = reg & ~(3 << 0);
	iowrite32(reg | (3 << 2), (void *)(base + 0x3028));
#endif

	// impelment MAC1
	/*==== RMII =====*/
	if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii)) {
		unsigned long reg;

		printk("%s: pinmux detect RMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RMII, &v_nvt_plat_info[ETH_QOS_INDEX]);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			if (spclk_en &&
				(!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {
				struct clk *clk;
#if 1

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;
				ret = nvt_pinmux_update(pinmux_config, 1);

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n", __func__, __LINE__);
				}

				printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);

				if (spclk_en) {
					clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					if (IS_ERR(clk)) {
						dev_err(&pdev->dev, "can't find clock %s\n",
								v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
						clk = NULL;
					} else {
						clk_prepare(clk);
						clk_enable(clk);
					}
					clk_put(clk);
				}
			}
#else
				if (spclk_en &&
					(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
					struct clk *clk;

					printk("%s: change extphyclk pinmux\r\n", __func__);
					pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

					ret = nvt_pinmux_update(pinmux_config, 1);
					if (ret)
						pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
								__func__, __LINE__);

					clk = clk_get(&pdev->dev, "ext_phy_clk");
					if (IS_ERR(clk)) {
						dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
						clk = NULL;
					} else {
						clk_prepare(clk);
						clk_enable(clk);
					}
					clk_put(clk);
				}
#endif
		}

		// assert phy reset
		set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii, &v_nvt_plat_info[ETH_QOS_INDEX]);
		gpio_direction_output(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		mdelay(20);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 1);
		mdelay(50);

#endif

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
#ifdef CONFIG_NVT_FPGA_EMULATION
		//reg |= 1<<31; // output at clock falling
#endif
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));

		// setup RMII REF_CLK direction
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		// pre-assume refclk: phy--> mac
		reg |= 1 << 0;
		reg &= ~(1 << 4);
#ifdef CONFIG_NVT_FPGA_EMULATION
		reg |= 1 << 2;
#endif
#ifdef CONFIG_OF
		// change RMII REF_CLK direction by DTB setting
		if (!of_property_read_u32(np, "ref-clk-out", &refclk_out)) {
			printk("%s: find ref-clk-out %d\r\n", __func__, refclk_out);
			if (refclk_out) {
				printk("%s: change REF_CLK out\r\n", __func__);
				reg &= ~(1 << 0);
				reg |= 1 << 4;
			}
		}
		iowrite32(reg | (1 << 5), (void *)(base + 0x3000 + 0x14));
#endif
		//iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== RGMII =====*/
	} else if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii)) {
		unsigned long reg;

		printk("%s: pinmux detect RGMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RGMII, &v_nvt_plat_info[ETH_QOS_INDEX]);

#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
#if 1
			printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);
			if (spclk_en &&
				(!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;

				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
#else
			if (spclk_en &&
				(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

				ret = nvt_pinmux_update(pinmux_config, 1);
				if (ret)
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
							__func__, __LINE__);

				clk = clk_get(&pdev->dev, "ext_phy_clk");
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
#endif
		}

		// assert phy reset
		set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii, &v_nvt_plat_info[ETH_QOS_INDEX]);
		gpio_direction_output(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		mdelay(20);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 1);
		mdelay(50);

#endif

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));
		//iowrite32(0x102, (void *)(base + 0x3000 + 0x2C)); //SRAM

		// setup TXD_SRC as RGMII
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		//reg |= (1<<0);
		reg |= (3 << 4);
		reg &= ~(3 << 30);
		reg |= 1 << 30;

#ifdef CONFIG_NVT_FPGA_EMULATION
		// coverity[identical_branches]
		if (ETH_QOS_INDEX == 0) {
			// TXCLK_DELAY = 12, RX_DLY = 7;
			reg |= (0 << 16) | (0 << 20);
		} else {
			// TXCLK_DELAY = 8, RX_DLY = 8;
			reg |= (0 << 16) | (0 << 20);
		}
		//reg |= (1<<29);
#endif

		iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== embd phy =====*/
	} else {
		unsigned long reg;

		printk("%s: pinmux detect emb phy 0x%x\r\n", __func__, pinmux_config[0].config);

		nvt_eth_phy_poweron(base);
		*embd_en = 1;

		// set phy mdio address
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg | (1 << 1), (void *)(base + 0x3800 + 0x00));
		reg = ioread32((void *)(base + 0x3800 + 0x1C));
		iowrite32(reg | (1 << 0), (void *)(base + 0x3800 + 0x1C));
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg & ~(1 << 1), (void *)(base + 0x3800 + 0x00));
	}

	return (int)ETH_QOS_INDEX;
}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02201)
/* ---------------------NS02201-------------------------- */
static const struct NVT_PLAT_INFO v_nvt_plat_info[2] = {
	// MAC0 (0xFCC0_0000)
	{
		NVT_ETH_BASE_PHYS,
		"rmii0_refclk_i",
		"eth0_rxclk_i",
		"eth0_txclk_i",
		"eth0_extphy_clk",
		"eth0_ptp_clk",

		PIN_ETH_CFG_ETH_RGMII_1,//PIN_ETH_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_ETH_RGMII_2,//PIN_ETH_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_ETH_RMII_1,//PIN_ETH_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_ETH_RMII_2,//PIN_ETH_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_ETH_EXTPHYCLK,//PIN_ETH_CFG_REFCLK_PINMUX,

		D_GPIO(9),	// gpio reset
		S_GPIO(31),	// gpio reset

		PAD_DS_DGPIO8,	// PHYCLK PIN_1
		PAD_DS_LGPIO24,	// TXCLK PIN_1
		PAD_DS_LGPIO19,	// TXCTL PIN_1
		PAD_DS_LGPIO25,	// REFCLK (RMII) PIN_1
		PAD_DS_LGPIO20,	// TXD[0] PIN_1
		PAD_DS_LGPIO21,	// TXD[1] PIN_1
		PAD_DS_LGPIO22,	// TXD[2] PIN_1
		PAD_DS_LGPIO23,	// TXD[3] PIN_1

		PAD_DS_SGPIO30,	// PHYCLK PIN_2
		PAD_DS_SGPIO21,	// TXCLK PIN_2
		PAD_DS_SGPIO16,	// TXCTL PIN_2
		PAD_DS_SGPIO22,	// REFCLK (RMII) PIN_2
		PAD_DS_SGPIO17,	// TXD[0] PIN_2
		PAD_DS_SGPIO18,	// TXD[1] PIN_2
		PAD_DS_SGPIO19,	// TXD[2] PIN_2
		PAD_DS_SGPIO20,	// TXD[3] PIN_2
	},
	// MAC1 (0xFCD0_0000)
	{
		NVT_ETH2_BASE_PHYS,
		"rmii1_refclk_i",
		"eth1_rxclk_i",
		"eth1_txclk_i",
		"eth1_extphy_clk",
		"eth1_ptp_clk",

		PIN_ETH_CFG_ETH2_RGMII_1,//PIN_ETH2_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_ETH2_RGMII_2,//PIN_ETH2_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_ETH2_RMII_1,//PIN_ETH2_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_ETH2_RMII_2,//PIN_ETH2_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_ETH2_EXTPHYCLK,//PIN_ETH2_CFG_REFCLK_PINMUX,

		L_GPIO(18),	//L_GPIO(18),            // gpio reset
		S_GPIO(15),	//NOT SUP,           // gpio reset

		PAD_DS_LGPIO17,	// PHYCLK
		PAD_DS_LGPIO8,	// TXCLK
		PAD_DS_LGPIO3,	// TXCTL
		PAD_DS_LGPIO9,	// REFCLK (RMII)
		PAD_DS_LGPIO4,	// TXD[0]
		PAD_DS_LGPIO5,	// TXD[1]
		PAD_DS_LGPIO6,	// TXD[2]
		PAD_DS_LGPIO7,	// TXD[3]

		PAD_DS_SGPIO14,	// PHYCLK PIN_2
		PAD_DS_SGPIO5,	// TXCLK PIN_2
		PAD_DS_SGPIO0,	// TXCTL PIN_2
		PAD_DS_SGPIO6,	// REFCLK (RMII) PIN_2
		PAD_DS_SGPIO1,	// TXD[0] PIN_2
		PAD_DS_SGPIO2,	// TXD[1] PIN_2
		PAD_DS_SGPIO3,	// TXD[2] PIN_2
		PAD_DS_SGPIO4,	// TXD[3] PIN_2
	},
};


static void nvt_eth_phy_poweron(void __iomem *base_addr)
{
	unsigned long reg;

	reg = ioread32((void *)(base_addr + 0x3800 + 0xF8));
	iowrite32(reg | (1 << 7), (void *)(base_addr + 0x3800 + 0xF8));
	udelay(20);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(200);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 1)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(250);
	reg = ioread32((void *)(base_addr + 0x3800 + 0x2E8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x2E8));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xCC));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xCC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xDC));
	iowrite32(reg | (1 << 0), (void *)(base_addr + 0x3800 + 0xDC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0x9C));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x9C));
}

static int set_pad_driving(UINT interface, const struct NVT_PLAT_INFO *p_info, int chip_id)
{
	if((interface == PIN_ETH_CFG_ETH_RMII_1) ||
			(interface == PIN_ETH_CFG_ETH2_RMII_1))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			/* Not support in pcie_lib, if needed, setting nvt-gpio.dtsi*/
/*			pad_set_drivingsink_ep(p_info->pad_phy_clk,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_rmii_refclk,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txctl,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d0,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
*/
#endif
		} else {
			pad_set_drivingsink(p_info->pad_phy_clk,
					PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_rmii_refclk,
					PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_txctl,
					PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d0,
					PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d1,
					PAD_DRIVINGSINK_LEVEL_10);
		}
	} else if ((interface == PIN_ETH_CFG_ETH_RGMII_1) ||
			(interface == PIN_ETH_CFG_ETH2_RGMII_1))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			/* Not support in pcie_lib, if needed, setting nvt-gpio.dtsi*/
/*			pad_set_drivingsink_ep(p_info->pad_phy_clk,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txclk,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txctl,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d0,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d1,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d3,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
*/
#endif
		} else {
			pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_txclk, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d3, PAD_DRIVINGSINK_LEVEL_10);
		}
	} else if ((interface == PIN_ETH_CFG_ETH_RMII_2) ||
			(interface == PIN_ETH_CFG_ETH2_RMII_2))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			/* Not support in pcie_lib, if needed, setting nvt-gpio.dtsi*/
/*			pad_set_drivingsink_ep(p_info->pad_phy_clk_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_rmii_refclk_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txctl_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d0_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
*/
#endif
		} else {
			pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_rmii_refclk_2,
					PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d1_2, PAD_DRIVINGSINK_LEVEL_10);
		}
	} else if ((interface == PIN_ETH_CFG_ETH_RGMII_2) ||
			(interface == PIN_ETH_CFG_ETH2_RGMII_2))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			/* Not support in pcie_lib, if needed, setting nvt-gpio.dtsi*/
/*			pad_set_drivingsink_ep(p_info->pad_phy_clk_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txclk_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txctl_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d0_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d1_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d2_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d3_2,
					PAD_DRIVINGSINK_LEVEL_10, chip_id - CHIP_EP0);
*/
#endif
		} else {
			pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_txclk_2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d1_2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d2_2, PAD_DRIVINGSINK_LEVEL_10);
			pad_set_drivingsink(p_info->pad_tx_d3_2, PAD_DRIVINGSINK_LEVEL_10);
		}
	}

	return 0;
}

static int set_pad_pull(UINT interface, const struct NVT_PLAT_INFO *p_info, int chip_id){

	if ((interface == PIN_ETH_CFG_ETH_RMII_1) || (interface == PIN_ETH_CFG_ETH_RGMII_1)) {
		pad_set_pull_updown(PAD_PIN_DGPIO7, PAD_NONE);
	} else if ((interface == PIN_ETH_CFG_ETH_RMII_2) || (interface == PIN_ETH_CFG_ETH_RGMII_2)) {
		pad_set_pull_updown(PAD_PIN_SGPIO29, PAD_NONE);
	} else if((interface == PIN_ETH_CFG_ETH2_RMII_1) || (interface == PIN_ETH_CFG_ETH2_RGMII_1)) {
		pad_set_pull_updown(PAD_PIN_LGPIO16, PAD_NONE);
	} else if ((interface == PIN_ETH_CFG_ETH2_RMII_2) || (interface == PIN_ETH_CFG_ETH2_RGMII_2)) {
		pad_set_pull_updown(PAD_PIN_SGPIO13, PAD_NONE);
	}

	return 0;
}

static int get_pad_driving(UINT interface, const struct NVT_PLAT_INFO *p_info, int chip_id)
{
	PAD_DRIVINGSINK driving[7] = {0};

	if((interface == PIN_ETH_CFG_ETH_RMII_1) ||
			(interface == PIN_ETH_CFG_ETH2_RMII_1))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
/*			pad_get_drivingsink_ep(p_info->pad_phy_clk,
					&driving[0], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_rmii_refclk,
					&driving[1], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txctl,
					&driving[2], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d0,
					&driving[3], chip_id - CHIP_EP0);
			printk("ETH RMII PIN, driving level: %d%d%d%d\n", driving[0], driving[1], driving[2], driving[3]);
*/
#endif
		} else {
			pad_get_drivingsink(p_info->pad_phy_clk, &driving[0]);
			pad_get_drivingsink(p_info->pad_rmii_refclk, &driving[1]);
			pad_get_drivingsink(p_info->pad_txctl, &driving[2]);
			pad_get_drivingsink(p_info->pad_tx_d0, &driving[3]);
			pad_get_drivingsink(p_info->pad_tx_d1, &driving[4]);
			printk("ETH RMII PIN, driving level: %d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4]);
		}
	} else if ((interface == PIN_ETH_CFG_ETH_RGMII_1) ||
			(interface == PIN_ETH_CFG_ETH2_RGMII_1))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
/*
			pad_get_drivingsink_ep(p_info->pad_phy_clk,
					&driving[0], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txclk,
					&driving[1], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txctl,
					&driving[2], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d0,
					&driving[3], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d1,
					&driving[4], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d2,
					&driving[5], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d3,
					&driving[6], chip_id - CHIP_EP0);
			printk("ETH RGMII PIN, driving level: %d%d%d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4], driving[5], driving[6]);
*/
#endif
		} else {
			pad_get_drivingsink(p_info->pad_phy_clk, &driving[0]);
			pad_get_drivingsink(p_info->pad_txclk, &driving[1]);
			pad_get_drivingsink(p_info->pad_txctl, &driving[2]);
			pad_get_drivingsink(p_info->pad_tx_d0, &driving[3]);
			pad_get_drivingsink(p_info->pad_tx_d1, &driving[4]);
			pad_get_drivingsink(p_info->pad_tx_d2, &driving[5]);
			pad_get_drivingsink(p_info->pad_tx_d3, &driving[6]);
			printk("ETH RGMII PIN, driving level: %d%d%d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4], driving[5], driving[6]);
		}
	} else if ((interface == PIN_ETH_CFG_ETH_RMII_2) ||
			(interface == PIN_ETH_CFG_ETH2_RMII_2))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
/*			pad_get_drivingsink_ep(p_info->pad_phy_clk_2,
					&driving[0], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_rmii_refclk_2,
					&driving[1], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txctl_2,
					&driving[2], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d0_2,
					&driving[3], chip_id - CHIP_EP0);
			printk("ETH RMII PIN, driving level: %d%d%d%d\n", driving[0], driving[1], driving[2], driving[3]);
*/
#endif
		} else {
			pad_get_drivingsink(p_info->pad_phy_clk_2, &driving[0]);
			pad_get_drivingsink(p_info->pad_rmii_refclk_2, &driving[1]);
			pad_get_drivingsink(p_info->pad_txctl_2, &driving[2]);
			pad_get_drivingsink(p_info->pad_tx_d0_2, &driving[3]);
			pad_get_drivingsink(p_info->pad_tx_d1_2, &driving[4]);
			printk("ETH RMII PIN, driving level: %d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4]);
		}
	} else if ((interface == PIN_ETH_CFG_ETH_RGMII_2) ||
			(interface == PIN_ETH_CFG_ETH2_RGMII_2))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
/*			pad_get_drivingsink_ep(p_info->pad_phy_clk_2,
					&driving[0], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txclk_2,
					&driving[1], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txctl_2,
					&driving[2], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d0_2,
					&driving[3], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d1_2,
					&driving[4], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d2_2,
					&driving[5], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d3_2,
					&driving[6], chip_id - CHIP_EP0);
			printk("ETH RGMII PIN, driving level: %d%d%d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4], driving[5], driving[6]);
*/
#endif
		} else {
			pad_get_drivingsink(p_info->pad_phy_clk_2, &driving[0]);
			pad_get_drivingsink(p_info->pad_txclk_2, &driving[1]);
			pad_get_drivingsink(p_info->pad_txctl_2, &driving[2]);
			pad_get_drivingsink(p_info->pad_tx_d0_2, &driving[3]);
			pad_get_drivingsink(p_info->pad_tx_d1_2, &driving[4]);
			pad_get_drivingsink(p_info->pad_tx_d2_2, &driving[5]);
			pad_get_drivingsink(p_info->pad_tx_d3_2, &driving[6]);
			printk("ETH RGMII PIN, driving level: %d%d%d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4], driving[5], driving[6]);
		}
	}

	return 0;
}

int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base)
{
	int ret = 0;
	struct resource *res;
	void __iomem *base;
	PIN_GROUP_CONFIG pinmux_config[1];
	UINT ETH_QOS_INDEX; // index to MAC (0: MAC0, 1: MAC1);
#if IS_ENABLED(CONFIG_TIMESTAMP)
	struct clk *ptpclk;
#endif
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;
	int spclk_en = 0;
	int refclk_out = 0;
#endif
#if IS_ENABLED(CONFIG_MULTI_IRQ)
	unsigned long reg;
#endif
	UINT reset_gpio, reset_gpio_2;
	int chip_id = 0;

//	ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No memory resources\n");
		return -1;
	}
	printk("%s: IO MEM res start 0x%llx\r\n", __func__, res->start);

	if (res->start == v_nvt_plat_info[0].base_pa) {
		ETH_QOS_INDEX = 0;
	} else {
		ETH_QOS_INDEX = 1;
	}
	printk("%s: ETH_QOS_INDEX %d\n", __func__, ETH_QOS_INDEX);
#if IS_ENABLED(CONFIG_PCI)
	chip_id = nvt_eth_dts_chipid(np);
	printk("%s: chip_id = %d\n", __func__, chip_id);

	if (nvtpcie_is_common_pci() || nvt_eth_dts_chipid(np) != CHIP_RC) {
		if (0 != nvtpcie_conv_resource(nvt_eth_dts_chipid(np), res)) {
			printk("conv res failed, %pR\n", res);
		}
		printk("%s: IO MEM res start 0x%llx\r\n", __func__, res->start);
	}

	if (nvtpcie_is_common_pci()) {
		base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	} else {
		base = devm_ioremap_resource(&pdev->dev, res);
	}
#else
	base = devm_ioremap_resource(&pdev->dev, res);
#endif

	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to find IORESOURCE_MEM\n");
		return -1;
//		ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	} else {
		*p_base = base;
//		ETH_QOS_REG_BASE = base;
	}
//	ETH_QOS_INDEX = 1;   // temp force to MAC1 (for embedded phy)
	printk("%s: get IO MEM 0x%px\r\n", __func__, base);

	pinmux_config[0].pin_function = PIN_FUNC_ETH;
#if IS_ENABLED(CONFIG_PCI)
	if (nvt_eth_dts_chipid(np) > CHIP_RC) {
		ret = nvt_eth_ep_pinmux_capture(pinmux_config, np);
	} else {
		ret = nvt_pinmux_capture(pinmux_config, 1);
	}
#else
	ret = nvt_pinmux_capture(pinmux_config, 1);
#endif
	if (ret)
		dev_err(&pdev->dev, "get pinmux config failed\n");

	printk("%s: get pinmux 0x%x\r\n", __func__, pinmux_config[0].config);

#ifdef CONFIG_OF
	if (of_property_read_u32(np, "reset_gpio", &reset_gpio))
#endif
		reset_gpio = v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset;

#ifdef CONFIG_OF
	if (of_property_read_u32(np, "reset_gpio_2", &reset_gpio_2))
#endif
		reset_gpio_2 = v_nvt_plat_info[ETH_QOS_INDEX].gpio_2_reset;

#if IS_ENABLED(CONFIG_TIMESTAMP)
	if (chip_id == 0) {
		ptpclk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
		if (IS_ERR(ptpclk)) {
			dev_err(&pdev->dev, "can't find ptp_clock %s\n",
				v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
			ptpclk = NULL;
		} else {
			clk_prepare(ptpclk);
			clk_enable(ptpclk);
		}
		clk_put(ptpclk);
	}
#endif

#if IS_ENABLED(CONFIG_MULTI_IRQ)

	reg = ioread32((void *)(base + 0x3004));
	iowrite32(reg | (1 << 0), (void *)(base + 0x3004));

	reg = ioread32((void *)(base + 0x3028));
	reg = reg & ~(3 << 0);
	iowrite32(reg | (3 << 2), (void *)(base + 0x3028));
#endif

	// impelment MAC1
	/*==== RMII =====*/
	if (pinmux_config[0].config &
			(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii |
			 v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RMII, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			if (spclk_en &&
				(!(pinmux_config[0].config &
				   v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {
#if 1

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;

#if IS_ENABLED(CONFIG_PCI)
				if (nvt_eth_dts_chipid(np) > 0) {
					/* Not support in pcie_lib*/
//					ret = nvt_pinmux_update_ep(pinmux_config, 1,
//						nvt_eth_dts_chipid(np) - CHIP_EP0);
				} else {
					ret = nvt_pinmux_update(pinmux_config, 1);
				}
#else
				ret = nvt_pinmux_update(pinmux_config, 1);
#endif

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n", __func__, __LINE__);
				}

				printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);

			}

			if (spclk_en && (chip_id == 0)) {
				struct clk *clk;
				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					if (!__clk_is_enabled(clk)) {
						clk_prepare(clk);
						clk_enable(clk);
					}
				}
				clk_put(clk);
			}
#else
				if (spclk_en &&
					(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
					struct clk *clk;

					printk("%s: change extphyclk pinmux\r\n", __func__);
					pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

#if IS_ENABLED(CONFIG_PCI)
					if (nvt_eth_dts_chipid(np) > 0) {
						ret = nvt_pinmux_update_ep(pinmux_config, 1,
							nvt_eth_dts_chipid(np) - CHIP_EP0);
					} else {
						ret = nvt_pinmux_update(pinmux_config, 1);
					}
#else
					ret = nvt_pinmux_update(pinmux_config, 1);
#endif
					if (ret)
						pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
								__func__, __LINE__);

					clk = clk_get(&pdev->dev, "ext_phy_clk");
					if (IS_ERR(clk)) {
						dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
						clk = NULL;
					} else {
						clk_prepare(clk);
						clk_enable(clk);
					}
					clk_put(clk);
				}
#endif
		}

		// assert phy reset
		if(pinmux_config[0].config &
				v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii){
			set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			get_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			set_pad_pull(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
#if IS_ENABLED(CONFIG_PCI)
			if (nvt_eth_dts_chipid(np) > 0) {
				nvtpcie_gpio_direction_output(nvt_eth_dts_chipid(np), reset_gpio, 0);
				nvtpcie_gpio_set_value(nvt_eth_dts_chipid(np), reset_gpio, 0);
				mdelay(20);
				nvtpcie_gpio_set_value(nvt_eth_dts_chipid(np), reset_gpio, 1);
				mdelay(50);

			} else {
				gpio_direction_output(reset_gpio, 0);
				gpio_set_value(reset_gpio, 0);
				mdelay(20);
				gpio_set_value(reset_gpio, 1);
				mdelay(50);
			}
#else
			gpio_direction_output(reset_gpio, 0);
			gpio_set_value(reset_gpio, 0);
			mdelay(20);
			gpio_set_value(reset_gpio, 1);
			mdelay(50);
#endif
		} else {
			set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			get_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			set_pad_pull(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
#if IS_ENABLED(CONFIG_PCI)
			if (nvt_eth_dts_chipid(np) > 0) {
				nvtpcie_gpio_direction_output(nvt_eth_dts_chipid(np), reset_gpio_2, 0);
				nvtpcie_gpio_set_value(nvt_eth_dts_chipid(np), reset_gpio_2, 0);
				mdelay(20);
				nvtpcie_gpio_set_value(nvt_eth_dts_chipid(np), reset_gpio_2, 1);
				mdelay(50);

			} else {
				gpio_direction_output(reset_gpio_2, 0);
				gpio_set_value(reset_gpio_2, 0);
				mdelay(20);
				gpio_set_value(reset_gpio_2, 1);
				mdelay(50);
			}
#else
			gpio_direction_output(reset_gpio_2, 0);
			gpio_set_value(reset_gpio_2, 0);
			mdelay(20);
			gpio_set_value(reset_gpio_2, 1);
			mdelay(50);
#endif
		}
#endif

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
#ifdef CONFIG_NVT_FPGA_EMULATION
		//reg |= 1<<31; // output at clock falling
#endif
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));

		// setup RMII REF_CLK direction
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		// pre-assume refclk: phy--> mac
		reg |= 1 << 0;
		reg &= ~(1 << 4);
#ifdef CONFIG_NVT_FPGA_EMULATION
		reg |= 1 << 2;
#endif
#ifdef CONFIG_OF
		// change RMII REF_CLK direction by DTB setting
		if (!of_property_read_u32(np, "ref-clk-out", &refclk_out)) {
			printk("%s: find ref-clk-out %d\r\n", __func__, refclk_out);
			if (refclk_out) {
				printk("%s: change REF_CLK out\r\n", __func__);
				reg &= ~(1 << 0);
				reg |= 1 << 4;
			}
		}
		iowrite32(reg | (1 << 5), (void *)(base + 0x3000 + 0x14));
#endif
		//iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== RGMII =====*/
	} else if (pinmux_config[0].config &
			(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii |
			 v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RGMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RGMII, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);

#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
#if 1
			printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);
			if (spclk_en &&
				(!(pinmux_config[0].config &
				   v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;

#if IS_ENABLED(CONFIG_PCI)
				if (nvt_eth_dts_chipid(np) > 0) {
					/* Not support in pcie_lib*/
//					ret = nvt_pinmux_update_ep(pinmux_config, 1,
//						nvt_eth_dts_chipid(np) - CHIP_EP0);
				} else {
					ret = nvt_pinmux_update(pinmux_config, 1);
				}
#else
				ret = nvt_pinmux_update(pinmux_config, 1);
#endif

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
							__func__, __LINE__);
				}
			}

			if (spclk_en && (chip_id == 0)) {
				struct clk *clk;
				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					if (!__clk_is_enabled(clk)) {
						clk_prepare(clk);
						clk_enable(clk);
					}
				}
				clk_put(clk);
			}
#else
			if (spclk_en &&
				(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

#if IS_ENABLED(CONFIG_PCI)
				if (nvt_eth_dts_chipid(np) > 0) {
					ret = nvt_pinmux_update_ep(pinmux_config, 1,
						nvt_eth_dts_chipid(np) - CHIP_EP0);
				} else {
					ret = nvt_pinmux_update(pinmux_config, 1);
				}
#else
				ret = nvt_pinmux_update(pinmux_config, 1);
#endif

				if (ret)
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
							__func__, __LINE__);

				clk = clk_get(&pdev->dev, "ext_phy_clk");
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
#endif
		}

		// assert phy reset
		if(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii){
			set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			get_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			set_pad_pull(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
#if IS_ENABLED(CONFIG_PCI)
			if (nvt_eth_dts_chipid(np) > 0) {
				nvtpcie_gpio_direction_output(nvt_eth_dts_chipid(np), reset_gpio, 0);
				nvtpcie_gpio_set_value(nvt_eth_dts_chipid(np), reset_gpio, 0);
				mdelay(20);
				nvtpcie_gpio_set_value(nvt_eth_dts_chipid(np), reset_gpio, 1);
				mdelay(50);

			} else {
				gpio_direction_output(reset_gpio, 0);
				gpio_set_value(reset_gpio, 0);
				mdelay(20);
				gpio_set_value(reset_gpio, 1);
				mdelay(50);
			}
#else
			gpio_direction_output(reset_gpio, 0);
			gpio_set_value(reset_gpio, 0);
			mdelay(20);
			gpio_set_value(reset_gpio, 1);
			mdelay(50);
#endif
		} else {
			set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			get_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			set_pad_pull(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
#if IS_ENABLED(CONFIG_PCI)
			if (nvt_eth_dts_chipid(np) > 0) {
				nvtpcie_gpio_direction_output(nvt_eth_dts_chipid(np), reset_gpio_2, 0);
				nvtpcie_gpio_set_value(nvt_eth_dts_chipid(np), reset_gpio_2, 0);
				mdelay(20);
				nvtpcie_gpio_set_value(nvt_eth_dts_chipid(np), reset_gpio_2, 1);
				mdelay(50);

			} else {
				gpio_direction_output(reset_gpio_2, 0);
				gpio_set_value(reset_gpio_2, 0);
				mdelay(20);
				gpio_set_value(reset_gpio_2, 1);
				mdelay(50);
			}
#else
			gpio_direction_output(reset_gpio_2, 0);
			gpio_set_value(reset_gpio_2, 0);
			mdelay(20);
			gpio_set_value(reset_gpio_2, 1);
#endif
		}

#endif

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));
		//iowrite32(0x102, (void *)(base + 0x3000 + 0x2C)); //SRAM

		// setup TXD_SRC as RGMII
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		//reg |= (1<<0);
		reg |= (3 << 4);
		reg &= ~(3 << 30);
		reg |= 1 << 30;

#ifdef CONFIG_NVT_FPGA_EMULATION
		// coverity[identical_branches]
		if (ETH_QOS_INDEX == 0) {
			// TXCLK_DELAY = 12, RX_DLY = 7;
			reg |= (0 << 16) | (0 << 20);
		} else {
			// TXCLK_DELAY = 8, RX_DLY = 8;
			reg |= (0 << 16) | (0 << 20);
		}
		//reg |= (1<<29);
#endif

		iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== embd phy =====*/
	} else {
		unsigned long reg;

		printk("%s: pinmux detect emb phy 0x%x\r\n", __func__, pinmux_config[0].config);

		nvt_eth_phy_poweron(base);

		// set phy mdio address
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg | (1 << 1), (void *)(base + 0x3800 + 0x00));
		reg = ioread32((void *)(base + 0x3800 + 0x1C));
		iowrite32(reg | (1 << 0), (void *)(base + 0x3800 + 0x1C));
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg & ~(1 << 1), (void *)(base + 0x3800 + 0x00));
	}

	return (int)ETH_QOS_INDEX;
}

#elif defined(CONFIG_NVT_IVOT_PLAT_NA51055)
/* ---------------------NA51055-------------------------- */
static void nvt_eth_phy_poweron(void __iomem *base_addr)
{
	unsigned long reg;

	reg = ioread32((void *)(base_addr + 0x3800 + 0xF8));
	iowrite32(reg | (1 << 7), (void *)(base_addr + 0x3800 + 0xF8));
	udelay(20);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(200);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 1)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(250);
	reg = ioread32((void *)(base_addr + 0x3800 + 0x2E8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x2E8));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xCC));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xCC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xDC));
	iowrite32(reg | (1 << 0), (void *)(base_addr + 0x3800 + 0xDC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0x9C));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x9C));
}

int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base, int *embd_en)
{
	int ret = 0;
	struct resource *res;
	void __iomem *base;
	PIN_GROUP_CONFIG pinmux_config[1];
	UINT ETH_QOS_INDEX = 0; // index to MAC (0: MAC0, 1: MAC1);
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;
	int spclk_en = 0;
	int refclk_out = 0;
#endif
//	ETH_QOS_REG_BASE = (void __iomem *)(0xFD2B0000);    // pre-assume base address
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No memory resources\n");
		return -1;
	}
	printk("%s: IO MEM res start 0x%x\r\n", __func__, res->start);

	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to find IORESOURCE_MEM\n");
		return -1;
//		ETH_QOS_REG_BASE = (void __iomem *)(0xFD2B0000);    // pre-assume base address
	} else {
		*p_base = base;
//		ETH_QOS_REG_BASE = base;
	}
	printk("%s: get IO MEM 0x%px\r\n", __func__, base);

	pinmux_config[0].pin_function = PIN_FUNC_ETH;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		dev_err(&pdev->dev, "Get eth pinmux fail\n");

	if (pinmux_config[0].config & (PIN_ETH_CFG_RMII|PIN_ETH_CFG_RMII_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RMII 0x%x\r\n", __func__, pinmux_config[0].config);
		if (nvt_get_chip_id() != CHIP_NA51055) {
			struct clk *eth_clk;
			struct clk *src_clk;

			eth_clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
			if (IS_ERR(eth_clk)) {
				pr_err("%s: failed to get eth clock\n", __func__);
				eth_clk = NULL;
			} else {
				src_clk = clk_get(NULL, "pll16");
				if (IS_ERR(src_clk)) {
					pr_err("%s: failed to get pll16\n", __func__);
					src_clk = NULL;
				}
				clk_set_parent(eth_clk, src_clk);
				clk_put(src_clk);
			}
			clk_put(eth_clk);
		}
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if(!of_property_read_u32(np, "sp-clk", &spclk_en)) {
			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			if(spclk_en &&
					(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

				ret = nvt_pinmux_update(pinmux_config, 1);
				if (ret)
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
					        __func__,__LINE__);

				clk = clk_get(&pdev->dev, "ext_phy_clk");
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
		}

		// assert phy reset
		gpio_direction_output(L_GPIO(22), 0);
		gpio_set_value(L_GPIO(22), 0);
		mdelay(20);
		gpio_set_value(L_GPIO(22), 1);
		mdelay(50);
#endif

		// select external phy
		reg = ioread32((void*)(base + 0x3000 + 0x4));
		iowrite32(reg | (1<<8) | (1<<31), (void *)(base + 0x3000 + 0x4));

		// setup RMII REF_CLK direction
		reg = ioread32((void*)(base + 0x3000 + 0x14));
		// pre-assume refclk: phy--> mac
		reg |= 1<<0;
		reg &= ~(1<<4);
#ifdef CONFIG_OF
		// change RMII REF_CLK direction by DTB setting
		if(!of_property_read_u32(np, "ref-clk-out", &refclk_out)) {
			printk("%s: find ref-clk-out %d\r\n", __func__, refclk_out);
			if(refclk_out) {
				printk("%s: change REF_CLK out\r\n", __func__);
				reg &= ~(1<<0);
				reg |= 1<<4;
			}
		}
#endif
		iowrite32(reg | (1<<5), (void *)(base + 0x3000 + 0x14));
	} else if ((pinmux_config[0].config & PIN_ETH_CFG_RGMII) &&
			(nvt_get_chip_id() != CHIP_NA51055)) {
		unsigned long reg;
		struct clk *eth_clk;
		struct clk *src_clk;

		printk("%s: pinmux detect RGMII 0x%x\r\n", __func__, pinmux_config[0].config);
		eth_clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
                if (IS_ERR(eth_clk)) {
                        pr_err("%s: failed to get eth clock\n", __func__);
                        eth_clk = NULL;
                } else {
                        src_clk = clk_get(NULL, "pll16");
                        if (IS_ERR(src_clk)) {
                                pr_err("%s: failed to get pll16\n", __func__);
                                src_clk = NULL;
                        }
                        clk_set_parent(eth_clk, src_clk);
                        clk_put(src_clk);
                }
                clk_put(eth_clk);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if(!of_property_read_u32(np, "sp-clk", &spclk_en)) {
			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			if(spclk_en &&
					(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

				ret = nvt_pinmux_update(pinmux_config, 1);
				if (ret)
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
					        __func__,__LINE__);

				clk = clk_get(&pdev->dev, "ext_phy_clk");
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
		}

		// assert phy reset
		gpio_direction_output(L_GPIO(22), 0);
		gpio_set_value(L_GPIO(22), 0);
		mdelay(20);
		gpio_set_value(L_GPIO(22), 1);
		mdelay(50);
#endif

		// select external phy
		reg = ioread32((void*)(base + 0x3000 + 0x4));
		iowrite32(reg | (1<<8), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));

		// setup TXD_SRC as RGMII
		reg = ioread32((void*)(base + 0x3000 + 0x14));
		reg |= 1<<4;
		reg &= ~(3<<30);
		reg |= 1<<30;
		iowrite32(reg, (void *)(base + 0x3000 + 0x14));
	} else {
		int emb_phy_driving;

		printk("%s: pinmux detect emb phy 0x%x\r\n", __func__, pinmux_config[0].config);
		nvt_eth_phy_poweron(base);
		*embd_en = 1;
		if(!of_property_read_u32(np, "emb-phy-driving", &emb_phy_driving)) {
			unsigned long reg;
			unsigned long step;
			int value;

			reg = ioread32((void*)(base + 0x3800 + 0x378));
			step = reg & 0x1F;
			reg &= ~0x1F;
			// transform 2's compelment to integer
			if (step & 0x10) {
				step = ~step;
				step++;
				value = step & 0x1F;
				value = 0 - value;
			} else {
				value = step & 0xF;
			}
			value += emb_phy_driving;
			// transform integer to 2's complement
			if (value >= 0) {
				if (value > 15) value = 15;
				step = value;
			} else {
				if (value <= -16) value = -16;
				value = 0 - value;
				step = value;
				step = ~step;
				step++;
			}
			step &= 0x1F;
			reg |= step;
			iowrite32(reg, (void *)(base + 0x3800 + 0x378));

			printk("%s: DTS emb driving inc %d\r\n", __func__, emb_phy_driving);
		}
	}

	return (int)ETH_QOS_INDEX;
}

#elif defined(CONFIG_NVT_IVOT_PLAT_NA51089)
static void nvt_eth_phy_poweron(void __iomem *base_addr)
{
	unsigned long reg;

	reg = ioread32((void *)(base_addr + 0x3800 + 0xF8));
	iowrite32(reg | (1 << 7), (void *)(base_addr + 0x3800 + 0xF8));
	udelay(20);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(200);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 1)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(250);
	reg = ioread32((void *)(base_addr + 0x3800 + 0x2E8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x2E8));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xCC));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xCC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xDC));
	iowrite32(reg | (1 << 0), (void *)(base_addr + 0x3800 + 0xDC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0x9C));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x9C));
}

int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base)
{
	int ret = 0;
	struct resource *res;
	void __iomem *base;
	PIN_GROUP_CONFIG pinmux_config[1];
	UINT ETH_QOS_INDEX = 0; // index to MAC (0: MAC0, 1: MAC1);
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;
	int spclk_en = 0;
	int refclk_out = 0;
#endif

//	ETH_QOS_REG_BASE = (void __iomem *)(0xFD2B0000);    // pre-assume base address
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No memory resources\n");
		return -1;
	}
	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to find IORESOURCE_MEM, assume 0xFD2B0000\n");
		return -1;
//		ETH_QOS_REG_BASE = (void __iomem *)(0xFD2B0000);    // pre-assume base address
	} else {
		*p_base = base;
//		ETH_QOS_REG_BASE = base;
	}
	printk("%s: get IO MEM 0x%px\r\n", __func__, base);

	pinmux_config[0].pin_function = PIN_FUNC_ETH;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		dev_err(&pdev->dev, "Get eth pinmux fail\n");

	if (pinmux_config[0].config & (PIN_ETH_CFG_RMII|PIN_ETH_CFG_RMII_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RMII 0x%x\r\n", __func__, pinmux_config[0].config);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if(!of_property_read_u32(np, "sp-clk", &spclk_en)) {
			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			if(spclk_en &&
					(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

				ret = nvt_pinmux_update(pinmux_config, 1);
				if (ret)
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
					        __func__,__LINE__);

				clk = clk_get(&pdev->dev, "ext_phy_clk");
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
		}

		if(pinmux_config[0].config & (PIN_ETH_CFG_RMII)){
			pad_set_drivingsink(PAD_DS_DSIGPIO5,PAD_DRIVINGSINK_8MA);
			// assert phy reset
			gpio_direction_output(DSI_GPIO(6), 0);
			gpio_set_value(DSI_GPIO(6), 0);
			mdelay(20);
			gpio_set_value(DSI_GPIO(6), 1);
			mdelay(50);
		}else{
			pad_set_drivingsink(PAD_DS_PGPIO7,PAD_DRIVINGSINK_8MA);
			pad_set_drivingsink(PAD_DS_PGPIO8,PAD_DRIVINGSINK_8MA);
			gpio_direction_output(P_GPIO(11), 0);
                        gpio_set_value(P_GPIO(11), 0);
                        mdelay(20);
                        gpio_set_value(P_GPIO(11), 1);
                        mdelay(50);
		}

#endif

		// select external phy
		reg = ioread32((void*)(base + 0x3000 + 0x4));
		reg &= ~(1<<31);
		iowrite32(reg | (1<<4) , (void *)(base + 0x3000 + 0x4));

		// setup RMII REF_CLK direction
		reg = ioread32((void*)(base + 0x3000 + 0x14));
		// pre-assume refclk: phy--> mac
		reg |= 1<<0;
		reg &= ~(1<<4);
#ifdef CONFIG_OF
		// change RMII REF_CLK direction by DTB setting
		if(!of_property_read_u32(np, "ref-clk-out", &refclk_out)) {
			printk("%s: find ref-clk-out %d\r\n", __func__, refclk_out);
			if(refclk_out) {
				printk("%s: change REF_CLK out\r\n", __func__);
				reg &= ~(1<<0);
				reg |= 1<<4;
			}
		}
#endif
		iowrite32(reg | (1<<5), (void *)(base + 0x3000 + 0x14));
	} else {
		int emb_phy_driving;

		printk("%s: pinmux detect emb phy 0x%x\r\n", __func__, pinmux_config[0].config);
		nvt_eth_phy_poweron(base);
		if(!of_property_read_u32(np, "emb-phy-driving", &emb_phy_driving)) {
			unsigned long reg;
			unsigned long step;
			int value;

			reg = ioread32((void*)(base + 0x3800 + 0x378));
			step = reg & 0x1F;
			reg &= ~0x1F;
			// transform 2's compelment to integer
			if (step & 0x10) {
				step = ~step;
				step++;
				value = step & 0x1F;
				value = 0 - value;
			} else {
				value = step & 0xF;
			}
			value += emb_phy_driving;
			// transform integer to 2's complement
			if (value >= 0) {
				if (value > 15) value = 15;
				step = value;
			} else {
				if (value <= -16) value = -16;
				value = 0 - value;
				step = value;
				step = ~step;
				step++;
			}
			step &= 0x1F;
			reg |= step;
			iowrite32(reg, (void *)(base + 0x3800 + 0x378));

			printk("%s: DTS emb driving inc %d\r\n", __func__, emb_phy_driving);
		}
	}
	return (int)ETH_QOS_INDEX;
}

#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)

/* ---------------------NS02302-------------------------- */
static const struct NVT_PLAT_INFO v_nvt_plat_info[2] = {
	// MAC0 (0xFCC0_0000)
	{
		NVT_ETH_BASE_PHYS,
		"rmii0_refclk_i",
		"eth0_rxclk_i",
		"eth0_txclk_i",
		"eth0_extphy_clk",
		"eth0_ptp_clk",

		PIN_ETH_CFG_NONE,//PIN_ETH_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_ETH_RGMII,//PIN_ETH_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_ETH_RMII_1,//PIN_ETH_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_ETH_RMII_2,//PIN_ETH_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_ETH_EXTPHYCLK,//PIN_ETH_CFG_REFCLK_PINMUX,

		L_GPIO(10),//L_GPIO(10),          // gpio reset
		D_GPIO(1),//D_GPIO(1),            // gpio reset

		PAD_DS_LGPIO9,//PAD_DS_LGPIO9,  // PHYCLK PIN_1
		PAD_NONE,//NOT SUP,    // TXCLK PIN_1
		PAD_DS_LGPIO0,//PAD_DS_LGPIO19,    // TXCTL PIN_1
		PAD_DS_LGPIO3,//PAD_DS_LGPIO25,    // REFCLK (RMII) PIN_1
		PAD_DS_LGPIO1,//PAD_DS_LGPIO20,    // TXD[0] PIN_1
		PAD_DS_LGPIO2,//PAD_DS_LGPIO21,    // TXD[1] PIN_1
		PAD_NONE,//NOT SUP,    // TXD[2] PIN_1
		PAD_NONE,//NOT SUP,    // TXD[3] PIN_1

		PAD_DS_DGPIO0,//PAD_DS_DGPIO0,    // PHYCLK PIN_2
		PAD_DS_LGPIO16,//PAD_DS_LGPIO16,   // TXCLK PIN_2
		PAD_DS_LGPIO11,//PAD_DS_LGPIO11,   // TXCTL PIN_2
		PAD_DS_LGPIO17,//PAD_DS_LGPIO17,   // REFCLK (RMII) PIN_2
		PAD_DS_LGPIO12,//PAD_DS_LGPIO12,   // TXD[0] PIN_2
		PAD_DS_LGPIO13,//PAD_DS_LGPIO13,   // TXD[1] PIN_2
		PAD_DS_LGPIO14,//PAD_DS_LGPIO14,   // TXD[2] PIN_2
		PAD_DS_LGPIO15,//PAD_DS_LGPIO15,   // TXD[3] PIN_2
	},
	// MAC1 NOT SUP
	{
		0,
		"rmii1_refclk_i",
		"eth1_rxclk_i",
		"eth1_txclk_i",
		"eth1_extphy_clk",
		"eth1_ptp_clk",

		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_REFCLK_PINMUX,

		L_GPIO(18),//L_GPIO(18),            // gpio reset
		L_GPIO(18),//NOT SUP,           // gpio reset

		PAD_NONE,//NOT SUP,  // PHYCLK
		PAD_NONE,//NOT SUP,  // TXCLK
		PAD_NONE,//NOT SUP,  // TXCTL
		PAD_NONE,//NOT SUP,  // REFCLK (RMII)
		PAD_NONE,//NOT SUP,  // TXD[0]
		PAD_NONE,//NOT SUP,  // TXD[1]
		PAD_NONE,//NOT SUP,  // TXD[2]
		PAD_NONE,//NOT SUP,  // TXD[3]

		PAD_NONE,//NOT SUP,  // PHYCLK PIN_2
		PAD_NONE,//NOT SUP,  // TXCLK PIN_2
		PAD_NONE,//NOT SUP,  // TXCTL PIN_2
		PAD_NONE,//NOT SUP,  // REFCLK (RMII) PIN_2
		PAD_NONE,//NOT SUP,  // TXD[0] PIN_2
		PAD_NONE,//NOT SUP,  // TXD[1] PIN_2
		PAD_NONE,//NOT SUP,  // TXD[2] PIN_2
		PAD_NONE,//NOT SUP,  // TXD[3] PIN_2
	},
};


static void nvt_eth_phy_poweron(void __iomem *base_addr)
{
	unsigned long reg;

	reg = ioread32((void *)(base_addr + 0x3800 + 0xF8));
	iowrite32(reg | (1 << 7), (void *)(base_addr + 0x3800 + 0xF8));
	udelay(20);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(200);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 1)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(250);
	reg = ioread32((void *)(base_addr + 0x3800 + 0x2E8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x2E8));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xCC));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xCC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xDC));
	iowrite32(reg | (1 << 0), (void *)(base_addr + 0x3800 + 0xDC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0x9C));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x9C));
}

static int set_pad_driving(UINT interface, const struct NVT_PLAT_INFO *p_info)
{

	if ((interface == PIN_ETH_CFG_ETH_RMII_1)) {
		pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_rmii_refclk, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_1);
	} else if (interface == PIN_ETH_CFG_ETH_RMII_2) {
		pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_rmii_refclk_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d1_2, PAD_DRIVINGSINK_LEVEL_1);
	} else if (interface == PIN_ETH_CFG_ETH_RGMII) {

		pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txclk_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d1_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d2_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d3_2, PAD_DRIVINGSINK_LEVEL_1);
	}

	return 0;
}

static void nvt_eth_assert_phy_reset(UINT pinmux_config, UINT ETH_QOS_INDEX)
{
	UINT interface, gpio_reset;

	/* PIN_ETH_CFG_ETH_RMII_1 */
	if (pinmux_config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii) {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset;

	/* PIN_ETH_CFG_ETH_RMII_2 */
	} else if (pinmux_config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2) {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_2_reset;
	/* PIN_ETH_CFG_ETH_RGMII */
	} else if (pinmux_config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii) {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset;
	/* PIN_ETH_CFG_ETH_RGMII_2 */
	} else if (pinmux_config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2) {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_2_reset;
	} else {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset;
	}

	printk("%s, %d, interface = 0x%x, gpio_reset = 0x%x\n", __func__, __LINE__, interface, gpio_reset);
	set_pad_driving(interface, &v_nvt_plat_info[ETH_QOS_INDEX]);
	gpio_direction_output(gpio_reset, 0);
	gpio_set_value(gpio_reset, 0);
	mdelay(20);
	gpio_set_value(gpio_reset, 1);
	mdelay(50);
}

int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base, int *embd_en)
{
	int ret = 0;
	struct resource *res;
	void __iomem *base;
	PIN_GROUP_CONFIG pinmux_config[1];
	UINT ETH_QOS_INDEX; // index to MAC (0: MAC0, 1: MAC1);
#if IS_ENABLED(CONFIG_TIMESTAMP)
	struct clk *ptpclk;
#endif
#if IS_ENABLED(CONFIG_MULTI_IRQ)
	unsigned long reg;
#endif
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;
	int spclk_en = 0;
	int refclk_out = 0;
#endif
//	ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No memory resources\n");
		return -1;
	}
	printk("%s: IO MEM res start 0x%llx\r\n", __func__, res->start);

	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to find IORESOURCE_MEM\n");
		return -1;
//		ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	} else {
		*p_base = base;
//		ETH_QOS_REG_BASE = base;
	}
	if (res->start == v_nvt_plat_info[0].base_pa) {
		ETH_QOS_INDEX = 0;
	} else {
		ETH_QOS_INDEX = 1;
	}
//	ETH_QOS_INDEX = 1;   // temp force to MAC1 (for embedded phy)
	printk("%s: get IO MEM 0x%px\r\n", __func__, base);

	pinmux_config[0].pin_function = PIN_FUNC_ETH;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		dev_err(&pdev->dev, "get pinmux config failed\n");

	printk("%s: get pinmux 0x%x\r\n", __func__, pinmux_config[0].config);

#if IS_ENABLED(CONFIG_TIMESTAMP)
	ptpclk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
	if (IS_ERR(ptpclk)) {
		dev_err(&pdev->dev, "can't find ptp_clock %s\n",
				v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
		ptpclk = NULL;
	} else {
		clk_prepare(ptpclk);
		clk_enable(ptpclk);
	}
	clk_put(ptpclk);
#endif

#if IS_ENABLED(CONFIG_MULTI_IRQ)

	reg = ioread32((void *)(base + 0x3004));
	iowrite32(reg | (1 << 0), (void *)(base + 0x3004));

	reg = ioread32((void *)(base + 0x3028));
	reg = reg & ~(3 << 0);
	iowrite32(reg | (3 << 2), (void *)(base + 0x3028));
#endif

	// impelment MAC1
	/*==== RMII =====*/
	printk("%s: get pinmux 0x%x\r\n", __func__, pinmux_config[0].config);
	if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii) ||
			pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RMII, &v_nvt_plat_info[ETH_QOS_INDEX]);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			if (spclk_en &&
				(!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;
				ret = nvt_pinmux_update(pinmux_config, 1);

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n", __func__, __LINE__);
				}

			}

			if (spclk_en) {
				struct clk *clk;

				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					if (!__clk_is_enabled(clk)) {
						clk_prepare(clk);
						clk_enable(clk);
					}
				}
				clk_put(clk);
			}

		}

#endif
		// assert phy reset
		nvt_eth_assert_phy_reset(pinmux_config[0].config, ETH_QOS_INDEX);


		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
#ifdef CONFIG_NVT_FPGA_EMULATION
		//reg |= 1<<31; // output at clock falling
#endif
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));

		// setup RMII REF_CLK direction
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		// pre-assume refclk: phy--> mac
		reg |= 1 << 0;
		reg &= ~(1 << 4);
#ifdef CONFIG_NVT_FPGA_EMULATION
		reg |= 1 << 2;
#endif
#ifdef CONFIG_OF
		// change RMII REF_CLK direction by DTB setting
		if (!of_property_read_u32(np, "ref-clk-out", &refclk_out)) {
			printk("%s: find ref-clk-out %d\r\n", __func__, refclk_out);
			if (refclk_out) {
				printk("%s: change REF_CLK out\r\n", __func__);
				reg &= ~(1 << 0);
				reg |= 1 << 4;
			}
		}
		iowrite32(reg | (1 << 5), (void *)(base + 0x3000 + 0x14));
#endif
		//iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== RGMII =====*/
	} else if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii) || pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RGMII 0x%x\r\n", __func__, pinmux_config[0].config);

#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);
			if (spclk_en &&
				(!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;
				ret = nvt_pinmux_update(pinmux_config, 1);

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n", __func__, __LINE__);
				}

			}

			if (spclk_en) {
				struct clk *clk;

				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					if (!__clk_is_enabled(clk)) {
						clk_prepare(clk);
						clk_enable(clk);
					}
				}
				clk_put(clk);
			}
		}

#endif
		// assert phy reset
		nvt_eth_assert_phy_reset(pinmux_config[0].config, ETH_QOS_INDEX);

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));
		//iowrite32(0x102, (void *)(base + 0x3000 + 0x2C)); //SRAM

		// setup TXD_SRC as RGMII
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		//reg |= (1<<0);
		reg |= (3 << 4);
		reg &= ~(3 << 30);
		reg |= 1 << 30;

#ifdef CONFIG_NVT_FPGA_EMULATION
		// coverity[identical_branches]
		if (ETH_QOS_INDEX == 0) {
			// TXCLK_DELAY = 12, RX_DLY = 7;
			reg |= (0 << 16) | (0 << 20);
		} else {
			// TXCLK_DELAY = 8, RX_DLY = 8;
			reg |= (0 << 16) | (0 << 20);
		}
		//reg |= (1<<29);
#endif

		iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== embd phy =====*/
	} else {
		unsigned long reg;

		printk("%s: pinmux detect emb phy 0x%x\r\n", __func__, pinmux_config[0].config);

		nvt_eth_phy_poweron(base);
		*embd_en = 1;

		// set phy mdio address
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg | (1 << 1), (void *)(base + 0x3800 + 0x00));
		reg = ioread32((void *)(base + 0x3800 + 0x1C));
		iowrite32(reg | (1 << 0), (void *)(base + 0x3800 + 0x1C));
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg & ~(1 << 1), (void *)(base + 0x3800 + 0x00));
	}

	return (int)ETH_QOS_INDEX;
}

#elif defined(CONFIG_NVT_IVOT_PLAT_NS02301)
/* ---------------------NS02301-------------------------- */
static const struct NVT_PLAT_INFO v_nvt_plat_info[2] = {
	// MAC0 (0xF02B_0000)
	{
		NVT_ETH_BASE_PHYS,
		"rmii0_refclk_i",
		"eth0_rxclk_i",
		"eth0_txclk_i",
		"eth0_extphy_clk",
		"eth0_ptp_clk",

		PIN_ETH_CFG_NONE,//PIN_ETH_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_ETH_RMII_1,//PIN_ETH_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_ETH_EXTPHYCLK,//PIN_ETH_CFG_REFCLK_PINMUX,

		P_GPIO(11),          // gpio reset
		0,          // gpio reset

		PAD_DS_PGPIO7,       // PHYCLK 25M PIN_1
		PAD_NONE,//NOT SUP,  // TXCLK PIN_1
		PAD_DS_PGPIO2,       // TXCTL(TX_EN) PIN_1
		PAD_DS_PGPIO8,       // REFCLK 50M (RMII) PIN_1
		PAD_DS_PGPIO0,       // TXD[0] PIN_1
		PAD_DS_PGPIO1,       // TXD[1] PIN_1
		PAD_NONE,//NOT SUP,  // TXD[2] PIN_1
		PAD_NONE,//NOT SUP,  // TXD[3] PIN_1

		PAD_NONE,//NOT SUP,  // PHYCLK 25M PIN_2
		PAD_NONE,//NOT SUP,  // TXCLK PIN_2
		PAD_NONE,//NOT SUP,  // TXCTL(TX_EN) PIN_2
		PAD_NONE,//NOT SUP,  // REFCLK 50M (RMII) PIN_2
		PAD_NONE,//NOT SUP,  // TXD[0] PIN_2
		PAD_NONE,//NOT SUP,  // TXD[1] PIN_2
		PAD_NONE,//NOT SUP,  // TXD[2] PIN_2
		PAD_NONE,//NOT SUP,  // TXD[3] PIN_2
	},
	// MAC1 NOT SUP
	{
		0,
		"rmii1_refclk_i",
		"eth1_rxclk_i",
		"eth1_txclk_i",
		"eth1_extphy_clk",
		"eth1_ptp_clk",

		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_REFCLK_PINMUX,

		0,//L_GPIO(18),            // gpio reset
		0,//NOT SUP,           // gpio reset

		PAD_NONE,//NOT SUP,  // PHYCLK
		PAD_NONE,//NOT SUP,  // TXCLK
		PAD_NONE,//NOT SUP,  // TXCTL
		PAD_NONE,//NOT SUP,  // REFCLK (RMII)
		PAD_NONE,//NOT SUP,  // TXD[0]
		PAD_NONE,//NOT SUP,  // TXD[1]
		PAD_NONE,//NOT SUP,  // TXD[2]
		PAD_NONE,//NOT SUP,  // TXD[3]

		PAD_NONE,//NOT SUP,  // PHYCLK PIN_2
		PAD_NONE,//NOT SUP,  // TXCLK PIN_2
		PAD_NONE,//NOT SUP,  // TXCTL PIN_2
		PAD_NONE,//NOT SUP,  // REFCLK (RMII) PIN_2
		PAD_NONE,//NOT SUP,  // TXD[0] PIN_2
		PAD_NONE,//NOT SUP,  // TXD[1] PIN_2
		PAD_NONE,//NOT SUP,  // TXD[2] PIN_2
		PAD_NONE,//NOT SUP,  // TXD[3] PIN_2
	},
};


static void nvt_eth_phy_poweron(void __iomem *base_addr)
{
	unsigned long reg;

	reg = ioread32((void *)(base_addr + 0x3800 + 0xF8));
	iowrite32(reg | (1 << 7), (void *)(base_addr + 0x3800 + 0xF8));
	udelay(20);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(200);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 1)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(250);
	reg = ioread32((void *)(base_addr + 0x3800 + 0x2E8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x2E8));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xCC));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xCC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xDC));
	iowrite32(reg | (1 << 0), (void *)(base_addr + 0x3800 + 0xDC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0x9C));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x9C));
}

static int set_pad_driving(UINT interface, const struct NVT_PLAT_INFO *p_info)
{

	/* PIN_ETH_CFG_ETH_RMII_1 */
	if ((interface & p_info->pinmux_rmii)) {
		pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_rmii_refclk, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_1);
	/* PIN_ETH_CFG_ETH_RMII_2 */
	} else if (interface & p_info->pinmux_rmii_2) {
		pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_rmii_refclk_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d1_2, PAD_DRIVINGSINK_LEVEL_1);
	/* PIN_ETH_CFG_ETH_RGMII */
	} else if (interface & p_info->pinmux_rgmii) {
		pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txclk, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d3, PAD_DRIVINGSINK_LEVEL_1);
	/* PIN_ETH_CFG_ETH_RGMII_2 */
	} else if (interface & p_info->pinmux_rgmii_2) {
		pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txclk_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d1_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d2_2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(p_info->pad_tx_d3_2, PAD_DRIVINGSINK_LEVEL_1);
	}


	return 0;
}

static void nvt_eth_assert_phy_reset(UINT pinmux_config, UINT ETH_QOS_INDEX)
{
	UINT interface, gpio_reset;

	/* PIN_ETH_CFG_ETH_RMII_1 */
	if (pinmux_config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii) {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset;
	/* PIN_ETH_CFG_ETH_RMII_2 */
	} else if (pinmux_config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2) {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_2_reset;
	/* PIN_ETH_CFG_ETH_RGMII */
	} else if (pinmux_config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii) {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset;
	/* PIN_ETH_CFG_ETH_RGMII_2 */
	} else if (pinmux_config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2) {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_2_reset;
	} else {
		interface = v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii;
		gpio_reset = v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset;
	}

	printk("%s, %d, interface = 0x%x, gpio_reset = 0x%x\n", __func__, __LINE__, interface, gpio_reset);
	set_pad_driving(interface, &v_nvt_plat_info[ETH_QOS_INDEX]);
	gpio_direction_output(gpio_reset, 0);
	gpio_set_value(gpio_reset, 0);
	mdelay(20);
	gpio_set_value(gpio_reset, 1);
	mdelay(50);
}

int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base)
{
	int ret = 0;
	struct resource *res;
	void __iomem *base;
	PIN_GROUP_CONFIG pinmux_config[1];
	UINT ETH_QOS_INDEX; // index to MAC (0: MAC0, 1: MAC1);
#if IS_ENABLED(CONFIG_TIMESTAMP)
	struct clk *ptpclk;
#endif
#if IS_ENABLED(CONFIG_MULTI_IRQ)
	unsigned long reg;
#endif
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;
	int spclk_en = 0;
	int refclk_out = 0;
#endif
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No memory resources\n");
		return -1;
	}
	printk("%s: IO MEM res start 0x%x\r\n", __func__, res->start);

	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to find IORESOURCE_MEM\n");
		return -1;
	} else {
		*p_base = base;
	}
	if (res->start == v_nvt_plat_info[0].base_pa) {
		ETH_QOS_INDEX = 0;
	} else {
		ETH_QOS_INDEX = 1;
	}
	printk("%s: get IO MEM 0x%px\r\n", __func__, base);

	pinmux_config[0].pin_function = PIN_FUNC_ETH;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		dev_err(&pdev->dev, "get pinmux config failed\n");

	printk("%s: get pinmux 0x%x\r\n", __func__, pinmux_config[0].config);

#if IS_ENABLED(CONFIG_TIMESTAMP)
	ptpclk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
	if (IS_ERR(ptpclk)) {
		dev_err(&pdev->dev, "can't find ptp_clock %s\n",
				v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
		ptpclk = NULL;
	} else {
		clk_prepare(ptpclk);
		clk_enable(ptpclk);
	}
	clk_put(ptpclk);
#endif

#if IS_ENABLED(CONFIG_MULTI_IRQ)

	reg = ioread32((void *)(base + 0x3004));
	iowrite32(reg | (1 << 0), (void *)(base + 0x3004));

	reg = ioread32((void *)(base + 0x3028));
	reg = reg & ~(3 << 0);
	iowrite32(reg | (3 << 2), (void *)(base + 0x3028));
#endif

	/*==== RMII =====*/
	if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii) ||
			pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RMII, &v_nvt_plat_info[ETH_QOS_INDEX]);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			if (spclk_en &&
				(!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;
				ret = nvt_pinmux_update(pinmux_config, 1);

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n", __func__, __LINE__);
				}

			}

			if (spclk_en) {
				struct clk *clk;

				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					if (!__clk_is_enabled(clk)) {
						clk_prepare(clk);
						clk_enable(clk);
					}
				}
				clk_put(clk);
			}

		}

#endif
		// assert phy reset
		nvt_eth_assert_phy_reset(pinmux_config[0].config, ETH_QOS_INDEX);

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
#ifdef CONFIG_NVT_FPGA_EMULATION
		//reg |= 1<<31; // output at clock falling
#endif
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));

		// setup RMII REF_CLK direction
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		// pre-assume refclk: phy--> mac
		reg |= 1 << 0;
		reg &= ~(1 << 4);
#ifdef CONFIG_NVT_FPGA_EMULATION
		reg |= 1 << 2;
#endif
#ifdef CONFIG_OF
		// change RMII REF_CLK direction by DTB setting
		if (!of_property_read_u32(np, "ref-clk-out", &refclk_out)) {
			printk("%s: find ref-clk-out %d\r\n", __func__, refclk_out);
			if (refclk_out) {
				printk("%s: change REF_CLK out\r\n", __func__);
				reg &= ~(1 << 0);
				reg |= 1 << 4;
			}
		}
		iowrite32(reg | (1 << 5), (void *)(base + 0x3000 + 0x14));
#endif
		//iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== RGMII =====*/
	} else if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii) || pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RGMII 0x%x\r\n", __func__, pinmux_config[0].config);

#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);
			if (spclk_en &&
				(!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;
				ret = nvt_pinmux_update(pinmux_config, 1);

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n", __func__, __LINE__);
				}

			}

			if (spclk_en) {
				struct clk *clk;

				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					if (!__clk_is_enabled(clk)) {
						clk_prepare(clk);
						clk_enable(clk);
					}
				}
				clk_put(clk);
			}
		}

#endif
		// assert phy reset
		nvt_eth_assert_phy_reset(pinmux_config[0].config, ETH_QOS_INDEX);

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));
		//iowrite32(0x102, (void *)(base + 0x3000 + 0x2C)); //SRAM

		// setup TXD_SRC as RGMII
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		//reg |= (1<<0);
		reg |= (3 << 4);
		reg &= ~(3 << 30);
		reg |= 1 << 30;

#ifdef CONFIG_NVT_FPGA_EMULATION
		// coverity[identical_branches]
		if (ETH_QOS_INDEX == 0) {
			// TXCLK_DELAY = 12, RX_DLY = 7;
			reg |= (0 << 16) | (0 << 20);
		} else {
			// TXCLK_DELAY = 8, RX_DLY = 8;
			reg |= (0 << 16) | (0 << 20);
		}
		//reg |= (1<<29);
#endif

		iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== embd phy =====*/
	} else {
		unsigned long reg;

		printk("%s: pinmux detect emb phy 0x%x\r\n", __func__, pinmux_config[0].config);

		nvt_eth_phy_poweron(base);

		// set phy mdio address
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg | (1 << 1), (void *)(base + 0x3800 + 0x00));
		reg = ioread32((void *)(base + 0x3800 + 0x1C));
		iowrite32(reg | (1 << 0), (void *)(base + 0x3800 + 0x1C));
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg & ~(1 << 1), (void *)(base + 0x3800 + 0x00));
	}

	return (int)ETH_QOS_INDEX;
}
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51090)
/* ---------------------NA51090-------------------------- */
static const struct NVT_PLAT_INFO v_nvt_plat_info[2] = {
	// MAC0 (0xFCC0_0000)
	{
		NVT_ETH_BASE_PHYS,
		"rmii0_refclk_i",
		"eth0_rxclk_i",
		"eth0_txclk_i",
		"eth0_extphy_clk",
		"eth0_ptp_clk",

		PIN_ETH_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_REFCLK_PINMUX,

		E_GPIO(1),			// gpio reset
		S_GPIO(1),			// gpio reset

		PAD_DS_EGPIO0,	// PHYCLK PIN_1
		PAD_DS_EGPIO8,	// TXCLK PIN_1
		PAD_DS_EGPIO9,	// TXCTL PIN_1
		PAD_DS_EGPIO2,	// REFCLK (RMII) PIN_1
		PAD_DS_EGPIO10,	// TXD[0] PIN_1
		PAD_DS_EGPIO11,	// TXD[1] PIN_1
		PAD_DS_EGPIO12,	// TXD[2] PIN_1
		PAD_DS_EGPIO13,	// TXD[3] PIN_1

		PAD_DS_SGPIO0,	// PHYCLK PIN_2
		PAD_DS_SGPIO8,	// TXCLK PIN_2
		PAD_DS_SGPIO9,	// TXCTL PIN_2
		PAD_DS_SGPIO2,	// REFCLK (RMII) PIN_2
		PAD_DS_SGPIO10,	// TXD[0] PIN_2
		PAD_DS_SGPIO11,	// TXD[1] PIN_2
		PAD_DS_SGPIO12,	// TXD[2] PIN_2
		PAD_DS_SGPIO13,	// TXD[3] PIN_2
	},
	// MAC1 (0xFCD0_0000)
	{
		NVT_ETH2_BASE_PHYS,
		"rmii1_refclk_i",
		"eth1_rxclk_i",
		"eth1_txclk_i",
		"eth1_extphy_clk",
		"eth1_ptp_clk",

		PIN_ETH2_CFG_RGMII_1ST_PINMUX,
		PIN_ETH2_CFG_RGMII_2ND_PINMUX,

		PIN_ETH2_CFG_RMII_1ST_PINMUX,
		PIN_ETH2_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH2_CFG_REFCLK_PINMUX,

		E_GPIO(17),			// gpio reset
		S_GPIO(19),			// gpio reset

		PAD_DS_EGPIO16,	// PHYCLK
		PAD_DS_EGPIO24,	// TXCLK
		PAD_DS_EGPIO25,	// TXCTL
		PAD_DS_EGPIO18,	// REFCLK (RMII)
		PAD_DS_EGPIO26,	// TXD[0]
		PAD_DS_EGPIO27,	// TXD[1]
		PAD_DS_EGPIO28,	// TXD[2]
		PAD_DS_EGPIO29,	// TXD[3]

		PAD_DS_SGPIO18,	// PHYCLK PIN_2
		PAD_DS_SGPIO26,	// TXCLK PIN_2
		PAD_DS_SGPIO27,	// TXCTL PIN_2
		PAD_DS_SGPIO20,	// REFCLK (RMII) PIN_2
		PAD_DS_SGPIO28,	// TXD[0] PIN_2
		PAD_DS_SGPIO29,	// TXD[1] PIN_2
		PAD_DS_SGPIO30,	// TXD[2] PIN_2
		PAD_DS_SGPIO31,	// TXD[3] PIN_2
	},
};

static void nvt_eth_phy_poweron(void __iomem *base_addr)
{
	unsigned long reg;

	reg = ioread32((void *)(base_addr + 0x3800 + 0xF8));
	iowrite32(reg | (1 << 7), (void *)(base_addr + 0x3800 + 0xF8));
	udelay(20);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(200);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 1)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(250);
	reg = ioread32((void *)(base_addr + 0x3800 + 0x2E8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x2E8));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xCC));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xCC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xDC));
	iowrite32(reg | (1 << 0), (void *)(base_addr + 0x3800 + 0xDC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0x9C));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x9C));
}

static int set_pad_driving(UINT interface, const struct NVT_PLAT_INFO *p_info, int chip_id)
{
	if((interface == PIN_ETH_CFG_RMII_1ST_PINMUX) ||
			(interface == PIN_ETH2_CFG_RMII_1ST_PINMUX))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			pad_set_drivingsink_ep(p_info->pad_phy_clk,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_rmii_refclk,
					PAD_DRIVINGSINK_LEVEL_0, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txctl,
					PAD_DRIVINGSINK_LEVEL_0, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d0,
					PAD_DRIVINGSINK_LEVEL_0, chip_id - CHIP_EP0);

#endif
		} else {
			pad_set_drivingsink(p_info->pad_phy_clk,
					PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_rmii_refclk,
					PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(p_info->pad_txctl,
					PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(p_info->pad_tx_d0,
					PAD_DRIVINGSINK_LEVEL_0);
		}
	} else if ((interface == PIN_ETH_CFG_RGMII_1ST_PINMUX) ||
			(interface == PIN_ETH2_CFG_RGMII_1ST_PINMUX))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			pad_set_drivingsink_ep(p_info->pad_phy_clk,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txclk,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txctl,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d0,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d1,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d2,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d3,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);

#endif
		} else {
			pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_txclk, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_tx_d2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_tx_d3, PAD_DRIVINGSINK_LEVEL_1);
		}
	} else if ((interface == PIN_ETH_CFG_RMII_2ND_PINMUX) ||
			(interface == PIN_ETH2_CFG_RMII_2ND_PINMUX))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			pad_set_drivingsink_ep(p_info->pad_phy_clk_2,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_rmii_refclk_2,
					PAD_DRIVINGSINK_LEVEL_0, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txctl_2,
					PAD_DRIVINGSINK_LEVEL_0, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d0_2,
					PAD_DRIVINGSINK_LEVEL_0, chip_id - CHIP_EP0);

#endif
		} else {
			pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_rmii_refclk_2,
					PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_0);
		}
	} else if ((interface == PIN_ETH_CFG_RGMII_2ND_PINMUX) ||
			(interface == PIN_ETH2_CFG_RGMII_2ND_PINMUX))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			pad_set_drivingsink_ep(p_info->pad_phy_clk_2,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txclk_2,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_txctl_2,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d0_2,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d1_2,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d2_2,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);
			pad_set_drivingsink_ep(p_info->pad_tx_d3_2,
					PAD_DRIVINGSINK_LEVEL_1, chip_id - CHIP_EP0);

#endif
		} else {
			pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_txclk_2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_tx_d1_2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_tx_d2_2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(p_info->pad_tx_d3_2, PAD_DRIVINGSINK_LEVEL_1);
		}
	}

	return 0;
}

static int get_pad_driving(UINT interface, const struct NVT_PLAT_INFO *p_info, int chip_id)
{
	PAD_DRIVINGSINK driving[7] = {0};

	if((interface == PIN_ETH_CFG_RMII_1ST_PINMUX) ||
			(interface == PIN_ETH2_CFG_RMII_1ST_PINMUX))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			pad_get_drivingsink_ep(p_info->pad_phy_clk,
					&driving[0], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_rmii_refclk,
					&driving[1], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txctl,
					&driving[2], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d0,
					&driving[3], chip_id - CHIP_EP0);
			printk("ETH RMII PIN, driving level: %d%d%d%d\n", driving[0], driving[1], driving[2], driving[3]);

#endif
		} else {
			pad_get_drivingsink(p_info->pad_phy_clk, &driving[0]);
			pad_get_drivingsink(p_info->pad_rmii_refclk, &driving[1]);
			pad_get_drivingsink(p_info->pad_txctl, &driving[2]);
			pad_get_drivingsink(p_info->pad_tx_d0, &driving[3]);
			printk("ETH RMII PIN, driving level: %d%d%d%d\n", driving[0], driving[1], driving[2], driving[3]);
		}
	} else if ((interface == PIN_ETH_CFG_RGMII_1ST_PINMUX) ||
			(interface == PIN_ETH2_CFG_RGMII_1ST_PINMUX))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)

			pad_get_drivingsink_ep(p_info->pad_phy_clk,
					&driving[0], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txclk,
					&driving[1], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txctl,
					&driving[2], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d0,
					&driving[3], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d1,
					&driving[4], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d2,
					&driving[5], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d3,
					&driving[6], chip_id - CHIP_EP0);
			printk("ETH RGMII PIN, driving level: %d%d%d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4], driving[5], driving[6]);

#endif
		} else {
			pad_get_drivingsink(p_info->pad_phy_clk, &driving[0]);
			pad_get_drivingsink(p_info->pad_txclk, &driving[1]);
			pad_get_drivingsink(p_info->pad_txctl, &driving[2]);
			pad_get_drivingsink(p_info->pad_tx_d0, &driving[3]);
			pad_get_drivingsink(p_info->pad_tx_d1, &driving[4]);
			pad_get_drivingsink(p_info->pad_tx_d2, &driving[5]);
			pad_get_drivingsink(p_info->pad_tx_d3, &driving[6]);
			printk("ETH RGMII PIN, driving level: %d%d%d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4], driving[5], driving[6]);
		}
	} else if ((interface == PIN_ETH_CFG_RMII_2ND_PINMUX) ||
			(interface == PIN_ETH2_CFG_RMII_2ND_PINMUX))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			pad_get_drivingsink_ep(p_info->pad_phy_clk_2,
					&driving[0], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_rmii_refclk_2,
					&driving[1], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txctl_2,
					&driving[2], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d0_2,
					&driving[3], chip_id - CHIP_EP0);
			printk("ETH RMII PIN, driving level: %d%d%d%d\n", driving[0], driving[1], driving[2], driving[3]);

#endif
		} else {
			pad_get_drivingsink(p_info->pad_phy_clk_2, &driving[0]);
			pad_get_drivingsink(p_info->pad_rmii_refclk_2, &driving[1]);
			pad_get_drivingsink(p_info->pad_txctl_2, &driving[2]);
			pad_get_drivingsink(p_info->pad_tx_d0_2, &driving[3]);
			printk("ETH RMII PIN, driving level: %d%d%d%d\n", driving[0], driving[1], driving[2], driving[3]);
		}
	} else if ((interface == PIN_ETH_CFG_RGMII_2ND_PINMUX) ||
			(interface == PIN_ETH2_CFG_RGMII_2ND_PINMUX))
	{
		if (chip_id > 0) {
#if IS_ENABLED(CONFIG_PCI)
			pad_get_drivingsink_ep(p_info->pad_phy_clk_2,
					&driving[0], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txclk_2,
					&driving[1], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_txctl_2,
					&driving[2], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d0_2,
					&driving[3], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d1_2,
					&driving[4], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d2_2,
					&driving[5], chip_id - CHIP_EP0);
			pad_get_drivingsink_ep(p_info->pad_tx_d3_2,
					&driving[6], chip_id - CHIP_EP0);
			printk("ETH RGMII PIN, driving level: %d%d%d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4], driving[5], driving[6]);

#endif
		} else {
			pad_get_drivingsink(p_info->pad_phy_clk_2, &driving[0]);
			pad_get_drivingsink(p_info->pad_txclk_2, &driving[1]);
			pad_get_drivingsink(p_info->pad_txctl_2, &driving[2]);
			pad_get_drivingsink(p_info->pad_tx_d0_2, &driving[3]);
			pad_get_drivingsink(p_info->pad_tx_d1_2, &driving[4]);
			pad_get_drivingsink(p_info->pad_tx_d2_2, &driving[5]);
			pad_get_drivingsink(p_info->pad_tx_d3_2, &driving[6]);
			printk("ETH RGMII PIN, driving level: %d%d%d%d%d%d%d\n", driving[0], driving[1], driving[2], driving[3], driving[4], driving[5], driving[6]);
		}
	}

	return 0;
}

int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base)
{
	int ret = 0;
	struct resource *res;
	void __iomem *base;
	PIN_GROUP_CONFIG pinmux_config[1];
	UINT ETH_QOS_INDEX; // index to MAC (0: MAC0, 1: MAC1);
#if IS_ENABLED(CONFIG_TIMESTAMP)
	struct clk *ptpclk;
#endif
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;
	int spclk_en = 0;
	int refclk_out = 0;
#endif
#if IS_ENABLED(CONFIG_MULTI_IRQ)
	unsigned long reg;
#endif
	UINT reset_gpio, reset_gpio_2;
	int chip_id = 0;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No memory resources\n");
		return -1;
	}
	printk("%s: IO MEM res start 0x%llx\r\n", __func__, res->start);

	if (res->start == v_nvt_plat_info[0].base_pa) {
		ETH_QOS_INDEX = 0;
	} else {
		ETH_QOS_INDEX = 1;
	}
	printk("%s: ETH_QOS_INDEX %d\n", __func__, ETH_QOS_INDEX);
#if IS_ENABLED(CONFIG_PCI)
	chip_id = nvt_eth_dts_chipid(np);
	printk("%s: chip_id = %d\n", __func__, chip_id);

	if (nvtpcie_is_common_pci() || nvt_eth_dts_chipid(np) != CHIP_RC) {
		if (0 != nvtpcie_conv_resource(nvt_eth_dts_chipid(np), res)) {
			printk("conv res failed, %pR\n", res);
		}
		printk("%s: IO MEM res start 0x%llx\r\n", __func__, res->start);
	}

	if (nvtpcie_is_common_pci()) {
		base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	} else {
		base = devm_ioremap_resource(&pdev->dev, res);
	}
#else
	base = devm_ioremap_resource(&pdev->dev, res);
#endif

	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to find IORESOURCE_MEM\n");
		return -1;
//		ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	} else {
		*p_base = base;
//		ETH_QOS_REG_BASE = base;
	}
//	ETH_QOS_INDEX = 1;   // temp force to MAC1 (for embedded phy)
	printk("%s: get IO MEM 0x%px\r\n", __func__, base);

	pinmux_config[0].pin_function = PIN_FUNC_ETH;
#if IS_ENABLED(CONFIG_PCI)
	if (nvt_eth_dts_chipid(np) > CHIP_RC) {
//		ret = nvt_eth_ep_pinmux_capture(pinmux_config, np);
		ret = nvt_pinmux_capture_ep(pinmux_config, 1, nvt_eth_dts_chipid(np) - CHIP_EP0);
	} else {
		ret = nvt_pinmux_capture(pinmux_config, 1);
	}
#else
	ret = nvt_pinmux_capture(pinmux_config, 1);
#endif
	if (ret)
		dev_err(&pdev->dev, "get pinmux config failed\n");

	printk("%s: get pinmux 0x%x\r\n", __func__, pinmux_config[0].config);

#ifdef CONFIG_OF
	if (of_property_read_u32(np, "reset_gpio", &reset_gpio))
#endif
		reset_gpio = v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset;

#ifdef CONFIG_OF
	if (of_property_read_u32(np, "reset_gpio_2", &reset_gpio_2))
#endif
		reset_gpio_2 = v_nvt_plat_info[ETH_QOS_INDEX].gpio_2_reset;

#if IS_ENABLED(CONFIG_TIMESTAMP)
	if (chip_id == 0) {
		ptpclk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
		if (IS_ERR(ptpclk)) {
			dev_err(&pdev->dev, "can't find ptp_clock %s\n",
				v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
			ptpclk = NULL;
		} else {
			clk_prepare(ptpclk);
			clk_enable(ptpclk);
		}
		clk_put(ptpclk);
	}
#endif

#if IS_ENABLED(CONFIG_MULTI_IRQ)

	reg = ioread32((void *)(base + 0x3004));
	iowrite32(reg | (1 << 0), (void *)(base + 0x3004));

	reg = ioread32((void *)(base + 0x3028));
	reg = reg & ~(3 << 0);
	iowrite32(reg | (3 << 2), (void *)(base + 0x3028));
#endif

	// impelment MAC1
	/*==== RMII =====*/
	if (pinmux_config[0].config &
			(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii |
			 v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RMII, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			if (spclk_en &&
				(!(pinmux_config[0].config &
				   v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {
#if 1

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;

#if IS_ENABLED(CONFIG_PCI)
				if (nvt_eth_dts_chipid(np) > 0) {
					ret = nvt_pinmux_update_ep(pinmux_config, 1,
						nvt_eth_dts_chipid(np) - CHIP_EP0);
				} else {
					ret = nvt_pinmux_update(pinmux_config, 1);
				}
#else
				ret = nvt_pinmux_update(pinmux_config, 1);
#endif

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n", __func__, __LINE__);
				}

				printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);

			}

			if (spclk_en && (chip_id == 0)) {
				struct clk *clk;
				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					if (!__clk_is_enabled(clk)) {
						clk_prepare(clk);
						clk_enable(clk);
					}
				}
				clk_put(clk);
			}
#else
				if (spclk_en &&
					(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
					struct clk *clk;

					printk("%s: change extphyclk pinmux\r\n", __func__);
					pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

#if IS_ENABLED(CONFIG_PCI)
					if (nvt_eth_dts_chipid(np) > 0) {
						ret = nvt_pinmux_update_ep(pinmux_config, 1,
							nvt_eth_dts_chipid(np) - CHIP_EP0);
					} else {
						ret = nvt_pinmux_update(pinmux_config, 1);
					}
#else
					ret = nvt_pinmux_update(pinmux_config, 1);
#endif
					if (ret)
						pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
								__func__, __LINE__);

					clk = clk_get(&pdev->dev, "ext_phy_clk");
					if (IS_ERR(clk)) {
						dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
						clk = NULL;
					} else {
						clk_prepare(clk);
						clk_enable(clk);
					}
					clk_put(clk);
				}
#endif
		}

		// assert phy reset
		if(pinmux_config[0].config &
				v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii){
			set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			get_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
//			set_pad_pull(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			gpio_direction_output(reset_gpio, 0);
			gpio_set_value(reset_gpio, 0);
			mdelay(20);
			gpio_set_value(reset_gpio, 1);
			mdelay(50);
		} else {
			set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			get_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
//			set_pad_pull(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			gpio_direction_output(reset_gpio_2, 0);
			gpio_set_value(reset_gpio_2, 0);
			mdelay(20);
			gpio_set_value(reset_gpio_2, 1);
			mdelay(50);
		}
#endif

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
#ifdef CONFIG_NVT_FPGA_EMULATION
		//reg |= 1<<31; // output at clock falling
#endif
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));

		// setup RMII REF_CLK direction
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		// pre-assume refclk: phy--> mac
		reg |= 1 << 0;
		reg &= ~(1 << 4);
#ifdef CONFIG_NVT_FPGA_EMULATION
		reg |= 1 << 2;
#endif
#ifdef CONFIG_OF
		// change RMII REF_CLK direction by DTB setting
		if (!of_property_read_u32(np, "ref-clk-out", &refclk_out)) {
			printk("%s: find ref-clk-out %d\r\n", __func__, refclk_out);
			if (refclk_out) {
				printk("%s: change REF_CLK out\r\n", __func__);
				reg &= ~(1 << 0);
				reg |= 1 << 4;
			}
		}
		iowrite32(reg | (1 << 5), (void *)(base + 0x3000 + 0x14));
#endif
		//iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== RGMII =====*/
	} else if (pinmux_config[0].config &
			(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii |
			 v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2)) {
		unsigned long reg;

		printk("%s: pinmux detect RGMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RGMII, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);

#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
#if 1
			printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);
			if (spclk_en &&
				(!(pinmux_config[0].config &
				   v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;

#if IS_ENABLED(CONFIG_PCI)
				if (nvt_eth_dts_chipid(np) > 0) {
					ret = nvt_pinmux_update_ep(pinmux_config, 1,
						nvt_eth_dts_chipid(np) - CHIP_EP0);
				} else {
					ret = nvt_pinmux_update(pinmux_config, 1);
				}
#else
				ret = nvt_pinmux_update(pinmux_config, 1);
#endif

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
							__func__, __LINE__);
				}
			}

			if (spclk_en && (chip_id == 0)) {
				struct clk *clk;
				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					if (!__clk_is_enabled(clk)) {
						clk_prepare(clk);
						clk_enable(clk);
					}
				}
				clk_put(clk);
			}
#else
			if (spclk_en &&
				(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

#if IS_ENABLED(CONFIG_PCI)
				if (nvt_eth_dts_chipid(np) > 0) {
					ret = nvt_pinmux_update_ep(pinmux_config, 1,
						nvt_eth_dts_chipid(np) - CHIP_EP0);
				} else {
					ret = nvt_pinmux_update(pinmux_config, 1);
				}
#else
				ret = nvt_pinmux_update(pinmux_config, 1);
#endif

				if (ret)
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
							__func__, __LINE__);

				clk = clk_get(&pdev->dev, "ext_phy_clk");
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
#endif
		}

		// assert phy reset
		if(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii){
			set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			get_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
//			set_pad_pull(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			gpio_direction_output(reset_gpio, 0);
			gpio_set_value(reset_gpio, 0);
			mdelay(20);
			gpio_set_value(reset_gpio, 1);
			mdelay(50);
		} else {
			set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			get_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
//			set_pad_pull(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2, &v_nvt_plat_info[ETH_QOS_INDEX], chip_id);
			gpio_direction_output(reset_gpio_2, 0);
			gpio_set_value(reset_gpio_2, 0);
			mdelay(20);
			gpio_set_value(reset_gpio_2, 1);
			mdelay(50);
		}

#endif

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));
		//iowrite32(0x102, (void *)(base + 0x3000 + 0x2C)); //SRAM

		// setup TXD_SRC as RGMII
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		//reg |= (1<<0);
		reg |= (3 << 4);
		reg &= ~(3 << 30);
		reg |= 1 << 30;

#ifdef CONFIG_NVT_FPGA_EMULATION
		// coverity[identical_branches]
		if (ETH_QOS_INDEX == 0) {
			// TXCLK_DELAY = 12, RX_DLY = 7;
			reg |= (0 << 16) | (0 << 20);
		} else {
			// TXCLK_DELAY = 8, RX_DLY = 8;
			reg |= (0 << 16) | (0 << 20);
		}
		//reg |= (1<<29);
#endif

		iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== embd phy =====*/
	} else {
		unsigned long reg;

		printk("%s: pinmux detect emb phy 0x%x\r\n", __func__, pinmux_config[0].config);

		nvt_eth_phy_poweron(base);

		// set phy mdio address
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg | (1 << 1), (void *)(base + 0x3800 + 0x00));
		reg = ioread32((void *)(base + 0x3800 + 0x1C));
		iowrite32(reg | (1 << 0), (void *)(base + 0x3800 + 0x1C));
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg & ~(1 << 1), (void *)(base + 0x3800 + 0x00));
	}

	return (int)ETH_QOS_INDEX;
}
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51103)
/* ---------------------NA51103-------------------------- */
static const struct NVT_PLAT_INFO v_nvt_plat_info[2] = {
	// MAC0 (0xFCC0_0000)
	{
		NVT_ETH_BASE_PHYS,
		"rmii0_refclk_i",
		"eth0_rxclk_i",
		"eth0_txclk_i",
		"eth0_extphy_clk",
		"eth0_ptp_clk",

		PIN_ETH_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_REFCLK_PINMUX,

		E_GPIO(1),			// gpio reset
		S_GPIO(1),			// gpio reset

		PAD_DS_EGPIO0,	// PHYCLK PIN_1
		PAD_DS_EGPIO8,	// TXCLK PIN_1
		PAD_DS_EGPIO9,	// TXCTL PIN_1
		PAD_DS_EGPIO2,	// REFCLK (RMII) PIN_1
		PAD_DS_EGPIO10,	// TXD[0] PIN_1
		PAD_DS_EGPIO11,	// TXD[1] PIN_1
		PAD_DS_EGPIO12,	// TXD[2] PIN_1
		PAD_DS_EGPIO13,	// TXD[3] PIN_1

		PAD_DS_SGPIO0,	// PHYCLK PIN_2
		PAD_DS_SGPIO8,	// TXCLK PIN_2
		PAD_DS_SGPIO9,	// TXCTL PIN_2
		PAD_DS_SGPIO2,	// REFCLK (RMII) PIN_2
		PAD_DS_SGPIO10,	// TXD[0] PIN_2
		PAD_DS_SGPIO11,	// TXD[1] PIN_2
		PAD_DS_SGPIO12,	// TXD[2] PIN_2
		PAD_DS_SGPIO13,	// TXD[3] PIN_2
	},
	// MAC1 (0xFCD0_0000)
	{
		NVT_ETH2_BASE_PHYS,
		"rmii1_refclk_i",
		"eth1_rxclk_i",
		"eth1_txclk_i",
		"eth1_extphy_clk",
		"eth1_ptp_clk",

		PIN_ETH2_CFG_RGMII_1ST_PINMUX,
		PIN_ETH2_CFG_RGMII_2ND_PINMUX,

		PIN_ETH2_CFG_RMII_1ST_PINMUX,
		PIN_ETH2_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH2_CFG_REFCLK_PINMUX,

		S_GPIO(1),			// ETH gpio reset
		S_GPIO(1),			// ETH2 gpio reset

		PAD_DS_SGPIO0,	// PHYCLK
		PAD_DS_SGPIO8,	// TXCLK
		PAD_DS_SGPIO9,	// TXCTL
		PAD_DS_SGPIO2,	// REFCLK (RMII)
		PAD_DS_SGPIO10,	// TXD[0]
		PAD_DS_SGPIO11,	// TXD[1]
		PAD_DS_SGPIO12,	// TXD[2]
		PAD_DS_SGPIO13,	// TXD[3]

		PAD_DS_SGPIO0,	// PHYCLK PIN_2
		PAD_DS_SGPIO8,	// TXCLK PIN_2
		PAD_DS_SGPIO9,	// TXCTL PIN_2
		PAD_DS_SGPIO2,	// REFCLK (RMII) PIN_2
		PAD_DS_SGPIO10,	// TXD[0] PIN_2
		PAD_DS_SGPIO11,	// TXD[1] PIN_2
		PAD_DS_SGPIO12,	// TXD[2] PIN_2
		PAD_DS_SGPIO13,	// TXD[3] PIN_2
	},
};


static void nvt_eth_phy_poweron(void __iomem *base_addr)
{
	unsigned long reg;

	reg = ioread32((void*)(base_addr + 0x3800 + 0xF8));
	iowrite32(reg | (1<<7), (void *)(base_addr + 0x3800 + 0xF8));
	udelay(20);
	reg = ioread32((void*)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1<<0)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(200);
	reg = ioread32((void*)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1<<1)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(250);
	reg = ioread32((void*)(base_addr + 0x3800 + 0x2E8));
	iowrite32(reg & (~(1<<0)), (void *)(base_addr + 0x3800 + 0x2E8));
	reg = ioread32((void*)(base_addr + 0x3800 + 0xCC));
	iowrite32(reg & (~(1<<0)), (void *)(base_addr + 0x3800 + 0xCC));
	reg = ioread32((void*)(base_addr + 0x3800 + 0xDC));
	iowrite32(reg | (1<<0), (void *)(base_addr + 0x3800 + 0xDC));
	reg = ioread32((void*)(base_addr + 0x3800 + 0x9C));
	iowrite32(reg & (~(1<<0)), (void *)(base_addr + 0x3800 + 0x9C));
}

static int set_pad_driving(UINT interface, const struct NVT_PLAT_INFO *p_info)
{

    if((interface == PIN_ETH_CFG_RMII_1ST_PINMUX) || (interface == PIN_ETH2_CFG_RMII_1ST_PINMUX))
    {
        pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_rmii_refclk, PAD_DRIVINGSINK_LEVEL_0);
        pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_0);
        pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_0);
    }else if((interface == PIN_ETH_CFG_RGMII_1ST_PINMUX) || (interface == PIN_ETH2_CFG_RGMII_1ST_PINMUX))
    {

        pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_txclk, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_tx_d2, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_tx_d3, PAD_DRIVINGSINK_LEVEL_1);
    }else if((interface == PIN_ETH_CFG_RMII_2ND_PINMUX) || (interface == PIN_ETH2_CFG_RMII_2ND_PINMUX))
    {
        pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_rmii_refclk_2, PAD_DRIVINGSINK_LEVEL_0);
        pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_0);
        pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_0);
    }else if((interface == PIN_ETH_CFG_RGMII_2ND_PINMUX) || (interface == PIN_ETH2_CFG_RGMII_2ND_PINMUX))
    {
        pad_set_drivingsink(p_info->pad_phy_clk_2, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_txclk_2, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_txctl_2, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_tx_d0_2, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_tx_d1_2, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_tx_d2_2, PAD_DRIVINGSINK_LEVEL_1);
        pad_set_drivingsink(p_info->pad_tx_d3_2, PAD_DRIVINGSINK_LEVEL_1);
    }

	return 0;
}

int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base, int *embd_en)
{
	int ret = 0;
	struct resource *res;
	void __iomem *base;
	PIN_GROUP_CONFIG pinmux_config[1];
	UINT ETH_QOS_INDEX;	// index to MAC (0: MAC0, 1: MAC1);
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;
	int spclk_en = 0;
	int refclk_out = 0;
#endif

//	ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No memory resources\n");
		return -1;
	}
	printk("%s: IO MEM res start 0x%llx\r\n", __func__, (u64)res->start);
	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to find IORESOURCE_MEM\n");
		return -1;
//		ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	} else {
		*p_base = base;
//		ETH_QOS_REG_BASE = base;
	}
	if (res->start == v_nvt_plat_info[0].base_pa) {
		ETH_QOS_INDEX = 0;
	} else {
		ETH_QOS_INDEX = 1;
	}
//	ETH_QOS_INDEX = 1;	// temp force to MAC1 (for embedded phy)
	printk("%s: get IO MEM 0x%px\r\n", __func__, base);

	pinmux_config[0].pin_function = PIN_FUNC_ETH;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		dev_err(&pdev->dev, "get pinmux config failed\n");
	printk("%s: get pinmux 0x%x\r\n", __func__, pinmux_config[0].config);

	// impelment MAC1
/*==== RMII =====*/
	if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii)) {
		unsigned long reg;

		printk("%s: pinmux detect RMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RMII, &v_nvt_plat_info[ETH_QOS_INDEX]);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if(!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
            if(spclk_en &&
                (!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {
                struct clk *clk;
#if 1

                printk("%s: change extphyclk pinmux\r\n", __func__);
                pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;
                ret = nvt_pinmux_update(pinmux_config, 1);
                if (ret)
                    pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",__func__,__LINE__);

                //printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);

		if(spclk_en) {
			clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
			if (IS_ERR(clk)) {
				dev_err(&pdev->dev, "can't find clock %s\n",
						v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				clk = NULL;
			} else {
				clk_prepare(clk);
				clk_enable(clk);
			}
			clk_put(clk);
		}
	    }
#else
			if(spclk_en &&
					(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

				ret = nvt_pinmux_update(pinmux_config, 1);
				if (ret)
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
					        __func__,__LINE__);

				clk = clk_get(&pdev->dev, "ext_phy_clk");
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
#endif
		}

		// assert phy reset
		set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii, &v_nvt_plat_info[ETH_QOS_INDEX]);
		gpio_direction_output(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		mdelay(20);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 1);
		mdelay(50);
#endif

		// select external phy
		reg = ioread32((void*)(base + 0x3000 + 0x4));
#ifdef CONFIG_NVT_FPGA_EMULATION
		reg |= 1<<31;	// output at clock falling
#endif
		iowrite32(reg | (1<<4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));

		// setup RMII REF_CLK direction
		reg = ioread32((void*)(base + 0x3000 + 0x14));
		// pre-assume refclk: phy--> mac
		reg |= 1<<0;
		reg &= ~(1<<4);
#ifdef CONFIG_NVT_FPGA_EMULATION
        reg |= 1<<2;
#endif
#ifdef CONFIG_OF
		// change RMII REF_CLK direction by DTB setting
		if(!of_property_read_u32(np, "ref-clk-out", &refclk_out)) {
			printk("%s: find ref-clk-out %d\r\n", __func__, refclk_out);
			if(refclk_out) {
				printk("%s: change REF_CLK out\r\n", __func__);
				reg &= ~(1<<0);
				reg |= 1<<4;
			}
		}
		iowrite32(reg | (1<<5), (void *)(base + 0x3000 + 0x14));
#endif
		//iowrite32(reg, (void *)(base + 0x3000 + 0x14));
/*==== RGMII =====*/
	} else if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii)) {
		unsigned long reg;

		printk("%s: pinmux detect RGMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RGMII, &v_nvt_plat_info[ETH_QOS_INDEX]);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if(!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
#if 1
			//printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);
			if(spclk_en &&
					(!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {
                struct clk *clk;

                printk("%s: change extphyclk pinmux\r\n", __func__);
                pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;

				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
                if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
						v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
#else
			if(spclk_en &&
					(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

				ret = nvt_pinmux_update(pinmux_config, 1);
				if (ret)
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
					        __func__,__LINE__);

				clk = clk_get(&pdev->dev, "ext_phy_clk");
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
#endif
		}

		// assert phy reset
		set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii, &v_nvt_plat_info[ETH_QOS_INDEX]);
		gpio_direction_output(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		mdelay(20);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 1);
		mdelay(50);

#endif

		// select external phy
		reg = ioread32((void*)(base + 0x3000 + 0x4));
		iowrite32(reg | (1<<4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));
        //iowrite32(0x102, (void *)(base + 0x3000 + 0x2C)); //SRAM

		// setup TXD_SRC as RGMII
		reg = ioread32((void*)(base + 0x3000 + 0x14));
        //reg |= (1<<0);
        reg |= (3<<4);
		reg &= ~(3<<30);
		reg |= 1<<30;

#ifdef CONFIG_NVT_FPGA_EMULATION
		if (ETH_QOS_INDEX == 0) {
			// TXCLK_DELAY = 12, RX_DLY = 7;
			reg |= (0<<16) | (0<<20);
		} else {
			// TXCLK_DELAY = 8, RX_DLY = 8;
			reg |= (0<<16) | (0<<20);
		}
        //reg |= (1<<29);
#endif

		iowrite32(reg, (void *)(base + 0x3000 + 0x14));
/*==== embd phy =====*/
	} else {
		unsigned long reg;
		int emb_phy_driving;

		printk("%s: pinmux detect emb phy 0x%x\r\n", __func__, pinmux_config[0].config);

		nvt_eth_phy_poweron(base);

		// set phy selection for internal
		reg = ioread32((void*)(base + 0x3000 + 0x04));
		iowrite32(reg & ~(3<<4), (void *)(base + 0x3000 + 0x04));

		// set phy mdio address
		reg = ioread32((void*)(base + 0x3800 + 0x00));
		iowrite32(reg | (1<<1), (void *)(base + 0x3800 + 0x00));
		reg = ioread32((void*)(base + 0x3800 + 0x1C));
		iowrite32(reg | (1<<0), (void *)(base + 0x3800 + 0x1C));
		reg = ioread32((void*)(base + 0x3800 + 0x00));
		iowrite32(reg & ~(1<<1), (void *)(base + 0x3800 + 0x00));
		*embd_en = 1;
		if(!of_property_read_u32(np, "emb-phy-driving", &emb_phy_driving)) {
			unsigned long reg;
			unsigned long step;
			int value;

			reg = ioread32((void*)(base + 0x3800 + 0x378));
			step = reg & 0x1F;
			reg &= ~0x1F;
			// transform 2's compelment to integer
			if (step & 0x10) {
				step = ~step;
				step++;
				value = step & 0x1F;
				value = 0 - value;
			} else {
				value = step & 0xF;
			}
			value += emb_phy_driving;
			// transform integer to 2's complement
			if (value >= 0) {
				if (value > 15) value = 15;
				step = value;
			} else {
				if (value <= -16) value = -16;
				value = 0 - value;
				step = value;
				step = ~step;
				step++;
			}
			step &= 0x1F;
			reg |= step;
			iowrite32(reg, (void *)(base + 0x3800 + 0x378));

			printk("%s: DTS emb driving inc %d\r\n", __func__, emb_phy_driving);
		}
	}

	return (int)ETH_QOS_INDEX;
}
#else
/* ---------------------default-------------------------- */
static const struct NVT_PLAT_INFO v_nvt_plat_info[2] = {
	// MAC0 (0xFCC0_0000)
	{
		0x2F02b0000,
		"rmii0_refclk_i",
		"eth0_rxclk_i",
		"eth0_txclk_i",
		"eth0_extphy_clk",
		"eth0_ptp_clk",

		PIN_ETH_CFG_ETH_RGMII_1,//PIN_ETH_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_ETH_RMII_1,//PIN_ETH_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_ETH_EXTPHYCLK,//PIN_ETH_CFG_REFCLK_PINMUX,

		D_GPIO(8),//D_GPIO(8),          // gpio reset
		D_GPIO(8),//NOT SUP,            // gpio reset

		PAD_DS_DGPIO7,//PAD_DS_DGPIO7,  // PHYCLK PIN_1
		PAD_DS_LGPIO24,//PAD_DS_LGPIO24,    // TXCLK PIN_1
		PAD_DS_LGPIO19,//PAD_DS_LGPIO19,    // TXCTL PIN_1
		PAD_DS_LGPIO25,//PAD_DS_LGPIO25,    // REFCLK (RMII) PIN_1
		PAD_DS_LGPIO20,//PAD_DS_LGPIO20,    // TXD[0] PIN_1
		PAD_DS_LGPIO21,//PAD_DS_LGPIO21,    // TXD[1] PIN_1
		PAD_DS_LGPIO22,//PAD_DS_LGPIO22,    // TXD[2] PIN_1
		PAD_DS_LGPIO23,//PAD_DS_LGPIO23,    // TXD[3] PIN_1

		PAD_DS_DGPIO7,//NOT SUP,    // PHYCLK PIN_2
		PAD_DS_LGPIO24,//NOT SUP,   // TXCLK PIN_2
		PAD_DS_LGPIO19,//NOT SUP,   // TXCTL PIN_2
		PAD_DS_LGPIO25,//NOT SUP,   // REFCLK (RMII) PIN_2
		PAD_DS_PGPIO20,//NOT SUP,   // TXD[0] PIN_2
		PAD_DS_LGPIO21,//NOT SUP,   // TXD[1] PIN_2
		PAD_DS_LGPIO22,//NOT SUP,   // TXD[2] PIN_2
		PAD_DS_LGPIO23,//NOT SUP,   // TXD[3] PIN_2
	},
	// MAC1 (0xFCD0_0000)
	{
		0x2F0430000,
		"rmii1_refclk_i",
		"eth1_rxclk_i",
		"eth1_txclk_i",
		"eth1_extphy_clk",
		"eth1_ptp_clk",

		PIN_ETH_CFG_ETH2_RGMII_1,//PIN_ETH2_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_ETH2_RMII_1,//PIN_ETH2_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_NONE,//PIN_ETH2_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		PIN_ETH_CFG_ETH2_EXTPHYCLK,//PIN_ETH2_CFG_REFCLK_PINMUX,

		L_GPIO(18),//L_GPIO(18),            // gpio reset
		L_GPIO(18),//NOT SUP,           // gpio reset

		PAD_DS_LGPIO17,//PAD_DS_LGPIO17,    // PHYCLK
		PAD_DS_LGPIO8,//PAD_DS_LGPIO8,  // TXCLK
		PAD_DS_LGPIO3,//PAD_DS_LGPIO3,  // TXCTL
		PAD_DS_LGPIO9,//PAD_DS_LGPIO9,  // REFCLK (RMII)
		PAD_DS_LGPIO4,//PAD_DS_LGPIO4,  // TXD[0]
		PAD_DS_LGPIO5,//PAD_DS_LGPIO5,  // TXD[1]
		PAD_DS_LGPIO6,//PAD_DS_LGPIO6,  // TXD[2]
		PAD_DS_LGPIO7,//PAD_DS_LGPIO7,  // TXD[3]

		PAD_DS_LGPIO17,//NOT SUP,   // PHYCLK PIN_2
		PAD_DS_LGPIO8,//NOT SUP,    // TXCLK PIN_2
		PAD_DS_LGPIO3,//NOT SUP,    // TXCTL PIN_2
		PAD_DS_LGPIO9,//NOT SUP,    // REFCLK (RMII) PIN_2
		PAD_DS_LGPIO4,//NOT SUP,    // TXD[0] PIN_2
		PAD_DS_LGPIO5,//NOT SUP,    // TXD[1] PIN_2
		PAD_DS_LGPIO6,//NOT SUP,    // TXD[2] PIN_2
		PAD_DS_LGPIO7,//NOT SUP,    // TXD[3] PIN_2
	},
};


static void nvt_eth_phy_poweron(void __iomem *base_addr)
{
	unsigned long reg;

	reg = ioread32((void *)(base_addr + 0x3800 + 0xF8));
	iowrite32(reg | (1 << 7), (void *)(base_addr + 0x3800 + 0xF8));
	udelay(20);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(200);
	reg = ioread32((void *)(base_addr + 0x3800 + 0xC8));
	iowrite32(reg & (~(1 << 1)), (void *)(base_addr + 0x3800 + 0xC8));
	udelay(250);
	reg = ioread32((void *)(base_addr + 0x3800 + 0x2E8));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x2E8));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xCC));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0xCC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0xDC));
	iowrite32(reg | (1 << 0), (void *)(base_addr + 0x3800 + 0xDC));
	reg = ioread32((void *)(base_addr + 0x3800 + 0x9C));
	iowrite32(reg & (~(1 << 0)), (void *)(base_addr + 0x3800 + 0x9C));
}

static int set_pad_driving(UINT interface, const struct NVT_PLAT_INFO *p_info)
{

	if ((interface == PIN_ETH_CFG_ETH_RMII_1) || (interface == PIN_ETH_CFG_ETH2_RMII_1)) {
		pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_rmii_refclk, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_10);
	} else if ((interface == PIN_ETH_CFG_ETH_RGMII_1) || (interface == PIN_ETH_CFG_ETH2_RGMII_1)) {

		pad_set_drivingsink(p_info->pad_phy_clk, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_txclk, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_txctl, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d0, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d1, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d2, PAD_DRIVINGSINK_LEVEL_10);
		pad_set_drivingsink(p_info->pad_tx_d3, PAD_DRIVINGSINK_LEVEL_10);
	}

	return 0;
}

int nvt_eth_env_probe(struct platform_device *pdev, void __iomem **p_base)
{
	int ret = 0;
	struct resource *res;
	void __iomem *base;
	PIN_GROUP_CONFIG pinmux_config[1];
	UINT ETH_QOS_INDEX; // index to MAC (0: MAC0, 1: MAC1);
#if IS_ENABLED(CONFIG_TIMESTAMP)
	struct clk *ptpclk;
#endif
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;
	int spclk_en = 0;
	int refclk_out = 0;
#endif

//	ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "No memory resources\n");
		return -1;
	}
	printk("%s: IO MEM res start 0x%llx\r\n", __func__, res->start);

	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "Unable to find IORESOURCE_MEM\n");
		return -1;
//		ETH_QOS_REG_BASE = (void __iomem *)(0xFCD00000);    // pre-assume base address
	} else {
		*p_base = base;
//		ETH_QOS_REG_BASE = base;
	}
	if (res->start == v_nvt_plat_info[0].base_pa) {
		ETH_QOS_INDEX = 0;
	} else {
		ETH_QOS_INDEX = 1;
	}
//	ETH_QOS_INDEX = 1;   // temp force to MAC1 (for embedded phy)
	printk("%s: get IO MEM 0x%px\r\n", __func__, base);

	pinmux_config[0].pin_function = PIN_FUNC_ETH;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		dev_err(&pdev->dev, "get pinmux config failed\n");

	printk("%s: get pinmux 0x%x\r\n", __func__, pinmux_config[0].config);

#if IS_ENABLED(CONFIG_TIMESTAMP)
	ptpclk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
	if (IS_ERR(ptpclk)) {
		dev_err(&pdev->dev, "can't find ptp_clock %s\n",
				v_nvt_plat_info[ETH_QOS_INDEX].ptp_clk_name);
		ptpclk = NULL;
	} else {
		clk_prepare(ptpclk);
		clk_enable(ptpclk);
	}
	clk_put(ptpclk);
#endif

#if IS_ENABLED(CONFIG_MULTI_IRQ)

	reg = ioread32((void *)(base + 0x3004));
	iowrite32(reg | (1 << 0), (void *)(base + 0x3004));

	reg = ioread32((void *)(base + 0x3028));
	reg = reg & ~(3 << 0);
	iowrite32(reg | (3 << 2), (void *)(base + 0x3028));
#endif

	// impelment MAC1
	/*==== RMII =====*/
	if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii)) {
		unsigned long reg;

		printk("%s: pinmux detect RMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RMII, &v_nvt_plat_info[ETH_QOS_INDEX]);
#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
			if (spclk_en &&
				(!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {
				struct clk *clk;
#if 1

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;
				ret = nvt_pinmux_update(pinmux_config, 1);

				if (ret) {
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n", __func__, __LINE__);
				}

				printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);

				if (spclk_en) {
					clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					if (IS_ERR(clk)) {
						dev_err(&pdev->dev, "can't find clock %s\n",
								v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
						clk = NULL;
					} else {
						clk_prepare(clk);
						clk_enable(clk);
					}
					clk_put(clk);
				}
			}
#else
				if (spclk_en &&
					(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
					struct clk *clk;

					printk("%s: change extphyclk pinmux\r\n", __func__);
					pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

					ret = nvt_pinmux_update(pinmux_config, 1);
					if (ret)
						pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
								__func__, __LINE__);

					clk = clk_get(&pdev->dev, "ext_phy_clk");
					if (IS_ERR(clk)) {
						dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
						clk = NULL;
					} else {
						clk_prepare(clk);
						clk_enable(clk);
					}
					clk_put(clk);
				}
#endif
		}

		// assert phy reset
		set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii, &v_nvt_plat_info[ETH_QOS_INDEX]);
		gpio_direction_output(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		mdelay(20);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 1);
		mdelay(50);

#endif

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
#ifdef CONFIG_NVT_FPGA_EMULATION
		//reg |= 1<<31; // output at clock falling
#endif
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));

		// setup RMII REF_CLK direction
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		// pre-assume refclk: phy--> mac
		reg |= 1 << 0;
		reg &= ~(1 << 4);
#ifdef CONFIG_NVT_FPGA_EMULATION
		reg |= 1 << 2;
#endif
#ifdef CONFIG_OF
		// change RMII REF_CLK direction by DTB setting
		if (!of_property_read_u32(np, "ref-clk-out", &refclk_out)) {
			printk("%s: find ref-clk-out %d\r\n", __func__, refclk_out);
			if (refclk_out) {
				printk("%s: change REF_CLK out\r\n", __func__);
				reg &= ~(1 << 0);
				reg |= 1 << 4;
			}
		}
		iowrite32(reg | (1 << 5), (void *)(base + 0x3000 + 0x14));
#endif
		//iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== RGMII =====*/
	} else if (pinmux_config[0].config & (v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii)) {
		unsigned long reg;

		printk("%s: pinmux detect RGMII 0x%x\r\n", __func__, pinmux_config[0].config);
		//set_pad_driving(PHY_INTERFACE_MODE_RGMII, &v_nvt_plat_info[ETH_QOS_INDEX]);

#ifdef CONFIG_OF
		// output 25 MHz as external phy crystal input by DTB setting
		if (!of_property_read_u32(np, "sp-clk", &spclk_en)) {

			printk("%s: find sp-clk %d\r\n", __func__, spclk_en);
#if 1
			printk("%s: TBD: pinctrl still not implement split phy clk\r\n", __func__);
			if (spclk_en &&
				(!(pinmux_config[0].config & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy;

				clk = clk_get(&pdev->dev, v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n",
							v_nvt_plat_info[ETH_QOS_INDEX].ext_phy_clk_name);
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
#else
			if (spclk_en &&
				(!(pinmux_config[0].config & PIN_ETH_CFG_EXTPHYCLK))) {
				struct clk *clk;

				printk("%s: change extphyclk pinmux\r\n", __func__);
				pinmux_config[0].config |= PIN_ETH_CFG_EXTPHYCLK;

				ret = nvt_pinmux_update(pinmux_config, 1);
				if (ret)
					pr_info("%s(%d) setup EXTPHYCLK pinmux fail\n",
							__func__, __LINE__);

				clk = clk_get(&pdev->dev, "ext_phy_clk");
				if (IS_ERR(clk)) {
					dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
					clk = NULL;
				} else {
					clk_prepare(clk);
					clk_enable(clk);
				}
				clk_put(clk);
			}
#endif
		}

		// assert phy reset
		set_pad_driving(v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii, &v_nvt_plat_info[ETH_QOS_INDEX]);
		gpio_direction_output(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 0);
		mdelay(20);
		gpio_set_value(v_nvt_plat_info[ETH_QOS_INDEX].gpio_reset, 1);
		mdelay(50);

#endif

		// select external phy
		reg = ioread32((void *)(base + 0x3000 + 0x4));
		iowrite32(reg | (1 << 4), (void *)(base + 0x3000 + 0x4));
		iowrite32(0x1FF, (void *)(base + 0x3000 + 0xC));
		//iowrite32(0x102, (void *)(base + 0x3000 + 0x2C)); //SRAM

		// setup TXD_SRC as RGMII
		reg = ioread32((void *)(base + 0x3000 + 0x14));
		//reg |= (1<<0);
		reg |= (3 << 4);
		reg &= ~(3 << 30);
		reg |= 1 << 30;

#ifdef CONFIG_NVT_FPGA_EMULATION
		// coverity[identical_branches]
		if (ETH_QOS_INDEX == 0) {
			// TXCLK_DELAY = 12, RX_DLY = 7;
			reg |= (0 << 16) | (0 << 20);
		} else {
			// TXCLK_DELAY = 8, RX_DLY = 8;
			reg |= (0 << 16) | (0 << 20);
		}
		//reg |= (1<<29);
#endif

		iowrite32(reg, (void *)(base + 0x3000 + 0x14));
		/*==== embd phy =====*/
	} else {
		unsigned long reg;

		printk("%s: pinmux detect emb phy 0x%x\r\n", __func__, pinmux_config[0].config);

		nvt_eth_phy_poweron(base);

		// set phy mdio address
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg | (1 << 1), (void *)(base + 0x3800 + 0x00));
		reg = ioread32((void *)(base + 0x3800 + 0x1C));
		iowrite32(reg | (1 << 0), (void *)(base + 0x3800 + 0x1C));
		reg = ioread32((void *)(base + 0x3800 + 0x00));
		iowrite32(reg & ~(1 << 1), (void *)(base + 0x3800 + 0x00));
	}

	return (int)ETH_QOS_INDEX;
}
#endif

#ifdef CONFIG_PM
void nvt_eth_get_clk_name(struct DWC_ETH_QOS_prv_data *pdata, int idx_mac)
{
#if defined(CONFIG_NVT_IVOT_PLAT_NA51089) || defined(CONFIG_NVT_IVOT_PLAT_NA51055)
	pdata->ext_phy_clk_name = "ext_phy_clk";
#else
	pdata->ext_phy_clk_name = v_nvt_plat_info[idx_mac].ext_phy_clk_name;

#if IS_ENABLED(CONFIG_TIMESTAMP)
	pdata->ptp_clk_name = v_nvt_plat_info[idx_mac].ptp_clk_name;
#endif
#endif
}
#endif
