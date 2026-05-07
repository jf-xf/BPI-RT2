#
# Copyright (c) 2022 Realtek.com
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

IMAGEDIR := $(TOPDIR)/images
BOOTLOADER_DIR :=$(TOPDIR)/build_dir/target-aarch64-openwrt-linux-gnu_glibc/rtk_uboot-1.0
TOOLS_DIR := $(TOPDIR)/rtk_openwrt/target/linux/realtek_bb/image
DTB_SBU_PATH := realtek

ifdef CONFIG_ARCH_CORTINA_VENUS
DTS_PREFIX := ca8277B
DTS_COMP_NAME := cortina,venus
endif
ifdef CONFIG_ARCH_REALTEK_TAURUS
DTS_PREFIX := rtl8277C
DTS_COMP_NAME := realtek,taurus
endif
ifdef CONFIG_ARCH_REALTEK_9607F
DTS_PREFIX := rtl9607F
DTS_COMP_NAME := realtek,9607f
endif
DTB_FILE := $(DTS_PREFIX)-engboard.dtb

export BOOTLOADER_DIR TOOLS_DIR IMAGEDIR
export LINUX_DIR LINUX_KARCH
export DTS_PREFIX DTS_COMP_NAME
export CONFIG_UBOOT_SIGNATURE_FIT CONFIG_UBOOT_SIGN_PSS CONFIG_UBOOT_CIPHER

UBINIZE_ARGS = -m 2048 -p 128KiB -s 2048
ifdef CONFIG_OPTEE
DEVICE_CFG = ubinize-g3-optee-eng_ubi.cfg
else
DEVICE_CFG = ubinize-g3-eng_ubi.cfg
endif
LUNA_DEVICE_CFG = ubinize-luna-eng.cfg

kernel_ep = `$(KERNEL_CROSS)readelf -h $(1) | grep "Entry point address" | grep -o 0x.*`
MIPS_LOADADDR := 0x80000000

ifeq ($(CONFIG_LINUX_5_10),y)
LINUX_5_10 :=y
else
LINUX_5_10 :=n
endif

##### Secure ITS Var ########
CONFIG_UBOOT_SIGNATURE_FIT ?= n
CONFIG_UBOOT_SIGN_PSS ?= n
CONFIG_UBOOT_CIPHER ?= n

ifeq ($(CONFIG_UBOOT_SIGNATURE_FIT),y)
FIT_SIGNATURE :=y
else
FIT_SIGNATURE :=n
endif

ifeq ($(CONFIG_LUNA_FWU_SIGNATURE),y)
SIGN_FW :=y
else
SIGN_FW :=n
endif

ifneq ($(findstring aarch64,$(CONFIG_ARCH)),aarch64)
PREPARE_DM_VERITY_UBOOT_SCRIPT = $(TOOLS_DIR)/prepare-dm-verity-uboot-script.sh
DEFAULT_BOOTM_ADDR='$$$${freeAddr}'
else
PREPARE_DM_VERITY_UBOOT_SCRIPT = $(TOOLS_DIR)/prepare-dm-verity-uboot-script_g3.sh
DEFAULT_BOOTM_ADDR='$$$${unzip_src}'
endif

#define Device/Default
#  PROFILES = Default
#  KERNEL := append-dtb
#  DEVICE_DTS_DIR := ../dts-$(KERNEL_PATCHVER)
#  DEVICE_DTS = $$(SOC)_$(1)
#endef

#define Device/engboard
#  SOC := $(SUBTARGET)
#  DEVICE_VENDOR := REALTEK
#  DEVICE_MODEL := rtl9607
#endef
#TARGET_DEVICES += engboard

