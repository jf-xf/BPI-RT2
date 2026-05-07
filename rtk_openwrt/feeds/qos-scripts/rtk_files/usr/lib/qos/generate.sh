#!/bin/sh

#DEBUG log
QOS_DEBUG_LOG='/tmp/qos_script_log'

#RTK TR142 control
TR142CTL='/bin/tr142_ctl'
TR142_QUEUE_NUM=8;
TR142_USER_QUEUE_NUM=0;
TR142_PIR_MAX=262143992;

#GPON up rate in kbps
GPON_RATE_MAX=32767999;

#RTK smux rules define here
SMUXCTL='/bin/smuxctl'
SMUXCTL_8021P_REMARK_NAME='ip_qos_mark_8021p_us_'
SMUX_MAX_TAGS=2

#RTK Ebtables Leagcy
EBTABLES='/usr/sbin/ebtables'

#RTK FC parameter define here
skbmark_qid=0;

#FC meter use mark2
swShaper_en=0;
meterIdx=0;
meterIdx_en=0;
MAX_METER_NUM=32;
UBUS_ADD_RATELIMIT='/bin/ubus call rtk-rpc add_ratelimit_rule'
UBUS_FLUSH_RATELIMIT='/bin/ubus call rtk-rpc flush_ratelimit_rule'
UBUS_FLUSH_CONNTRACK='/bin/ubus call rtk-rpc flush_conntrack'


[ -e /etc/profile.d/fc_env.sh ] && {
        source /etc/profile.d/fc_env.sh
        if [ -z $skbmark_qid ]; then
                skbmark_qid=0;
        fi
        if [ -z $meterIdx_en ]; then
                meterIdx_en=1;
        fi
        if [ -z $swShaper_en ]; then
                swShaper_en=2;
        fi
        if [ -z $meterIdx ]; then
                meterIdx=3;
        fi
}
echo "Mark: skbmark_qid=$skbmark_qid, skbmark_8021p=$skbmark_8021p, skbmark_8021pEn=$skbmark_8021pEn" > $QOS_DEBUG_LOG
echo "Mark2: meterIdx_en=$meterIdx_en, swShaper_en=$swShaper_en, meterIdx=$meterIdx" >> $QOS_DEBUG_LOG

[ -e /lib/functions.sh ] && . /lib/functions.sh || . ./functions.sh
[ -x /sbin/modprobe ] && {
	insmod="modprobe"
	rmmod="$insmod -r"
} || {
	insmod="insmod"
	rmmod="rmmod"
}

add_insmod() {
	eval "export isset=\${insmod_$1}"
	case "$isset" in
		1) ;;
		*) {
			[ "$2" ] && append INSMOD "$rmmod $1 >&- 2>&-" "$N"
			append INSMOD "$insmod $* >&- 2>&-" "$N"; export insmod_$1=1
		};;
	esac
}

[ -e /etc/config/network ] && {
	# only try to parse network config on openwrt

	. /lib/functions/network.sh

	find_ifname() {
		local ifname
		if network_get_device ifname "$1"; then
			echo "$ifname"
		else
			echo "Device for interface $1 not found." >&2
			exit 1
		fi
	}
} || {
	find_ifname() {
		echo "Interface not found." >&2
		exit 1
	}
}

parse_matching_rule() {
	local var="$1"
	local section="$2"
	local options="$3"
	local prefix="$4"
	local suffix="$5"
	local proto="$6"
	local ebtrule_var="$7"
	local mport=""
	local ports=""
	local direction=-1
	local wan_ifname="na"
	append "$var" "$prefix" "$N"

	for option in $options; do
		case "$option" in
			proto) config_get value "$section" proto; proto="${proto:-$value}";;
		esac
	done
	config_get type "$section" TYPE
#	case "$type" in
#		classify) unset pkt; append "$var" "-m mark --mark 0/0x70";;
#		default) pkt=1; append "$var" "-m mark --mark 0/0x70";;
#		reclassify) pkt=1;;
#	esac
	append "$var" "${proto:+-p $proto}"
	for option in $options; do
		config_get value "$section" "$option"
		
		case "$pkt:$option" in
			*:srchost)
				append "$var" "-s $value"
			;;
			*:dsthost)
				append "$var" "-d $value"
			;;
			*:ports|*:srcports|*:dstports)
				value="$(echo "$value" | sed -e 's,-,:,g')"
				lproto=${lproto:-tcp}
				case "$proto" in
					""|tcp|udp) append "$var" "-m ${proto:-tcp -p tcp} -m multiport";;
					icmp) append "$var";;
					*) unset "$var"; return 0;;
				esac
				case "$option" in
					ports)
						config_set "$section" srcports ""
						config_set "$section" dstports ""
						config_set "$section" portrange ""
						case "$proto" in
							""|tcp|udp) append "$var" "--ports $value";;
						esac
					;;
					srcports)
						config_set "$section" ports ""
