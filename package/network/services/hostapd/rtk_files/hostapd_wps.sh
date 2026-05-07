#!/bin/sh

#echo $@

# System Settings - START
if [ -f "/bin/rtk_rpc" ];then
	SYSTEMD_USOCK_SERVER=""
	UBUS_OBJ_NAME="rtk-rpc"
elif [ -f "/var/run/systemd.usock" ]; then
	SYSTEMD_USOCK_SERVER="/var/run/systemd.usock"
elif [ -f "/bin/usockd" ];then
	SYSTEMD_USOCK_SERVER=""
	UBUS_OBJ_NAME="rtk-usock"
fi

WPS_LED_INTERFACE="/tmp/wps_led_interface"
LED_WPS_G_LABEL="LED_WPS"
WPS_LED_CONTROL_STATUS_FILE="/tmp/wps_led_control_status"
WPS_LED_CONTROL_RESTORE_FILE="/tmp/wps_led_control_restore"
WPS_LED_TIMER_PID="/var/run/wps_timer.sh.pid"
# System Settings - END

LED_WPS_G_TRIGGER="/sys/class/leds/$LED_WPS_G_LABEL/trigger"
LED_WPS_G_DELAY_ON="/sys/class/leds/$LED_WPS_G_LABEL/delay_on"
LED_WPS_G_DELAY_OFF="/sys/class/leds/$LED_WPS_G_LABEL/delay_off"
LED_WPS_G_BRIGHTNESS="/sys/class/leds/$LED_WPS_G_LABEL/brightness"
LED_WPS_G_SHOT="/sys/class/leds/$LED_WPS_G_LABEL/shot"
LED_WPS_G_MAX_BRIGHTNESS="/sys/class/leds/$LED_WPS_G_LABEL/max_brightness"
WPS_LED_TIMER_FILE="/etc/hostapd_wps_timer.sh"
WPS_EVENT_NEW_AP_SETTINGS="WPS-NEW-AP-SETTINGS"
WPS_EVENT_REG_SUCCESS="WPS-REG-SUCCESS"
WPS_EVENT_ACTIVE="WPS-PBC-ACTIVE"
WPS_EVENT_PIN_ACTIVE="WPS-PIN-ACTIVE"
WPS_EVENT_SUCCESS="WPS-SUCCESS"
WPS_EVENT_OVERLAP="WPS-OVERLAP-DETECTED"
WPS_EVENT_TIMEOUT="WPS-TIMEOUT"
WPS_EVENT_FAIL="WPS-FAIL"
WPA_EVENT_EAP_FAILURE="CTRL-EVENT-EAP-FAILURE"

WPS_START_LED_GPIO_number=2
WPS_PBC_overlapping_GPIO_number=0
WPS_SUCCESS_LED_GPIO_number=1
WPS_SUCCESS_LED_time_out=300000 #ms
WPS_ERROR_LED_GPIO_number=0
WPS_END_LED_unconfig_GPIO_number=0
WPS_END_LED_config_GPIO_number=0

LED_WSC_START=-1
LED_WSC_TIMEOUT=-2
LED_PBC_OVERLAPPED=-3
LED_WSC_ERROR=-4
LED_WSC_SUCCESS=-5
LED_WSC_NOP=-6
LED_WSC_FAIL=-7

wps_reset(){
	if [ -f "$WPS_LED_TIMER_PID" ]; then
		kill -SIGTERM $(cat $WPS_LED_TIMER_PID)
	fi
	if [ -f "$WPS_LED_CONTROL_RESTORE_FILE" ]; then
		rm $WPS_LED_CONTROL_RESTORE_FILE
	fi
}

wps_led_blink(){
	wps_reset
	echo "echo timer > $LED_WPS_G_TRIGGER" > $WPS_LED_CONTROL_RESTORE_FILE
	echo "echo 200 > $LED_WPS_G_DELAY_ON" >> $WPS_LED_CONTROL_RESTORE_FILE
	echo "echo 100 > $LED_WPS_G_DELAY_OFF" >> $WPS_LED_CONTROL_RESTORE_FILE
	if [ ! -f "$WPS_LED_CONTROL_STATUS_FILE" ]; then
		echo "timer" > $LED_WPS_G_TRIGGER
		echo 200 > $LED_WPS_G_DELAY_ON
		echo 100 > $LED_WPS_G_DELAY_OFF
	fi
}

