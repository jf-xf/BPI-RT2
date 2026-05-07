#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <dt-bindings/gpio/gpio.h>
#include <rtk/init.h>
#include <rtk/led.h>
#include "rtk_gpio.h"
#include "led-generic.h"
#include "pushbutton.h"

#if defined(CONFIG_EXTERNAL_PHY_POLLING)
#include <rtk/mdio.h>
#include <rtk/gpio.h>
#include <linux/delay.h>
#define REG32(reg)      (*(volatile unsigned int   *)(reg))
#define BIT(nr)         (1UL << (nr))
#endif

#if 0 //ndef CONFIG_PUSHBUTTON_USE_EVENT_HANDLE
static void board_pb_init(void) {
	printk("board_pb_init\n");
};

static int board_pb_is_pushed(int which) {
    switch(which) {
    case PB_RESET:
        if(rtk_gpio->dev_reset_gpio) //just for reboot
            return (gpiod_get_value(rtk_gpio->dev_reset_gpio));
        break;
        //FH_V100 default/WPS uses same button
    case PB_WPS:
        if(rtk_gpio->dev_wps_gpio)
            return (gpiod_get_value(rtk_gpio->dev_wps_gpio));
        break;
    default:
        return 0;
    }
    return 0;
}

static struct pushbutton_operations board_pb_op = {
    .handle_init = board_pb_init,
    .handle_is_pushed = board_pb_is_pushed,
};
#endif
/***************************************************************
 * Use linux led drivers
 * So must init after module_platform_driver(ca77xx_leds_driver)
 **************************************************************/

