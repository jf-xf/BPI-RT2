#!/bin/sh

usage()
{
	printf "Usage: \n [custom2bin]:\tomci_custom2bin.sh [YourPreconfig]\n"
}

if [ $# -lt 1 ]; then
	usage
	exit 1
fi

OMCI_PWD=$(pwd | sed 's/TOOL.*//g')
BUILD_PWD=$(pwd |sed 's/linux.*//g')

cd ${OMCI_PWD}
echo $(pwd)

#update OMCI customized feature
#svn up (restore .c file)
#svn up $BASE_PWD
if [ "${2}" == "" ]; then
	echo "OMCI Version is refered to SVN revison... "
	OMCI_VERSION=`svn info | grep -e Rev: | sed -e 's/.*Rev: //'`
else
	echo "OMCI VERSION is "${2}""
	OMCI_VERSION="${2}"
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


