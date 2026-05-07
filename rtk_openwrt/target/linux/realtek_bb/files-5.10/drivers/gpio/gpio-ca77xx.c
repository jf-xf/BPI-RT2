/*
 * FILE NAME gpio-ca77xx.c
 *
 * BRIEF MODULE DESCRIPTION
 *  Driver for Cortina Access CA77XX GPIO controller.
 *
 * Copyright (C) 2015 Cortina Access, Inc.
 *		http://www.cortina-access.com
 *
 * Based on gpio-tz1090.c and gpio-bcm-kona.c
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>

#include <linux/reset.h>

/* GPIO Register Map */
#define CA77XX_GPIO_CFG		0x00
#define CA77XX_GPIO_OUT		0x04
#define CA77XX_GPIO_IN		0x08
#define CA77XX_GPIO_LVL		0x0C
#define CA77XX_GPIO_EDGE	0x10
#define CA77XX_GPIO_BOTHEDGE	0x14
#define CA77XX_GPIO_IE		0x18
#define CA77XX_GPIO_INT		0x1C
#define CA77XX_GPIO_STAT	0x20

/* ca77xx_GPIO_CFG BIT */
#define CA77XX_GPIO_CFG_OUT	0
#define CA77XX_GPIO_CFG_IN	1

#define GPIO_MAX_BANK_NUM	5
#define GPIO_BANK_SIZE		32
#define GPIO_BANK_REG_OFFSET	0x24

/**
 * struct ca77xx_gpio_bank - GPIO bank private data
 * @chip:	Generic GPIO chip for GPIO bank
 * @domain:	IRQ domain for GPIO bank (may be NULL)
 * @reg:	Base of registers, offset for this GPIO bank
 * @irq:	IRQ number for GPIO bank
 * @label:	Debug GPIO bank label, used for storage of chip->label
 *
 * This is the main private data for a GPIO bank. It encapsulates a gpio_chip,
 * and the callbacks for the gpio_chip can access the private data with the
 * function gpiochip_get_data().
 */
struct ca77xx_gpio_bank {
	struct gpio_chip chip;
	struct irq_domain *domain;
	void __iomem *reg;
	void __iomem *mux_reg;
	int irq;
	char label[16];
	raw_spinlock_t lock; /* lock access to bank regs */
};

/**
 * struct ca77xx_gpio_bank_info - Temporary registration info for GPIO bank
 * @priv:	Overall GPIO device private data
 * @node:	Device tree node specific to this GPIO bank
 * @index:	Index of bank in range 0 - (GPIO_MAX_BANK_NUM - 1)
 */
struct ca77xx_gpio_bank_info {
	struct ca77xx_gpio *priv;
	struct device_node *node;
	unsigned int index;
	struct ca77xx_gpio_bank *bank;
};

/**
 * struct ca77xx_gpio - Overall GPIO device private data
 * @dev:	Device (from platform device)
 * @reg:	Base of GPIO registers
 *
 * Represents the overall GPIO device. This structure is actually only
 * temporary, and used during init.
 */
struct ca77xx_gpio {
	struct device *dev;
	void __iomem *reg;
	void __iomem *mux_reg;
	int idx;
	struct ca77xx_gpio_bank_info *info[GPIO_MAX_BANK_NUM];
};

/* Convenience register accessors */
static inline void ca77xx_gpio_write(struct ca77xx_gpio_bank *bank,
				     unsigned int reg_offs, u32 data)
{
	iowrite32(data, bank->reg + reg_offs);
}

static inline u32 ca77xx_gpio_read(struct ca77xx_gpio_bank *bank,
				   unsigned int reg_offs)
{
	return ioread32(bank->reg + reg_offs);
}

static inline void _ca77xx_gpio_clear_bit(struct ca77xx_gpio_bank *bank,
					  unsigned int reg_offs,
					  unsigned int offset)
{
	u32 value;

	value = ca77xx_gpio_read(bank, reg_offs);
	value &= ~BIT(offset);
	ca77xx_gpio_write(bank, reg_offs, value);
}

