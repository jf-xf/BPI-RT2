#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <cyg/hal/bspchip.h>
#include "pci_98d_wfo.h"
#include "../linux/pci_intf.c"

#ifdef PLATFORM_ECOS
int wfo_enable = 1;
#endif /* PLATFORM_ECOS */

#define PADDR(addr)  ((addr) & 0x1FFFFFFF)

#define BSP_PCI_MISC		0xb8000504
#define BSP_PCIE1_D_IO		0xB8C00000
#define BSP_PCIE0_D_IO		0xB8E00000
#define BSP_PCIE1_D_MEM		0xB9000000
#define BSP_PCIE0_D_MEM		0xBA000000
#define BSP_PCIE1_D_CFG0	0xB8B10000
#define BSP_PCIE0_D_CFG0	0xB8B30000
#define BSP_PCIE1_H_CFG		0xB8B00000
#define BSP_PCIE1_H_EXT		0xB8B01000
#define BSP_PCIE0_H_CFG		0xB8B20000
#define BSP_PCIE0_H_EXT		0xB8B21000

#define BSP_ENABLE_PCIE0	(1 << 7)
#define BSP_ENABLE_PCIE1	(1 << 6)

struct pcie_para {
	u8 reg;
	u16 value;
};

#define MAX_NUM_DEV	4
#define FL_MDIO_RESET	(1 << 0)
#define FL_SET_BAR	(1 << 1)

struct rtk_pci_controller {
	void __iomem *devcfg_base;
	void __iomem *hostcfg_base;
	void __iomem *hostext_base;
	spinlock_t lock;
	struct pci_controller controller;
	const struct pcie_para *phy_param;
	unsigned long rst_data;
	u8 irq;
	u8 flags;
	u8 port;
	u8 is_linked;
	u8 bus_addr;
};

static struct rtk_pci_controller *rtk_pci_ctrl;
static cyg_uint32 pci_interrupt_isr(cyg_vector_t vector, cyg_addrword_t data);
static cyg_interrupt rtk_pci_intr;
static cyg_handle_t rtk_pci_intr_handle;

