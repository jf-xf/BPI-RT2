#!/bin/sh

[ -e /lib/functions.sh ] && . /lib/functions.sh || . ./functions.sh
[ -e /lib/functions.sh ] && . /lib/functions/network.sh

#RTK Ebtables Leagcy
IP=`which ip`
IPTABLES=`which iptables`
IP6TABLES=`which ip6tables`
EBTABLES=`which ebtables`

lan_data=$(ubus call network.interface.lan status)
#lan_l3dev=$(jsonfilter -s "$lan_data" -e '@["l3_device"]')
network_get_device lan_l3dev lan
BRIDGE_NAME=$lan_l3dev

FW3_PMAP_MAIN_SCRIPT="/usr/lib/firewall/fw3_custome_include_pmap"

FW3_PMAP_SCRIPT_DIR="/var/firewall"
FW3_PMAP_SETUP_SCRIPT="fw3_pmap_setup"
FW3_PMAP_CLEAR_SCRIPT="fw3_pmap_clear"

FW3_PMAP_EB_BROUTE_CHAIN="portmapping_broute"
FW3_PMAP_EB_INPUT_CHAIN="portmapping_input"
FW3_PMAP_EB_FWD_CHAIN="portmapping"
FW3_PMAP_EB_OUPUT_CHAIN="portmapping_output"
FW3_PMAP_EB_SERVICE_CHAIN="portmapping_service"
FW3_PMAP_EB_BLOCK_RA_CHAIN="ipv6_ra_local_out"
FW3_PMAP_IP_MANGLE_FORWARD_CHAIN="portmapping_rt_tbl"

# Realtek socket mark parameter define here
rtk_script_env_file="/etc/profile.d/rtk_script_env.sh"
if [ -e $rtk_script_env_file ]; then
	source $rtk_script_env_file
else
	echo "no $rtk_script_env_file"
	return 1
fi

file_reset() {
	[ -f $1 ] && : > $1
}

xappend() {
	local file=$1
	shift

	echo $@ >> $file
}

pmap_reset() {
	file_reset $FW3_PMAP_SCRIPT_DIR/$FW3_PMAP_CLEAR_SCRIPT
	file_reset $FW3_PMAP_SCRIPT_DIR/$FW3_PMAP_SETUP_SCRIPT
}

pmap_setup_xappend() {
	xappend $FW3_PMAP_SCRIPT_DIR/$FW3_PMAP_SETUP_SCRIPT $@
}

pmap_clear_xappend() {
	xappend $FW3_PMAP_SCRIPT_DIR/$FW3_PMAP_CLEAR_SCRIPT $@
}

