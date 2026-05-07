#!/bin/sh

[ -e /lib/functions.sh ] && . /lib/functions.sh || . ./functions.sh

FW3_WIFI_BLOCK_CHAIN="wlan_block"
EBTABLES=`which ebtables`

# clear ebtable wifi block chain
$EBTABLES -t filter -D FORWARD -j $FW3_WIFI_BLOCK_CHAIN
$EBTABLES -t filter -F $FW3_WIFI_BLOCK_CHAIN
$EBTABLES -t filter -X $FW3_WIFI_BLOCK_CHAIN

# add ebtable wifi block chain
$EBTABLES -t filter -N $FW3_WIFI_BLOCK_CHAIN
$EBTABLES -t filter -P $FW3_WIFI_BLOCK_CHAIN RETURN
$EBTABLES -t filter -A FORWARD -j $FW3_WIFI_BLOCK_CHAIN

config_load network
config_get wlan_block "rtkglobals" wlan_block "0"

if [ "$wlan_block" = "1" ]; then
	$EBTABLES -t filter -A $FW3_WIFI_BLOCK_CHAIN -i wlan+ -o eth0.+ -j DROP
	$EBTABLES -t filter -A $FW3_WIFI_BLOCK_CHAIN -i eth0.+ -o wlan+ -j DROP
fi
