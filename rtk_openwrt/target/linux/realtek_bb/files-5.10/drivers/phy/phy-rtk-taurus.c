/*
 * PHY driver for Realtek Taurus SoCs
 *
 * Based on phy-ca77xx.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <dt-bindings/phy/phy-rtl8277c.h>

//GLOBAL_PHY_CONTROL
#define USB2_LPM_PLL_ALIVE       BIT(4)
#define USB2_PORT_1_CLOSE        BIT(5)
#define USB2_REF_SEL_0           BIT(8)
#define USB2_REF_SEL_1           BIT(9)
#define USB2_REF_SEL_2           BIT(10)
#define USB2_REF_SEL_3           BIT(11)
#define SYS_PCIE2_AND_S2_SEL     BIT(18)
#define SYS_USB3_AND_S2_SEL      BIT(19)
#define S2_PCIE_USB3_SEL         BIT(20)
#define S2_PCIE_USB3_SGMII_SEL_0 BIT(21)
#define S2_PCIE_USB3_SGMII_SEL_1 BIT(22)
#define S3_PCIE_USB3_SEL         BIT(24)
#define S3_PCIE_USB3_SGMII_SEL_0 BIT(25)
#define S3_PCIE_USB3_SGMII_SEL_1 BIT(26)
#define S0_RXAUI_MODE            BIT(27)
#define S0_P_MDIO_ENABLE_REG	 BIT(28)
#define S1_P_MDIO_ENABLE_REG	 BIT(29)
#define S2_P_MDIO_ENABLE_REG	 BIT(30)
#define S3_P_MDIO_ENABLE_REG	 BIT(31)

//GLOBAL_PHY_MISC_CONTROL
#define S2_USB3_BG_EN           BIT(8)
#define S2_USB3_MBIAS_EN        BIT(9)
#define S3_USB3_BG_EN           BIT(10)
#define S3_USB3_MBIAS_EN        BIT(11)
#define S2_USB3_OOBS_PDB        BIT(30)
#define S3_USB3_OOBS_PDB        BIT(31)

//GLOBAL_PHY_STATUS
#define STATUS_USB2_CKUSABLE    GENMASK(1, 0)
#define STATUS_S2_CKUSABLE      BIT(3)
#define STATUS_S3_CKUSABLE      BIT(4)
#define STATUS_S0_CKUSABLE      BIT(5)
#define STATUS_S1_CKUSABLE      BIT(6)
#define STATUS_CKUSABLE_SGMII   BIT(7)

//GLOBAL_PHY_SGMII_MISC_CONTROL
#define PHY_SGMII_MISC_CONTROL  0xC
#define S3_SGMII_CKBUF_EN       BIT(0)
#define S3_SGMII_CLK_MODE_SEL_0 BIT(1)
#define S3_SGMII_CLK_MODE_SEL_1 BIT(2)
#define S3_SGMII_BG_EN          BIT(3)
#define S3_SGMII_MBIAS_EN       BIT(4)
#define S3_SGMII_RX50_LINK      BIT(5)
#define S3_SGMII_TX_DEEMP_SEL   BIT(7)
#define S3_SGMII_EQHOLD         BIT(8)

//GLOBAL_PHY_SGMII_PCS_CONTROL
#define PHY_SGMII_PCS_CONTROL   0x10

//GLOBAL_PHY_ISO_POWER_CONTROL
#define S0_PCIE2_ISOLATE_0      BIT(0)
#define S0_PCIE2_ISOLATE_1      BIT(1)
#define S1_PCIE2_ISOLATE_0      BIT(2)
#define S1_PCIE2_ISOLATE_1      BIT(3)
#define S2_CPHY_ISOLATE         BIT(4)
#define S3_CPHY_ISOLATE         BIT(5)
#define S0_POW_PCIE2_0          BIT(7)
#define S0_POW_PCIE2_1          BIT(8)
#define S1_POW_PCIE2_0          BIT(9)
#define S1_POW_PCIE2_1          BIT(10)
#define S2_POW_USB3             BIT(11)
#define S2_POW_PCIE2            BIT(12)
#define S3_POW_USB3             BIT(13)
#define S3_POW_PCIE2            BIT(14)
#define S0_ISO_ANA_B            BIT(16)
#define S1_ISO_ANA_B            BIT(17)
#define S2_ISO_ANA_B            BIT(18)
#define S3_ISO_ANA_B            BIT(19)
#define S0_LANE0_CA09PC_EN      BIT(20)
#define S0_LANE0_CA33PC_EN      BIT(21)
#define S0_LANE1_CA09PC_EN      BIT(22)
#define S0_LANE1_CA33PC_EN      BIT(23)
#define S1_LANE0_CA09PC_EN      BIT(24)
#define S1_LANE0_CA33PC_EN      BIT(25)
#define S1_LANE1_CA09PC_EN      BIT(26)
#define S1_LANE1_CA33PC_EN      BIT(27)
#define S2_CA09PC_EN            BIT(28)
#define S2_CA33PC_EN            BIT(29)
#define S3_CA09PC_EN            BIT(30)
#define S3_CA33PC_EN            BIT(31)

static int debug;
struct rtk_taurus_phy_core;

struct serdes_cfg {
	u32 addr;
	u32 val;
};

/**
 * struct rtk_taurus_phy
 * @core: pointer to the PHY core control
 * @id: internal ID to identify the PHY
 * @phy: pointer to the kernel PHY device
 */
