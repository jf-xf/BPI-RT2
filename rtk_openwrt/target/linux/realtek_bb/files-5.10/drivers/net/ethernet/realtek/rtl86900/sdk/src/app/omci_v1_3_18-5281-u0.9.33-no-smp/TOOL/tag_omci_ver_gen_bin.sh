#!/bin/bash

usage()
{
	printf "Usage: \n ./tag_omci_ver_gen_bin.sh\n"
}

SVN_BIN_MK_FILE="http://cadinfo.realtek.com.tw/svn/CN/Switch/PON/public/applications/rtk_omci_release/BIN_OMCI_TEMPLATE.mak"
OMCI_PATH=$(pwd | sed 's/TOOL.*//g')
BUILD_PWD=$(pwd |sed 's/linux.*//g')
BIN_OMCI_APP=omci_app
BIN_OMCI_CLI=omcicli
cd ${OMCI_PATH}
echo ${OMCI_PATH}
OMCI_VERSION=r`svn info | grep -e Rev: | sed -e 's/.*Rev: //'`
echo ${OMCI_VERSION}
cd $BUILD_PWD

# modify release and svn revision in Makefile

RELEASE_VERSION="\\\\\"$OMCI_VERSION\\\\\""
sed -i '/^RELEASE_SUBVERSION/d' "${OMCI_PATH}/Makefile" && test 0 -ne $? && exit 1
		
if [ -z "`cat ${OMCI_PATH}/Makefile |\
	 grep "^RELEASE_VERSION" | head -n 1`" ]; then
	# Not exist RELEASE_VERSION variable
	sed -i "20 a RELEASE_VERSION\t\t\t= "${RELEASE_VERSION}""\
		 "${OMCI_PATH}/Makefile" && test 0 -ne $? && exit 1
else
	# exist RELEASE_VERSION variable
	sed -i "s/^RELEASE_VERSION.*\"$/RELEASE_VERSION\t\t\t= \
		"${RELEASE_VERSION}"/g" "${OMCI_PATH}/Makefile" && test 0 -ne $? && exit 1
fi

RTKDRV_RELEASE_VERSION="\\\\\"$OMCI_VERSION\\\\\""

# for rtk drv release version
if [ -z "`cat ${OMCI_PATH}/DRV/platform/rtl9607_rtk/Makefile |\
	 grep "^RTKDRV_RELEASE_VERSION" | head -n 1`" ]; then
	# Not exist RTKDRV_RELEASE_VERSION variable
	sed -i "3 a RTKDRV_RELEASE_VERSION\t\t\t= "${RTKDRV_RELEASE_VERSION}""\
		 "${OMCI_PATH}/DRV/platform/rtl9607_rtk/Makefile" && test 0 -ne $? && exit 1
else
	# exist RTKDRV_RELEASE_VERSION variable
	sed -i "s/^RTKDRV_RELEASE_VERSION.*\"$/RTKDRV_RELEASE_VERSION\t\t\t= \
		"${RTKDRV_RELEASE_VERSION}"/g" \
		"${OMCI_PATH}/DRV/platform/rtl9607_rtk/Makefile" && test 0 -ne $? && exit 1
fi

RGDRV_RELEASE_VERSION="\\\\\"$OMCI_VERSION\\\\\""

# for rg  drv release version
if [ -z "`cat ${OMCI_PATH}/DRV/platform/rtl9607_rg/Makefile |\
	 grep "^RGDRV_RELEASE_VERSION" | head -n 1`" ]; then
	# Not exist RTKDRV_RELEASE_VERSION variable
	sed -i "3 a RGDRV_RELEASE_VERSION\t\t\t= "${RGDRV_RELEASE_VERSION}"" \
		"${OMCI_PATH}/DRV/platform/rtl9607_rg/Makefile" && test 0 -ne $? && exit 1
else
	# exist RTKDRV_RELEASE_VERSION variable
	sed -i "s/^RGDRV_RELEASE_VERSION.*\"$/RGDRV_RELEASE_VERSION\t\t\t= \
		"${RGDRV_RELEASE_VERSION}"/g" \
		"${OMCI_PATH}/DRV/platform/rtl9607_rg/Makefile" && test 0 -ne $? && exit 1
fi


FCDRV_RELEASE_VERSION="\\\\\"$OMCI_VERSION\\\\\""

