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

STA_CONNECTED="CONNECTED"
STA_DISCONNECTED="DISCONNECTED"

# DPP Events
DPP_EVENT_CONNECTOR_STORE="DPP-CONNECTOR-STORE"
DPP_EVENT_CONNECTOR_1905_STORE="DPP-CONNECTOR-1905-STORE"
DPP_EVENT_C_SIGN_KEY_STORE="DPP-C-SIGN-KEY-STORE"
DPP_EVENT_PP_KEY_STORE="DPP-PP-KEY-STORE"
DPP_EVENT_NET_ACCESS_KEY_STORE="DPP-NET-ACCESS-KEY-STORE"
DPP_EVENT_CONFOBJ_SSID_STORE="DPP-CONFOBJ-SSID-STORE"
DPP_EVENT_CONFOBJ_PASS_STORE="DPP-CONFOBJ-PASS-STORE"
DPP_EVENT_CONFOBJ_AKM_STORE="DPP-CONFOBJ-AKM-STORE"
DPP_EVENT_CONFOBJ_PARSE_SUCCESS="DPP-CONFOBJ-PARSE_SUCCESS"

MAP_FILE=/var/multiap_mib.conf
DPP_CONN_FILE=/tmp/dpp_map_conn

MAP_HAPD_WPAS_LUA=/bin/map_hapd_wpas

#echo $@

case "$2" in
$STA_CONNECTED)
	if [ -z "$SYSTEMD_USOCK_SERVER" ]; then
		. /usr/share/libubox/jshn.sh
		json_init
		json_add_string wlan_inf "$1"
		ubus call $UBUS_OBJ_NAME do_wlan_sta_connect_event "$(json_dump)"
	else
		sysconf send_unix_sock_message $SYSTEMD_USOCK_SERVER do_wlan_sta_connect_event $1
	fi
	if [ -f "$MAP_FILE" ]; then
		lua $MAP_HAPD_WPAS_LUA do_sta_connected $1 $2
		lua $MAP_HAPD_WPAS_LUA do_send_map_unix_socket_message $1 $2
	fi
	if [ -f "$DPP_CONN_FILE" ]; then
	    lua $MAP_HAPD_WPAS_LUA do_dpp_connected
	fi
	exit 0
	;;
$STA_DISCONNECTED)
	if [ -f "$DPP_CONN_FILE" ]; then
	    lua $MAP_HAPD_WPAS_LUA do_dpp_disconnected
	fi
	exit 0
	;;
$DPP_EVENT_CONNECTOR_STORE)
	lua $MAP_HAPD_WPAS_LUA do_dpp_store_connector $1 $3
	exit 0
	;;
$DPP_EVENT_CONNECTOR_1905_STORE)
	lua $MAP_HAPD_WPAS_LUA do_dpp_store_connector_1905 $3
	exit 0
	;;
$DPP_EVENT_C_SIGN_KEY_STORE)
	lua $MAP_HAPD_WPAS_LUA do_dpp_store_csign_key $3
	exit 0
	;;
$DPP_EVENT_PP_KEY_STORE)
	lua $MAP_HAPD_WPAS_LUA do_dpp_store_pp_key $3
	exit 0
	;;
$DPP_EVENT_NET_ACCESS_KEY_STORE)
	lua $MAP_HAPD_WPAS_LUA do_dpp_store_net_access_key $3
	exit 0
	;;
$DPP_EVENT_CONFOBJ_AKM_STORE)
	lua $MAP_HAPD_WPAS_LUA do_dpp_store_akm $1 $3
	exit 0
	;;
$DPP_EVENT_CONFOBJ_SSID_STORE)
	lua $MAP_HAPD_WPAS_LUA do_dpp_store_ssid $1 $3
	exit 0
	;;
$DPP_EVENT_CONFOBJ_PASS_STORE)
	lua $MAP_HAPD_WPAS_LUA do_dpp_store_psk $1 $3
	exit 0
	;;
$DPP_EVENT_CONFOBJ_PARSE_SUCCESS)
	lua $MAP_HAPD_WPAS_LUA do_dpp_store_success
	exit 0
	;;
*)
	exit 1
	;;
esac
