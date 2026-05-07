/*
 * Copyright (C) 2015 Cortina Access, Inc.
 *              http://www.cortina-access.com
 *
 * Based on qfprom.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/nvmem-provider.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#define USE_SMC_CALL 1
#if USE_SMC_CALL
#define OTP_SIZE (1<<10)

extern int rtk_smc_gphy_patch_read(char *buff, size_t len);
#endif

static int ca_otp_reg_read(void *context,
			   unsigned int reg, void *_val, size_t bytes)
{
	void __iomem *base = context;
	u8 *val = _val;
	int i = 0, words = bytes;
#ifdef USE_SMC_CALL
	while (words--)
		*val++ = *(u8 *)(base + reg + i++);
#else
	while (words--)
		*val++ = readb(base + reg + i++);
#endif
	return 0;
}

static int ca_otp_reg_write(void *context,
			    unsigned int reg, void *_val, size_t bytes)
{
#ifdef USE_SMC_CALL
#else
	void __iomem *base = context;
	u8 *val = _val;
	int i = 0, words = bytes;

	while (words--)
		writeb(*val++, base + reg + i++);
#endif
	return 0;
}

static int ca_otp_remove(struct platform_device *pdev)
{
	struct nvmem_device *nvmem = platform_get_drvdata(pdev);

	nvmem_unregister(nvmem);
	return 0;
}

static struct nvmem_config econfig = {
	.name = "cortina-otp",
	.owner = THIS_MODULE,
	.read_only = true,
	.stride = 1,
	.word_size = 1,
	.reg_read = ca_otp_reg_read,
	.reg_write = ca_otp_reg_write,
};

static int ca_otp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct nvmem_device *nvmem;
	void __iomem *base;

	#if USE_SMC_CALL
	u8 *otp_buf;
	u32 otp_offset;
	size_t gphy_size;
	#endif

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	#if USE_SMC_CALL
	gphy_size = resource_size(res);

	base = devm_kzalloc(dev, gphy_size, GFP_KERNEL);
	if(base == NULL){
		return -ENOMEM;
	}
	do {
		otp_buf = devm_kzalloc(dev, OTP_SIZE, GFP_KERNEL);
		if(otp_buf == NULL){
			devm_kfree(dev, base);
			return -ENOMEM;
		}
		otp_offset = res->start & (OTP_SIZE - 1);
		rtk_smc_gphy_patch_read(otp_buf, OTP_SIZE);
		memcpy(base, &otp_buf[otp_offset], gphy_size);
		devm_kfree(dev, otp_buf);
	} while (0);

	#else
	base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (IS_ERR(base))
		return PTR_ERR(base);
	#endif

	econfig.size = resource_size(res);
	econfig.dev = dev;
	econfig.priv = base;

	nvmem = nvmem_register(&econfig);
	if (IS_ERR(nvmem))
		return PTR_ERR(nvmem);

	platform_set_drvdata(pdev, nvmem);

	return 0;
}

static const struct of_device_id ca_otp_of_match[] = {
	{ .compatible = "cortina,cortina-otp" },
	{},
};
MODULE_DEVICE_TABLE(of, ca_otp_of_match);

static struct platform_driver ca_otp_driver = {
	.probe = ca_otp_probe,
	.remove = ca_otp_remove,
	.driver = {
		.name = "cortina,cortina-otp",
		.of_match_table = ca_otp_of_match,
	},
};
module_platform_driver(ca_otp_driver);
MODULE_DESCRIPTION("Cortina OTP driver");
MODULE_LICENSE("GPL v2");
