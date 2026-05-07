MIB_CMD_SLEEP=6
BACKHAUL_BSS=64
BACKHAUL_STA=128

rtk_wlan_phy_cal() {
	local ifname="$1"
	
	if [ ! -d "/sys/class/net/${ifname}" ]; then
		return
	fi
	
	wlan_cal $ifname
}

rtk_detect_mac80211() {
	local phy="$1"

	if [ "$mode_band" = "5g" ]; then
		if [ "$(iw phy $phy info | grep -c HE160)" -gt 0 ]; then
			htmode="HE160"
		fi
	fi
	
	if [ "$mode_band" = "2g" ]; then
		if [ "$(iw phy $phy info | grep -c 'VHT Capabilities')" -gt 0 ]; then
			is_2g_256qam="1"
		fi
	fi
}

rtk_mac80211_init_device_config() {
	config_add_boolean amsdu
	config_add_boolean txbf txbf_mu
	config_add_int ofdma
	config_add_boolean is_2g_256qam
	config_add_int power_percent
}

rtk_mac80211_init_iface_config() {
	config_add_int map_bss_type
	config_add_int map_vlan_id
}

rtk_hostapd_common_add_device_config() {
	echo "TODO"
}

rtk_hostapd_common_add_bss_config() {
	echo "TODO"
}

rtk_hostapd_set_wpa_pairwise() {

	case "$auth_type" in
		eap192)
			wpa_pairwise=GCMP-256
		;;
	esac
}

rtk_hostapd_set_group_mgmt_cipher() {

	case "$auth_type" in
		eap192)
			ieee80211w_mgmt_cipher=BIP-GMAC-256
		;;
	esac
}

rtk_hostapd_append_wpa_key_mgmt() {
	local ifname="$1"

	case "$auth_type" in
		psk|eap)
			[ "${ieee80211w:-0}" -eq 2 ] && wpa_key_mgmt="WPA-${auth_type_l}-SHA256"
		;;
	esac

	local akm=`cat "/tmp/dpp_akm_${ifname}"`
	if [[ "$akm" == *"dpp"* ]]; then
		append wpa_key_mgmt "DPP"
	fi

}

rtk_hostapd_set_default() {
	set_default bss_load_update_period 0
	set_default is_2g_256qam 0
}

rtk_brsnoop_isolation_set_default() {
	set_default multicast_to_unicast 0
}

rtk_mac80211_hostapd_setup_base() {
	echo "$auto_channel, $htmode, $country" >> /var/log/debug.log
	[ "$auto_channel" -gt 0 ] && [ "$htmode" = "HE160" ] && [ "$country" = "US" ] && \
		channel_list="36-48"
		
	[ "$is_2g_256qam" -gt 0 ] && {
		append base_cfg "ieee80211ac=1" "$N"
	}
	
	[ "$band" = "5g" ] && [ "$auto_channel" = "0" ] && hostapd_noscan=1
}

rtk_mac80211_hostapd_remove_start_disabled() {

	[ "$start_disabled" -eq 0 ] &&  sed -i '/start_disabled/d' /var/run/hostapd-$phy.conf
}

rtk_mac80211_chan_width_check() {

	[ "$vht_oper_chwidth" -ne 2 ] && vht160=0
}

rtk_mac80211_set_iface_config() {
	local ifname inactive_timeout
	json_select config
	json_get_vars ifname inactive_timeout
	json_select ..
	if [ -z "$ifname" ]; then return; fi
	if [ -z "$inactive_timeout" ]; then
		inactive_timeout=$(iwpriv $ifname get_mib inactive_timeout | cut -d : -f 2)
	fi
	eval "iwpriv $ifname set_mib inactive_timeout=$inactive_timeout &"
}

rtk_multiap_ap_set_bss_type() {
	local ifname map_bss_type bss_check
	json_select config
	json_get_vars ifname map_bss_type
	json_select ..
	if [ -z "$ifname" ]; then return; fi
	if [ -z "$map_bss_type" ]; then
		eval "sleep $MIB_CMD_SLEEP && iwpriv $ifname set_mib a4_enable=0 &"
		eval "sleep $MIB_CMD_SLEEP && iwpriv $ifname set_mib multiap_bss_type=0 &"
	else
		eval "sleep $MIB_CMD_SLEEP && iwpriv $ifname set_mib multiap_bss_type=$map_bss_type &"
		bss_check=$(( !!($map_bss_type & $BACKHAUL_BSS) ))
		if [[ "$bss_check" = 1 ]]; then
			eval "sleep $MIB_CMD_SLEEP && iwpriv $ifname set_mib a4_enable=1 &"
		else
			eval "sleep $MIB_CMD_SLEEP && iwpriv $ifname set_mib a4_enable=0 &"
		fi
	fi
}

