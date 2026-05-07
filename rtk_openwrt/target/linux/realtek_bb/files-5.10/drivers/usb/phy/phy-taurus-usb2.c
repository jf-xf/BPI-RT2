/*
 * USB2.0 PHY Device Driver for dwc3 core controller on
 * Realtek TAURUS SoCs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/usb/otg.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/reset-controller.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/reset.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <soc/cortina/cortina-soc.h>

#define TAURUS_USB2PHY_NAME "taurus-usb2phy"
#define TAURUS_USB2PHY_VER_UNKNOWN "unknown"

#define TAURUS_USB2_PORTNUM 2

/* USB2(USB High Speed) register offsets */
#define USB2CFG_PHY_PORT0_CONFIG_OFFSET		0x00
#define USB2CFG_PHY_PORT1_CONFIG_OFFSET		0x08
#define USB2CFG_PHY_PORT_VSTATUS_IN_MASK(x)		((0xff & (x)) << 24)
#define USB2CFG_PHY_PORT_VCNTRL_MASK(x)		((0xf & (x)) << 20)
#define USB2CFG_PHY_PORT_VLOADM					BIT(19)
#define USB2CFG_PHY_PORT_BY_PASS_ON				BIT(18)
#define USB2CFG_PHY_PORT_LF_PD_R_EN				BIT(9)
#define USB2CFG_PHY_PORT_CLKTSTEN				BIT(8)
#define USB2CFG_PHY_PORT_DPPULLDOWN				BIT(6)
#define USB2CFG_PHY_PORT_DMPULLDOWN				BIT(5)
#define USB2CFG_PHY_PORT_TXBITSTUFF_ENABLE		BIT(4)
#define USB2CFG_PHY_PORT_TXBITSTUFF_ENABLE_H	BIT(3)
#define USB2CFG_PHY_PORT_TX_ENABLE_N			BIT(2)
#define USB2CFG_PHY_PORT_TX_DAT					BIT(1)
#define USB2CFG_PHY_PORT_TX_SE0					BIT(0)

#define USB2CFG_PHY_PORT0_STATUS_OFFSET		0x04
#define USB2CFG_PHY_PORT1_STATUS_OFFSET		0x0c
#define USB2CFG_PHY_PORT_VSTATUS_OUT_MASK(x)	((0xff & (x)) << 24)
#define USB2CFG_PHY_PORT_DEBUG_MASK(x)			(0xff & (x))

#define USB2CFG_PHY_PORT0_FORCE_MODE_OFFSET 0x10
#define USB2CFG_PHY_PORT0_RX_TERM_SEL			BIT(9)
#define USB2CFG_PHY_PORT0_FSUS			BIT(7)
#define USB2CFG_PHY_PORT1_FORCE_MODE_OFFSET 0x14
#define USB2CFG_PHY_PORT1_RX_TERM_SEL			BIT(25)

struct phy_data {
	int size;
	u8 *addr;
	u8 *data;
	const char *ver;
};

struct taurus_usb2_phy {
	struct usb_phy	phy;
	struct device	*dev;
	struct phy_data *phy_data;
	void __iomem	*phy_regbase;
	int port_mask;
	struct reset_control	*usbcore_reset; /* reset xhci only once */
	struct reset_control	*usb2port1_reset; /* usb2 port 1 reset */
	struct gpio_desc        *u2port0_vbus;
	spinlock_t		lock;	/* lock access to bank regs */
};

static int debug = 0;

