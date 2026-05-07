#!/bin/bash

set -e

STR_IMG_SIGN_ATTRS=
STR_IMG_CIPHER_ATTRS=
STR_CONF_ATTRS=
# assuming the following ENV is defined
REQUIRED_VARS=(TOPDIR LINUX_DIR LINUX_KARCH STAGING_DIR_HOST DTS_PREFIX DTS_COMP_NAME CONFIG_UBOOT_SIGNATURE_FIT CONFIG_UBOOT_SIGN_PSS CONFIG_UBOOT_CIPHER)

ROOTDIR=${TOPDIR}
ARCH=${LINUX_KARCH}

DTSDIR=${ROOTDIR}/preconfig
KERNNAME=Image
ITSNAME=uImage.its
TMPDATADIR=${ROOTDIR}/images/.fit
KEYDIR=${BOOTLOADER_DIR}/Verified_Boot/UBOOT_FIP_PRI_KEY/

#dependent tools
LZMA_TOOL=${STAGING_DIR_HOST}/bin/lzma

#MKIMG=${BOOTLOADER_DIR}/image/mkimage
if [ -f ${BOOTLOADER_DIR}/image/mkimage ]; then
MKIMG=${BOOTLOADER_DIR}/image/mkimage
elif [ -f ${TOOLS_DIR}/uboot_tools/mkimage ]; then
MKIMG=${TOOLS_DIR}/uboot_tools/mkimage
else
MKIMG=${STAGING_DIR_HOST}/bin/mkimage
fi

check_requirement() {
	for v in "${REQUIRED_VARS[@]}"; do
		if [ -z ${!v+x} ]; then
			echo "Required env '$v' is not set";
			exit -1
		fi
		echo "${v}=${!v}"
	done
}

usage() {
	echo "USAGE:";
}

create_key_and_cert() {
	echo "Create KEY pair and Sign";
	openssl genpkey -algorithm RSA -out ${TMPDATADIR}/dev.key \
    -pkeyopt rsa_keygen_bits:2048 -pkeyopt rsa_keygen_pubexp:65537
	openssl req -batch -new -x509 -key ${TMPDATADIR}/dev.key -out ${TMPDATADIR}/dev.crt
}

