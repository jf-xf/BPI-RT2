/*
 * TAURUS USB3.0 PHY Device Driver for dwc3 core controller on
 * Realtek TAURUS SoCs
 *
 * Based on phy-ca77xx-usb3.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/usb/otg.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/usb/ch11.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/reset-controller.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/reset.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <soc/cortina/cortina-soc.h>

#define TAURUS_USB3PHY_NAME "taurus-usb3phy"
#define TAURUS_USB3PHY_VER_UNKNOWN "unknown"

/* USB3(USB Super Speed) register offsets */
#define USB3CFG_CNTRL_OFFSET				0x00
#define USB3CFG_PHY_VAUX_RESET					BIT(31)
#define USB3CFG_BUS_CLKEN_GSLAVE				BIT(30)
#define USB3CFG_BUS_CLKEN_GMASTER				BIT(29)
#define USB3CFG_BIGENDIAN_GSLAVE				BIT(28)
#define USB3CFG_HOST_PORT_POWER_CTRL_PRESENT	BIT(27)
#define USB3CFG_HOST_MSI_ENABLE					BIT(26)
#define USB3CFG_HOST_LEGACY_SMI_PCI_CMD_REG_WR	BIT(25)
#define USB3CFG_HOST_LEGACY_SMI_BAR_WR			BIT(24)
#define USB3CFG_FLADJ_30MHZ_REG_MASK(x)			((0x3f & (x)) << 16)
#define USB3CFG_PHY_VAUX_RESET_U2_PORT1			BIT(9)
#define USB3CFG_PHY_VAUX_RESET_U2_PORT0			BIT(8)
#define USB3CFG_XHCI_BUS_MASTER_ENABLE			BIT(4)
#define USB3CFG_BUS_FILTER_BYPASS_MASK(x)		(0xf & (x))

#define USB3CFG_STATUS_OFFSET				0x04
#define USB3CFG_HOST_SYSTEM_ERR					BIT(31)
#define USB3CFG_LEGACY_SMI_INTERRUPT			BIT(16)
#define USB3CFG_HOST_CURRENT_BELT_MASK(x)		(0xfff & (x))

#define USB3CFG_PORT_CONFIG_OFFSET			0x08
#define USB3CFG_U3P1_HOST_DISABLE				BIT(26)
#define USB3CFG_U3P1_HUB_PORT_OVERCURRENT		BIT(25)
#define USB3CFG_U3P1_HUB_PORT_PERM_ATTACH		BIT(24)
#define USB3CFG_U3P0_HOST_DISABLE				BIT(18)
#define USB3CFG_U3P0_HUB_PORT_OVERCURRENT		BIT(17)
#define USB3CFG_U3P0_HUB_PORT_PERM_ATTACH		BIT(16)
#define USB3CFG_U2P1_HOST_DISABLE				BIT(10)
#define USB3CFG_U2P1_HUB_PORT_OVERCURRENT		BIT(9)
#define USB3CFG_U2P1_HUB_PORT_PERM_ATTACH		BIT(8)
#define USB3CFG_U2P0_HOST_DISABLE				BIT(2)
#define USB3CFG_U2P0_HUB_PORT_OVERCURRENT		BIT(1)
#define USB3CFG_U2P0_HUB_PORT_PERM_ATTACH		BIT(0)

#define USB3CFG_PORT_STATUS_OFFSET			0x0c
#define USB3CFG_U3P1_PIPE3_PHY_MODE_MASK(x)		((0x3 & (x)) << 28)
#define USB3CFG_U3P1_HUB_VBUS_CTRL				BIT(24)
#define USB3CFG_U3P0_PIPE3_PHY_MODE_MASK(x)		((0x3 & (x)) << 20)
#define USB3CFG_U3P0_HUB_VBUS_CTRL				BIT(16)
#define USB3CFG_U2P1_UTMI_FSLS_LOW_POWER		BIT(9)
#define USB3CFG_U2P1_HUB_VBUS_CTRL				BIT(8)
#define USB3CFG_U2P0_UTMI_FSLS_LOW_POWER		BIT(1)
#define USB3CFG_U2P0_HUB_VBUS_CTRL				BIT(0)