setup_pmap_ip_rule(){
	local tableID=$(( $PMAP_POLICY_ROUTE_TBL_START + $WAN_IDX ))
	#echo $section
	#echo $rootdev
	#echo $l2dev
	#echo $l3dev
	#echo $rtk_ipmode
	#echo $rtk_chmode
	#echo $wanMark
	#echo $markMask

	if [ $rtk_chmode == "Bridged" ]; then
		# ip -4 rule del fwmark 0x22000/0x7e000 prohibit
		pmap_clear_xappend $IP -4 rule del fwmark $wanMark/$markMask prohibit

		# ip -4 rule add fwmark 0x22000/0x7e000 pref 30000 prohibit
		pmap_setup_xappend $IP -4 rule add fwmark $wanMark/$markMask pref $RULE_PRI_SKB_MARK_PROHIBIT prohibit

		# ip -6 rule del fwmark 0x22000/0x7e000 prohibit
		pmap_clear_xappend $IP -6 rule del fwmark $wanMark/$markMask prohibit

		# ip -6 rule add fwmark 0x22000/0x7e000 pref 30000 prohibit
		pmap_setup_xappend $IP -6 rule add fwmark $wanMark/$markMask pref $RULE_PRI_SKB_MARK_PROHIBIT prohibit
	else
		#iptables -t mangle -A portmapping_rt_tbl -i veip0 -j MARK --set-mark 0x22000/0x7e000
		pmap_setup_xappend $IPTABLES -t mangle -A $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN -i $l2dev -j MARK --set-mark $wanMark/$markMask
		#ip6tables -t mangle -A portmapping_rt_tbl -i veip0 -j MARK --set-mark 0x22000/0x7e000
		pmap_setup_xappend $IP6TABLES -t mangle -A $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN -i $l2dev -j MARK --set-mark $wanMark/$markMask
		if [ $rtk_chmode == 'PPPoE' ]; then
			local l3dev="ppp$section"
			#iptables -t mangle -A portmapping_rt_tbl -i pppveip0_0 -j MARK --set-mark 0x22000/0x7e000
			pmap_setup_xappend $IPTABLES -t mangle -A $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN -i $l3dev -j MARK --set-mark $wanMark/$markMask
			#ip6tables -t mangle -A portmapping_rt_tbl -i pppveip0_0 -j MARK --set-mark 0x22000/0x7e000
			pmap_setup_xappend $IP6TABLES -t mangle -A $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN -i $l3dev -j MARK --set-mark $wanMark/$markMask
		fi

		for intf in $interface; do
			wan_data=$(ubus call network.interface."$intf" status)
			up=$(jsonfilter -s "$wan_data" -e '@["up"]')
			if [ $up == "true" ]; then
				if [ $intf == $section"_v4" ]; then
					network_get_ipaddr ipaddr $intf
					mask=$(jsonfilter -s "$wan_data" -e '@["ipv4-address"][0].mask')

					# set policy rule
					if [ $rtk_chmode != "Bridged" ]; then
						# ip -4 rule del from 192.168.88.210 table 33
						[ -n $ipaddr ] && pmap_clear_xappend $IP -4 rule del from $ipaddr table $tableID

						# ip -4 rule del fwmark 0x22000/0x7e000 table 33
						pmap_clear_xappend $IP -4 rule del fwmark $wanMark/$markMask table $tableID

						# ip -4 route flush table 33
						pmap_clear_xappend $IP -4 route flush table $tableID

						# ip -4 rule add from 192.168.88.210 pref 20000 table 33
						[ -n $ipaddr ] && pmap_setup_xappend $IP -4 rule add from $ipaddr pref $RULE_PRI_SRC_RT table $tableID

						# ip -4 rule add fwmark 0x22000/0x7e000 pref 20100 table 33
						pmap_setup_xappend $IP -4 rule add fwmark $wanMark/$markMask pref $RULE_PRI_SKB_MARK_RT table $tableID

						if [ $rtk_chmode == "IPoE" ]; then
							subnet=$(ipcalc.sh $ipaddr $mask | awk -F '=' '/NETWORK/{print $2}')
							# ip -4 route add 192.168.88.0/24 dev veip0 table 33
							[ -n $subnet ] && [ -n $mask ] && pmap_setup_xappend $IP -4 route add $subnet/$mask dev $l3dev table $tableID

							network_get_gateway nexthop $intf true	# true is for inaction case
							# ip -4 route add default via 192.168.88.1 dev veip0 table 33
							[ -n $nexthop ] && pmap_setup_xappend $IP -4 route add default via $nexthop dev $l3dev table $tableID
						else
							if [ $rtk_chmode == "PPPoE" ]; then
								# ip -4 route add default dev pppveip0_0 table 33
								pmap_setup_xappend $IP -4 route add default dev $l3dev table $tableID
							fi
						fi

						network_get_ipaddr lan_ipaddr lan
						#lan_addr=$(jsonfilter -s "$lan_data" -e '@["ipv4-address"][0].address')
						lan_mask=$(jsonfilter -s "$lan_data" -e '@["ipv4-address"][0].mask')
						lan_subnet=$(ipcalc.sh $lan_ipaddr $lan_mask | awk -F '=' '/NETWORK/{print $2}')
						# ip -4 route add 192.168.1.0/24 dev br-lan table 33
						[ -n $lan_subnet ] && [ -n $lan_mask ] && pmap_setup_xappend $IP -4 route add $lan_subnet/$lan_mask dev $BRIDGE_NAME table $tableID
					fi

					# ip -4 rule del fwmark 0x22000/0x7e000 prohibit
					pmap_clear_xappend $IP -4 rule del fwmark $wanMark/$markMask prohibit

					# ip -4 rule add fwmark 0x22000/0x7e000 pref 30000 prohibit
					pmap_setup_xappend $IP -4 rule add fwmark $wanMark/$markMask pref $RULE_PRI_SKB_MARK_PROHIBIT prohibit
				fi

				if [ $intf == $section"_v6" ]; then
					# set policy rule
					if [ $rtk_chmode != "Bridged" ]; then
						network_get_ipaddr6 ipaddr6 $intf

						# ip -6 rule del oif veip0 pref 20000 table 33
						pmap_clear_xappend $IP -6 rule del oif $l3dev pref $RULE_PRI_SRC_RT table $tableID

						# ip -6 rule del from 2001:db8:88:ff00:2e0:4cff:fe18:2631 table 33
						[ -n $ipaddr6 ] && pmap_clear_xappend $IP -6 rule del from $ipaddr6 table $tableID

						# ip -6 rule del fwmark 0x22000/0x7e000 table 33
						pmap_clear_xappend $IP -6 rule del fwmark $wanMark/$markMask table $tableID

						# ip -6 route flush table 33
						pmap_clear_xappend $IP -6 route flush table $tableID

						# ip -6 rule add oif veip0 pref 20000 table 33
						# For socket bind case of applications, Ex: dhcpv6/radvd port binding
						pmap_setup_xappend $IP -6 rule add oif $l3dev pref $RULE_PRI_SRC_RT table $tableID

						# ip -6 rule add from 2001:db8:88:ff00:2e0:4cff:fe18:2631 pref 20000 table 33
						[ -n $ipaddr6 ] && pmap_setup_xappend $IP -6 rule add from $ipaddr6 pref $RULE_PRI_SRC_RT table $tableID

						# ip -6 rule add fwmark 0x22000/0x7e000 pref 20100 table 33
						pmap_setup_xappend $IP -6 rule add fwmark $wanMark/$markMask pref $RULE_PRI_SKB_MARK_RT table $tableID

						network_get_subnet6 rt_subnet $intf
						# ip -6 route add 2001:db8:88:ff00:2e0:4cff:fe86:7001/64 dev veip0 table 33
						[ -n $rt_subnet ] && pmap_setup_xappend $IP -6 route add $rt_subnet dev $l3dev table $tableID

						# add active pd route
						lan_prefix=$(jsonfilter -s "$wan_data" -e '@["ipv6-prefix"][0]["assigned"]["lan"].address')
						pd_mask=$(jsonfilter -s "$wan_data" -e '@["ipv6-prefix"][0]["assigned"]["lan"].mask')
						# ip -6 route add 2001:db8:88:ff02::/64 dev br-lan table 33
						[ -n $lan_prefix ] && [ $pd_mask ] && pmap_setup_xappend $IP -6 route add $lan_prefix/$pd_mask dev $BRIDGE_NAME table $tableID

						# ip -6 route append fe80::/64 dev veip0 table 33
						# For default route nexthop(fe80::/64)
						pmap_setup_xappend $IP -6 route append fe80::/64 dev $l3dev table $tableID

						# ip -6 route append fe80::/64 dev br-lan table 33
						# For binding port ping non-binding port
						pmap_setup_xappend $IP -6 route append fe80::/64 dev $BRIDGE_NAME table $tableID

						# add default policy route rule
						# IPv6 default need add gateway even for PPP interface
						network_get_gateway6 nexthop $intf true	# true is for inaction case
						# ip -6 route add default via fe80::c6ad:34ff:fec6:11dc dev veip0 table 33
						[ -n $nexthop ] && pmap_setup_xappend $IP -6 route add default via $nexthop dev $l3dev table $tableID
					fi

					# ip -6 rule del fwmark 0x22000/0x7e000 pref 30000 prohibit
					pmap_clear_xappend $IP -6 rule del fwmark $wanMark/$markMask prohibit

					# ip -6 rule add fwmark 0x22000/0x7e000 pref 30000 prohibit
					pmap_setup_xappend $IP -6 rule add fwmark $wanMark/$markMask pref $RULE_PRI_SKB_MARK_PROHIBIT prohibit
				fi
			fi
		done
	fi
}