build_template() {
	KERNEL_LOAD=0x0
	KERNEL_ENTRY=0x0
	ITS_FILE=${TMPDATADIR}/${ITSNAME}
	cat >${ITS_FILE} <<EOF
/dts-v1/;
/ {
    description = "fitImage for RTK aarch64 kernel";
    #address-cells = <0x1>;
    images {
        description = "Kernel binary";
        kernel {
            description = "Linux Kernel";
            data = /incbin/("./${KERNNAME}.lzma");
            type = "kernel";
            arch = "${ARCH}";
            os = "linux";
            compression = "lzma";
            load = <${KERNEL_LOAD}>;
            entry = <${KERNEL_ENTRY}>;
            ${STR_IMG_SIGN_ATTRS}
            ${STR_IMG_CIPHER_ATTRS}
        };
EOF

	DTS=$1
	for i in "${DTS[@]}"
	do
		NAME=$(basename $i) && CONF=${NAME#dts_}
		cat >> ${ITS_FILE} <<EOF
        fdt-${CONF} {
            description = "FDT blob";
            data = /incbin/("./${NAME}.dtb.lzma");
            type = "flat_dt";
            load = <0x01800000>;
            arch = "${ARCH}";
            compression = "lzma";
            ${STR_IMG_SIGN_ATTRS}
            ${STR_IMG_CIPHER_ATTRS}
        };
EOF
	done

cat >> ${ITS_FILE} <<EOF
        script {
            description = "script";
            data = /incbin/("./uboot_verity_script");
            type = "script";
            compression = "none";
            ${STR_IMG_SIGN_ATTRS}
        };
EOF

	DEFAULT=$(readlink ${DTSDIR}/dts) && CONF=${DEFAULT#dts_}
	[ ! -z "$DEFAULT" ] && DEFAULT="default = \"conf-${CONF}\";"
	[ -z "$DEFAULT" ] && DEFAULT="default = \"conf-dts\";"
	cat >> ${ITS_FILE} <<EOF
    };
    configurations {
        ${DEFAULT}
EOF

	DTS=$1
	for i in "${DTS[@]}"
	do
		NAME=$(basename $i) && CONF=${NAME#dts_}
		cat >> ${ITS_FILE} <<EOF
        conf-${CONF} {
            description = "FDT blob for ${CONF}";
            kernel = "kernel";
            fdt = "fdt-${CONF}";
            script = "script";
            compatible = "${DTS_COMP_NAME}";
            ${STR_CONF_ATTRS}
        };
EOF
	done

	cat >> ${ITS_FILE} <<EOF
    };
};
EOF

}

build_dtb() {
	DTS=$1
	for i in "${DTS[@]}"
	do
		echo "Processing $i"
		NAME=$(basename $i)
		cp -a $i/* ${LINUX_DIR}/arch/${ARCH}/boot/dts/realtek/
		cd ${LINUX_DIR}
		gcc -E -Wp,-MMD,arch/${ARCH}/boot/dts/realtek/.${DTS_PREFIX}-engboard.dtb.d.pre.tmp -nostdinc -I./scripts/dtc/include-prefixes -undef -D__DTS__ -x assembler-with-cpp -o arch/${ARCH}/boot/dts/realtek/.${DTS_PREFIX}-engboard.dtb.dts.tmp arch/${ARCH}/boot/dts/realtek/${DTS_PREFIX}-engboard.dts
		./scripts/dtc/dtc -O dtb -o ${TMPDATADIR}/${NAME}.dtb -b 0 -iarch/${ARCH}/boot/dts/realtek/ -i./scripts/dtc/include-prefixes -Wno-interrupt_provider -Wno-unit_address_vs_reg -Wno-unit_address_format -Wno-avoid_unnecessary_addr_size -Wno-alias_paths -Wno-graph_child_address -Wno-simple_bus_reg -Wno-unique_unit_address -Wno-pci_device_reg arch/${ARCH}/boot/dts/realtek/.${DTS_PREFIX}-engboard.dtb.dts.tmp
		#${LZMA_TOOL} -f ${TMPDATADIR}/${NAME}.dtb
		${LZMA_TOOL} e ${TMPDATADIR}/${NAME}.dtb ${TMPDATADIR}/${NAME}.dtb.lzma
		cd -
	done
}

# $1 filename $2 size-to-align
pad_to_align() {
	filesize=$(stat -c "%s" $1)
	m=$(($filesize & ($2 - 1)))
	if [ "$m" -ne 0 ]; then
		padcount=$(($2 - $m))
		echo "Current ${filesize}, will pad ${padcount}"
		dd if=/dev/zero ibs=1 count="${padcount}" >> $1
	fi
}

# Main
check_requirement
mkdir -p ${TMPDATADIR}

if [ "$#" -ge 1 ]; then
	case "$1" in
		create_key)
			create_key_and_cert
			;;
		*)
			echo "unknown action: $1"
	esac
else
	if [ -L ${DTSDIR}/dts ]; then
		echo "Multiple DTS"
		DTS=($(ls -d ${DTSDIR}/dts_*))
		build_dtb $DTS
	else
		echo "Single DTS"
		cp ${LINUX_DIR}/arch/${ARCH}/boot/dts/realtek/${DTS_PREFIX}-engboard.dtb ${TMPDATADIR}/dts.dtb
		#${LZMA_TOOL} -f ${TMPDATADIR}/dts.dtb
		${LZMA_TOOL} e ${TMPDATADIR}/dts.dtb ${TMPDATADIR}/dts.dtb.lzma
		DTS=($(ls -d ${DTSDIR}/dts))
	fi
	[ "${CONFIG_UBOOT_SIGN_PSS}" == "y" ] && STR_IMG_SIGN_PSS="padding =\"pss\";"
	if [ "${CONFIG_UBOOT_SIGNATURE_FIT}" == "y" ]; then
		STR_IMG_SIGN_ATTRS=$(cat <<END
hash {
                algo = "sha256";
            };
            signature {
                algo = "sha256,rsa2048";
                key-name-hint = "dev";
                ${STR_IMG_SIGN_PSS}
            };
END
)
		STR_CONF_ATTRS+=$(cat <<END
hash {
                algo = "sha256";
            };
            signature {
                algo = "sha256,rsa2048";
                key-name-hint = "dev";
                sign-images = "fdt", "kernel", "script";
                ${STR_IMG_SIGN_PSS}
            };
END
)
		[ "${CONFIG_UBOOT_CIPHER}" == "y" ] && STR_IMG_CIPHER_ATTRS+=$(cat <<END

                cipher {
                algo = "aes256";
                key-name-hint = "DATA_KEY";
                iv-name-hint = "DATA_IV";
            };
END
)
	fi

	if [ "${CONFIG_UBOOT_SIGNATURE_FIT}" == "y" ];then
		cd ${TMPDATADIR}
		rm -f UBOOT_FIP_PRI_KEY
		ln -sf ${BOOTLOADER_DIR}/Verified_Boot/UBOOT_FIP_PRI_KEY UBOOT_FIP_PRI_KEY
		cd -
	fi

	cd ${TMPDATADIR}
	rm -f uboot_verity_script
	ln -sf ${ROOTDIR}/images/uboot_verity_script uboot_verity_script
	cd -

	echo "Generate template..."
	build_template $DTS
	echo "Compress ${KERNNAME}..."
	ln -sf ${LINUX_DIR}/arch/${ARCH}/boot/${KERNNAME} ${TMPDATADIR}/${KERNNAME}
	#echo "${LZMA_TOOL} -f -k ${TMPDATADIR}/${KERNNAME}"
	#${LZMA_TOOL} -f -k ${TMPDATADIR}/${KERNNAME}
	echo "${LZMA_TOOL} e ${TMPDATADIR}/${KERNNAME} ${TMPDATADIR}/${KERNNAME}.lzma"
	${LZMA_TOOL} e ${TMPDATADIR}/${KERNNAME} ${TMPDATADIR}/${KERNNAME}.lzma
	cd ${TMPDATADIR}
	echo "${MKIMG} -f ${ITSNAME} uImage"
	${MKIMG} -f ${ITSNAME} uImage

	echo "${MKIMG} -F -k UBOOT_FIP_PRI_KEY -r uImage"
	[ "${CONFIG_UBOOT_SIGNATURE_FIT}" == "y" ] && \
		echo "Signing..." && ${MKIMG} -F -k UBOOT_FIP_PRI_KEY -r uImage
	cd -
	mv ${TMPDATADIR}/uImage ${ROOTDIR}/images/
	echo "Done"
fi