#define USB3CFG_PHY_PORT0_CONFIG0_OFFSET	0x10
#define USB3CFG_PHY_PORT1_CONFIG0_OFFSET	0x1c
#define USB3CFG_PHY_PORT_CKBUF_EN					BIT(31)
#define USB3CFG_PHY_PORT_MAC_PHY_PLL_EN_REG			BIT(30)
#define USB3CFG_PHY_PORT_MAC_PHY_RECV_DET_EN_REG	BIT(29)
#define USB3CFG_PHY_PORT_MAC_PHY_RECV_DET_REQ_REG	BIT(28)
#define USB3CFG_PHY_PORT_CLK_MODE_SEL_MASK(x)		(0x3 & (x))

#define USB3CFG_PHY_PORT0_STATUS0_OFFSET	0x14
#define USB3CFG_PHY_PORT1_STATUS0_OFFSET	0x20
#define USB3CFG_PHY_PORT_PHY_MAC_RECV_DET_END		BIT(21)
#define USB3CFG_PHY_PORT_PHY_MAC_RECV_DET_ACK		BIT(20)
#define USB3CFG_PHY_PORT_CLK_RDY					BIT(19)
#define USB3CFG_PHY_PORT_COUNT_NUM_VAL(x)			(0x7ffff & (x))

#define USB3CFG_PHY_PORT0_STATUS1_OFFSET	0x18
#define USB3CFG_PHY_PORT1_STATUS1_OFFSET	0x24
#define USB3CFG_PHY_PORT_DISPARITY_ERR_MASK(x)		((0xff & (x)) << 24)
#define USB3CFG_PHY_PORT_ELASTIC_BUF_UDF_MASK(x)	((0xff & (x)) << 16)
#define USB3CFG_PHY_PORT_ELASTIC_BUF_OVF_MASK(x)	((0xff & (x)) << 8)
#define USB3CFG_PHY_PORT_DECODE_ERR_MASK(x)			(0xff & (x))

#define USB3CFG_EXT_CTRL_OFFSET				0x00
#define USB3CFG_RX50_TERM_SEL				BIT(28)

struct phy_data {
	int size;
	u8 *addr;
	u16 *data;
	const char *ver;
};

struct taurus_usb3_phy {
	struct usb_phy	phy;
	struct usb_phy	*u2phy;
	struct device	*dev;
	void __iomem	*phy_regbase;
	void __iomem	*ext_regbase;
	int				 phy_mask;
	struct gpio_desc		*vbus;
	struct phy_data *phy_data;
	void __iomem	*serdes_regbase;
	struct phy				*serdes_phy;
	spinlock_t		lock;	/* lock access to bank regs */
};

static int debug = 0;

static inline u32 taurus_usb3_phy_read(struct taurus_usb3_phy *taurus_phy,
                                       u8 addr)
{
	u32 data = 0;
	u32 offset = addr << 2;

	data = readl(taurus_phy->serdes_regbase + offset);

	if (debug) {
		dev_info(taurus_phy->dev, "Read offset=0x%04x, data=0x%08x\n",
		         offset, data);
	}

	return data;
}

static inline void taurus_usb3_phy_write(struct taurus_usb3_phy *taurus_phy,
        u8 addr, u16 data)
{
	u32 offset = addr << 2;

	writel(data, taurus_phy->serdes_regbase + offset);
}


