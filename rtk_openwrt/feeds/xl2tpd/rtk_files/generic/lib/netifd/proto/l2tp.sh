#!/bin/sh

[ -x /usr/sbin/xl2tpd ] || exit 0

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

file_reset() {
	: > "$1"
}

xappend() {
	local file="$1"
	shift

	echo "$@" >> "$file"
}

###########################################
clean_ipsec_config(){
	local interface="$1"
	local ike_name=l2tp_${interface}

	swanctl --terminate --ike $ike_name
}

setup_ipsec_config(){
	local interface="$1"
	local ike_name=l2tp_${interface}
	local timeout=0

	while [ $timeout -lt 30 ]; do
		local result=`swanctl --list-sas | grep ESTABLISHED | grep ${ike_name}`
		if [ ! -z "$result" ]; then
			break
		fi

		sleep 1	#wait ipsec tunnel established
		timeout=$(($timeout + 1))
	done

	if [ -z $result ]; then
		echo "no active ipsec connection for ${ike_name}" >> /tmp/l2tp_dbg
		proto_setup_failed "$interface" # "proto_setup_failed" will retry l2tp connection
		exit 1
	fi
}
###########################################

proto_l2tp_init_config() {
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "keepalive"
	proto_config_add_string "pppd_options"
	proto_config_add_boolean "ipv6"
	proto_config_add_int "mtu"
	proto_config_add_int "checkup_interval"
	proto_config_add_string "server"
	proto_config_add_boolean "ipsec"
	proto_config_add_string "rtk_ppp_authmode"
	proto_config_add_string "rtk_ppp_mppe"
	available=1
	no_device=1
	no_proto_task=1
	teardown_on_l3_link_down=1
}

proto_l2tp_setup() {
	local interface="$1"
	local optfile="/tmp/l2tp/options.${interface}"
	local ip serv_addr server host
	local ipsec
	local rtk_ppp_authmode
	local rtk_ppp_mppe

	json_get_var server server
	json_get_var ipsec ipsec
	json_get_var rtk_ppp_authmode rtk_ppp_authmode
	json_get_var rtk_ppp_mppe rtk_ppp_mppe
	host="${server%:*}"
	for ip in $(resolveip -t 5 "$host"); do
		( proto_add_host_dependency "$interface" "$ip" )
		serv_addr=1
	done
	[ -n "$serv_addr" ] || {
		echo "Could not resolve server address" >&2
		sleep 5
		proto_setup_failed "$interface"
		exit 1
	}

	if [ "$ipsec" = 1 ]; then
		setup_ipsec_config ${interface}
	fi

	# Start and wait for xl2tpd
	if [ ! -p /var/run/xl2tpd/l2tp-control -o -z "$(pidof xl2tpd)" ]; then
		/etc/init.d/xl2tpd restart

		local wait_timeout=0
		while [ ! -p /var/run/xl2tpd/l2tp-control ]; do
			wait_timeout=$(($wait_timeout + 1))
			[ "$wait_timeout" -gt 5 ] && {
				echo "Cannot find xl2tpd control file." >&2
				proto_setup_failed "$interface"
				exit 1
			}
			sleep 1
		done
	fi

	local ipv6 keepalive username password pppd_options mtu
	json_get_vars ipv6 keepalive username password pppd_options mtu
	[ "$ipv6" = 1 ] || ipv6=""

	local interval="${keepalive##*[, ]}"
	[ "$interval" != "$keepalive" ] || interval=5

	keepalive="${keepalive:+lcp-echo-interval $interval lcp-echo-failure ${keepalive%%[, ]*}}"
	username="${username:+user \"$username\" password \"$password\"}"
	ipv6="${ipv6:++ipv6}"
	mtu="${mtu:+mtu $mtu mru $mtu}"

	mkdir -p /tmp/l2tp
	cat <<EOF >"$optfile"
usepeerdns
nodefaultroute
ipparam "$interface"
ifname "l2tp-$interface"
ip-up-script /lib/netifd/ppp-up
ipv6-up-script /lib/netifd/ppp-up
ip-down-script /lib/netifd/ppp-down
ipv6-down-script /lib/netifd/ppp-down
# Don't wait for LCP term responses; exit immediately when killed.
lcp-max-terminate 0
$keepalive
$username
$ipv6
$mtu
$pppd_options
EOF

	case $rtk_ppp_authmode in
		pap)
			xappend $optfile refuse-chap
			xappend $optfile refuse-mschap
			xappend $optfile refuse-mschap-v2
			xappend $optfile refuse-eap
			xappend $opt_file "nomppe"
		;;
		chap)
			xappend $optfile refuse-pap
			xappend $optfile refuse-mschap
			xappend $optfile refuse-mschap-v2
			xappend $optfile refuse-eap
			xappend $opt_file "nomppe"
		;;
		mschap)
			xappend $optfile refuse-pap
			xappend $optfile refuse-chap
			xappend $optfile refuse-mschap-v2
			xappend $optfile refuse-eap
			[ -z $rtk_ppp_mppe ] || xappend $optfile $rtk_ppp_mppe
		;;
		mschapv2)
			xappend $optfile refuse-pap
			xappend $optfile refuse-chap
			xappend $optfile refuse-mschap
			xappend $optfile refuse-eap
			[ -z $rtk_ppp_mppe ] || xappend $optfile $rtk_ppp_mppe
		;;
		eap)
			xappend $optfile refuse-pap
			xappend $optfile refuse-chap
			xappend $optfile refuse-mschap
			xappend $optfile refuse-mschap-v2
			xappend $opt_file "nomppe"
		;;
		*)
		;;
	esac

	xl2tpd-control add-lac l2tp-${interface} pppoptfile=${optfile} lns=${server} || {
		echo "xl2tpd-control: Add l2tp-$interface failed" >&2
		proto_setup_failed "$interface"
		exit 1
	}
	xl2tpd-control connect-lac l2tp-${interface} || {
		echo "xl2tpd-control: Connect l2tp-$interface failed" >&2
		proto_setup_failed "$interface"
		exit 1
	}
}

proto_l2tp_teardown() {
	local interface="$1"
	local optfile="/etc/ppp/options.${interface}"
        local ipsec

	rm -f ${optfile}
	if [ -p /var/run/xl2tpd/l2tp-control ]; then
		xl2tpd-control remove-lac l2tp-${interface} || {
			echo "xl2tpd-control: Remove l2tp-$interface failed" >&2
		}
	fi
	# Wait for interface to go down
	while [ -d /sys/class/net/l2tp-${interface} ]; do
		sleep 1
	done

	clean_ipsec_config ${interface}
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol l2tp
}
