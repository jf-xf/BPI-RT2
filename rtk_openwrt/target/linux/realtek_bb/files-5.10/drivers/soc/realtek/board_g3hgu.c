#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/gpio/consumer.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
//#include <linux/leds-ca77xx.h>
#include "rtk_gpio.h"
#include "led-generic.h"
#include "pushbutton.h"
/*
#define POWER_LED   3
#define WIFI_LED    4
#define INTERNET_LED 12
*/
#define TRUE    1
#define FALSE   0

#include <linux/notifier.h>
#include <linux/reboot.h>

#define DBG(fmt, ...) // printk(fmt, __VA_ARGS__)


#ifdef CONFIG_LEDS_RTK_GPIO_LED
/* linux-4.4.x/drivers/leds/leds-rtk-gpio-led.c*/
extern void rtk_led_sw_on(int gpio_led_num , int op);
extern void rtk_led_sw_blink(int gpio_led_num , int interval);
static void rtk_board_led_init(void){
   rtk_led_sw_blink(LED_PWR_G, 0.25*HZ);
}
static void rtk_led_sw(int led_num, int op){
    switch (led_num){
        case LED_POWER_GREEN:
            rtk_led_sw_on(LED_PWR_G, op);
            break;
        case LED_INTERNET_GREEN:
            rtk_led_sw_on(LED_PPP_G, op);
            break;
	case LED_WPS_GREEN:
	    rtk_led_sw_on(LED_WPS_G, op);
	    break;
        default:
            break;
    }
}
#else
/* Cortina LED System */
static void rtk_board_led_init(void){
    int i;
    for(i =0 ; i < LED_TOTAL; i++){
      if((rtk_led_num[i] > 0) && (rtk_led_num[i] < 15)){
          DBG("[%s]rtk_led_num = %d\n", __func__, rtk_led_num[i]);
       ca77xx_led_enable(rtk_led_num[i], TRUE);
      }
    }
    
}
static void rtk_led_sw(int led_num, int op){
    DBG("%s: led_num(%d), Phy_LED_NUM(%d)\n",__func__, led_num, rtk_led_num[led_num]);
    if(rtk_led_num[led_num]>0)
        ca77xx_led_sw_on(rtk_led_num[led_num], op); 
}

static void board_led_clear(void){
    int i;
    for(i =0 ; i < LED_TOTAL; i++){
     rtk_led_sw(i, FALSE);
    }
    
}

static int board_notify_sys(struct notifier_block *this,
                      unsigned long code, void *unused)
{
    board_led_clear();
    smp_mb();
    return NOTIFY_DONE;
}
static struct notifier_block board_roboot_notifier = {
    .notifier_call = board_notify_sys,
};

#endif

/****************************
 * op : LED_ON:1 , LED_OFF:0
 * *************************/
static void board_01_handle_set(int which, int op) {
    //printk("%s: led %d op %d\n", __FUNCTION__, which, op);
    switch (which) {
    case LED_POWER_GREEN:
        rtk_led_sw(LED_POWER_GREEN, op);
        break;
#ifdef CONFIG_WIFI_LED_USE_SOC_GPIO
    case LED_WIFI:
        rtk_led_sw(LED_WIFI, op);
        break;
#endif
    case LED_INTERNET_GREEN:
        rtk_led_sw(LED_INTERNET_GREEN, op);
        break;
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
    case LED_WPS_GREEN:
    case LED_WPS_YELLOW:
    case LED_WPS_RED:
                rtk_led_sw(LED_WPS_GREEN, op);
        break;
#endif

    default:
        led_handle_set(which, op);
    }
}

static void board_01_handle_init(void) {
        /* succeed  from luna_pro_yueme */
//        board_01_handle_set(LED_INTERNET_GREEN, LED_ON); // yh change to on
    board_01_handle_set(LED_INTERNET_GREEN, LED_OFF);
        board_01_handle_set(LED_INTERNET_RED, LED_OFF);
        board_01_handle_set(LED_WPS_GREEN, LED_OFF);
        board_01_handle_set(LED_WPS_RED, LED_OFF);
        board_01_handle_set(LED_WPS_YELLOW, LED_OFF);
#ifdef CONFIG_SW_USB_LED1
        board_01_handle_set(LED_USB_1, LED_OFF);
#endif //CONFIG_SW_USB_LED1
#ifdef CONFIG_SW_USB_LED0
        board_01_handle_set(LED_USB_0, LED_OFF);
#endif
};