setup_pmap_policy_rules(){
	# ignore non-wan device
	[ ${section:0:5} != "veip0" ] && return
	WAN_IDX=$(($(echo $section | awk -F"_" '{print $2}')+1))

	local wanMark=$(( $WAN_IDX << $SOCK_MARK_WAN_INDEX_START))
	local markMask=$(( ((1<<($SOCK_MARK_WAN_INDEX_END-$SOCK_MARK_WAN_INDEX_START+1))-1) << $SOCK_MARK_WAN_INDEX_START))

#	FW3_PMAP_EB_INTF_INPUT_CHAIN="portmapping_"$section"_in"
#	FW3_PMAP_EB_INTF_OUPUT_CHAIN="portmapping_"$section"_out"
#
#	# clear ebtable pmap port input/output chain
#	pmap_clear_xappend $EBTABLES -F $FW3_PMAP_EB_INTF_INPUT_CHAIN
#	pmap_clear_xappend $EBTABLES -X $FW3_PMAP_EB_INTF_INPUT_CHAIN
#	pmap_clear_xappend $EBTABLES -F $FW3_PMAP_EB_INTF_OUPUT_CHAIN
#	pmap_clear_xappend $EBTABLES -X $FW3_PMAP_EB_INTF_OUPUT_CHAIN

	config_get ports $1"_pmap" ports
	[ -z $ports ] && return
	config_get rootdev "$1" ifname
	config_get l2dev "$1" name
	config_get rtk_ipmode "$1" rtk_ipmode
	config_get rtk_chmode "$1" rtk_chmode
	config_get interface "$1" interface
	l3dev=$l2dev
	if [ $rtk_chmode == 'PPPoE' ]; then
		l3dev="ppp$section"
	fi

#	# add ebtable pmap port input/output chain
#	pmap_setup_xappend $EBTABLES -N $FW3_PMAP_EB_INTF_INPUT_CHAIN
#	pmap_setup_xappend $EBTABLES -P $FW3_PMAP_EB_INTF_INPUT_CHAIN DROP
#	pmap_setup_xappend $EBTABLES -N $FW3_PMAP_EB_INTF_OUPUT_CHAIN
#	pmap_setup_xappend $EBTABLES -P $FW3_PMAP_EB_INTF_OUPUT_CHAIN DROP
#
#	if [ $rtk_chmode == "Bridged" ]; then
#		#ebtables -A portmapping_nas0_1_in -o veip0 -j RETURN
#		pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_INTF_INPUT_CHAIN -o $l2dev -j RETURN
#
#		#ebtables -A portmapping_nas0_1_out -i veip0 -j RETURN
#		pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_INTF_OUPUT_CHAIN -i $l2dev -j RETURN
#	fi

	# set policy route rule -> replace by rule/rule6/route/route6 of uci
	#setup_pmap_ip_rule

	for port in $ports; do
		#ebtables -t broute -A portmapping_broute -i eth0.2 -j mark --mark-set 0x22000 --mark-target CONTINUE
		pmap_setup_xappend $EBTABLES -t broute -A $FW3_PMAP_EB_BROUTE_CHAIN -i $port -j mark --mark-set $wanMark --mark-target CONTINUE

#		#ebtables -A portmapping_nas0_1_out -i eth0.2 -j RETURN
#		pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_INTF_OUPUT_CHAIN -i $port -j RETURN
#
#		if [ $rtk_chmode == "Bridged" ]; then
#			#ebtables -A portmapping_nas0_1_in -o eth0.2 -j RETURN
#			pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_INTF_INPUT_CHAIN -o $port -j RETURN
#
#			#ebtables -A portmapping_output -o eth0.2 -j portmapping_service
#			pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_OUPUT_CHAIN -o $port -j $FW3_PMAP_EB_SERVICE_CHAIN
#		fi
#
#		#ebtables -A portmapping_output -o eth0.2 -j portmapping_nas0_1_out
#		pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_OUPUT_CHAIN -o $port -j $FW3_PMAP_EB_INTF_OUPUT_CHAIN
#
#		if [ $rtk_chmode != "Bridged" ]; then
#			#ebtables -A ipv6_ra_local_out -p IPv6 --logical-out br0 -o eth0.2 --ip6-proto ipv6-icmp --ip6-icmp-type router-advertisement -j DROP
#			pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_BLOCK_RA_CHAIN -p IPv6 --logical-out $BRIDGE_NAME -o $port --ip6-proto ipv6-icmp --ip6-icmp-type router-advertisement -j DROP
#		fi
	done

	if [ $rtk_chmode == "Bridged" ]; then
		#ebtables -A portmapping_broute -i veip0 -j mark --mark-or 0x22000 --mark-target CONTINUE
		pmap_setup_xappend $EBTABLES -t broute -A $FW3_PMAP_EB_BROUTE_CHAIN -i $l2dev -j mark --mark-or $wanMark --mark-target CONTINUE

#		#ebtables -A portmapping_nas0_1_in -o ! veip0+ -j RETURN
#		pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_INTF_INPUT_CHAIN -o ! $rootdev+ -j RETURN
#
#		#ebtables -A portmapping_output -o veip0 -j portmapping_service
#		pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_OUPUT_CHAIN -o $l2dev -j $FW3_PMAP_EB_SERVICE_CHAIN
#
#		#ebtables -A portmapping -o veip0 -j portmapping_nas0_1_in
#		pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_FWD_CHAIN -o $l2dev -j $FW3_PMAP_EB_INTF_INPUT_CHAIN
#
#		#ebtables -A portmapping -o veip0 -j portmapping_nas0_1_out
#		pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_FWD_CHAIN -o $l2dev -j $FW3_PMAP_EB_INTF_OUPUT_CHAIN
	fi

#	#ebtables -A portmapping_nas0_1_out -i ! nas0+ --mark 0x0/0x7e000 -j RETURN
#	pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_INTF_OUPUT_CHAIN -i ! $rootdev+ --mark 0x0/$markMask -j RETURN
#	#ebtables -A portmapping_nas0_1_out --logical-in ! br+ --mark 0x0/0x7e000 -j RETURN
#	pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_INTF_OUPUT_CHAIN --logical-in ! $BRIDGE_NAME --mark 0x0/$markMask -j RETURN
#	#ebtables -A portmapping_nas0_1_out --mark 0x22000/0x7e000 -j RETURN
#	pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_INTF_OUPUT_CHAIN --mark $wanMark/$markMask -j RETURN
}

