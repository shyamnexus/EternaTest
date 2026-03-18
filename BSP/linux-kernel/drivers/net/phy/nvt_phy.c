/*
 * drivers/net/phy/nvt_phy.c
 *
 * Driver for Novatek PHYs
 *
 * Copyright (c) 2023 Novatek Microelectronics Corp. All rights reserved.
 *
 */
#include <linux/phy.h>
#include <linux/module.h>
//#include <linux/gpio.h>
//#include <nvt-gpio.h>
//#include <nvt-gpio.h>
#include <linux/delay.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#endif

#include "nvt_phy.h"
#include <plat/top.h>
#include <linux/soc/nvt/nvt-info.h>
#ifdef NVT_ETH_TRIM_APPLY
#include <plat/efuse_protected.h>
#include <linux/soc/nvt/rcw_macro.h>
#endif

static int g_resumed = 0;
static int bplug = 0;
static int g_enable_e_loopback = 0;
static int g_enable_i_loopback = 0;
static int g_phy_power_down = 0;

static enum phy_state na510xx_phy_state = PHY_DOWN;

static void __iomem *NVT_PHY_BASE = 0;
PIN_GROUP_CONFIG pinmux_config[1];

static ssize_t loopback_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "external loopback mode is %s\n"
			"internal loopback mode is %s\n",
			(g_enable_e_loopback) ? "enable" : "disable",
			(g_enable_i_loopback) ? "enable" : "disable");
}

static ssize_t loopback_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t len)
{
	struct phy_device *phydev = to_phy_device(dev);
	int enable_lm = 0, loopback_mode = 0, ret, val;

	ret = sscanf(buf, "%d %d", &loopback_mode, &enable_lm);

	if ((ret != 2) || ((loopback_mode >> 1) != 0) || ((enable_lm >> 1) != 0)) {
		printk("invalid loopback_mode : %d (internal = 0, external = 1)"
			"or enable value : %d (disable = 0, enable = 1)\n",
			loopback_mode, enable_lm);
		return -EINVAL;
	}

	if (loopback_mode == 1) {
		// external loppback mode
		if (enable_lm == 1) {
			phy_sw_reset_enable();
			iowrite32(0x3c, (void *)(NVT_PHY_BASE + 0x804));
			iowrite32(0x80, (void *)(NVT_PHY_BASE + 0x874));
			usleep_range(10000, 20000);
			phy_sw_reset_disable();
			g_enable_e_loopback = 1;
		} else {
			phy_sw_reset_enable();
			iowrite32(0x00, (void *)(NVT_PHY_BASE + 0x804));
			usleep_range(10000, 20000);
			phy_sw_reset_disable();
			g_enable_e_loopback = 0;
		}
	} else {
		//internal loopback mode
		if (enable_lm == 1) {
			val = phy_read(phydev, MII_BMCR);
			if (val < 0)
				return -EIO;

			val |= BMCR_LOOPBACK;

			ret = phy_write(phydev, MII_BMCR, val);
			if (ret < 0)
				return -EIO;
			g_enable_i_loopback = 1;
		} else {
			val = phy_read(phydev, MII_BMCR);
			if (val < 0)
				return -EIO;

			val &= ~BMCR_LOOPBACK;

			ret = phy_write(phydev, MII_BMCR, val);
			if (ret < 0)
				return -EIO;
			g_enable_i_loopback = 0;
		}
	}
	return len;
}

static ssize_t phy_power_down_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "suspend power_down is %s\n",
			(g_phy_power_down) ? "enable" : "disable");
}

static ssize_t phy_power_down_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t len)
{
	int ret, val;

	ret = sscanf(buf, "%d", &val);

	if ((ret != 1) || ((val >> 1) != 0)) {
		printk("invalid phy_power_down setting (0:disable, 1:enable): %d", val);
		return -EINVAL;
	}

	g_phy_power_down = val;

	return len;
}


static DEVICE_ATTR_RW(loopback);
static DEVICE_ATTR_RW(phy_power_down);

static int nvt_phy_sysfs_init(struct device *dev)
{
	int ret = 0, ret1 = 0;

	ret = sysfs_create_file(&dev->kobj, &dev_attr_loopback.attr);
	ret1 = sysfs_create_file(&dev->kobj, &dev_attr_phy_power_down.attr);

	return ret | ret1;
}

