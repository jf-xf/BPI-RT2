#!/bin/sh

echo "Scan Channel"

echo $1 > /tmp/channel_scan_band
if [ $1 = '0' ]; then
	rm -rf /tmp/map_channel_scan_report_2G
	rm -rf /tmp/map_channel_scan_report_5G
	rm -rf /tmp/map_channel_scan_nr_2G
	rm -rf /tmp/map_channel_scan_nr_5G
elif [ $1 = '1' ]; then
	rm -rf /tmp/map_channel_scan_report_2G
	rm -rf /tmp/map_channel_scan_nr_2G
elif [ $1 = '2' ]; then
	rm -rf /tmp/map_channel_scan_report_5G
	rm -rf /tmp/map_channel_scan_nr_5G
fi
