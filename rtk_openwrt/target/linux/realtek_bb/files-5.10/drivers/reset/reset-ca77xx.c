/*
 * FILE NAME reset-ca77xx.c
 *
 * BRIEF MODULE DESCRIPTION
 *  Driver for Cortina Access CA77XX RESET controller.
 *
 * Copyright (C) 2015 Cortina Access, Inc.
 *		http://www.cortina-access.com
 *
 * Based on reset-socfpga.c
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#ifdef CONFIG_ARCH_CORTINA_SMC
#include <soc/cortina/cortina-smc.h>
#endif
#define NR_BANKS		1
#define SZ_BANK			32
#define OFFSET_BLOCK_RESET	0x0

struct ca77xx_reset_data {
	spinlock_t			lock;	/* lock access to bank regs */
	void __iomem			*iobase;
#ifdef CONFIG_ARCH_CORTINA_SMC
	unsigned int			physical_addr;
#endif
	struct reset_controller_dev	rcdev;
};

static int ca77xx_reset_assert(struct reset_controller_dev *rcdev,
			       unsigned long id)
{
	struct ca77xx_reset_data *data = container_of(rcdev,
						     struct ca77xx_reset_data,
						     rcdev);
	int bank = id / SZ_BANK;
	int offset = id % SZ_BANK;
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(&data->lock, flags);
#ifdef CONFIG_ARCH_CORTINA_SMC
	reg = data->physical_addr;
	CA_SMC_CALL_REG_BIT_SET(reg + (bank * NR_BANKS), offset);
#else
	reg = readl(data->iobase + OFFSET_BLOCK_RESET + (bank * NR_BANKS));
	writel(reg | BIT(offset), data->iobase + OFFSET_BLOCK_RESET +
				 (bank * NR_BANKS));
#endif
	spin_unlock_irqrestore(&data->lock, flags);

	return 0;
}

static int ca77xx_reset_deassert(struct reset_controller_dev *rcdev,
				 unsigned long id)
{
	struct ca77xx_reset_data *data = container_of(rcdev,
						     struct ca77xx_reset_data,
						     rcdev);

	int bank = id / SZ_BANK;
	int offset = id % SZ_BANK;
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(&data->lock, flags);
#ifdef CONFIG_ARCH_CORTINA_SMC
	reg = data->physical_addr;
	CA_SMC_CALL_REG_BIT_CLEAR(reg + (bank * NR_BANKS), offset);
#else
	reg = readl(data->iobase + OFFSET_BLOCK_RESET + (bank * NR_BANKS));
	writel(reg & ~BIT(offset), data->iobase + OFFSET_BLOCK_RESET +
				  (bank * NR_BANKS));
#endif
	spin_unlock_irqrestore(&data->lock, flags);

	return 0;
}

static int ca77xx_reset_status(struct reset_controller_dev *rcdev,
			       unsigned long id)
{
	struct ca77xx_reset_data *data = container_of(rcdev,
						     struct ca77xx_reset_data,
						     rcdev);
	int bank = id / SZ_BANK;
	int offset = id % SZ_BANK;
	u32 reg;
#ifdef CONFIG_ARCH_CORTINA_SMC
	reg = CA_SMC_CALL_REG_READ(data->physical_addr + OFFSET_BLOCK_RESET + (bank * NR_BANKS));
#else
	reg = readl(data->iobase + OFFSET_BLOCK_RESET + (bank * NR_BANKS));
#endif
	return (reg & BIT(offset));
}

static struct reset_control_ops ca77xx_reset_ops = {
	.assert		= ca77xx_reset_assert,
	.deassert	= ca77xx_reset_deassert,
	.status		= ca77xx_reset_status,
};

static int ca77xx_reset_probe(struct platform_device *pdev)
{
	struct ca77xx_reset_data *data;
	struct resource *res;

	/*
	 * The binding was mainlined without the required property.
	 * Do not continue, when we encounter an old DT.
	 */
	if (!of_find_property(pdev->dev.of_node, "#reset-cells", NULL)) {
		dev_err(&pdev->dev, "%s missing #reset-cells property\n",
			pdev->dev.of_node->full_name);
		return -EINVAL;
	}

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
#ifdef CONFIG_ARCH_CORTINA_SMC
	data->physical_addr = (unsigned int)res->start;
#endif
	data->iobase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(data->iobase))
		return PTR_ERR(data->iobase);
	dev_info(&pdev->dev, "resource - %pr mapped at 0x%pK\n", res,
		 data->iobase);

	spin_lock_init(&data->lock);

	data->rcdev.owner = THIS_MODULE;
	data->rcdev.nr_resets = NR_BANKS * SZ_BANK;
	data->rcdev.ops = &ca77xx_reset_ops;
	data->rcdev.of_node = pdev->dev.of_node;
	reset_controller_register(&data->rcdev);

	return 0;
}

static int ca77xx_reset_remove(struct platform_device *pdev)
{
	struct ca77xx_reset_data *data = platform_get_drvdata(pdev);

	reset_controller_unregister(&data->rcdev);

	return 0;
}

static const struct of_device_id ca77xx_reset_dt_ids[] = {
	{ .compatible = "cortina,rst-mgr", },
	{ /* sentinel */ },
};

static struct platform_driver ca77xx_reset_driver = {
	.probe	= ca77xx_reset_probe,
	.remove	= ca77xx_reset_remove,
	.driver = {
		.name		= "ca77xx-reset",
		.of_match_table	= ca77xx_reset_dt_ids,
	},
};
module_platform_driver(ca77xx_reset_driver);

MODULE_DESCRIPTION("CA77XX Reset Controller Driver");
MODULE_LICENSE("GPL");
