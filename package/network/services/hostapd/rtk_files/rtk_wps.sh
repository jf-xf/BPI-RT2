#!/bin/sh

. /lib/functions.sh

TEST_BTN=0

if [ -f "/tmp/btn_test" ]
then
	TEST_BTN=1
fi

if [ "$ACTION" = "pressed" -a "$BUTTON" = "wps" ]; then
	if [ -f "/tmp/btn_test" ]
	then
		echo "[btn test]Do press wps button" > /dev/console
	fi
	hwbtn_trigger -b WPS -a 0 -d $TEST_BTN -t 1 > /dev/console
fi

return 0
