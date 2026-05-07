#!/bin/bash

usage()
{
	printf "Usage: \n [Release]:\tsrc2bin.sh v1.9.0.5 v1.9.0.5 v1.9.0.5\n"
	printf "\t\t\t Note: This script put at ./omci_v1/ if you change src to bin\n"
	printf "\t\t 1th = omci user application version \n"
	printf "\t\t 2th = omci rtk driver version\n"
	printf "\t\t 3th = omci rg driver version\n"
	
}


if [ $# -lt 2 ]; then
	usage
	exit 1
fi

BIN_OMCI_APP="omci_app"
BIN_OMCI_CLI="omcicli"

SVN_BIN_MK_FILE="http://cadinfo.realtek.com.tw/svn/CN/Switch/PON/public/applications/rtk_omci_release/BIN_OMCI_TEMPLATE.mak"

RELEASE_VERSION="\\\\\"$1\\\\\""
		
	sed -i '/^RELEASE_SUBVERSION/d' \
		"./Makefile" && test 0 -ne $? && exit 1
		
	if [ -z "`cat ./Makefile |\
		 grep "^RELEASE_VERSION" | head -n 1`" ]; then
		# Not exist RELEASE_VERSION variable
		sed -i "20 a RELEASE_VERSION\t\t\t= "${RELEASE_VERSION}""\
			 "./Makefile" && test 0 -ne $? && exit 1
	else
		# exist RELEASE_VERSION variable
		sed -i "s/^RELEASE_VERSION.*\"$/RELEASE_VERSION\t\t\t= \
			"${RELEASE_VERSION}"/g" "./Makefile" && test 0 -ne $? && exit 1
	fi

RTKDRV_RELEASE_VERSION="\\\\\"$2\\\\\""

	# for rtk drv release version
	if [ -z "`cat ./DRV/platform/rtl9607_rtk/Makefile |\
		 grep "^RTKDRV_RELEASE_VERSION" | head -n 1`" ]; then
		# Not exist RTKDRV_RELEASE_VERSION variable
		sed -i "3 a RTKDRV_RELEASE_VERSION\t\t\t= "${RTKDRV_RELEASE_VERSION}""\
			 "./DRV/platform/rtl9607_rtk/Makefile" && test 0 -ne $? && exit 1
	else
		# exist RTKDRV_RELEASE_VERSION variable
		sed -i "s/^RTKDRV_RELEASE_VERSION.*\"$/RTKDRV_RELEASE_VERSION\t\t\t= \
			"${RTKDRV_RELEASE_VERSION}"/g" \
			"./DRV/platform/rtl9607_rtk/Makefile" && test 0 -ne $? && exit 1
	fi

RGDRV_RELEASE_VERSION="\\\\\"$3\\\\\""

	# for rg  drv release version
	if [ -z "`cat ./DRV/platform/rtl9607_rg/Makefile |\
		 grep "^RGDRV_RELEASE_VERSION" | head -n 1`" ]; then
		# Not exist RTKDRV_RELEASE_VERSION variable
		sed -i "3 a RGDRV_RELEASE_VERSION\t\t\t= "${RGDRV_RELEASE_VERSION}""\
			 "./DRV/platform/rtl9607_rg/Makefile" && test 0 -ne $? && exit 1
	else
		# exist RTKDRV_RELEASE_VERSION variable
		sed -i "s/^RGDRV_RELEASE_VERSION.*\"$/RGDRV_RELEASE_VERSION\t\t\t= \
			"${RGDRV_RELEASE_VERSION}"/g" \
			"./DRV/platform/rtl9607_rg/Makefile" && test 0 -ne $? && exit 1
	fi
 
	rm -rf ./LIB/lib*
	rm -rf ./LIB/omci/mib*

# prepare binary OMCI
	find . -name "*.c" | xargs -n2 rm && test 0 -ne $? && exit 1
	find . -name "*.o" | xargs -n2 rm && test 0 -ne $? && exit 1
	find . -name "*.d" | xargs -n2 rm && test 0 -ne $? && exit 1
	rm -rf "./LIB/omci/*.so" && test 0 -ne $? && exit 1
	find "./DRV" -name "*.cmd" | xargs -n2 rm && test 0 -ne $? && exit 1
	find "./DRV" -name "modules.order" | xargs -n2 rm && test 0 -ne $? && exit 1
	find "./DRV" -name "Module.symvers" | xargs -n2 rm && test 0 -ne $? && exit 1
	find "./DRV" -name ".tmp_versions" | xargs -n2 rm -rf
	find "./DRV" -name ".mod" | xargs -n2 rm && test 0 -ne $? && exit 1

# All binaray files will be removed if make clean from sdk path. 
#Therefore, change name of omci binaray file 
	find . -name "*.so" | \
		sed -e p -e "s/so/so."${1#v*}.${2#v*}.${3#v*}"/g" | xargs -n2 mv && test 0 -ne $? && exit 1

	find . -name "*.ko" | \
		sed -e p -e "s/ko/ko_"${1#v*}.${2#v*}.${3#v*}"/g" | xargs -n2 mv && test 0 -ne $? && exit 1

	mv "./${BIN_OMCI_APP}" \
		"./${BIN_OMCI_APP}.${1#v*}.${2#v*}.${3#v*}" && test 0 -ne $? && exit 1

	mv "./${BIN_OMCI_CLI}" \
		"./${BIN_OMCI_CLI}.${1#v*}.${2#v*}.${3#v*}" && test 0 -ne $? && exit 1

	rm -f ./Makefile && test 0 -ne $? && exit 1
			
# prepare Makefile for binary OMCI
MAKEFILE="BIN_OMCI_TEMPLATE.mak"
svn export ${SVN_BIN_MK_FILE} ./Makefile && test 0 -ne $? && exit 1

	if [ -z "`cat ./Makefile | \
		grep "^RELEASE_VERSION" | head -n 1`" ]; then
		# Not exist RELEASE_VERSION variable
		sed -i "18 a RELEASE_VERSION="${1#v*}.${2#v*}.${3#v*}"" \
			./Makefile && test 0 -ne $? && exit 1
	else
		# exist RELEASE_VERSION variable
		sed -i "s/^RELEASE_VERSION.*/RELEASE_VERSION="${1#v*}.${2#v*}.${3#v*}"/g"\
			 ./Makefile && test 0 -ne $? && exit 1
	fi


