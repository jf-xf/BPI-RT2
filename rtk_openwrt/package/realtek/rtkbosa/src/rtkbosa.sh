#!/bin/sh

pon_mode=`mib get PON_MODE | grep PON_MODE`
GPON_K_FILE=/var/config/rtkbosa_gpon_k.bin
EPON_K_FILE=/var/config/rtkbosa_epon_k.bin
BOSA_K_FILE=/var/config/rtkbosa_k.bin
BOSA_K_FILE_CHECKSUM=/var/config/rtkbosa_k_checksum
BOSA_K_STATIC_FILE=/var/config_static/rtkbosa_k.bin

if [ "$pon_mode" == "PON_MODE=1" ] && [ -f $GPON_K_FILE ] ; then
	echo "rtkbosa: GPON mode and [$GPON_K_FILE] is found" > /dev/console
	rtkbosa -c $GPON_K_FILE

elif [ "$pon_mode" == "PON_MODE=2" ] && [ -f $EPON_K_FILE ] ; then
	echo "rtkbosa: EPON mode and [$EPON_K_FILE] is found"  > /dev/console
	rtkbosa -c $EPON_K_FILE

else
	echo "rtkbosa: Don’t care pon mode, use general file"  > /dev/console
	checksum=`md5sum ${BOSA_K_FILE} | grep ${BOSA_K_FILE} | sed 's/^\(.*\)[[:space:]].*$/\1/g'`
	checksumold=`cat ${BOSA_K_FILE_CHECKSUM} | sed 's/^\(.*\)[[:space:]].*$/\1/g'`
	echo "rtkbosa: checksumold = ${checksumold}" > /dev/console
	if [ -z ${checksumold} ] || [ ${checksumold}==${checksum} ] ;then
		echo "rtkbosa: Use general file [$BOSA_K_FILE]" > /dev/console
		rtkbosa -c $BOSA_K_FILE
	else
		echo "rtkbosa: [$BOSA_K_FILE] is incorrect, use static file [$BOSA_K_STATIC_FILE]" > /dev/console
		rtkbosa -c $BOSA_K_STATIC_FILE
	fi

fi

