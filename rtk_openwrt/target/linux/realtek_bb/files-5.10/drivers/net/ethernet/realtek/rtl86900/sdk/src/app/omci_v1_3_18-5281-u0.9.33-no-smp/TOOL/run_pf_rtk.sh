# User should must flash set DEVICE_TYPE 0 and DUAL_MGMT_MODE 0 as pure SFU
OMCI_APP_PID=`ps | grep omci_app | grep -v grep | awk -F " " '{print $1}'`
kill -9 $OMCI_APP_PID
echo 2 > /proc/rg/hwnat
rmmod rtk_tr142
rmmod me_00100000
rmmod pf_rg
rmmod omcidrv
/bin/diag debug rtk-init
/bin/diag vlan init
/bin/diag svlan init
/bin/diag acl init
/bin/diag l2-table init
insmod /lib/modules/omcidrv.ko
insmod /lib/modules/pf_rtk.ko
/etc/runomci.sh

