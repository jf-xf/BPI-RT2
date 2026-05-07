/*
 * Pin control driver for Cortina-Access CA77XX SoCs
 *
 * Copyright (C) 2017 Cortina Access, Inc.
 *		 http://www.cortina-access.com
 *
 * Based on pinctrl-digicolor.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/gpio/driver.h>
#include <linux/spinlock.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#ifdef CONFIG_ARCH_CORTINA_SMC
#include <soc/cortina/cortina-smc.h>
#endif
#include <soc/cortina/cortina-soc.h>
#include "pinctrl-utils.h"

#define DRIVER_NAME	"pinctrl-cortina"

static u8 ARCH_OFFSET = 0;

#define PIN_MUX_OFFSET		0x00
#define PIN_MUX_NEW_OFFSET	0x04 /* only Taurus */
#define SD_IO_PULL_CONTROL	0x08 /* only Taurus */
#define IO_DRV_OFFSET		(0x04 + ARCH_OFFSET)
#define GPIO0_MUX_OFFSET	(0x08 + ARCH_OFFSET)
#define GPIO1_MUX_OFFSET	(0x0C + ARCH_OFFSET)
#define GPIO2_MUX_OFFSET	(0x10 + ARCH_OFFSET)
#define GPIO3_MUX_OFFSET	(0x14 + ARCH_OFFSET)
#define GPIO4_MUX_OFFSET	(0x18 + ARCH_OFFSET)


/*** Taurus ****************
0xf43200f0 GLOBAL_PIN_MUX
0xf43200f4 GLOBAL_PIN_MUX_NEW
0xf43200f8 GLOBAL_SD_IO_PULL_CONTROL
0xf43200fc GLOBAL_IO_DRIVE_CONTROL
0xf4320100 GLOBAL_GPIO_MUX_0
0xf4320104 GLOBAL_GPIO_MUX_1
0xf4320108 GLOBAL_GPIO_MUX_2
0xf432010c GLOBAL_GPIO_MUX_3
0xf4320110 GLOBAL_GPIO_MUX_4
**********************************/


/* G3/G3HGU equips 5 GPIO, but 4 for SFU/SFP */
#if defined(CONFIG_ARCH_CORTINA_G3) || defined(CONFIG_ARCH_CORTINA_G3HGU) || defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS)
#define GPIO_GROUP			5
#else
#define GPIO_GROUP			4
#endif
#ifdef CONFIG_ARCH_REALTEK_TAURUS
#define PIN_COLLECTIONS		(GPIO_GROUP + 2)	/* GPIO_MUX_0~4 + PIN_MUX */
#else
#define PIN_COLLECTIONS		(GPIO_GROUP + 1)	/* GPIO_MUX_0~4 + PIN_MUX */
#endif
#define PINS_PER_COLLECTION	32
#define PINS_COUNT		(PIN_COLLECTIONS * PINS_PER_COLLECTION)

struct ca_pinmap {
	void __iomem		*regs;
	unsigned int		physical_addr;
	struct device		*dev;
	struct pinctrl_dev	*pctl;

	struct pinctrl_desc	*desc;
	const char		*pin_names[PINS_COUNT];

	struct gpio_chip	chip;
	spinlock_t		lock;
	char	is_taurus;
};

static int ca_get_groups_count(struct pinctrl_dev *pctldev)
{
	return PINS_COUNT;
}

static const char *ca_get_group_name(struct pinctrl_dev *pctldev,
				     unsigned selector)
{
	struct ca_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);

	/* Exactly one group per pin */
	return pmap->desc->pins[selector].name;
}

static int ca_get_group_pins(struct pinctrl_dev *pctldev, unsigned selector,
			     const unsigned **pins,
			     unsigned *num_pins)
{
	struct ca_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);

	*pins = &pmap->desc->pins[selector].number;
	*num_pins = 1;

	return 0;
}

static struct pinctrl_ops ca_pinctrl_ops = {
	.get_groups_count	= ca_get_groups_count,
	.get_group_name		= ca_get_group_name,
	.get_group_pins		= ca_get_group_pins,
	.dt_node_to_map		= pinconf_generic_dt_node_to_map_pin,
	.dt_free_map		= pinctrl_utils_free_map,
};

 const char *const ca_functions[] = {
	"gpio",
	"multi-fn",
};

static int ca_get_functions_count(struct pinctrl_dev *pctldev)
{
	return ARRAY_SIZE(ca_functions);
}

static const char *ca_get_fname(struct pinctrl_dev *pctldev, unsigned selector)
{
	return ca_functions[selector];
}

static int ca_get_groups(struct pinctrl_dev *pctldev, unsigned selector,
			 const char * const **groups,
			 unsigned * const num_groups)
{
	struct ca_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);

	*groups = pmap->pin_names;
	*num_groups = PINS_COUNT;

	return 0;
}

