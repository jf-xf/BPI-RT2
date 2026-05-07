#
# Copyright (c) 2022 Realtek.com
#

include $(TOPDIR)/rules.mk

BOARD:=realtek_bb
BOARDNAME:=Realtek Broadband
SUBTARGETS:=rtl9617c rtl9607f rtl9607c
KERNEL_PATCHVER:=5.10
FEATURES:=squashfs nand ubifs
CONFIG_TARGET:=subtarget

ifneq ($(wildcard ./config-default),)
CONFIG_TARGET=subtarget_target
endif

include $(INCLUDE_DIR)/target.mk

#define target/description
#	build firmware images for realtek based boards
#endef

# For Realtek OpenWrt SDK
KERNEL_VERSION = $(LINUX_VERSION)

ROOTDIR = $(KERNEL_BUILD_DIR)
LINUXDIR = $(shell basename $(LINUX_DIR))
KDIR = $(LINUX_DIR)
NETDIR = $(KDIR)/drivers/net/ethernet/realtek
NICDIR = .
APOLLODIR = $(NETDIR)/rtl86900
SDKDIR = $(APOLLODIR)/sdk
CA_PACKAGES_SRC = $(KERNEL_BUILD_DIR)/ca_packages

## force change KERNEL build time
LOCAL_TIMEZONE:=$(shell cat /etc/timezone)
KBUILD_BUILD_TIMESTAMP:=$(shell perl -e '$$ENV{"TZ"}="$(LOCAL_TIMEZONE)"; print scalar localtime')
SOURCE_DATE_EPOCH=$(shell date +%s -d '${KBUILD_BUILD_TIMESTAMP} UTC')
export SOURCE_DATE_EPOCH KBUILD_BUILD_TIMESTAMP
## 

KBUILD_EXTRA_SYMBOLS = $(CA_PACKAGES_SRC)/ca-network-engine/1.0-r0/ca-network-engine-1.0/Module.symvers

export KERNEL_VERSION ROOTDIR LINUXDIR KDIR NETDIR NICDIR APOLLODIR SDKDIR KBUILD_EXTRA_SYMBOLS
##  Force linux-5 modpost treat  "modpost: xxx undefined!" as warning.
export KBUILD_MODPOST_WARN=1

ifeq ("$(ARCH)", "aarch64")
  DTS_DIR = $(LINUX_DIR)/arch/arm64/boot/dts/realtek
else ifeq ("$(ARCH)", "aarch64")
  DTS_DIR = $(LINUX_DIR)/arch/arm/boot/dts/realtek
else 
  DTS_DIR = $(LINUX_DIR)/arch/mips/boot/dts/realtek
endif

define RTK_VOIP/Prepare
	mkdir -p $(LINUX_DIR)/rtk_voip/
	$(CP) $(TOPDIR)/rtk_voip/kernel/* $(LINUX_DIR)/rtk_voip/
	ln -nsf $(LINUX_DIR)/rtk_voip $(LINUX_DIR)/drivers/
	mkdir -p $(LINUX_DIR)/drivers/char/aipc/
	$(CP) $(TOPDIR)/rtk_voip/aipc_char/* $(LINUX_DIR)/drivers/char/aipc/
	ln -nsf $(LINUX_DIR) $(LINUX_DIR)/source
endef

ifeq ($(CONFIG_PACKAGE_kmod-ca_packages),y)
include $(TOPDIR)/rtk_openwrt/package/kernel/realtek/ca_packages/build.mk
endif

define Kernel/Prepare
	$(call Kernel/Prepare/Default)
	$(call RTK_VOIP/Prepare)
ifeq ($(CONFIG_PACKAGE_kmod-ca_packages),y)
	$(call CA_PACKAGES/Prepare)
endif
	- test -d ${TOPDIR}/preconfig/dts && cp -f ${TOPDIR}/preconfig/dts/* $(DTS_DIR)/
endef

ifeq ($(CONFIG_PACKAGE_kmod-ca_packages),y)
define Kernel/CompileModules
	+$(KERNEL_MAKE) oldconfig
	+$(KERNEL_MAKE) prepare
	+$(KERNEL_MAKE) scripts
	+$(KERNEL_MAKE) modules_prepare
	$(call CA_PACKAGES/Compile)
	$(call Kernel/CompileModules/Default)
endef
endif

$(eval $(call BuildTarget))