static inline void _ca77xx_gpio_set_bit(struct ca77xx_gpio_bank *bank,
					unsigned int reg_offs,
					unsigned int offset)
{
	u32 value;

	value = ca77xx_gpio_read(bank, reg_offs);
	value |= BIT(offset);
	ca77xx_gpio_write(bank, reg_offs, value);
}

static inline void _ca77xx_gpio_mod_bit(struct ca77xx_gpio_bank *bank,
					unsigned int reg_offs,
					unsigned int offset,
					bool val)
{
	u32 value;

	value = ca77xx_gpio_read(bank, reg_offs);
	value &= ~BIT(offset);
	if (val)
		value |= BIT(offset);
	ca77xx_gpio_write(bank, reg_offs, value);
}

static inline u32 _ca77xx_gpio_get_bit(struct ca77xx_gpio_bank *bank,
				       unsigned int reg_offs,
				       unsigned int offset)
{
	u32 value;

	value =	ca77xx_gpio_read(bank, reg_offs) & BIT(offset);

	return value ? 1 :  0;
}

static int ca77xx_gpio_request(struct gpio_chip *chip, unsigned int offset)
{
	struct ca77xx_gpio_bank *bank = gpiochip_get_data(chip);
	unsigned long flags;

#ifdef CONFIG_PINCTRL
	int ret;

	//ret = pinctrl_request_gpio(chip->base + offset);
	ret = pinctrl_gpio_request(chip->base + offset);
	if (ret)
		return ret;
#else
	u32 value;

	value = ioread32(bank->mux_reg);
	value |= BIT(offset);
#endif

	raw_spin_lock_irqsave(&bank->lock, flags);

	_ca77xx_gpio_set_bit(bank, CA77XX_GPIO_CFG, offset);

	raw_spin_unlock_irqrestore(&bank->lock, flags);

	return 0;
}

static void ca77xx_gpio_free(struct gpio_chip *chip, unsigned int offset)
{
	struct ca77xx_gpio_bank *bank = gpiochip_get_data(chip);
	unsigned long flags;

	raw_spin_lock_irqsave(&bank->lock, flags);

	_ca77xx_gpio_set_bit(bank, CA77XX_GPIO_CFG, offset);

	raw_spin_unlock_irqrestore(&bank->lock, flags);

#ifdef CONFIG_PINCTRL
	pinctrl_gpio_free(chip->base + offset);
#else
	u32 value;

	value = ioread32(bank->mux_reg);
	value &= ~BIT(offset);
	iowrite32(value, bank->mux_reg);
#endif
}

static int ca77xx_gpio_get_direction(struct gpio_chip *chip,
				     unsigned int offset)
{
	struct ca77xx_gpio_bank *bank = gpiochip_get_data(chip);
	unsigned long flags;
	u32 val;

	raw_spin_lock_irqsave(&bank->lock, flags);

	val = _ca77xx_gpio_get_bit(bank, CA77XX_GPIO_CFG, offset);

	raw_spin_unlock_irqrestore(&bank->lock, flags);

	return (val == CA77XX_GPIO_CFG_IN) ? GPIOF_DIR_IN : GPIOF_DIR_OUT;
}

static int ca77xx_gpio_direction_input(struct gpio_chip *chip,
				       unsigned int offset)
{
	struct ca77xx_gpio_bank *bank = gpiochip_get_data(chip);
	unsigned long flags;

	raw_spin_lock_irqsave(&bank->lock, flags);

	_ca77xx_gpio_set_bit(bank, CA77XX_GPIO_CFG, offset);

	raw_spin_unlock_irqrestore(&bank->lock, flags);

	return 0;
}