static void taurus_usb3_host_reset(struct taurus_usb3_phy *taurus_phy)
{
	u32 reg_val = 0;
	unsigned long flags;

	if (debug)
		dev_info(taurus_phy->dev, "%s()\n", __func__);

	/* Serdes PHY Reset */
	phy_power_on(taurus_phy->serdes_phy);
	msleep(20);

	/* For USB3.0 & USB2.0 PHY RESET */
	spin_lock_irqsave(&taurus_phy->lock, flags);
	reg_val = readl(taurus_phy->phy_regbase + USB3CFG_CNTRL_OFFSET);
	reg_val |= (USB3CFG_PHY_VAUX_RESET | USB3CFG_PHY_VAUX_RESET_U2_PORT0 | USB3CFG_PHY_VAUX_RESET_U2_PORT1);
	writel(reg_val, taurus_phy->phy_regbase + USB3CFG_CNTRL_OFFSET);
	spin_unlock_irqrestore(&taurus_phy->lock, flags);
	if (debug) {
		dev_info(taurus_phy->dev, "read USB3CFG_CNTRL reg_val = 0x%08x",
		         readl(taurus_phy->phy_regbase + USB3CFG_CNTRL_OFFSET));
	}
}

static void taurus_usb3_rx_term_setup(struct taurus_usb3_phy *taurus_phy)
{
	u32 reg_val = 0;
	unsigned long flags;

	if (debug)
		dev_info(taurus_phy->dev, "%s()\n", __func__);

	/* For USB3.0 & USB2.0 PHY RESET */
	spin_lock_irqsave(&taurus_phy->lock, flags);
	reg_val = readl(taurus_phy->ext_regbase + USB3CFG_EXT_CTRL_OFFSET);
	reg_val &= ~USB3CFG_RX50_TERM_SEL;
	writel(reg_val, taurus_phy->ext_regbase + USB3CFG_EXT_CTRL_OFFSET);
	spin_unlock_irqrestore(&taurus_phy->lock, flags);

	if (debug) {
		dev_info(taurus_phy->dev, "read USB3CFG_EXT_CTRL reg_val = 0x%08x",
		         readl(taurus_phy->phy_regbase + USB3CFG_EXT_CTRL_OFFSET));
	}
}

__weak struct proc_dir_entry *realtek_proc;

static ssize_t taurus_usb3_phy_proc_write(struct file * file, const char __user * userbuf, size_t count, loff_t * off)
{
	pr_info("Use \"devmem 0xf433[serdes phy id]000 + addr*0x4 32 [value]\" to write PHY parameter\n");
	pr_info("ex: devmem 0xf43337004 32 0xac46\n");
	return count;
}


static int taurus_usb3_phy_show(struct seq_file *s, void *v)
{
	struct taurus_usb3_phy *taurus_phy = s->private;
	int addr;
	u32 data;

	seq_printf(s, "USB3 PHY version: %s\n\n", taurus_phy->phy_data->ver);

	seq_printf(s, "Page 0: \tPage 1:\n");
	for (addr = 0; addr <= 0x1f; addr++) {
		data = taurus_usb3_phy_read(taurus_phy, addr);
		if (addr <= 0x15) {
			seq_printf(s, "%02x: %04x\t", addr, data);
			data = taurus_usb3_phy_read(taurus_phy, addr+0x20);
			seq_printf(s, "%02x: %04x\n", addr, data);
		} else
			seq_printf(s, "%02x: %04x\n", addr, data);
	}

	return 0;
}

static int taurus_usb3_phy_open(struct inode *inode, struct file *file)
{
	return(single_open(file, taurus_usb3_phy_show, PDE_DATA(inode)));
}


static const struct proc_ops proc_ops_taurus_usb3_phy = {
	.proc_open  		= taurus_usb3_phy_open,
	.proc_write 		= taurus_usb3_phy_proc_write,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};


