#!/bin/sh

. /lib/functions.sh

# $1: device name
# $2: disabled value
wlan_onoff() {
	case $2 in
		0)
			uci set wireless.$1.disabled=1
			echo "$1 disabled" > /dev/console
		;;
		1)
			uci set wireless.$1.disabled=0
			echo "$1 enabled" > /dev/console
		;;
	esac
}

handle_wlan() {
	local disabled
	
	echo device=$1 > /dev/console
	config_get disabled $1 disabled
	[ -n "$disabled" ] || disabled=0
	wlan_onoff $1 $disabled
}

handle_by_wlan0() {
	local disabled

	# The wlan onoff behavior will set all radio on or off based on 
	# first radio status.
	disabled=$(uci get wireless.@wifi-device[0].disabled)
	[ -n "$disabled" ] || disabled=0
	config_foreach wlan_onoff wifi-device $disabled
}

if [ "$ACTION" = "released" ] && [ "$BUTTON" = "wlan" ]; then
	# If the button was pressed for 3 seconds or more, toggle WiFi onoff.
	if [ "$SEEN" -ge 3 ] ; then
		config_load wireless
		#config_foreach handle_wlan wifi-device
		handle_by_wlan0
		uci commit wireless
		wifi
	fi
fi

