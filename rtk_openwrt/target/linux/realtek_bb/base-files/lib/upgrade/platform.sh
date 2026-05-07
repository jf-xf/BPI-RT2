#!/bin/sh

# This is the top-level folder for the information of a firmware bank.
# Information is stored sysfs-style, in /tmp/fwbank/<bankid>/<attribute> files.
tmp_dir=/tmp/fwu

# The status field can have one of the following values.
# Note that these values map directly to the "Status" field of the TR-181
# Device.DeviceInfo.FirmwareImage.{i} object.
# https://usp-data-models.broadband-forum.org/tr-181-2-14-0-usp.html#D.Device:2.Device.DeviceInfo.FirmwareImage.{i}.
# - NoImage: The image bank is empty.
# - Active: This image bank is the one we are booted into right now.
# - Downloading: An image is being downloaded for this bank.
# - Validating: The downloaded image for this bank is being validated.
# - Available: The image for this bank has been downloaded, validated
#   and installed, and is ready to be activated.
# - DownloadFailed: Image download failed for this bank.
# - ValidationFailed: Downloaded image validation failed for this bank.
# - InstallationFailed: Validated image install failed for this bank.
# - ActivationFailed: Image activation failed for this bank.

# Name of the file, used for caching the status field of
# the firmware bank in this fwbank implementation.
# Stored in $iop_bank_prefix/<bankid>/.
rtk_status_file=status

sentinel="fwu.sh"
sentinel_gz="fwu.sh.gz"
md5_cmp="md5.txt"
md5_cmd="md5sum"
md5_tmp="md5_tmp" 
md5_rt_result="md5_rt_result.txt"
diff_cmd="diff"

tar_img_idx="tar_img_idx"

# extra programs for temporary ramfs root
RAMFS_COPY_BIN="diff nv ubinfo"
RAMFS_COPY_DATA="/etc/openwrt_version"
export RAMFS_COPY_BIN RAMFS_COPY_DATA

