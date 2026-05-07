#!/bin/sh

if [ "$ACTION" = "released" ] && [ "$BUTTON" = "wps" ]; then
	# If the button was pressed for one second or more, trigger WPS on hostapd.
	if [ "$SEEN" -ge 1 ] ; then
		wps_done=0
		ubusobjs="$( ubus -S list hostapd.* )"
		for ubusobj in $ubusobjs; do
			ubus -S call $ubusobj wps_start && wps_done=1
		done
		[ $wps_done = 0 ] && echo "WPS: No hostapd instance to trigger." > /dev/console
	fi
fi

return 0