static int ca77xx_gpio_direction_output(struct gpio_chip *chip,
					unsigned int offset, int output_value)
{
	struct ca77xx_gpio_bank *bank = gpiochip_get_data(chip);
	unsigned long flags;

	raw_spin_lock_irqsave(&bank->lock, flags);

	_ca77xx_gpio_mod_bit(bank, CA77XX_GPIO_OUT, offset, output_value);
	_ca77xx_gpio_clear_bit(bank, CA77XX_GPIO_CFG, offset);

	raw_spin_unlock_irqrestore(&bank->lock, flags);

	return 0;
}

/*
 * Return GPIO level
 */
static int ca77xx_gpio_get(struct gpio_chip *chip, unsigned int offset)
{
	struct ca77xx_gpio_bank *bank = gpiochip_get_data(chip);
	unsigned long flags;
	u32 value;
	int dir;

	dir = ca77xx_gpio_get_direction(chip, offset);

	raw_spin_lock_irqsave(&bank->lock, flags);

	if (dir == GPIOF_DIR_IN)
		value = _ca77xx_gpio_get_bit(bank, CA77XX_GPIO_IN, offset);
	else
		value = _ca77xx_gpio_get_bit(bank, CA77XX_GPIO_OUT, offset);

	raw_spin_unlock_irqrestore(&bank->lock, flags);

	return value;
}

/*
 * Set output GPIO level
 */
static void ca77xx_gpio_set(struct gpio_chip *chip, unsigned int offset,
			    int output_value)
{
	struct ca77xx_gpio_bank *bank = gpiochip_get_data(chip);
	unsigned long flags;

	/* this function only applies to output pin */
	if (ca77xx_gpio_get_direction(chip, offset) == GPIOF_DIR_IN)
		return;

	raw_spin_lock_irqsave(&bank->lock, flags);

	if (output_value)
		_ca77xx_gpio_set_bit(bank, CA77XX_GPIO_OUT, offset);
	else
		_ca77xx_gpio_clear_bit(bank, CA77XX_GPIO_OUT, offset);

	raw_spin_unlock_irqrestore(&bank->lock, flags);
}

static int ca77xx_gpio_to_irq(struct gpio_chip *chip, unsigned int offset)
{
	struct ca77xx_gpio_bank *bank = gpiochip_get_data(chip);

	if (!bank->domain)
		return -EINVAL;

	return irq_create_mapping(bank->domain, offset);
}

/* IRQ chip handlers */

/* Get ca77xx GPIO chip from irq data provided to generic IRQ callbacks */
static inline struct ca77xx_gpio_bank *irqd_to_gpio_bank(struct irq_data *data)
{
	return (struct ca77xx_gpio_bank *)data->domain->host_data;
}

static unsigned int ca77xx_gpio_startup_irq(struct irq_data *data)
{
	/*
	 * This warning indicates that the type of the irq hasn't been set
	 * before enabling the irq. This would normally be done by passing some
	 * trigger flags to request_irq().
	 */
	WARN(irqd_get_trigger_type(data) == IRQ_TYPE_NONE,
	     "irq type not set before enabling gpio irq %d", data->irq);

	irq_gc_ack_clr_bit(data);
	irq_gc_mask_set_bit(data);
	return 0;
}

static int ca77xx_gpio_set_irq_type(struct irq_data *data, unsigned int type)
{
	struct ca77xx_gpio_bank *bank = irqd_to_gpio_bank(data);
	unsigned int level;
	unsigned int edge;
	unsigned int both;
	unsigned long flags;

	switch (type) {
	case IRQ_TYPE_LEVEL_LOW:
		level = 0;
		edge = 0;
		both = 0;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		level = 1;
		edge = 0;
		both = 0;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		level = 0;
		edge = 1;
		both = 0;
		break;
	case IRQ_TYPE_EDGE_RISING:
		level = 1;
		edge = 1;
		both = 0;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		level = 0;
		edge = 1;
		both = 1;
		break;
	default:
	return -EINVAL;
	}

	raw_spin_lock_irqsave(&bank->lock, flags);

	_ca77xx_gpio_mod_bit(bank, CA77XX_GPIO_LVL, data->hwirq, level);
	_ca77xx_gpio_mod_bit(bank, CA77XX_GPIO_EDGE, data->hwirq, edge);
	_ca77xx_gpio_mod_bit(bank, CA77XX_GPIO_BOTHEDGE, data->hwirq, both);

	raw_spin_unlock_irqrestore(&bank->lock, flags);

	irq_setup_alt_chip(data, type);

	return 0;
}

