#!/bin/sh

usage()
{
	printf "Usage: \n [custom2bin]:\tomci_custom2bin.sh [YourPreconfig] [sfu|hgu|hgu-fc] [omci version] [rtk drv version] [rg drv version ]\n"
}

OMCI_PWD=$(pwd | sed 's/TOOL.*//g')
BUILD_PWD=$(pwd |sed 's/linux.*//g')

function modVersion {

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

        # for fc  drv release version
        if [ -z "`cat ./DRV/platform/rtl9607_fc/Makefile |\
                 grep "^RGDRV_RELEASE_VERSION" | head -n 1`" ]; then
                # Not exist RTKDRV_RELEASE_VERSION variable
                sed -i "3 a RGDRV_RELEASE_VERSION\t\t\t= "${RGDRV_RELEASE_VERSION}""\
                         "./DRV/platform/rtl9607_fc/Makefile" && test 0 -ne $? && exit 1
        else
                # exist RTKDRV_RELEASE_VERSION variable
                sed -i "s/^RGDRV_RELEASE_VERSION.*\"$/RGDRV_RELEASE_VERSION\t\t\t= \
                        "${RGDRV_RELEASE_VERSION}"/g" \
                        "./DRV/platform/rtl9607_fc/Makefile" && test 0 -ne $? && exit 1
        fi

}

function remove_no_release_src {
	echo "keep $1"
	
	if [ "${1}" != "" ]; then
		if [ "${1}" != "hgu" ]; then
			find "${OMCI_PWD}/DRV" -name "*_rg" | xargs -n2 rm -rf && test 0 -ne $? && exit 1
		fi

		if [ "${1}" != "sfu" ]; then
			find "${OMCI_PWD}/DRV" -name "*_rtk" | xargs -n2 rm -rf && test 0 -ne $? && exit 1
		fi

		if [ "${1}" != "hgu-fc" ]; then
			find "${OMCI_PWD}/DRV" -name "*_fc" | xargs -n2 rm -rf && test 0 -ne $? && exit 1
		fi
	fi
}

if [ $# -lt 1 ]; then
	usage
	exit 1
fi

cd ${OMCI_PWD}
echo $(pwd)

modVersion $3 $4 $5

#update OMCI customized feature
#svn up (restore .c file)
#svn up $BASE_PWD
if [ "${3}" == "" ]; then
	echo "OMCI Version is refered to SVN revison... "
	OMCI_VERSION=`svn info | grep -e Rev: | sed -e 's/.*Rev: //'`
else
	echo "OMCI VERSION is "${3}""
	OMCI_VERSION="${3}"
fi
cd $BUILD_PWD

make $1 && test 0 -ne $? && exit 1
yes "xx" | make menuconfig
make all && test 0 -ne $? && exit 1

find "${OMCI_PWD}/FAL/src/features" -name "*.c" | xargs -n2 rm && test 0 -ne $? && exit 1
find "${OMCI_PWD}/FAL/src/cfg" -name "*.c" | xargs -n2 rm && test 0 -ne $? && exit 1
find "${OMCI_PWD}/DRV/features" -name "*.c" | xargs -n2 rm && test 0 -ne $? && exit 1

find "${OMCI_PWD}/DRV/features" -name "*.ko" | sed -e p -e "s/ko/ko_${OMCI_VERSION}/g" | xargs -n2 mv && test 0 -ne $? && exit 1
find "${OMCI_PWD}/FAL/src/features" -name "*.so" | sed -e p -e "s/so/so.${OMCI_VERSION}/g" | xargs -n2 mv && test 0 -ne $? && exit 1

make clean && test 0 -ne $? && exit 1

#remove and replace back src for open customized feature.
cd ${OMCI_PWD}
grep -v "^#" ${OMCI_PWD}/TOOL/openCustomized.lst > tmpOpenlst
while read svnline;do
	if [ "$svnline" == "" ];then
		continue 
	fi
	#remove open customized releated
	find "${OMCI_PWD}" -name "*$svnline*" | xargs -n2 rm -rf && test 0 -ne $? && exit 1
	#restore open customized .c file
	cd ${OMCI_PWD}
	svn export http://cadinfo.realtek.com.tw/svn/CN/Switch/PON/private/rtk_omci_v1/omci_v1/DRV/features/$svnline/ ./DRV/features/$svnline/
	svn export http://cadinfo.realtek.com.tw/svn/CN/Switch/PON/private/rtk_omci_v1/omci_v1/FAL/src/features/$svnline.c ./FAL/src/features/$svnline.c
done < tmpOpenlst

remove_no_release_src $2

if [ "${2}" == "" ]; then
	echo "please delete pf_rtk for HGU device, or delete pf_rg for SFU device."
fi

