#!/bin/sh

[ -e /lib/functions.sh ] && . /lib/functions.sh || . ./functions.sh

EBTABLES=`which ebtables`
FW3_BRWAN_EB_INPUT_CHAIN="brwan_input"
FW3_BRWAN_EB_OUPUT_CHAIN="brwan_output"

# clear ebtable brwan_input chain
$EBTABLES -D INPUT -j $FW3_BRWAN_EB_INPUT_CHAIN
$EBTABLES -F $FW3_BRWAN_EB_INPUT_CHAIN
$EBTABLES -X $FW3_BRWAN_EB_INPUT_CHAIN
## add ebtable brwan_input chain
$EBTABLES -N $FW3_BRWAN_EB_INPUT_CHAIN
$EBTABLES -P $FW3_BRWAN_EB_INPUT_CHAIN RETURN
$EBTABLES -I INPUT -j $FW3_BRWAN_EB_INPUT_CHAIN
#
## clear ebtable brwan_output chain
$EBTABLES -D OUTPUT -j $FW3_BRWAN_EB_OUPUT_CHAIN
$EBTABLES -F $FW3_BRWAN_EB_OUPUT_CHAIN
$EBTABLES -X $FW3_BRWAN_EB_OUPUT_CHAIN
## add ebtable brwan_output chain
$EBTABLES -N $FW3_BRWAN_EB_OUPUT_CHAIN
$EBTABLES -P $FW3_BRWAN_EB_OUPUT_CHAIN RETURN
$EBTABLES -I OUTPUT -j $FW3_BRWAN_EB_OUPUT_CHAIN
#

setup_brwan_policy_rules(){
	config_get l2dev "$1" name
	config_get rtk_chmode "$1" rtk_chmode
	
	if [ $rtk_chmode == "Bridged" ]; then
		$EBTABLES -A $FW3_BRWAN_EB_OUPUT_CHAIN -o $l2dev -j DROP
		$EBTABLES -A $FW3_BRWAN_EB_INPUT_CHAIN -i $l2dev -j DROP
	fi
}

config_load network
config_foreach setup_brwan_policy_rules device

