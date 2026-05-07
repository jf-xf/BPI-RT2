#!/bin/sh

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
# System Settings - END

AP_EVENT_ENABLED="AP-ENABLED"
AP_EVENT_DISABLED="AP-DISABLED"
WPA_EVENT_CHANNEL_SWITCH="CTRL-EVENT-CHANNEL-SWITCH"
AP_STA_CONNECTED="AP-STA-CONNECTED"
AP_STA_DISCONNECTED="AP-STA-DISCONNECTED"
AP_STA_POSSIBLE_PSK_MISMATCH="AP-STA-POSSIBLE-PSK-MISMATCH"

# DPP Events
DPP_CHIRP_RX="DPP-CHIRP-RX"

MAP_FILE=/var/multiap_mib.conf

MAP_HAPD_WPAS_LUA=/bin/map_hapd_wpas

#echo $@

case "$2" in
$AP_EVENT_ENABLED)
	exit 0
	;;
$AP_EVENT_DISABLED)
	exit 0
	;;
$WPA_EVENT_CHANNEL_SWITCH)
	if [ -z "$SYSTEMD_USOCK_SERVER" ]; then
		. /usr/share/libubox/jshn.sh
		json_init
		json_add_string wlan_inf "$1"
		ubus call $UBUS_OBJ_NAME do_wlan_channel_change_event "$(json_dump)"
	else
		sysconf send_unix_sock_message $SYSTEMD_USOCK_SERVER do_wlan_channel_change_event $1
	fi
	exit 0
	;;
$AP_STA_CONNECTED)
	if [ -z "$SYSTEMD_USOCK_SERVER" ]; then
		. /usr/share/libubox/jshn.sh
		json_init
		json_add_string wlan_inf "$1"
		json_add_string sta_mac "$3"
		ubus call $UBUS_OBJ_NAME do_wlan_ap_sta_connect_event "$(json_dump)"
	else
		sysconf send_unix_sock_message $SYSTEMD_USOCK_SERVER do_wlan_ap_sta_connect_event $1 $3
	fi
	if [ -f "$MAP_FILE" ]; then
		lua $MAP_HAPD_WPAS_LUA do_send_map_unix_socket_message $1 $2 $3
	fi
	exit 0
	;;
$AP_STA_DISCONNECTED)
	if [ -z "$SYSTEMD_USOCK_SERVER" ]; then
		. /usr/share/libubox/jshn.sh
		json_init
		json_add_string wlan_inf "$1"
		json_add_string sta_mac "$3"
		ubus call $UBUS_OBJ_NAME do_wlan_ap_sta_disconnect_event "$(json_dump)"
	else
		sysconf send_unix_sock_message $SYSTEMD_USOCK_SERVER do_wlan_ap_sta_disconnect_event $1 $3
	fi
	if [ -f "$MAP_FILE" ]; then
		lua $MAP_HAPD_WPAS_LUA do_send_map_unix_socket_message $1 $2 $3
	fi
	exit 0
	;;
$AP_STA_POSSIBLE_PSK_MISMATCH)
	if [ -z "$SYSTEMD_USOCK_SERVER" ]; then
		. /usr/share/libubox/jshn.sh
		json_init
		json_add_string wlan_inf "$1"
		json_add_string sta_mac "$3"
		ubus call $UBUS_OBJ_NAME do_wlan_ap_sta_psk_mismatch_event "$(json_dump)"
	else
		sysconf send_unix_sock_message $SYSTEMD_USOCK_SERVER do_wlan_ap_sta_psk_mismatch_event $1 $3
	fi
	if [ -f "$MAP_FILE" ]; then
		lua $MAP_HAPD_WPAS_LUA do_send_map_unix_socket_message $1 $2 $3
	fi
	exit 0
	;;
$DPP_CHIRP_RX)
	if [ -f "$MAP_FILE" ]; then
		lua $MAP_HAPD_WPAS_LUA do_send_map_unix_socket_message $1 $2 $3 $4 $5 $6
	fi
	exit 0
	;;
*)
	exit 1
	;;
esac