static void taurus_usb2_host_reset(struct taurus_usb2_phy *taurus_phy)
{
	u32 reg_val = 0;
	unsigned long flags;
	int port_mask = taurus_phy->port_mask;

	if (debug)
		dev_info(taurus_phy->dev, "%s()\n", __func__);

	reset_control_assert(taurus_phy->usbcore_reset);
	msleep(20);
	reset_control_deassert(taurus_phy->usbcore_reset);
	msleep(20);

	if (port_mask & 0x2) {
		//deassert port1 block reset if it's asserted
		if (reset_control_status(taurus_phy->usb2port1_reset) > 0)
			reset_control_deassert(taurus_phy->usb2port1_reset);
	}

	/* For USB2.0 PHY PORT RESET */
	spin_lock_irqsave(&taurus_phy->lock, flags);

	if (port_mask & 0x1) {
		reg_val = readl(taurus_phy->phy_regbase + USB2CFG_PHY_PORT0_CONFIG_OFFSET);
		reg_val |= 0x00080064;
		writel(reg_val, taurus_phy->phy_regbase + USB2CFG_PHY_PORT0_CONFIG_OFFSET);
		if (debug) {
			dev_info(taurus_phy->dev, "read USB2CFG_PHY_PORT0_CONFIG reg_val = 0x%08x\n",
			         readl(taurus_phy->phy_regbase + USB2CFG_PHY_PORT0_CONFIG_OFFSET));
		}
	}

	if (port_mask & 0x2) {
		reg_val = readl(taurus_phy->phy_regbase + USB2CFG_PHY_PORT1_CONFIG_OFFSET);
		reg_val |= 0x00080064;
		writel(reg_val, taurus_phy->phy_regbase + USB2CFG_PHY_PORT1_CONFIG_OFFSET);
		if (debug) {
			dev_info(taurus_phy->dev, "read USB2CFG_PHY_PORT1_CONFIG reg_val = 0x%08x\n",
			         readl(taurus_phy->phy_regbase + USB2CFG_PHY_PORT1_CONFIG_OFFSET));
		}
	}

	spin_unlock_irqrestore(&taurus_phy->lock, flags);
}

static void taurus_usb2_rx_term_setup(struct taurus_usb2_phy *taurus_phy)
{
	u32 reg_val = 0;
	unsigned long flags;
	int port_mask = taurus_phy->port_mask;

	if (debug)
		dev_info(taurus_phy->dev, "%s()\n", __func__);

	/* For USB2.0 RX termination */
	spin_lock_irqsave(&taurus_phy->lock, flags);

	if (port_mask & 0x1) {
		reg_val = readl(taurus_phy->phy_regbase + USB2CFG_PHY_PORT0_FORCE_MODE_OFFSET);
		reg_val &= ~USB2CFG_PHY_PORT0_RX_TERM_SEL;
		reg_val |= USB2CFG_PHY_PORT0_FSUS;
		writel(reg_val, taurus_phy->phy_regbase + USB2CFG_PHY_PORT0_FORCE_MODE_OFFSET);
		if (debug) {
			dev_info(taurus_phy->dev, "read USB2CFG_PHY_PORT0_FORCE_MODE reg_val = 0x%08x\n",
			         readl(taurus_phy->phy_regbase + USB2CFG_PHY_PORT0_FORCE_MODE_OFFSET));
		}
	}

	if (port_mask & 0x2) {
		reg_val = readl(taurus_phy->phy_regbase + USB2CFG_PHY_PORT1_FORCE_MODE_OFFSET);
		reg_val &= ~USB2CFG_PHY_PORT1_RX_TERM_SEL;
		writel(reg_val, taurus_phy->phy_regbase + USB2CFG_PHY_PORT1_FORCE_MODE_OFFSET);
		if (debug) {
			dev_info(taurus_phy->dev, "read USB2CFG_PHY_PORT1_FORCE_MODE reg_val = 0x%08x\n",
			         readl(taurus_phy->phy_regbase + USB2CFG_PHY_PORT1_FORCE_MODE_OFFSET));
		}
	}

	spin_unlock_irqrestore(&taurus_phy->lock, flags);
}

static void taurus_usb2_phy_port_cfg(struct taurus_usb2_phy *taurus_phy,
                                     u8 vstat_in_mask, u8 vctrl_mask, int port_id)
{
	u32 offset = 0;
	u32 reg_val = 0;
	u8 vctrl_mask1, vctrl_mask2;

	vctrl_mask1 = vctrl_mask & 0x0F;
	vctrl_mask2 = (vctrl_mask >> 4) & 0x0F;
	reg_val |= (USB2CFG_PHY_PORT_VSTATUS_IN_MASK(vstat_in_mask)
	            | USB2CFG_PHY_PORT_VCNTRL_MASK(vctrl_mask1)
	            | USB2CFG_PHY_PORT_DPPULLDOWN
	            | USB2CFG_PHY_PORT_DMPULLDOWN
	            | USB2CFG_PHY_PORT_VLOADM
	            | USB2CFG_PHY_PORT_TX_ENABLE_N);

	if (port_id == 0)
		offset = USB2CFG_PHY_PORT0_CONFIG_OFFSET;
	else if (port_id == 1)
		offset = USB2CFG_PHY_PORT1_CONFIG_OFFSET;
	else {
		dev_info(taurus_phy->dev, "The usb2phy port id is invalid.\n");
		return;
	}

	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "\nAddr = 0x%02x, Data = 0x%02x\n",
		         vctrl_mask, vstat_in_mask);
	if (debug)
		dev_info(taurus_phy->dev, "1read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	reg_val &= ~USB2CFG_PHY_PORT_VLOADM;
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "2read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	reg_val |= USB2CFG_PHY_PORT_VLOADM;
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "3read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	/* Clear VCONTROL field before refilling */
	reg_val &= ~USB2CFG_PHY_PORT_VCNTRL_MASK(0xF);
	reg_val |= USB2CFG_PHY_PORT_VCNTRL_MASK(vctrl_mask2);
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "4read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	reg_val &= ~USB2CFG_PHY_PORT_VLOADM;
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "5read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	reg_val |= USB2CFG_PHY_PORT_VLOADM;
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "6read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));
}