#ifdef CONFIG_SUSPEND
static int ca77xx_gpio_set_irq_wake(struct irq_data *data, unsigned int on)
{
	struct ca77xx_gpio_bank *bank = irqd_to_gpio_bank(data);

#ifdef CONFIG_PM_DEBUG
	dev_info(bank->chip.parent, "irq_wake irq%d state:%d\n", data->irq, on);
#endif

	/* wake on gpio block interrupt */
	return irq_set_irq_wake(bank->irq, on);
}
#else
#define ca77xx_gpio_set_irq_wake NULL
#endif

static void ca77xx_gpio_irq_handler(struct irq_desc *desc)
{
	irq_hw_number_t hw;
	unsigned int irq_stat, irq_no;
	struct ca77xx_gpio_bank *bank;
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct irq_desc *child_desc;

	bank = (struct ca77xx_gpio_bank *)irq_desc_get_handler_data(desc);
	irq_stat = ca77xx_gpio_read(bank, CA77XX_GPIO_INT) &
		   ca77xx_gpio_read(bank, CA77XX_GPIO_IE);
	ca77xx_gpio_write(bank, CA77XX_GPIO_INT, irq_stat);

	chained_irq_enter(chip, desc);

	for (hw = 0; irq_stat; irq_stat >>= 1, ++hw) {
		if (!(irq_stat & 1))
			continue;

		irq_no = irq_linear_revmap(bank->domain, hw);
		child_desc = irq_to_desc(irq_no);
		generic_handle_irq_desc(child_desc);
	}

	chained_irq_exit(chip, desc);
}

