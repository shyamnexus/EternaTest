// SPDX-License-Identifier: GPL-2.0+
/*
 * Maxlinear GPY211 PHY drivers
 *
 * Copyright (c) 2024, NOVATEK MICROELECTRONIC CORPORATION. All rights reserved.
 *
 */
#include <common.h>
#include <linux/bitops.h>
#include <phy.h>
#include <linux/delay.h>
#include <linux/bitfield.h>

#define PHY_MIISTAT     0x18    /* MII state */
#define PHY_IMASK       0x19    /* interrupt mask */
#define PHY_ISTAT       0x1A    /* interrupt status */
#define PHY_FWV         0x1E    /* firmware version */

#define PHY_MIISTAT_SPD_MASK    GENMASK(2, 0)
#define PHY_MIISTAT_DPX     BIT(3)
#define PHY_MIISTAT_LS      BIT(10)

#define PHY_MIISTAT_SPD_10  0
#define PHY_MIISTAT_SPD_100 1
#define PHY_MIISTAT_SPD_1000    2
#define PHY_MIISTAT_SPD_2500    4



static int gpy_startup(struct phy_device *phydev)
{
	int ret, val;

	ret = genphy_update_link(phydev);
	if (ret) {
		return ret;
	}

	ret = genphy_parse_link(phydev);
	if (ret) {
		return ret;
	}

	val = phy_read(phydev, MDIO_DEVAD_NONE, PHY_MIISTAT);
	if (val < 0) {
		return val;
	}

	phydev->link = (val & PHY_MIISTAT_LS) ? 1 : 0;
	phydev->duplex = (val & PHY_MIISTAT_DPX) ? DUPLEX_FULL : DUPLEX_HALF;
	switch (FIELD_GET(PHY_MIISTAT_SPD_MASK, val)) {
	case PHY_MIISTAT_SPD_10:
		phydev->speed = SPEED_10;
		break;
	case PHY_MIISTAT_SPD_100:
		phydev->speed = SPEED_100;
		break;
	case PHY_MIISTAT_SPD_1000:
		phydev->speed = SPEED_1000;
		break;
	case PHY_MIISTAT_SPD_2500:
		phydev->speed = SPEED_2500;
		break;
	}

	return 0;
}

static int gpy_probe(struct phy_device *phydev)
{
	return 0;
}

static int gpy_config(struct phy_device *phydev)
{
	genphy_config_aneg(phydev);

	return 0;
}

/* Support for GPY211 PHY */
static struct phy_driver GPY211_driver = {
	.name = "Maxlinear GPY211 2.5Gbps PHY",
	.uid = 0x67c9de10,
	.mask = 0xffffffff,
	.features = PHY_GBIT_FEATURES,
	.probe = &gpy_probe,
	.config = &gpy_config,
	.startup = &gpy_startup,
	.shutdown = &genphy_shutdown,
};

/* Support for GPY215 PHY */
static struct phy_driver GPY215_driver = {
	.name = "Maxlinear GPY215 2.5Gbps PHY",
	.uid = 0x67c9df20,
	.mask = 0xffffffff,
	.features = PHY_GBIT_FEATURES,
	.probe = &gpy_probe,
	.config = &gpy_config,
	.startup = &gpy_startup,
	.shutdown = &genphy_shutdown,
};

int phy_maxlinear_init(void)
{
	phy_register(&GPY211_driver);
	phy_register(&GPY215_driver);

	return 0;
}