# for fc  drv release version
if [ -z "`cat ${OMCI_PATH}/DRV/platform/rtl9607_fc/Makefile |\
	 grep "^RGDRV_RELEASE_VERSION" | head -n 1`" ]; then
	# Not exist RTKDRV_RELEASE_VERSION variable
	sed -i "3 a RGDRV_RELEASE_VERSION\t\t\t= "${RGDRV_RELEASE_VERSION}"" \
		"${OMCI_PATH}/DRV/platform/rtl9607_fc/Makefile" && test 0 -ne $? && exit 1
else
	# exist RTKDRV_RELEASE_VERSION variable
	sed -i "s/^RGDRV_RELEASE_VERSION.*\"$/RGDRV_RELEASE_VERSION\t\t\t= \
		"${RGDRV_RELEASE_VERSION}"/g" \
		"${OMCI_PATH}/DRV/platform/rtl9607_fc/Makefile" && test 0 -ne $? && exit 1
fi

make user/diagshell_clean
rm -rf ${OMCI_PATH}/../bin/omci*
make user/diagshell_only


rm -rf ${OMCI_PATH}/LIB/lib*
rm -rf ${OMCI_PATH}/LIB/omci/mib*

# prepare binary OMCI
find ${OMCI_PATH} -name "*.c" | xargs -n2 rm && test 0 -ne $? && exit 1
find ${OMCI_PATH} -name "*.o" | xargs -n2 rm && test 0 -ne $? && exit 1
find ${OMCI_PATH} -name "*.d" | xargs -n2 rm && test 0 -ne $? && exit 1
rm -rf ${OMCI_PATH}/LIB/omci/*.so && test 0 -ne $? && exit 1
find ${OMCI_PATH}/DRV -name "*.cmd" | xargs -n2 rm && test 0 -ne $? && exit 1
find ${OMCI_PATH}/DRV -name "modules.order" | xargs -n2 rm && test 0 -ne $? && exit 1
find ${OMCI_PATH}/DRV -name "Module.symvers" | xargs -n2 rm && test 0 -ne $? && exit 1
find ${OMCI_PATH}/DRV -name ".tmp_versions" | xargs -n2 rm -rf
find ${OMCI_PATH}/DRV -name ".mod" | xargs -n2 rm && test 0 -ne $? && exit 1

# All binaray files will be removed if make clean from sdk path. 
#Therefore, change name of omci binaray file 
find ${OMCI_PATH} -name "*.so" | \
     sed -e p -e "s/so/so."${OMCI_VERSION#r*}.${OMCI_VERSION#r*}.${OMCI_VERSION#r*}"/g" | xargs -n2 mv && test 0 -ne $? && exit 1

find ${OMCI_PATH} -name "*.ko" | \
     sed -e p -e "s/ko/ko_"${OMCI_VERSION#r*}.${OMCI_VERSION#r*}.${OMCI_VERSION#r*}"/g" | xargs -n2 mv && test 0 -ne $? && exit 1

mv "${OMCI_PATH}/${BIN_OMCI_APP}" \
    "${OMCI_PATH}/${BIN_OMCI_APP}.${OMCI_VERSION#r*}.${OMCI_VERSION#r*}.${OMCI_VERSION#r*}" && test 0 -ne $? && exit 1

mv "${OMCI_PATH}/${BIN_OMCI_CLI}" \
    "${OMCI_PATH}/${BIN_OMCI_CLI}.${OMCI_VERSION#r*}.${OMCI_VERSION#r*}.${OMCI_VERSION#r*}" && test 0 -ne $? && exit 1

rm -f ${OMCI_PATH}/Makefile && test 0 -ne $? && exit 1

# prepare Makefile for binary OMCI
MAKEFILE="BIN_OMCI_TEMPLATE.mak"
svn export ${SVN_BIN_MK_FILE} ${OMCI_PATH}/Makefile && test 0 -ne $? && exit 1

if [ -z "`cat ${OMCI_PATH}/Makefile | \
     grep "^RELEASE_VERSION" | head -n 1`" ]; then
     # Not exist RELEASE_VERSION variable
     sed -i "18 a RELEASE_VERSION="${OMCI_VERSON#r*}.${OMCI_VERSION#r*}.${OMCI_VERSION#r*}"" \
          ${OMCI_PATH}/Makefile && test 0 -ne $? && exit 1
else
     # exist RELEASE_VERSION variable
     sed -i "s/^RELEASE_VERSION.*/RELEASE_VERSION="${OMCI_VERSION#r*}.${OMCI_VERSION#r*}.${OMCI_VERSION#r*}"/g"\
         ${OMCI_PATH}/Makefile && test 0 -ne $? && exit 1
fi






	


