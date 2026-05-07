#!/bin/sh
### BEGIN INIT INFO
# Provides:          module-init-tools
# Required-Start:    
# Required-Stop:     
# Should-Start:      checkroot
# Should-stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Process /etc/modules.
# Description:       Load the modules listed in /etc/modules.
### END INIT INFO
str2hex()
{
  I=0
  while [ $I -lt ${#1} ];
  do
    printf "%02x" "'${1:$I:1}";
    let "I += 1"
  done
}
toBig() {
  v=$1
  v2=$(( ($v<<8 & 0xff00ff00) | ($v>>8 & 0xff00ff) ))
  v2=$(( (v2<<16 & 0xffff0000) | v2>>16 ))
  printf '0x%08x\n' $v2
}

pon_mode=`mib get PON_MODE | grep PON_MODE`
pon_speed=`mib get PON_SPEED | grep PON_SPEED`
pon_sub_mode=`mib get PON_SUB_MODE | grep PON_SUB_MODE`
pon_ben_pol_reverse=`mib get PON_BEN_POL_REVERSE | grep PON_BEN_POL_REVERSE`
grep_scfg_file=/config/default_scfg.txt
scfg_file=/config/scfg.txt
scfg_pon_mode=`grep CFG_ID_PON_MAC_MODE $grep_scfg_file | awk '{print $NF}' | sed 's/;$//'`
tr247_cfg=`mib get TR247_UNMATCHED_VEIP_CFG`

ca_rtk_pon_set_scfg() {
    if [ -f $scfg_file ] ; then
        cur_voq=`grep CFG_ID_PON_VOQ_MODE $grep_scfg_file | awk '{print $NF}' | sed 's/;$//'`
        cur_vendor=`grep CFG_ID_PON_VENDOR_ID $grep_scfg_file | awk '{print $NF}' | sed 's/;$//'`
        cur_vssn=`grep CFG_ID_PON_VSSN $grep_scfg_file | awk '{print $NF}' | sed 's/;$//'`
        cur_puc_buf_lmt_A=`grep CFG_ID_PUC_VOQBUFLIMIT_A $grep_scfg_file | awk '{print $NF}' | sed 's/;$//'`
        if [ "$tr247_cfg" == "TR247_UNMATCHED_VEIP_CFG=1" ] && [ ${cur_puc_buf_lmt_A:2} -gt 3000 ]; then
            sed -i  '/CFG_ID_PUC_VOQBUFLIMIT_A/d' $scfg_file
            sed -i  '$a INT CFG_ID_PUC_VOQBUFLIMIT_A = 0x3000;' $scfg_file
            echo "setting CFG_ID_PUC_VOQBUFLIMIT_A of scfg to 0x3000 for TR-247 testing ..."
        fi

        if [ "$pon_mode" == "PON_MODE=255" ] ; then
            sed -i  '/CFG_ID_PON_MAC_MODE/d' $scfg_file
            sed -i  '$a CHAR CFG_ID_PON_MAC_MODE = 0xFF;' $scfg_file
            echo "setting CFG_ID_PON_MAC_MODE of scfg to IGNORED(0xFF) ..."
            return 1
        fi
        if [ "$scfg_pon_mode" == "0xFF" ]; then
            echo "default CFG_ID_PON_MAC_MODE of scfg is IGNORED(0xFF) ..."
            return 1
        fi
        if [ "$pon_mode" == "PON_MODE=1" ] ; then
            gpon_vendor=`mib get GPON_SN | cut -c 9-12`
            gpon_vssn=`mib get GPON_SN | cut -c 13-20`
            gpon_vssn="0x$gpon_vssn"
            if [ "$pon_speed" == "PON_SPEED=0" ]; then
                ngpon_pwd=`mib get GPON_PLOAM_PASSWD | cut -d '=' -f2 | cut -c 1-10`
            else
                ngpon_pwd=`mib get GPON_PLOAM_PASSWD | cut -d '=' -f2 `
            fi
            ngpon_pwd_hex=`str2hex $ngpon_pwd`
            pad="000000000000000000000000000000000000000000000000000000000000000000000000"
            ngpon_pwd_hex=`printf '%s%*.*s\n' "$ngpon_pwd_hex" 0 $((72 - ${#ngpon_pwd_hex})) "$pad"`

            for i in  0 1 2 3 4 5 6 7 8
            do
                scfg_regX=`grep CFG_ID_PON_REGISTRATION_ID$i $grep_scfg_file | awk '{print $NF}' | sed 's/^=//' | sed 's/;$//'`
                scfg_reg="$scfg_reg""`toBig $scfg_regX | sed 's/^0x//'`"
            done
        fi
        if [ "$pon_mode" == "PON_MODE=0" ]; then
            sed -i  '/CFG_ID_PON_MAC_MODE/d' $scfg_file
            sed -i  '$a CHAR CFG_ID_PON_MAC_MODE = 0x07;' $scfg_file
            echo "setting CFG_ID_PON_MAC_MODE of scfg to Active Ethernet(0x07) ..."
        elif [ "$pon_mode" == "PON_MODE=1" ]; then
            if [ "$pon_speed" == "PON_SPEED=0" ]; then
                    sed -i  '/CFG_ID_PON_MAC_MODE/d' $scfg_file
                    sed -i  '$a CHAR CFG_ID_PON_MAC_MODE = 0x03;' $scfg_file
                    sed -i  '$a CHAR CFG_ID_PON_LASER_BURST_POLARITY = 0x00;' $scfg_file
                    echo "setting CFG_ID_PON_MAC_MODE of scfg to GPON(0x03) ..."
            elif [ "$pon_speed" == "PON_SPEED=1" ]; then
                    sed -i  '/CFG_ID_PON_MAC_MODE/d' $scfg_file
                    sed -i  '$a CHAR CFG_ID_PON_MAC_MODE = 0x04;' $scfg_file
                    echo "setting CFG_ID_PON_MAC_MODE of scfg to XGPON1(0x04) ..."
            elif [ "$pon_speed" == "PON_SPEED=2" ]; then
                    sed -i  '/CFG_ID_PON_MAC_MODE/d' $scfg_file
                    sed -i  '$a CHAR CFG_ID_PON_MAC_MODE = 0x05;' $scfg_file
                    echo "setting CFG_ID_PON_MAC_MODE of scfg to XGSPON(0x05) ..."
            else
                echo "not supporting pon_mode=$pon_mode and pon_speed=$pon_speed ..."
                return 0
            fi
            if [ "$cur_voq" != "0x0" ]; then
                sed -i  '/CFG_ID_PON_VOQ_MODE/d' $scfg_file
                sed -i  '$a CHAR CFG_ID_PON_VOQ_MODE = 0x0;' $scfg_file
                echo "setting CFG_ID_PON_VOQ_MODE of scfg to VOQ_M_8Q(0x0) ..."
            fi
        elif [ "$pon_mode" == "PON_MODE=2" ]; then
            if [ "$pon_speed" == "PON_SPEED=1" ]; then
                    sed -i  '/CFG_ID_PON_MAC_MODE/d' $scfg_file
                    sed -i  '$a CHAR CFG_ID_PON_MAC_MODE = 0x01;' $scfg_file
                    echo "setting CFG_ID_PON_MAC_MODE of scfg to EPON-D10G(0x01) ..."
            elif [ "$pon_speed" == "PON_SPEED=2" ]; then
                    sed -i  '/CFG_ID_PON_MAC_MODE/d' $scfg_file
                    sed -i  '$a CHAR CFG_ID_PON_MAC_MODE = 0x02;' $scfg_file
                    echo "setting CFG_ID_PON_MAC_MODE of scfg to EPON-BI10G(0x02) ..."
            elif [ "$pon_speed" == "PON_SPEED=0" ]; then
                    sed -i  '/CFG_ID_PON_MAC_MODE/d' $scfg_file
                    sed -i  '$a CHAR CFG_ID_PON_MAC_MODE = 0x00;' $scfg_file
                    sed -i  '$a CHAR CFG_ID_PON_LASER_BURST_POLARITY = 0x00;' $scfg_file
                    echo "setting CFG_ID_PON_MAC_MODE of scfg to EPON-1G(0x0) ..."
            else
                echo "not supporting pon_mode=$pon_mode and pon_speed=$pon_speed ..."
                return 0
            fi
            sed -i  '/CFG_ID_EPON_ENCRYPTION_MODE/d' $scfg_file
            sed -i  '$a INT CFG_ID_EPON_ENCRYPTION_MODE = 0x04;' $scfg_file
            if [ "$cur_voq" != "0x0" ]; then
                sed -i  '/CFG_ID_PON_VOQ_MODE/d' $scfg_file
                sed -i  '$a CHAR CFG_ID_PON_VOQ_MODE = 0x0;' $scfg_file
                echo "setting CFG_ID_PON_VOQ_MODE of scfg to VOQ_M_8Q(0x0) ..."
            fi
        else
            echo "not supporting pon_mode=$pon_mode ..."
            return 0
        fi
    else
        echo "Not exist $scfg_file!! Please check it!!"
        return 0
    fi
    return 1
}

ca_rtk_pon_set_mode() {
    if [ "$scfg_pon_mode" == "0xFF" ]; then
        if [ "$pon_speed" == "PON_SPEED=0" ]; then
            rt_pon_speed=0
        elif [ "$pon_speed" == "PON_SPEED=1" ]; then
            rt_pon_speed=1
        elif [ "$pon_speed" == "PON_SPEED=2" ]; then
            rt_pon_speed=2
        fi
        if [ "$pon_mode" == "PON_MODE=1" ] ; then
            rt_pon_mode=0
        elif [ "$pon_mode" == "PON_MODE=2" ]; then
            rt_pon_mode=1
        elif [ "$pon_mode" == "PON_MODE=0" ]; then
            rt_pon_mode=2
            rt_pon_speed=3
        fi
        echo "set RT PONMAC mode=$rt_pon_mode, speed=$rt_pon_speed"
        diag rt_ponmisc init
        if [ "$pon_ben_pol_reverse" == "PON_BEN_POL_REVERSE=1" ]; then
		echo "set RT ponmisc burstPolarityReverse state enable"
		diag rt_ponmisc set burstPolarityReverse state enable
	else
		echo "set RT ponmisc burstPolarityReverse state disable"
		diag rt_ponmisc set burstPolarityReverse state disable
	fi
        diag rt_ponmisc set mode $rt_pon_mode speed $rt_pon_speed
        return 1
    fi
    return 1
}

rtk_pon_set_scfg() {
    echo "RTL Chip............"
    return 1
}

#Calling presetting scfg
if [ -f /lib/modules/`uname -r`/ca-rtk.ko ] ; then
ca_rtk_pon_set_scfg
modprobe ca-rtk
ca_rtk_pon_set_mode
else
rtk_pon_set_scfg
fi

LOAD_MODULE=modprobe
[ -f /proc/modules ] || exit 0
[ -f /etc/modules ] || [ -d /etc/modules-load.d ] || exit 0
[ -e /sbin/modprobe ] || LOAD_MODULE=insmod

if [ ! -f /lib/modules/`uname -r`/modules.dep ]; then
    [ "$VERBOSE" != no ] && echo "Calculating module dependencies ..."
    depmod -Ae
fi

loaded_modules=" "

process_file() {
    file=$1

    (cat $file; echo; ) |
    while read module args
    do
        case "$module" in
            \#*|"") continue ;;
        esac
        [ -n "$(echo $loaded_modules | grep " $module ")" ] && continue
        [ "$VERBOSE" != no ] && echo -n "$module "
        eval "$LOAD_MODULE $module $args >/dev/null 2>&1"
        loaded_modules="${loaded_modules}${module} "
    done
}


[ "$VERBOSE" != no ] && echo -n "Loading modules: "
##### SPACC #############
if [ -f /lib/modules/`uname -r`/elppdu.ko ] ; then
	insmod /lib/modules/`uname -r`/elppdu.ko
fi
if [ -f /lib/modules/`uname -r`/elpspacc.ko ] ; then
	insmod  /lib/modules/`uname -r`/elpspacc.ko 
fi
if [ -f /lib/modules/`uname -r`/elpspacc.ko ] ; then
	insmod  /lib/modules/`uname -r`/elpspacccrypt.ko
fi
######### CA: Kernel modules ##################
[ -f /etc/modules ] && process_file /etc/modules

[ -d /etc/modules-load.d ] || exit 0

for f in /etc/modules-load.d/*.conf; do
    process_file $f
done


#######Other modules without module_dependency

if [ -f /lib/modules/`uname -r`/fc_mgr.ko ]; then
        modprobe fc_mgr
        #modprobe fc_g3_hook
fi
if [ -f /lib/modules/`uname -r`/kernel/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/fc_8198f.ko ]; then
        modprobe fc_8198f
        modprobe fc_mgr
fi
if [ -f /lib/modules/`uname -r`/kernel/drivers/net/ethernet/realtek/rtl86900/romeDriver/rg.ko ]; then
        modprobe rg
fi
if [ -f /lib/modules/`uname -r`/kernel/drivers/net/ethernet/realtek/rtl86900/sdk/src/module/ext_phy_polling/ext_phy_polling.ko ]; then
        $LOAD_MODULE /lib/modules/`uname -r`/kernel/drivers/net/ethernet/realtek/rtl86900/sdk/src/module/ext_phy_polling/ext_phy_polling.ko
fi
##### Wifi #############
if [ -f /lib/modules/`uname -r`/kernel/drivers/net/wireless/realtek/rtl8192cd/rtl8192cd.ko ]; then
        modprobe rg
        modprobe rtl8192cd
fi
if [ -f /lib/modules/`uname -r`/kernel/drivers/net/wireless/realtek/rtl8192cd-link/rtl8192cd.ko ]; then
        modprobe rg
        modprobe rtl8192cd
fi
#if [ -f /etc/scripts/wifi_modutils.sh ]; then
#	/etc/scripts/wifi_modutils.sh
#fi
##### RTL8226B #############
if [ -f /lib/modules/`uname -r`/kernel/drivers/net/ethernet/realtek/rtl8226B/rtl8226b.ko ]; then
        modprobe rtl8226b
fi
##### RTL8367 #############
if [ -f /lib/modules/`uname -r`/kernel/drivers/net/ethernet/realtek/rtl8367/rtl8367.ko ]; then
        modprobe rtl8367
fi

##### RTL8261 #############
if [ -f /lib/modules/`uname -r`/kernel/drivers/net/ethernet/realtek/rtl82xx/rtl8261.ko ]; then
        modprobe rtl8261
fi

if [ -f /etc/conf/compat.ko ]; then
#        modprobe rg
        insmod /etc/conf/compat.ko
fi
if [ -f /etc/conf/cfg80211.ko ]; then
#        modprobe rg
        insmod /etc/conf/cfg80211.ko
fi

if [ -e /etc/scripts/g6_drv_inst.sh ]; then
	. g6_drv_inst.sh
fi

if [ -f /lib/modules/`uname -r`/extra/drivers/net/wireless/realtek/rtl8192fe/rtl8192cd.ko ]; then
	modprobe rtl8192cd
fi

if [ -f /etc/conf/8821cu.ko ]; then
	insmod /etc/conf/8821cu.ko rtw_dfs_region_domain=3
fi
if [ -f /etc/conf/8723fu.ko ]; then
        insmod etc/conf/8723fu.ko  rtw_dfs_region_domain=3
fi

[ "$VERBOSE" != no ] && echo

exit 0