static u8 taurus_usb2_phy_port_read(struct taurus_usb2_phy *taurus_phy,
                                    u8 vstat_in_mask, u8 vctrl_mask, int port_id)
{
	u32 offset = 0;
	u32 reg_val = 0;
	u8 vctrl_mask1, vctrl_mask2;

	vctrl_mask1 = vctrl_mask & 0x0F;
	vctrl_mask2 = ((vctrl_mask >> 4) & 0x0D); //bit[1] = 0 for read
	reg_val |= (USB2CFG_PHY_PORT_VSTATUS_IN_MASK(vstat_in_mask)
	            | USB2CFG_PHY_PORT_VCNTRL_MASK(vctrl_mask1)
	            | USB2CFG_PHY_PORT_DPPULLDOWN
	            | USB2CFG_PHY_PORT_DMPULLDOWN
	            | USB2CFG_PHY_PORT_VLOADM
	            | USB2CFG_PHY_PORT_TX_ENABLE_N);

	if (port_id == 0)
		offset = USB2CFG_PHY_PORT0_CONFIG_OFFSET;
	else if (port_id == 1)
		offset = USB2CFG_PHY_PORT1_CONFIG_OFFSET;
	else {
		dev_err(taurus_phy->dev, "The usb2phy port id is invalid.\n");
		return 0;
	}

	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "\nAddr = 0x%02x, Data = 0x%02x\n",
		         vctrl_mask, vstat_in_mask);
	if (debug)
		dev_info(taurus_phy->dev, "1read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	reg_val &= ~USB2CFG_PHY_PORT_VLOADM;
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "2read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	reg_val |= USB2CFG_PHY_PORT_VLOADM;
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "3read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	/* Clear VCONTROL field before refilling */
	reg_val &= ~USB2CFG_PHY_PORT_VCNTRL_MASK(0xF);
	reg_val |= USB2CFG_PHY_PORT_VCNTRL_MASK(vctrl_mask2);
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "4read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	reg_val &= ~USB2CFG_PHY_PORT_VLOADM;
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "5read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	reg_val |= USB2CFG_PHY_PORT_VLOADM;
	writel(reg_val, taurus_phy->phy_regbase + offset);

	if (debug)
		dev_info(taurus_phy->dev, "6read U2PHY_P%d_CFG reg_val = 0x%08x",
		         port_id, readl(taurus_phy->phy_regbase + offset));

	/* Read PHY data */
	if (port_id == 0)
		offset = USB2CFG_PHY_PORT0_STATUS_OFFSET;
	else if (port_id == 1)
		offset = USB2CFG_PHY_PORT1_STATUS_OFFSET;

	return (readl(taurus_phy->phy_regbase + offset) >> 24) & 0xff;
}


__weak struct proc_dir_entry *realtek_proc;

static void show_usage(void)
{

	printk("	w [addr] [data] [port]: write [value] to [reg] of [port] (data should be 8 bits, addr should be in [E0-E7], [F0-F6])\n");
}

static ssize_t taurus_usb2_phy_proc_write(struct file * file, const char __user * userbuf, size_t count, loff_t * off)
{
	char buf[32];
	int len;
	u8 addr, data, port;
	struct taurus_usb2_phy *taurus_phy = PDE_DATA(file_inode(file));

	len = min(sizeof(buf), count);
	if (copy_from_user(buf, userbuf, len))
		return -E2BIG;

	if (strncmp(buf, "help", 4) == 0) {
		show_usage();
	} else if (strncmp(buf, "w ", 2) == 0) {
		if( 3 == sscanf(buf, "w %hhx %hhx %hhx", &addr, &data, &port)) {
			if (port > 2 || addr < 0xe0 || (addr > 0xe7 && addr < 0xf0) || addr > 0xf7)
				return -EINVAL;

			taurus_usb2_phy_port_cfg(taurus_phy, data, addr, port);
		} else {
			goto ERROR_PARA;
		}
	} else if (strncmp(buf, "r ", 2) == 0) {
		if(2 == sscanf(buf, "r %hhx %hhx", &addr, &port)) {
			if (port > 2 || addr < 0xe0 || (addr > 0xe7 && addr < 0xf0) || addr > 0xf7)
				return -EINVAL;

			data = taurus_usb2_phy_port_read(taurus_phy, 0x0, addr, port);
			printk("%02x = %02x\n", addr, data);
		}
	} else {
		goto ERROR_PARA;
	}
	return count;
ERROR_PARA:
	printk("error parameter...\n");
	show_usage();
	return -EPERM;
}