[ -d $FW3_PMAP_SCRIPT_DIR ] || mkdir -p $FW3_PMAP_SCRIPT_DIR
# clear original rule before generate new script.
[ -x $FW3_PMAP_MAIN_SCRIPT ] && sh $FW3_PMAP_MAIN_SCRIPT clear

config_load network
pmap_reset
pmap_setup_xappend "#!/bin/sh"
pmap_clear_xappend "#!/bin/sh"

# clear original rule before rule setup script
pmap_setup_xappend "[ -x $FW3_PMAP_MAIN_SCRIPT ] && sh $FW3_PMAP_MAIN_SCRIPT clear"

# clear ebtable pmap broute chain
pmap_clear_xappend $EBTABLES -t broute -D BROUTING -j $FW3_PMAP_EB_BROUTE_CHAIN
pmap_clear_xappend $EBTABLES -t broute -F $FW3_PMAP_EB_BROUTE_CHAIN
pmap_clear_xappend $EBTABLES -t broute -X $FW3_PMAP_EB_BROUTE_CHAIN
# add ebtable pmap broute chain
pmap_setup_xappend $EBTABLES -t broute -N $FW3_PMAP_EB_BROUTE_CHAIN
pmap_setup_xappend $EBTABLES -t broute -P $FW3_PMAP_EB_BROUTE_CHAIN RETURN
pmap_setup_xappend $EBTABLES -t broute -A BROUTING -j $FW3_PMAP_EB_BROUTE_CHAIN