static void nvt_phy_sysfs_remove(struct device *dev)
{
	sysfs_remove_file(&dev->kobj, &dev_attr_loopback.attr);
	sysfs_remove_file(&dev->kobj, &dev_attr_phy_power_down.attr);
}

/**
 * phy_poll_reset - Safely wait until a PHY reset has properly completed
 * @phydev: The PHY device to poll
 *
 * Description: According to IEEE 802.3, Section 2, Subsection 22.2.4.1.1, as
 *   published in 2008, a PHY reset may take up to 0.5 seconds.  The MII BMCR
 *   register must be polled until the BMCR_RESET bit clears.
 *
 *   Furthermore, any attempts to write to PHY registers may have no effect
 *   or even generate MDIO bus errors until this is complete.
 *
 *   Some PHYs (such as the Marvell 88E1111) don't entirely conform to the
 *   standard and do not fully reset after the BMCR_RESET bit is set, and may
 *   even *REQUIRE* a soft-reset to properly restart autonegotiation.  In an
 *   effort to support such broken PHYs, this function is separate from the
 *   standard phy_init_hw() which will zero all the other bits in the BMCR
 *   and reapply all driver-specific and board-specific fixups.
 */
static int phy_poll_reset(struct phy_device *phydev)
{
	/* Poll until the reset bit clears (50ms per retry == 0.6 sec) */
	unsigned int retries = 12;
	int ret;

	printk("%s: enter\r\n", __func__);
	do {
//		msleep(50);
		ret = phy_read(phydev, MII_BMCR);
		if (ret < 0)
			return ret;
	} while (ret & BMCR_RESET && --retries);
	if (ret & BMCR_RESET)
		return -ETIMEDOUT;

	/* Some chips (smsc911x) may still need up to another 1ms after the
	 * BMCR_RESET bit is cleared before they are usable.
	 */
//	usleep_range(1000, 2000);
	return 0;
}

/**
 * genphy_soft_reset - software reset the PHY via BMCR_RESET bit
 * @phydev: target phy_device struct
 *
 * Description: Perform a software PHY reset using the standard
 * BMCR_RESET bit and poll for the reset bit to be cleared.
 *
 * Returns: 0 on success, < 0 on failure
 */
static int nvt_soft_reset(struct phy_device *phydev)
{
	int val;
	int ret;

	printk("%s: enter\r\n", __func__);
	val = phy_read(phydev, MII_BMCR);
	if (val < 0)
		return val;

	val |= BMCR_RESET;

	ret = phy_write(phydev, MII_BMCR, val);
	if (ret < 0)
		return ret;

	return phy_poll_reset(phydev);
}

static int nvt_config_init(struct phy_device *phydev)
{
	int val;
	__ETHTOOL_DECLARE_LINK_MODE_MASK(features) = { 0, };

	linkmode_set_bit(ETHTOOL_LINK_MODE_TP_BIT, features);
	linkmode_set_bit(ETHTOOL_LINK_MODE_MII_BIT, features);
	linkmode_set_bit(ETHTOOL_LINK_MODE_AUI_BIT, features);
	linkmode_set_bit(ETHTOOL_LINK_MODE_FIBRE_BIT, features);
	linkmode_set_bit(ETHTOOL_LINK_MODE_BNC_BIT, features);
	linkmode_set_bit(ETHTOOL_LINK_MODE_Pause_BIT, features);
	linkmode_set_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT, features);

	/* Do we support autonegotiation? */
	val = phy_read(phydev, MII_BMSR);
	if (val < 0)
		return val;

	if (val & BMSR_ANEGCAPABLE)
		linkmode_set_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, features);

	if (val & BMSR_100FULL)
		linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, features);
	if (val & BMSR_100HALF)
		linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT, features);
	if (val & BMSR_10FULL)
		linkmode_set_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT, features);
	if (val & BMSR_10HALF)
		linkmode_set_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT, features);

	if (val & BMSR_ESTATEN) {
		val = phy_read(phydev, MII_ESTATUS);
		if (val < 0)
			return val;

		if (val & ESTATUS_1000_TFULL)
			linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT, features);
		if (val & ESTATUS_1000_THALF)
			linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT, features);
	}

	linkmode_and(phydev->supported, phydev->supported, features);
	linkmode_and(phydev->advertising, phydev->advertising, features);

	return 0;
}