static struct led_operations board_01_operation = {
    .name = "board_g3hgu",
    .handle_init = board_01_handle_init,
    .handle_set = board_01_handle_set,
};

#ifndef CONFIG_PUSHBUTTON_USE_EVENT_HANDLE
static void board_01_pb_init(void) {
    printk("board_01_pb_init\n");
};

static int board_01_pb_is_pushed(int which) {
    switch(which) {
    case PB_RESET:
         if(rtk_gpio->dev_reset_gpio)
           return (gpiod_get_value(rtk_gpio->dev_reset_gpio));
         break;
    case PB_WIFISW:
        if(rtk_gpio->dev_wifi_gpio)
            return (gpiod_get_value(rtk_gpio->dev_wifi_gpio));
        break;
    case PB_WPS:
         if(rtk_gpio->dev_wps_gpio)
           return (gpiod_get_value(rtk_gpio->dev_wps_gpio));
         break;
    default:
        return 0;
    }
    return 0;
}

static struct pushbutton_operations board_01_pb_op = {
    .handle_init = board_01_pb_init,
    .handle_is_pushed = board_01_pb_is_pushed,
};
#endif
#ifdef CONFIG_LEDS_RTK_GPIO_LED
extern void rtk_led_set_all(int control);
extern int rtk_led_get_all(void);
#else
extern void ca77xx_led_set_all(int control);
extern int ca77xx_led_get_all(void);
#endif
static ssize_t
mptest_level_write(struct file *file, const char __user *buf,
        size_t count, loff_t *offset)
{
    char tmpbuf[8];
    int err, val;
    
    memset(tmpbuf, 0, sizeof(tmpbuf));
    if (count > sizeof(tmpbuf)-1)
        count = sizeof(tmpbuf)-1;
    if (copy_from_user(tmpbuf, buf, count))
        return -EFAULT;
    tmpbuf[count] = '\0';
    
    err = kstrtoint(strstrip(tmpbuf), 0, &val);
    if (err < 0)
        return err;
#ifdef CONFIG_LEDS_RTK_GPIO_LED
    rtk_led_set_all(val);
#else       
    ca77xx_led_set_all(val);    
#endif
    return count;
}

static int mptest_level_read(struct seq_file *seq, void *v) {
#ifdef CONFIG_LEDS_RTK_GPIO_LED
    seq_printf(seq, "%d\n", rtk_led_get_all());
#else
    seq_printf(seq, "%d\n", ca77xx_led_get_all());
#endif  
    return 0;
}

static int mptest_open(struct inode *inode, struct file *file)
{
    return single_open(file, mptest_level_read, inode->i_private);
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
static const struct file_operations mptest_fops = {
    .owner          = THIS_MODULE,
    .open           = mptest_open,
    .read           = seq_read,
    .write          = mptest_level_write,
    .llseek         = seq_lseek,
    .release        = single_release,
};
#else
static struct proc_ops mptest_fops = {
        .proc_open           = mptest_open,
        .proc_read           = seq_read,
        .proc_write          = mptest_level_write,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#endif
/***************************************************************
 * Use linux led drivers 
 * So must init after module_platform_driver(ca77xx_leds_driver)
 **************************************************************/
static void board_led_init(void){
    
    rtk_board_led_init();
#ifndef CONFIG_LEDS_RTK_GPIO_LED
    register_reboot_notifier(&board_roboot_notifier);
#endif
}


static int __init board_01_led_init(void) {

    led_register_operations(&board_01_operation);
#ifndef CONFIG_PUSHBUTTON_USE_EVENT_HANDLE
    pb_register_operations(&board_01_pb_op);
#endif
    board_led_init();
    
    proc_create_data("mptest", 0600, NULL, &mptest_fops, NULL);
    
    return 0;
}

static void __exit board_01_led_exit(void) {
}

//module_init(board_01_led_init);
device_initcall_sync(board_01_led_init);
module_exit(board_01_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");


