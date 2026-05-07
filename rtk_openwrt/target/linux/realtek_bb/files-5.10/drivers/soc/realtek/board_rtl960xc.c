//#include "bspgpio.h"
#include "led-generic.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <dt-bindings/gpio/gpio.h>
#include <rtk/i2c.h>
#include <rtk/init.h>
#include <rtk/led.h>
#include <common/error.h>
//#include "pushbutton.h"

/* set board led initial state */
static void board_init_led(void) {
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
  rtk_led_parallelEnable_set(2, ENABLED);  // port 0
  rtk_led_parallelEnable_set(3, ENABLED);  // port 1
  rtk_led_parallelEnable_set(17, ENABLED); // port 2
  rtk_led_parallelEnable_set(5, ENABLED);  // port 3
  //rtk_led_parallelEnable_set(16, ENABLED); // port 4
  //rtk_led_parallelEnable_set(4, ENABLED);  // port 5
  rtk_led_parallelEnable_set(9, ENABLED);  // LOS
  	
  rtk_led_config_set(2,  LED_TYPE_UTP0,  &led_cfg);
  rtk_led_config_set(3,  LED_TYPE_UTP1,  &led_cfg);
  rtk_led_config_set(17, LED_TYPE_UTP2,  &led_cfg);
  rtk_led_config_set(5,  LED_TYPE_UTP3,  &led_cfg);
  //rtk_led_config_set(16, LED_TYPE_UTP4,  &led_cfg);
  //rtk_led_config_set(4,  LED_TYPE_FIBER, &led_cfg);
  rtk_led_config_set(9,  LED_TYPE_PON,   &led_cfg);
  
  //memset(&led_cfg, 0x0, sizeof(rtk_led_config_t));
  //led_cfg.ledEnable[LED_CONFIG_FORCE_MODE] = ENABLED;	
  //rtk_led_config_set(0, LED_TYPE_PON, &led_cfg); // power led
  //rtk_led_modeForce_set(0, LED_FORCE_BLINK);
  if(rtk_led_blinkRate_set(LED_BLINK_GROUP_FORCE_MODE, LED_BLINKRATE_256MS) != RT_ERR_OK)
	  printk("\n %s %d\n",__FUNCTION__,__LINE__);
}

void rtk_board_mptest_led(int op) {
  switch(op) {
	case 0: /* all off */
	case 1: /* all on */
		break;
	default: /* restore to default */
		board_init_led();
  }
}
//EXPORT_SYMBOL(rtk_board_mptest_led);

#if 0 //ndef CONFIG_PUSHBUTTON_USE_EVENT_HANDLE
static int gpio_reset = -1;
static int gpio_wifi  = -1;
static enum of_gpio_flags gpio_reset_flags;
static enum of_gpio_flags gpio_wifi_flags;
static void board_pb_init(void) {
};

static int board_pb_is_pushed(int which) {
  int ret = 0;
  switch(which) {
  case PB_RESET:
    if (gpio_reset>=0) {
		ret = gpio_get_value(gpio_reset);	
		if (GPIO_ACTIVE_LOW&gpio_reset_flags)
			ret = !ret;
	}
	break;
  case PB_WIFISW:
    if (gpio_wifi>=0) {
		ret = gpio_get_value(gpio_wifi);	
		if (GPIO_ACTIVE_LOW&gpio_wifi_flags)
			ret = !ret;
	}
	break;
  }
  return ret;
}

static struct pushbutton_operations board_pb_op = {
  .handle_init = board_pb_init,
  .handle_is_pushed = board_pb_is_pushed,
};
#endif

static int __init board_init(void) {

#ifdef CONFIG_PCIE_POWER_SAVING
  REG32(IO_MODE_EN_REG) |= IO_MODE_INTRPT1_EN; // ENABLE interrupt 1
#endif
  rtk_core_init();
  rtk_i2c_init(0);
  if(rtk_led_init() != RT_ERR_OK)
	  printk("\n %s %d\n",__FUNCTION__,__LINE__);
  board_init_led();

#if 0//ndef CONFIG_PUSHBUTTON_USE_EVENT_HANDLE
  /* probe device tree for push-button */
  do {
	  struct device_node *dn, *child;
	  
	  dn = of_find_compatible_node(NULL, NULL, "gpio-keys-polled");
	  if (dn){
		  const char *btn_name;
		  for_each_child_of_node(dn, child) {
			  enum of_gpio_flags flags;
			  int gpio;
			  of_property_read_string(child, "label", &btn_name);
			  gpio = of_get_named_gpio_flags(child, "gpios", 0, &flags);
			  if (btn_name && (gpio >= 0)) {				
				  if (!strcmp("btn,wifi", btn_name)) {
					  gpio_wifi = gpio;
					  gpio_wifi_flags = flags;
				  } else if (!strcmp("btn,reset", btn_name)) {
					  gpio_reset = gpio;
					  gpio_reset_flags = flags;
				  }
			  }
		  }
	  }
  } while (0);   
	//pb_register_operations(&board_pb_op);
#endif
    
  return 0;
}

module_init(board_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RTL960XC board initialization");