static bool is_valid_gpio_pin(int pin_num)
{
#ifdef CONFIG_ARCH_REALTEK_TAURUS
	/*
	 * GPIO1[4] ~ GPIO1[9]
	 * GPIO1[11] ~ GPIO1[14]
	 * are not valid in taurus
	 */
	if ((pin_num >= 36 && pin_num <= 41) || (pin_num >= 43 && pin_num <= 46))
		return 0;
	else
#endif
		return 1;
}

static void ca_client_sel(const struct ca_pinmap *pmap, int pin_num, int *reg, int *bit)
{
	*bit = (pin_num % PINS_PER_COLLECTION);

	if(pin_num >= (PINS_PER_COLLECTION*GPIO_GROUP)){	/* PIN_MUX */
		*reg = pmap->is_taurus ? ((pin_num-PINS_PER_COLLECTION*GPIO_GROUP) / (PINS_PER_COLLECTION) * 4) : PIN_MUX_OFFSET;
	} else {
		if (is_valid_gpio_pin(pin_num))
			/* Skip IO_DRIVE_CONTROL */
			*reg = (pin_num/PINS_PER_COLLECTION)*4 + GPIO0_MUX_OFFSET;
		else
			*reg = 0;
	}
}

static int is_pinmux(struct ca_pinmap *pmap, int reg_off) {
	int hi;
	hi = pmap->is_taurus ? PIN_MUX_NEW_OFFSET : PIN_MUX_OFFSET;
	return (reg_off <= hi);
}

static int ca_set_mux(struct pinctrl_dev *pctldev, unsigned selector,
		      unsigned group)
{
	struct ca_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);
	int bit_off, reg_off;
	u32 reg;

	ca_client_sel(pmap, group, &reg_off, &bit_off);

	printk("%s: group:%x reg_off:%x bit_off:%x\n",__func__, group, reg_off, bit_off);

#ifdef CONFIG_ARCH_CORTINA_SMC
	if(is_pinmux(pmap, reg_off)) /* PINMUX is secured register, invoke SMC */
		reg = CA_SMC_CALL_REG_READ(pmap->physical_addr);
	else
		reg = readl_relaxed(pmap->regs + reg_off);
#else
	reg = readl_relaxed(pmap->regs + reg_off);
#endif
	if(is_pinmux(pmap, reg_off)){	/* PIN_MUX set 1 as multi-function */
		reg |= (1<< bit_off);
	} else {			/* GPIO_MUX set 0 as multi-function */
		reg &= ~(1<< bit_off);
	}
#ifdef CONFIG_ARCH_CORTINA_SMC
	if(is_pinmux(pmap, reg_off)) /* PINMUX is secured register, invoke SMC */
		CA_SMC_CALL_REG_WRITE(pmap->physical_addr, reg);
	else
		writel_relaxed(reg, pmap->regs + reg_off);
#else
	writel_relaxed(reg, pmap->regs + reg_off);
#endif

	return 0;
}

static int ca_pmx_request_gpio(struct pinctrl_dev *pcdev,
			       struct pinctrl_gpio_range *range,
			       unsigned offset)
{
	struct ca_pinmap *pmap = pinctrl_dev_get_drvdata(pcdev);
	int bit_off, reg_off;
	u32 reg;

	ca_client_sel(pmap, offset, &reg_off, &bit_off);

	printk("%s: offset:%x reg_off:%x bit_off:%x\n",__func__, offset, reg_off, bit_off);

#ifdef CONFIG_ARCH_CORTINA_SMC
	if(is_pinmux(pmap, reg_off)) /* PINMUX is secured register, invoke SMC */
		reg = CA_SMC_CALL_REG_READ(pmap->physical_addr);
	else
		reg = readl_relaxed(pmap->regs + reg_off);
#else
	reg = readl_relaxed(pmap->regs + reg_off);
#endif
	if(is_pinmux(pmap, reg_off)){	/* Overflow! */
		printk("\033[1;31mERROR! request invalid GPIO pin %d\033[0m\n", offset);
		return -EBUSY;
	} else {			/* GPIO_MUX set 0 as multi-function */
		reg |= (1<< bit_off);
	}
#ifdef CONFIG_ARCH_CORTINA_SMC
	if(is_pinmux(pmap, reg_off)) /* PINMUX is secured register, invoke SMC */
		CA_SMC_CALL_REG_WRITE(pmap->physical_addr, reg);
	else
		writel_relaxed(reg, pmap->regs + reg_off);
#else
	writel_relaxed(reg, pmap->regs + reg_off);
#endif

	return 0;
}

static struct pinmux_ops ca_pmxops = {
	.get_functions_count	= ca_get_functions_count,
	.get_function_name	= ca_get_fname,
	.get_function_groups	= ca_get_groups,
	.set_mux		= ca_set_mux,
	.gpio_request_enable	= ca_pmx_request_gpio,
};

