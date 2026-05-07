#/bin/sh
ROOTDIR=$1
LINUX_DIR=$2
LZMA_TOOL=${STAGING_DIR_HOST}/bin/lzma
if [ -f ${BOOTLOADER_DIR}/image/mkimage ]; then
MKIMG=${BOOTLOADER_DIR}/image/mkimage
elif [ -f ${TOOLS_DIR}/uboot_tools/mkimage ]; then
MKIMG=${TOOLS_DIR}/uboot_tools/mkimage
else
MKIMG=${STAGING_DIR_HOST}/bin/mkimage
fi
DTB_B=$3
ARCH=$4
FIT_SIGNATURE=$5
CONFIG_LINUX_5_10=$6
FIT_SIGN_PSS=$7
FIT_CIPHER=$8
echo "------------------options ---------------------------"
echo " FIT_SIGNATURE                        =$FIT_SIGNATURE"
echo "  - FIT_SIGNATURE(PSS)                =$FIT_SIGN_PSS"
echo "  - FIT_SIGNATURE(PSS) + FIT_CIPHER   =$FIT_CIPHER"
echo "-----------------------------------------------------"
filepath=$(pwd)
echo "ROOTDIR: ${ROOTDIR}"
echo "LINUX_DIR: ${LINUX_DIR}"
echo "DTB_B: ${DTB_B}"
echo "ARCH: ${ARCH}"
echo "FIT_SIGNATURE: ${FIT_SIGNATURE}"
echo "CONFIG_LINUX_5_10: ${CONFIG_LINUX_5_10}"
echo "BOOTLOADER_DIR: ${BOOTLOADER_DIR}"
echo "STAGING_DIR_HOST: ${STAGING_DIR_HOST}"
echo "pwd: ${filepath}"

if [ "$ARCH" = "arm" ];then
kernel_img="zImage"
elif [ "$ARCH" = "arm64" ];then
kernel_img="Image"
else
	echo "arch error!"
	exit 1
fi

cd ${ROOTDIR}/images

if [ ! -f ${MKIMG} ] ;then
        echo "Error: Not found mkimage: ${MKIMG}"
        exit 1
fi


if [ ! -d ${LINUX_DIR}/arch/${ARCH}/boot/dts/realtek/ ];then
	echo "arch: \"${ARCH}\""
    echo "Miss ${LINUX_DIR}/arch/${ARCH}";
	exit 1
fi


if [ ! -f ${LINUX_DIR}/arch/${ARCH}/boot/dts/realtek/${DTB_B} ];then
        echo "Miss ${LINUX_DIR}/arch/${ARCH}/boot/dts/realtek/${DTB_B}";
        exit 1;
fi

if [ ! -f ${LINUX_DIR}/arch/${ARCH}/boot/${kernel_img} ];then
        echo "Miss ${LINUX_DIR}/arch/${ARCH}/boot/${kernel_img}";
        exit 1;
fi

if [ ! -d .fit_uImage ];then
	mkdir .fit_uImage
fi

cd .fit_uImage

rm -f board_raw.dtb ${kernel_img}

ln -sf ${LINUX_DIR}/arch/${ARCH}/boot/dts/realtek/${DTB_B}	board_raw.dtb

ln -sf ${LINUX_DIR}/arch/${ARCH}/boot/${kernel_img} ${kernel_img}

if [ "$FIT_SIGNATURE" = "y" ];then
rm -f uboot_verity_script UBOOT_FIP_PRI_KEY
ln -sf ${ROOTDIR}/images/uboot_verity_script uboot_verity_script
ln -sf ${BOOTLOADER_DIR}/Verified_Boot/UBOOT_FIP_PRI_KEY UBOOT_FIP_PRI_KEY
fi

if [ "$ARCH" = "arm64" ];then
	#${LZMA_TOOL} -f -k  ${kernel_img}
	${LZMA_TOOL} e ${kernel_img} ${kernel_img}.lzma
	if [ "$CONFIG_LINUX_5_10" = "y" ];then
		if [ "$FIT_SIGNATURE" = "y" ];then
			ITS_FILE=${TOOLS_DIR}/uboot_fit_signature_image_A64_1DTB_linux5.its
			if [ "$FIT_SIGN_PSS" = "y" ];then
				ITS_FILE=${TOOLS_DIR}/uboot_fit_signature_pss_image_A64_linux5.its
			fi
			if [ "$FIT_CIPHER" = "y" ];then
				ITS_FILE=${TOOLS_DIR}/uboot_fit_sign_pss_cipher_image_A64_linux5.its
			fi
			ln -sf ${ITS_FILE}  uboot_fit_image_A64_1DTB_linux5.its
			echo "Doing: ${MKIMG} -f  ${ITS_FILE}  uImage"
		else
			ln -sf ${TOOLS_DIR}/uboot_fit_image_A64_1DTB_linux5.its uboot_fit_image_A64_1DTB_linux5.its
			echo "B Doing: ${MKIMG} -f  uboot_fit_image_A64_1DTB_linux5.its uImage"
		fi
		${MKIMG} -f  uboot_fit_image_A64_1DTB_linux5.its uImage
		ret=$?
		if [ $ret -ne 0 ];then
			printf "\033[1;31m   ${MKIMG} Error! ITS FILE:${ITS_FILE} \n \033[0m";
			exit -1
		fi
	else
		if [ "$FIT_SIGNATURE" = "y" ];then
			ln -sf ${TOOLS_DIR}/uboot_fit_signature_image_A64_1DTB.its uboot_fit_image_A64_1DTB.its
			echo "C Doing: ${MKIMG} -f  uboot_fit_signature_image_A64_1DTB.its uImage"
		else
			ln -sf ${TOOLS_DIR}/uboot_fit_image_A64_1DTB.its uboot_fit_image_A64_1DTB.its
			echo "D Doing: ${MKIMG} -f  uboot_fit_image_A64_1DTB.its uImage"
		fi
		${MKIMG} -f  uboot_fit_image_A64_1DTB.its uImage
	fi
else
	if [ "$FIT_SIGNATURE" = "y" ];then
		ln -sf ${TOOLS_DIR}/uboot_fit_signature_image_A32_singleDTB.its uboot_fit_image_A32_singleDTB.its
		echo "Doing: ${MKIMG} -f  uboot_fit_signature_image_A32_singleDTB.its uImage"
	else
		ln -sf ${TOOLS_DIR}/uboot_fit_image_A32_singleDTB.its uboot_fit_image_A32_singleDTB.its 
		echo "Doing: ${MKIMG} -f  uboot_fit_image_A32_singleDTB.its uImage"
	fi
	${MKIMG} -f  uboot_fit_image_A32_singleDTB.its uImage
		ret=$?
		if [ $ret -ne 0 ];then
			printf "\033[1;31m   ${MKIMG} Error! \n \033[0m";
			exit -1
		fi
fi

if [ ! -f uImage ];then
        echo "uImage with ${DTB_B} generated failed!"
        exit 1
fi

if [ "$FIT_SIGNATURE" = "y" ];then
	echo "Doing: ${MKIMG} -F -k UBOOT_FIP_PRI_KEY  -r uImage"
	${MKIMG} -F -k UBOOT_FIP_PRI_KEY  -r uImage
		ret=$?
		if [ $ret -ne 0 ];then
			printf "\033[1;31m   ${MKIMG} Error! \n \033[0m";
			exit -1
		fi
fi

cp uImage ${ROOTDIR}/images/
cd ${ROOTDIR}/images/
echo "Use uImage instead of Image.lzma. Link Image.lzma to uImage"
rm -f Image.lzma && ln -s uImage Image.lzma