static int ca77xx_gpio_bank_probe(struct ca77xx_gpio_bank_info *info)
{
	struct device_node *np = info->node;
	struct device *dev = info->priv->dev;
	struct ca77xx_gpio_bank *bank;
	struct irq_chip_generic *gc;
	unsigned int ngpios;
	int err;

	bank = devm_kzalloc(dev, sizeof(*bank), GFP_KERNEL);
	if (!bank)
		return -ENOMEM;

	raw_spin_lock_init(&bank->lock);

	/* Offset the main registers to the first register in this bank */
	bank->reg = info->priv->reg + info->index * GPIO_BANK_REG_OFFSET;
	if (info->priv->mux_reg)
		bank->mux_reg = info->priv->mux_reg + info->index * 4;

	/* Set up GPIO chip */
	snprintf(bank->label, sizeof(bank->label), "ca-gpio%u",
		 info->index);
	bank->chip.label		= bank->label;
	bank->chip.parent		= dev;
	bank->chip.request		= ca77xx_gpio_request;
	bank->chip.free			= ca77xx_gpio_free;
	bank->chip.get_direction	= ca77xx_gpio_get_direction;
	bank->chip.direction_input	= ca77xx_gpio_direction_input;
	bank->chip.direction_output	= ca77xx_gpio_direction_output;
	bank->chip.get			= ca77xx_gpio_get;
	bank->chip.set			= ca77xx_gpio_set;
	bank->chip.to_irq		= ca77xx_gpio_to_irq;
	bank->chip.of_node		= np;

	/* GPIO numbering from 0 */
	bank->chip.base			= info->index * GPIO_BANK_SIZE +
		info->priv->idx * GPIO_MAX_BANK_NUM * GPIO_BANK_SIZE;
	if (of_property_read_u32(np, "ngpios", &ngpios)) {
		dev_warn(dev, "Missing ngpios OF property\n");
		ngpios = GPIO_BANK_SIZE;
	}
	if (ngpios > GPIO_BANK_SIZE) {
		dev_err(dev, "ngpios property is not valid\n");
		ngpios = GPIO_BANK_SIZE;
	}
	bank->chip.ngpio = ngpios;

	/* Add the GPIO bank */
	err = devm_gpiochip_add_data(dev, &bank->chip, bank);
	if (err < 0) {
		dev_err(dev, "Couldn't add GPIO chip -- %d\n", err);
		return err;
	}

	info->bank = bank;

	/* Get the GPIO bank IRQ if provided */
	bank->irq = irq_of_parse_and_map(np, 0);

	/* The interrupt is optional (it may be used by another core on chip) */
	if (!bank->irq) {
		dev_info(dev, "IRQ not provided for bank %u, IRQs disabled\n",
			 info->index);
		return 0;
	}
	dev_info(dev, "Setting up IRQs for GPIO bank %u\n",
		 info->index);

	/* Add a virtual IRQ for each GPIO */
	bank->domain = irq_domain_add_linear(np,
					     bank->chip.ngpio,
					     &irq_generic_chip_ops,
					     bank);

	/* Set up a generic irq chip with 2 chip types (level and edge) */
	err = irq_alloc_domain_generic_chips(bank->domain, bank->chip.ngpio, 2,
					     bank->label, handle_bad_irq, 0, 0,
					     IRQ_GC_INIT_NESTED_LOCK);
	if (err) {
		dev_info(dev,
			 "irq_alloc_domain_generic_chips failed for bank %u, IRQs disabled\n",
			 info->index);
		irq_domain_remove(bank->domain);
		return 0;
	}

	gc = irq_get_domain_generic_chip(bank->domain, 0);
	gc->reg_base	= bank->reg;

	/* level chip type */
	gc->chip_types[0].type			= IRQ_TYPE_LEVEL_MASK;
	gc->chip_types[0].handler		= handle_level_irq;
	gc->chip_types[0].regs.ack		= CA77XX_GPIO_INT;
	gc->chip_types[0].regs.mask		= CA77XX_GPIO_IE;
	gc->chip_types[0].chip.name		= bank->label;
	gc->chip_types[0].chip.irq_startup	= ca77xx_gpio_startup_irq;
	gc->chip_types[0].chip.irq_ack		= irq_gc_ack_clr_bit;
	gc->chip_types[0].chip.irq_mask		= irq_gc_mask_clr_bit;
	gc->chip_types[0].chip.irq_unmask	= irq_gc_mask_set_bit;
	gc->chip_types[0].chip.irq_set_type	= ca77xx_gpio_set_irq_type;
	gc->chip_types[0].chip.irq_set_wake	= ca77xx_gpio_set_irq_wake;
	gc->chip_types[0].chip.flags		= IRQCHIP_MASK_ON_SUSPEND;

	/* edge chip type */
	gc->chip_types[1].type			= IRQ_TYPE_EDGE_BOTH;
	gc->chip_types[1].handler		= handle_edge_irq;
	gc->chip_types[1].regs.ack		= CA77XX_GPIO_INT;
	gc->chip_types[1].regs.mask		= CA77XX_GPIO_IE;
	gc->chip_types[1].chip.name		= bank->label;
	gc->chip_types[1].chip.irq_startup	= ca77xx_gpio_startup_irq;
	gc->chip_types[1].chip.irq_ack		= irq_gc_ack_clr_bit;
	gc->chip_types[1].chip.irq_mask		= irq_gc_mask_clr_bit;
	gc->chip_types[1].chip.irq_unmask	= irq_gc_mask_set_bit;
	gc->chip_types[1].chip.irq_set_type	= ca77xx_gpio_set_irq_type;
	gc->chip_types[1].chip.irq_set_wake	= ca77xx_gpio_set_irq_wake;
	gc->chip_types[1].chip.flags		= IRQCHIP_MASK_ON_SUSPEND;

	/* Setup chained handler for this GPIO bank */
	irq_set_chained_handler_and_data(bank->irq, ca77xx_gpio_irq_handler,
					 bank);

	return 0;
}

