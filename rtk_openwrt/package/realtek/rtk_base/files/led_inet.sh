#!/bin/sh

. /lib/functions.sh

LED_LABEL=/sys/class/leds/LED_FUN_G/brightness
num_up=0
num_down=0

handle_interface() {
	local ifname=$1
	local status

	status=$(ubus call network.interface.$ifname status | jsonfilter -e "@.up")
	[ "$status" = "true" ] && num_up="$(($num_up + 1))"
	[ "$status" = "false" ] && num_down="$(($num_down + 1))"
}

handle_internet() {
	local name
	
	config_get name $1 name
	[ "$name" = "INTERNET" ] && {
		config_list_foreach $1 network handle_interface
		#echo num_up=$num_up > /dev/console
		#echo num_down=$num_down > /dev/console
	}
}

#echo ACTION=$ACTION > /dev/console
#echo DEVICE=$DEVICE > /dev/console
#echo INTERFACE=$INTERFACE > /dev/console

config_load firewall
config_foreach handle_internet zone
if [ $num_down -gt 0 ]; then
	echo 0 > "$LED_LABEL"
else
	if [ $num_up -gt 0 ]; then
		echo 1 > "$LED_LABEL"
	else
		echo 0 > "$LED_LABEL"
	fi
fi