define Image/Prepare
	@echo "================================="
	@echo "$(SUBTARGET): Image/Prepare"
	@echo "================================="
	rm -rf $(IMAGEDIR)
	mkdir -p $(IMAGEDIR)
	- test -d ${TOPDIR}/preconfig/base-files && cp -rf ${TOPDIR}/preconfig/base-files/* $(TARGET_DIR)
endef

define Image/BuildKernel_arm64
	@echo "================================="
	@echo "$(SUBTARGET): Image/BuildKernel_arm64"
	@echo "================================="
	$(STAGING_DIR_HOST)/bin/lzma e $(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/Image $(IMAGEDIR)/Image.lzma
endef

define Image/BuildKernel_mips
	@echo "================================="
	@echo "$(SUBTARGET): Image/BuildKernel_mips"
	@echo "================================="
	cp $(LINUX_DIR)/vmlinux $(IMAGEDIR)
	$(TARGET_CROSS)strip $(IMAGEDIR)/vmlinux --remove-section=.bss
	$(TARGET_CROSS)objcopy -S -O binary $(IMAGEDIR)/vmlinux $(IMAGEDIR)/vmlinux.bin
	lzma e $(IMAGEDIR)/vmlinux.bin $(IMAGEDIR)/vmlinux.lzma
	$(call Image/BuildKernel/MkuImage,	\
		lzma,$(MIPS_LOADADDR),$(call kernel_ep,$(KDIR)/vmlinux.elf),	\
		$(IMAGEDIR)/vmlinux.lzma,$(IMAGEDIR)/uImage)
endef

define Image/BuildKernel
	@echo "================================="
	@echo "$(SUBTARGET): Image/BuildKernel"
	@echo "================================="
	$(call Image/BuildKernel_$(LINUX_KARCH))
endef

define Image/mkfs/squashfs
	@echo "================================="
	@echo "$(SUBTARGET): Image/mkfs/squashfs"
	@echo "================================="
	$(STAGING_DIR_HOST)/bin/mksquashfs4 $(TARGET_DIR) $(IMAGEDIR)/rootfs -comp xz -noappend -pf squashfs-pf.list -always-use-fragments -all-root
endef

define Image/Build_arm64/squashfs
	@echo "================================="
	@echo "$(SUBTARGET): Image/Build_arm64/squashfs"
	@echo "================================="
ifeq ($(CONFIG_DM_VERITY), y)
	cp $(IMAGEDIR)/rootfs $(IMAGEDIR)/rootfs_org
	veritysetup format --hash-block-size=4096 --hash-offset=`stat -c "%s" $(IMAGEDIR)/rootfs` $(IMAGEDIR)/rootfs $(IMAGEDIR)/rootfs | $(PREPARE_DM_VERITY_UBOOT_SCRIPT) > $(IMAGEDIR)/uboot_verity_script
else
	echo bootm $(DEFAULT_BOOTM_ADDR) > $(IMAGEDIR)/uboot_verity_script
endif
endef

define Image/Build_arm64
	@echo "================================="
	@echo "$(SUBTARGET): Image/Build_arm64"
	@echo "================================="
	@echo "CONFIG_ARCH = $(CONFIG_ARCH)"
	@echo "LINUX_DIR = $(LINUX_DIR)"
	@echo "BOOTLOADER_DIR = $(BOOTLOADER_DIR)"
	@echo "LINUX_KARCH = $(LINUX_KARCH)"
	@echo "[Prepare $(IMAGEDIR)/dtb.img]"
	$(call Image/Build_$(LINUX_KARCH)/$(1))
	cp $(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/dts/$(DTB_SBU_PATH)/$(DTB_FILE) $(IMAGEDIR)
ifeq ($(CONFIG_UIMAGE_MULTI_DTB),y)
	@echo -e "\n[gen uboot multi dtb fit images]"
	sh ./gen_fit_image.sh
else
	@echo -e "\n[gen uboot single dtb fit images]"
	sh ./gen_uboot_single_dtb_fit_uImage.sh $(TOPDIR) $(LINUX_DIR) $(DTB_FILE) $(LINUX_KARCH) $(FIT_SIGNATURE) $(LINUX_5_10) $(CONFIG_UBOOT_SIGN_PSS) $(CONFIG_UBOOT_CIPHER)
endif
	ln -fs $(IMAGEDIR)/$(DTB_FILE) $(IMAGEDIR)/dtb.img
	@echo -e "\n[Do alignment for images]"
	sh ./luna_g3_image_align.sh $(IMAGEDIR) $(DTB_FILE)
	sh ./luna_g3_image_align.sh $(IMAGEDIR) uImage
	cd $(IMAGEDIR) && ln -sf uImage Image.lzma && cd -
	@echo -e "\n[Generate $(IMAGEDIR)/vm.img]"
	sh ./mkrom_vmimg_g3.sh -ubifs $(IMAGEDIR) $(LINUX_DIR)

	@echo -e "\n[Generate $(IMAGEDIR)/img.tar]"
	cp $(IMAGEDIR)/dtb.img dtb
	cp $(IMAGEDIR)/Image.lzma kimage
	cp $(IMAGEDIR)/rootfs .
	cp fwu_g3.sh fwu.sh -f
	echo $(RTK_HW_VERSION) > hw_ver
	echo $(REVISION) > fwu_ver
	md5sum dtb kimage rootfs fwu.sh hw_ver fwu_ver md5.txt > md5.txt
	tar -cf $(IMAGEDIR)/img.tar dtb kimage rootfs fwu.sh hw_ver fwu_ver md5.txt
ifeq ($(CONFIG_LUNA_FWU_SIGNATURE),y)
	sh ./gen_sign_img.sh $(TOPDIR) $(SIGN_FW)
endif
	@echo -e "\n[Generate $(IMAGEDIR)/G3_ubi_device.ubi]"
	cp $(IMAGEDIR)/dtb.img .
	cp $(IMAGEDIR)/Image.lzma .
	$(STAGING_DIR_HOST)/bin/ubinize -o $(IMAGEDIR)/G3_ubi_device.ubi ${UBINIZE_ARGS} $(DEVICE_CFG)
ifeq ($(CONFIG_PACKAGE_rtk_uboot),y)
	@echo "================================="
	@echo "[Prepare bootloader image.......]"
	@echo "================================="
	cp $(BOOTLOADER_DIR)/image/*.img $(IMAGEDIR)/
	cp $(BOOTLOADER_DIR)/image/uboot-env*.bin $(IMAGEDIR)/uboot-env.bin
endif
	rm -rf dtb dtb.img Image.lzma kimage rootfs
endef

define Image/Build_mips
	@echo "================================="
	@echo "$(SUBTARGET): Image/Build_mips"
	@echo "================================="
	cp $(IMAGEDIR)/uImage uImage
	cp $(IMAGEDIR)/rootfs .
	cp fwu_luna_ubi.sh fwu.sh -f
	echo $(RTK_HW_VERSION) > hw_ver
	echo $(REVISION) > fwu_ver
	md5sum uImage rootfs fwu.sh hw_ver fwu_ver md5.txt > md5.txt
	tar -cf $(IMAGEDIR)/img.tar uImage rootfs fwu.sh hw_ver fwu_ver md5.txt
	@echo -e "\n[Generate $(IMAGEDIR)/ubi_device_img.ubi]"
	cp $(IMAGEDIR)/uImage .
	$(STAGING_DIR_HOST)/bin/ubinize -o $(IMAGEDIR)/ubi_device_img.ubi ${UBINIZE_ARGS} $(LUNA_DEVICE_CFG)
	rm -rf uImage rootfs
endef

define Image/Build
	@echo "================================="
	@echo "$(SUBTARGET): Image/Build"
	@echo "================================="
	$(call Image/Build_$(LINUX_KARCH),$(1))
endef

$(eval $(call BuildImage))