static void ca77xx_gpio_register_banks(struct ca77xx_gpio *priv)
{
	struct device_node *np = priv->dev->of_node;
	struct device_node *node;
	int i;

	for (i = 0; i < GPIO_MAX_BANK_NUM; i++)
		priv->info[i] = NULL;

	for_each_available_child_of_node(np, node) {
		struct ca77xx_gpio_bank_info *info;
		u32 addr;
		int ret;

		ret = of_property_read_u32(node, "reg", &addr);
		if (ret) {
			dev_err(priv->dev, "invalid reg on %pOF\n", node);
			continue;
		}
		if (addr >= GPIO_MAX_BANK_NUM) {
			dev_err(priv->dev, "index %u in %pOF out of range\n",
				addr, node);
			continue;
		}

		info = devm_kzalloc(priv->dev, sizeof(*info), GFP_KERNEL);
		if (!info)
			return;

		info->index = addr;
		info->node = of_node_get(node);
		info->priv = priv;
		info->bank = NULL;

		ret = ca77xx_gpio_bank_probe(info);
		if (ret) {
			dev_err(priv->dev, "failure registering %pOF\n", node);
			of_node_put(node);
			continue;
		}

		priv->info[info->index] = info;
	}
}

int ca77xx_gpio_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct resource *res_regs;
	struct ca77xx_gpio *priv;

	if (!np) {
		dev_err(dev, "must be instantiated via devicetree\n");
		return -ENOENT;
	}

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = dev;
	platform_set_drvdata(pdev, priv);

	res_regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->reg = devm_ioremap_resource(dev, res_regs);
	if (!priv->reg) {
		dev_err(dev, "unable to ioremap registers\n");
		return -ENOMEM;
	}
	dev_info(dev, "resource - %pr mapped at 0x%pK\n", res_regs, priv->reg);

#ifndef CONFIG_PINCTRL
	res_regs = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	priv->mux_reg = devm_ioremap_resource(dev, res_regs);
	if (!priv->mux_reg) {
		dev_err(dev, "unable to ioremap registers\n");
		return -ENOMEM;
	}
	dev_info(dev, "resource - %pr mapped at 0x%pK\n", res_regs,
		 priv->mux_reg);
#endif

	if (of_property_read_u32(np, "id", &priv->idx)) {
		dev_err(dev, "missing id property\n");
		return -EIDRM;
	}
	dev_info(dev, "id %d\n", priv->idx);

	ca77xx_gpio_register_banks(priv);

	return 0;
}

static int ca77xx_gpio_remove(struct platform_device *pdev)
{
	struct ca77xx_gpio *priv = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < GPIO_MAX_BANK_NUM; i++) {
		if (priv->info[i])
			gpiochip_remove(&priv->info[i]->bank->chip);
	}

	return 0;
}

const struct of_device_id ca77xx_gpio_of_match[] = {
	{ .compatible = "cortina,ca77xx-gpio" },
	{ },
};

static struct platform_driver ca77xx_gpio_driver = {
	.driver = {
		.name		= "ca77xx-gpio",
		.of_match_table	= ca77xx_gpio_of_match,
	},
	.probe		= ca77xx_gpio_probe,
	.remove		= ca77xx_gpio_remove,
};

module_platform_driver(ca77xx_gpio_driver);

MODULE_DESCRIPTION("Cortina Access CA77XX GPIO Driver");
MODULE_LICENSE("GPL v2");
