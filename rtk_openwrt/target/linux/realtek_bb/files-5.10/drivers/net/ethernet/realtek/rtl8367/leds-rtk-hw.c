#include <linux/module.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/property.h>
#include <linux/of.h>

#include <rtk_switch.h>
#include <rtk_types.h>
#include <led.h>

#if 0
#define LED_DEBUG
#endif

#ifdef LED_DEBUG
#define DEBUG_PRINT(fmt, args...) printk(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

enum rtk_hw_led_rtl8367_type {
    HW_LED = 0,
    SW_LED,
};

struct rtk_hw_led_rtl8367_hw_config {
    rtksw_enable_t enable;
    rtksw_led_operation_t mode;
    rtksw_port_t type;
    unsigned int cfg_bitmap;
};

struct rtk_hw_led_rtl8367 {
    const char *name;
    const char *default_trigger;
    unsigned int id;
    unsigned int type;
    struct rtk_hw_led_rtl8367_hw_config hwcfg;
};

struct rtk_hw_led_rtl8367_platform_data {
    int num_leds;
    const struct rtk_hw_led_rtl8367 *leds;
};

struct rtk_hw_led_rtl8367_data {
    struct led_classdev cdev;
    unsigned int id;
    unsigned int type;
    struct rtk_hw_led_rtl8367_hw_config hwcfg;
};

struct rtk_hw_led_rtl8367_priv {
    int num_leds;
    struct rtk_hw_led_rtl8367_data leds[];
};

static char* isConfigSelected(unsigned int bitmap, int id)
{
    return (bitmap == id)?"*":" ";
}

static void rtk_hw_led_rtl8367_set_type_sw(unsigned int id, struct rtk_hw_led_rtl8367_hw_config hwcfg, rtksw_led_force_mode_t force);

static void rtk_hw_led_rtl8367_set_brightness(struct led_classdev *led_cdev,
                                      enum led_brightness value)
{
    struct rtk_hw_led_rtl8367_data *led_dat = container_of(led_cdev, struct rtk_hw_led_rtl8367_data, cdev);

    if (led_dat->type == HW_LED)
        return;

	rtk_hw_led_rtl8367_set_type_sw(led_dat->id, led_dat->hwcfg, (value == LED_OFF) ? LED_FORCE_OFF : LED_FORCE_ON);
	
    return;
}

static void rtk_hw_led_rtl8367_set_type_hw(unsigned int id, const struct rtk_hw_led_rtl8367_hw_config hwcfg)
{
    rtksw_led_congig_t cfg = 0;
	rtksw_portmask_t portMask;

    DEBUG_PRINT("%s\nled hw id %d\n", __func__, id);
    DEBUG_PRINT("enable: %d | mode: %d | type: %d | cfg_bitmap: 0x%x\n",
                hwcfg.enable, hwcfg.mode, hwcfg.type, hwcfg.cfg_bitmap);

	cfg = hwcfg.cfg_bitmap;
    
	rtksw_led_enable_get(id, &portMask);
	if(hwcfg.enable==0 || hwcfg.cfg_bitmap == LED_CONFIG_LEDOFF){
		RTKSW_PORTMASK_PORT_CLEAR(portMask, hwcfg.type);
	}
	else{
		RTKSW_PORTMASK_PORT_SET(portMask, hwcfg.type);
	}
	rtksw_led_enable_set(id, &portMask);
}


static void rtk_hw_led_rtl8367_set_type_sw(unsigned int id, struct rtk_hw_led_rtl8367_hw_config hwcfg, rtksw_led_force_mode_t force)
{
    DEBUG_PRINT("%s: led id %d\n", __func__, id);
    rtksw_led_congig_t cfg = 0;
	rtksw_portmask_t portMask;

    rtksw_led_enable_get(id, &portMask);
	if(hwcfg.enable==0){
		RTKSW_PORTMASK_PORT_CLEAR(portMask, hwcfg.type);
	}
	else{
		RTKSW_PORTMASK_PORT_SET(portMask, hwcfg.type);
	}
	
	rtksw_led_enable_set(id, &portMask);
	rtksw_led_modeForce_set(hwcfg.type, id, force);
}