static int nvt_config_aneg(struct phy_device *phydev)
{
	int ret;

	printk("%s: enter\r\n", __func__);
	if (phydev->autoneg) {
		iowrite32(0xE0, (void *)(NVT_PHY_BASE + 0xB80));
	} else {
		iowrite32(0xA0, (void *)(NVT_PHY_BASE + 0xB80));
	}
	ret = genphy_config_aneg(phydev);
	if (ret) return ret;

	if (phydev->autoneg) {
		ret = genphy_restart_aneg(phydev);
	}

	return ret;
}

static void eth_phy_poweron(void)
{
	unsigned long reg;

	printk("%s: enter\r\n", __func__);
	if (NVT_PHY_BASE == NULL) {
		printk("%s: NVT_PHY_BASE is NULL\r\n", __func__);
		return;
	}

	reg = ioread32((void *)(NVT_PHY_BASE + 0x8F8));
	iowrite32(reg | (1 << 7), (void *)(NVT_PHY_BASE + 0x8F8));
	udelay(20);
	reg = ioread32((void *)(NVT_PHY_BASE + 0x8C8));
	iowrite32(reg & (~(1 << 0)), (void *)(NVT_PHY_BASE + 0x8C8));
	udelay(200);
	reg = ioread32((void *)(NVT_PHY_BASE + 0x8C8));
	iowrite32(reg & (~(1 << 1)), (void *)(NVT_PHY_BASE + 0x8C8));
	udelay(250);
	reg = ioread32((void *)(NVT_PHY_BASE + 0xAE8));
	iowrite32(reg & (~(1 << 0)), (void *)(NVT_PHY_BASE + 0xAE8));
	reg = ioread32((void *)(NVT_PHY_BASE + 0x8CC));
	iowrite32(reg & (~(1 << 0)), (void *)(NVT_PHY_BASE + 0x8CC));
	reg = ioread32((void *)(NVT_PHY_BASE + 0x8DC));
	iowrite32(reg | (1 << 0), (void *)(NVT_PHY_BASE + 0x8DC));
	reg = ioread32((void *)(NVT_PHY_BASE + 0x89C));
	iowrite32(reg & (~(1 << 0)), (void *)(NVT_PHY_BASE + 0x89C));
}

static void eth_phy_poweroff(void)
{
	printk("%s: enter\r\n", __func__);
	if (NVT_PHY_BASE == NULL) {
		printk("%s: NVT_PHY_BASE is NULL\r\n", __func__);
		return;
	}

	iowrite32(0x01, (void *)(NVT_PHY_BASE + 0x8DC));
	iowrite32(0x33, (void *)(NVT_PHY_BASE + 0x89C));
	iowrite32(0xa5, (void *)(NVT_PHY_BASE + 0x8CC));
	iowrite32(0x11, (void *)(NVT_PHY_BASE + 0x88C));
	iowrite32(0x01, (void *)(NVT_PHY_BASE + 0xAE8));
	iowrite32(0x11, (void *)(NVT_PHY_BASE + 0xAE0));
	iowrite32(0x01, (void *)(NVT_PHY_BASE + 0x8C8));
	iowrite32(0x08, (void *)(NVT_PHY_BASE + 0x888));
	iowrite32(0x03, (void *)(NVT_PHY_BASE + 0x8C8));
	iowrite32(0x08, (void *)(NVT_PHY_BASE + 0x888));
	iowrite32(0x61, (void *)(NVT_PHY_BASE + 0x8f8));
}