#define ITEM_PER_LINE 4
static int taurus_usb2_phy_show(struct seq_file *s, void *v)
{
	struct taurus_usb2_phy *taurus_phy = s->private;
	int port_mask = taurus_phy->port_mask;
	struct phy_data *phy_data = taurus_phy->phy_data;
	int addr, port_id, n = 0;
	u8 data;

	seq_printf(s, "USB2 PHY version: %s\n\n", phy_data->ver);

	/* USB2 PHY ports calibration and initialization */
	for (port_id = 0; port_id < TAURUS_USB2_PORTNUM; port_id++) {

		if (!(1 << port_id & port_mask))
			continue;

		seq_printf(s, "Port[%d]:\n", port_id);

		/*swtich to page 0*/
		taurus_usb2_phy_port_cfg(taurus_phy, 0x9b, 0xF4, port_id);
		seq_printf(s, "Page 0\n");
		n = 0;
		for (addr = 0xE0; addr <= 0xE7; addr++) {
			n++;
			data = taurus_usb2_phy_port_read(taurus_phy, 0x0,
			                                 addr, port_id);
			seq_printf(s, "%02X: %4x   ", addr, data);
			if ((n % ITEM_PER_LINE) == 0)
				seq_printf(s, "\n");
		}
		for (addr = 0xF0; addr <= 0xF7; addr++) {
			n++;
			data = taurus_usb2_phy_port_read(taurus_phy, 0x0,
			                                 addr, port_id);
			seq_printf(s, "%02X: %4x   ", addr, data);
			if ((n % ITEM_PER_LINE) == 0)
				seq_printf(s, "\n");
		}

		/*swtich to page 1*/
		taurus_usb2_phy_port_cfg(taurus_phy, 0xbb, 0xF4, port_id);
		seq_printf(s, "\nPage 1\n");
		for (addr = 0xE0; addr <= 0xE7; addr++) {
			n++;
			data = taurus_usb2_phy_port_read(taurus_phy, 0x0,
			                                 addr, port_id);
			seq_printf(s, "%02X: %4x   ", addr, data);
			if ((n % ITEM_PER_LINE) == 0)
				seq_printf(s, "\n");
		}
		seq_printf(s, "\n");

		/*swtich to page 2*/
		taurus_usb2_phy_port_cfg(taurus_phy, 0xdb, 0xF4, port_id);
		seq_printf(s, "\nPage 2\n");
		for (addr = 0xE0; addr <= 0xE7; addr++) {
			n++;
			data = taurus_usb2_phy_port_read(taurus_phy, 0x0,
			                                 addr, port_id);
			seq_printf(s, "%02X: %4x   ", addr, data);
			if ((n % ITEM_PER_LINE) == 0)
				seq_printf(s, "\n");
		}
		seq_printf(s, "\n");

		/*swtich back to page 0*/
		taurus_usb2_phy_port_cfg(taurus_phy, 0x9b, 0xF4, port_id);
		phy_data++;
	}
	return 0;
}

static int taurus_usb2_phy_open(struct inode *inode, struct file *file)
{
	return(single_open(file, taurus_usb2_phy_show, PDE_DATA(inode)));
}


static const struct proc_ops proc_ops_taurus_usb2_phy = {
	.proc_open  		= taurus_usb2_phy_open,
	.proc_write 		= taurus_usb2_phy_proc_write,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};