# Description:
#   call this function when file download finish.
# Return Value:
#   0: Success
#   1: Failure
platform_check_image() {
	case "$1" in
		/*)
			img="$1"
			;;
		*)
			img="`pwd`/$1"
			;;
	esac
	echo "img=$img"
	
	# Stop this script upon any error
	#set -e

	if [ -d $tmp_dir ]; then
		rm -rf $tmp_dir
	fi
	mkdir -p $tmp_dir

	cd $tmp_dir 
	tar -xf $img $sentinel $md5_cmp
	if [ $? != 0 ]; then
		tar -xf $img $sentinel_gz $md5_cmp
		if [ $? != 0 ]; then
			echo "Failed to extract $img, aborted image updating !"
			return 1
		else
			gzip -d $sentinel_gz
			if [ $? != 0 ]; then
				echo "$sentinel NOT found, aborted image updating !"
				return 1;
			fi
		fi
	fi

	grep $sentinel $md5_cmp > $md5_tmp
	$md5_cmd $sentinel > $md5_rt_result
	$diff_cmd $md5_rt_result $md5_tmp

	if [ $? != 0 ]; then 
	    echo "$sentinel md5_sum inconsistent, aborted image updating !"
	    return 1
	fi
}

# Before switch to ramfs
# Return Value:
#   0: Success
#   1: Failure
platform_pre_upgrade() {
	# For dual image, if active is '0' now, then '1' should be upgraded
	#nv getenv sw_active | awk -F'=' '{x=1-$2}END{print x}' > $tmp_dir/$tar_img_idx
	echo $(rtk_get_nv_env sw_active) | awk -F'=' '{x=1-$2}END{print x}' > $tmp_dir/$tar_img_idx
}

# Description:
#   Switch ramfs, and starting flash image
# Return Value:
#   0: Success
#   1: Failure
platform_do_upgrade() {
	partition=`cat $tmp_dir/$tar_img_idx`
	case "$1" in
		/*)
			img="$1"
			;;
		*)
			img="`pwd`/$1"
			;;
	esac
	echo "img=$img will flash to partition $partition."

	lowmem=0
#	if [ $# = 3 ]; then
#		lowmem=$3
#	fi

	# Run firmware upgrade script extracted from image tar ball
	sh $tmp_dir/$sentinel $partition $img $lowmem
	if [ $? != 0 ]; then
		echo "Incorrect image, aborted image updating !"
		return 1
	fi

	return 0
}

# Return Value:
#	0: Success
#	1: Failure
platform_copy_config() {
	return 0
}

# Get current bank id
platform_get_current_bank_id() {
	#echo -n `nv getenv sw_active | sed 's/^.*=//g'`
	echo -n `echo $(rtk_get_nv_env sw_active) | sed 's/^.*=//g'`
}

# Get next bank id
platform_get_next_bank_id() {
	# For dual image, if active is '0' now, then '1' should be upgraded
	#echo -n `nv getenv sw_active | awk -F'=' '{x=1-$2}END{print x}'`
	echo -n `echo $(rtk_get_nv_env sw_active) | awk -F'=' '{x=1-$2}END{print x}'`
}

# Tell the bootloader to boot from flash "bank" $1 next time.
# $1: bank identifier (0 or 1)
platform_set_bootbank() {
	local bank_id="$1"
	case "$bank_id" in
	0|1)
		rtk_set_nv_env sw_tryactive $bank_id
		#nv setenv sw_tryactive $bank_id
		v "Set sw_tryactive to $bank_id"
		;;
	*)
		v "Illegal bank id: $bank_id"
		return 1
		;;
	esac;
#	status=$?
#    	if [ $status -eq 0 ]; then
#		echo -n "$1" > /var/run/bcm_bootbank || return 1
#		return 0
#    	else
#		return 1
#    	fi

	return 0
}

# Return what "bank" the bootloader will select next time.
platform_get_bootbank() {
	#local sw_tryactive=`nv getenv sw_tryactive | sed 's/^.*=//g'`
	local sw_tryactive=`echo $(rtk_get_nv_env sw_tryactive) | sed 's/^.*=//g'`
	case "$sw_tryactive" in
	0|1)
		echo -n $sw_tryactive
		;;
	2)
		# Normal Case
		#echo -n `nv getenv sw_active | sed 's/^.*=//g'`
		echo -n `echo $(rtk_get_nv_env sw_active) | sed 's/^.*=//g'`
		;;
	*)
		v "Illegal sw_tryactive: $sw_tryactive"
		echo -n $sw_tryactive
		return 1
		;;
	esac

	return 0
}

rtk_get_nv_env(){
	[ $# -ne 1 ] && return 1
	nv getenv $1
	return 0
}

rtk_set_nv_env(){
	[ $# -ne 2 ] && return 1
	nv setenv $1 $2
	v "$(rtk_get_nv_env $1)"
	return 0
}

# Set the status for a given FW bank.
# $1 is the bank id. (0 or 1)
# $2 is the desired state to be set.
rtk_set_fwbank_status()
{
	local fwbank_dir="$tmp_dir/$1"

	[ ! -d "$fwbank_dir" ] && mkdir -p "$fwbank_dir"

	# clear version-cache on status-change
	#local iop_version_cached="$fwbank_dir/$iop_version_file"
	#rm -f "${iop_version_cached}"*

	local status_file="$fwbank_dir/$rtk_status_file"
	echo "$2" > "$status_file"
	v "`cat $status_file`"

	return $?
}

# If there is no existing status file for the given
# bank, then either return "Active" or "Available",
# depending on if we're booted in the bank or not
_get_default_fwbank_status()
{
	if [ "$1" -eq "$(platform_get_current_bank_id)" ]; then
		rtk_set_fwbank_status "$1" "Active"
	elif [ "$1" -eq "$(platform_get_next_bank_id)" ]; then
		rtk_set_fwbank_status "$1" "Available"
	fi
}

# Return a given FW bank's status.
# $1 is the bank id. (0 or 1)
rtk_get_fwbank_status()
{
	local fwbank_dir="$tmp_dir/$1"

	local status_file="$fwbank_dir/$rtk_status_file"
	if [ ! -f "$status_file" ]; then
		_get_default_fwbank_status "$1"
	fi

	cat "$status_file"

	return $?
}