#ifdef NVT_ETH_TRIM_APPLY
static void set_eth_phy_trim(void)
{
	int trim_ret;
	UINT16 trim_val;
	UINT16 tx_trim_rout = NVT_ETH_TX_TRIM_ROUT;
	UINT16 tx_trim_dac = NVT_ETH_TX_TRIM_DAC;
	UINT16 tx_trim_sel_rx = NVT_ETH_TX_TRIM_SEL_RX;
	UINT16 tx_trim_sel_tx = NVT_ETH_TX_TRIM_SEL_TX;
	unsigned long reg;

	trim_ret = efuse_readParamOps(EFUSE_ETHERNET_TRIM_DATA, &trim_val);
	if (trim_ret == EFUSE_SUCCESS) {
		printk("%s: trim data 0x%04x \r\n", __func__, trim_val);
		tx_trim_rout = trim_val & 0x7;              // bit[2..0]
/*		if (!NVT_TRIM_RANGE_CHECK(0x0, 0x7, (INT16)tx_trim_rout)) {
			tx_trim_rout = NVT_ETH_TX_TRIM_ROUT;
		}
*/
		tx_trim_dac = (trim_val >> 3) & 0x1F;       // bit[7..3]
		if (!NVT_TRIM_RANGE_CHECK(0x0, 0xc, (INT16)tx_trim_dac) &&
				!NVT_TRIM_RANGE_CHECK(0x14, 0x1f, (INT16)tx_trim_dac)) {
			tx_trim_dac = NVT_ETH_TX_TRIM_DAC;
		}

		tx_trim_sel_rx = (trim_val >> 8) & 0x7;     // bit[10..8]
		if (!NVT_TRIM_RANGE_CHECK(0x0, 0x6, (INT16)tx_trim_sel_rx)) {
				tx_trim_sel_rx = NVT_ETH_TX_TRIM_SEL_RX;
		}

		tx_trim_sel_tx = (trim_val >> 11) & 0x7;    // bit[13..11]
		if (!NVT_TRIM_RANGE_CHECK(0x0, 0x6, (INT16)tx_trim_sel_tx)) {
				tx_trim_sel_tx = NVT_ETH_TX_TRIM_SEL_TX;
		}
	}

	reg = ioread32((void *)(NVT_PHY_BASE + 0xB74));
	reg &= ~(0x38);
	iowrite32(reg | (tx_trim_rout << 3), (void *)(NVT_PHY_BASE + 0xB74));
	reg = ioread32((void *)(NVT_PHY_BASE + 0xB78));
	reg &= ~(0x1F);
	iowrite32(reg | tx_trim_dac, (void *)(NVT_PHY_BASE + 0xB78));
	reg = ioread32((void *)(NVT_PHY_BASE + 0xB68));
	reg &= ~(0xFC);
	iowrite32(reg | (tx_trim_sel_rx << 5) | (tx_trim_sel_tx << 2), (void *)(NVT_PHY_BASE + 0xB68));
}
#endif

static int nvt_resume(struct phy_device *phydev)
{
	int inv_led = 0;
#ifdef CONFIG_OF
	struct device_node *of_node = NULL, *of_node_64 = NULL;

	if (g_resumed) {
		goto genphy_config_an;
	}
#if defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	of_node_64 = of_find_node_by_path("/phy@2,f01b3800");
#else
	of_node = of_find_node_by_path("/phy@f02b3800");
#endif
	if (of_node) {
		// coverity[dead_error_line]
		if (!of_property_read_u32(of_node, "led-inv", &inv_led)) {
			printk("%s: led-inv %d found\r\n", __func__, inv_led);
		}
	} else {
		if (!of_property_read_u32(of_node_64, "led-inv", &inv_led)) {
			printk("%s: led-inv %d found\r\n", __func__, inv_led);
		}
	}
#endif
	printk("%s: enter\r\n", __func__);

	na510xx_phy_state = phydev->state;

	if (NVT_PHY_BASE != NULL && g_resumed == 0) {
		eth_phy_poweron();
#ifdef NVT_ETH_TRIM_APPLY
		set_eth_phy_trim();
#endif
		set_am_option();
		if (nvt_get_chip_id() == CHIP_NA51055) {
			/* NA51055 */
			set_best_setting_t28();
		} else {
			/* else */
			set_best_setting_t22();
		}
		phy_sw_reset_enable();
		set_break_link_timer();
#if defined(CONFIG_NVT_IVOT_PLAT_NA51055)
		if ((pinmux_config[0].config&PIN_ETH_CFG_LED_1ST_ONLY) ||
			(pinmux_config[0].config&PIN_ETH_CFG_LED_2ND_ONLY)) {
			set_one_led_link_act();
		} else {
			set_two_leds_link_act();
		}
#else
		set_two_leds_link_act();
#endif
		if (inv_led) {
			set_led_inv();
		}
#if defined(CONFIG_NVT_IVOT_PLAT_NA51055) || defined(CONFIG_NVT_IVOT_PLAT_NA51089) || defined(CONFIG_NVT_IVOT_PLAT_NA51103)
		usleep_range(10000, 20000);
#endif
		phy_sw_reset_disable();
		udelay(50);
		g_resumed = 1;
	} else {
		printk("%s: NVT_PHY_BASE is NULL resume = %d\r\n", __func__, g_resumed);
	}

genphy_config_an:
	genphy_config_aneg(phydev);

//	mutex_unlock(&phydev->lock);

	return 0;
}

