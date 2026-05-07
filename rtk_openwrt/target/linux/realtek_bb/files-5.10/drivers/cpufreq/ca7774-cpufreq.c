/*
 * Cortina CA7774 CPUFreq Support
 *
 * Copyright (c) 2017-2018 Cortina Access Inc.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

static const struct of_device_id machine_matches[] = {
	{ .compatible = "cortina,ca7774", },
	{ .compatible = "cortina,ca8289", },
	{ .compatible = "cortina,ca8277B", },
	{ .compatible = "realtek,taurus", },
	{ .compatible = "realtek,9607f", },
	{}
};
MODULE_DEVICE_TABLE(of, machine_matches);

static int __init ca7774_cpufreq_driver_init(void)
{
	struct platform_device *pdev;
	int cnt = sizeof(machine_matches) / sizeof(const struct of_device_id);
	int ret;
	int i;

	for (i = 0; i < cnt; i++) {
		ret = of_machine_is_compatible(machine_matches[i].compatible);
		if (ret)
			break;
	}

	if (!ret)
		return -ENODEV;

	pdev = platform_device_register_simple("cpufreq-dt", -1, NULL, 0);
	return PTR_ERR_OR_ZERO(pdev);
}
module_init(ca7774_cpufreq_driver_init);

MODULE_AUTHOR("Alex Nemirovsky <alex.nemirovsky@cortina-access.com>");
MODULE_DESCRIPTION("Cortina CA7774 cpufreq driver");
MODULE_LICENSE("GPL v2");
