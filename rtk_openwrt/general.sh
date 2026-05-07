#!/bin/sh

check_product_dir() {
	for dir in "$@"
	do
		#echo "${dir}"
		if [ -f "${VENDORDIR}/${dir}/${CONFDIR}/${PRECONFIG}/config_user" ]; then
			PRODUCTDIR=${dir}
			return 0
		fi
	done
	return 1
}

PRECONFIG=$1
echo "OpenWrt version : ${OPENWRT_VERSION}"
#TOPDIR=$(pwd)
RTK_OPENWRT_DIR=${TOPDIR}/rtk_openwrt
VENDORDIR=${RTK_OPENWRT_DIR}/vendor

if [ "${OPENWRT_VERSION}" = "22.03" ] ; then
	echo "Add realtek_bb target for OpenWrt ${OPENWRT_VERSION}"
	TARGET_LINUX_REALTEK_DIR=target/linux/realtek_bb
	CONFDIR="conf2203"
elif [ "${OPENWRT_VERSION}" = "19.07" ] ; then
	echo "Add realtek target for OpenWrt ${OPENWRT_VERSION}"
	TARGET_LINUX_REALTEK_DIR=target/linux/realtek
	CONFDIR="conf1907"
else
	echo "Error to get OpenWrt version, Please use make preconfig_XXXX"
	exit 1
fi

PRODUCTDIR=""
cd ${VENDORDIR}
check_product_dir `ls`
cd - >/dev/null 2>&1

if [ "${PRODUCTDIR}" = "" ]; then
	echo "Error found preconfig '${PRECONFIG}'"
	exit 1
fi

rm -rf ${TOPDIR}/preconfig_*_loaded
#mkdir -p ${TOPDIR}/preconfig
#cp -rf ${VENDORDIR}/${CONFDIR}/${PRECONFIG}/* ${TOPDIR}/preconfig
rm -rf ${TOPDIR}/preconfig
ln -sf ${VENDORDIR}/${PRODUCTDIR}/${CONFDIR}/${PRECONFIG} ${TOPDIR}/preconfig

PACKAGE_KERNEL_REALTEK_DIR=package/kernel/realtek
PACKAGE_REALTEK_DIR=package/realtek
PACKAGE_CUSTOMIZE_SCFG=ca_packages/src/ca-scfg/1.0-r0/ca-scfg-1.0/customize_scfg.txt

ln -fsn ${RTK_OPENWRT_DIR}/${TARGET_LINUX_REALTEK_DIR}/ ${TOPDIR}/${TARGET_LINUX_REALTEK_DIR}
ln -fsn ${RTK_OPENWRT_DIR}/${PACKAGE_KERNEL_REALTEK_DIR}/ ${TOPDIR}/${PACKAGE_KERNEL_REALTEK_DIR}
ln -fsn ${RTK_OPENWRT_DIR}/${PACKAGE_REALTEK_DIR}/ ${TOPDIR}/${PACKAGE_REALTEK_DIR}
if [ -f ${TOPDIR}/preconfig/customize_scfg.txt ]; then
	cp ${TOPDIR}/preconfig/customize_scfg.txt ${RTK_OPENWRT_DIR}/${PACKAGE_KERNEL_REALTEK_DIR}/${PACKAGE_CUSTOMIZE_SCFG}
else
	rm ${RTK_OPENWRT_DIR}/${PACKAGE_KERNEL_REALTEK_DIR}/${PACKAGE_CUSTOMIZE_SCFG}
fi

${TOPDIR}/scripts/feeds uninstall -a
${TOPDIR}/scripts/feeds update -a

FEEDS_PATCHES=${TOPDIR}/feeds_patches
FEEDS=${TOPDIR}/feeds
if [ -d ${FEEDS_PATCHES} ]; then
	for pdir in `ls ${FEEDS_PATCHES}`
	do
		if [ -d ${FEEDS}/${pdir} -a -d ${FEEDS_PATCHES}/${pdir}/patches ]; then
			if [ ! -f ${FEEDS}/${pdir}/.patches ]; then
				echo "===> patch ${pdir} ..."
				ln -sf ${FEEDS_PATCHES}/${pdir}/patches ${FEEDS}/${pdir}/
				cd ${FEEDS}/${pdir}
				echo -n "" > series
				for i in `ls patches/*.patch`
				do 
					b=`basename $i`
					echo ${b} >> series
				done
				quilt push -a || exit 1; 
				touch .patches
				cd -
			fi	
		fi
	done
fi

${TOPDIR}/scripts/feeds install -a -f


## after feeds update, sync config 
cp -f ${TOPDIR}/preconfig/config_user ${TOPDIR}/.config

test -f ${TARGET_LINUX_REALTEK_DIR}/config-default && rm ${TARGET_LINUX_REALTEK_DIR}/config-default
test -f ${TOPDIR}/preconfig/config_kernel && cp -f ${TOPDIR}/preconfig/config_kernel ${TARGET_LINUX_REALTEK_DIR}/config-default
test -f ${TOPDIR}/preconfig/scfg.txt && cp -f ${TOPDIR}/preconfig/scfg.txt ${PACKAGE_KERNEL_REALTEK_DIR}/ca_packages/src/ca-scfg/1.0-r0/ca-scfg-1.0/customize_scfg.txt

# auto sync g6 Kconfig to Config.in
if [ -e rtk_openwrt/package/kernel/realtek/rtk_wifi6_ap ]; then
	touch rtk_openwrt/package/kernel/realtek/rtk_wifi6_ap/Kconfig.openwrt
fi

if [ -f preconfig/auto_gen_g6_config.sh ]; then
	./preconfig/auto_gen_g6_config.sh
fi

## last cmd should always success
touch ${TOPDIR}/preconfig_${PRECONFIG}_loaded