static int taurus_usb3_phy_init(struct usb_phy *phy)
{
	struct taurus_usb3_phy *taurus_phy = (struct taurus_usb3_phy *)phy;
	u8 *addr = taurus_phy->phy_data->addr;
	u16 *data = taurus_phy->phy_data->data;
	int size = taurus_phy->phy_data->size;
	int index;
	int ret;


	if (debug)
		dev_info(phy->dev, "%s()\n", __func__);

	dev_info(phy->dev, "USB3 PHY version: %s\n", taurus_phy->phy_data->ver);

	for (index = 0; index < size; index++) {
		taurus_usb3_phy_write(taurus_phy, *(addr + index),
		                      *(data + index));
		if (debug)
			(void)taurus_usb3_phy_read(taurus_phy, *(addr+index));
	}

	taurus_usb3_rx_term_setup(taurus_phy);
	if (debug) {
		dev_info(phy->dev, "%s Initialized RTK USB 3.0 PHY\n",
		         __func__);
	}

	/* init U2 phy */
	taurus_phy->u2phy = devm_usb_get_phy_by_phandle(taurus_phy->dev, "usb2-phy", 0);
	if (IS_ERR(taurus_phy->u2phy)) {
		ret = PTR_ERR(taurus_phy->u2phy);
		if (ret == -EPROBE_DEFER) {
			taurus_phy->u2phy = NULL;
			return ret;
		}
	} else {
		dev_info(phy->dev, "Invoke U2 PHY init\n");
		ret = usb_phy_init(taurus_phy->u2phy);
		if (ret) {
			dev_err(phy->dev, "Init U2 PHY error\n");
			return ret;
		}
	}

	return 0;
}

static void taurus_usb3_phy_shutdown(struct usb_phy *phy)
{
	/* Todo */
	if (debug)
		dev_info(phy->dev, "%s()\n", __func__);
}

static int taurus_usb3_phy_set_suspend(struct usb_phy *phy,
                                       int suspend)
{
	/* Todo */
	if (debug)
		dev_info(phy->dev, "%s()\n", __func__);

	return 0;
}

static int taurus_usb3_phy_set_wakeup(struct usb_phy *phy,
                                      bool enabled)
{
	/* Todo */
	if (debug)
		dev_info(phy->dev, "%s()\n", __func__);

	return 0;
}

static int taurus_usb3_phy_set_vbus(struct usb_phy *phy, int on)
{
	struct taurus_usb3_phy *taurus_phy = (struct taurus_usb3_phy *)phy;

	if (on) { /* on == true */
		gpiod_set_value_cansleep(taurus_phy->vbus, 1);
	} else { /* on == false */
		gpiod_set_value_cansleep(taurus_phy->vbus, 0);
	}
	msleep(100);

	if (debug)
		dev_info(phy->dev, "%s() -- vbus: %d\n", __func__,
		         gpiod_get_value_cansleep(taurus_phy->vbus));

	return 0;
}

enum {
	REV_A = 0,
	REV_B,
	REV_CNT,
};

static const char *phy_ver_str[] = {
	"phy_data_verA",
	"phy_data_verB",
};

static const char *phy_size_str[] = {
	"phy_data_sizeA",
	"phy_data_sizeB",
};

static const char *phy_addr_str[] = {
	"phy_data_addrA",
	"phy_data_addrB",
};

static const char *phy_val_str[] = {
	"phy_data_arrayA",
	"phy_data_arrayB",
};

static int taurus_usb3_phy_get_dts_label(struct device *dev, const char **s,
					 const char **a, const char **v,
					 const char **r)
{
	struct ca_soc_data soc;
	int rev, ret;

	/* get chip revision */
	memset(&soc, 0, sizeof(soc));
	ret = ca_soc_data_get(&soc);
	switch (soc.chip_revision) {
	case 'A':
		rev = REV_A;
		break;
	case 'B':
	default:
		rev = REV_B;
		break;
	}
	dev_info(dev, "Chip revision is %d using params %s\n", soc.chip_revision, phy_ver_str[rev]);

	*s = phy_size_str[rev];
	*a = phy_addr_str[rev];
	*v = phy_val_str[rev];
	*r = phy_ver_str[rev];

	return 0;
}