## clear ebtable pmap input chain
#pmap_clear_xappend $EBTABLES -D INPUT -j $FW3_PMAP_EB_INPUT_CHAIN
#pmap_clear_xappend $EBTABLES -F $FW3_PMAP_EB_INPUT_CHAIN
#pmap_clear_xappend $EBTABLES -X $FW3_PMAP_EB_INPUT_CHAIN
## add ebtable pmap input chain
#pmap_setup_xappend $EBTABLES -N $FW3_PMAP_EB_INPUT_CHAIN
#pmap_setup_xappend $EBTABLES -P $FW3_PMAP_EB_INPUT_CHAIN RETURN
#pmap_setup_xappend $EBTABLES -A INPUT -j $FW3_PMAP_EB_INPUT_CHAIN
#
## clear ebtable pmap output chain
#pmap_clear_xappend $EBTABLES -D OUTPUT -j $FW3_PMAP_EB_OUPUT_CHAIN
#pmap_clear_xappend $EBTABLES -F $FW3_PMAP_EB_OUPUT_CHAIN
#pmap_clear_xappend $EBTABLES -X $FW3_PMAP_EB_OUPUT_CHAIN
## add ebtable pmap output chain
#pmap_setup_xappend $EBTABLES -N $FW3_PMAP_EB_OUPUT_CHAIN
#pmap_setup_xappend $EBTABLES -P $FW3_PMAP_EB_OUPUT_CHAIN RETURN
#pmap_setup_xappend $EBTABLES -A OUTPUT -j $FW3_PMAP_EB_OUPUT_CHAIN
#
## clear ebtable pmap block ra local output chain
#pmap_clear_xappend $EBTABLES -D OUTPUT -j $FW3_PMAP_EB_BLOCK_RA_CHAIN
#pmap_clear_xappend $EBTABLES -F $FW3_PMAP_EB_BLOCK_RA_CHAIN
#pmap_clear_xappend $EBTABLES -X $FW3_PMAP_EB_BLOCK_RA_CHAIN
## add ebtable pmap block ra local output chain
#pmap_setup_xappend $EBTABLES -N $FW3_PMAP_EB_BLOCK_RA_CHAIN
#pmap_setup_xappend $EBTABLES -P $FW3_PMAP_EB_BLOCK_RA_CHAIN RETURN
#pmap_setup_xappend $EBTABLES -A OUTPUT -j $FW3_PMAP_EB_BLOCK_RA_CHAIN
#
## clear ebtable pmap server chain
#pmap_clear_xappend $EBTABLES -F $FW3_PMAP_EB_SERVICE_CHAIN
#pmap_clear_xappend $EBTABLES -X $FW3_PMAP_EB_SERVICE_CHAIN
## add ebtable pmap server chain
#pmap_setup_xappend $EBTABLES -N $FW3_PMAP_EB_SERVICE_CHAIN
#pmap_setup_xappend $EBTABLES -P $FW3_PMAP_EB_SERVICE_CHAIN RETURN
#pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_SERVICE_CHAIN -p ipv4 --ip-proto udp --ip-dport 67:68 -j DROP
#pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_SERVICE_CHAIN -p ipv4 --ip-proto 2 -j DROP
#pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_SERVICE_CHAIN -d multicast -p ipv4 --ip-proto ! icmp -j DROP
#pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_SERVICE_CHAIN -p ipv6 --ip6-proto udp --ip6-dport 546:547 -j DROP
#pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_SERVICE_CHAIN -p ipv6 --ip6-proto ipv6-icmp --ip6-icmp-type 130:134 -j DROP
#pmap_setup_xappend $EBTABLES -A $FW3_PMAP_EB_SERVICE_CHAIN -d multicast -p ipv6 --ip6-proto ! ipv6-icmp -j DROP
#
## clear ebtable pmap forward chain
#pmap_clear_xappend $EBTABLES -D FORWARD -j $FW3_PMAP_EB_FWD_CHAIN
#pmap_clear_xappend $EBTABLES -F $FW3_PMAP_EB_FWD_CHAIN
#pmap_clear_xappend $EBTABLES -X $FW3_PMAP_EB_FWD_CHAIN
## add ebtable pmap forward chain
#pmap_setup_xappend $EBTABLES -N $FW3_PMAP_EB_FWD_CHAIN
#pmap_setup_xappend $EBTABLES -P $FW3_PMAP_EB_FWD_CHAIN RETURN
#pmap_setup_xappend $EBTABLES -A FORWARD -j $FW3_PMAP_EB_FWD_CHAIN
#
## clear iptables mangle pmap chain
#pmap_clear_xappend $IPTABLES -t mangle -D FORWARD -j $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN
#pmap_clear_xappend $IPTABLES -t mangle -F $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN
#pmap_clear_xappend $IPTABLES -t mangle -X $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN
## add iptables mangle pmap chain
#pmap_setup_xappend $IPTABLES -t mangle -N $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN
#pmap_setup_xappend $IPTABLES -t mangle -A FORWARD -j $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN
#
## clear ip6tables mangle pmap chain
#pmap_clear_xappend $IP6TABLES -t mangle -D PREROUTING -j $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN
#pmap_clear_xappend $IP6TABLES -t mangle -F $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN
#pmap_clear_xappend $IP6TABLES -t mangle -X $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN
## add ip6tables mangle pmap chain
#pmap_setup_xappend $IP6TABLES -t mangle -N $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN
#pmap_setup_xappend $IP6TABLES -t mangle -A PREROUTING -j $FW3_PMAP_IP_MANGLE_FORWARD_CHAIN

# gothrough all device type section
config_foreach setup_pmap_policy_rules device

chmod +x $FW3_PMAP_SCRIPT_DIR/$FW3_PMAP_SETUP_SCRIPT
chmod +x $FW3_PMAP_SCRIPT_DIR/$FW3_PMAP_CLEAR_SCRIPT
