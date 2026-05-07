#!/bin/sh

IMAGEDIR=$1
IMAGE=$2

if [ ! -d ${IMAGEDIR} ];then
	echo "IMAGEDIR Not Found!"
	exit 1
fi
if [ ! -f ${IMAGEDIR}/${IMAGE} ];then
	echo "Image: ${IMAGE} Not Found!"
	exit 1
fi

echo "Backup original ${IMAGE} as ${IMAGE}.noalign"
cp ${IMAGEDIR}/${IMAGE} ${IMAGEDIR}/${IMAGE}.noalign

echo "--------${IMAGE}--------"

Image_size=`du -sb ${IMAGEDIR}/${IMAGE} | head -n 1 | awk '{ print $1 }'` ;
echo "Input File Size: ${Image_size}" ;

Block=$(( $Image_size / 262144)) ;
echo "  Block Count: ${Block}" ;

Desired_Block=$(( ${Block} + 1 )) ;
echo "  Desired Block Count: ${Desired_Block}" ;

DSIZE=$(( ${Desired_Block} * 262144)) ;
echo "  Desired Size: ${DSIZE}" ;

PaddingSize=$(( ${DSIZE} - ${Image_size} )) ;
echo "  Pading Size: ${PaddingSize}" ;

dd if=/dev/zero bs=1 count=${PaddingSize}>> ${IMAGEDIR}/${IMAGE};

Image_size=`du -sb ${IMAGEDIR}/${IMAGE} | head -n 1 | awk '{ print $1 }'` ;
echo "Output File Size: ${Image_size}" ;

echo "--------Done--------"

