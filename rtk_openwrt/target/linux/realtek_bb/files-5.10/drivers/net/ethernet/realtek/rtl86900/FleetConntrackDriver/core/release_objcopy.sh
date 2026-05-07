#!/bin/sh

#BASEDIR=$(dirname "$0")
#echo ${BASEDIR}

FC_COREFOLDER="core"
FC_ASICDRVFOLDER="asicDrv"

SDK_DIR=$(pwd | sed 's/\/linux.*//g')
LINUX_DIR=$(pwd | sed 's/\/drivers\/net.*//g')
FC_DIR=$(pwd | sed 's/\/core.*//g')

#echo SDK_DIR: ${SDK_DIR}
#echo LINUX_DIR: ${LINUX_DIR}
#echo FC_DIR: ${FC_DIR}

MODULE_STR=$(cat ${LINUX_DIR}/.config | grep 'CONFIG_RTK_L34_MODULE_STRING' | sed 's/^.*="//g' | sed 's/"//g')
MODULE_FMT=$(cat ${LINUX_DIR}/.config | grep 'CONFIG_RTK_L34_FC_KERNEL_MODULE' | sed 's/^.*=//g' | sed 's/"//g')

if [ "$MODULE_FMT" = "y" ]; then
	
	echo "====== [FC] backup core module release file ====="

	FC_CORE_RFOLDER=release_${MODULE_STR}_obj
	
	if [ -d "$LINUX_DIR" ]; then
		echo "Linux config file in ${LINUX_DIR}/"
	fi

	if [ -d "$FC_DIR/$FC_CORE_RFOLDER" ]; then
		echo "release folder: ${FC_CORE_RFOLDER} is exist"
		#echo "copy ${FC_DIR}/${FC_COREFOLDER}/*.o to ${FC_DIR}/${FC_CORE_RFOLDER}/"
	else
		echo "release folder: ${FC_CORE_RFOLDER} is not exist, let's create it."
		mkdir $FC_DIR/$FC_CORE_RFOLDER
	fi


	echo "backup core module obj files to ${FC_DIR}/${FC_CORE_RFOLDER}/..."
	for filename in ${FC_DIR}/${FC_COREFOLDER}/*.o; do
		cp -rf "$filename" "${FC_DIR}/${FC_CORE_RFOLDER}/$(basename "$filename").r"
	done
	for filename in ${FC_DIR}/${FC_COREFOLDER}/${FC_ASICDRVFOLDER}/*.o; do
		cp -rf "$filename" "${FC_DIR}/${FC_CORE_RFOLDER}/$(basename "$filename").r"
	done


else
	
	echo "====== [FC] skip obj copy because of module format ====="
fi


