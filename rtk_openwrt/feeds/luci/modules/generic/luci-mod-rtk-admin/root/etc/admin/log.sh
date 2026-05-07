#!/bin/sh

echo "save log"

/bin/dmesg -r > /www/kernel.log
tar cfvz /www/log.tar.gz /www/system.log /www/kernel.log
