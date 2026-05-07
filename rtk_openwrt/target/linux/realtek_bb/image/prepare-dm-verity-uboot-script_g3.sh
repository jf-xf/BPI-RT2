#!/bin/bash

while read line; do
	key=$(echo ${line} | cut -f1 -d':')
	value=$(echo ${line} | cut -f2 -d':')

	case "${key}" in
	"UUID")
		UUID=${value}
		;;
	"Data blocks")
		DATA_BLOCKS=${value}
		;;
	"Data block size")
		DATA_BLOCK_SIZE=${value}
		;;
	"Hash block size")
		HASH_BLOCK_SIZE=${value}
		;;
	"Hash algorithm")
		HASH_ALG=${value}
		;;
	"Salt")
		SALT=${value}
		;;
	"Root hash")
		ROOT_HASH=${value}
		;;
	esac
done

rfs_key=${BOOTLOADER_DIR}/Verified_Boot/UBOOT_FIP_PRI_KEY/RFS_DATA_KEY.bin
rfs=${IMAGEDIR}/rootfs
rfs_encrypted=${IMAGEDIR}/rootfs_encrypted
rfs_unencrypted=${IMAGEDIR}/rootfs_unencrypted
cipher=aes-xts-plain64
is_encrypt=`cat ${LINUX_DIR}/.config | grep "CONFIG_DM_CRYPT=" | sed s/[^y]//g | sed s/\"//g`
rfs_sectors=""
dmkey=""
chk_and_create_key(){
	[ ! -f ${rfs_key} ] && dd if=/dev/random  of=${rfs_key} bs=1 count=32
	[ ! -f ${rfs_key} ] && echo "Key file not exit"  && exit 1;
}

do_rootfs_encrypt()
{
	chk_and_create_key
	${TOOLS_DIR}/rootfs_encrypt ${rfs} ${rfs_encrypted} ${rfs_key}
	if [ "$?" = 1 ]; then
		echo "ERROR: Encrypt rootfs Error!"
		exit 1
	fi
	mv ${rfs} ${rfs_unencrypted}
	mv ${rfs_encrypted} ${rfs}
	rfs_size=$(stat -c "%s" ${rfs})
	rfs_sectors=$(awk -v size=$rfs_size 'BEGIN{print size/512}')
	dmkey=$(xxd -p ${rfs_key} | xargs | sed s/[[:space:]]//g)
	echo setenv blk_num ${rfs_sectors}
	echo setenv cipher ${cipher}
	echo 'setenv crypto_root ${root_mtd}'
	echo setenv dmkey ${dmkey}
}
if [ "$is_encrypt" = "y" ]; then
	do_rootfs_encrypt
fi

SECTORS=$((${DATA_BLOCKS} * 8))

echo setenv verity_sectors $((${DATA_BLOCKS} * 8))
echo setenv verity_data_blocks ${DATA_BLOCKS}
echo setenv verity_hash_start $((${DATA_BLOCKS} + 1))
echo setenv verity_data_block_sz ${DATA_BLOCK_SIZE}
echo setenv verity_hash_block_sz ${HASH_BLOCK_SIZE}
echo setenv verity_hash_alg ${HASH_ALG}
echo setenv verity_salt ${SALT}
echo setenv verity_root_hash ${ROOT_HASH}
echo 'setenv rootdev "${root_mtd}"'
echo 'setenv "root_mtd" "/dev/dm-0"'
echo run setmoreargs
if [ "$is_encrypt" = "y" ]; then
	echo 'setenv rootdev "/dev/dm-1"'
	echo setenv bootargs '"${basicargs} ${more_args} ${ramargs} ${mtdparts} dm-mod.create=\"dm-crypt,,1,ro,0 ${blk_num} crypt ${cipher} ${dmkey} 0 ${crypto_root} 0 1 sector_size:2048;dm-verity,,0,ro,0 ${verity_sectors} verity 1 ${rootdev} ${rootdev} ${verity_data_block_sz} ${verity_hash_block_sz} ${verity_data_blocks} ${verity_hash_start} ${verity_hash_alg} ${verity_root_hash} ${verity_salt}\""
if test -n ${b_conf} ; then bootm ${unzip_src}#${b_conf} ; else bootm ${unzip_src} ; fi'
else
echo setenv bootargs '"${basicargs} ${more_args} ${ramargs} ${mtdparts} dm-mod.create=\"dm-verity,,0,ro,0 ${verity_sectors} verity 1 ${rootdev} ${rootdev} ${verity_data_block_sz} ${verity_hash_block_sz} ${verity_data_blocks} ${verity_hash_start} ${verity_hash_alg} ${verity_root_hash} ${verity_salt}\""
if test -n ${b_conf} ; then bootm ${unzip_src}#${b_conf} ; else bootm ${unzip_src} ; fi'
fi
