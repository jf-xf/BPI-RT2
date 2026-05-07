#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/inetdevice.h>
#include <linux/resource.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/gpio/consumer.h>
#include <linux/spinlock.h>
#include <linux/leds.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <soc/cortina/registers.h>

#include "../soc/realtek/led-generic.h"

#define RTK_GPIO_LED_MAX_COUNT   24
#define RTK_TRIG_NAME_LEN        24
#define RTK_TRIG_NAME_PREX       "GP_LED"

struct led_timer_data {
    struct timer_list led_timer;
    int interval;               //hz
    unsigned short on_off;
    unsigned short gp_led_num;
    unsigned short run;
};

typedef struct rtk_gpio_led_cfg {
    struct led_classdev cdev;
    spinlock_t *lock;           /* protect LED resource access */
    unsigned long data;
    int idx;
    int enable;
    struct device *dev;
    struct gpio_desc *dev_gpio;
    struct led_timer_data *led_timer_data_p;

    struct led_trigger *ledtrig_rtk_dev;
    char *rtk_ledtrig_name;

} rtk_gpio_led_cfg_s, *rtk_gpio_led_cfg_p ;



struct rtk_gpio_led_ctrl {
    spinlock_t *lock;           /* protect LED resource access */
    struct device *dev;
    struct rtk_gpio_led_cfg *led_cfg[RTK_GPIO_LED_MAX_COUNT];
    u8 led_count;
};

static const char *LEDS_STRING[] = {
    FOREACH_LEDS(GENERATE_STRING)
};
/***** Data Storage ***************/
struct rtk_gpio_led_ctrl *glb_led_ctrl;
static int led_control = 2;

u8 LEDS_MAPPING_To_LEDTRIGGER[LED_GPIO_MAX];

/*** func *************************/
void rtk_led_sw_on(int rtk_enum_led_id, int op);
static void ledtrig_rtk_default_add(rtk_gpio_led_cfg_p led_cfg){
    if(led_cfg->cdev.default_trigger != NULL){
        //  printk("[%s . %d] %d %s \n", __func__, __LINE__, led_cfg->idx, led_cfg->cdev.default_trigger );
        /*the led has specific trigger in DTS by user */
        return;
    }

    led_cfg->rtk_ledtrig_name = kzalloc(RTK_TRIG_NAME_LEN, GFP_KERNEL);
    if(led_cfg->rtk_ledtrig_name) {
            snprintf(led_cfg->rtk_ledtrig_name, RTK_TRIG_NAME_LEN -1 , "%s%d", RTK_TRIG_NAME_PREX, led_cfg->idx);

            led_trigger_register_simple(led_cfg->rtk_ledtrig_name, &led_cfg->ledtrig_rtk_dev);
            led_cfg->cdev.default_trigger = led_cfg->rtk_ledtrig_name;
    }

}
/**************************************************
* enum id -> dts's gpio led
********************************************************/

static int led_enum_id_2_lednum(int enum_led_id, u8 *led_id){
    if(unlikely( (glb_led_ctrl==NULL) || enum_led_id >= LED_GPIO_MAX) ){
        return 1;
    }
    *led_id=LEDS_MAPPING_To_LEDTRIGGER[enum_led_id];
    if(unlikely(*led_id == 0xff)){
         return 1; //fail,not mapping
    }

    return 0;
}




static void ledtrig_rtk_ctrl(unsigned int gp_led_id, unsigned int value)
{
    struct led_trigger *ledtrig_rtk_dev = NULL;
    ledtrig_rtk_dev = glb_led_ctrl->led_cfg[gp_led_id]->ledtrig_rtk_dev;
    if(likely(ledtrig_rtk_dev)){
        led_trigger_event(ledtrig_rtk_dev, value);
    }
}



/********************************/

static void led_use_lable_to_map(struct rtk_gpio_led_cfg *p){
    int i;
    for(i=0; i<LED_GPIO_MAX; i++){
        if(strcmp(LEDS_STRING[i], p->cdev.name) == 0){
            LEDS_MAPPING_To_LEDTRIGGER[i] = p->idx;
            return;
        }
    }
    return;
}

static void __rtk_led_sw_on(int gp_led_id, int op)
{
       ledtrig_rtk_ctrl(gp_led_id, op);
}


//static void led_blink_timer_func(unsigned long arg)
static void led_blink_timer_func(struct timer_list *t)
{
    struct led_timer_data *data = from_timer(data, t, led_timer);
    if (data->on_off) {
        __rtk_led_sw_on(data->gp_led_num, LED_OFF);
        data->on_off = 0;
    } else {
        __rtk_led_sw_on(data->gp_led_num, LED_ON);
        data->on_off = 1;
    }
    mod_timer(&data->led_timer, jiffies + data->interval);
}


