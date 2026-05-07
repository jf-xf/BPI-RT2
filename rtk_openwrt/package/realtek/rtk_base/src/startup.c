/* startup.c  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "common.h"
#include "fc_api.h"
#include "web_ctrl.h"
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
#include <rtk/pon_led.h>
#endif

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
int startPON(void)
{
	char buf[256]={0};
	unsigned int pon_led_spec_type;
	int ret;

	if(mib_get_s2(MIB_PON_LED_SPEC, (void *)&buf, sizeof(buf)) != 0){
		pon_led_spec_type = (unsigned int)atoi(buf);
		if((ret = rtk_pon_led_SpecType_set(pon_led_spec_type)) != 0)
			printf("rtk_pon_led_SpecType_set failed, ret = %d\n", ret);
		else
			printf("rtk_pon_led_SpecType_set %d\n", pon_led_spec_type);
	}
	else
		printf("MIB_PON_LED_SPEC get failed\n");

	return 0;
}
#endif

int main(int argc, char *argv[])
{
	char *val;
	mib_init();

	rtk_web_ctrl_init();
	
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	rtk_fc_ctrl_init();
#endif

	setup_port();
	
	setup_wan_port();
	
	setup_wlan();
	
	set_all_port_powerdown_state(0);
	
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	startPON();
#endif

	mib_commit();
	mib_deinit();
	
	return 0;
}
