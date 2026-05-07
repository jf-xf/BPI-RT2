#!/bin/sh

SSID1=BPI-RT2-5G
SSID2=BPI-RT2-2.4G
SERVERSSID=BPI-RT2-Server
append DRIVERS "mac80211"

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d /sys/class/ieee80211/$phy ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		phy="$(iwinfo nl80211 phyname "path=$devpath")"
		[ -n "$phy" ] && return
	}

	local macaddr="$(config_get "$device" macaddr | tr 'A-Z' 'a-z')"
	[ -n "$macaddr" ] && {
		for _phy in /sys/class/ieee80211/*; do
			[ -e "$_phy" ] || continue

			[ "$macaddr" = "$(cat ${_phy}/macaddress)" ] || continue
			phy="${_phy##*/}"
			return
		done
	}
	phy=
	return
}

find_mac80211_phy() {
	local device="$1"

	config_get phy "$device" phy
	lookup_phy
	[ -n "$phy" -a -d "/sys/class/ieee80211/$phy" ] || {
		echo "PHY for wifi device $1 not found"
		return 1
	}
	config_set "$device" phy "$phy"

	config_get macaddr "$device" macaddr
	[ -z "$macaddr" ] && {
		config_set "$device" macaddr "$(cat /sys/class/ieee80211/${phy}/macaddress)"
	}

	return 0
}

check_mac80211_device() {
	config_get phy "$1" phy
	[ -z "$phy" ] && {
		find_mac80211_phy "$1" >/dev/null || return 0
		config_get phy "$1" phy
	}
	[ "$phy" = "$dev" ] && found=1
}


__get_band_defaults() {
	local phy="$1"

	( iw phy "$phy" info; echo ) | awk '
BEGIN {
        bands = ""
}

($1 == "Band" || $1 == "") && band {
        if (channel) {
		mode="NOHT"
		if (ht) mode="HT20"
		if (vht && band != "1:") mode="VHT80"
		if (he) mode="HE80"
		if (he && band == "1:") mode="HE20"
                sub("\\[", "", channel)
                sub("\\]", "", channel)
                bands = bands band channel ":" mode " "
        }
        band=""
}

$1 == "Band" {
        band = $2
        channel = ""
	vht = ""
	ht = ""
	he = ""
}

$0 ~ "Capabilities:" {
	ht=1
}

$0 ~ "VHT Capabilities" {
	vht=1
}

$0 ~ "HE Iftypes" {
	he=1
}

$1 == "*" && $3 == "MHz" && $0 !~ /disabled/ && band && !channel {
        channel = $4
}

END {
        print bands
}'
}

get_band_defaults() {
	local phy="$1"

	for c in $(__get_band_defaults "$phy"); do
		local band="${c%%:*}"
		c="${c#*:}"
		local chan="${c%%:*}"
		c="${c#*:}"
		local mode="${c%%:*}"

		case "$band" in
			1) band=2g;;
			2) band=5g;;
			3) band=60g;;
			4) band=6g;;
			*) band="";;
		esac

		[ -n "$band" ] || continue
		[ -n "$mode_band" -a "$band" = "6g" ] && return

		mode_band="$band"
		channel="$chan"
		htmode="$mode"
	done
}