struct rtk_taurus_phy {
	struct rtk_taurus_phy_core *core;
	struct phy *phy;
	char *dts_name;
	phys_addr_t serdes_addr, serdes_size;
	void __iomem *serdes_base;
	struct serdes_cfg *cfg;
	int cfg_cnt;
	int id;
};

/**
 * struct rtk_taurus_phy_core - PHY core control
 * @dev: pointer to device
 * @reg_ctrl: control register reg_ctrl
 * @reg_pwr: iso power control register reg_ctrl
 * @lock: mutex to protect access to individual PHYs
 * @phys: pointer to PHY device
 */
struct rtk_taurus_phy_core {
	struct device *dev;
	void __iomem *reg_ctrl;
	void __iomem *reg_pwr;
	struct mutex lock;
	struct rtk_taurus_phy phys[PHY_NUMS];
};

static int rtk_taurus_phy_serdes_config(struct rtk_taurus_phy *phy, bool enable)
{
	struct device *dev = phy->core->dev;
	u32 off, val;
	int i;

	if (!phy->serdes_addr || !phy->serdes_size)
		return -ENXIO;

	if (!phy->serdes_base) {
		if (!devm_request_mem_region(dev, phy->serdes_addr,
					     phy->serdes_size, phy->dts_name)) {
			dev_err(dev, "can't request region\n");
			phy->serdes_base = NULL;
		} else {
			phy->serdes_base = devm_ioremap(dev, phy->serdes_addr,
							phy->serdes_size);
			if (!phy->serdes_base) {
				dev_err(dev, "ioremap failed for resource\n");
				devm_release_mem_region(dev, phy->serdes_addr,
							phy->serdes_size);
			}
		}

		if (!phy->serdes_base)
			return -ENXIO;
	}

	if (!enable)
		return 0;

	if (phy->cfg_cnt > 0 && phy->cfg) {
		for (i = 0; i < phy->cfg_cnt; i++) {
			off = (phys_addr_t)phy->cfg[i].addr - phy->serdes_addr;
			val = phy->cfg[i].val;

			if (off >= 0 && off < phy->serdes_size)
				writel(val, phy->serdes_base + off);
		}

		return 0;
	}

	switch (phy->id) {
		case PHY_S0_PCIE0:
			/* PCIe phy parameters migrate to pcie-ca77xx.c */
			break;
		case PHY_S0_PCIE1:
			/* PCIe phy parameters migrate to pcie-ca77xx.c */
			break;
		case PHY_S1_PCIE0:
			/* PCIe phy parameters migrate to pcie-ca77xx.c */
			break;
		case PHY_S2_PCIE0:
			/* PCIe phy parameters migrate to pcie-ca77xx.c */
			break;
		case PHY_S2_USB:
			/* USB phy parameters migrate to phy_ca77xx_usb3.c */
			break;
		case PHY_S3_USB:
			/* USB phy parameters migrate to phy_ca77xx_usb3.c */
			break;

		case PHY_S3_SGMII:
			writel(0x5359, phy->serdes_base + 0x001c);
			break;
	}

	return 0;
}

