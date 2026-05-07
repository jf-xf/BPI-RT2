#!/bin/sh
#set -x
#################################################################
# Venus Partitions 
#################################################################
#device nand0 <ca_spinand_flash>, # parts = 5
# #: name                size            offset          mask_flags
# 0: boot                0x00180000      0x00000000      0
# 1: env                 0x00040000      0x00280000      0
# 2: env2                0x00040000      0x002C0000      0
# 3: static_conf         0x00040000      0x00300000      0
# 4: ubi_device          0x0f5c0000      0x00340000      0
#################################################################
ROOTDIR=`pwd`
#BOOTLOADER_DIR=${ROOTDIR}/bootloader
BOOTLOADER_DIR=${ROOTDIR}/build_dir/target-aarch64-openwrt-linux-gnu_glibc/rtk_uboot-1.0
PROG_IMG_DIR=${ROOTDIR}/images/programmer_images

PLATFORM_CONFIG="Flash_and_DDR_Support.txt"
pfl_bbl1_r=0x200000
pfl_bbl2_r=0x240000
pfl_env1_r=0x280000
pfl_env2_r=0x2C0000
STATIC_CONFIG_SIZE=262144

BOOTLOADER_IMG="bootloader.bin"
UBOOTENV_IMG="uboot-env.bin"
UBI_DEVICE_IMG="G3_ubi_device.ubi"

ALL_ONE_FILE="all_one_file.bin"

COMBINED_LOADER_IMAGE="combined_loader_image.bin"
COMBINED_ONEBIN_IMAGE="combined_onebin_image.bin"

SFLASH_PROGGRAMER_IMG_LOADER="SFLASH_PROGGRAMER_LOADER.bin"
SFLASH_PROGGRAMER_IMG_COMBINED="SFLASH_PROGGRAMER_FULLSYS.bin"


CHUNK_SIZE=4096
#usage() {
#     echo "prog <in-filename> <out-filename> <thread_n>"
#}
# func <in-filename> <out-filename> <thread_n>
bin2nand_split() {
    if [ "$#" -lt 2 ]; then
        usage
        exit 1
    fi 

    if [ ! -f "$1" ]; then
        echo "$1 not exist"
        exit 1
    fi

    let cpu_nums=`cat /proc/cpuinfo | grep processor | wc -l`
    
    set -e
    let file_size=`stat -c '%s' $1`
    let split_size="($file_size/$cpu_nums+$CHUNK_SIZE-1)/$CHUNK_SIZE*$CHUNK_SIZE"

    split -b $split_size $1 -d tmp_

    for file in tmp_*; do 
    {
        ${BIN2NAND_TOOL} $file --ecc-layout=BCH16 --pagesize=$PAGE_SIZE > fl_$file
        rm -f $file
    } &
    done

    wait

    cat  fl_tmp_* > $2
    rm -f fl_tmp_*

    set +e
}


get_image_combine_addr(){
    echo "------------------------------------" 
    bbl1_addr=`printf "%d" ${pfl_bbl1_r}`
    bbl2_addr=`printf "%d" ${pfl_bbl2_r}`
    bbl_size=$(( $bbl2_addr - $bbl1_addr ))
    env1_addr=`printf "%d" ${pfl_env1_r}`
    env2_addr=`printf "%d" ${pfl_env2_r}`
    env_size=$(( $env2_addr - $env1_addr ))
    static_addr=$(( $env2_addr + $env_size ))
    ubidev_addr=$(( $static_addr + $STATIC_CONFIG_SIZE ))

    #### Generate ALL ONE file for padding
    tr '\000' '\377' < /dev/zero| dd of=${ALL_ONE_FILE} bs=1 count=${STATIC_CONFIG_SIZE}    

    
    echo "get_image_combine_addr: bbl1_addr=${bbl1_addr}"
    echo "get_image_combine_addr: bbl2_addr=${bbl2_addr}"
    echo "get_image_combine_addr: env1_addr=${env1_addr}"
    echo "get_image_combine_addr: env2_addr=${env2_addr}"

    if [ "${ONLY_BOOTLOADER}" = 0 ];then
        echo "get_image_combine_addr: static_addr=${static_addr}"
        echo "get_image_combine_addr: ubidev_addr=${ubidev_addr}"
    fi
}