static int nvt_suspend(struct phy_device *phydev)
{
	printk("%s: enter\r\n", __func__);

	mutex_lock(&phydev->lock);
	bplug = 0;
	mutex_unlock(&phydev->lock);

	if (nvt_get_suspend_mode() || g_phy_power_down) {
		eth_phy_poweroff();
		g_resumed = 0;
	}

	return 0;
//	return genphy_suspend(phydev);
}

static void nvt_link_change_notify(struct phy_device *phydev)
{
	int status, phy_link;
	int ret;
#ifdef NVT_PHY_AN_FAIL_10M_TO_100M
	static unsigned long crc_count = 0, crc_count_old = 0;
	static unsigned long reg_old, tx_reg_old;
	unsigned long reg = 0, tx_reg = 0;
#endif
	status = phy_read(phydev, MII_BMSR);
	if (status < 0) {
		printk("%s phy_read fail : 0x%x\r\n", __func__, status);
		return;
	}

	if ((status & BMSR_LSTATUS) == 0)
		phy_link = 0;
	else
		phy_link = 1;

#ifdef NVT_PHY_AN_FAIL_10M_TO_100M
	if (phy_link) {
		tx_reg = ioread32((void *)(NVT_PHY_BASE + 0xA04));
		reg = ioread32((void *)(NVT_PHY_BASE + 0xA10));
		if (reg != reg_old)
			crc_count += reg;

		reg_old = reg;
		if ((crc_count - crc_count_old > 100) && (tx_reg == tx_reg_old)) {
			ret = nvt_soft_reset(phydev);
			if (ret < 0)
				printk("reset fail\r\n");
		}
		crc_count_old = crc_count;
		tx_reg_old = tx_reg;
	}
#endif

	if (((na510xx_phy_state != PHY_HALTED) && (phydev->state == PHY_HALTED)) ||
		((phydev->state == PHY_NOLINK) && (!phy_link))) {
		printk("%s remove\r\n", __func__);
		bplug = 0;
		if (NVT_PHY_BASE != NULL) {
			eq_reset_enable();
			msleep(50);
			eq_reset_disable();
		}
	} else if ((bplug == 0) && (phydev->state == PHY_RUNNING)) {
		printk("%s plug\r\n", __func__);
		ret = phy_write(phydev, 0x1F, 0xC00);
		if (ret < 0) {
			printk("phy write fail, ret = %x\r\n", ret);
			return;
		}

		bplug = 1;
	}
	na510xx_phy_state = phydev->state;
}

static int nvt_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int ret;
#ifdef CONFIG_OF
	struct device_node *np;
	const __be32 *cell;
#endif
	phys_addr_t eth0_pa = 0, eth1_pa = 0;

	printk("%s: enter\r\n", __func__);

	if (NVT_PHY_BASE != NULL) {
		printk("Warning: emb phy already on, please check pinmux setting\r\n");
		return 0;
	}

