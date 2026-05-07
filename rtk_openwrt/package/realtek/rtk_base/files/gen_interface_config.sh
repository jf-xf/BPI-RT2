#!/bin/bash

if [ -f ${TOPDIR}/.config ]; then
	. ${TOPDIR}/.config
fi

if [ -f ${LINUXDIR}/.config ]; then
	. ${LINUXDIR}/.config
fi

config_file=$1

NIC_NUM=1
NIC_PREFIX="eth"
NIC_IDX=0

LAN_NUM=3
LAN_PREFIX="eth0."
LAN_IDX=3

WAN_NUM=1
WAN_PREFIX="nas"
WAN_IDX=0

if [ ! "${CONFIG_RTK_MULTI_ETH_NIC_NAME_CUSTOMIZE}" = "" -a ! "${CONFIG_RTK_MULTI_ETH_NIC_NAME}" = "" ]; then
	NIC_PREFIX=${CONFIG_RTK_MULTI_ETH_NIC_NAME}
	NIC_IDX=${CONFIG_RTK_MULTI_ETH_NIC_NAME_IDX}
fi

if [ ! "${CONFIG_RTK_MULTI_LAN_NAME_CUSTOMIZE}" = "" -a ! "${CONFIG_RTK_MULTI_LAN_NAME}" = "" ]; then
	LAN_PREFIX=${CONFIG_RTK_MULTI_LAN_NAME}
	LAN_IDX=${CONFIG_RTK_MULTI_LAN_NAME_IDX}
fi

if [ ! "${CONFIG_LAN_PORT_NUM}" = "" ]; then
	LAN_NUM=${CONFIG_LAN_PORT_NUM}
fi

if [ ! "${CONFIG_RTK_MULTI_WAN_NAME_CUSTOMIZE}" = "" -a ! "${CONFIG_RTK_MULTI_WAN_NAME}" = "" ]; then
	WAN_PREFIX=${CONFIG_RTK_MULTI_WAN_NAME}
	WAN_IDX=${CONFIG_RTK_MULTI_WAN_NAME_IDX}
fi

function genIntfConfig
{
	local f=$1
	local intfp=$2
	local idx=$3
	local num=$4
	local vnp=$5
	local vnl=$6
	
	local i=0
	local list_intf=""
	while [ $i -lt ${num} ]
	do
		echo -en "${vnp}$((i+1))=\"${intfp}${idx}\"\n" >> ${f}
		list_intf+="\${${vnp}$((i+1))} "
		let i++
		let idx++
	done
	echo -en "${vnl}=\"${list_intf}\"\n\n" >> ${f}
}

rm ${config_file}
echo -en "#!/bin/sh\n\n" > ${config_file}

genIntfConfig ${config_file} ${NIC_PREFIX} ${NIC_IDX} ${NIC_NUM} "INTF_NIC_" "INTF_NIC_LISTS"

genIntfConfig ${config_file} ${LAN_PREFIX} ${LAN_IDX} ${LAN_NUM} "INTF_LAN_" "INTF_LAN_LISTS"

genIntfConfig ${config_file} ${WAN_PREFIX} ${WAN_IDX} ${WAN_NUM} "INTF_WAN_" "INTF_WAN_LISTS"

chmod 555 ${config_file}