gen_sflash_image(){
    echo "------------------------------------" 
    echo "@Generate SPI flash combined image of bootloader, uboot-env, ubi device images"

    ln -s ${BOOTLOADER_DIR}/image/*loader*.img ${BOOTLOADER_DIR}/image/${BOOTLOADER_IMG}
    ln -s ${BOOTLOADER_DIR}/image/*-env*.bin ${BOOTLOADER_DIR}/image/${UBOOTENV_IMG}
    
    echo "dd if=${BOOTLOADER_DIR}/image/${BOOTLOADER_IMG} of=${COMBINED_LOADER_IMAGE} seek=0  conv=notrunc" | tee -a ./.programmer-image-combine.log
#    echo "dd if=${ALL_ONE_FILE} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${bbl1_addr}  conv=notrunc" | tee -a ./.programmer-image-combine.log
#    echo "dd if=${ALL_ONE_FILE} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${bbl2_addr}  conv=notrunc" | tee -a ./.programmer-image-combine.log
#    echo "dd if=${ALL_ONE_FILE} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${env1_addr}  conv=notrunc" | tee -a ./.programmer-image-combine.log
#    echo "dd if=${ALL_ONE_FILE} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${env2_addr}  conv=notrunc" | tee -a ./.programmer-image-combine.log
    echo "dd if=${BOOTLOADER_DIR}/image/${UBOOTENV_IMG} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${env1_addr}  conv=notrunc" | tee -a ./.programmer-image-combine.log
    echo "dd if=${BOOTLOADER_DIR}/image/${UBOOTENV_IMG} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${env2_addr}  conv=notrunc" | tee -a ./.programmer-image-combine.log
    dd if=${BOOTLOADER_DIR}/image/${BOOTLOADER_IMG} of=${COMBINED_LOADER_IMAGE} seek=0  conv=notrunc 
    dd if=${ALL_ONE_FILE} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${bbl1_addr}  count=$bbl_size conv=notrunc
    dd if=${ALL_ONE_FILE} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${bbl2_addr}  conv=notrunc
    dd if=${ALL_ONE_FILE} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${env1_addr}  conv=notrunc
    dd if=${ALL_ONE_FILE} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${env2_addr}  conv=notrunc
    dd if=${BOOTLOADER_DIR}/image/${UBOOTENV_IMG} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${env1_addr}  conv=notrunc
    dd if=${BOOTLOADER_DIR}/image/${UBOOTENV_IMG} of=${COMBINED_LOADER_IMAGE} bs=1 seek=${env2_addr}  conv=notrunc
    if [ "${BOOTDEV}" = "SPI" ];then
        echo "cp ${COMBINED_LOADER_IMAGE} ${SFLASH_PROGGRAMER_IMG_LOADER}" | tee -a ./.programmer-image-combine.log          
        cp ${COMBINED_LOADER_IMAGE} ${SFLASH_PROGGRAMER_IMG_LOADER}
    fi

    if [ "${ONLY_BOOTLOADER}" = 0 ];then
        cp ${COMBINED_LOADER_IMAGE} ${COMBINED_ONEBIN_IMAGE}
#        echo "gen_combined_image: static_addr=${static_addr}"
#        echo "gen_combined_image: ubidev_addr=${ubidev_addr}"
#        echo "dd if=${ALL_ONE_FILE} of=${COMBINED_ONEBIN_IMAGE} bs=1 seek=${static_addr}  conv=notrunc" | tee -a ./.programmer-image-combine.log
        echo "dd if=${ROOTDIR}/images/${UBI_DEVICE_IMG} of=${COMBINED_ONEBIN_IMAGE} bs=1 seek=${ubidev_addr}  conv=notrunc" | tee -a ./.programmer-image-combine.log
        dd if=${ALL_ONE_FILE} of=${COMBINED_ONEBIN_IMAGE} bs=1 seek=${static_addr}  conv=notrunc
        dd if=${ROOTDIR}/images/${UBI_DEVICE_IMG} of=${COMBINED_ONEBIN_IMAGE} bs=1 seek=${ubidev_addr}  conv=notrunc
        
        if [ "${BOOTDEV}" = "SPI" ];then
            echo "cp ${COMBINED_ONEBIN_IMAGE} ${SFLASH_PROGGRAMER_IMG_COMBINED}" | tee -a ./.programmer-image-combine.log          
            cp ${COMBINED_ONEBIN_IMAGE} ${SFLASH_PROGGRAMER_IMG_COMBINED}
        fi
    fi
   
    rm ${BOOTLOADER_DIR}/image/${BOOTLOADER_IMG}
    rm ${BOOTLOADER_DIR}/image/${UBOOTENV_IMG} 
}


################################
# main
################################
if [ ! -d ${ROOTDIR}/images ];then
    echo "Can't find ${ROOTDIR}/images !"
    exit 1
fi

if [ ! -d ${PROG_IMG_DIR} ];then
    mkdir  ${PROG_IMG_DIR}
else
    rm -rf ${PROG_IMG_DIR}/*
fi

#Step1: Check only bootloader
ONLY_BOOTLOADER="$1"
if [ x"${ONLY_BOOTLOADER}" = x ];then
    ONLY_BOOTLOADER=0
else
    ONLY_BOOTLOADER=1
fi
echo "ONLY_BOOTLOADER=${ONLY_BOOTLOADER}"


#Step2: cd ${ROOTDIR}/programmer_images
echo "ROOTDIR=${ROOTDIR}"
echo "BOOTLOADER_DIR=${BOOTLOADER_DIR}"
echo "PROG_IMG_DIR=${PROG_IMG_DIR}"
cd ${PROG_IMG_DIR}

#Step3: get address for combine image
get_image_combine_addr

#Step4: cd ${ROOTDIR}/programmer_images
gen_sflash_image

#Step5: Remove tmpp files
rm ${ALL_ONE_FILE}