static void rtk_led_timer_init(rtk_gpio_led_cfg_p led_cfg)
{
    struct timer_list *led_timer_p = NULL;
    led_cfg->led_timer_data_p =  devm_kzalloc(glb_led_ctrl->dev, sizeof(struct led_timer_data), GFP_KERNEL);
    led_timer_p = &led_cfg->led_timer_data_p->led_timer;
 //   setup_timer(led_timer_p, led_blink_timer_func, (unsigned long) led_cfg->led_timer_data_p);
    timer_setup(led_timer_p, led_blink_timer_func, 0);
    led_cfg->led_timer_data_p->interval = HZ / 2;        //0.5
    led_cfg->led_timer_data_p->gp_led_num = led_cfg->idx;
}

/*************************************************************
gpio_led_num is in
enum RTK_GPIO_LEDS{
    FOREACH_LEDS(GENERATE_ENUM)
};
on->interval->off-->interval->on
**************************************************************/

void rtk_led_sw_blink(int rtkled_enum_id, int interval)
{
    struct led_timer_data *data_p = NULL;
    u8 gp_led_id = 0;

    if(led_enum_id_2_lednum(rtkled_enum_id, &gp_led_id)){
        return;
    }

    data_p = glb_led_ctrl->led_cfg[gp_led_id]->led_timer_data_p;
        if(likely(data_p)){
           data_p->run = 1;
           data_p->interval = interval;
           if( data_p->led_timer.function != NULL){
                mod_timer(&data_p->led_timer, jiffies + interval);
           }
        }
}

EXPORT_SYMBOL(rtk_led_sw_blink);
/*************************************************************
gpio_led_num is in
enum RTK_GPIO_LEDS{
    FOREACH_LEDS(GENERATE_ENUM)
};
**************************************************************/
void rtk_led_sw_on(int rtkled_enum_id, int op)
{
    struct led_timer_data *data_p = NULL;
    u8 gp_led_id = 0;

    if(led_enum_id_2_lednum(rtkled_enum_id, &gp_led_id)){
        return;
    }

    data_p = glb_led_ctrl->led_cfg[gp_led_id]->led_timer_data_p;
        if (data_p && data_p->run) {
            data_p->run = 0;
            del_timer_sync(&data_p->led_timer);
        }

        __rtk_led_sw_on(gp_led_id, op);

}

EXPORT_SYMBOL(rtk_led_sw_on);


/*******************************************************
** For realtek/board_g3hgu.c: ** mptest
********************************************************/
static void __rtk_led_set_all(int op)
{
    int i;
    for (i = 0; i < glb_led_ctrl->led_count; i++) {
        if(glb_led_ctrl->led_cfg[i] == NULL){
            continue;
        }
        if (glb_led_ctrl->led_cfg[i]->dev_gpio) {
            gpiod_set_value(glb_led_ctrl->led_cfg[i]->dev_gpio, op);
        }
    }
}

static void __rtk_save_led_init_value(void)
{
    int i;
    if (led_control != 2)       // has save init value
        return;

    for (i = 0; i < glb_led_ctrl->led_count; i++) {
        if( glb_led_ctrl->led_cfg[i] == NULL)
            continue;

        if (glb_led_ctrl->led_cfg[i]->dev_gpio) {
            glb_led_ctrl->led_cfg[i]->data =
                gpiod_get_value(glb_led_ctrl->led_cfg[i]->dev_gpio);
        }
    }

}

static void __rtk_restore_led_init_value(void)
{
    int i;

    for (i = 0; i < glb_led_ctrl->led_count; i++) {
        if( glb_led_ctrl->led_cfg[i] == NULL)
            continue;
        if (glb_led_ctrl->led_cfg[i]->dev_gpio) {
            if (glb_led_ctrl->led_cfg[i]->data)
                gpiod_set_value(glb_led_ctrl->led_cfg[i]->dev_gpio,
                                LED_ON);
            else
                gpiod_set_value(glb_led_ctrl->led_cfg[i]->dev_gpio,
                                LED_OFF);
        }
    }
}


/*******************************************
** For realtek/board_g3hgu.c
** mptest
********************************************/
void rtk_led_set_all(int control)
{
    //unsigned long flags;
    //spin_lock_irqsave(rtk_gpio->lock, flags);
    if (led_control == control)
        goto out;

    if ((led_control > 2) || (led_control < 0))
        goto out;

    __rtk_save_led_init_value();

    led_control = control;

    switch (control) {
        case 0:
            /*disable all led */
            __rtk_led_set_all(LED_OFF);
            break;
        case 1:
            /*enable all led */
            __rtk_led_set_all(LED_ON);
            break;
        case 2:
            /*restore original */
            __rtk_restore_led_init_value();
            break;
    }


  out:
    //spin_unlock_irqrestore(rtk_gpio->lock, flags);
    return;

}
/*******************************************
** For realtek/board_g3hgu.c
** mptest
********************************************/
int rtk_led_get_all(void)
{
    return led_control;
}


