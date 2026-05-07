#!/bin/bash

REALTEK_AP_G6=rtk_openwrt/package/kernel/realtek/rtk_wifi6_ap/

[ -f preconfig/config_user ] &&  if [ `cat preconfig/config_user | grep rtk_wifi6_ap=y` != "" ]; then
	if [ -f ${REALTEK_AP_G6}/src/Kconfig ]; then
		cp ${REALTEK_AP_G6}/src/Kconfig ${REALTEK_AP_G6}/Kconfig.openwrt;
		comment_line_num=`cat -n ${REALTEK_AP_G6}/Kconfig.openwrt | grep "if RTLWIFI6" | awk '{print $1}'`
		comment_line_num=$((comment_line_num-1))
		sed -i "1,${comment_line_num} s/^/#/" ${REALTEK_AP_G6}/Kconfig.openwrt;
		sed -i '/wfo/ s/^/#/g' ${REALTEK_AP_G6}/Kconfig.openwrt;
		sed -i '/depends on PCI/ s/^/#/g' ${REALTEK_AP_G6}/Kconfig.openwrt;

		config_line_num=`cat -n ${REALTEK_AP_G6}/Config.in | grep -w config | awk '{print $1" "$2}' | grep config | awk '{print $1}'`
		for i in ${config_line_num}
		do
			sed -n "${i},${i}p" ${REALTEK_AP_G6}/Config.in | awk '{print $2"="}' >> ${REALTEK_AP_G6}/tmp_local-symbols
		done

		config_line_num=`cat -n ${REALTEK_AP_G6}/Kconfig.openwrt | grep -w config | awk '{print $1" "$2}' | grep config | awk '{print $1}'`
		for i in ${config_line_num}
		do
			sed -n "${i},${i}p" ${REALTEK_AP_G6}/Kconfig.openwrt | awk '{print $2"="}' >> ${REALTEK_AP_G6}/tmp_local-symbols
		done
		cp ${REALTEK_AP_G6}/tmp_local-symbols ${REALTEK_AP_G6}/local-symbols
		rm -f ${REALTEK_AP_G6}/tmp_local-symbols
	fi
fi
