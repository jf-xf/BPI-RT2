// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Taurus Packet Engine Reset Controller Driver
 *
 * Based on reset-simple.c
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>
#include <linux/spinlock.h>
#include <soc/cortina/registers.h>
#include <linux/slab.h>

#define GLOBAL_PE_CTRL_TAROKO0  0x9030306c
#define GLOBAL_PE_CTRL_TAROKO1  0x90303074
#define PE_CTRL_TAROKO_SIZE		8
#define ALL_SD_BIT				5
#define NUM_CPUS				2

#define to_taurus_pe_reset_attr(x) container_of(x, struct taurus_pe_reset_attribute, attr)
#define kobj_to_taurus_pe_reset_cpu(x) container_of(x, struct taurus_pe_reset_cpu, kobj)

struct taurus_pe_reset_priv;

struct taurus_pe_reset_cpu {
	struct kobject kobj;
	unsigned int cpu_id;
	u32 duration_ms;
	struct taurus_pe_reset_priv *priv;
};

struct taurus_pe_reset_priv {
	spinlock_t			lock;
	struct kref refcount;
	struct reset_controller_dev	rcdev;
	struct taurus_pe_reset_cpu cpus[];
};

struct taurus_pe_reset_attribute {
	struct attribute attr;
	ssize_t (*show)(struct taurus_pe_reset_cpu *cpu,
			struct taurus_pe_reset_attribute *attr, char *buf);
	ssize_t (*store)(struct taurus_pe_reset_cpu *cpu,
			 struct taurus_pe_reset_attribute *attr,
			 const char *buf, size_t count);
};

static void taurus_check_global_bist_status(void)
{
	void __iomem *bist_ctl13_addr;
	GLOBAL_BIST_CONTROL13_t bist_ctl13_reg;

	bist_ctl13_addr = ioremap(GLOBAL_BIST_CONTROL13, 4);
	bist_ctl13_reg.wrd = readl(bist_ctl13_addr);

	if(bist_ctl13_reg.wrd & BIT(18))
	{
		bist_ctl13_reg.wrd &= ~BIT(18);
		writel(bist_ctl13_reg.wrd, bist_ctl13_addr);
	}

	iounmap(bist_ctl13_addr);
}

static int taurus_pe_reset_assert(struct reset_controller_dev *rcdev,
			       unsigned long cpu_id)
{
	struct taurus_pe_reset_priv *priv = container_of(rcdev,
							struct taurus_pe_reset_priv,
							rcdev);
	unsigned long flags;
	void __iomem *block_reset_addr, *pe_ctl_addr;
	GLOBAL_BLOCK_RESET_t blk_rst_reg;
	u32 pe_ctl_reg;

	if(cpu_id > 1)
	{
		dev_err(rcdev->dev, "Invalid cpu id %ld.\n", cpu_id);
		return -EINVAL;
	}

	block_reset_addr = ioremap(GLOBAL_BLOCK_RESET, 4);
	pe_ctl_addr = ioremap(GLOBAL_PE_CTRL_TAROKO0 + (cpu_id * PE_CTRL_TAROKO_SIZE), 4);

	spin_lock_irqsave(&priv->lock, flags);

	blk_rst_reg.wrd = readl(block_reset_addr);
	switch (cpu_id) {
		case 0:
			blk_rst_reg.bf.reset_pe0 = 1;
			blk_rst_reg.bf.reset_rcpu0 = 1;
			break;
		case 1:
			blk_rst_reg.bf.reset_pe1 = 1;
			blk_rst_reg.bf.reset_rcpu1 = 1;
			break;
	}
	writel(blk_rst_reg.wrd, block_reset_addr);

	pe_ctl_reg = readl(pe_ctl_addr);
	pe_ctl_reg |= BIT(ALL_SD_BIT);
	writel(pe_ctl_reg, pe_ctl_addr);

	spin_unlock_irqrestore(&priv->lock, flags);

	iounmap(block_reset_addr);
	iounmap(pe_ctl_addr);

	return 0;
}