static int ca_pinctrl_probe(struct platform_device *pdev)
{
	struct ca_pinmap *pmap;
	struct resource *r;
	struct pinctrl_pin_desc *pins;
	struct pinctrl_desc *pctl_desc;
	char *pin_names;
	int name_len = strlen("GPIO_xxx") + 1;	/* or PMUX_xxx */
	int i, j;

	pmap = devm_kzalloc(&pdev->dev, sizeof(*pmap), GFP_KERNEL);
	if (!pmap)
		return -ENOMEM;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
#ifdef CONFIG_ARCH_CORTINA_SMC
	pmap->physical_addr = (unsigned int)r->start;
#endif
	pmap->regs = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(pmap->regs))
		return PTR_ERR(pmap->regs);

	pins = devm_kzalloc(&pdev->dev, sizeof(*pins)*PINS_COUNT, GFP_KERNEL);
	if (!pins)
		return -ENOMEM;
	pin_names = devm_kzalloc(&pdev->dev, name_len * PINS_COUNT,
				 GFP_KERNEL);
	if (!pin_names)
		return -ENOMEM;

	for (i = 0; i < GPIO_GROUP; i++) {	/* GPIO_MUX */
		for (j = 0; j < PINS_PER_COLLECTION; j++) {
			int pin_id = i*PINS_PER_COLLECTION + j;
			char *name = &pin_names[pin_id * name_len];

			snprintf(name, name_len, "GPIO_%c%c%c", '0'+(i), '0'+(j/10), '0'+(j%10));
			pins[pin_id].number = pin_id;
			pins[pin_id].name = name;
			pmap->pin_names[pin_id] = name;
		}
	}

	for (i = GPIO_GROUP; i < PIN_COLLECTIONS; i++) {	/* PIN_MUX */
		for (j = 0; j < PINS_PER_COLLECTION; j++) {
			int pin_id = i*PINS_PER_COLLECTION + j;
			char *name = &pin_names[pin_id * name_len];

			snprintf(name, name_len, "PMUX_%c%c%c", '0'+(i-GPIO_GROUP), '0'+(j/10), '0'+(j%10));
			pins[pin_id].number = pin_id;
			pins[pin_id].name = name;
			pmap->pin_names[pin_id] = name;
		}
	}

	pctl_desc = devm_kzalloc(&pdev->dev, sizeof(*pctl_desc), GFP_KERNEL);
	if (!pctl_desc)
		return -ENOMEM;

	pctl_desc->name	= DRIVER_NAME,
	pctl_desc->owner = THIS_MODULE,
	pctl_desc->pctlops = &ca_pinctrl_ops,
	pctl_desc->pmxops = &ca_pmxops,
	pctl_desc->npins = PINS_COUNT;
	pctl_desc->pins = pins;
	pmap->desc = pctl_desc;

	pmap->dev = &pdev->dev;
#if defined(CONFIG_ARCH_CORTINA_VENUS) || defined(CONFIG_ARCH_REALTEK_TAURUS)
	if(IS_TAURUS){
		ARCH_OFFSET = 8;
		pmap->is_taurus = 1;
		dev_info(&pdev->dev, "pinctrl driver registration: ARCH_OFFSET=%u\n", ARCH_OFFSET);
	}
#endif
	pmap->pctl = pinctrl_register(pctl_desc, &pdev->dev, pmap);
	if (IS_ERR(pmap->pctl)) {
		dev_err(&pdev->dev, "pinctrl driver registration failed\n");
		return PTR_ERR(pmap->pctl);
	}

#ifdef CONFIG_LEDS_CA77XX_PHY_2DIR
	{
	u32 reg;

	reg = readl_relaxed(pmap->regs + IO_DRV_OFFSET);
	reg |= BIT(9);	/* flash_ds for PHY Tx/Rx monitor */
	writel_relaxed(reg, pmap->regs + IO_DRV_OFFSET);
	}
#endif

	return 0;
}

static int ca_pinctrl_remove(struct platform_device *pdev)
{
	struct ca_pinmap *pmap = platform_get_drvdata(pdev);

	pinctrl_unregister(pmap->pctl);
	gpiochip_remove(&pmap->chip);

	return 0;
}

static const struct of_device_id ca_pinctrl_ids[] = {
	{ .compatible = "cortina,g3-pinctrl" },
	{ .compatible = "cortina,saturn-pinctrl" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ca_pinctrl_ids);

static struct platform_driver ca_pinctrl_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = ca_pinctrl_ids,
	},
	.probe = ca_pinctrl_probe,
	.remove = ca_pinctrl_remove,
};
module_platform_driver(ca_pinctrl_driver);
