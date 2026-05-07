#!/bin/sh

pon_mode=`mib get PON_MODE`
if [ "$pon_mode" == "PON_MODE=1" ]; then
	if [ ! -f /var/config/run_customized_sdk.sh ]; then
		cp /etc/run_customized_sdk.sh /var/config/run_customized_sdk.sh
	fi
		/var/config/run_customized_sdk.sh
		if [ -f /lib/modules/pf_rt_fc.ko ]; then
			ifconfig eth0 up
		fi
		/etc/runomci.sh
		/bin/pondetect 1&
		echo "running GPON mode ..."
elif [ "$pon_mode" == "PON_MODE=2" ]; then
        modprobe ca-rtk-epon-drv
        /etc/runoam.sh
        sleep 2
	/bin/pondetect 2&
        echo "running EPON mode ..."
elif [ "$pon_mode" == "PON_MODE=3" ]; then
        echo "running Ether mode..."
else
        echo "Not supported PON/WAN mode..."
fi

