#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>

#define CA77XX_LED_MAX_COUNT 16

struct rtk_spare_reg_ctrl {
    void __iomem *mem;
    spinlock_t *lock;	/* protect spare reg access */

    struct device *dev;
};

enum {
    SPARE_GPIO = 0,
    SPARE_PARALLEL_LED
};

int rtk_spare_reg_is_probed = 0;

struct rtk_spare_reg_ctrl *glb_spare_ctrl;

static void rtk_spare_reg_set_bit(unsigned int bit)
{
    u32 val;
    unsigned long flags;

    spin_lock_irqsave(glb_spare_ctrl->lock, flags);
    val = ioread32(glb_spare_ctrl->mem);
    val |= 1 << bit;

    iowrite32(val, glb_spare_ctrl->mem);
    spin_unlock_irqrestore(glb_spare_ctrl->lock, flags);
}

static void rtk_spare_reg_clear_bit(unsigned int bit)
{
    u32 val;
    unsigned long flags;

    spin_lock_irqsave(glb_spare_ctrl->lock, flags);
    val = ioread32(glb_spare_ctrl->mem);
    val &= ~(1 << bit);

    iowrite32(val, glb_spare_ctrl->mem);
    spin_unlock_irqrestore(glb_spare_ctrl->lock, flags);
}

/* API function for other drivers */
int rtk_spare_reg_mode_set(unsigned int bit, int mode)
{

    if (!rtk_spare_reg_is_probed) {
        WARN(1, "Calling spare reg API before probed\n");
        return -ENODEV;
    }

    if (bit > CA77XX_LED_MAX_COUNT) {
        dev_err(glb_spare_ctrl->dev, "Invalid bit(%d) for PER_SPARE reg setting\n", bit);
        return -EINVAL;
    }

    if (mode == SPARE_GPIO)
        rtk_spare_reg_clear_bit(bit);
    else if (mode == SPARE_PARALLEL_LED)
        rtk_spare_reg_set_bit(bit);
    else {
        dev_err(glb_spare_ctrl->dev, "Invalid mode(%d) for PER_SPARE reg setting\n", mode);
        return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(rtk_spare_reg_mode_set);

static int rtk_spare_reg_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct resource *reg_res;
    spinlock_t *lock;	/* protect LED resource access */

    glb_spare_ctrl = devm_kzalloc(dev, sizeof(struct rtk_spare_reg_ctrl),
                                  GFP_KERNEL);
    if (!glb_spare_ctrl)
        return -ENOMEM;

    glb_spare_ctrl->dev = dev;

    /* get spare reg base addr */
    reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!reg_res)
        return -EINVAL;

    glb_spare_ctrl->mem = devm_ioremap_resource(dev, reg_res);
    if (IS_ERR(glb_spare_ctrl->mem))
        return PTR_ERR(glb_spare_ctrl->mem);

    dev_info(dev, "resource - %pr mapped at 0x%pK\n", reg_res, glb_spare_ctrl->mem);

    /* alloc lock */
    lock = devm_kzalloc(dev, sizeof(*lock), GFP_KERNEL);
    if (!lock)
        return -ENOMEM;

    spin_lock_init(lock);
    glb_spare_ctrl->lock = lock;

    dev_set_drvdata(dev, glb_spare_ctrl);

    rtk_spare_reg_is_probed = 1;

    return 0;
}

static int rtk_spare_reg_revome(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int i;

    /* clear all PER_SPARE */
    for (i = 0; i < CA77XX_LED_MAX_COUNT; i++)
        rtk_spare_reg_clear_bit(i);

    devm_kfree(dev, glb_spare_ctrl);
    rtk_spare_reg_is_probed = 0;

    return 0;
}

static const struct of_device_id rtk_spare_reg_of_match[] = {
    { .compatible = "realtek,rtk_spare_reg", },
    { },
};
MODULE_DEVICE_TABLE(of, rtk_spare_reg_of_match);

static struct platform_driver rtk_spare_reg_driver = {
    .probe = rtk_spare_reg_probe,
    .remove = rtk_spare_reg_revome,
    .driver = {
        .name = "reg-rtk_spare",
        .of_match_table = rtk_spare_reg_of_match,
    },
};

static int __init rtk_spare_reg_init(void)
{
    platform_driver_register(&rtk_spare_reg_driver);
    return 0;
}

/*
static void __exit rtk_spare_reg_exit(void)
{
    platform_driver_unregister(&rtk_spare_reg_driver);
}
*/

arch_initcall(rtk_spare_reg_init);

MODULE_DESCRIPTION("PER_SPARE REG driver for CA8277B");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:reg-rtk_spare");