static int taurus_usb2_phy_init(struct usb_phy *phy)
{
	struct taurus_usb2_phy *taurus_phy = (struct taurus_usb2_phy *)phy;
	struct phy_data *phy_data = taurus_phy->phy_data;
	u8 *addr;
	u8 *data;
	int size;
	int port_mask = taurus_phy->port_mask;
	int index, port_id;

	if (debug)
		dev_info(phy->dev, "%s()\n", __func__);

	dev_info(phy->dev, "USB2 PHY version: %s\n", phy_data->ver);
	/* USB2 PHY ports calibration and initialization */
	for (port_id = 0; port_id < TAURUS_USB2_PORTNUM; port_id++) {
		addr = phy_data->addr;
		data = phy_data->data;
		size = phy_data->size;

		if (1 << port_id & port_mask) {
			for (index = 0; index < size; index++) {
				taurus_usb2_phy_port_cfg(taurus_phy, *(data + index),
				                         *(addr + index), port_id);
			}
		}
		phy_data++;
	}

	taurus_usb2_rx_term_setup(taurus_phy);


	if (debug) {
		dev_info(phy->dev, "%s Initialized Taurus USB 2.0 PHY\n",
		         __func__);
	}
	return 0;
}

static void taurus_usb2_phy_shutdown(struct usb_phy *phy)
{
	/* Todo */
	if (debug)
		dev_info(phy->dev, "%s()\n", __func__);
}

static int taurus_usb2_phy_set_suspend(struct usb_phy *phy,
                                       int suspend)
{
	/* Todo */
	if (debug)
		dev_info(phy->dev, "%s()\n", __func__);

	return 0;
}

static int taurus_usb2_phy_set_wakeup(struct usb_phy *phy,
                                      bool enabled)
{
	/* Todo */
	if (debug)
		dev_info(phy->dev, "%s()\n", __func__);

	return 0;
}