/**
 * rtk_gpio_led_set - set on/off software event
 */
static void rtk_gpio_led_set(struct led_classdev *led_cdev,
                             enum led_brightness value)
{
    struct rtk_gpio_led_cfg *led_cfg =
        container_of(led_cdev, struct rtk_gpio_led_cfg, cdev);
    //unsigned long flags;
    //u32 val;

    if (led_cfg->dev_gpio == NULL)
        return;

    led_cfg->data = (value == LED_OFF) ? LED_OFF : LED_ON;

    if (led_control != 2)
        return;

    gpiod_set_value(led_cfg->dev_gpio, led_cfg->data);
}


static int __init rtk_gpio_led_probe(struct rtk_gpio_led_ctrl *led_ctrl,
                                     struct device_node *nc, u32 sn,
                                     spinlock_t * lock)
{
    rtk_gpio_led_cfg_p led_cfg=NULL;
    enum of_gpio_flags of_flags;
    unsigned long gpio_flags;

    int rc;
    int gpio_item;
    int need_timer = 1;
	int state = 0;

    led_cfg = devm_kzalloc(led_ctrl->dev, sizeof(*led_cfg), GFP_KERNEL);
    if (!led_cfg)
        return -ENOMEM;

    led_cfg->lock = lock;
    led_cfg->dev = led_ctrl->dev;

    led_cfg->cdev.name = of_get_property(nc, "label", NULL) ? : nc->name;

    if(led_cfg->cdev.name == NULL){
        printk("rtk_gpio_led_probe failed: led(%d) miss lable. It is required\n", sn);
        goto error;
    }

    led_cfg->idx = sn;
    led_use_lable_to_map(led_cfg);

    led_cfg->cdev.default_trigger = of_get_property(nc,
                                                    "linux,default-trigger",
                                                    NULL);

    if(led_cfg->cdev.default_trigger){
        need_timer =1;
    }

    gpio_item = of_get_named_gpio_flags(nc, "led-gpio", 0, &of_flags);


    if (gpio_item == -EPROBE_DEFER) {
        printk("rtk_gpio_led_probe failed: %s\n", led_cfg->cdev.name);
        goto error;
    }
    if (gpio_is_valid(gpio_item)) {
        if (of_flags & OF_GPIO_ACTIVE_LOW) {
            gpio_flags = GPIOF_ACTIVE_LOW | GPIOF_OUT_INIT_LOW;
        } else {
            gpio_flags = GPIOF_OUT_INIT_HIGH;
        }
        //Always out
        gpio_flags |= GPIOF_DIR_OUT;
        dev_dbg(led_cfg->dev, "%s %d is active %s\n", __func__, gpio_item,
                 (of_flags & OF_GPIO_ACTIVE_LOW) ? "low" : "high");

        rc = devm_gpio_request_one(led_cfg->dev, gpio_item, gpio_flags,
                                   led_cfg->cdev.name);
        if (rc) {
            printk("rtk_gpio_probe failed: devm_gpio_request_one for %s\n",
                   led_cfg->cdev.name);
            goto error;
        }
        led_cfg->dev_gpio = gpio_to_desc(gpio_item);
        gpiod_direction_output(led_cfg->dev_gpio, 0);
    } else {
        rc = -1;
        goto error;
    }



    {
        led_cfg->enable = 1;
        led_cfg->cdev.brightness = LED_OFF;
    }
    state = of_get_property(nc,
					"default-state",
					NULL);
    if(state==1){
		led_cfg->cdev.brightness = LED_ON;
    }

    ledtrig_rtk_default_add(led_cfg);

    led_cfg->cdev.brightness_set = rtk_gpio_led_set;
    //led_cfg->cdev.blink_set = ca77xx_blink_set;
    //led_cfg->cdev.groups = ca77xx_led_groups;

    rc = devm_led_classdev_register(led_ctrl->dev, &led_cfg->cdev);
    if (rc < 0)
        goto error;

    dev_set_drvdata(led_cfg->cdev.dev, led_cfg);

    dev_dbg(led_ctrl->dev, "registered LED %s\n", led_cfg->cdev.name);
    //dev_info(led_ctrl->dev, "registered LED %s\n", led_cfg->cdev.name);
    //dev_info(led_ctrl->dev, "  trigger: %s\n",led_cfg->cdev.default_trigger);
    if (led_cfg->cdev.default_trigger) {
        mutex_lock(&led_cfg->cdev.led_access);
        led_sysfs_enable(&led_cfg->cdev);
        mutex_unlock(&led_cfg->cdev.led_access);
        dev_dbg(led_ctrl->dev, "  trigger: %s\n",
                 led_cfg->cdev.default_trigger);

    } else {
        mutex_lock(&led_cfg->cdev.led_access);
        led_sysfs_disable(&led_cfg->cdev);
        mutex_unlock(&led_cfg->cdev.led_access);
        led_cfg->cdev.default_trigger = "none";
    }

    led_ctrl->led_cfg[led_cfg->idx] = led_cfg;
            /*** timer **/
    if(need_timer)
        rtk_led_timer_init(led_cfg);

    return 0;
  error:
    if(led_cfg){

        if(led_cfg->ledtrig_rtk_dev)
          led_trigger_unregister_simple(led_cfg->ledtrig_rtk_dev);

        if(led_cfg->rtk_ledtrig_name)
            kfree(led_cfg->rtk_ledtrig_name);

        devm_kfree(led_ctrl->dev, led_cfg);
    }
    return -1;
}