static int taurus_pe_reset_deassert(struct reset_controller_dev *rcdev,
					unsigned long cpu_id)
{
	struct taurus_pe_reset_priv *priv = container_of(rcdev,
						     struct taurus_pe_reset_priv,
						     rcdev);
	unsigned long flags;
	void __iomem *block_reset_addr, *pe_ctl_addr;
	GLOBAL_BLOCK_RESET_t blk_rst_reg;
	u32 pe_ctl_reg;

	if(cpu_id > 1)
	{
		dev_err(rcdev->dev, "Invalid cpu id %ld.\n", cpu_id);
		return -EINVAL;
	}

	block_reset_addr = ioremap(GLOBAL_BLOCK_RESET, 4);
	pe_ctl_addr = ioremap(GLOBAL_PE_CTRL_TAROKO0 + (cpu_id * PE_CTRL_TAROKO_SIZE), 4);

	spin_lock_irqsave(&priv->lock, flags);

	pe_ctl_reg = readl(pe_ctl_addr);
	pe_ctl_reg &= ~BIT(ALL_SD_BIT);
	writel(pe_ctl_reg, pe_ctl_addr);

	blk_rst_reg.wrd = readl(block_reset_addr);
	switch (cpu_id) {
		case 0:
			blk_rst_reg.bf.reset_pe0 = 0;
			break;
		case 1:
			blk_rst_reg.bf.reset_pe1 = 0;
			break;
	}
	writel(blk_rst_reg.wrd, block_reset_addr);
	mdelay(1);

	switch (cpu_id) {
		case 0:
			blk_rst_reg.bf.reset_rcpu0 = 0;
			break;
		case 1:
			blk_rst_reg.bf.reset_rcpu1 = 0;
			break;
	}
	writel(blk_rst_reg.wrd, block_reset_addr);

	spin_unlock_irqrestore(&priv->lock, flags);

	iounmap(block_reset_addr);
	iounmap(pe_ctl_addr);

	return 0;
}

static int reset_taurus_pe_reset(struct reset_controller_dev *rcdev,
			      unsigned long cpu_id)
{
	struct taurus_pe_reset_priv *priv = container_of(rcdev,
						     struct taurus_pe_reset_priv,
						     rcdev);
	int ret;

	ret = taurus_pe_reset_assert(rcdev, cpu_id);

	if(ret < 0)
		return ret;

	if (priv->cpus[cpu_id].duration_ms > 0)
		msleep(priv->cpus[cpu_id].duration_ms);

	ret = taurus_pe_reset_deassert(rcdev, cpu_id);

	return ret;
}

static void taurus_pe_reset_free_priv(struct kref *ref)
{
	struct taurus_pe_reset_priv *priv = container_of(ref,
					struct taurus_pe_reset_priv, refcount);

	kfree(priv);
}

static void taurus_pe_reset_release_kobj(struct kobject *kobj)
{
	struct taurus_pe_reset_cpu *cpu;

	cpu = kobj_to_taurus_pe_reset_cpu(kobj);

	kref_put(&cpu->priv->refcount, taurus_pe_reset_free_priv);
}

