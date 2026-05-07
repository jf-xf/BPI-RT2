#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
	echo init_proto "$@"
}

proto_static_setup()
{
	# echo "===> proto_static_setup $@" > /dev/console
	local config="$1"; shift
	local enable_dhcpc dhcp6c_req_opt
	local gateway ip6gw ip6table ip4table ip6table2 ip4table2 peerdns iface_dslite zone_dslite peeraddr upstream
	
	json_get_vars gateway ip6gw ip6table ip4table ip6table2 ip4table2 peerdns iface_dslite zone_dslite peeraddr upstream
	
	enable_dhcpc=0
	dhcp6c_req_opt=""
	
	if [ "${upstream:0}" -eq 1 ]; then 
		## Static WAN
		[ -z "${ip6gw}" ] || echo "${ip6gw}" > ${RTK_MER_GWINFO_V6}.${ifname}
		[ -z "${gateway}" ] || echo "${gateway}" > ${RTK_MER_GWINFO}.${ifname}
		
		if [ ! -z "${iface_dslite}" ]; then
			if [ ! -z "${peeraddr}" ]; then
				json_init
				json_add_string name "$iface_dslite"
				json_add_string ifname "@$ifname"
				json_add_string proto "dslite"
				json_add_string peeraddr "$peeraddr"
				json_add_string tunlink "$interface"
				json_add_boolean upstream "$upstream"
				[ -z "$zone_dslite" ] || json_add_string zone "$zone_dslite"
				[ -z "$ip6table" ] || json_add_string ip6table "$ip6table"
				[ -z "$ip4table" ] || json_add_string ip4table "$ip4table"
				[ -z "$ip6table2" ] || json_add_string ip6table2 "$ip6table2"
				[ -z "$ip4table2" ] || json_add_string ip4table2 "$ip4table2"
				[ -n "$ENCAPLIMIT_DSLITE" ] && json_add_string encaplimit "$ENCAPLIMIT_DSLITE"
				[ -n "$IFACE_DSLITE_DELEGATE" ] && json_add_boolean delegate "$IFACE_DSLITE_DELEGATE"
				json_close_object
				echo "$(json_dump)" > /tmp/${ifname}_dslite_${iface_dslite}
				ubus call network add_dynamic "$(json_dump)"
			else
				enable_dhcpc=1
				dhcp6c_req_opt=$dhcp6c_req_opt' -r 64'
			fi
		fi
		
		if [ "${peerdns:0}" -eq 1 ]; then
			enable_dhcpc=1
			dhcp6c_req_opt=$dhcp6c_req_opt' -r 23'
		fi
		
		if [ "${enable_dhcpc:0}" -eq 1 -a -x /usr/sbin/odhcp6c ]; then
			proto_export "INTERFACE=$config"
			proto_export "PROTO=static"
			proto_run_command "$config" /usr/sbin/odhcp6c \
				-s /lib/netifd/dhcpv6.script -N none -R $dhcp6c_req_opt \
				-p /var/run/odhcp6c.$ifname.pid -t 120 \
				$ifname
		fi	
	fi
}

proto_static_renew()
{
	# echo "===> proto_static_renew $@" > /dev/console
	local config="$1"; shift
}

proto_static_teardown()
{
	# echo "===> proto_static_teardown $@" > /dev/console
	local config="$1"; shift
	local ip6table ip4table ip6table2 ip4table2 peerdns iface_dslite zone_dslite peeraddr upstream
	json_get_vars ip6table ip4table ip6table2 ip4table2 peerdns iface_dslite zone_dslite peeraddr upstream
	
	if [ "${upstream:0}" -eq 1 ]; then 
		## Static WAN
		if [ ! -z "${iface_dslite}" -a ! -z "${peeraddr}" ]; then
			rm /tmp/${ifname}_dslite_${iface_dslite}
			rm ${RTK_MER_GWINFO_V6}.${ifname}
			rm ${RTK_MER_GWINFO}.${ifname}
			# if [ -f "/var/run/odhcp6c.$ifname.pid" ]; then
			# 	kill -9 `cat /var/run/odhcp6c.$ifname.pid`
			# fi
			# ubus call network.interface.${iface_dslite} remove
		fi
	fi
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol static
}