wps_led_change_brightness(){
	wps_reset
	echo "echo none > $LED_WPS_G_TRIGGER" > $WPS_LED_CONTROL_RESTORE_FILE
	if [ "$1" == 0 ]; then
		echo "echo $1 > $LED_WPS_G_BRIGHTNESS" >> $WPS_LED_CONTROL_RESTORE_FILE
	else
		echo "echo $(cat $LED_WPS_G_MAX_BRIGHTNESS) > $LED_WPS_G_BRIGHTNESS" >> $WPS_LED_CONTROL_RESTORE_FILE
	fi
	if [ ! -f "$WPS_LED_CONTROL_STATUS_FILE" ]; then
		echo "none" > $LED_WPS_G_TRIGGER
		if [ "$1" == 0 ]; then
			echo $1 > $LED_WPS_G_BRIGHTNESS
		else
			echo $(cat $LED_WPS_G_MAX_BRIGHTNESS) > $LED_WPS_G_BRIGHTNESS
		fi
	fi
}

wps_led_oneshot(){
	wps_reset
	$WPS_LED_TIMER_FILE $2 $LED_WPS_G_BRIGHTNESS $WPS_LED_TIMER_PID &
	if [ ! -f "$WPS_LED_CONTROL_STATUS_FILE" ]; then
		echo "none" > $LED_WPS_G_TRIGGER
		echo $(cat $LED_WPS_G_MAX_BRIGHTNESS) > $LED_WPS_G_BRIGHTNESS
	fi
}

case "$2" in
$WPS_EVENT_NEW_AP_SETTINGS)
	if [ -z "$SYSTEMD_USOCK_SERVER" ]; then
		if [ "$#" == 3 ]; then
			. /usr/share/libubox/jshn.sh
			json_init
			json_add_string wlan_inf "$1"
			json_add_string hexdump "$3"
			ubus call $UBUS_OBJ_NAME do_wlan_hostapd_wps_new_settings "$(json_dump)"
		else
			. /usr/share/libubox/jshn.sh
			json_init
			json_add_string wlan_inf "$1"
			ubus call $UBUS_OBJ_NAME do_wlan_hostapd_wps_new_settings "$(json_dump)"
		fi
	else
		if [ "$#" == 3 ]; then
			sysconf send_unix_sock_message $SYSTEMD_USOCK_SERVER do_wlan_hostapd_wps_new_settings $1 $3
		else
			sysconf send_unix_sock_message $SYSTEMD_USOCK_SERVER do_wlan_hostapd_wps_new_settings $1
		fi
	fi
	exit 0
	;;
$WPS_EVENT_ACTIVE|$WPS_EVENT_PIN_ACTIVE)
	flag=$LED_WSC_START
	;;
$WPS_EVENT_SUCCESS|$WPS_EVENT_REG_SUCCESS)
	flag=$LED_WSC_SUCCESS
	;;
$WPS_EVENT_OVERLAP)
	flag=$LED_PBC_OVERLAPPED
	;;
$WPS_EVENT_TIMEOUT)
	flag=$LED_WSC_TIMEOUT
	;;
$WPS_EVENT_FAIL)
	flag=$LED_WSC_FAIL
	;;
*)
	exit 1
	;;
esac

case "$flag" in
$LED_WSC_TIMEOUT|$LED_WSC_FAIL)
	enable=$WPS_END_LED_config_GPIO_number
	;;
$LED_WSC_START)
	enable=$WPS_START_LED_GPIO_number
	;;
$LED_PBC_OVERLAPPED)
	enable=$WPS_PBC_overlapping_GPIO_number
	;;
$LED_WSC_ERROR)
	enable=$WPS_ERROR_LED_GPIO_number
	;;
$LED_WSC_SUCCESS)
	enable=$WPS_SUCCESS_LED_GPIO_number
	;;
*)
	exit 1
	;;
esac

if [ $enable == $WPS_START_LED_GPIO_number ]; then
	wps_led_blink
	echo $1 > $WPS_LED_INTERFACE
elif [ $enable == $WPS_SUCCESS_LED_GPIO_number ]; then
	if [ $WPS_SUCCESS_LED_time_out != 0 ]; then
		wps_led_oneshot $enable $WPS_SUCCESS_LED_time_out
	else
		wps_led_change_brightness $enable
	fi
	echo $1 > $WPS_LED_INTERFACE
else
	wps_led_change_brightness $enable
fi

if [ $2 == $WPS_EVENT_SUCCESS ]; then
	if [ -z "$SYSTEMD_USOCK_SERVER" ]; then
		. /usr/share/libubox/jshn.sh
		json_init
		json_add_string wlan_inf "$1"
		ubus call $UBUS_OBJ_NAME do_wlan_hostapd_wps_success "$(json_dump)"
	else
		sysconf send_unix_sock_message $SYSTEMD_USOCK_SERVER do_wlan_hostapd_wps_success $1
	fi
fi

exit 0