static void __init rtk_gpio_led_pin_mux_clr(void)
{
#ifdef CONFIG_CORTINA_BOARD_G3HGU_ENG
    GLOBAL_PIN_MUX_t reg_v;
    int updated_flag = 0;
    void __iomem *addr;
    addr = ioremap(GLOBAL_PIN_MUX, 4);
    reg_v.wrd = readl_relaxed(addr);
    if (addr) {
        //printk("GLOBAL_PIN_MUX: addr(0x%x) = 0x%x\n", addr, reg_v.wrd);

        reg_v.bf.iomux_led_enable = 0;
        updated_flag = 1;

        if (updated_flag) {
            writel_relaxed(reg_v.wrd, addr);
            smp_mb();
            reg_v.wrd = readl_relaxed(addr);
        }

        printk("[RTK GPIO_LED]GLOBAL_PIN_MUX: 0x%x\n", reg_v.wrd);
        iounmap(addr);
    } else {
        printk("[RTK GPIO_LED]GLOBAL_PIN_MUX: ioremap failed!\n");
    }
#endif                          //only for G3HGU_ENG
}

static int  rtk_gpio_leds_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = pdev->dev.of_node;
    struct device_node *child;
    int rc;
    spinlock_t *lock;           /* protect LED resource access */

    memset(LEDS_MAPPING_To_LEDTRIGGER, 0xff, sizeof(LEDS_MAPPING_To_LEDTRIGGER));

    glb_led_ctrl = devm_kzalloc(dev, sizeof(struct rtk_gpio_led_ctrl),
                                GFP_KERNEL);
    if (!glb_led_ctrl)
        return -ENOMEM;

    lock = devm_kzalloc(dev, sizeof(*lock), GFP_KERNEL);
    if (!lock)
        return -ENOMEM;

    spin_lock_init(lock);
    glb_led_ctrl->lock = lock;

    glb_led_ctrl->dev = dev;

    for_each_available_child_of_node(np, child) {

        if (glb_led_ctrl->led_count >= RTK_GPIO_LED_MAX_COUNT) {
            dev_err(dev, "MAX is %d. Invalid LED (%u >= %d)\n",RTK_GPIO_LED_MAX_COUNT,  glb_led_ctrl->led_count, RTK_GPIO_LED_MAX_COUNT);
            break;
        }

        rc = rtk_gpio_led_probe(glb_led_ctrl, child, glb_led_ctrl->led_count, lock);
        if (rc < 0) {
            dev_err(dev, "Not find LED(%d)'s GPIO setting or is wrong.\n", glb_led_ctrl->led_count);
            of_node_put(child);
            return rc;
        }
        glb_led_ctrl->led_count++;
    }

    dev_set_drvdata(dev, glb_led_ctrl);

    //return sysfs_create_group(&dev->kobj, &ca77xx_leds_attr_group);

    rtk_gpio_led_pin_mux_clr();

    return 0;
}

static int rtk_gpio_leds_remove(struct platform_device *pdev)
{
    //sysfs_remove_group(&pdev->dev.kobj, &ca77xx_leds_attr_group);

    return 0;
}

static const struct of_device_id rtk_gpio_leds_of_match[] = {
    {.compatible = "realtek,g3hgu-gpio-led",},
    {},
};

MODULE_DEVICE_TABLE(of, rtk_gpio_leds_of_match);

static struct platform_driver rtk_gpio_leds_driver = {
    .probe = rtk_gpio_leds_probe,
    .remove = rtk_gpio_leds_remove,
    .driver = {
               .name = "leds-rtk-gpio_led",
               .of_match_table = rtk_gpio_leds_of_match,
               },
};

module_platform_driver(rtk_gpio_leds_driver);

MODULE_DESCRIPTION("LED driver for RTK GPIO");
MODULE_LICENSE("GPL v2");