static void board_led_init(void){
	/* ethernet LED */
#ifdef CONFIG_RTK_BI_COLOR_LED_SUPPORT
	rtk_led_config_t led_cfg_giga;
	rtk_led_config_t led_cfg_10_100m;
  
	memset(&led_cfg_giga, 0, sizeof(led_cfg_giga));  
	led_cfg_giga.ledEnable[LED_CONFIG_SPD1000ACT] = ENABLED;
	led_cfg_giga.ledEnable[LED_CONFIG_SPD500ACT] = ENABLED;
	led_cfg_giga.ledEnable[LED_CONFIG_SPD1000] = ENABLED;
	led_cfg_giga.ledEnable[LED_CONFIG_SPD500] = ENABLED;

	memset(&led_cfg_10_100m, 0, sizeof(led_cfg_10_100m));
	led_cfg_10_100m.ledEnable[LED_CONFIG_SPD100ACT] = ENABLED;
	led_cfg_10_100m.ledEnable[LED_CONFIG_SPD10ACT] = ENABLED;
	led_cfg_10_100m.ledEnable[LED_CONFIG_SPD100] = ENABLED;
	led_cfg_10_100m.ledEnable[LED_CONFIG_SPD10] = ENABLED;
  	
	rtk_led_operation_set(LED_OP_PARALLEL);
	rtk_led_parallelEnable_set(14, ENABLED);  // port 0
	rtk_led_parallelEnable_set(13, ENABLED);  // port 1
	rtk_led_parallelEnable_set(17, ENABLED); // port 2
	rtk_led_parallelEnable_set(5, ENABLED);  // port 3
	rtk_led_parallelEnable_set(4, ENABLED);  // port 0
	rtk_led_parallelEnable_set(1, ENABLED);  // port 1
	rtk_led_parallelEnable_set(9, ENABLED);  // port 2
	rtk_led_parallelEnable_set(8, ENABLED);  // port 3
  	
	rtk_led_config_set(14,  LED_TYPE_UTP0,  &led_cfg_giga);
	rtk_led_config_set(13,  LED_TYPE_UTP1,  &led_cfg_giga);
	rtk_led_config_set(17, LED_TYPE_UTP2,  &led_cfg_giga);
	rtk_led_config_set(5,  LED_TYPE_UTP3,  &led_cfg_giga);

	rtk_led_config_set(4,  LED_TYPE_UTP0,  &led_cfg_10_100m);
	rtk_led_config_set(1,  LED_TYPE_UTP1,  &led_cfg_10_100m);
	rtk_led_config_set(9,  LED_TYPE_UTP2,  &led_cfg_10_100m);
	rtk_led_config_set(8,  LED_TYPE_UTP3,  &led_cfg_10_100m);
	
#else
	rtk_led_config_t led_cfg;
  
	memset(&led_cfg, 0, sizeof(led_cfg));
  
	led_cfg.ledEnable[LED_CONFIG_SPD1000ACT] = ENABLED;
	led_cfg.ledEnable[LED_CONFIG_SPD500ACT] = ENABLED;
	led_cfg.ledEnable[LED_CONFIG_SPD100ACT] = ENABLED;
	led_cfg.ledEnable[LED_CONFIG_SPD10ACT] = ENABLED;
	led_cfg.ledEnable[LED_CONFIG_SPD1000] = ENABLED;
	led_cfg.ledEnable[LED_CONFIG_SPD500] = ENABLED;
	led_cfg.ledEnable[LED_CONFIG_SPD100] = ENABLED;
	led_cfg.ledEnable[LED_CONFIG_SPD10] = ENABLED;
  	
	rtk_led_operation_set(LED_OP_PARALLEL);
	rtk_led_parallelEnable_set(14, ENABLED);  // port 0
	rtk_led_parallelEnable_set(13, ENABLED);  // port 1
	rtk_led_parallelEnable_set(17, ENABLED); // port 2
	rtk_led_parallelEnable_set(5, ENABLED);  // port 3
  	
	rtk_led_config_set(14,  LED_TYPE_UTP0,  &led_cfg);
	rtk_led_config_set(13,  LED_TYPE_UTP1,  &led_cfg);
	rtk_led_config_set(17, LED_TYPE_UTP2,  &led_cfg);
	rtk_led_config_set(5,  LED_TYPE_UTP3,  &led_cfg);
#endif
  
#if defined(CONFIG_EXTERNAL_PHY_POLLING)
	{
	uint16 data;

	/* LAN4 LED: 98D port 6 (8211FS), controlled by 8211FS, set phy page 0xd04 reg 16 bit 0/1/3/4 = 1 */
	rtk_mdio_c22_write(31, 0xd04);
	rtk_mdio_c22_read(16, &data);
	data |= 0x1b;
	rtk_mdio_c22_write(16, data);
	rtk_mdio_c22_write(31, 0);

	// enable reg4.10: advertise support of pause frames
	rtk_mdio_c22_read(4, &data);
	data |= (1<< 10);
	rtk_mdio_c22_write(4, data);
	
#ifdef CONFIG_SWITCH_INIT_LINKDOWN
	rtk_mdio_c22_read(0, &data);
	data |= (1<< 11);
	rtk_mdio_c22_write(0, data);
#endif

	//REG32(0xBB041800) |= (1 << 8); /* set SP_INV_HSO=1, invert the serdes output 20-bit data (polarity change manually) */
	//REG32(0xBB041800) |= ((1 << 8) | (1 << 9)); /* set SP_INV_HSI=1 and SP_INV_HSO=1, invert the serdes input/output 20-bit data (polarity change manually) */	
	}
#endif

	//rtk_led_blinkRate_set(LED_BLINK_GROUP_FORCE_MODE, LED_BLINKRATE_256MS);
#ifndef CONFIG_LEDS_GPIO
	if(rtk_gpio->dev_sys_led_gpio){
		gpiod_set_value(rtk_gpio->dev_sys_led_gpio, 1);
	}
	if(rtk_gpio->dev_wifi_2G_led_gpio){
		gpiod_set_value(rtk_gpio->dev_wifi_2G_led_gpio, 1);
	}
#endif
}

#if defined(CONFIG_EXTERNAL_PHY_POLLING)
#define MDIO_VIRTUAL_PORT   0
#define MDIO_PHYID          5
#ifdef CONFIG_MDC_MDIO_HW_SET
#define MDIO_SET            CONFIG_MDC_MDIO_HW_SET
#else
#define MDIO_SET            0
#endif

#if (MDIO_SET==0)
#define _GPIO_SET0_MDC		6
#define _GPIO_SET0_MDIO		7
#else
#define _GPIO_SET0_MDC		12
#define _GPIO_SET0_MDIO		10
#endif