static int taurus_usb3_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct taurus_usb3_phy *taurus_usb_phy;
	struct phy_data *phy_data;

	struct resource *res;
	int	ret = 0;
	const char *size, *addr, *val, *ver;
	int addr_size, data_size;

	struct proc_dir_entry *e;

	if (debug)
		dev_info(dev, "Starting taurus USB 3.0 PHY Probe\n");

	taurus_usb_phy = devm_kzalloc(dev, sizeof(*taurus_usb_phy), GFP_KERNEL);
	if (IS_ERR(taurus_usb_phy))
		return -ENOMEM;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
	                                   "u3host_reg");
	taurus_usb_phy->phy_regbase = devm_ioremap_resource(dev, res);
	if (IS_ERR(taurus_usb_phy->phy_regbase))
		return PTR_ERR(taurus_usb_phy->phy_regbase);
	dev_info(dev, "usb3_phy resource - %pr mapped at 0x%pK\n", res,
	         taurus_usb_phy->phy_regbase);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
	                                   "u3ext_reg");
	taurus_usb_phy->ext_regbase = devm_ioremap_resource(dev, res);
	if (IS_ERR(taurus_usb_phy->ext_regbase))
		return PTR_ERR(taurus_usb_phy->ext_regbase);
	dev_info(dev, "usb3_phy resource - %pr mapped at 0x%pK\n", res,
	         taurus_usb_phy->ext_regbase);

	taurus_usb_phy->vbus = devm_gpiod_get_optional(dev, "usb-vbus", GPIOD_OUT_LOW);
	if (IS_ERR(taurus_usb_phy->vbus))
		dev_info(dev, "USB3-port VBUS GPIO Resource Get Failed and Just ignored\n");
	else
		dev_info(dev, "USB3-port VBUS GPIO Resource Get O.K and vbus:%d\n",
		         gpiod_get_value_cansleep(taurus_usb_phy->vbus));


	taurus_usb_phy->dev				= dev;
	taurus_usb_phy->phy.dev			= taurus_usb_phy->dev;
	taurus_usb_phy->phy.label		= TAURUS_USB3PHY_NAME;
	taurus_usb_phy->phy.init		= taurus_usb3_phy_init;
	taurus_usb_phy->phy.shutdown	= taurus_usb3_phy_shutdown;
	taurus_usb_phy->phy.set_suspend	= taurus_usb3_phy_set_suspend;
	taurus_usb_phy->phy.set_wakeup	= taurus_usb3_phy_set_wakeup;
	taurus_usb_phy->phy.set_vbus	= taurus_usb3_phy_set_vbus;
	taurus_usb_phy->phy.type		= USB_PHY_TYPE_USB3;

	spin_lock_init(&taurus_usb_phy->lock);

	phy_data = devm_kzalloc(dev, sizeof(*phy_data), GFP_KERNEL);
	if (IS_ERR(phy_data))
		return -ENOMEM;

	/* get DTS label for different chip revision */
	taurus_usb3_phy_get_dts_label(dev, &size, &addr, &val, &ver);

	if (node) {
		ret = of_property_read_u32_index(node, "phymask", 0,
		                                 &taurus_usb_phy->phy_mask);
		if (ret)
			goto err;
		//check phy mask
		if (taurus_usb_phy->phy_mask != 0x1 && taurus_usb_phy->phy_mask != 0x2) {
			dev_err(dev, "Invalid PHY mask (%d). PHY mask should be 1(S2) or 2(S3)\n",
			        taurus_usb_phy->phy_mask);
			return -EINVAL;
		}

		ret = of_property_read_u32_index(node, size, 0,
		                                 &phy_data->size);
		if (!ret)
			dev_warn(dev, "phy data size from DTS was ignored\n");

		addr_size = of_property_count_elems_of_size(node, addr,
							  sizeof(u8));
		data_size = of_property_count_elems_of_size(node, val,
							  sizeof(u16));
		if(addr_size != data_size)
		{
			dev_err(dev, "phy address size doesn't match with phy data size. (address size %d, data size %d)\n", addr_size, data_size);
			return -EINVAL;
		}

		phy_data->size = data_size;
		phy_data->addr = devm_kzalloc(dev, sizeof(u8)*phy_data->size,
		                              GFP_KERNEL);
		if (!phy_data->addr)
			return -ENOMEM;
		phy_data->data = devm_kzalloc(dev, sizeof(u16)*phy_data->size,
		                              GFP_KERNEL);
		if (!phy_data->data)
			return -ENOMEM;
		ret = of_property_read_u8_array(node, addr,
		                                phy_data->addr, phy_data->size);
		if (ret)
			goto err;
		ret = of_property_read_u16_array(node, val,
		                                 phy_data->data, phy_data->size);
		if (ret)
			goto err;
		ret = of_property_read_string(node, ver, &phy_data->ver);
		if (ret) {
			phy_data->ver = TAURUS_USB3PHY_VER_UNKNOWN;
		}
		taurus_usb_phy->phy_data = phy_data;
	}

	if (taurus_usb_phy->phy_mask == 0x1) {
		/* Enable S2 for USB3.0 port */
		taurus_usb_phy->serdes_phy = devm_phy_get(dev, "s2usb3-phy");
		if (IS_ERR(taurus_usb_phy->serdes_phy)) {
			ret = PTR_ERR(taurus_usb_phy->serdes_phy);
			dev_err(dev, "Failed to get serdes_phy.\n");
			return ret;
		}

		res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
		                                   "s2usbphy_u3port");
		taurus_usb_phy->serdes_regbase = devm_ioremap_resource(dev, res);
		if (IS_ERR(taurus_usb_phy->serdes_regbase))
			return PTR_ERR(taurus_usb_phy->serdes_regbase);
		dev_info(dev, "Enabled S2 u3port resource - %pr mapped at 0x%pK\n", res,
		         taurus_usb_phy->serdes_regbase);
	} else if (taurus_usb_phy->phy_mask == 0x2) {
		/* Enable S3 for USB3.0 port */
		taurus_usb_phy->serdes_phy = devm_phy_get(dev, "s3usb3-phy");
		if (IS_ERR(taurus_usb_phy->serdes_phy)) {
			ret = PTR_ERR(taurus_usb_phy->serdes_phy);
			dev_err(dev, "Failed to get serdes_phy.\n");
			return ret;
		}

		res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
		                                   "s3usbphy_u3port");
		taurus_usb_phy->serdes_regbase = devm_ioremap_resource(dev, res);
		if (IS_ERR(taurus_usb_phy->serdes_regbase))
			return PTR_ERR(taurus_usb_phy->serdes_regbase);
		dev_info(dev, "Enabled S3 u3port resource - %pr mapped at 0x%pK\n", res,
		         taurus_usb_phy->serdes_regbase);
	}

	platform_set_drvdata(pdev, taurus_usb_phy);
	ret = usb_add_phy_dev(&taurus_usb_phy->phy);
	if (ret)
		goto err;

	/* dphy reset and AUX bus reset */
	taurus_usb3_host_reset(taurus_usb_phy);

	e = proc_create_data("u3_phy", S_IRUGO | S_IWUSR, realtek_proc, &proc_ops_taurus_usb3_phy, taurus_usb_phy);

	if (debug)
		dev_info(dev, "Finished taurus USB 3.0 PHY Probe\n");
err:
	return ret;
}

static int taurus_usb3_phy_remove(struct platform_device *pdev)
{
	struct taurus_usb3_phy *taurus_usb_phy = platform_get_drvdata(pdev);

	usb_remove_phy(&taurus_usb_phy->phy);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id usbphy_taurus_dt_match[] = {
	{ .compatible = "realtek,taurus-usb3phy", },
	{},
};
MODULE_DEVICE_TABLE(of, usbphy_taurus_dt_match);
#endif

static struct platform_driver taurus_usb3_phy_driver = {
	.probe		= taurus_usb3_phy_probe,
	.remove		= taurus_usb3_phy_remove,
	.driver		= {
		.name	= TAURUS_USB3PHY_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(usbphy_taurus_dt_match),
	},
};
module_platform_driver(taurus_usb3_phy_driver);

MODULE_DESCRIPTION("Realtek Taurus USB3 PHY driver");
MODULE_ALIAS("platform:" TAURUS_USB3PHY_NAME);
MODULE_LICENSE("GPL v2");