rtk_multiap_sta_set_bss_type() {
	local ifname map_bss_type sta_check
	json_select config
	json_get_vars ifname map_bss_type
	json_select ..
	if [ -z "$ifname" ]; then return; fi
	if [ -z "$map_bss_type" ]; then
		eval "iwpriv $ifname set_mib a4_enable=0"
		eval "iwpriv $ifname set_mib multiap_bss_type=0"
	else
		eval "iwpriv $ifname set_mib multiap_bss_type=$map_bss_type"
		sta_check=$(( !!($map_bss_type & $BACKHAUL_STA) ))
		if [[ "$sta_check" = 1 ]]; then
			eval "iwpriv $ifname set_mib a4_enable=1"
		else
			eval "iwpriv $ifname set_mib a4_enable=0"
		fi
	fi
}

rtk_mac80211_phy_oob_config() {
	local primary_ap=$1
	local ofdma_mib=$(iwpriv $primary_ap get_mib ofdma_enable | cut -d : -f 2)
	local txbf_mib=$(iwpriv $primary_ap get_mib txbf | cut -d : -f 2)
	local txbf_mu_mib=$(iwpriv $primary_ap get_mib txbf_mu | cut -d : -f 2)
	local down_up_wifi=0

	json_select config
	json_get_vars amsdu txbf txbf_mu ofdma power_percent
	json_get_vars band htmode noscan
	json_select ..

	rtk_wlan_phy_cal $primary_ap

	set_default amsdu 1
	set_default txbf 0
	set_default txbf_mu 0
	set_default ofdma 0
	set_default noscan 0
	set_default power_percent 100

	if [ -n "$primary_ap" ]; then
		iwpriv $primary_ap set_mib amsdu=$amsdu

		if [ -n "$power_percent" ]; then
			iwpriv $primary_ap set_mib powerpercent=$power_percent
		fi

		if [ "$band" = "2g" ]; then
			if [ "$htmode" = "HT40" -o "$htmode" = "HE40" ]; then
				if [ "$noscan" -eq 0 ]; then
					iwpriv $primary_ap set_mib coexist=1
				else
					iwpriv $primary_ap set_mib coexist=0
				fi
			else
				iwpriv $primary_ap set_mib coexist=0
			fi
		fi
		
		if [ "$txbf_mib" -ne "$txbf" ]; then
			iwpriv $primary_ap set_mib txbf=$txbf
			down_up_wifi=1
		fi

		if [ "$txbf_mu_mib" -ne "$txbf_mu" ]; then
			iwpriv $primary_ap set_mib txbf_auto_snd=$txbf_mu
			iwpriv $primary_ap set_mib txbf_mu=$txbf_mu
			down_up_wifi=1
		fi

		if [ "$ofdma_mib" -ne "$ofdma" ]; then
			iwpriv $primary_ap set_mib ofdma_enable=$ofdma
			down_up_wifi=1
		fi

		[ $down_up_wifi = 1 ] && eval "sleep $MIB_CMD_SLEEP && wpa_cli -g /var/run/hostapd/global raw REMOVE $primary_ap &"
	fi
}

rtk_mac80211_interface_oob_config() {
	local primary_ap=$1

	rtk_mac80211_set_iface_config
	rtk_multiap_ap_set_bss_type
}

rtk_hostapd_wps_led_control_setup() {
	local phy="$1"
	local ifname
	local wps_pushbutton

	json_select data
	json_get_vars ifname
	json_select ..

	json_select config
	json_get_vars wps_pushbutton
	json_select ..

	set_default wps_pushbutton 0
	if [ "$wps_pushbutton" -eq 0 ]; then
		logger -p user.info -t "netifd.wireless.mac80211" "Skip for $ifname wps led control."
		return
	fi

	if [ -f /etc/hostapd_wps_led.sh ]; then
		/usr/sbin/hostapd_cli -a /etc/hostapd_wps_led.sh -B \
			-P /var/run/hostapd_cli.pid.$ifname.wps -i $ifname
		wireless_add_process "$(cat /var/run/hostapd_cli.pid.$ifname.wps)" \
			"/usr/sbin/hostapd_cli" 1
	fi
}

