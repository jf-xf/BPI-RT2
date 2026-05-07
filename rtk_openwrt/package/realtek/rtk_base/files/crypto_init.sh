#!/bin/sh

eval `mib get FC_IPSEC_EN_SHORTCUT`

if [ $FC_IPSEC_EN_SHORTCUT != "" ]; then
	if [ $FC_IPSEC_EN_SHORTCUT == "1" ]; then
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
	elif [ $FC_IPSEC_EN_SHORTCUT == "2" ]; then
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
		echo "mode 7 port 0 cpu 0 port 1 cpu 2 port 2 cpu 0 port 3 cpu 2 port 4 cpu 0 rps 0x5" > proc/fc/ctrl/smp_dispatch_nic_rx
		echo "mode 1 cpu 1" > /proc/fc/ctrl/smp_dispatch_nic_tx
	else
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
		echo 1 > /proc/fc/ctrl/ipsec_en_pe_offload
		sleep 1
		modprobe rtk_hwcrypto
	fi
fi