static int taurus_usb2_phy_set_vbus(struct usb_phy *phy,
                                    int on)
{
	struct taurus_usb2_phy *taurus_phy = (struct taurus_usb2_phy *)phy;

	if (on) { /* on == true */
		gpiod_set_value_cansleep(taurus_phy->u2port0_vbus, 0);
	} else { /* on == false */
		gpiod_set_value_cansleep(taurus_phy->u2port0_vbus, 1);
	}
	msleep(100);

	if (debug)
		dev_info(phy->dev, "%s() -- u2port0_vbus: %d", __func__,
		         gpiod_get_value_cansleep(taurus_phy->u2port0_vbus));

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

static int taurus_usb2_phy_get_dts_label(struct device *dev,
					const char **s, const char **a,
					const char **v, const char **r)
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

static int taurus_usb2_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node, *phy_node;
	struct taurus_usb2_phy *taurus_usb_phy;
	struct phy_data *phy_data;
	struct resource *res;
	const char *size, *addr, *val, *ver;
	int addr_size, data_size;
	char port_name[10] = {0};
	int port_id = 0;
	int ret = 0;

	struct proc_dir_entry *e;

	if (debug)
		dev_info(dev, "Starting Taurus USB 2.0 PHY Probe\n");

	taurus_usb_phy = devm_kzalloc(dev, sizeof(*taurus_usb_phy), GFP_KERNEL);
	if (IS_ERR(taurus_usb_phy))
		return -ENOMEM;
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
	                                   "u2host_reg");
	taurus_usb_phy->phy_regbase = devm_ioremap_resource(dev, res);
	if (IS_ERR(taurus_usb_phy->phy_regbase))
		return PTR_ERR(taurus_usb_phy->phy_regbase);
	dev_info(dev, "usb2_phy resource - %pr mapped at 0x%pK\n", res,
	         taurus_usb_phy->phy_regbase);

	taurus_usb_phy->u2port0_vbus = devm_gpiod_get_optional(dev, "u2port0-vbus", GPIOD_OUT_HIGH);
	if (IS_ERR(taurus_usb_phy->u2port0_vbus))
		dev_info(dev, "USB2-port0 VBUS GPIO Resource Get Failed and Just ignored\n");
	else
		dev_info(dev, "USB2-port0 VBUS GPIO Resource Get O.K and u2port0_vbus:%d\n",
		         gpiod_get_value_cansleep(taurus_usb_phy->u2port0_vbus));

	taurus_usb_phy->dev				= dev;
	taurus_usb_phy->phy.dev			= taurus_usb_phy->dev;
	taurus_usb_phy->phy.label		= TAURUS_USB2PHY_NAME;
	taurus_usb_phy->phy.init		= taurus_usb2_phy_init;
	taurus_usb_phy->phy.shutdown	= taurus_usb2_phy_shutdown;
	taurus_usb_phy->phy.set_suspend	= taurus_usb2_phy_set_suspend;
	taurus_usb_phy->phy.set_wakeup	= taurus_usb2_phy_set_wakeup;
	taurus_usb_phy->phy.set_vbus	= taurus_usb2_phy_set_vbus;
	taurus_usb_phy->phy.type        = USB_PHY_TYPE_USB2;

	spin_lock_init(&taurus_usb_phy->lock);

	phy_data = devm_kzalloc(dev, sizeof(*phy_data) * TAURUS_USB2_PORTNUM, GFP_KERNEL);
	if (IS_ERR(phy_data))
		return -ENOMEM;

	if (node) {
		taurus_usb_phy->usbcore_reset = of_reset_control_get(node,
		                                "usbcore_reset");
		if (IS_ERR(taurus_usb_phy->usbcore_reset)) {
			ret = PTR_ERR(taurus_usb_phy->usbcore_reset);
			dev_err(dev, "Failed to get usbcore_reset.\n");
			return ret;
		}

		taurus_usb_phy->usb2port1_reset = of_reset_control_get(node,
		                                  "usb2port1_reset");
		if (IS_ERR(taurus_usb_phy->usb2port1_reset)) {
			ret = PTR_ERR(taurus_usb_phy->usb2port1_reset);
			dev_err(dev, "Failed to get usb2port1_reset.\n");
			return ret;
		}

		ret = of_property_read_u32_index(node, "portmask", 0,
		                                 &taurus_usb_phy->port_mask);
		if (ret)
			goto err;

		taurus_usb_phy->phy_data = phy_data;

		taurus_usb2_phy_get_dts_label(dev, &size, &addr, &val, &ver);

		for(port_id = 0; port_id < TAURUS_USB2_PORTNUM; port_id++)
		{
			ret = of_property_read_string(node, ver, &phy_data->ver);
			if (ret) {
				phy_data->ver = TAURUS_USB2PHY_VER_UNKNOWN;
			}

			if (1 << port_id & taurus_usb_phy->port_mask)
			{
				/* get different phy node for port */
				phy_node = of_parse_phandle(node, "phy_sel", port_id);
				dev_info(dev, "USB2 port%d get %s OK\n",port_id, phy_node->name);

				ret = of_property_read_u32_index(phy_node, size, 0,
												 &phy_data->size);
				if (!ret)
					dev_warn(dev, "phy data size from DTS was ignored\n");

				addr_size = of_property_count_elems_of_size(phy_node, addr,
									  sizeof(u8));
				data_size = of_property_count_elems_of_size(phy_node, val,
									  sizeof(u8));

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

				phy_data->data = devm_kzalloc(dev, sizeof(u8)*phy_data->size,
											  GFP_KERNEL);
				if (!phy_data->data)
					return -ENOMEM;

				ret = of_property_read_u8_array(phy_node, addr,
												phy_data->addr, phy_data->size);
				if (ret)
					goto err;

				ret = of_property_read_u8_array(phy_node, val,
												phy_data->data, phy_data->size);
				if (ret)
					goto err;
			}
			phy_data++;
		}
	}
	platform_set_drvdata(pdev, taurus_usb_phy);

	ret = usb_add_phy_dev(&taurus_usb_phy->phy);
	if (ret)
		goto err;

	/* dphy reset and AUX bus reset for usb core (xhci controller)  */
	taurus_usb2_host_reset(taurus_usb_phy);

	e = proc_create_data("u2_phy", S_IRUGO | S_IWUSR, realtek_proc, &proc_ops_taurus_usb2_phy, taurus_usb_phy);

	if (debug)
		dev_info(dev, "Finished Taurus USB 2.0 PHY Probe\n");
err:
	return ret;
}

static int taurus_usb2_phy_remove(struct platform_device *pdev)
{
	struct taurus_usb2_phy *taurus_usb_phy = platform_get_drvdata(pdev);

	usb_remove_phy(&taurus_usb_phy->phy);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id usbphy_taurus_dt_match[] = {
	{ .compatible = "realtek,taurus-usb2phy", },
	{},
};
MODULE_DEVICE_TABLE(of, usbphy_taurus_dt_match);
#endif

static struct platform_driver taurus_usb2_phy_driver = {
	.probe		= taurus_usb2_phy_probe,
	.remove		= taurus_usb2_phy_remove,
	.driver		= {
		.name	= TAURUS_USB2PHY_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(usbphy_taurus_dt_match),
	},
};
module_platform_driver(taurus_usb2_phy_driver);

MODULE_DESCRIPTION("Realtek Taurus USB2 PHY driver");
MODULE_ALIAS("platform:" TAURUS_USB2PHY_NAME);
MODULE_LICENSE("GPL v2");
