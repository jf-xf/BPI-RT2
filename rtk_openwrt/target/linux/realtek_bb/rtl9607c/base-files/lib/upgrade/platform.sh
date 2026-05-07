PART_NAME=linux
RAMDISK_SWITCH=1
NO_LDD_SUPPORT=1

platform_check_image() {
	[ "$ARGC" -gt 1 ] && return 1

	img=$1
	echo "Image name is $img"
	tmp_dir="/tmp/fwu"
	sentinel="fwu.sh"
	sentinel_gz="fwu.sh.gz"
	md5_cmp="md5.txt"
	md5_cmd="/usr/bin/md5sum"
	md5_tmp="md5_tmp"
	md5_rt_result="md5_rt_result.txt"
	if [ -d $tmp_dir ]; then
		rm -rf $tmp_dir
	fi
	mkdir -p $tmp_dir

	cd $tmp_dir
	tar -xf $img $sentinel_gz $md5_cmp
	if [ $? != 0 ]; then
		echo tar -xf $img $sentinel $md5_cmp
		tar -xf $img $sentinel $md5_cmp
	else
		gzip -d $sentinel_gz
	fi
	if [ $? != 0 ]; then
		echo "Failed to extract $img, aborted image updating !"
		return 1
	fi

	grep $sentinel $md5_cmp > $md5_tmp
	$md5_cmd $sentinel > $md5_rt_result
	diff $md5_rt_result $md5_tmp

	if [ $? != 0 ]; then
			echo "sentinel md5_sum inconsistent, aborted image updating !"
			return 1
	fi

}

platform_do_upgrade() {
	NV=/bin/nv
	TAIL=/usr/bin/tail
	SED=/bin/sed
	tmp_dir="/tmp/fwu"
	img=/tmp/firmware.bin
	lowmem=0

	cd $tmp_dir
	active_img=`$NV getenv sw_active | $TAIL -n 1 | $SED 's/^.*=//g'`
	if [ $active_img == 0 ]; then
		bak_img=1
	elif [ $active_img == 1 ]; then
		bak_img=0
	else
		echo "ERROR: incorrect sw_active flag, should be 0 or 1 but got $active_img"
		return 1
	fi
	echo "Update image $bak_img!"
	sentinel="fwu.sh"

	sh $sentinel $bak_img $img $lowmem
	if [ $? != 0 ]; then
		echo "Incorrect image, aborted image updating !"
		exit 1
	fi
	echo ---- update image $bak_img ----
	echo "set sw_tryactive $bak_img"
	$NV setenv sw_tryactive $bak_img
	echo "set sw_updater web"
	$NV setenv sw_updater web
}

export RAMFS_COPY_BIN='/usr/bin/diff /bin/nv /usr/bin/expr /usr/sbin/ubinfo /usr/bin/tail'