rtk_wpas_wps_led_control_setup() {
	local phy="$1"
	local ifname
	local wps_pushbutton

	json_select data
	json_get_vars ifname
	json_select ..

	if [ -f /etc/wpa_supplicant_wps_led.sh ]; then
		/usr/sbin/wpa_cli -a /etc/wpa_supplicant_wps_led.sh -B \
			-P /var/run/wpa_cli.pid.$ifname.wps -i $ifname
		wireless_add_process "$(cat /var/run/wpa_cli.pid.$ifname.wps)" \
			"/usr/sbin/wpa_cli" 1
	fi
}

rtk_wps_led_control_teardown() {
	local phy

	json_select data
	json_get_vars phy
	json_select ..

	[ -f /etc/hostapd_wps_led.sh ] && /etc/hostapd_wps_led.sh wlan${phy#phy} SYSTEM-WPS-STOP
	[ -f /etc/wpa_supplicant_wps_led.sh ] && /etc/wpa_supplicant_wps_led.sh wlan${phy#phy} SYSTEM-WPS-STOP
}

rtk_hostapd_event_action_setup() {
	json_select config
	json_get_vars ifname disabled
	json_select ..

	[ "$disabled" = "1" ] && return 

	if [ -f /etc/hostapd_general_event.sh ]; then
		/usr/sbin/hostapd_cli -a /etc/hostapd_general_event.sh -B \
			-P /var/run/hostapd_cli.pid.$ifname.general -i $ifname
		wireless_add_process "$(cat /var/run/hostapd_cli.pid.$ifname.general)" \
			"/usr/sbin/hostapd_cli" 1
	fi
}

rtk_wpa_event_action_setup() {
	json_select config
	json_get_vars ifname disabled
	json_select ..

	[ "$disabled" = "1" ] && return

	if [ -f /etc/wpa_supplicant_general_event.sh ]; then
		/usr/sbin/wpa_cli -a /etc/wpa_supplicant_general_event.sh -B \
			-P /var/run/wpa_cli.pid.$ifname.general -i $ifname
		wireless_add_process "$(cat /var/run/wpa_cli.pid.$ifname.general)" \
			"/usr/sbin/wpa_cli" 1
	fi
}

rtk_mac80211_fc_add_iface() {
	local phy="$1"
	local ifname="$2"
	local band="$3"
	local fc_band
	
	if [ -f /proc/fc/sw_dump/wlan_devmap ]; then
		if [ "$band" = "5g" ]; then
			fc_band="0"
		else 
			fc_band="1"
		fi
		
		echo "add dev $ifname band $fc_band" > /proc/fc/sw_dump/wlan_devmap
	fi
}

rtk_mac80211_fc_del_iface() {
	local ifname="$1"
	
	if [ -f /proc/fc/sw_dump/wlan_devmap ]; then
		echo "del dev $ifname" > /proc/fc/sw_dump/wlan_devmap
	fi
}

rtk_mac80211_fc_del_alliface() {
	local phy="$1"
	local vaps="$2"
	
	if [ -d "/sys/class/ieee80211/${phy}/device/net" ]; then
		vaps=`ls /sys/class/ieee80211/${phy}/device/net`
	fi
	
	for i in $vaps; do
		rtk_mac80211_fc_del_iface "$i"
	done
}

rtk_mac80211_fc_update_wlan_wanportmask() {
	local ifname="$1"
	local enable="$2"
	local is_lan="$3"
	local is_wwan="$4"

	echo "rtk_mac80211_fc_update_wlan_wanportmask ifname=$ifname enable=$enable is_lan=$is_lan is_wwan=$is_wwan" > /dev/console

	if [ -f /proc/fc/ctrl/wlan_wan_port_mask ]; then
		if [ "$enable" = 0 ]; then
			echo "0 $ifname" > /proc/fc/sw_dump/wlan_wan_port_mask
		else
			if [ "$is_wwan" -gt 0 ]; then
				if [ "$is_lan" -gt 0 ]; then
					echo "0 $ifname" > /proc/fc/ctrl/wlan_wan_port_mask
				else
					echo "1 $ifname" > /proc/fc/ctrl/wlan_wan_port_mask
				fi
			else
				if [ "$is_lan" -gt 0 ]; then
					echo "0 $ifname" > /proc/fc/ctrl/wlan_wan_port_mask
				fi
			fi
		fi
	fi
}