static ssize_t rtk_hw_led_rtl8367_port_show(struct device *dev,
                                    struct device_attribute *attr,
                                    char *buf)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct rtk_hw_led_rtl8367_data *led_dat = container_of(led_cdev, struct rtk_hw_led_rtl8367_data, cdev);

    sprintf(buf, "led port: %d\n0: UTP0\n1: UTP1\n2: UTP2\n3: UTP3\n4: UTP4\n",
            led_dat->hwcfg.type);

    return strlen(buf) + 1;
}

static ssize_t rtk_hw_led_rtl8367_port_store(struct device *dev,
                                     struct device_attribute *attr,
                                     const char *buf,
                                     size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct rtk_hw_led_rtl8367_data *led_dat = container_of(led_cdev, struct rtk_hw_led_rtl8367_data, cdev);
    unsigned long value;
    int ret = -EINVAL;

    ret = kstrtoul(buf, 0, &value);
    if (ret)
        return ret;

    led_dat->hwcfg.type = (unsigned int)value;
    DEBUG_PRINT("set led port to: %d\n", led_dat->hwcfg.type);
    DEBUG_PRINT("0: UTP0\n1: UTP1\n2: UTP2\n3: UTP3\n4: UTP4\n");

    if (led_dat->type == HW_LED)
        rtk_hw_led_rtl8367_set_type_hw(led_dat->id, led_dat->hwcfg);

    return size;
}

static DEVICE_ATTR(port, 0644, rtk_hw_led_rtl8367_port_show, rtk_hw_led_rtl8367_port_store);

