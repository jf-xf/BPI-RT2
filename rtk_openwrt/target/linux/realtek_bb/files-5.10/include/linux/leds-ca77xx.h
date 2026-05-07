#ifndef __LINUX_LEDS_CA77XX_H
#define __LINUX_LEDS_CA77XX_H

#define TRIGGER_HW_RX		0
#define TRIGGER_HW_TX		1
#define TRIGGER_SW		2
#define TRIGGER_NONE		3

#define BLINK_RATE1		0
#define BLINK_RATE2		1

int ca77xx_led_test_mode(int enable);
int ca77xx_led_config(int idx, int active_low, int off_event, int blink_event,
		      int on_event, int port, int blink);
int ca77xx_led_enable(int idx, int enable);
int ca77xx_led_sw_on(int idx, int on);

#endif /* __LINUX_LEDS_CA77XX_H */

