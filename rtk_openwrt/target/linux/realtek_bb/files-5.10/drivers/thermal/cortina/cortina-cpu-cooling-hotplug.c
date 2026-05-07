/*
 * Cortina CA8279 CPU Hot-Plug for thermal zone
 *
 * Copyright (c) 2018 Cortina-Access Inc.
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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/thermal.h>
#include <linux/cpu.h>

struct cpu_hotplug_data {
	struct platform_device *pdev;
	/* Cooling device if any */
	struct thermal_cooling_device *cdev;
	struct mutex lock;
	unsigned int number_of_state;
	int state;
};

static int cpu_hotplug_get_max_state(struct thermal_cooling_device *cdev,
									 unsigned long *state)
{
	struct cpu_hotplug_data *cpu_hotplug_data = cdev->devdata;

	if (!cpu_hotplug_data)
		return -EINVAL;

	*state = cpu_hotplug_data->number_of_state - 1;
	return 0;
}

static int cpu_hotplug_get_cur_state(struct thermal_cooling_device *cdev,
									 unsigned long *state)
{
	struct cpu_hotplug_data *cpu_hotplug_data = cdev->devdata;

	if (!cpu_hotplug_data)
		return -EINVAL;

	*state = cpu_hotplug_data->state;

	return 0;
}

static int cpu_hotplug_set_cur_state(struct thermal_cooling_device *cdev,
									 unsigned long state)
{
	struct cpu_hotplug_data *cpu_hotplug_data = cdev->devdata;

	if (!cpu_hotplug_data)
		return -EINVAL;

	if (state >= cpu_hotplug_data->number_of_state)
		return -EINVAL;

	/* Nothing to do if state not change */
	if (cpu_hotplug_data->state == state)
		return 0;

	if (state > cpu_hotplug_data->state)
		cpu_down(cpu_hotplug_data->number_of_state - state);
	else
		cpu_up(cpu_hotplug_data->number_of_state - cpu_hotplug_data->state);

	cpu_hotplug_data->state = state;
	return 0;
}

static const struct thermal_cooling_device_ops cpu_hotplug_cool_ops = {
	.get_max_state = cpu_hotplug_get_max_state,
	.get_cur_state = cpu_hotplug_get_cur_state,
	.set_cur_state = cpu_hotplug_set_cur_state,
};

static const struct of_device_id of_cpu_hotplug_match[] = {
	{.compatible = "thermal-cpu-hotplug"},
	{},
};

static int cpu_hotplug_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct thermal_cooling_device *cdev;
	struct cpu_hotplug_data *cpu_hotplug_data;
	int num;

	cpu_hotplug_data = devm_kzalloc(&pdev->dev, sizeof(struct cpu_hotplug_data),
									GFP_KERNEL);
	if (!cpu_hotplug_data)
		return -ENOMEM;

	cpu_hotplug_data->pdev = pdev;
	platform_set_drvdata(pdev, cpu_hotplug_data);
	mutex_init(&cpu_hotplug_data->lock);

	if (!of_property_read_u32(np, "cooling-max-level", &num))
		cpu_hotplug_data->number_of_state = num + 1;
	printk("Num of state:%d\n", cpu_hotplug_data->number_of_state);

	if (IS_ENABLED(CONFIG_THERMAL)) {
		cdev =
			thermal_of_cooling_device_register(pdev->dev.of_node,
											   "thermal-cpu-hotplug",
											   cpu_hotplug_data,
											   &cpu_hotplug_cool_ops);
		if (IS_ERR(cdev)) {
			dev_err(&pdev->dev,
					"Failed to register CPU hot-plug as cooling device");
			return PTR_ERR(cdev);
		}
		cpu_hotplug_data->cdev = cdev;
		thermal_cdev_update(cdev);
	}

	dev_info(&pdev->dev, "CPU hotplug for Thermal Zone\n");

	return 0;
}

static int cpu_hotplug_remove(struct platform_device *pdev)
{
	struct cpu_hotplug_data *cpu_hotplug_data = platform_get_drvdata(pdev);

	if (!IS_ERR(cpu_hotplug_data->cdev))
		thermal_cooling_device_unregister(cpu_hotplug_data->cdev);

	return 0;
}

static void cpu_hotplug_shutdown(struct platform_device *pdev)
{
	cpu_hotplug_remove(pdev);
}

static struct platform_driver cpu_hotplug_driver = {
	.probe = cpu_hotplug_probe,
	.remove = cpu_hotplug_remove,
	.shutdown = cpu_hotplug_shutdown,
	.driver = {
			   .name = "cpu-hotplug",
			   .of_match_table = of_match_ptr(of_cpu_hotplug_match),
			   },
};

module_platform_driver(cpu_hotplug_driver);

MODULE_AUTHOR("Jason Li <jason.li@cortina-access.com>");
MODULE_DESCRIPTION("CPU Hot-Plug driver for thermal-zone");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:CPU hot-plug");