#						config_set "$section" dstports ""
						config_set "$section" portrange ""
						case "$proto" in
							""|tcp|udp) append "$var" "--sports $value";;
						esac
					;;
					dstports)
						config_set "$section" ports ""
#						config_set "$section" srcports ""
						config_set "$section" portrange ""
						case "$proto" in
							""|tcp|udp) append "$var" "--dports $value"
						esac
					;;
				esac
				ports=1
			;;
			*:portrange)
				config_set "$section" ports ""
				config_set "$section" srcports ""
				config_set "$section" dstports ""
				value="$(echo "$value" | sed -e 's,-,:,g')"
				case "$proto" in
					""|tcp|udp) append "$var" "-m ${proto:-tcp -p tcp} --sport $value --dport $value";;
					icmp) append "$var";;
					*) unset "$var"; return 0;;
				esac
				ports=1
			;;
			*:connbytes)
				value="$(echo "$value" | sed -e 's,-,:,g')"
				add_insmod xt_connbytes
				append "$var" "-m connbytes --connbytes $value --connbytes-dir both --connbytes-mode bytes"
			;;
			*:comment)
				add_insmod xt_comment
				append "$var" "-m comment --comment '$value'"
			;;
			*:tos)
				if [ "$value" -ge 0 ] 2>/dev/null ;then 
					add_insmod xt_dscp
					case "$value" in
						!*) append "$var" "-m tos ! --tos $value";;
						*) append "$var" "-m tos --tos $value"
					esac
				fi
			;;
			*:dscp)
				if [ "$value" -ge 0 ] 2>/dev/null ;then 
					add_insmod xt_dscp
					dscp_option="--dscp"
					[ -z "${value%%[EBCA]*}" ] && dscp_option="--dscp-class"
					config_get lanif "$section" "lan_interface"
					if [ -n "$lanif" ]; then
						value=0x$(printf %x $value)
						append "$ebtrule_var" "-p ipv4 --ip-tos $value"
					else
						case "$value" in
							!*) append "$var" "-m dscp ! $dscp_option $value";;
							*) append "$var" "-m dscp $dscp_option $value"
						esac
					fi
				fi
			;;
			*:direction)
				value="$(echo "$value" | sed -e 's,-,:,g')"
				if [ "$value" = "out" ]; then
					append "$var" "-o $device"
				elif [ "$value" = "in" ]; then
					append "$var" "-i $device"
				elif [ "$value" = "0" ]; then #for rateLimit upstream
					direction=0
					#append "$var" "-o veip+"
				elif [ "$value" = "1" ]; then #for rateLimit downstream
					direction=1
					#append "$var" "-o br-lan"
				fi
			;;
			*:srciface)
				append "$var" "-i $value"
			;;
			*:lan_interface)
				add_insmod ebt_ftos
				append "$ebtrule_var" "-i $value"
			;;
			*:src_mac)
				append "$ebtrule_var" "-s $value"
			;;
			*:dst_mac)
				append "$ebtrule_var" "-d $value"
			;;
			*:ether_type)
				append "$ebtrule_var" "-p 0x$value"
			;;
			*:wan_interface)
				wan_ifname=`ubus call network.interface.$value status | grep 'l3_device' |awk '{print $2}'`
				wan_ifname=${wan_ifname%,*}