static ssize_t rtk_hw_led_rtl8367_config_show(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct rtk_hw_led_rtl8367_data *led_dat = container_of(led_cdev, struct rtk_hw_led_rtl8367_data, cdev);
    int len;
	
    len = sprintf(buf, "led config bitmap: 0x%x\n", led_dat->hwcfg.cfg_bitmap);
    len += sprintf(buf + len, "[%s]  0: OFF \n[%s]  1: COL\n[%s]  2: ACT\n",
					isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LEDOFF),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_DUPCOL),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LINK_ACT));
		   
	len += sprintf(buf + len, "[%s]  3: SPD1000\n[%s]  4: SPD100\n[%s]  5: SPD10\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD1000),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD100),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD10));
		   
    len += sprintf(buf + len, "[%s]  6: SPD1000_ACT\n[%s]  7: SPD100_ACT\n[%s]  8: SPD10_ACT\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD1000ACT),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD100ACT),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD10ACT));
		   
    len += sprintf(buf + len, "[%s]  9: SPD10010ACT\n[%s]  10: LOOPDETECT\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD10010ACT),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LOOPDETECT));

    len += sprintf(buf + len, "[%s]  11: SPD10_LNK\n[%s]  12: LINKRX\n[%s]  13: LINKTX\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_EEE),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LINKRX),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LINKTX));
		   
    len += sprintf(buf + len, "[%s]  14: MASTER\n[%s]  15: ACT\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_MASTER),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_ACT));

    return strlen(buf) + 1;
}

static ssize_t rtk_hw_led_rtl8367_config_store(struct device *dev,
                                       struct device_attribute *attr,
                                       const char *buf,
                                       size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct rtk_hw_led_rtl8367_data *led_dat = container_of(led_cdev, struct rtk_hw_led_rtl8367_data, cdev);
    unsigned long value;
    int ret = -EINVAL;

    ret = kstrtoul(buf, 0, &value);
    if (ret)
        return ret;
	
    led_dat->hwcfg.cfg_bitmap = (unsigned int)value;

    DEBUG_PRINT("set led config bitmap to 0x%x\n", led_dat->hwcfg.cfg_bitmap);
	DEBUG_PRINT("[%s]  0: OFF \n[%s]  1: COL\n[%s]  2: ACT\n",
					isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LEDOFF),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_DUPCOL),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LINK_ACT));
				   
	DEBUG_PRINT("[%s]  3: SPD1000\n[%s]  4: SPD100\n[%s]  5: SPD10\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD1000),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD100),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD10));
				   
    DEBUG_PRINT("[%s]  6: SPD1000_ACT\n[%s]  7: SPD100_ACT\n[%s]  8: SPD10_ACT\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD1000ACT),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD100ACT),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD10ACT));
				   
    DEBUG_PRINT("[%s]  9: SPD10010ACT\n[%s]  10: LOOPDETECT\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_SPD10010ACT),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LOOPDETECT));
    DEBUG_PRINT("[%s]  11: SPD10_LNK\n[%s]  12: LINKRX\n[%s]  13: LINKTX\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_EEE),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LINKRX),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_LINKTX));
				   
    DEBUG_PRINT("[%s]  14: MASTER\n[%s]  15: ACT\n",
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_MASTER),
                   isConfigSelected(led_dat->hwcfg.cfg_bitmap, LED_CONFIG_ACT));
	
    if (led_dat->type == HW_LED){
        rtk_hw_led_rtl8367_set_type_hw(led_dat->id, led_dat->hwcfg);
	}

    return size;
}

static DEVICE_ATTR(config, 0644, rtk_hw_led_rtl8367_config_show, rtk_hw_led_rtl8367_config_store);

static ssize_t rtk_hw_led_rtl8367_type_show(struct device *dev,
                                    struct device_attribute *attr,
                                    char *buf)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct rtk_hw_led_rtl8367_data *led_dat = container_of(led_cdev, struct rtk_hw_led_rtl8367_data, cdev);

    sprintf(buf, "type: %s\nled id: %d\n",
            led_dat->type?"SW":"HW",
            led_dat->id);

    return strlen(buf) + 1;
}

static ssize_t rtk_hw_led_rtl8367_type_store(struct device *dev,
                                     struct device_attribute *attr,
                                     const char *buf,
                                     size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct rtk_hw_led_rtl8367_data *led_dat = container_of(led_cdev, struct rtk_hw_led_rtl8367_data, cdev);
    unsigned long value;
    static unsigned int last_type;
    int ret = -EINVAL;

    ret = kstrtoul(buf, 0, &value);
    if (ret)
        return ret;

    last_type = led_dat->type;
    led_dat->type = (unsigned int)value;

    if (led_dat->type == HW_LED) {
        rtk_hw_led_rtl8367_set_type_hw(led_dat->id, led_dat->hwcfg);
        if (last_type != HW_LED)  {
            device_create_file(led_cdev->dev, &dev_attr_port);
            device_create_file(led_cdev->dev, &dev_attr_config);
        }
    } else {
        rtk_hw_led_rtl8367_set_type_sw(led_dat->id, led_dat->hwcfg, LED_FORCE_OFF);
        if (last_type == HW_LED) {
            device_remove_file(led_cdev->dev, &dev_attr_port);
            device_remove_file(led_cdev->dev, &dev_attr_config);
        }
    }

    return size;
}

static DEVICE_ATTR(type, 0644, rtk_hw_led_rtl8367_type_show, rtk_hw_led_rtl8367_type_store);

static int create_rtk_hw_led_rtl8367_classdev(const struct rtk_hw_led_rtl8367 *template,
                                      struct rtk_hw_led_rtl8367_data *led_dat, struct device *parent)
{
    int ret;

    led_dat->cdev.name = template->name;
    led_dat->cdev.brightness_set = rtk_hw_led_rtl8367_set_brightness;
    led_dat->id = template->id;
    led_dat->type = template->type;
    led_dat->hwcfg = template->hwcfg;

    if (led_dat->type == HW_LED) {
        led_dat->cdev.default_trigger = "none";
        rtk_hw_led_rtl8367_set_type_hw(led_dat->id, led_dat->hwcfg);
    } else {
        led_dat->cdev.default_trigger = template->default_trigger;
        rtk_hw_led_rtl8367_set_type_sw(led_dat->id, led_dat->hwcfg, LED_FORCE_OFF);
    }

    ret = led_classdev_register(parent, &led_dat->cdev);
    if (ret < 0 )
        return ret;

    DEBUG_PRINT("\n\n%s: create attr\n\n", __func__);
    ret = device_create_file(led_dat->cdev.dev, &dev_attr_type);
    if (ret) {
        pr_err("%s: create device attribute failed\n", __func__);
        return -1;
    }

    if (led_dat->type == HW_LED) {
        ret = device_create_file(led_dat->cdev.dev, &dev_attr_port);
        if (ret) {
            pr_err("%s: create device attribute failed\n", __func__);
            return -1;
        }
        ret = device_create_file(led_dat->cdev.dev, &dev_attr_config);
        if (ret) {
            pr_err("%s: create device attribute failed\n", __func__);
            return -1;
        }
    }

    return 0;
}

static void delete_rtk_hw_led_rtl8367_classdev(struct rtk_hw_led_rtl8367_data *led_dat)
{
    led_classdev_unregister(&led_dat->cdev);
}

static inline int sizeof_rtk_hw_leds_rtl8367_priv(int num)
{
    return (sizeof(struct rtk_hw_led_rtl8367_priv) + sizeof(struct rtk_hw_led_rtl8367_data) * num);
}

static struct rtk_hw_led_priv *rtk_hw_led_rtl8367_create(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct fwnode_handle *child;
    struct rtk_hw_led_rtl8367_priv *priv;
    int count, ret;
    struct device_node *np;

    count = device_get_child_node_count(dev);
    if (!count)
        return ERR_PTR(-ENODEV);

    priv = devm_kzalloc(dev, sizeof_rtk_hw_leds_rtl8367_priv(count), GFP_KERNEL);
    if(!priv)
        return ERR_PTR(-ENOMEM);

    device_for_each_child_node(dev, child) {
        struct rtk_hw_led_rtl8367 led = {};

        np = to_of_node(child);

        if (fwnode_property_present(child, "label")) {
            fwnode_property_read_string(child, "label", &led.name);
        } else {
            if (IS_ENABLED(CONFIG_OF) && !led.name && np)
                led.name = np->name;
            if (!led.name) {
                ret = -EINVAL;
                goto err;
            }
        }

        fwnode_property_read_string(child, "linux,default-trigger", &led.default_trigger);

        fwnode_property_read_u32(child, "id", &led.id);

        fwnode_property_read_u32(child, "default-type", &led.type);

        fwnode_property_read_u32(child, "enable", &led.hwcfg.enable);

        fwnode_property_read_u32(child, "mode", &led.hwcfg.mode);

        fwnode_property_read_u32(child, "led-type", &led.hwcfg.type);

        fwnode_property_read_u32(child, "config", &led.hwcfg.cfg_bitmap);
		
		rtksw_led_groupConfig_set(led.id, led.hwcfg.cfg_bitmap);

        ret = create_rtk_hw_led_rtl8367_classdev(&led, &priv->leds[priv->num_leds], dev);
        if (ret < 0) {
            fwnode_handle_put(child);
            goto err;
        }
        priv->num_leds++;
    }

    return priv;

err:
    for (count = priv->num_leds - 1; count >= 0; count--)
        delete_rtk_hw_led_rtl8367_classdev(&priv->leds[count]);
    return ERR_PTR(ret);
}

static const struct of_device_id of_rtk_hw_rtl8367_leds_match[] = {
    { .compatible = "realtek,rtk-hw-leds-rtl8367", },
    {},
};

MODULE_DEVICE_TABLE(of, of_rtk_hw_rtl8367_leds_match);

static int rtk_hw_led_rtl8367_probe(struct platform_device *pdev)
{
    struct rtk_hw_led_rtl8367_platform_data *pdata = dev_get_platdata(&pdev->dev);
    struct rtk_hw_led_rtl8367_priv *priv;
    int i, ret = 0;

    if (pdata && pdata->num_leds) {
        priv = devm_kzalloc(&pdev->dev,
                            sizeof_rtk_hw_leds_rtl8367_priv(pdata->num_leds),
                            GFP_KERNEL);
        if (!priv)
            return -ENOMEM;

        priv->num_leds = pdata->num_leds;
        for (i = 0; i < priv->num_leds; i++) {
            ret = create_rtk_hw_led_rtl8367_classdev(&pdata->leds[i],
                                             &priv->leds[i],
                                             &pdev->dev);
            if (ret < 0) {
                /* On failure: unwind the led creations */
                for (i = i - 1; i >= 0; i--)
                    delete_rtk_hw_led_rtl8367_classdev(&priv->leds[i]);
                return ret;
            }
        }
    } else {
        priv = rtk_hw_led_rtl8367_create(pdev);
        if (IS_ERR(priv))
            return PTR_ERR(priv);
    }

    platform_set_drvdata(pdev, priv);

    return 0;
}

static int rtk_hw_led_rtl8367_remove(struct platform_device *pdev)
{
    struct rtk_hw_led_rtl8367_priv *priv = platform_get_drvdata(pdev);
    int i;

    for (i = 0; i < priv->num_leds; i++)
        delete_rtk_hw_led_rtl8367_classdev(&priv->leds[i]);

    return 0;
}

//static struct platform_device *led_rtk_hw_device;

static struct platform_driver led_rtk_hw_rtl8367_driver = {
    .probe		= rtk_hw_led_rtl8367_probe,
    .remove		= rtk_hw_led_rtl8367_remove,
    .driver		= {
        .name	= "leds-rtk-hw-rtl8367",
        .owner	= THIS_MODULE,
        .of_match_table = of_rtk_hw_rtl8367_leds_match,
    },
};

int led_rtk_hw_rtl8367_led_register(void)
{
	platform_driver_register(&led_rtk_hw_rtl8367_driver);

	return 0;
}

#if 0
static int __init led_rtk_hw_rtl8367_init(void)
{
    //struct platform_device *pdev;
    //struct rtk_hw_led_platform_data pdata;
#if 0
    pdev = platform_device_alloc("leds-rtk-hw", -1);
    if (!pdev)
        goto err_0;

    led_rtk_hw_device = pdev;

    pdata.num_leds = sizeof(myleds)/sizeof(struct rtk_hw_led);
    pdata.leds = myleds;

    err = platform_device_add_data(pdev, &pdata, sizeof(pdata));
    if (err)
        goto err_1;

    err = platform_device_add(pdev);
    if (err)
        goto err_1;
#endif

	led_rtk_hw_rtl8367_led_register();

    return 0;

err_1:
    //platform_device_put(pdev);
//err_0:
//   pr_err("%s: error occurs during create platform device\n", __func__);
    return err;
}
#endif

int led_rtk_hw_rtl8367_led_unregister(void)
{
	
	platform_driver_unregister(&led_rtk_hw_rtl8367_driver);

	return 0;
}

#if 0
static void __exit led_rtk_hw_rtl8367_exit(void)
{
//    platform_device_put(led_rtk_hw_device);
    led_rtk_hw_rtl8367_led_unregister();
}

module_init(led_rtk_hw_rtl8367_init);
module_exit(led_rtk_hw_rtl8367_exit);
MODULE_LICENSE("GPL");
#endif