static ssize_t taurus_pe_reset_attr_show(struct kobject *kobj,
				    struct attribute *attr,
				    char *buf)
{
	struct taurus_pe_reset_attribute *attribute;
	struct taurus_pe_reset_cpu *cpu;

	attribute = to_taurus_pe_reset_attr(attr);
	cpu = kobj_to_taurus_pe_reset_cpu(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(cpu, attribute, buf);
}

static ssize_t taurus_pe_reset_attr_store(struct kobject *kobj,
				     struct attribute *attr,
				     const char *buf, size_t len)
{
	struct taurus_pe_reset_attribute *attribute;
	struct taurus_pe_reset_cpu *cpu;

	attribute = to_taurus_pe_reset_attr(attr);
	cpu = kobj_to_taurus_pe_reset_cpu(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(cpu, attribute, buf, len);
}

static ssize_t control_store(struct taurus_pe_reset_cpu *cpu,
			     struct taurus_pe_reset_attribute *attr,
			     const char *buf, size_t count)
{
	char action[10];
	char *eol;

	strncpy(action, buf, min(count, sizeof(action)));
	action[sizeof(action) - 1] = '\0';
	eol = strrchr(action, '\n');
	if (eol)
		*eol = '\0';

	if (!strcmp("reset", action))
		reset_taurus_pe_reset(&cpu->priv->rcdev, cpu->cpu_id);
	else if (!strcmp("assert", action))
		taurus_pe_reset_assert(&cpu->priv->rcdev, cpu->cpu_id);
	else if (!strcmp("deassert", action))
		taurus_pe_reset_deassert(&cpu->priv->rcdev, cpu->cpu_id);
	else
		return -EINVAL;

	return count;
}

static ssize_t duration_ms_show(struct taurus_pe_reset_cpu *cpu,
				struct taurus_pe_reset_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%d\n", cpu->duration_ms);
}

static ssize_t duration_ms_store(struct taurus_pe_reset_cpu *cpu,
				 struct taurus_pe_reset_attribute *attr,
				 const char *buf, size_t count)
{
	if (kstrtou32(buf, 0, &cpu->duration_ms) < 0)
		return -EINVAL;

	return count;
}

static struct taurus_pe_reset_attribute control_attribute = __ATTR_WO(control);
static struct taurus_pe_reset_attribute duration_attribute = __ATTR_RW(duration_ms);

static struct attribute *taurus_pe_reset_attrs[] = {
	&control_attribute.attr,
	&duration_attribute.attr,
	NULL
};

static const struct sysfs_ops taurus_pe_reset_sysfs_ops = {
	.show = taurus_pe_reset_attr_show,
	.store = taurus_pe_reset_attr_store,
};

static struct kobj_type taurus_pe_reset_ktype = {
	.release = taurus_pe_reset_release_kobj,
	.sysfs_ops = &taurus_pe_reset_sysfs_ops,
	.default_attrs = taurus_pe_reset_attrs,
};

static int taurus_pe_reset_create_sysfs(struct taurus_pe_reset_cpu *cpu)
{
	int ret;

	ret = kobject_init_and_add(&cpu->kobj, &taurus_pe_reset_ktype,
				   &cpu->priv->rcdev.dev->kobj, "cpu%d",
				   cpu->cpu_id);

	kref_get(&cpu->priv->refcount); /* kobject part of private structure */

	if (ret) {
		kobject_put(&cpu->kobj);
		return ret;
	}

	kobject_uevent(&cpu->kobj, KOBJ_ADD);

	return 0;
}

static void taurus_pe_reset_destroy_sysfs(struct taurus_pe_reset_cpu *cpu)
{
	kobject_put(&cpu->kobj);
}

const struct reset_control_ops taurus_pe_reset_ops = {
	.assert		= taurus_pe_reset_assert,
	.deassert	= taurus_pe_reset_deassert,
    .reset		= reset_taurus_pe_reset,
};

static const struct of_device_id taurus_pe_reset_dt_ids[] = {
	{ .compatible = "rtk,taurus-pe-reset" },
	{ /* sentinel */ },
};

static void taurus_pe_reset_free_cpu(struct taurus_pe_reset_cpu *cpu)
{
	taurus_pe_reset_destroy_sysfs(cpu);
}

static int taurus_pe_reset_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct taurus_pe_reset_priv *priv;
	struct taurus_pe_reset_cpu *cpu;
	int ret;
	int i;

	priv = devm_kzalloc(dev, sizeof(*priv) + sizeof(*cpu) * NUM_CPUS, GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	spin_lock_init(&priv->lock);
	priv->rcdev.owner = THIS_MODULE;
	priv->rcdev.ops = &taurus_pe_reset_ops;
	priv->rcdev.of_node = dev->of_node;
	priv->rcdev.dev = dev;
	priv->rcdev.nr_resets = NUM_CPUS;

	cpu = priv->cpus;

	kref_init(&priv->refcount);

	for (i = 0; i < NUM_CPUS; i++) {
		of_property_read_u32(np, "duration-ms", &cpu->duration_ms);
		cpu->cpu_id = i;
		cpu->priv = priv;
		ret = taurus_pe_reset_create_sysfs(cpu);
		if (ret)
			goto rollback;
		cpu++;
	}

	taurus_pe_reset_assert(&priv->rcdev, 0);
	taurus_pe_reset_assert(&priv->rcdev, 1);
	taurus_check_global_bist_status();

	ret = reset_controller_register(&priv->rcdev);
	platform_set_drvdata(pdev, priv);

	if (ret)
	{
		dev_err(dev, "Failed to register taurus pe reset controller!\n");
		return ret;
	}

	return 0;

rollback:
	while (cpu >= priv->cpus)
		taurus_pe_reset_free_cpu(cpu--);

	kref_put(&priv->refcount, taurus_pe_reset_free_priv);

	return ret;
}

static int taurus_pe_reset_remove(struct platform_device *pdev)
{
	struct taurus_pe_reset_priv *priv = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < NUM_CPUS; i++)
		taurus_pe_reset_free_cpu(&priv->cpus[i]);

	reset_controller_unregister(&priv->rcdev);
	kref_put(&priv->refcount, taurus_pe_reset_free_priv);

	return 0;
}

static struct platform_driver taurus_pe_reset_driver = {
	.probe	= taurus_pe_reset_probe,
	.remove	= taurus_pe_reset_remove,
	.driver = {
		.name		= "taurus-pe-reset",
		.of_match_table	= taurus_pe_reset_dt_ids,
	},
};
module_platform_driver(taurus_pe_reset_driver);

MODULE_DESCRIPTION("Taurus Reset Packet Engine Controller Driver");
MODULE_LICENSE("GPL");