#				echo "wan_ifname=$wan_ifname" >&2
				#append "$var" "-o $wan_ifname"
			;;
			1:pktsize)
				value="$(echo "$value" | sed -e 's,-,:,g')"
				add_insmod xt_length
				append "$var" "-m length --length $value"
			;;
			1:limit)
				add_insmod xt_limit
				append "$var" "-m limit --limit $value"
			;;
			1:tcpflags)
				case "$proto" in
					tcp) append "$var" "-m tcp --tcp-flags ALL $value";;
					*) unset $var; return 0;;
				esac
			;;
			1:mark)
				config_get class "${value##!}" classnr
				[ -z "$class" ] && continue;
				case "$value" in
					!*) append "$var" "-m mark ! --mark $class/0x70";;
					*) append "$var" "-m mark --mark $class/0x70";;
				esac
			;;
			1:TOS)
				add_insmod xt_DSCP
				config_get TOS "$rule" 'TOS'
				suffix="-j TOS --set-tos "${TOS:-"Normal-Service"}
			;;
			1:DSCP)
				add_insmod xt_DSCP
				config_get DSCP "$rule" 'DSCP'
				[ -z "${DSCP%%[EBCA]*}" ] && set_value="--set-dscp-class $DSCP" \
				|| set_value="--set-dscp $DSCP"
				suffix="-j DSCP $set_value"
			;;
		esac
	done
	if [ "-1" -eq "$direction" ]; then
		if [ "na" != "$wan_ifname" ]; then
			append "$var" "-o $wan_ifname"
		fi
	else
		if [ "na" == "$wan_ifname" ]; then
			if [ 1 -eq "$direction" ]; then
				append "$var" "-o br-lan"
			else
				append "$var" "-o veip+"
			fi
		else
			if [ 1 -eq "$direction" ]; then
				append "$var" "-i $wan_ifname"
			else
				append "$var" "-o $wan_ifname"
			fi
		fi
	fi

	append "$var" "$suffix"
	case "$ports:$proto" in
		1:)	parse_matching_rule "$var" "$section" "$options" "$prefix" "$suffix" "udp";;
	esac
}

config_cb() {
	option_cb() {
		return 0
	}
	case "$1" in
		interface)
			config_set "$2" "classgroup" "Default"
			config_set "$2" "upload" "128"
		;;
		classify|default|reclassify|ratelimit)
			option_cb() {
				append "CONFIG_${CONFIG_SECTION}_options" "$1"
			}
		;;
	esac
}

qos_parse_config() {
	config_get TYPE "$1" TYPE
	case "$TYPE" in
		interface)
			config_get_bool enabled "$1" enabled 1
			[ 1 -eq "$enabled" ] && {
				config_get classgroup "$1" classgroup
				config_set "$1" ifbdev "$C"
				C=$(($C+1))
				append INTERFACES "$1"
				config_set "$classgroup" enabled 1
				config_get device "$1" device
				[ -z "$device" ] && {
# RTK didn't need to parse device for veip0, didn't exist in /etc/config/network
#					device="$(find_ifname $1)"
#					[ -z "$device" ] && exit 1
					config_set "$1" device "$device"
				}

			}
		;;
		classgroup) append CG "$1";;
		classify|default|reclassify)
			case "$TYPE" in
				classify) var="ctrules";;
				*) var="rules";;
			esac
			append "$var" "$1"
		;;
		ratelimit)
			case "$TYPE" in
				ratelimit) var="rtrules";;
				*) var="rules";;
			esac
			append "$var" "$1"
		;;
	esac
}

enum_classes() {
	local c="0"
	config_get classes "$1" classes
	config_get default "$1" default
	for class in $classes; do
		c="$(($c + 1))"
		config_set "${class}" classnr $c
		case "$class" in
			$default) class_default=$c;;
		esac
	done
	class_default="${class_default:-$c}"
}

cls_var() {
	local varname="$1"
	local class="$2"
	local name="$3"
	local type="$4"
	local default="$5"
	local tmp tmp1 tmp2
	config_get tmp1 "$class" "$name"
	config_get tmp2 "${class}_${type}" "$name"
	tmp="${tmp2:-$tmp1}"
	tmp="${tmp:-$tmp2}"
	export ${varname}="${tmp:-$default}"
}

tcrules() {
	_dir=/usr/lib/qos
	[ -e $_dir/tcrules.awk ] || _dir=.
	echo "$cstr" | awk \
		-v device="$dev" \
		-v linespeed="$rate" \
		-v direction="$dir" \
		-f $_dir/tcrules.awk
}

