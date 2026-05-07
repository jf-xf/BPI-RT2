#/bin/sh
#Generate openwrt firmware upgarde image tar ball
ROOTDIR=$1
SIGN_FW=$2
image_dir=${ROOTDIR}/images

#echo "$ROOTDIR"
#echo "$SIGN_FW"
#echo "$image_dir"
# For FW sigature
fw_tar_file="$image_dir/img.tar"
all_files_to_sign="$fw_tar_file"
openssl_exe="openssl"
unsigned_fw_suffix="orig"
openssl_dgst_algo="-sha256"
openssl_private_key_name="fwu_private.pem"
openssl_private_key_path=$ROOTDIR/rtk_openwrt/package/realtek/rtk_mib/files/security/$openssl_private_key_name
#echo "--->$openssl_private_key_path"

echo all files list: $all_files_to_sign
for file in $all_files_to_sign
do
	if [ -f $file ]; then
		echo "signature $file...."
		mv $file $file.$unsigned_fw_suffix
		if [ "$SIGN_FW" = "y" ]; then
			$openssl_exe dgst $openssl_dgst_algo -sign $openssl_private_key_path -out $file.sig $file.$unsigned_fw_suffix
		else
			$openssl_exe dgst $openssl_dgst_algo -binary -out $file.sig $file.$unsigned_fw_suffix
		fi
		cat $file.$unsigned_fw_suffix $file.sig > $file
	fi
done