static const unsigned short phy_data[]={
 31, 0x0A43,
 27, 0x8146,
 28, 0x9501,
 31, 0x0B82,
 23, 0x0001,
 31, 0x0A4B,
 17, 0x1110,
 31, 0x0000,
 13, 0x0007,
 14, 0x003C,
 13, 0x4007,
 14, 0x0000,
 31, 0x0A42,
 22, 0x0F10,
 31, 0x0A43,
 27, 0x84AA,
 28, 0xAF84,
 28, 0xB6AF,
 28, 0x853B,
 28, 0xAF85,
 28, 0x6FAF,
 28, 0x85D0,
 28, 0x020E,
 28, 0x6AF8,
 28, 0xF9FA,
 28, 0xEF69,
 28, 0xFAE1,
 28, 0x8BF9,
 28, 0xAC28,
 28, 0x0EBF,
 28, 0x85DF,
 28, 0x025E,
 28, 0x18E5,
 28, 0x8BFF,
 28, 0xD101,
 28, 0xE58B,
 28, 0xF9BF,
 28, 0x85D3,
 28, 0x025E,
 28, 0x18EF,
 28, 0x64E5,
 28, 0x8BF4,
 28, 0xE38B,
 28, 0xFB1F,
 28, 0x221B,
 28, 0x45AC,
 28, 0x2711,
 28, 0xEF46,
 28, 0xE38B,
 28, 0xF71B,
 28, 0x45AD,
 28, 0x2716,
 28, 0xD102,
 28, 0xE58B,
 28, 0xF3AE,
 28, 0x39D1,
 28, 0x01E5,
 28, 0x8BF3,
 28, 0xE18B,
 28, 0xFFE2,
 28, 0x8BFE,
 28, 0x1A12,
 28, 0xAE0D,
 28, 0xD104,
 28, 0xE58B,
 28, 0xF3E1,
 28, 0x8BFF,
 28, 0xE28B,
 28, 0xF81A,
 28, 0x12AD,
 28, 0x2C02,
 28, 0xD10F,
 28, 0xBF85,
 28, 0xDF02,
 28, 0x5DDA,
 28, 0xBF85,
 28, 0xE202,
 28, 0x5DDA,
 28, 0xBF85,
 28, 0xE502,
 28, 0x5DDA,
 28, 0xBF85,
 28, 0xE802,
 28, 0x5DDA,
 28, 0xFEEF,
 28, 0x96FE,
 28, 0xFDFC,
 28, 0xAF0D,
 28, 0x2402,
 28, 0x0D3F,
 28, 0xF8FA,
 28, 0xEF69,
 28, 0x1F11,
 28, 0xE58B,
 28, 0xF3E0,
 28, 0x8BF9,
 28, 0xAD20,
 28, 0x1BE1,
 28, 0x8BFF,
 28, 0xBF85,
 28, 0xDF02,
 28, 0x5DDA,
 28, 0xBF85,
 28, 0xE202,
 28, 0x5DDA,
 28, 0xBF85,
 28, 0xE502,
 28, 0x5DDA,
 28, 0xBF85,
 28, 0xE802,
 28, 0x5DDA,
 28, 0xEF96,
 28, 0xFEFC,
 28, 0xAF0C,
 28, 0xBB02,
 28, 0x4EF3,
 28, 0xF8FA,
 28, 0xEF69,
 28, 0xFABF,
 28, 0x62FE,
 28, 0x025E,
 28, 0x18A1,
 28, 0x0148,
 28, 0xBF5E,
 28, 0x6C02,
 28, 0x5E18,
 28, 0xEF01,
 28, 0x0D14,
 28, 0x7903,
 28, 0x9F17,
 28, 0xEF10,
 28, 0x7930,
 28, 0x9E1F,
 28, 0xEF10,
 28, 0x793F,
 28, 0x9E19,
 28, 0xBF85,
 28, 0xF402,
 28, 0x5E18,
 28, 0xAD28,
 28, 0x10AE,
 28, 0x1BEF,
 28, 0x100D,
 28, 0x1479,
 28, 0x049F,
 28, 0x06EF,
 28, 0x1039,
 28, 0x44AB,
 28, 0x0DD1,
 28, 0x00BF,
 28, 0x6301,
 28, 0x025D,
 28, 0xDA02,
 28, 0x6404,
 28, 0xAE08,
 28, 0xD101,
 28, 0xBF63,
 28, 0x0102,
 28, 0x5DDA,
 28, 0xFEEF,
 28, 0x96FE,
 28, 0xFCAF,
 28, 0x0400,
 28, 0xAF00,
 28, 0x1170,
 28, 0xA880,
 28, 0x60A8,
 28, 0x18E8,
 28, 0xA818,
 28, 0x60A8,
 28, 0x1A30,
 28, 0xBCDC,
 28, 0x74BC,
 28, 0xDCB8,
 28, 0xBCDC,
 28, 0xFCBC,
 28, 0xDC96,
 28, 0xA810,
 28, 0x40A8,
 28, 0x10F0,
 28, 0xA804,
 28, 0x11A7,
 28, 0x1200,
 27, 0x80FF,
 28, 0x2DED,
 27, 0x811E,
 28, 0x2DED,
 27, 0x809A,
 28, 0xA444,
 27, 0x809F,
 28, 0x6B13,
 27, 0x80A5,
 28, 0xAF00,
 27, 0x80A9,
 28, 0x2300,
 27, 0x80A0,
 28, 0x2300,
 27, 0x8BF3,
 28, 0x0000,
 27, 0x8BF5,
 28, 0x0800,
 27, 0x8BF7,
 28, 0x1E05,
 27, 0x8BF8,
 28, 0x0500,
 27, 0x8BFA,
 28, 0x021E,
 27, 0x8BFC,
 28, 0x7A7A,
 27, 0x8BFE,
 28, 0x0200,
 27, 0xB818,
 28, 0x0D21,
 27, 0xB81A,
 28, 0x0CB8,
 27, 0xB81C,
 28, 0x03FD,
 27, 0xB81E,
 28, 0x000F,
 27, 0xB832,
 28, 0x0007,
 31, 0x0A43,
 27, 0x0000,
 28, 0x0000,
 27, 0x8146,
 28, 0x0000,
 31, 0x0B82,
 23, 0x0000,
 16, 0x0000,
};

