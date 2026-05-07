#!/bin/sh

IMAGEDIR=$2
LINUXDIR=$3

echo "IMAGEDIR: ${IMAGEDIR}"
echo "LINUXCONFIG: ${LINUXDIR}"

is_yueme=`cat ${LINUXDIR}/.config | grep "CONFIG_YUEME=" | sed s/[^y]//g | sed s/\"//g`
is_cmcc=`cat ${LINUXDIR}/.config | grep "CONFIG_CMCC=" | sed s/[^y]//g | sed s/\"//g`
is_cu_yueme=`cat ${LINUXDIR}/.config | grep "CONFIG_CU_BASEON_YUEME=" | sed s/[^y]//g | sed s/\"//g`
is_cu_cmcc=`cat ${LINUXDIR}/.config | grep "CONFIG_CU_BASEON_CMCC=" | sed s/[^y]//g | sed s/\"//g`
extra_img=""

use_ubifs=1
if [ "$1" = "-ubifs" ]; then
	use_ubifs=0
fi

TMPDIR=${IMAGEDIR}/.tmp
rm -rf ${TMPDIR}
mkdir -p ${TMPDIR}

flash_base=0xbd000000
ImageOffsetHex=`echo "obase=16;ibase=10; $(($flash_base))" | bc`
echo "ImageOffsetHex: ${ImageOffsetHex}"

add_vmimg_header()
{
	echo "--------$1 [Key: $2]--------"
	if [ "$1" = "osgi.img" ]; then
	./genhead -i ${IMAGEDIR}/$1 -o ${TMPDIR}/$1.hdr -k $2 -f $ImageOffsetHex -a 0x80000000 -e 0x80000000
	else
	./genhead -i ${IMAGEDIR}/$1 -o ${TMPDIR}/$1.hdr -k $2 -f $ImageOffsetHex
	fi
	cat ${TMPDIR}/$1.hdr ${IMAGEDIR}/$1 > ${TMPDIR}/$1.vm
}

add_vmimg_header dtb.img 0xa0000443
add_vmimg_header Image.lzma 0xa0000203
if [ "$use_ubifs" = "1" ]; then
add_vmimg_header rootfs_G3_1.ubi 0xa0000403
else
add_vmimg_header rootfs 0xa0000403
fi

cat ${TMPDIR}/*.vm ${extra_img} > ${IMAGEDIR}/vm.img

if [ "y" = "$is_cmcc" ] || [ "y" = "$is_cu_cmcc" ]; then
	add_vmimg_header osgi.img 0xCAFEBABE
fi

#rm -rf ${TMPDIR}