start_interface() {
	local iface="$1"
	local num_ifb="$2"
	config_get device "$iface" device
	config_get_bool enabled "$iface" enabled 1
	[ -z "$device" -o 1 -ne "$enabled" ] && {
		return 1 
	}
	config_get upload "$iface" upload
	config_get_bool halfduplex "$iface" halfduplex
	config_get download "$iface" download
	config_get classgroup "$iface" classgroup
	config_get_bool overhead "$iface" overhead 0
	
	download="${download:-${halfduplex:+$upload}}"
	enum_classes "$classgroup"
	for dir in ${halfduplex:-up} ${download:+down}; do
		case "$dir" in
			up)
				[ "$overhead" = 1 ] && upload=$(($upload * 98 / 100 - (15 * 128 / $upload)))
				dev="$device"
				rate="$upload"
				dl_mode=""
				prefix="cls"
			;;
			down)
				[ "$(ls -d /proc/sys/net/ipv4/conf/ifb* 2>&- | wc -l)" -ne "$num_ifb" ] && add_insmod ifb numifbs="$num_ifb"
				config_get ifbdev "$iface" ifbdev
				[ "$overhead" = 1 ] && download=$(($download * 98 / 100 - (80 * 1024 / $download)))
				dev="ifb$ifbdev"
				rate="$download"
				dl_mode=1
				prefix="d_cls"
			;;
			*) continue;;
		esac
		cstr=
		for class in $classes; do
			cls_var pktsize "$class" packetsize $dir 1500
			cls_var pktdelay "$class" packetdelay $dir 0
			cls_var maxrate "$class" limitrate $dir 100
			cls_var prio "$class" priority $dir 1
			cls_var avgrate "$class" avgrate $dir 0
			cls_var qdisc "$class" qdisc $dir ""
			cls_var filter "$class" filter $dir ""
			config_get classnr "$class" classnr
			append cstr "$classnr:$prio:$avgrate:$pktsize:$pktdelay:$maxrate:$qdisc:$filter" "$N"
		done
		append ${prefix}q "$(tcrules)" "$N"
		export dev_${dir}="ifconfig $dev up >&- 2>&-
tc qdisc del dev $dev root >&- 2>&-
tc qdisc add dev $dev root handle 1: hfsc default ${class_default}0
tc class add dev $dev parent 1: classid 1:1 hfsc sc rate ${rate}kbit ul rate ${rate}kbit"
	done
	[ -n "$download" ] && {
		add_insmod cls_u32
		add_insmod em_u32
		add_insmod act_connmark
		add_insmod act_mirred
		add_insmod sch_ingress
	}
	if [ -n "$halfduplex" ]; then
		export dev_up="tc qdisc del dev $device root >&- 2>&-
tc qdisc add dev $device root handle 1: hfsc
tc filter add dev $device parent 1: prio 10 u32 match u32 0 0 flowid 1:1 action mirred egress redirect dev ifb$ifbdev"
	elif [ -n "$download" ]; then
		append dev_${dir} "tc qdisc del dev $device ingress >&- 2>&-
tc qdisc add dev $device ingress
tc filter add dev $device parent ffff: prio 1 u32 match u32 0 0 flowid 1:1 action connmark action mirred egress redirect dev ifb$ifbdev" "$N"
	fi
	add_insmod cls_fw
	add_insmod sch_hfsc

	cat <<EOF
${INSMOD:+$INSMOD$N}${dev_up:+$dev_up
$clsq
}${ifbdev:+$dev_down
$d_clsq
$d_clsl
$d_clsf
}
EOF
	unset INSMOD clsq clsf clsl d_clsq d_clsl d_clsf dev_up dev_down
}

start_interfaces() {
	local C="$1"
	for iface in $INTERFACES; do
		start_interface "$iface" "$C"
	done
}

add_rt_rules() {
	local var="$1"
	local rules="$2"
	local prefix="$3"
	local i=0;
	target_mask=0;
	rt_meter_index=-1;

	$UBUS_FLUSH_RATELIMIT
	for rule in $rules; do
		unset iptrule
		config_get options "$rule" options
		config_get rate_limit "$rule" "rate_limit"
		parse_matching_rule iptrule "$rule" "$options" "$prefix" "-j qos_RateLimit_ct_rule_$i" "" ""
		$command -w -t mangle -N qos_RateLimit_ct_rule_$i
		append "$var" "$iptrule" "$N"
		unset iptrule

		if [ "$i" -le $MAX_METER_NUM ] 2>/dev/null; then
			#/bin/ubus call rtk-rpc add_ratelimit_rule '{"ratelimit_index":"'$i'", "ratelimit_rate":"'$rate_limit'"}'
			rt_meter_index=`$UBUS_ADD_RATELIMIT '{"ratelimit_index":"'$i'", "ratelimit_rate":"'$rate_limit'"}'| grep result | awk '{print$2}'`
			if [ "$rt_meter_index" -ge 0 ] 2>/dev/null; then
				echo "Setup RT Meter: rule = $i, RT meter ID = $rt_meter_index rate=$rate_limit" >> $QOS_DEBUG_LOG
				target=$(($rt_meter_index << $meterIdx))
				target_mask=$((31<<$meterIdx))
				if [ ! -z $meterIdx_en ]; then
					target=$(($target|(1<< $meterIdx_en)))
					target_mask=$(($target_mask|(1<<$meterIdx_en)))
				fi
				if [ ! -z $swShaper_en ]; then
					target=$(($target|(1<< $swShaper_en)))
					target_mask=$(($target_mask|(1<<$swShaper_en)))
				fi
				target=0x$(printf %x $target)
				target_mask=0x$(printf %x $target_mask)
				$command -w -t mangle -A qos_RateLimit_ct_rule_$i -j MARK2 --set-mark2 $target/$target_mask
				echo "$command -w -t mangle -A qos_RateLimit_ct_rule_$i -j MARK2 --set-mark2 $target/$target_mask" >> $QOS_DEBUG_LOG
			else
				echo "Error! can't add anymore rt ratelimit rules!" >> $QOS_DEBUG_LOG
			fi
		else
			echo "Error! Sw meter idx support up to $MAX_METER_NUM, can't add anymore rules!" >> $QOS_DEBUG_LOG
		fi
		$command -w -t mangle -A qos_RateLimit_ct_rule_$i -j RETURN
		i=$((i+1))
	done
}