static int rtk_taurus_phy_sgmii_config(struct rtk_taurus_phy *phy, bool enable)
{
	struct rtk_taurus_phy_core *core = phy->core;

	if (!enable)
		return 0;

	switch (phy->id) {
		case PHY_S3_SGMII:
			writel(0x00000030, core->reg_ctrl + PHY_SGMII_MISC_CONTROL);
			writel(0x34200248, core->reg_ctrl + PHY_SGMII_PCS_CONTROL);
			break;
	}

	return 0;
}

static int rtk_taurus_phy_power_config(struct rtk_taurus_phy *phy, bool enable)
{
	struct rtk_taurus_phy_core *core = phy->core;
	u32 ctrl_en_set_m, ctrl_en_clr_m;
	u32 pwr_en_set_m, pwr_en_clr_m;
	u32 ctrl_v, pwr_v;

	switch (phy->id) {
	case PHY_S0_PCIE0:
		ctrl_en_set_m = 0;
		ctrl_en_clr_m = S0_RXAUI_MODE;
		pwr_en_set_m  = S0_POW_PCIE2_0 | S0_ISO_ANA_B | S0_LANE0_CA09PC_EN | S0_LANE0_CA33PC_EN;
		pwr_en_clr_m  = S0_PCIE2_ISOLATE_0;
		break;
	case PHY_S0_PCIE1:
		//assume lane 1 will not be on alone, so don't care about S0_RXAUI_MODE and S0_ISO_ANA_B
		ctrl_en_set_m = 0;
		ctrl_en_clr_m = 0;
		pwr_en_set_m  = S0_POW_PCIE2_1 | S0_LANE1_CA09PC_EN | S0_LANE1_CA33PC_EN;
		pwr_en_clr_m  = S0_PCIE2_ISOLATE_1;
		break;
	case PHY_S0_RXAUI:
		ctrl_en_set_m = S0_RXAUI_MODE;
		ctrl_en_clr_m = 0;
		//lane 0
		pwr_en_set_m  = S0_POW_PCIE2_0 | S0_ISO_ANA_B | S0_LANE0_CA09PC_EN | S0_LANE0_CA33PC_EN;
		pwr_en_clr_m  = S0_PCIE2_ISOLATE_0;
		//lane 1
		pwr_en_set_m  |= S0_POW_PCIE2_1 | S0_LANE1_CA09PC_EN | S0_LANE1_CA33PC_EN;
		pwr_en_clr_m  |= S0_PCIE2_ISOLATE_1;
		break;
	case PHY_S1_PCIE0:
		ctrl_en_set_m = 0;
		ctrl_en_clr_m = 0;
		pwr_en_set_m  = S1_POW_PCIE2_0 | S1_ISO_ANA_B | S1_LANE0_CA09PC_EN | S1_LANE0_CA33PC_EN;
		pwr_en_clr_m  = S1_PCIE2_ISOLATE_0;
		break;
	case PHY_S1_PCIE1:
		//assume lane 1 will not be on alone, so don't care about S1_ISO_ANA_B
		ctrl_en_set_m = 0;
		ctrl_en_clr_m = 0;
		pwr_en_set_m  = S1_POW_PCIE2_1 | S1_LANE1_CA09PC_EN | S1_LANE1_CA33PC_EN;
		pwr_en_clr_m  = S1_PCIE2_ISOLATE_1;
		break;
	case PHY_S2_PCIE0:
		ctrl_en_set_m = SYS_PCIE2_AND_S2_SEL;
		ctrl_en_clr_m = S2_PCIE_USB3_SEL | S2_PCIE_USB3_SGMII_SEL_0 | S2_PCIE_USB3_SGMII_SEL_1;
		pwr_en_set_m  = S2_POW_PCIE2 | S2_ISO_ANA_B | S2_CA09PC_EN | S2_CA33PC_EN;
		pwr_en_clr_m  = S2_CPHY_ISOLATE | S2_POW_USB3;
		break;
	case PHY_S2_USB:
		ctrl_en_set_m = S2_PCIE_USB3_SEL | SYS_USB3_AND_S2_SEL;
		ctrl_en_clr_m = S2_PCIE_USB3_SGMII_SEL_0 | S2_PCIE_USB3_SGMII_SEL_1;
		pwr_en_set_m  = S2_POW_USB3 | S2_ISO_ANA_B | S2_CA09PC_EN | S2_CA33PC_EN;
		pwr_en_clr_m  = S2_CPHY_ISOLATE | S2_POW_PCIE2;
		break;
	case PHY_S3_USB:
		ctrl_en_set_m = S3_PCIE_USB3_SEL;
		ctrl_en_clr_m = S3_PCIE_USB3_SGMII_SEL_0 | S3_PCIE_USB3_SGMII_SEL_1 | SYS_USB3_AND_S2_SEL;
		pwr_en_set_m  = S3_POW_USB3 | S3_ISO_ANA_B | S3_CA09PC_EN | S3_CA33PC_EN;
		pwr_en_clr_m  = S3_CPHY_ISOLATE | S3_POW_PCIE2;
		break;
	case PHY_S3_SGMII:
		ctrl_en_set_m = S3_PCIE_USB3_SGMII_SEL_0;
		ctrl_en_clr_m = S3_PCIE_USB3_SGMII_SEL_1;
		pwr_en_set_m  = S3_ISO_ANA_B | S3_CA09PC_EN | S3_CA33PC_EN;
		pwr_en_clr_m  = S3_CPHY_ISOLATE | S3_POW_PCIE2 | S3_POW_USB3;
		break;
	case PHY_S3_HSGMII:
		ctrl_en_set_m = S3_PCIE_USB3_SGMII_SEL_1;
		ctrl_en_clr_m = S3_PCIE_USB3_SGMII_SEL_0;
		pwr_en_set_m  = S3_ISO_ANA_B | S3_CA09PC_EN | S3_CA33PC_EN;
		pwr_en_clr_m  = S3_CPHY_ISOLATE | S3_POW_PCIE2 | S3_POW_USB3;
		break;
	default:
		dev_err(core->dev, "PHY %d invalid\n", phy->id);
		return -EINVAL;
	}

	mutex_lock(&core->lock);

	ctrl_v = readl(core->reg_ctrl);
	pwr_v = readl(core->reg_pwr);

	if (debug) {
		dev_info(core->dev, "%s - GLOBAL_PHY_CONTROL val 0x%X\n", __func__, ctrl_v);
		dev_info(core->dev, "%s - GLOBAL_PHY_ISO_POWER_CONTROL val 0x%X\n", __func__, pwr_v);
	}

	if (enable) {
		ctrl_v |= ctrl_en_set_m;
		ctrl_v &= ~ctrl_en_clr_m;
		pwr_v |= pwr_en_set_m;
		pwr_v &= ~pwr_en_clr_m;
	} else {
		/* ctrl_v &= ~ctrl_en_set_m;
		ctrl_v |= ctrl_en_clr_m; */
		pwr_v &= ~pwr_en_set_m;
		pwr_v |= pwr_en_clr_m;
	}

	if (debug) {
		dev_info(core->dev, "%s - write 0x%X to GLOBAL_PHY_CONTROL\n", __func__, ctrl_v);
		dev_info(core->dev, "%s - write 0x%X to GLOBAL_PHY_ISO_POWER_CONTROL\n", __func__, pwr_v);
	}

	writel(ctrl_v, core->reg_ctrl);
	writel(pwr_v, core->reg_pwr);

	mutex_unlock(&core->lock);
	dev_dbg(core->dev, "PHY %d %s\n", phy->id,
		enable ? "enabled" : "disabled");

	rtk_taurus_phy_serdes_config(phy, enable);
	rtk_taurus_phy_sgmii_config(phy, enable);

	return 0;
}