void rtl8211fs_init(void)
{
	int ready_to_patch = 0, len, i;
	uint16 data;

	// switch MDC/MDIO gpio pin to MDC/MDIO mode
	rtk_gpio_state_set(_GPIO_SET0_MDC, DISABLED);
	rtk_gpio_state_set(_GPIO_SET0_MDIO, DISABLED);

	rtk_mdio_init();
	rtk_mdio_cfg_set(MDIO_SET,MDIO_VIRTUAL_PORT,MDIO_PHYID,MDIO_FMT_C22);

	rtk_mdio_c22_write(31, 0x0B82);
	rtk_mdio_c22_write(16, 0x0010);
	rtk_mdio_c22_write(31, 0x0B80);
	rtk_mdio_c22_read(16, &data);
	if (data & BIT(6))
		ready_to_patch = 1;
	else {
		mdelay(400);
		rtk_mdio_c22_read(16, &data);
		if (data & BIT(6))
			ready_to_patch = 1;
	}

	if (!ready_to_patch)
		return;

	len = sizeof(phy_data)/sizeof(unsigned short);
	for(i=0;i<len;i=i+2) {
		rtk_mdio_c22_write(phy_data[i], phy_data[i+1]);
	}

	rtk_mdio_c22_write(31, 0x0);
	printk("RTL8211FS patch completely\n");

	return;
}
#endif

/*********************************************************
 * Device Tree Blob
 **********************************************************/


static int __init board_init(void) {
    printk("[%s - %d]\n", __func__, __LINE__);
#if 0 //ndef CONFIG_PUSHBUTTON_USE_EVENT_HANDLE
    pb_register_operations(&board_pb_op);
#endif

#if defined(CONFIG_EXTERNAL_PHY_POLLING)
    rtl8211fs_init();
#endif

    board_led_init();

    return 0;
}

static void __exit board_exit(void) {
}

#ifndef CONFIG_RTK_PROC_GPIO_FILES
#ifdef CONFIG_RTL_8221B_SUPPORT
const char board_setting_name[] = "board_setting";
const char rtl8221b_reset_name[2][20] = {"rtl8221b_dev0_reset", "rtl8221b_dev1_reset"};

static int rtl8221b_reset_pin(const char *prop_name)
{
  int gpio = -EFAULT;
  struct device_node *node;
  enum of_gpio_flags flags;

  node = of_find_node_by_name(NULL, board_setting_name);
  if (node) {
    gpio = of_get_named_gpio_flags(node, prop_name, 0, &flags);
    printk("RTL8221B: %s pin is set to GPIO #%d\n", prop_name, gpio);
  }
  return (gpio);
}

int rtk_8221b_reset_gpio_get(unsigned int dev)
{
  if (dev > 1)
    return (-EFAULT);
  else
    return (rtl8221b_reset_pin(rtl8221b_reset_name[dev]));
}

EXPORT_SYMBOL(rtk_8221b_reset_gpio_get);
#endif
#endif

module_init(board_init);
module_exit(board_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RTL8198D board initialization");