add_rules() {
	local var="$1"
	local rules="$2"
	local prefix="$3"
	local i=0;
	target_mask=0;

	for rule in $rules; do
		unset iptrule
		config_get target_queue "$rule" target
		#config_get target "$target" classnr
		config_get options "$rule" options
		## If we want to override the TOS field, let's clear the DSCP field first.
		[ ! -z "$(echo $options | grep 'TOS')" ] && {
			s_options=${options%%TOS}
			add_insmod xt_DSCP
			parse_matching_rule iptrule "$rule" "$s_options" "$prefix" "-j DSCP --set-dscp 0"
			append "$var" "$iptrule" "$N"
			unset iptrule
		}
		reorder_qid=`uci get qos.$target_queue.reorder`

		if [ "$reorder_qid" = "0" ]
		then
			val=0
		else
			val=$(($TR142_USER_QUEUE_NUM-$reorder_qid));
		fi
		target=$(($val << $skbmark_qid))
		target_mask=$((7<<$skbmark_qid))

        if [ ! -z $skbmark_8021pEn ]; then
			config_get value "$rule" "8021p"
			if [ "$value" -gt -1 ] 2>/dev/null ;then
				target=$(($target|($value<< $skbmark_8021p)|(1<<$skbmark_8021pEn)))
				target_mask=$(($target_mask|(7<<$skbmark_8021p)|(1<<$skbmark_8021pEn)))
			fi
        fi

		target=0x$(printf %x $target)
		target_mask=0x$(printf %x $target_mask)
#		parse_matching_rule iptrule "$rule" "$options" "$prefix" "-j MARK --set-mark $target/0x70"
#		parse_matching_rule iptrule "$rule" "$options" "$prefix" "-j MARK --set-mark $target/$target_mask"
		unset ebtrule
		parse_matching_rule iptrule "$rule" "$options" "$prefix" "-j qos_${cg}_ct_rule_$i" "" ebtrule
		if [ -z "$ebtrule" ];then
			 $command -w -t mangle -N qos_${cg}_ct_rule_$i
			append "$var" "$iptrule" "$N"
		elif [ "$command" = "iptables" ]; then
			$EBTABLES -N qos_${cg}_ct_rule_$i
			$EBTABLES -t broute -N qos_${cg}_ct_rule_$i
		fi
		unset iptrule
		[ ! -z "$(echo $options | grep 'DSCP')" ] && {
			s_options=${options%%DSCP}
			config_get value "$rule" "DSCP"
			if [ "$value" -gt -1 ] 2>/dev/null ;then 
				add_insmod xt_DSCP
				if [ -z "$ebtrule" ];then
					$command -w -t mangle -A qos_${cg}_ct_rule_$i -j DSCP --set-dscp $value
				elif [ "$command" = "iptables" ]; then
					#command have iptables and ip6tables, just do once
					value=$(($value << 2))
					$EBTABLES -A qos_${cg}_ct_rule_$i -j ftos --set-ftos $value
					$EBTABLES -t broute -A qos_${cg}_ct_rule_$i -j ftos --set-ftos $value
				fi
			fi
		}
		if [ -z "$ebtrule" ];then
			$command -w -t mangle -A qos_${cg}_ct_rule_$i -j MARK --set-mark $target/$target_mask
			$command -w -t mangle -A qos_${cg}_ct_rule_$i -j RETURN
		elif [ "$command" = "iptables" ]; then
			#command have iptables and ip6tables, just do once
			$EBTABLES -A qos_Default_ct $ebtrule -j qos_${cg}_ct_rule_$i
			$EBTABLES -A qos_${cg}_ct_rule_$i -j mark --mark-or $target
			$EBTABLES -A qos_${cg}_ct_rule_$i -j RETURN
			$EBTABLES -t broute -A qos_Default_ct $ebtrule -j qos_${cg}_ct_rule_$i
			$EBTABLES -t broute -A qos_${cg}_ct_rule_$i -j mark --mark-or $target
			$EBTABLES -t broute -A qos_${cg}_ct_rule_$i -j RETURN
		fi

		i=$((i+1))
		unset ebtrule
	done
}