static int rtk_taurus_phy_power_on(struct phy *p)
{
	struct rtk_taurus_phy *phy = phy_get_drvdata(p);

	if (debug)
		dev_info(phy->core->dev, "Request to power_on phy(%d)\n",
			 phy->id);

	return rtk_taurus_phy_power_config(phy, true);
}

static int rtk_taurus_phy_power_off(struct phy *p)
{
	struct rtk_taurus_phy *phy = phy_get_drvdata(p);

	if (debug)
		dev_info(phy->core->dev, "Request to power_off phy(%d)\n",
			 phy->id);

	return rtk_taurus_phy_power_config(phy, false);
}

static struct phy_ops rtk_taurus_phy_ops = {
	.init = NULL,
	.exit = NULL,
	.power_on = rtk_taurus_phy_power_on,
	.power_off = rtk_taurus_phy_power_off,
	.owner = THIS_MODULE,
};

static void rtk_taurus_serdes_probe(struct device *dev, struct device_node *np,
				struct rtk_taurus_phy *p)
{
	phys_addr_t serdes[2];
	int ret, size, cnt;

#ifdef CONFIG_PHYS_ADDR_T_64BIT
	ret = of_property_read_u64_array(np, "serdes-reg", serdes, 2);
#else
	ret = of_property_read_u32_array(np, "serdes-reg", serdes, 2);
#endif
	if (ret) {
		p->dts_name = NULL;
		p->serdes_addr = 0;
		p->serdes_size = 0;
		p->serdes_base = NULL;

		return;
	}