static int rtk_pcie_read(struct pci_bus *bus, unsigned int devfn,
			int where, int size, unsigned int *val)
{
	struct rtk_pci_controller *ctrl = rtk_pci_ctrl;
	unsigned long flags;
	void __iomem *base;
	u32 data;

	switch (PCI_SLOT(devfn)) {
	case 0:
		base = ctrl->hostcfg_base;
		break;
	case 1:
		base = ctrl->devcfg_base;
		break;
	default:
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	spin_lock_irqsave(&ctrl->lock, flags);
	REG32(ctrl->hostext_base+0xC) = PCI_FUNC(devfn);
	switch (size) {
	case 1:
		data = REG08(base + where);
		break;
	case 2:
		data = REG16(base + where);
		break;
	case 4:
		data = REG32(base + where);
		break;
	default:
		spin_unlock_irqrestore(&ctrl->lock, flags);
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}
	spin_unlock_irqrestore(&ctrl->lock, flags);

	//printk("%s: reading data from address 0x%x\n", __func__, base + where);

	*val = data;

	return PCIBIOS_SUCCESSFUL;
}

static int rtk_pcie_write(struct pci_bus *bus, unsigned int devfn,
		int where, int size, u32 val)
{
	struct rtk_pci_controller *ctrl = rtk_pci_ctrl;
	unsigned long flags;
	void __iomem *base;

	switch (PCI_SLOT(devfn)) {
	case 0:
		base = ctrl->hostcfg_base;
		break;
	case 1:
		base = ctrl->devcfg_base;
		break;
	default:
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	spin_lock_irqsave(&ctrl->lock, flags);
	REG32(ctrl->hostext_base+0xC) = PCI_FUNC(devfn);
	switch (size) {
	case 1:
		REG08(base + where) = val;
		break;
	case 2:
		REG16(base + where) = val;
		break;
	case 4:
		REG32(base + where) = val;
		break;
	default:
		spin_unlock_irqrestore(&ctrl->lock, flags);
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}
	spin_unlock_irqrestore(&ctrl->lock, flags);

	//printk("%s: writing data to address 0x%x value = 0x%x\n", __func__, base + where, val);

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_bus pcie_bus;
static struct pci_dev wfo_pcie_dev = {
	.bus	= &pcie_bus,
	.devfn	= 8,
	.dev = {
		.init_name = CONFIG_RTW_2_4G_DEV,
	},
	.pcie_cap	= 0x70,
	.irq		= BSP_PCIE1_IRQ,
};
static struct pci_bus pcie_bus = {
	.name = "RTK-WFO PCIE",
	.self = NULL,
};
static struct pci_ops pcie_ops = {
	.read = rtk_pcie_read,
	.write = rtk_pcie_write
};

static struct resource pcie1_io_resource = {
	.name   = "PCIE1 IO",
	.flags  = IORESOURCE_IO,
	.start  = PADDR(BSP_PCIE1_D_IO),
	.end    = PADDR(BSP_PCIE1_D_IO + 0xFFFF)
};

static struct resource pcie1_mem_resource = {
	.name   = "PCIE1 MEM",
	.flags  = IORESOURCE_MEM,
	.start  = PADDR(BSP_PCIE1_D_MEM),
	.end    = PADDR(BSP_PCIE1_D_MEM + 0xFFFFFF)
};

static struct rtk_pci_controller wfo_pcie1_controller = {
	.devcfg_base		= (void __iomem *)BSP_PCIE1_D_CFG0,
	.hostcfg_base		= (void __iomem *)BSP_PCIE1_H_CFG,
	.hostext_base		= (void __iomem *)BSP_PCIE1_H_EXT,
	.irq			= BSP_PCIE1_IRQ,
	.controller = {
		.bus		= &pcie_bus,
		.pci_ops	= &pcie_ops,
		.mem_resource	= &pcie1_mem_resource,
		.io_resource	= &pcie1_io_resource,
	},
	.port			= 1,
	.flags			= FL_MDIO_RESET,
	.bus_addr		= 0xff,
};

static struct resource pcie0_io_resource = {
	.name	= "PCIE0 IO",
	.flags	= IORESOURCE_IO,
	.start	= PADDR(BSP_PCIE0_D_IO),
	.end	= PADDR(BSP_PCIE0_D_IO + 0xFFFF)
};

static struct resource pcie0_mem_resource = {
	.name	= "PCIE0 MEM",
	.flags	= IORESOURCE_MEM,
	.start	= PADDR(BSP_PCIE0_D_MEM),
	.end    = PADDR(BSP_PCIE0_D_MEM + 0xFFFFFF)
};

static struct rtk_pci_controller wfo_pcie0_controller = {
	.devcfg_base		= (void __iomem *)BSP_PCIE0_D_CFG0,
	.hostcfg_base		= (void __iomem *)BSP_PCIE0_H_CFG,
	.hostext_base		= (void __iomem *)BSP_PCIE0_H_EXT,
	.irq			= BSP_PCIE1_IRQ,
	.controller = {
		.bus		= &pcie_bus,
		.pci_ops	= &pcie_ops,
		.mem_resource	= &pcie0_mem_resource,
		.io_resource	= &pcie0_io_resource,
	},
	.port			= 0,
	.flags			= FL_MDIO_RESET,
	.bus_addr		= 0xff,
};

int rtk_wfo_pcie_init(void)
{
	void *intr_dev;
	int i, pci_id_tbl_len;

	rtk_pci_ctrl = &wfo_pcie1_controller;

	printk("RTK PCI init: %s%d, irq = %u\n",
			rtk_pci_ctrl->controller.bus->name,
			rtk_pci_ctrl->port,
			rtk_pci_ctrl->irq);
	printk("Dev cfg base = 0x%x, mem base = 0x%x, io base = 0x%x\n",
			rtk_pci_ctrl->devcfg_base,
			rtk_pci_ctrl->controller.mem_resource->start,
			rtk_pci_ctrl->controller.io_resource->start);

	memcpy(&(wfo_pcie_dev.resource[0]), rtk_pci_ctrl->controller.mem_resource , sizeof(struct resource));
	memcpy(&(wfo_pcie_dev.resource[1]), rtk_pci_ctrl->controller.io_resource, sizeof(struct resource));

	register_pci_controller(&(rtk_pci_ctrl->controller));

	rtw_drv_entry();

	/* Get PCI device id in share memory */
	pci_id_tbl_len = sizeof(rtw_pci_id_tbl)/sizeof(rtw_pci_id_tbl[0]);
	for (i = 0; i<pci_id_tbl_len; i++) {
		if (rtw_pci_id_tbl[i].device == RTK_SHM_DRAM->pci_param.pci_dev_id) {
			RTK_SHM_DRAM->pci_param.pci_status = 2;
			rtw_dev_probe(&wfo_pcie_dev, &rtw_pci_id_tbl[i]);
			break;
		}
	}

	if (i == pci_id_tbl_len) {
		RTK_SHM_DRAM->pci_param.pci_status = 1;
		printk("Error: PE can't find PCI device ID in rtw_pci_id_tbl!");
		return -1;
	}

	/* Register interrupt at eCos */
	intr_dev = pci_get_drvdata(&wfo_pcie_dev);

	cyg_interrupt_create(rtk_pci_ctrl->irq, 0, intr_dev, &pci_interrupt_isr,
			ecos_dsr_raise_softirq, &rtk_pci_intr_handle, &rtk_pci_intr);
	cyg_interrupt_attach(rtk_pci_intr_handle);
	cyg_interrupt_unmask(rtk_pci_ctrl->irq);

	return 0;
}

void rtk_wfo_poll_irq(void)
{
	struct dvobj_priv *dvobj = pci_get_drvdata(&wfo_pcie_dev);

	pci_interrupt_isr(wfo_pcie_dev.irq, dvobj);
}

void rtk_wfo_get_irq(u32 *irq)
{
	*irq = wfo_pcie_dev.irq;
}

void rtk_wfo_get_mac(u8 *mac, u8 idx)
{
	if (idx) {
		struct dev_map_tbl_s *map = get_radio_map(idx);
		_adapter *adapter = (_adapter *)(map->adapter);
		rtw_macaddr_cfg(mac, adapter_mac_addr(adapter));
	} else {
		struct dvobj_priv *dvobj = pci_get_drvdata(&wfo_pcie_dev);
		rtw_macaddr_cfg(mac, adapter_mac_addr(dvobj->padapters[idx]));
	}
}

void *rtk_wfo_get_net_dev(u8 idx)
{

	if (idx >= CONFIG_IFACE_NUMBER )
		return NULL;

	if (idx) {
		struct dev_map_tbl_s *map = NULL;
		map = get_radio_map(idx);
		return map->ndev;
	} else {
		struct dvobj_priv *dvobj = pci_get_drvdata(&wfo_pcie_dev);
		return dvobj->padapters[idx]->pnetdev;
	}
}

cyg_uint32 pci_interrupt_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	rtk_interrupt_count[vector]++;
	ecos_interrupt_in ++;

	//cyg_interrupt_mask(vector);

	rtw_pci_interrupt(vector, data, NULL);

	//cyg_interrupt_unmask(vector);
	ecos_interrupt_out ++;

	return (CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);
}