re_order_queue() {
	order_id=1;
	config_get classes "Default" classes
	for class in $classes; do
		config_get queue_enabled "$class" enabled
		if [ 1 -eq "$queue_enabled" ]; then
			uci set qos.$class.reorder="$order_id"
			order_id=$((order_id+1))
			TR142_USER_QUEUE_NUM=$((TR142_USER_QUEUE_NUM+1))
		else
			uci set qos.$class.reorder="0"
		fi
	done
	uci commit
}

start_cg() {
	local cg="$1"
	local iptrules
	local rt_iptrules
	local pktrules
	local sizerules
	enum_classes "$cg"
	local i=0;
	$EBTABLES -N qos_${cg}
	$EBTABLES -N qos_${cg}_ct
	$EBTABLES -P qos_${cg} RETURN
	$EBTABLES -P qos_${cg}_ct RETURN
	$EBTABLES -t broute -N qos_${cg}
	$EBTABLES -t broute -N qos_${cg}_ct
	$EBTABLES -t broute -P qos_${cg} RETURN
	$EBTABLES -t broute -P qos_${cg}_ct RETURN

	for iface in $INTERFACES; do
		config_get classgroup "$iface" classgroup
		config_get device "$iface" device
		config_get ifbdev "$iface" ifbdev
		config_get bandwidth_enabled "$iface" bandwidth_enabled
		config_get upload "$iface" upload
		config_get download "$iface" download
		config_get halfduplex "$iface" halfduplex
		config_get enabled "$iface" enabled

		if [ 1 -eq "$enabled" ]; then
			for command in $iptables; do
				add_rules iptrules "$ctrules" "$command -w -t mangle -A qos_${cg}_ct"
				add_rt_rules rt_iptrules "$rtrules" "$command -w -t mangle -A qos_RateLimit_ct"
			done

			config_get classes "$cg" classes
			for class in $classes; do
				config_get mark "$class" classnr
				config_get maxsize "$class" maxsize
				config_get queue_enabled "$class" enabled
				[ -z "$maxsize" -o -z "$mark" ] || {
					add_insmod xt_length
					for command in $iptables; do
						append pktrules "$command -w -t mangle -A qos_${cg} -m mark --mark $mark/0x70 -m length --length $maxsize: -j MARK --set-mark 0/0x70" "$N"
					done
				}
			done
			for command in $iptables; do
				add_rules pktrules "$rules" "$command -w -t mangle -A qos_${cg}"
			done
		fi

		download="${download:-${halfduplex:+$upload}}"
		for command in $iptables; do
#			append up "$command -w -t mangle -A OUTPUT -o $device -j qos_${cg}" "$N"
#			append up "$command -w -t mangle -A FORWARD -o $device -j qos_${cg}" "$N"
			append up "$command -w -t mangle -A OUTPUT -j qos_${cg}" "$N"
			append up "$command -w -t mangle -A FORWARD -j qos_${cg}" "$N"
		done
		append up "$EBTABLES -A OUTPUT -j qos_${cg}" "$N"
		append up "$EBTABLES -A FORWARD -j qos_${cg}" "$N"
		append up "$EBTABLES -t broute -A BROUTING -j qos_${cg}" "$N"

		#802.1p Remarking Rules via smuxctl
		if [ ! -z $skbmark_8021pEn ]; then
			echo "#### SMUX Remarking #####" >> $QOS_DEBUG_LOG
			i=7
			while [ $i -ge 0 ]
			do
				target=$(( ($i << $skbmark_8021p)|(1<<$skbmark_8021pEn) ))
				target=0x$(printf %x $target)
				target_mask=$(( (7 << $skbmark_8021p)|(1<<$skbmark_8021pEn) ))
				target_mask=0x$(printf %x $target_mask)
				#echo "smuxtags: i=$i target=$target target_mask=$target_mask" >&2
				smuxtags=$SMUX_MAX_TAGS
				while [ $smuxtags -ge 0 ]
					do
						if [ 1 -eq "$enabled" ]; then
							echo "[start_cg-remarking]:$SMUXCTL --if nas0 --tx --tags $smuxtags --filter-skb-mark $target_mask $target --set-priority $i 1 --rule-priority 1100 --rule-alias ip_qos_mark_8021p_us_nas0_$i --rule-append" >> $QOS_DEBUG_LOG
							$SMUXCTL --if nas0 --tx --tags $smuxtags --filter-skb-mark $target_mask $target --set-priority $i 1 --rule-priority 1100 --rule-alias ip_qos_mark_8021p_us_nas0_$i --rule-append
						else
							echo "[start_cg-remarking]:$SMUXCTL --if nas0 --tx --tags $smuxtags --rule-remove-alias ip_qos_mark_8021p_us_+" >> $QOS_DEBUG_LOG
							$SMUXCTL --if nas0 --tx --tags $smuxtags --rule-remove-alias ip_qos_mark_8021p_us_+
						fi
						smuxtags=$(($smuxtags-1));
					done
				i=$(($i-1));
			done
		fi

		#TR142 control
		if [ 1 -eq "$enabled" ]; then
			echo "#### TR142 Queue setting #####" >> $QOS_DEBUG_LOG
			config_get classes "Default" classes
			config_get prio_mode "Default" mode
			sch_str='sp';
			weight_value=0;
			echo "[start_cg-SPR]:$TR142CTL config_qos --en 0 --num 0" >> $QOS_DEBUG_LOG
			#$TR142CTL config_qos --en $enabled
			$TR142CTL config_qos --en 0 --num 0
			user_queue_num=$TR142_USER_QUEUE_NUM;
			echo "[start_cg-queue]:user_queue_num=$TR142_USER_QUEUE_NUM" >> $QOS_DEBUG_LOG
			qid=0;
			for class in $classes; do
				#Pre-queue setting
				config_get queue_enable "$class" enabled
				config_get prio_mode "Default" mode
				qpri=`uci get qos.$class.reorder`
				if [ "1" -eq "$queue_enable" ]; then
					if [ "$prio_mode" = "prio" ]; then
						#tr142_ctl config_qos --en 1 --num 8 --sch sp --qid 0 --qen 1 --qpri 7
						echo "[start_cg-SPR]:$TR142CTL config_qos --en $enabled --num $user_queue_num --sch sp --qid $qid --qen $queue_enable --qpri $qpri --qwt 0 --qpir $TR142_PIR_MAX" >> $QOS_DEBUG_LOG
						$TR142CTL config_qos --en $enabled --num $user_queue_num --sch sp --qid $qid --qen $queue_enable --qpri $qpri --qwt 0 --qpir $TR142_PIR_MAX
					else
						config_get weight_value "$class" weight
						#tr142_ctl config_qos --en 1 --num 8 --sch sp --qid 0 --qen 1 --qwt 30
						echo "[start_cg-WRR]:$TR142CTL config_qos --en $enabled --num $user_queue_num --sch wrr --qid $qid --qen $queue_enable --qpri 0 --qwt $weight_value --qpir $TR142_PIR_MAX" >> $QOS_DEBUG_LOG
						$TR142CTL config_qos --en $enabled --num $user_queue_num --sch wrr --qid $qid --qen $queue_enable --qpri 0 --qwt $weight_value --qpir $TR142_PIR_MAX
					fi
					qid=$(($qid+1));
				fi
			done
		else
			echo "[start_cg-disable]:$TR142CTL config_qos --en 0 --num 0" >> $QOS_DEBUG_LOG
			$TR142CTL config_qos --en 0 --num 0
		fi

		#GPON upstream rate limit
		if [ 1 -eq "$bandwidth_enabled" ]; then
			echo "[start_cg-Up Rate]:diag rt_gpon set egress rate $upload" >> $QOS_DEBUG_LOG
			diag rt_gpon set egress rate $upload
		else
			echo "[start_cg-Up Rate]:diag rt_gpon set egress rate $GPON_RATE_MAX" >> $QOS_DEBUG_LOG
			diag rt_gpon set egress rate $GPON_RATE_MAX
		fi
	done
	cat <<EOF
$INSMOD
EOF

for command in $iptables; do
	cat <<EOF
	$command -w -t mangle -N qos_${cg} 
	$command -w -t mangle -N qos_${cg}_ct
	$command -w -t mangle -N qos_RateLimit_ct
EOF
done
cat <<EOF
	${iptrules:+${iptrules}${N}}
EOF
cat <<EOF
	${rt_iptrules:+${rt_iptrules}${N}}
EOF
for command in $iptables; do
	cat <<EOF
#	$command -w -t mangle -A qos_${cg}_ct -j CONNMARK --save-mark --mask 0x70
#	$command -w -t mangle -A qos_${cg} -j CONNMARK --restore-mark --mask 0x70
#	$command -w -t mangle -A qos_${cg} -m mark --mark 0/0x70 -j qos_${cg}_ct
	$command -w -t mangle -A qos_${cg} -j qos_${cg}_ct
	$command -w -t mangle -A qos_${cg} -j qos_RateLimit_ct
EOF
done

$EBTABLES -A qos_${cg} -j qos_${cg}_ct
$EBTABLES -t broute -A qos_${cg} -j qos_${cg}_ct

cat <<EOF
$pktrules
EOF
for command in $iptables; do
	cat <<EOF
#	$command -w -t mangle -A qos_${cg} -j CONNMARK --save-mark --mask 0x70
EOF
done
cat <<EOF
$up$N${down:+${down}$N}
EOF
	unset INSMOD
}