	p->dts_name = (char *)np->name;
	p->serdes_addr = serdes[0];
	p->serdes_size = serdes[1];
	p->serdes_base = NULL;

	size = sizeof(struct serdes_cfg);
	p->cfg_cnt = of_property_count_elems_of_size(np, "serdes-cfg", size);
	if (p->cfg_cnt > 0) {
		p->cfg = devm_kmalloc_array(dev, p->cfg_cnt, size, GFP_KERNEL);

		cnt = p->cfg_cnt * sizeof(struct serdes_cfg) / sizeof(u32);
		of_property_read_u32_array(np, "serdes-cfg", (u32 *)p->cfg,
					   cnt);
	} else {
		p->cfg_cnt = 0;
		p->cfg = NULL;
	}
}

static int rtk_taurus_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node, *child;
	struct rtk_taurus_phy_core *core;
	struct phy_provider *provider;
	struct resource *res;
	unsigned cnt = 0;
	int ret;

	if (of_get_child_count(node) == 0) {
		dev_err(dev, "PHY no child node\n");
		return -ENODEV;
	}

	core = devm_kzalloc(dev, sizeof(*core), GFP_KERNEL);
	if (!core)
		return -ENOMEM;

	core->dev = dev;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "control");
	core->reg_ctrl = devm_ioremap_resource(dev, res);
	if (IS_ERR(core->reg_ctrl))
		return PTR_ERR(core->reg_ctrl);
	dev_info(dev, "resource - %pr mapped at 0x%pK\n", res, core->reg_ctrl);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "power");
	core->reg_pwr = devm_ioremap_resource(dev, res);
	if (IS_ERR(core->reg_pwr))
		return PTR_ERR(core->reg_pwr);
	dev_info(dev, "resource - %pr mapped at 0x%pK\n", res, core->reg_pwr);

	mutex_init(&core->lock);

	for_each_available_child_of_node(node, child) {
		unsigned int id;
		struct rtk_taurus_phy *p;

		if (of_property_read_u32(child, "reg", &id)) {
			dev_err(dev, "missing reg property for %s\n",
				child->name);
			ret = -EINVAL;
			goto put_child;
		}

		if (id >= PHY_NUMS) {
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
		p->phy = devm_phy_create(dev, child, &rtk_taurus_phy_ops);
		if (IS_ERR(p->phy)) {
			dev_err(dev, "failed to create PHY\n");
			ret = PTR_ERR(p->phy);
			goto put_child;
		}

		p->core = core;
		p->id = id;

		rtk_taurus_serdes_probe(dev, child, p);

		phy_set_drvdata(p->phy, p);
		cnt++;
	}

	dev_set_drvdata(dev, core);

	provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	if (IS_ERR(provider)) {
		dev_err(dev, "failed to register PHY provider\n");
		return PTR_ERR(provider);
	}
	dev_dbg(dev, "registered %u PHY(s)\n", cnt);

	return 0;
put_child:
	of_node_put(child);
	return ret;
}

static const struct of_device_id rtk_taurus_phy_match_table[] = {
	{ .compatible = "realtek,taurus-phy" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtk_taurus_phy_match_table);

static struct platform_driver rtk_taurus_phy_driver = {
	.driver = {
		.name = "rtk-taurus-phy",
		.of_match_table = rtk_taurus_phy_match_table,
	},
	.probe = rtk_taurus_phy_probe,
};
module_platform_driver(rtk_taurus_phy_driver);

MODULE_DESCRIPTION("Realtek Taurus PHY driver");
MODULE_LICENSE("GPL v2");