detect_mac80211() {
	devidx=0
	ifname="wlan1"
	ifname1="wlan0"
	config_load wireless
	while :; do
		config_get type "phy$devidx" type
		[ -n "$type" ] || break
		devidx=$(($devidx + 1))
	done

	for _dev in /sys/class/ieee80211/*; do
		[ -e "$_dev" ] || continue

		dev="${_dev##*/}"

		found=0
		config_foreach check_mac80211_device wifi-device
		[ "$found" -gt 0 ] && continue

		mode_band=""
		channel=""
		htmode=""
		ht_capab=""
		is_2g_256qam="0"

		get_band_defaults "$dev"

		path="$(iwinfo nl80211 path "$dev")"
		if [ -n "$path" ]; then
			dev_id="set wireless.phy${devidx}.path='$path'"
		else
			dev_id="set wireless.phy${devidx}.macaddr=$(cat /sys/class/ieee80211/${dev}/macaddress)"
		fi

		rtk_detect_mac80211 "$dev"

		if [ ${devidx} -eq 0 ]; then
			uci -q batch <<-EOF
				set wireless.phy${devidx}=wifi-device
				set wireless.phy${devidx}.type=mac80211
				${dev_id}
				set wireless.phy${devidx}.channel=${channel}
				set wireless.phy${devidx}.band=${mode_band}
				set wireless.phy${devidx}.htmode=$htmode
				set wireless.phy${devidx}.country=US

				set wireless.default_radio${devidx}0=wifi-iface
				set wireless.default_radio${devidx}0.device=phy${devidx}
				set wireless.default_radio${devidx}0.network=lan
				set wireless.default_radio${devidx}0.mode=ap
				set wireless.default_radio${devidx}0.ssid=${SSID1}
				set wireless.default_radio${devidx}0.encryption=none
				set wireless.default_radio${devidx}0.bss_load_update_period=0
				set wireless.default_radio${devidx}0.map_bss_type=32
				set wireless.default_radio${devidx}0.ifname=${ifname}

				set wireless.default_radio${devidx}1=wifi-iface
				set wireless.default_radio${devidx}1.device=phy${devidx}
				set wireless.default_radio${devidx}1.network=lan
				set wireless.default_radio${devidx}1.mode=ap
				set wireless.default_radio${devidx}1.ssid=${SSID1}-1
				set wireless.default_radio${devidx}1.encryption=none
				set wireless.default_radio${devidx}1.bss_load_update_period=0
				set wireless.default_radio${devidx}1.map_bss_type=32
				set wireless.default_radio${devidx}1.ifname=${ifname}-vap0
				set wireless.default_radio${devidx}1.disabled=1

				set wireless.default_radio${devidx}2=wifi-iface
				set wireless.default_radio${devidx}2.device=phy${devidx}
				set wireless.default_radio${devidx}2.network=lan
				set wireless.default_radio${devidx}2.mode=ap
				set wireless.default_radio${devidx}2.ssid=${SSID1}-2
				set wireless.default_radio${devidx}2.encryption=none
				set wireless.default_radio${devidx}2.bss_load_update_period=0
				set wireless.default_radio${devidx}2.map_bss_type=32
				set wireless.default_radio${devidx}2.ifname=${ifname}-vap1
				set wireless.default_radio${devidx}2.disabled=1

				set wireless.default_radio${devidx}3=wifi-iface
				set wireless.default_radio${devidx}3.device=phy${devidx}
				set wireless.default_radio${devidx}3.network=lan
				set wireless.default_radio${devidx}3.mode=ap
				set wireless.default_radio${devidx}3.ssid=${SSID1}-3
				set wireless.default_radio${devidx}3.encryption=none
				set wireless.default_radio${devidx}3.bss_load_update_period=0
				set wireless.default_radio${devidx}3.map_bss_type=32
				set wireless.default_radio${devidx}3.ifname=${ifname}-vap2
				set wireless.default_radio${devidx}3.disabled=1

				set wireless.default_radio${devidx}4=wifi-iface
				set wireless.default_radio${devidx}4.device=phy${devidx}
				set wireless.default_radio${devidx}4.network=lan
				set wireless.default_radio${devidx}4.mode=ap
				set wireless.default_radio${devidx}4.ssid=${SSID1}-4
				set wireless.default_radio${devidx}4.encryption=none
				set wireless.default_radio${devidx}4.bss_load_update_period=0
				set wireless.default_radio${devidx}4.map_bss_type=32
				set wireless.default_radio${devidx}4.ifname=${ifname}-vap3
				set wireless.default_radio${devidx}4.disabled=1

				set wireless.default_radio${devidx}5=wifi-iface
				set wireless.default_radio${devidx}5.device=phy${devidx}
				set wireless.default_radio${devidx}5.network=lan
				set wireless.default_radio${devidx}5.mode=sta
				set wireless.default_radio${devidx}5.ssid=${SERVERSSID}-5G
				set wireless.default_radio${devidx}5.encryption=none
				set wireless.default_radio${devidx}5.ifname=${ifname}-vxd
				set wireless.default_radio${devidx}5.disabled=1
				EOF
		else
			uci -q batch <<-EOF
				set wireless.phy${devidx}=wifi-device
				set wireless.phy${devidx}.type=mac80211
				${dev_id}
				set wireless.phy${devidx}.channel=${channel}
				set wireless.phy${devidx}.band=${mode_band}
				set wireless.phy${devidx}.htmode=$htmode
				set wireless.phy${devidx}.country=US
				set wireless.phy${devidx}.legacy_rates=1
				set wireless.phy${devidx}.is_2g_256qam=${is_2g_256qam}

				set wireless.default_radio${devidx}0=wifi-iface
				set wireless.default_radio${devidx}0.device=phy${devidx}
				set wireless.default_radio${devidx}0.network=lan
				set wireless.default_radio${devidx}0.mode=ap
				set wireless.default_radio${devidx}0.ssid=${SSID2}
				set wireless.default_radio${devidx}0.encryption=none
				set wireless.default_radio${devidx}0.bss_load_update_period=0
				set wireless.default_radio${devidx}0.ifname=${ifname1}
				set wireless.default_radio${devidx}0.map_bss_type=32

				set wireless.default_radio${devidx}1=wifi-iface
				set wireless.default_radio${devidx}1.device=phy${devidx}
				set wireless.default_radio${devidx}1.network=lan
				set wireless.default_radio${devidx}1.mode=ap
				set wireless.default_radio${devidx}1.ssid=${SSID2}-1
				set wireless.default_radio${devidx}1.encryption=none
				set wireless.default_radio${devidx}1.bss_load_update_period=0
				set wireless.default_radio${devidx}1.ifname=${ifname1}-vap0
				set wireless.default_radio${devidx}1.disabled=1
				set wireless.default_radio${devidx}1.map_bss_type=32

				set wireless.default_radio${devidx}2=wifi-iface
				set wireless.default_radio${devidx}2.device=phy${devidx}
				set wireless.default_radio${devidx}2.network=lan
				set wireless.default_radio${devidx}2.mode=ap
				set wireless.default_radio${devidx}2.ssid=${SSID2}-2
				set wireless.default_radio${devidx}2.encryption=none
				set wireless.default_radio${devidx}2.bss_load_update_period=0
				set wireless.default_radio${devidx}2.ifname=${ifname1}-vap1
				set wireless.default_radio${devidx}2.disabled=1
				set wireless.default_radio${devidx}2.map_bss_type=32

				set wireless.default_radio${devidx}3=wifi-iface
				set wireless.default_radio${devidx}3.device=phy${devidx}
				set wireless.default_radio${devidx}3.network=lan
				set wireless.default_radio${devidx}3.mode=ap
				set wireless.default_radio${devidx}3.ssid=${SSID2}-3
				set wireless.default_radio${devidx}3.encryption=none
				set wireless.default_radio${devidx}3.bss_load_update_period=0
				set wireless.default_radio${devidx}3.ifname=${ifname1}-vap2
				set wireless.default_radio${devidx}3.disabled=1
				set wireless.default_radio${devidx}3.map_bss_type=32

				set wireless.default_radio${devidx}4=wifi-iface
				set wireless.default_radio${devidx}4.device=phy${devidx}
				set wireless.default_radio${devidx}4.network=lan
				set wireless.default_radio${devidx}4.mode=ap
				set wireless.default_radio${devidx}4.ssid=${SSID2}-4
				set wireless.default_radio${devidx}4.encryption=none
				set wireless.default_radio${devidx}4.bss_load_update_period=0
				set wireless.default_radio${devidx}4.ifname=${ifname1}-vap3
				set wireless.default_radio${devidx}4.disabled=1
				set wireless.default_radio${devidx}4.map_bss_type=32

				set wireless.default_radio${devidx}5=wifi-iface
				set wireless.default_radio${devidx}5.device=phy${devidx}
				set wireless.default_radio${devidx}5.network=lan
				set wireless.default_radio${devidx}5.mode=sta
				set wireless.default_radio${devidx}5.ssid=${SERVERSSID}-2.4G
				set wireless.default_radio${devidx}5.encryption=none
				set wireless.default_radio${devidx}5.ifname=${ifname1}-vxd
				set wireless.default_radio${devidx}5.disabled=1
				EOF
		fi
	
		uci -q commit wireless

		devidx=$(($devidx + 1))
	done
}