start_firewall() {
	add_insmod xt_multiport
	add_insmod xt_connmark
	stop_firewall
	re_order_queue
	for group in $CG; do
		start_cg $group
	done
	$UBUS_FLUSH_CONNTRACK
	echo "$UBUS_FLUSH_CONNTRACK" >> $QOS_DEBUG_LOG
}

stop_firewall() {
	# Builds up a list of iptables commands to flush the qos_* chains,
	# remove rules referring to them, then delete them

	# Print rules in the mangle table, like iptables-save
	for command in $iptables; do
		$command -w -t mangle -S |
			# Find rules for the qos_* chains
			grep -E '(^-N qos_|-j qos_)' |
			# Exclude rules in qos_* chains (inter-qos_* refs)
			grep -v '^-A qos_' |
			# Replace -N with -X and hold, with -F and print
			# Replace -A with -D
			# Print held lines at the end (note leading newline)
			sed -e '/^-N/{s/^-N/-X/;H;s/^-X/-F/}' \
				-e 's/^-A/-D/' \
				-e '${p;g}' |
			# Make into proper iptables calls
			# Note: awkward in previous call due to hold space usage
			sed -n -e "s/^./${command} -w -t mangle &/p"
	done

	# Remove Ebtables rules
	RET=`$EBTABLES -L | grep qos_Default`
	if [ -n "$RET" ];then
		$EBTABLES -D OUTPUT -j qos_Default
		$EBTABLES -D FORWARD -j qos_Default
		$EBTABLES -t broute -D BROUTING -j qos_Default
		$EBTABLES -D qos_Default -j qos_Default_ct
		$EBTABLES -t broute -D qos_Default -j qos_Default_ct
		$EBTABLES -F qos_Default
		$EBTABLES -F qos_Default_ct
		$EBTABLES -t broute -F qos_Default
		$EBTABLES -t broute -F qos_Default_ct
		$EBTABLES -X qos_Default
		$EBTABLES -X qos_Default_ct
		$EBTABLES -t broute -X qos_Default
		$EBTABLES -t broute -X qos_Default_ct
	fi
	# Parse the qos_Default_ct_rule_x then delete
	# ebtables -L |grep qos_Default_ct_rule_ | awk '{print$3}' | sed -e 's/,//' | sed -n -e "s/^./`ebtables -X &`/"
	$EBTABLES -L |grep qos_Default_ct_rule_ | awk '{print$3}' | sed -e 's/,//' | sed -n -e "s/^./`$EBTABLES -X &`/"
	$EBTABLES -t broute -L |grep qos_Default_ct_rule_ | awk '{print$3}' | sed -e 's/,//' | sed -n -e "s/^./`$EBTABLES -t broute -X &`/"
	# Remove 802.1p Remarking smux rules
	smuxtags=$SMUX_MAX_TAGS
	while [ $smuxtags -ge 0 ]
	do
		$SMUXCTL --if nas0 --tx --tags $smuxtags --rule-remove-alias ip_qos_mark_8021p_us_+ 
		smuxtags=$(($smuxtags-1));
	done
	$UBUS_FLUSH_RATELIMIT
	$UBUS_FLUSH_CONNTRACK
}

C="0"
INTERFACES=""
[ -e ./qos.conf ] && {
	. ./qos.conf
	config_cb
} || {
	config_load qos
	config_foreach qos_parse_config
}

C="0"
for iface in $INTERFACES; do
	export C="$(($C + 1))"
done

[ -x /usr/sbin/ip6tables ] && {
	iptables="ip6tables iptables"
} || {
	iptables="iptables"
}

case "$1" in
	all)
		start_interfaces "$C"
		start_firewall
	;;
	interface)
		start_interface "$2" "$C"
	;;
	interfaces)
		start_interfaces
	;;
	firewall)
		case "$2" in
			stop)
				stop_firewall
			;;
			start|"")
				start_firewall
			;;
		esac
	;;
esac