#ifdef CONFIG_OF
	np = of_find_compatible_node(NULL, NULL, "nvt,synopsys_eth");
	if (np) {
		cell = of_get_property(np, "reg", NULL);
		if (cell) {
			eth0_pa = of_read_number(cell, of_n_addr_cells(np));
//			printk("%s: reg = 0x%llx\r\n", __func__, eth0_pa);
		} else {
			printk("%s: Get eth0 reg fail", __func__);
			return -1;
		}
	} else {
		printk("%s: Not get eth0 dts node", __func__);
	}

	np = of_find_compatible_node(np, NULL, "nvt,synopsys_eth");
	if (np) {
		cell = of_get_property(np, "reg", NULL);
		if (cell) {
			eth1_pa = of_read_number(cell, of_n_addr_cells(np));
//			printk("%s: reg = 0x%llx\r\n", __func__, eth1_pa);
		} else {
			printk("%s: Get eth1 reg fail", __func__);
			return -1;
		}
	} else {
		printk("%s: Not get eth1 dts node", __func__);
	}

#endif
	pinmux_config[0].pin_function = PIN_FUNC_ETH;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		printk("%s: get pinmux config failed\n", __func__);

	printk("%s: get pinmux 0x%x\r\n", __func__, pinmux_config[0].config);

#if defined(CONFIG_NVT_IVOT_PLAT_NA51055) || defined(CONFIG_NVT_IVOT_PLAT_NA51089)

	if ((pinmux_config[0].config & PIN_ETH_CFG_INTERANL)) {
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	if (!(pinmux_config[0].config &
		(PIN_ETH_CFG_ETH_RMII_1 | PIN_ETH_CFG_ETH_RMII_2 | PIN_ETH_CFG_ETH_RGMII))) {
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02301)
	if (!(pinmux_config[0].config & PIN_ETH_CFG_ETH_RMII_1)) {
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51103)
	if (!(pinmux_config[0].config & (PIN_ETH_CFG_RMII_1ST_PINMUX | PIN_ETH_CFG_RGMII_1ST_PINMUX))) {
#endif
		if (!eth0_pa) {
			printk("%s: Get eth0 base addr error", __func__);
			return -1;
		}

		NVT_PHY_BASE = ioremap(eth0_pa + ETH_TO_PHY_OFFSET, PHY_BASE_SIZE);
		printk("NVT EMB phy route to MAC0\r\n");
	}
	else {
		if (!eth1_pa) {
			printk("%s: Get eth1 base addr error", __func__);
			return -1;
		}

		NVT_PHY_BASE = ioremap(eth1_pa + ETH_TO_PHY_OFFSET, PHY_BASE_SIZE);
		printk("NVT EMB phy route to MAC1\r\n");
	}

	if (!NVT_PHY_BASE) {
		printk("Fail to map memory\r\n");
		return -1;
	}

	ret = nvt_phy_sysfs_init(dev);
	if (ret) {
		printk("%s: sysfs init fail ret = %d\r\n", __func__, ret);
	}

	printk("Get remap addr 0x%px\r\n", NVT_PHY_BASE);

	return 0;
}

static void nvt_remove(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	printk("%s: enter\r\n", __func__);

	eth_phy_poweroff();
	nvt_phy_sysfs_remove(dev);
	iounmap(NVT_PHY_BASE);
	NVT_PHY_BASE = NULL;
}

static struct phy_driver novatek_drv[] = {
	{
		.phy_id             = 0x00000001,
		.name               = "Novatek Fast Ethernet Phy",
		.phy_id_mask        = 0x001fffff,
		.features           = PHY_BASIC_FEATURES,
		.probe              = nvt_probe,
//		.get_features	    = genphy_read_abilities,
		.remove             = nvt_remove,
		.soft_reset         = nvt_soft_reset,
		.config_init        = nvt_config_init,
		.config_aneg        = nvt_config_aneg,
		.read_status        = genphy_read_status,
		.suspend            = nvt_suspend,
		.resume	            = nvt_resume,
		.link_change_notify	= nvt_link_change_notify,
	},
};

module_phy_driver(novatek_drv);

static struct mdio_device_id __maybe_unused novatek_tbl[] = {
	{ 0x00000001, 0x001fffff },
	{ }
};

MODULE_DEVICE_TABLE(mdio, novatek_tbl);

MODULE_DESCRIPTION("Novatek Ethernet PHY Driver");
MODULE_VERSION("1.01.013");
MODULE_LICENSE("GPL");
