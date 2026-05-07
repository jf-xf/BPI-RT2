#!/bin/sh

FW3_SCRIPT_DIR="/usr/share/fw3/firewall.d"
for file in $FW3_SCRIPT_DIR/*; do
	[ -x $file ] && sh $file
done
